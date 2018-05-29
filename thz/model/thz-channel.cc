/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 UBNANO (http://ubnano.tech/)
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

#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/mobility-model.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/object-factory.h"
#include "ns3/double.h"
#include "thz-channel.h"
#include "ns3/mac48-address.h"
#include "ns3/thz-spectrum-propagation-loss.h"
#include "ns3/thz-mac-header.h"
#include "ns3/thz-dir-antenna.h"

NS_LOG_COMPONENT_DEFINE ("THzChannel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (THzChannel);

TypeId
THzChannel::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::THzChannel")
    .SetParent<Object> ()
    .AddConstructor<THzChannel> ()
    .AddAttribute ("THzSpectrumPropagationLoss", "A pointer to the propagation loss model attached to this channel.",
                   PointerValue (CreateObject<THzSpectrumPropagationLoss> ()),
                   MakePointerAccessor (&THzChannel::m_loss),
                   MakePointerChecker<THzSpectrumPropagationLoss> ())
    .AddAttribute ("PropagationDelayModel", "A pointer to the propagation delay model attached to this channel.",
                   PointerValue (CreateObject<ConstantSpeedPropagationDelayModel> ()),
                   MakePointerAccessor (&THzChannel::m_delay),
                   MakePointerChecker<ConstantSpeedPropagationDelayModel> ())
    .AddAttribute ("NoiseFloor",
                   "Noise Floor (dBm)",
                   DoubleValue (-110.0),
                   MakeDoubleAccessor (&THzChannel::m_noiseFloor),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}
THzChannel::THzChannel ()
  : Channel ()
{
}
THzChannel::~THzChannel ()
{
}
void
THzChannel::Clear ()
{
  m_devList.clear ();
  m_noiseEntry.clear ();
}
uint32_t
THzChannel::GetNDevices () const
{
  return m_devList.size ();
}
Ptr<NetDevice>
THzChannel::GetDevice (uint32_t i) const
{
  return m_devList[i].first;
}
void
THzChannel::AddDevice (Ptr<THzNetDevice> dev, Ptr<THzPhy> phy)
{
  NS_LOG_INFO ("CH: Adding dev/phy pair number " << m_devList.size () + 1);
  m_devList.push_back (std::make_pair (dev, phy));
}

bool
THzChannel::SendPacket (Ptr<THzSpectrumSignalParameters> txParams)
{
  NS_LOG_FUNCTION ("");
  Ptr<MobilityModel> XnodeMobility = 0; // initiation
  Ptr<MobilityModel> YnodeMobility = 0;   // initiation
  m_thzDA = 0; // initiation

  NoiseEntry ne;
  ne.packet = txParams->packet;
  ne.txDuration = txParams->txDuration;
  THzDeviceList::const_iterator it = m_devList.begin ();
  for (; it != m_devList.end (); it++)
    {
      if (txParams->txPhy == it->second)
        {
          m_sendDev = it->first;
          XnodeMobility = it->first->GetNode ()->GetObject<MobilityModel> ();
          m_XnodeMode = it->first->GetDirAntenna ()->CheckAntennaMode ();
          m_thzDA = it->first->GetDirAntenna ();
          break;
        }
    }
  Simulator::Schedule (txParams->txDuration, &THzChannel::SendPacketDone, this, txParams->txPhy, txParams->packet);
  uint32_t j = 0;
  THzDeviceList::const_iterator itt = m_devList.begin ();
  for (; itt != m_devList.end (); itt++)
    {
      if (txParams->txPhy != itt->second)
        {
          YnodeMobility = itt->first->GetNode ()->GetObject<MobilityModel> ();
          m_YnodeMode = itt->first->GetDirAntenna ()->CheckAntennaMode ();
          Time delay = m_delay->GetDelay (XnodeMobility, YnodeMobility); // propagation delay
          if (m_XnodeMode == 1 && m_YnodeMode == 0) // 1--Receiver; 0--Transmitter
            {
              m_Rxorientation = m_thzDA->CheckRxOrientation ();  // turning sector by sector
            }
          if (m_XnodeMode == 0 && m_YnodeMode == 1)
            {
              m_Rxorientation = itt->first->GetDirAntenna ()->CheckRxOrientation ();
            }
          if (m_XnodeMode == 2 && m_YnodeMode == 2)
            {
              m_Rxorientation = 0;
            }
          m_totalGain = itt->first->GetDirAntenna ()->GetAntennaGain (XnodeMobility, YnodeMobility, m_XnodeMode, m_YnodeMode, m_Rxorientation);
          double rxPower = m_loss->CalcRxPowerDA (txParams, XnodeMobility, YnodeMobility, m_totalGain);
          NS_LOG_DEBUG ("node "<<it->first->GetNode ()->GetId ()<<"->"<<itt->first->GetNode ()->GetId ()<<", txPower = "<< txParams->txPower<<" dBm, totalGain = "<< m_totalGain +30 <<" dBm, rxPower = "<<rxPower<< " dBm"<< "  now: "<< Simulator::Now());
          uint32_t dstNodeId = itt->first->GetNode ()->GetId ();
          Ptr<Packet> copy = txParams->packet->Copy ();
          ne.packet = copy;
          ne.phy = itt->second;
          ne.rxPower = rxPower;
          ne.txEnd = Simulator::Now () + txParams->txDuration + delay;
          Simulator::ScheduleWithContext (dstNodeId, delay, &THzChannel::ReceivePacket, this, j, ne);
        }
      j++;
    }
  return true;
}


void
THzChannel::SendPacketDone (Ptr<THzPhy> phy, Ptr<Packet> packet)
{
  NS_LOG_FUNCTION ("");
  phy->SendPacketDone (packet);
}

void
THzChannel::ReceivePacket (uint32_t i, NoiseEntry ne)
{
  NS_LOG_FUNCTION ("");
  m_noiseEntry.push_back (ne);
  m_devList[i].second->ReceivePacket (ne.packet, ne.txDuration, ne.rxPower);
  Simulator::Schedule (ne.txDuration, &THzChannel::ReceivePacketDone, this, i, ne);
}
void
THzChannel::ReceivePacketDone (uint32_t i, NoiseEntry ne)
{
  NS_LOG_FUNCTION ("");
  m_devList[i].second->ReceivePacketDone (ne.packet, ne.rxPower);
  Simulator::ScheduleNow(&THzChannel::DeleteNoiseEntry, this, ne);
}

void
THzChannel::DeleteNoiseEntry (NoiseEntry ne)
{
  NS_LOG_FUNCTION (this);
  std::list<NoiseEntry>::iterator it = m_noiseEntry.begin ();
  for (; it != m_noiseEntry.end (); ++it)
    {
      if (it->packet == ne.packet && it->phy == ne.phy)
        {
          m_noiseEntry.erase (it);
          break;
        }
    }
}

double
THzChannel::GetNoiseW (double interference)
{
  Time now = Simulator::Now ();
  double noiseW = DbmToW (m_noiseFloor) + interference;
  return noiseW;
}


double
THzChannel::DbmToW (double dbm)
{
  double mw = pow (10.0,dbm / 10.0);
  return mw / 1000.0;
}

} // namespace ns3
