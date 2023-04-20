/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2023 Northeastern University (https://unlab.tech/)
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
 *         Josep Miquel Jornet <j.jornet@northeastern.edu>
 *         Daniel Morales <danimoralesbrotons@gmail.com>
 */

#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/mac48-address.h"
#include "thz-mac.h"
#include "thz-phy.h"
#include "thz-phy-macro.h"
#include "thz-spectrum-signal-parameters.h"
#include "ns3/callback.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/traced-value.h"
#include "ns3/mobility-model.h"
#include "thz-dir-antenna.h"
#include "thz-spectrum-waveform.h"
#include <iostream>


NS_LOG_COMPONENT_DEFINE ("THzPhyMacro");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (THzPhyMacro);

THzPhyMacro::THzPhyMacro ()
  : m_device (0),
  m_mac (0),
  m_channel (0),
  m_pktRx (0)

{
  m_csBusy = false;
  m_csBusyEnd = Seconds (0);
  m_state = IDLE;
  Simulator::ScheduleNow (&THzPhyMacro::CalTxPsd, this);
}



THzPhyMacro::~THzPhyMacro ()
{
  Clear ();
}
void
THzPhyMacro::Clear ()
{
  m_pktRx = 0;

}
TypeId
THzPhyMacro::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::THzPhyMacro")
    .SetParent<Object> ()
    .AddConstructor<THzPhyMacro> ()
    .AddAttribute ("PreambleDuration",
                   "Duration (us) of Preamble of PHY Layer",
                   TimeValue (MicroSeconds (0.0)),
                   MakeTimeAccessor (&THzPhyMacro::m_preambleDuration),
                   MakeTimeChecker ())
    .AddAttribute ("TrailerSize",
                   "Size of Trailer (e.g. FCS) (bytes)",
                   UintegerValue (2),
                   MakeUintegerAccessor (&THzPhyMacro::m_trailerSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("HeaderSize",
                   "Size of Header (bytes)",
                   UintegerValue (3),
                   MakeUintegerAccessor (&THzPhyMacro::m_headerSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("SinrTh",
                   "SINR Threshold",
                   DoubleValue (10),
                   MakeDoubleAccessor (&THzPhyMacro::m_sinrTh),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("CsPowerTh",
                   "Carrier Sense Threshold (dBm)",
                   DoubleValue (-100),
                   MakeDoubleAccessor (&THzPhyMacro::m_csTh),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxPower",
                   "Transmission Power (dBm)",
                   DoubleValue (-20),
                   MakeDoubleAccessor (&THzPhyMacro::SetTxPower),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("BasicRate",
                   "Transmission Rate (bps) for Control Packets",
                   DoubleValue (1.4801e11),
                   MakeDoubleAccessor (&THzPhyMacro::m_basicRate),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("DataRate",
                   "Transmission Rate (bps) for Data Packets",
                   DoubleValue (1.4801e11),
                   MakeDoubleAccessor (&THzPhyMacro::m_dataRate),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("DataRateBPSK",
                   "Transmission Rate (bps) for Data Packets",
                   DoubleValue (52.48e9),
                   MakeDoubleAccessor (&THzPhyMacro::m_dataRateBSPK),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("DataRateQPSK",
                   "Transmission Rate (bps) for Data Packets",
                   DoubleValue (105.28e9),
                   MakeDoubleAccessor (&THzPhyMacro::m_dataRateQSPK),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("DataRate8PSK",
                   "Transmission Rate (bps) for Data Packets",
                   DoubleValue (157.44e9),
                   MakeDoubleAccessor (&THzPhyMacro::m_dataRate8SPK),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("DataRate16QAM",
                   "Transmission Rate (bps) for Data Packets",
                   DoubleValue (210.24e9),
                   MakeDoubleAccessor (&THzPhyMacro::m_dataRate16QAM),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("DataRate64QAM",
                   "Transmission Rate (bps) for Data Packets",
                   DoubleValue (315.52e9),
                   MakeDoubleAccessor (&THzPhyMacro::m_dataRate64QAM),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

void
THzPhyMacro::CalTxPsd ()
{
  NS_LOG_FUNCTION ("");
  double txPowerMW = std::pow (10.0, m_txPower / 10.0);
  double txPowerW = txPowerMW / 1000.0;

  Ptr<THzSpectrumValueFactory> sf = CreateObject<THzSpectrumValueFactory> ();
  Ptr<SpectrumModel> InitTHzSpectrumWave;
  Ptr<SpectrumModel> InitTHzSpectrumWaveAll;
  InitTHzSpectrumWave = sf->THzSpectrumWaveformInitializer ();
  InitTHzSpectrumWaveAll = sf->AllTHzSpectrumWaveformInitializer ();
  m_txPsd = sf->CreateTxPowerSpectralDensity (txPowerW);
  m_numberOfSamples = sf->m_numsample;
  m_numberOfSubBands = sf->m_numsb;
  m_subBandBandwidth = sf->m_sbw;

}


void
THzPhyMacro::SetDevice (Ptr<THzNetDevice> device)
{
  m_device = device;
}

Ptr<THzNetDevice>
THzPhyMacro::GetDevice ()
{
  return m_device;
}

void
THzPhyMacro::SetMac (Ptr<THzMac> mac)
{
  m_mac = mac;
}
void
THzPhyMacro::SetChannel (Ptr<THzChannel> channel)
{
  m_channel = channel;
}

void
THzPhyMacro::SetTxPower (double dBm)
{
  m_txPower = dBm;
}

uint32_t
THzPhyMacro::GetBasicRate ()
{
  return m_basicRate;
}
double
THzPhyMacro::GetDataRate (int mcs)
{
  switch (mcs)
  {
  case 10:
    return m_dataRateBSPK;
  case 11:
    return m_dataRateQSPK;
  case 12:
    return m_dataRate8SPK;
  case 13:
    return m_dataRate16QAM;
  case 14:
    return m_dataRate64QAM;
  default:  // any other value, use default data rate
    return m_dataRate;
  }
}
//-----------------------------------------------------------------
Ptr<THzChannel>
THzPhyMacro::GetChannel ()
{
  return m_channel;
}
Mac48Address
THzPhyMacro::GetAddress ()
{
  return m_mac->GetAddress ();
}
double
THzPhyMacro::GetTxPower ()
{
  return m_txPower;
}

bool
THzPhyMacro::SendPacket (Ptr<Packet> packet, bool rate, uint16_t mcs)
{
  NS_LOG_FUNCTION (" from node " << m_device->GetNode ()->GetId () << " state " << (m_state));
  // RX might be interrupted by TX, but not vice versa
  if (m_state == TX)
    {
      NS_LOG_DEBUG ("Already in transmission mode");
      return false;
    }
  m_state = TX;
  Time txDuration;
  if (rate) // transmit packet with data rate
    {
      txDuration = CalTxDuration (0, packet->GetSize (), mcs);
    }
  else // transmit packets (e.g. RTS, CTS) with basic rate
    {
      txDuration = CalTxDuration (packet->GetSize (), 0, 0);
    }
  NS_LOG_DEBUG ("Tx will finish at " << (Simulator::Now () + txDuration).GetPicoSeconds () << "(ps) txPower=" << m_txPower << " dBm");

  Ptr<THzSpectrumSignalParameters> txParams = Create<THzSpectrumSignalParameters> ();
  txParams->txDuration = txDuration;
  txParams->txPower = m_txPower;
  txParams->numberOfSamples = m_numberOfSamples;
  txParams->numberOfSubBands = m_numberOfSubBands;
  txParams->subBandBandwidth = m_subBandBandwidth;
  txParams->txPhy = Ptr<THzPhyMacro> (this);
  txParams->txPsd = m_txPsd;
  txParams->packet = packet;
  // forward to CHANNEL
  m_channel->SendPacket (txParams);
  return true;
}

void
THzPhyMacro::SendPacketDone (Ptr<Packet> packet)
{
  m_state = IDLE;
  NS_LOG_FUNCTION ("from node " << m_device->GetNode ()->GetId () << " state " << (m_state));
  m_mac->SendPacketDone (packet);
}


void
THzPhyMacro::ReceivePacket (Ptr<Packet> packet, Time txDuration, double_t rxPower)
{
  NS_LOG_FUNCTION ("at node " << m_device->GetNode ()->GetId () << " rxPower " << rxPower << " dBm" << "busyEnd" << m_csBusyEnd << " state " << (m_state));

  OngoingRx ot;
  ot.rxStart = Simulator::Now ();
  ot.rxDuration = txDuration;
  ot.packet = packet;
  ot.rxPower = rxPower;
  m_ongoingRx.push_back (ot);

  if (m_state == TX)
    {
      NS_LOG_INFO ("Now transmitting Drop packet due to half-duplex");
      return;
    }
  // Start RX when energy is bigger than carrier sense threshold
  Time txEnd = Simulator::Now () + txDuration;
  if (rxPower > m_csTh && txEnd > m_csBusyEnd)
    {
      if (m_csBusy == false)
        {
          m_csBusy = true;
          m_pktRx = packet;
          //NS_LOG_UNCOND(m_device->GetNode ()->GetId () << ": Packet received. rxPower: " << rxPower << ", threshold: " << m_csTh);

          m_mac->ReceivePacket (this, packet);
        }
      m_state = RX;
      m_csBusyEnd = txEnd;
    }
  if (rxPower < m_csTh)
    {
      NS_LOG_INFO ("rxPower < m_csTh");
      /*if (rxPower > m_csTh - 10){
        NS_LOG_UNCOND(m_device->GetNode ()->GetId () << ": Not enough power. rxPower: " << rxPower << ", threshold: " << m_csTh);
      }*/
      return;
    }


}

void
THzPhyMacro::ReceivePacketDone (Ptr<Packet> packet, double rxPower)
{
  NS_LOG_FUNCTION ("at node " << m_device->GetNode ()->GetId () << "csBusy " << m_csBusyEnd << " now " << Simulator::Now () << " state " << (m_state));

  if (m_csBusyEnd <= Simulator::Now () + NanoSeconds (1))
    {
      m_csBusy = false;
    }
  if (m_state != RX)
    {
      NS_LOG_INFO ("Drop packet due to state");
      NS_LOG_DEBUG ("Current Status " << (m_state) << " Need Status RX ");
      return;
    }

  double interference = 0;
  if (packet == m_pktRx)
    {
      std::list<OngoingRx>::iterator it = m_ongoingRx.begin ();
      for (; it != m_ongoingRx.end (); ++it)
        {
          if (it->packet != m_pktRx &&  (Simulator::Now () - it->rxStart) <= it->rxDuration)
            {
              interference += DbmToW (it->rxPower);
              //NS_LOG_UNCOND("Interference at " << m_device->GetNode ()->GetId () << " of " << it->rxPower << " W");
            }
        }

      // We do support SINR !!
      double noiseW = m_channel->GetNoiseW (interference); // noise plus interference
      double rxPowerW = m_channel->DbmToW (rxPower);
      double sinr = rxPowerW / noiseW;
      double sinrDb = 10 * std::log10 (sinr);
      NS_LOG_DEBUG ("SINR = " << sinrDb << " dB; SINR TH = " << m_sinrTh << " dB");
      // ADD: CHANGE STATUS
      m_state = IDLE;
      //NS_LOG_UNCOND(m_device->GetNode ()->GetId () << ": SINR = " << sinrDb << " dB; SINR TH = " << m_sinrTh << " dB");
      if (sinrDb > m_sinrTh)
        {
          m_state = IDLE;
          m_mac->ReceivePacketDone (this, packet, true, rxPower);
          return;
        }
      else
        {
          m_mac->ReceivePacketDone (this,packet,false, rxPower);
        }

      for (; it != m_ongoingRx.end (); ++it)
        {
          if (it->packet == m_pktRx)
            {
              m_ongoingRx.erase (it);
            }
        }
    }

  if (!m_csBusy)
    {
      m_state = IDLE;
      m_mac->ReceivePacketDone (this, packet, false, rxPower);
    }
}

bool
THzPhyMacro::IsIdle ()
{
  if (m_state == IDLE && !m_csBusy)
    {
      return true;
    }
  return false;
}

Time
THzPhyMacro::CalTxDuration (uint32_t basicSize, uint32_t dataSize, uint8_t mcs)
{
  double_t txHdrTime = (double)(m_headerSize + basicSize + m_trailerSize) * 8.0 / (double)GetDataRate(mcs);
  double_t txMpduTime = (double)dataSize * 8.0 / (double)GetDataRate(mcs);
  return m_preambleDuration + Seconds (txHdrTime) + Seconds (txMpduTime);
}


double
THzPhyMacro::DbmToW (double dbm)
{
  double mw = pow (10.0,dbm / 10.0);
  return mw / 1000.0;
}

} // namespace ns3
