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
 * Author: Qing Xia <qingxia@buffalo.edu>
 *         Zahed Hossain <zahedhos@buffalo.edu>
 *         Josep Miquel Jornet <jmjornet@buffalo.edu>
 */

#include "ns3/attribute.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/log.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/node.h"
#include "thz-mac-header.h"
#include "thz-mac-macro.h"
#include "thz-phy-macro.h"
#include "thz-dir-antenna.h"
#include "thz-net-device.h"
#include <vector>
#include <iostream>
#include <iterator>
#include <string>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <stdio.h>
#include <time.h>
#include <sstream>

NS_LOG_COMPONENT_DEFINE ("THzMacMacro");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (THzMacMacro);

THzMacMacro::THzMacMacro ()
  : THzMac (),
  m_phy (0),
  m_state (IDLE),
  m_ccaTimeoutEvent (),
  m_backoffTimeoutEvent (),
  m_ctsTimeoutEvent (),
  m_ackTimeoutEvent (),
  m_sendCtsEvent (),
  m_sendAckEvent (),
  m_sendDataEvent (),
  m_retry (0),
  m_pktData (0)

{
  m_cw = m_cwMin;
  m_nav = Simulator::Now ();
  m_localNav = Simulator::Now ();
  m_backoffRemain = NanoSeconds (0);
  m_backoffStart = NanoSeconds (0);
  m_ite = 0;
  m_retry = 0;
  m_throughputAll = 0;
  m_state = IDLE;
  m_discard = 0;
  m_send = 0;
  m_thzAD = 0;
  m_rxIniAngle = 0;
  m_MinEnquePacketSize = 15000;
  m_tData = NanoSeconds (810.76);
  m_result.clear ();
  Simulator::Schedule (NanoSeconds (0.0001),&THzMacMacro::SetRxAntennaParameters, this); // initialization: turn antenna mode as receiver mode at all devices

}
THzMacMacro::~THzMacMacro ()
{
  Clear ();
}
void
THzMacMacro::Clear ()
{
  m_pktTx = 0;
  m_pktData = 0;
  m_pktQueue.clear ();
  m_seqList.clear ();
  m_pktRec = 0;
  m_throughput = 0;
  m_throughputAll = 0;
}

