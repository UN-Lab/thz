/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 University at Buffalo, the State University of New York
 * (http://ubnano.tech/)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Zahed Hossain <zahedhos@buffalo.edu>
 *         Qing Xia <qingxia@buffalo.edu>
 *         Josep Miquel Jornet <jmjornet@buffalo.edu>
 */


#include "ns3/attribute.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"
#include "ns3/log.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/node.h"
#include "ns3/rng-seed-manager.h"
#include "thz-energy-model.h"
#include "thz-dir-antenna.h"

#include "thz-mac-header.h"
#include "thz-mac-nano.h"

#include <vector>
#include <iostream>
#include <fstream>


NS_LOG_COMPONENT_DEFINE ("THzMacNano");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (THzMacNano);

THzMacNano::THzMacNano ()
  : THzMac (),
  m_phy (0),
  m_pktData (0)

{
  m_throughputAll = 0;
  m_sequence = 0;
  m_ite = 0;
  m_discarded = 0;

  Simulator::Schedule (MicroSeconds (0.0), &THzMacNano::InitEnergyCallback, this);
  Simulator::Schedule (NanoSeconds (3.25), &THzMacNano::SetAntenna, this); // initialization: turn antenna mode as Omnidirectional mode at all devices
}
THzMacNano::~THzMacNano ()
{
  Clear ();
}
void
THzMacNano::Clear ()
{
  m_pktData = 0;
  m_pktQueue.clear ();
  m_seqList.clear ();
  m_throughput = 0;
  m_throughputAll = 0;
}

TypeId
THzMacNano::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::THzMacNano")
    .SetParent<Object> ()
    .AddConstructor<THzMacNano> ()
    .AddAttribute ("EnableRts",
                   "If true, RTS is enabled",
                   BooleanValue (false),
                   MakeBooleanAccessor (&THzMacNano::m_rtsEnable),
                   MakeBooleanChecker ())
    .AddAttribute ("SlotTime",
                   "Time slot duration for MAC backoff",
                   TimeValue (MicroSeconds (8)),
                   MakeTimeAccessor (&THzMacNano::m_slotTime),
                   MakeTimeChecker ())
    .AddAttribute ("QueueLimit",
                   "Maximum packets to queue at MAC",
                   UintegerValue (1000),
                   MakeUintegerAccessor (&THzMacNano::m_queueLimit),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("DataRetryLimit",
                   "Maximum Limit for Data Retransmission",
                   UintegerValue (5),
                   MakeUintegerAccessor (&THzMacNano::m_dataRetryLimit),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("FrameLength",
                   "Actual packet length at the MAC layer",
                   UintegerValue (5),
                   MakeUintegerAccessor (&THzMacNano::m_FrameLength),
                   MakeUintegerChecker<uint16_t> ())
    .AddTraceSource ("CtsTimeout",
                     "Trace Hookup for CTS Timeout",
                     MakeTraceSourceAccessor (&THzMacNano::m_traceCtsTimeout),
                     "ns3::THzMac::TimeTracedCallback")
    .AddTraceSource ("AckTimeout",
                     "Trace Hookup for ACK Timeout",
                     MakeTraceSourceAccessor (&THzMacNano::m_traceAckTimeout),
                     "ns3::THzMac::TimeTracedCallback")
    .AddTraceSource ("SendDataDone",
                     "Trace Hookup for sending a data",
                     MakeTraceSourceAccessor (&THzMacNano::m_traceSendDataDone),
                     "ns3::THzMac::SendDataDoneTracedCallback")
    .AddTraceSource ("Enqueue",
                     "Trace Hookup for enqueue a data",
                     MakeTraceSourceAccessor (&THzMacNano::m_traceEnqueue),
                     "ns3::THzMac::TimeTracedCallback")
    .AddTraceSource ("Throughput",
                     "Trace Hookup for Throughput",
                     MakeTraceSourceAccessor (&THzMacNano::m_traceThroughput),
                     "ns3::THzMac::ThroughputTracedCallback")
  ;
  return tid;
}
// ------------------------ Set Functions -----------------------------
void
THzMacNano::SetAntenna ()
{
  NS_LOG_FUNCTION ("");
  // set as omnidirectional
  double turnSpeed = 0;
  double MaxGain = 40; //dB
  double beamwidthDegrees = 360;
  m_thzAD = m_device->GetDirAntenna ();
  m_thzAD->SetAttribute ("TuneRxTxMode", DoubleValue (2.0));
  NS_LOG_DEBUG ( "Tune as OmnidirectionalMode At node: " << m_device->GetNode ()->GetId () << " Antenna Mode: " << m_thzAD->CheckAntennaMode () );
  m_thzAD->SetAttribute ("InitialAngle", DoubleValue (0.0));
  m_thzAD->SetMaxGain (MaxGain);
  m_thzAD->SetBeamwidth (beamwidthDegrees);
  m_thzAD->SetRxTurningSpeed (turnSpeed);
}
void
THzMacNano::AttachPhy (Ptr<THzPhy> phy)
{
  m_phy = phy;
}
void
THzMacNano::InitEnergyCallback ()
{
  m_device->GetNode ()->GetObject<THzEnergyModel> ()->SetEnergyCallback (MakeCallback (&THzMacNano::TxFirstPacket, this));
  /*----------------------------------------------------------------------------------------
   * enable the result printing in a .txt file by uncommenting the content below
   *----------------------------------------------------------------------------------------*/
  /*std::ofstream myfile;
  myfile.open ("nano_2way_sucessful.txt", std::ofstream::out | std::ios::app);
  myfile.close ();
  std::ofstream myfile1;
  myfile1.open ("nano_2way_discarded.txt", std::ofstream::out | std::ios::app);
  myfile1.close ();*/
}
void
THzMacNano::SetDevice (Ptr<THzNetDevice> dev)
{
  m_device = dev;
}
void
THzMacNano::SetAddress (Mac48Address addr)
{
  NS_LOG_FUNCTION (addr);
  m_address = addr;
  // to help each node have different random seed
  uint8_t tmp[6];
  m_address.CopyTo (tmp);
  SeedManager::SetSeed (tmp[5] + 9);
}
void
THzMacNano::SetForwardUpCb (Callback<void, Ptr<Packet>, Mac48Address, Mac48Address> cb)
{
  m_forwardUpCb = cb;
}

void
THzMacNano::SetSlotTime (Time duration)
{
  m_slotTime = duration;
}
// ------------------------ Get Functions -----------------------------

Time
THzMacNano::GetSlotTime (void)
{
  return m_slotTime;
}

Mac48Address
THzMacNano::GetAddress () const
{
  return this->m_address;
}

Mac48Address
THzMacNano::GetBroadcast (void) const
{
  return Mac48Address::GetBroadcast ();
}
Time
THzMacNano::GetCtrlDuration (uint16_t type)
{
  THzMacHeader header = THzMacHeader (m_address, m_address, type);
  return m_phy->CalTxDuration (0, header.GetSize ());
}
Time
THzMacNano::GetDataDuration (Ptr<Packet> p)
{
  return m_phy->CalTxDuration (0, p->GetSize ());
}

// ----------------------- Queue Functions -----------------------------
bool
THzMacNano::Enqueue (Ptr<Packet> packet, Mac48Address dest)
{
  NS_LOG_DEBUG ("---------------------------------------------------------------------------------------------------");
  NS_LOG_FUNCTION ("         Time: " << Simulator::Now () << " at node: " << m_address << " Energy: " << m_device->GetNode ()->GetObject<THzEnergyModel> ()->GetRemainingEnergy () << "queue size" << m_pktQueue.size () << " dest:" << dest);

  if (packet->GetSize () == 64)
    {
      return true;
    }

  if (m_pktQueue.size () >= m_queueLimit)
    {
      return false;
    }
  m_traceEnqueue (m_device->GetNode ()->GetId (), m_device->GetIfIndex ());
  ++m_sequence;
  NS_LOG_DEBUG ("enqueued seq: " << m_sequence);
  THzMacHeader header = THzMacHeader (m_address, dest, THZ_PKT_TYPE_DATA);
  header.SetSequence (m_sequence);
  packet->AddHeader (header);
  m_pktQueue.push_back (packet);

  PktTx ot; //to keep track of tstart and retries for each packet
  ot.retry = 0;
  ot.destination = dest;
  ot.packet = packet;
  ot.sequence = m_sequence;
  ot.tstart = Simulator::Now ();

  std::list<PktTx>::iterator it = m_pktTx.begin ();
  for (; it != m_pktTx.end (); ++it)
    {
      Ptr<Packet> data = it->packet;
      THzMacHeader dataHeader;
      data->PeekHeader (dataHeader);
      if (dataHeader.GetSequence () < ot.sequence && dataHeader.GetDestination () == ot.destination)
        {
          NS_LOG_DEBUG ("same dest tx ");
          m_pktTx.push_back (ot);
          return true;
        }
    }

  m_pktTx.push_back (ot);
  TxFirstPacket ();
  return true;
}

void
THzMacNano::CheckResources (Ptr<Packet> packet)
{
  NS_LOG_DEBUG ("---------------------------------------------------------------------------------------------------");
  NS_LOG_FUNCTION ( "Time: " << Simulator::Now () << " at node: " << m_address << " Energy: " << m_device->GetNode ()->GetObject<THzEnergyModel> ()->GetRemainingEnergy () << "queue size" << m_pktQueue.size ());
  THzMacHeader header;
  packet->PeekHeader (header);
  NS_LOG_DEBUG ("dest : " << header.GetDestination ());
  if (header.GetDestination () != GetBroadcast () && m_rtsEnable == true)
    {
      THzMacHeader rtsHeader = THzMacHeader (m_address, m_address, THZ_PKT_TYPE_RTS);
      uint32_t controlPacketLength = rtsHeader.GetSize ();
      if (m_device->GetNode ()->GetObject<THzEnergyModel> ()->BookEnergy (packet->GetSize () + controlPacketLength, 2 * controlPacketLength))
        {
          NS_LOG_DEBUG ("Rem Energy after SendRTS: " << m_device->GetNode ()->GetObject<THzEnergyModel> ()->GetRemainingEnergy ());
          SendRts (packet);
          return;
        }
    }
  else
    {
      THzMacHeader ackHeader = THzMacHeader (m_address, m_address, THZ_PKT_TYPE_ACK);
      if (m_device->GetNode ()->GetObject<THzEnergyModel> ()->BookEnergy (packet->GetSize (), ackHeader.GetSize ()))
        {
          NS_LOG_DEBUG ("Rem Energy after SendData: " << m_device->GetNode ()->GetObject<THzEnergyModel> ()->GetRemainingEnergy ());
          SendData (packet);
          return;
        }
    }
}

void
THzMacNano::Backoff (Ptr<Packet> packet, uint32_t retry)
{
  NS_LOG_DEBUG ("---------------------------------------------------------------------------------------------------");
  NS_LOG_FUNCTION ( "Time: " << Simulator::Now () << " at node: " << m_device->GetNode ()->GetId () << " Energy: " << m_device->GetNode ()->GetObject<THzEnergyModel> ()->GetRemainingEnergy ());
  RngSeedManager seed;
  seed.SetSeed (static_cast<unsigned int> (time (0)));
  Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
  uint32_t bo = uv->GetInteger (1, pow (double(2.0), double(retry)));
  m_backoffRemain = Seconds ((double)(bo) * GetSlotTime ().GetSeconds ());
  NS_LOG_DEBUG ("backoff time : " << m_backoffRemain);
  Simulator::Schedule (m_backoffRemain, &THzMacNano::CheckResources, this, packet);
}
void
THzMacNano::Dequeue (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (m_pktQueue.size ());
  m_pktQueue.remove (packet);
}

// ----------------------- Send Functions ------------------------------
void
THzMacNano::TxFirstPacket ()
{
  NS_LOG_DEBUG ("---------------------------------------------------------------------------------------------------");
  NS_LOG_FUNCTION ("   Time: " << Simulator::Now () << " at node: " << m_address << " Energy: " << m_device->GetNode ()->GetObject<THzEnergyModel> ()->GetRemainingEnergy () << "queue size" << m_pktQueue.size ());

  if (m_pktQueue.empty ())
    {
      return;
    }
  m_pktData = m_pktQueue.front ();
  CheckResources (m_pktData);
}

void
THzMacNano::SendRts (Ptr<Packet> packet)
{
  NS_LOG_DEBUG ("---------------------------------------------------------------------------------------------------");
  NS_LOG_FUNCTION ( "Time: " << Simulator::Now () << " at node: " << m_address << " Energy: " << m_device->GetNode ()->GetObject<THzEnergyModel> ()->GetRemainingEnergy ());

  THzMacHeader dataHeader;
  packet->PeekHeader (dataHeader);

  Ptr<Packet> rtsPacket = Create<Packet> (0);
  THzMacHeader rtsHeader = THzMacHeader (m_address, dataHeader.GetDestination (), THZ_PKT_TYPE_RTS);
  rtsHeader.SetSequence (dataHeader.GetSequence ());

  rtsHeader.SetDuration (Seconds (0));
  rtsPacket->AddHeader (rtsHeader);

  Time ctsTimeout = GetCtrlDuration (THZ_PKT_TYPE_RTS)
    + GetCtrlDuration (THZ_PKT_TYPE_CTS)
    + PicoSeconds (666) + PicoSeconds (10);

  NS_LOG_DEBUG ("CTS timeout " << ctsTimeout << "s");
  CtsTimeouts ct;
  ct.sequence = dataHeader.GetSequence ();

  SendPacket (rtsPacket, 1);
  ct.m_ctsTimeoutEvent = Simulator::Schedule (ctsTimeout, &THzMacNano::CtsTimeout, this, packet);
  m_ctsTimeouts.push_back (ct);
}
void
THzMacNano::SendCts (Mac48Address dest, uint16_t sequence)
{
  NS_LOG_DEBUG ("---------------------------------------------------------------------------------------------------");
  NS_LOG_FUNCTION ("        Time: " << Simulator::Now () << " at node: " << m_address << " Energy: " << m_device->GetNode ()->GetObject<THzEnergyModel> ()->GetRemainingEnergy () << " to: " << dest);

  Ptr<Packet> packet = Create<Packet> (0);
  THzMacHeader ctsHeader = THzMacHeader (m_address, dest, THZ_PKT_TYPE_CTS);

  ctsHeader.SetDuration (Seconds (0));
  ctsHeader.SetSequence (sequence);
  packet->AddHeader (ctsHeader);
  SendPacket (packet, 1);
}
void
THzMacNano::SendData (Ptr<Packet> packet)
{
  NS_LOG_DEBUG ("---------------------------------------------------------------------------------------------------");
  THzMacHeader header;
  packet->RemoveHeader (header);
  NS_LOG_FUNCTION ( "Time: " << Simulator::Now () << " at node: " << m_address << " Energy: " << m_device->GetNode ()->GetObject<THzEnergyModel> ()->GetRemainingEnergy () << " to: " << header.GetDestination ());
  if (header.GetDestination () == GetBroadcast ()) // Broadcast
    {
      header.SetDuration (Seconds (0));
      packet->AddHeader (header);
      SendPacket (packet, 0);
      return;
    }

  if (header.GetDestination () != GetBroadcast ()) // Unicast
    {
      header.SetDuration (Seconds (0));
      packet->AddHeader (header);
      m_ackTimeout = GetDataDuration (packet) + GetCtrlDuration (THZ_PKT_TYPE_ACK) + PicoSeconds (666) + PicoSeconds (10);

      SendPacket (packet, 1); //
      AckTimeouts at;
      at.sequence = header.GetSequence ();
      NS_LOG_INFO ("scheduling ack timeout at:" << Simulator::Now () + m_ackTimeout << "seq" << at.sequence);
      at.m_ackTimeoutEvent = Simulator::Schedule (m_ackTimeout, &THzMacNano::AckTimeout, this, at.sequence);
      at.packet = packet;
      m_ackTimeouts.push_back (at);
      return;

    }
  NS_LOG_FUNCTION ("# dest" << header.GetDestination () << "seq" << m_sequence << "q-size" << m_pktQueue.size ());

}

void
THzMacNano::SendAck (Mac48Address dest,uint16_t sequence)
{
  NS_LOG_DEBUG ("---------------------------------------------------------------------------------------------------");
  NS_LOG_FUNCTION ("        Time: " << Simulator::Now () << " at node: " << m_address << " Energy: " << m_device->GetNode ()->GetObject<THzEnergyModel> ()->GetRemainingEnergy () << " to: " << dest);

  Ptr<Packet> packet = Create<Packet> ();
  THzMacHeader ackHeader = THzMacHeader (m_address, dest, THZ_PKT_TYPE_ACK);
  ackHeader.SetDuration (Seconds (0));
  ackHeader.SetSequence (sequence);
  packet->AddHeader (ackHeader);

  SendPacket (packet, 1);
}
bool
THzMacNano::SendPacket (Ptr<Packet> packet, bool rate)
{
  NS_LOG_DEBUG ("---------------------------------------------------------------------------------------------------");
  THzMacHeader header;
  packet->PeekHeader (header);
  NS_LOG_FUNCTION ("        Time: " << Simulator::Now () << " at node: " << m_address << " Energy: " << m_device->GetNode ()->GetObject<THzEnergyModel> ()->GetRemainingEnergy () << " to: " << header.GetDestination ());

  NS_LOG_INFO ("sequence" << header.GetSequence ());
  if (m_phy->SendPacket (packet, rate))
    {
      return true;
    }

  return false;
}
void
THzMacNano::SendPacketDone (Ptr<Packet> packet)
{
  NS_LOG_DEBUG ("---------------------------------------------------------------------------------------------------");
  std::list<PktTx>::iterator it = m_pktTx.begin ();
  for (; it != m_pktTx.end (); ++it)
    {
      if (it->packet == packet)
        {
          THzMacHeader header;
          packet->PeekHeader (header);
          NS_LOG_DEBUG ("data packet: " << header.GetSequence () << " dest: " << header.GetDestination () << " has been transmitted into channel");
        }
    }

  return;
}
void
THzMacNano::SendDataDone (bool success, Ptr<Packet> packet)
{
  NS_LOG_DEBUG ("---------------------------------------------------------------------------------------------------");
  THzMacHeader header;
  packet->PeekHeader (header);
  NS_LOG_FUNCTION ("        Time: " << Simulator::Now () << " at node: " << m_address << " Energy: " << m_device->GetNode ()->GetObject<THzEnergyModel> ()->GetRemainingEnergy () << " to: " << header.GetDestination ());
  std::list<PktTx>::iterator dit = m_pktTx.begin ();
  for (; dit != m_pktTx.end (); ++dit)
    {
      Ptr<Packet> data = dit->packet;
      THzMacHeader dataHeader;
      data->PeekHeader (dataHeader);
      NS_LOG_DEBUG ("data dest: " << dataHeader.GetDestination () << " acked pack dest: " << header.GetDestination ());
      if (dataHeader.GetSequence () > header.GetSequence () && dataHeader.GetDestination () == header.GetDestination ())
        {
          NS_LOG_DEBUG ("same destination");
          Simulator::Schedule (Seconds (0.0), &THzMacNano::CheckResources, this, data);
          break;
        }
    }

  m_tend = Simulator::Now ();
  std::list<PktTx>::iterator it = m_pktTx.begin ();
  for (; it != m_pktTx.end (); )
    {
      if (it->sequence == header.GetSequence ())
        {
          if (success)
            {
              NS_LOG_FUNCTION ("Success to transmit packet: " << it->sequence << "! at node: " << m_address);
              m_traceSendDataDone (m_device->GetNode ()->GetId (), m_device->GetIfIndex (), true);
              m_timeRec = (m_tend - it->tstart);
              m_throughput = it->packet->GetSize () * 8 / m_timeRec.GetSeconds ();
              m_throughputAll += m_throughput;
              m_ite += 1;
              m_throughputavg = m_throughputAll / (m_ite);
              m_traceThroughput (m_throughputavg);
              NS_LOG_DEBUG (it->packet->GetSize () << " bytes successfully transmitted during " << m_timeRec.GetSeconds () << " Seconds");
              NS_LOG_DEBUG ("  throughput : " << m_throughput);
              NS_LOG_DEBUG ("  overall throughput : " << m_throughputAll);
              NS_LOG_DEBUG ("  average throughput : " << m_throughputavg);
              NS_LOG_UNCOND (" discarded packets: " << m_discarded << " successful packets: " << m_ite << " throughput: " << m_throughput << " average throughput: " << m_throughputavg << " at node: " << m_address);
              /*----------------------------------------------------------------------------------------
               * enable the result printing in a .txt file by uncommenting the content below
               *----------------------------------------------------------------------------------------*/
              /*std::ofstream myfile;
              myfile.open ("nano_2way_sucessful.txt", std::ofstream::out | std::ios::app);
              myfile << m_device->GetNode ()->GetId () << "  " << m_timeRec.GetSeconds () << "   " << it->sequence << std::endl;
              myfile.close ();*/
              Dequeue (it->packet);
              it = m_pktTx.erase (it);
            }
          else
            {
              /*std::ofstream myfile;
              myfile.open ("nano_2way_discarded.txt", std::ofstream::out | std::ios::app);
              myfile << m_device->GetNode ()->GetId () << "  " << 1 << std::endl;
              myfile.close ();*/
              m_discarded += 1;
              NS_LOG_UNCOND (" discarded packets : " << m_discarded << "! at node: " << m_address);
              NS_LOG_FUNCTION ("Fail to transmit packet: " << it->sequence << "! at node: " << m_device->GetNode ()->GetId ());
              m_traceSendDataDone (m_device->GetNode ()->GetId (), m_device->GetIfIndex (), false);
              m_pktQueue.remove (it->packet);
              it = m_pktTx.erase (it);
            }
        }
      else
        {
          ++it;
        }
    }

}

// ---------------------- Receive Functions ----------------------------
void
THzMacNano::ReceiveRts (Ptr<Packet> packet)
{
  NS_LOG_DEBUG ("---------------------------------------------------------------------------------------------------");


  m_device->GetNode ()->GetObject<THzEnergyModel> ()->BookEnergy (0, packet->GetSize ());
  THzMacHeader header;
  packet->PeekHeader (header);
  NS_LOG_FUNCTION ("      Time: " << Simulator::Now () << " at node: " << m_address << " Energy: " << m_device->GetNode ()->GetObject<THzEnergyModel> ()->GetRemainingEnergy () << " from: " << header.GetSource ());

  if (header.GetDestination () != m_address)
    {
      NS_LOG_INFO ("RTS not for me");
      return;
    }

  //check if you have resources
  if (m_device->GetNode ()->GetObject<THzEnergyModel> ()->BookEnergy (2 * packet->GetSize (), m_FrameLength))
    {
      DataTimeouts dt;
      dt.sequence = header.GetSequence ();
      Time dataTimeout = GetCtrlDuration (THZ_PKT_TYPE_CTS)
        + PicoSeconds (666) + PicoSeconds (10);

      dt.m_dataTimeoutEvent = Simulator::Schedule (dataTimeout, &THzMacNano::DataTimeout, this, header.GetSequence ());
      m_dataTimeouts.push_back (dt);
      SendCts (header.GetSource (), header.GetSequence ());
      return;
    }
  else
    {
      NS_LOG_INFO ("Insufficient energy: " << m_device->GetNode ()->GetObject<THzEnergyModel> ()->GetRemainingEnergy ());
    }
}
void
THzMacNano::ReceiveCts (Ptr<Packet> packet)
{
  NS_LOG_DEBUG ("---------------------------------------------------------------------------------------------------");
  NS_LOG_FUNCTION ("at node: " << m_device->GetNode ()->GetId ());
  THzMacHeader header;
  packet->RemoveHeader (header);
  NS_LOG_FUNCTION ("      Time: " << Simulator::Now () << " at node: " << m_address << " Energy: " << m_device->GetNode ()->GetObject<THzEnergyModel> ()->GetRemainingEnergy () << " from: " << header.GetSource ());
  if (header.GetDestination () != m_address)
    {
      NS_LOG_INFO ("CTS not for me");
      return;
    }
  NS_LOG_INFO ("header seq:" << header.GetSequence ());
  std::list<CtsTimeouts>::iterator cit = m_ctsTimeouts.begin ();
  for (; cit != m_ctsTimeouts.end (); ++cit)
    {
      NS_LOG_INFO ("cit seq:" << cit->sequence);
      if (cit->sequence == header.GetSequence ())
        {
          cit->m_ctsTimeoutEvent.Cancel ();
          cit = m_ctsTimeouts.erase (cit);
          break;
        }
    }

  std::list<PktTx>::iterator it = m_pktTx.begin ();
  for (; it != m_pktTx.end (); ++it)
    {
      Ptr<Packet> data = it->packet;
      THzMacHeader dataHeader;
      data->PeekHeader (dataHeader);
      if (dataHeader.GetSequence () == header.GetSequence () && dataHeader.GetDestination () == header.GetSource ())
        {
          SendData (data);
          return;
        }
    }
  return;
}

void
THzMacNano::ReceiveData (Ptr<Packet> packet)
{
  NS_LOG_DEBUG ("---------------------------------------------------------------------------------------------------");
  NS_LOG_FUNCTION ("Time: " << Simulator::Now () << " at node: " << m_address << " Energy: " << m_device->GetNode ()->GetObject<THzEnergyModel> ()->GetRemainingEnergy ());
  THzMacHeader header;
  packet->RemoveHeader (header);

  if (header.GetDestination () == GetBroadcast ())
    {

      if (IsNewSequence (header.GetSource (), header.GetSequence ()))
        {
          m_forwardUpCb (packet, header.GetSource (), header.GetDestination ());
        }

      return;
    }

  if (header.GetDestination () !=  m_address) // destined not to me
    {
      NS_LOG_INFO ("This packet is not for me");
      return;
    }
  THzMacHeader ackHeader = THzMacHeader (m_address, m_address, THZ_PKT_TYPE_RTS);
  if  (m_rtsEnable == false) // for aloha
    {
      if (m_device->GetNode ()->GetObject<THzEnergyModel> ()->BookEnergy (ackHeader.GetSize (),packet->GetSize ()) != true )
        {
          NS_LOG_INFO ("Insufficient energy");
          return;
        }
    }
  NS_LOG_INFO ("header seq:" << header.GetSequence ());
  std::list<DataTimeouts>::iterator dit = m_dataTimeouts.begin ();
  for (; dit != m_dataTimeouts.end (); ++dit)
    {
      NS_LOG_INFO ("dit seq:" << dit->sequence);
      if (dit->sequence == header.GetSequence ())
        {
          dit->m_dataTimeoutEvent.Cancel ();
          dit = m_dataTimeouts.erase (dit);
          break;
        }
    }

  SendAck (header.GetSource (), header.GetSequence ());
  if (IsNewSequence (header.GetSource (), header.GetSequence ()))
    {
      m_forwardUpCb (packet, header.GetSource (), header.GetDestination ());
    }
  return;
}
void
THzMacNano::ReceiveAck (Ptr<Packet> packet)
{
  NS_LOG_DEBUG ("---------------------------------------------------------------------------------------------------");
  NS_LOG_FUNCTION ("Time: " << Simulator::Now () << " at node: " << m_address << " Energy: " << m_device->GetNode ()->GetObject<THzEnergyModel> ()->GetRemainingEnergy ());

  THzMacHeader header;
  packet->PeekHeader (header);
  NS_LOG_INFO ("sequence" << header.GetSequence () << " at node: " << m_device->GetNode ()->GetId ());

  if (header.GetDestination () == m_address)
    {
      std::list<AckTimeouts>::iterator it = m_ackTimeouts.begin ();
      for (; it != m_ackTimeouts.end (); ++it)
        {
          if (it->sequence == header.GetSequence ())
            {
              NS_LOG_INFO ("cancelling ack timeout");
              it->m_ackTimeoutEvent.Cancel ();
              SendDataDone (true, it->packet);
              it = m_ackTimeouts.erase (it);
              return;
            }
        }
    }
  else
    {
      NS_LOG_INFO ("ACK not for me");
    }
}


void
THzMacNano::ReceivePacket (Ptr<THzPhy> phy, Ptr<Packet> packet)
{
  NS_LOG_FUNCTION ("at node: " << m_device->GetNode ()->GetId ());

}
void
THzMacNano::ReceivePacketDone (Ptr<THzPhy> phy, Ptr<Packet> packet, bool success)
{

  THzMacHeader header;
  packet->PeekHeader (header);
  if (!success)
    {
      NS_LOG_DEBUG ("The packet is not encoded correctly. Drop it!");
      return;
    }

  switch (header.GetType ())
    {
    case THZ_PKT_TYPE_RTS:
      ReceiveRts (packet);
      break;
    case THZ_PKT_TYPE_CTS:
      ReceiveCts (packet);
      break;
    case THZ_PKT_TYPE_DATA:
      ReceiveData (packet);
      break;
    case THZ_PKT_TYPE_ACK:
      ReceiveAck (packet);
      break;
    default:
      break;
    }
}
// -------------------------- Timeout ----------------------------------
void
THzMacNano::CtsTimeout (Ptr<Packet> packet)
{
  NS_LOG_DEBUG ("---------------------------------------------------------------------------------------------------");
  THzMacHeader header;
  packet->PeekHeader (header);
  NS_LOG_FUNCTION ("!!! CTS timeout !!! at node: " << m_device->GetNode ()->GetId () << " for packet: " << header.GetSequence ());
  m_traceCtsTimeout (m_device->GetNode ()->GetId (), m_device->GetIfIndex ());

  THzMacHeader rtsHeader = THzMacHeader (m_address, m_address, THZ_PKT_TYPE_RTS);
  uint32_t controlPacketLength = rtsHeader.GetSize ();
  m_device->GetNode ()->GetObject<THzEnergyModel> ()->ReturnEnergy (m_FrameLength, 2 * controlPacketLength);

  std::list<CtsTimeouts>::iterator cit = m_ctsTimeouts.begin ();
  for (; cit != m_ctsTimeouts.end (); )
    {
      NS_LOG_INFO ("cit seq:" << cit->sequence);
      if (cit->sequence == header.GetSequence ())
        {
          cit = m_ctsTimeouts.erase (cit);
          break;
        }
      else
        {
          ++cit;
        }
    }

  std::list<PktTx>::iterator it = m_pktTx.begin ();
  for (; it != m_pktTx.end (); ++it)
    {
      if (it->sequence == header.GetSequence ())
        {
          NS_LOG_DEBUG ("retry: " << it->retry << " for packet:" << it->sequence);
          it->backoff = true;
          it->retry = it->retry + 1;
          NS_LOG_DEBUG ("retry: " << it->retry);
          if (it->retry >= m_dataRetryLimit)
            {
              SendDataDone (false, packet);
              return;
            }
          else
            {
              Backoff (it->packet, it->retry);
            }
          return;
        }
    }

}
void
THzMacNano::AckTimeout (uint16_t sequence)
{
  NS_LOG_DEBUG ("---------------------------------------------------------------------------------------------------");
  NS_LOG_FUNCTION ("now" << Simulator::Now ());
  NS_LOG_DEBUG ("!!! ACK timeout !!! for packet: " << sequence << " at node: " << m_device->GetNode ()->GetId ());
  NS_LOG_INFO ("Remaining energy: " << m_device->GetNode ()->GetObject<THzEnergyModel> ()->GetRemainingEnergy ());
  m_traceAckTimeout (m_device->GetNode ()->GetId (), m_device->GetIfIndex ());

  THzMacHeader rtsHeader = THzMacHeader (m_address, m_address, THZ_PKT_TYPE_RTS);
  uint32_t controlPacketLength = rtsHeader.GetSize ();
  m_device->GetNode ()->GetObject<THzEnergyModel> ()->ReturnEnergy (0,controlPacketLength);

  std::list<AckTimeouts>::iterator ait = m_ackTimeouts.begin ();
  for (; ait != m_ackTimeouts.end (); )
    {
      if (ait->sequence == sequence)
        {
          ait = m_ackTimeouts.erase (ait);
          break;
        }
      else
        {
          ++ait;
        }
    }

  std::list<PktTx>::iterator it = m_pktTx.begin ();
  for (; it != m_pktTx.end (); ++it)
    {
      if (it->sequence == sequence)
        {
          NS_LOG_DEBUG ("retry: " << it->retry << " for packet:" << it->sequence);
          it->retry = it->retry + 1;
          NS_LOG_DEBUG ("retry: " << it->retry);
          if (it->retry >= m_dataRetryLimit)
            {
              SendDataDone (false, it->packet);
              return;
            }
          else
            {
              Backoff (it->packet, it->retry);
            }
          return;
        }
    }
}
void
THzMacNano::DataTimeout (uint16_t sequence)
{
  NS_LOG_DEBUG ("---------------------------------------------------------------------------------------------------");
  NS_LOG_FUNCTION ("now" << Simulator::Now ());
  NS_LOG_DEBUG ("!!! Data timeout !!! for packet: " << sequence << " at node: " << m_device->GetNode ()->GetId ());

  THzMacHeader rtsHeader = THzMacHeader (m_address, m_address, THZ_PKT_TYPE_RTS);
  uint32_t controlPacketLength = rtsHeader.GetSize ();
  m_device->GetNode ()->GetObject<THzEnergyModel> ()->ReturnEnergy (controlPacketLength, m_FrameLength);

  std::list<DataTimeouts>::iterator dit = m_dataTimeouts.begin ();
  for (; dit != m_dataTimeouts.end (); ++dit)
    {
      if (dit->sequence == sequence)
        {
          dit = m_dataTimeouts.erase (dit);
          return;
        }
    }
}
// --------------------------- ETC -------------------------------------
bool
THzMacNano::IsNewSequence (Mac48Address addr, uint16_t seq)
{
  std::list<std::pair<Mac48Address, uint16_t> >::iterator it = m_seqList.begin ();
  for (; it != m_seqList.end (); ++it)
    {
      if (it->first == addr)
        {
          if (it->second == 65536 && seq < it->second)
            {
              it->second = seq;
              return true;
            }
          else if (seq > it->second)
            {
              it->second = seq;
              return true;
            }
          else
            {
              return false;
            }
        }
    }
  std::pair<Mac48Address, uint16_t> newEntry;
  newEntry.first = addr;
  newEntry.second = seq;
  m_seqList.push_back (newEntry);
  return true;
}


} // namespace ns3