TypeId
THzMacMacro::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::THzMacMacro")
    .SetParent<THzMac> ()
    .AddConstructor<THzMacMacro> ()
    .AddAttribute ("EnableRts",
                   "If true, RTS is enabled",
                   BooleanValue (false),
                   MakeBooleanAccessor (&THzMacMacro::m_rtsEnable),
                   MakeBooleanChecker ())
    .AddAttribute ("CwMin",
                   "Minimum value of CW",
                   UintegerValue (0),
                   MakeUintegerAccessor (&THzMacMacro::m_cwMin),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("CwMax",
                   "Maximum value of CW",
                   UintegerValue (2),
                   MakeUintegerAccessor (&THzMacMacro::m_cwMax),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("SlotTime",
                   "Time slot duration for MAC backoff",
                   TimeValue (NanoSeconds (5)),
                   MakeTimeAccessor (&THzMacMacro::m_slotTime),
                   MakeTimeChecker ())
    .AddAttribute ("SifsTime",
                   "Short Inter-frame Space",
                   TimeValue (NanoSeconds (0.0)),
                   MakeTimeAccessor (&THzMacMacro::m_sifs),
                   MakeTimeChecker ())
    .AddAttribute ("DifsTime",
                   "DFS Inter-frame Space",
                   TimeValue (NanoSeconds (0.0)),
                   MakeTimeAccessor (&THzMacMacro::m_difs),
                   MakeTimeChecker ())
    .AddAttribute ("QueueLimit",
                   "Maximum packets to queue at MAC",
                   UintegerValue (10000),
                   MakeUintegerAccessor (&THzMacMacro::m_queueLimit),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("RtsRetryLimit",
                   "Maximum Limit for RTS Retransmission",
                   UintegerValue (7),
                   MakeUintegerAccessor (&THzMacMacro::m_rtsRetryLimit),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("DataRetryLimit",
                   "Maximum Limit for Data Retransmission",
                   UintegerValue (5),
                   MakeUintegerAccessor (&THzMacMacro::m_dataRetryLimit),
                   MakeUintegerChecker<uint16_t> ())
    .AddTraceSource ("CtsTimeout",
                     "Trace Hookup for CTS Timeout",
                     MakeTraceSourceAccessor (&THzMacMacro::m_traceCtsTimeout),
                     "ns3::THzMac::TimeTracedCallback")
    .AddTraceSource ("AckTimeout",
                     "Trace Hookup for ACK Timeout",
                     MakeTraceSourceAccessor (&THzMacMacro::m_traceAckTimeout),
                     "ns3::THzMac::TimeTracedCallback")
    .AddTraceSource ("SendDataDone",
                     "Trace Hookup for sending a data",
                     MakeTraceSourceAccessor (&THzMacMacro::m_traceSendDataDone),
                     "ns3::THzMac::SendDataDoneTracedCallback")
    .AddTraceSource ("Enqueue",
                     "Trace Hookup for enqueue a data",
                     MakeTraceSourceAccessor (&THzMacMacro::m_traceEnqueue),
                     "ns3::THzMac::TimeTracedCallback")
    .AddTraceSource ("Throughput",
                     "Trace Hookup for Throughput",
                     MakeTraceSourceAccessor (&THzMacMacro::m_traceThroughput),
                     "ns3::THzMac::ThroughputTracedCallback")
  ;
  return tid;
}

// ------------------------ Set Functions -----------------------------
void
THzMacMacro::AttachPhy (Ptr<THzPhy> phy)
{
  m_phy = phy;
}
void
THzMacMacro::SetDevice (Ptr<THzNetDevice> dev)
{
  m_device = dev;
  SetCw (m_cwMin);
}

void
THzMacMacro::SetAddress (Mac48Address addr)
{
  NS_LOG_FUNCTION (addr);
  m_address = addr;
  // to help each node have different random seed
  uint8_t tmp[6];
  m_address.CopyTo (tmp);
}
void
THzMacMacro::SetForwardUpCb (Callback<void, Ptr<Packet>, Mac48Address, Mac48Address> cb)
{
  m_forwardUpCb = cb;
}
void
THzMacMacro::SetCwMin (uint32_t cw)
{
  m_cwMin = cw;
}
void
THzMacMacro::SetCw (uint32_t cw)
{
  m_cw = cw;
}
void
THzMacMacro::SetSlotTime (Time duration)
{
  m_slotTime = duration;
}
// ------------------------ Get Functions -----------------------------
uint32_t
THzMacMacro::GetCw (void)
{
  return m_cw;
}
Time
THzMacMacro::GetSlotTime (void)
{
  return m_slotTime;
}
Time
THzMacMacro::GetSifs (void) const
{
  return m_sifs;
}
Time
THzMacMacro::GetDifs (void) const
{
  return m_difs;
}

Mac48Address
THzMacMacro::GetAddress () const
{
  return this->m_address;
}

Mac48Address
THzMacMacro::GetBroadcast (void) const
{
  return Mac48Address::GetBroadcast ();
}
Time
THzMacMacro::GetCtrlDuration (uint16_t type)
{
  THzMacHeader header = THzMacHeader (m_address, m_address, type);
  return m_phy->CalTxDuration (header.GetSize (), 0, 0);
}
Time
THzMacMacro::GetDataDuration (Ptr<Packet> p)
{
  return m_phy->CalTxDuration (0, p->GetSize (), 0);
}

std::string
THzMacMacro::StateToString (State state)
{
  switch (state)
    {
    case IDLE:
      return "IDLE";
    case BACKOFF:
      return "BACKOFF";
    case WAIT_TX:
      return "WAIT_TX";
    case TX:
      return "TX";
    case WAIT_RX:
      return "WAIT_RX";
    case RX:
      return "RX";
    case COLL:
      return "COLL";
    default:
      return "??";
    }
}

// ------------------ Channel Access Functions -------------------------
// Checks if NAV (Network Allocation Vector) time has arrived. If not, waits. If yes, schedules BOStart
void
THzMacMacro::CcaForDifs ()
{
  NS_LOG_FUNCTION ("at node: " << m_device->GetNode ()->GetId () << " queue-size " << m_pktQueue.size ()  << " nav " << m_nav << " local nav " << m_localNav << StateToString (m_state) << " is phy idel " << m_phy->GetObject<THzPhyMacro> ()->IsIdle ());
  Time now = Simulator::Now ();

  if (m_pktQueue.size () == 0 || m_ccaTimeoutEvent.IsRunning ())
    {
      return;
    }
  Time nav = std::max (m_nav, m_localNav);
  if (nav > now + GetSlotTime ()) // Slot time = 5ns. Time slot duration for MAC backoff
    {
      m_ccaTimeoutEvent = Simulator::Schedule (nav - now, &THzMacMacro::CcaForDifs, this);
      return;
    }
  if (m_state != IDLE || !m_phy->GetObject<THzPhyMacro> ()->IsIdle ())
    {
      m_ccaTimeoutEvent = Simulator::ScheduleNow (&THzMacMacro::BackoffStart, this);
      return;
    }

  m_ccaTimeoutEvent = Simulator::Schedule (GetDifs (), &THzMacMacro::BackoffStart, this); // DIFS és 0 ns (m_difs), per tnat és el mateix que now
}
void
THzMacMacro::BackoffStart ()
{
  NS_LOG_FUNCTION (" start at " << Simulator::Now () << " BO remain" << m_backoffRemain << StateToString (m_state) << m_phy->GetObject<THzPhyMacro> ()->IsIdle ());
  m_backoffStart = Simulator::Now ();
  if (m_backoffRemain == Seconds (0))
    {
      Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
      uint32_t bo = uv->GetInteger (1, pow (double(2.0), double(m_retry)));
      m_backoffRemain = NanoSeconds ((double)(bo) * m_tData.GetNanoSeconds ());
    }
  if (m_state != IDLE || !m_phy->GetObject<THzPhyMacro> ()->IsIdle ())
    {
      m_ccaTimeoutEvent = Simulator::Schedule (m_backoffRemain, &THzMacMacro::CcaForDifs, this);
      return;
    }
  m_backoffTimeoutEvent = Simulator::Schedule (m_backoffRemain, &THzMacMacro::ChannelAccessGranted, this);
}
void
THzMacMacro::ChannelBecomesBusy ()
{
  NS_LOG_FUNCTION ("");
  if (m_backoffTimeoutEvent.IsRunning ())
    {
      m_backoffTimeoutEvent.Cancel ();
      Time elapse;
      if (Simulator::Now () > m_backoffStart)
        {
          elapse = Simulator::Now () - m_backoffStart;
        }
      if (elapse < m_backoffRemain)
        {
          m_backoffRemain = m_backoffRemain - elapse;
          m_backoffRemain = RoundOffTime (m_backoffRemain);
        }
      NS_LOG_DEBUG ("Freeze backoff! Remain " << m_backoffRemain);
    }
  CcaForDifs ();
}
void
THzMacMacro::ChannelAccessGranted ()
{
  NS_LOG_FUNCTION ("");
  if (m_pktQueue.size () == 0)
    {
      return;
    }

  m_backoffStart = Seconds (0);
  m_backoffRemain = Seconds (0);
  m_pktData = m_pktQueue.front ();
  if (m_pktData == 0)
    {
      NS_LOG_DEBUG ("Queue has null packet");
      return;
    }
  THzMacHeader header;
  m_pktData->PeekHeader (header);
  if (header.GetDestination () != GetBroadcast () && m_rtsEnable == true)
    {
      m_state = WAIT_TX;
      SendRts (m_pktData);
    }
  else
    {
      m_state = WAIT_TX;
      SendData (m_pktData);
    }
}

// Es crida periòdicament per anar girant l'antena al ritme indicat
void
THzMacMacro::SetRxAntennaParameters ()
{
  NS_LOG_DEBUG ("NODE: " << m_device->GetNode ()->GetId () << "   Now TX->RX  " << Simulator::Now ());
  m_thzAD = m_device->GetDirAntenna ();
  m_thzAD->SetAttribute ("TuneRxTxMode", DoubleValue (1));     // set as receiver
  m_thzAD->SetAttribute ("InitialAngle", DoubleValue (0));
  double beamwidthDegrees = m_thzAD->GetBeamwidth ();  //get default beamwidth
  m_thzAD->SetBeamwidth (beamwidthDegrees);           //set beamwidth to calculate antenna exponent for thz-dir-antenna module
  NS_LOG_DEBUG ("Tune as RxMode At node: " << m_device->GetNode ()->GetId () << " Antenna Mode: " << m_thzAD->CheckAntennaMode () << " Antenna Beamwidth: " << beamwidthDegrees << " deg, TurningSpeed: " << m_thzAD->GetRxTurningSpeed () << " MaxGain: " << m_thzAD->GetMaxGain () << "dB");

  m_thzAD->TuneRxOrientation (m_rxIniAngle);
  m_rxIniAngle = m_rxIniAngle + beamwidthDegrees;
  while (m_rxIniAngle <= -360)
    {
      m_rxIniAngle += 360;
    }
  while (m_rxIniAngle > 360)
    {
      m_rxIniAngle -= 360;
    }

  Time tCircle = Seconds (1 / m_thzAD->GetRxTurningSpeed ());
  int nSector = 360 / beamwidthDegrees;
  Time tSector = NanoSeconds (tCircle.GetNanoSeconds () / nSector);
  NS_LOG_DEBUG ("tSector = " << tSector << ", nSector = " << nSector << ", tCircle = " << tCircle );
  m_SetRxAntennaEvent = Simulator::Schedule (tSector, &THzMacMacro::SetRxAntennaParameters, this);
}

// ----------------------- Queue Functions -----------------------------
bool
THzMacMacro::Enqueue (Ptr<Packet> packet, Mac48Address dest)
{
  m_pktRec = packet->GetSize ();
  if (m_pktRec < m_MinEnquePacketSize)
    {
      m_pktQueue.push_front (packet);
      m_pktQueue.pop_front ();
    }
  else
    {
      THzMacHeader header = THzMacHeader (m_address, dest, THZ_PKT_TYPE_DATA);
      m_sequence++;
      header.SetSequence (m_sequence);
      packet->AddHeader (header);
      m_pktQueue.push_back (packet);
      m_SetRxAntennaEvent.Cancel ();    // WHY ??
      m_thzAD = m_device->GetDirAntenna ();
      m_thzAD->SetAttribute ("TuneRxTxMode", DoubleValue (0)); // set as transmitter
      m_thzAD->SetAttribute ("InitialAngle", DoubleValue (0.0));
      double beamwidthDegrees = m_thzAD->GetBeamwidth ();  //get default beamwidth
      m_thzAD->SetBeamwidth (beamwidthDegrees);           //set beamwidth to calculate antenna exponent for thz-dir-antenna module
      NS_LOG_DEBUG ("Tune as TxMode At node: " << m_device->GetNode ()->GetId () << " Antenna Mode: " << m_thzAD->CheckAntennaMode () << " Antenna Beamwidth: " << beamwidthDegrees << " deg, MaxGain: " << m_thzAD->GetMaxGain () << "dB");

      Rec rec;
      rec.RecSize = packet->GetSize ();
      rec.RecTime = Simulator::Now ();
      rec.RecSeq = m_sequence;
      rec.RecRetry = 0;
      rec.Recpacket = packet;
      m_rec.push_back (rec);
      m_pktData = packet;
      Simulator::Schedule (NanoSeconds (0.001), &THzMacMacro::CcaForDifs, this);
    }
  return false;
}
void
THzMacMacro::Dequeue ()
{
  NS_LOG_FUNCTION (m_pktQueue.size ());
  m_pktQueue.remove (m_pktData);
}

// ---------- Network allocation vector (NAV) functions ----------------
void
THzMacMacro::UpdateNav (Time nav)
{
  Time newNav;
  newNav = RoundOffTime (Simulator::Now () + nav);

  if (newNav > m_nav)
    {
      m_nav = newNav;
    }
  NS_LOG_INFO ("NAV: " << m_nav);
}
void
THzMacMacro::UpdateLocalNav (Time nav)
{
  m_localNav = RoundOffTime (Simulator::Now () + nav);
}

// ----------------------- Send Functions ------------------------------
void
THzMacMacro::SendRts (Ptr<Packet> pktData)
{
  NS_LOG_FUNCTION (" from node " << m_device->GetNode ()->GetId ());
  THzMacHeader dataHeader;
  pktData->PeekHeader (dataHeader);
  NS_LOG_DEBUG ("Send RTS from " << m_address << " to " << dataHeader.GetDestination ());
  Ptr<Packet> packet = Create<Packet> (0);
  THzMacHeader rtsHeader = THzMacHeader (m_address, dataHeader.GetDestination (), THZ_PKT_TYPE_RTS);
  Time nav = GetSifs () + GetCtrlDuration (THZ_PKT_TYPE_CTS) + NanoSeconds (33.3) // Sifs = 0ns, CtrlDuration és el temps que es triga físicament a transmetre els X bits
    + GetSifs () + GetDataDuration (pktData) + NanoSeconds (33.3)
    + GetSifs () + GetCtrlDuration (THZ_PKT_TYPE_ACK) + NanoSeconds (33.3)
    + GetSlotTime () + NanoSeconds (33.3);
  rtsHeader.SetDuration (nav);
  rtsHeader.SetSequence (dataHeader.GetSequence ());
  packet->AddHeader (rtsHeader);
  Time ctsTimeout = GetCtrlDuration (THZ_PKT_TYPE_RTS) + NanoSeconds (33.3)
    + GetSifs () + GetCtrlDuration (THZ_PKT_TYPE_CTS) + NanoSeconds (33.3)
    + GetSlotTime ();
  if (SendPacket (packet, 0))
    {
      UpdateLocalNav (ctsTimeout);
      CtsTimeouts ct;
      ct.sequence = rtsHeader.GetSequence ();
      ct.m_ctsTimeoutEvent = Simulator::Schedule (ctsTimeout, &THzMacMacro::CtsTimeout, this, rtsHeader.GetSequence ());
      m_ctsTimeouts.push_back (ct);
    }
  else
    {
      StartOver ();
    }
}

void
THzMacMacro::ReceiveRts (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION ("at node " << m_device->GetNode ()->GetId ());
  THzMacHeader header;
  packet->RemoveHeader (header);
  if (header.GetDestination () != m_address)
    {
      UpdateNav (header.GetDuration ());
      m_state = IDLE;
      CcaForDifs ();
      return;
    }
  // if NAV indicates the channel is not busy, do not respond to RTS (802.11 std)
  if (std::max (m_nav, m_localNav) > Simulator::Now ())
    {
      CcaForDifs ();
      return;
    }
  UpdateLocalNav (header.GetDuration ());
  m_state = WAIT_TX;
  m_sendCtsEvent = Simulator::Schedule (NanoSeconds (0.001), &THzMacMacro::SendCts, this, header.GetSource (), header.GetDuration (), header.GetSequence ());
}

void
THzMacMacro::SendCts (Mac48Address dest, Time duration, uint16_t sequence)
{
  NS_LOG_FUNCTION (" from node " << m_device->GetNode ()->GetId () << " to " << dest);
  Ptr<Packet> packet = Create<Packet> (0);
  THzMacHeader ctsHeader = THzMacHeader (m_address, dest, THZ_PKT_TYPE_CTS);
  Time nav = duration - GetSifs () - GetCtrlDuration (THZ_PKT_TYPE_CTS) - NanoSeconds (33.3);
  ctsHeader.SetDuration (nav);
  ctsHeader.SetSequence (sequence);
  packet->AddHeader (ctsHeader);
  if (SendPacket (packet, 0))
    {
      UpdateLocalNav (duration - GetSifs ());
    }
}

void
THzMacMacro::ReceiveCts (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION ("at node " << m_device->GetNode ()->GetId ());
  THzMacHeader header;
  packet->RemoveHeader (header);
  if (header.GetDestination () != m_address)
    {
      UpdateNav (header.GetDuration ());
      m_state = IDLE;
      CcaForDifs ();
      return;
    }
  std::list<Rec>::iterator it = m_rec.begin ();
  for (; it != m_rec.end (); ++it)
    {
      m_pktData = it->Recpacket;
      THzMacHeader dataHeader;
      m_pktData->PeekHeader (dataHeader);
      if (dataHeader.GetSequence () == header.GetSequence ())
        {
          break;
        }
    }
  UpdateLocalNav (header.GetDuration ());
  std::list<CtsTimeouts>::iterator itt = m_ctsTimeouts.begin ();
  for (; itt != m_ctsTimeouts.end (); ++itt)
    {
      if (itt->sequence == header.GetSequence ())
        {
          itt->m_ctsTimeoutEvent.Cancel ();
          m_state = WAIT_TX;
          m_sendDataEvent = Simulator::Schedule (NanoSeconds (0.001), &THzMacMacro::SendData, this, m_pktData);
          itt = m_ctsTimeouts.erase (itt);
          return;
        }
    }
}

void
THzMacMacro::SendData (Ptr<Packet> packet)
{
  if (m_pktQueue.size () == 0)
    {
      NS_LOG_INFO ("senddata check queue empty");
      m_state = IDLE;
      return;
    }
  else
    {
      NS_LOG_INFO ("senddata check queue nonempty");
      m_pktData = packet;
      NS_LOG_FUNCTION ("at node: " << m_device->GetNode ()->GetId () << " now: " << Simulator::Now () << " QueueSize " << m_pktQueue.size ());
      THzMacHeader header;
      m_pktData->PeekHeader (header);
      if (header.GetDestination () == GetBroadcast ())  // Broadcast
        {
          NS_LOG_INFO ("broadcast");
          header.SetDuration (Seconds (0));
          if (SendPacket (m_pktData, 0))
            {
              UpdateLocalNav (GetDataDuration (m_pktData) + GetSlotTime () + NanoSeconds (33.3));
            }
          else
            {
              StartOver ();
            }
        }
      if (header.GetDestination () != GetBroadcast ())  // Unicast
        {
          NS_LOG_INFO ("unicast");
          Time nav = GetSifs () + GetCtrlDuration (THZ_PKT_TYPE_ACK) + NanoSeconds (33.3);
          header.SetDuration (nav);
          if (SendPacket (m_pktData, 1))
            {
              Time ackTimeout = GetDataDuration (m_pktData) + NanoSeconds (33.3) + GetSifs () + GetCtrlDuration (THZ_PKT_TYPE_ACK) + NanoSeconds (33.3) + GetSlotTime ();
              UpdateLocalNav (ackTimeout);
              AckTimeouts at;
              at.sequence = header.GetSequence ();
              at.m_ackTimeoutEvent = Simulator::Schedule (ackTimeout, &THzMacMacro::AckTimeout, this, header.GetSequence ());
              m_ackTimeouts.push_back (at);
              NS_LOG_INFO (" scheduling ack timeout at: " << Simulator::Now () + ackTimeout);
            }
          else
            {
              StartOver ();
            }
        }
    }
}

void
THzMacMacro::StartOver ()
{
  NS_LOG_FUNCTION ("");
  m_backoffStart = Seconds (0);
  m_backoffRemain = Seconds (0);
  CcaForDifs ();
}

void
THzMacMacro::ReceiveData (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION ("at node " << m_device->GetNode ()->GetId ());
  THzMacHeader header;
  packet->RemoveHeader (header);
  header.GetDuration ();
  if (header.GetDestination () == GetBroadcast ())
    {
      m_state = IDLE;

      if (IsNewSequence (header.GetSource (), header.GetSequence ()))
        {
          m_forwardUpCb (packet, header.GetSource (), header.GetDestination ());
        }
      CcaForDifs ();
      return;
    }
  if (header.GetDestination () !=  m_address) // destined not to me
    {
      UpdateNav (header.GetDuration ());
      m_state = IDLE;
      CcaForDifs ();
      return;
    }
  UpdateLocalNav (header.GetDuration ());
  m_state = WAIT_TX;
  m_sendAckEvent = Simulator::Schedule (GetSifs (), &THzMacMacro::SendAck, this, header.GetSource (), header.GetSequence ());

  if (IsNewSequence (header.GetSource (), header.GetSequence ()))
    {
      m_forwardUpCb (packet, header.GetSource (), header.GetDestination ());
    }
}

void
THzMacMacro::SendAck (Mac48Address dest, uint16_t sequence)
{
  NS_LOG_FUNCTION ("from node " << m_device->GetNode ()->GetId () << " to " << dest);
  Ptr<Packet> packet = Create<Packet> (0);
  THzMacHeader ackHeader = THzMacHeader (m_address, dest, THZ_PKT_TYPE_ACK);
  ackHeader.SetSequence (sequence);
  packet->AddHeader (ackHeader);
  Time nav = GetCtrlDuration (THZ_PKT_TYPE_ACK) + NanoSeconds (33.33);
  ackHeader.SetDuration (Seconds (0));
  UpdateLocalNav (nav + GetSlotTime ());
  SendPacket (packet, 0);
}

void
THzMacMacro::ReceiveAck (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION ("at node " << m_device->GetNode ()->GetId ());
  THzMacHeader header;
  packet->RemoveHeader (header);
  m_state = IDLE;
  if (header.GetDestination () == m_address)
    {
      std::list<AckTimeouts>::iterator it = m_ackTimeouts.begin ();
      for (; it != m_ackTimeouts.end (); ++it)
        {
          if (it->sequence == header.GetSequence ())
            {
              it->m_ackTimeoutEvent.Cancel ();
              Simulator::Schedule (NanoSeconds (0.001), &THzMacMacro::SendDataDone, this, true, header.GetSequence ());
              it = m_ackTimeouts.erase (it);
              return;
            }
        }
    }
  CcaForDifs ();
}


bool
THzMacMacro::SendPacket (Ptr<Packet> packet, bool rate)
{
  NS_LOG_FUNCTION (" state " << m_state << " now " << Simulator::Now ());

  if (m_state == IDLE || m_state == WAIT_TX)
    {
      if (m_phy->SendPacket (packet, rate, 0))
        {
          m_state = TX;
          m_pktTx = packet;
          return true;
        }
      else
        {
          m_state = IDLE;
        }
    }
  return false;
}
void
THzMacMacro::SendPacketDone (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION ("at node " << m_device->GetNode ()->GetId () << " state " << StateToString (m_state));
  if (m_state != TX || m_pktTx != packet)
    {
      NS_LOG_DEBUG ("Something is wrong!");
      return;
    }
  m_state = IDLE;
  THzMacHeader header;
  packet->PeekHeader (header);
  switch (header.GetType ())
    {
    case THZ_PKT_TYPE_RTS:
    case THZ_PKT_TYPE_CTS:
      break;
    case THZ_PKT_TYPE_DATA:
      if (header.GetDestination () == GetBroadcast ())
        {
          SendDataDone (true, header.GetSequence ());
          CcaForDifs ();
          return;
        }
      break;
    case THZ_PKT_TYPE_ACK:
      CcaForDifs ();
      break;
    default:
      CcaForDifs ();
      break;
    }
}
void
THzMacMacro::SendDataDone (bool success, uint16_t sequence)
{
  NS_LOG_FUNCTION ("at node " << m_device->GetNode ()->GetId ());
  std::list<Rec>::iterator it = m_rec.begin ();
  for (; it != m_rec.end (); it++)
    {
      if (it->RecSeq == sequence)
        {
          Result result;
          result.nodeid = m_device->GetNode ()->GetId ();
          Simulator::ScheduleNow (&THzMacMacro::ResultsRecord,this);
          m_result.clear ();
          if (success)
            {
              NS_LOG_FUNCTION ("Success to transmit packet at node: " << m_device->GetNode ()->GetId ());
              if (m_pktQueue.size () == 0)
                {
                  NS_LOG_DEBUG ("node: " << m_device->GetNode ()->GetId () << " senddatadone check queue empty");
                  m_state = IDLE;
                  return;
                }
              m_pktQueue.remove (it->Recpacket);
              m_send++;
              NS_LOG_UNCOND ("Successfully Sent Packet number " << m_send << " from node " << m_device->GetNode ()->GetId () << " Discard " << m_discard << " Total send " << (m_send + m_discard) << " #queue " << m_pktQueue.size ());
              m_backoffStart = Seconds (0);
              m_backoffRemain = Seconds (0);
              SetCw (m_cwMin);
              m_state = IDLE;
              m_tend = Simulator::Now ();
              NS_LOG_DEBUG (" end at " << m_tend);
              m_tstart = it->RecTime;
              m_timeRec = (m_tend - m_tstart);
              result.Psize = (it->RecSize - 53); //byte
              result.delay = m_timeRec;
              result.success = true;
              result.discard = false;
              m_result.push_front (result);
              m_throughput = (it->RecSize - 53) * 8 / m_timeRec.GetSeconds ();
              m_throughputAll += m_throughput;
              m_ite += 1;
              m_throughputavg = m_throughputAll / (m_ite);
              m_traceThroughput (m_throughputavg);
              NS_LOG_UNCOND ("  throughput : " << m_throughput << " of node " << m_device->GetNode ()->GetId ());
              NS_LOG_DEBUG ("  overall throughput : " << m_throughputAll);
              NS_LOG_DEBUG ("  m_ite: " << m_ite );
              NS_LOG_UNCOND ("  average throughput : " << m_throughputavg << " of node " << m_device->GetNode ()->GetId ());
            }
          else
            {
              NS_LOG_FUNCTION ("Fail to transmit packet at node: " << m_device->GetNode ()->GetId ());
              m_discard++;
              result.Psize = (it->RecSize - 53); //byte
              result.delay = Seconds (0);
              result.success = false;
              result.discard = true;
              m_result.push_front (result);
              NS_LOG_UNCOND ("*** Discard Packet number " << m_discard << " from node " << m_device->GetNode ()->GetId () << " Total send " << (m_send + m_discard) << " #queue " << m_pktQueue.size ());
              m_backoffStart = Seconds (0);
              m_backoffRemain = Seconds (0);
              // According to IEEE 802.11-2007 std (p261)., CW should be reset to minimum value
              // when retransmission reaches limit or when DATA is transmitted successfully
              SetCw (m_cwMin);
              m_state = IDLE;
            }
          NS_LOG_DEBUG ("NODE: " << m_device->GetNode ()->GetId () << " SEND DATA DONE: m_sequence = " << sequence);
          it = m_rec.erase (it);
        }
    }
}


// ---------------------- Receive Functions ----------------------------

void
THzMacMacro::ReceivePacket (Ptr<THzPhy> phy, Ptr<Packet> packet)
{
  THzMacHeader header;
  packet->PeekHeader (header);
  NS_LOG_FUNCTION ("at node " << m_device->GetNode ()->GetId () << " from " << header.GetSource () << " now " << Simulator::Now () << " state: " << StateToString (m_state));
  ChannelBecomesBusy ();
  switch (m_state)
    {
    case WAIT_TX:
    case RX:
    case WAIT_RX:
    case BACKOFF:
    case IDLE:
      m_state = RX;
      break;
    case TX:
    case COLL:
      break;
    }
}
void
THzMacMacro::ReceivePacketDone (Ptr<THzPhy> phy, Ptr<Packet> packet, bool success, double rxPower)
{
  NS_LOG_FUNCTION ("at node " << m_device->GetNode ()->GetId () << " success? " << success);
  m_state = IDLE;
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
      CcaForDifs ();
      break;
    }
}
// -------------------------- Timeout ----------------------------------
void
THzMacMacro::CtsTimeout (uint16_t sequence)
{
  std::list<CtsTimeouts>::iterator ct = m_ctsTimeouts.begin ();
  for (; ct != m_ctsTimeouts.end (); )
    {
      if (ct->sequence == sequence)
        {
          ct = m_ctsTimeouts.erase (ct);
          break;
        }
      else
        {
          ++ct;
        }
    }
  NS_LOG_DEBUG ("!!! CTS timeout !!!");

  m_state = IDLE;
  std::list<Rec>::iterator it = m_rec.begin ();
  for (; it != m_rec.end (); ++it)
    {
      if (it->RecSeq == sequence)
        {
          it->RecRetry = it->RecRetry + 1;
          NS_LOG_DEBUG ("NODE: " << m_device->GetNode ()->GetId () << " CTS T/O: m_sequence = " << sequence << " RETRY = " << it->RecRetry);
          if (it->RecRetry >= 5)
            {
              m_pktQueue.remove (it->Recpacket);
              NS_LOG_DEBUG ("at node " << m_device->GetNode ()->GetId () << " cts timeout at:" << Simulator::Now () << " #queue " << m_pktQueue.size ());
              Simulator::Schedule (NanoSeconds (0.001), &THzMacMacro::SendDataDone, this, false, sequence);
            }
          else
            {
              NS_LOG_DEBUG ("at node " << m_device->GetNode ()->GetId () << " cts timeout at:" << Simulator::Now () << " #queue " << m_pktQueue.size ());
              Backoff (it->RecRetry);
            }
        }
    }
}

void
THzMacMacro::AckTimeout (uint16_t sequence)
{
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
  NS_LOG_DEBUG ("!!! ACK timeout !!!");
  m_state = IDLE;
  std::list<Rec>::iterator it = m_rec.begin ();
  for (; it != m_rec.end (); ++it)
    {
      if (it->RecSeq == sequence)
        {
          it->RecRetry = it->RecRetry + 1;
          NS_LOG_DEBUG ("NODE: " << m_device->GetNode ()->GetId () << " ACK T/O: m_sequence = " << sequence << " RETRY = " << it->RecRetry);
          m_thzAD = m_device->GetDirAntenna ();
          if (it->RecRetry >= 5)
            {
              m_pktQueue.remove (it->Recpacket);
              NS_LOG_DEBUG ("at node " << m_device->GetNode ()->GetId () << " ack timeout at:" << Simulator::Now () << " #queue " << m_pktQueue.size ());
              Simulator::Schedule (NanoSeconds (0.001), &THzMacMacro::SendDataDone, this, false, sequence);
            }
          else
            {
              NS_LOG_DEBUG ("at node " << m_device->GetNode ()->GetId () << " ack timeout at:" << Simulator::Now () << " #queue " << m_pktQueue.size ());
              Backoff (it->RecRetry);
            }
        }
    }
}


void
THzMacMacro::Backoff (uint32_t retry)
{
  m_retry = retry;
  Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
  uint32_t bo = uv->GetInteger (1, pow (double(2.0), double(retry)));
  m_boRemain = NanoSeconds ((double)(bo) * m_tData.GetNanoSeconds ());
  Simulator::Schedule (m_boRemain, &THzMacMacro::CcaForDifs, this);
}

// --------------------------- ETC -------------------------------------
bool
THzMacMacro::IsNewSequence (Mac48Address addr, uint16_t seq)
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
void
THzMacMacro::DoubleCw ()
{
  if (m_cw * 2 > m_cwMax)
    {
      m_cw = m_cwMax;
    }
  else
    {
      m_cw = m_cw * 2;
    }
}
// Nodes can start backoff procedure at different time because of propagation
// delay and processing jitter (it's very small but matter in simulation),

Time
THzMacMacro::RoundOffTime (Time time)
{
  int64_t realTime = time.GetNanoSeconds ();
  int64_t slotTime = GetSlotTime ().GetNanoSeconds ();
  if (realTime % slotTime >= slotTime / 2)
    {
      return NanoSeconds (GetSlotTime ().GetNanoSeconds () * (double)(realTime / slotTime + 1));
    }
  else
    {
      return NanoSeconds (GetSlotTime ().GetNanoSeconds () * (double)(realTime / slotTime));
    }

}


void
THzMacMacro::ResultsRecord ()
{
  /*----------------------------------------------------------------------------------------
   * enable the result printing in a .txt file by uncommenting the content in this function
   *----------------------------------------------------------------------------------------*/

  
  int seed_num;
   RngSeedManager seed;
   seed_num = seed.GetSeed ();

   std::stringstream txtname;
   txtname << "scratch/result" << seed_num << ".txt";
   std::string filename = txtname.str ();

   std::ofstream resultfile;
   resultfile.open (filename.c_str (), std::ios::app);
   std::list<Result>::iterator it = m_result.begin ();
   resultfile << it->nodeid << "\t" << it->Psize << "\t" << it->delay.GetNanoSeconds() << "\t" << it->success << "\t" << it->discard << std::endl;
   resultfile.close ();
   return;
 

}
} // namespace ns3
