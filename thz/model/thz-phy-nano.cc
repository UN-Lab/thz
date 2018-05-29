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
 * Author: Zahed Hossain <zahedhos@buffalo.edu>
 *         Qing Xia <qingxia@buffalo.edu>
 *         Josep Miquel Jornet <jmjornet@buffalo.edu>
 */



#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/mac48-address.h"
#include "thz-mac.h"
#include "thz-mac-nano.h"
#include "thz-phy-nano.h"
#include "thz-phy.h"
#include "thz-spectrum-signal-parameters.h"
#include "ns3/callback.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/traced-value.h"
#include "ns3/mobility-model.h"
#include "thz-spectrum-waveform.h"
#include <ns3/spectrum-value.h>

#include "ns3/nstime.h"
#include "ns3/object-factory.h"
#include "ns3/pointer.h"
#include "ns3/boolean.h"
#include <cmath> //to use fmod()


NS_LOG_COMPONENT_DEFINE ("THzPhyNano");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (THzPhyNano);


TypeId
THzPhyNano::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::THzPhyNano")
    .SetParent<THzPhy> ()
    .AddConstructor<THzPhyNano> ()
    .AddAttribute ("SinrTh",
                   "SINR Threshold (dB)",
                   DoubleValue (10),
                   MakeDoubleAccessor (&THzPhyNano::m_sinrTh),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxPower",
                   "Transmission Power (dBm)",
                   DoubleValue (-20),
                   MakeDoubleAccessor (&THzPhyNano::SetTxPower),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("PulseDuration", 
                   "Duration of a short pulse",
                   TimeValue (FemtoSeconds (100)),
                   MakeTimeAccessor (&THzPhyNano::m_pulseDuration),
                   MakeTimeChecker ())
    .AddAttribute ("Beta", 
                   "Ratio of symbol duratio to pulse duration",
                   DoubleValue (100),
                   MakeDoubleAccessor (&THzPhyNano::m_beta),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

THzPhyNano::THzPhyNano ()
  : m_device (0),
    m_mac (0),
    m_channel (0)
{
  Simulator::ScheduleNow (&THzPhyNano::CalTxPsd, this);
}

THzPhyNano::~THzPhyNano ()
{
  Clear ();
}
void
THzPhyNano::Clear ()
{
}

void
THzPhyNano::SetDevice (Ptr<THzNetDevice> device)
{
  m_device = device;
}
void
THzPhyNano::SetMac (Ptr<THzMac> mac)
{
  m_mac = mac;
}
void
THzPhyNano::SetChannel (Ptr<THzChannel> channel)
{
  m_channel = channel;
}
void
THzPhyNano::SetTxPower (double dBm)
{
  m_txPower = dBm;
}

//-----------------------------------------------------------------
Ptr<THzNetDevice>
THzPhyNano::GetDevice ()
{
  return m_device;
}
Ptr<THzChannel>
THzPhyNano::GetChannel ()
{
  return m_channel;
}
Mac48Address
THzPhyNano::GetAddress ()
{
  return m_mac->GetAddress ();
}
double
THzPhyNano::GetTxPower ()
{
  return m_txPower;
}

//----------------------------------------------------------------------
void
THzPhyNano::CalTxPsd ()
{
  NS_LOG_FUNCTION ("");
  double txPowerW = DbmToW (m_txPower);
  Ptr<THzSpectrumValueFactory> sf = CreateObject<THzSpectrumValueFactory> ();
  Ptr<SpectrumModel> InitTHzSpectrumWave;
  Ptr<SpectrumModel> InitTHzSpectrumWaveAll;
  Ptr<SpectrumModel> InitTHzPulseSpectrumWaveform;
  InitTHzSpectrumWave = sf->THzSpectrumWaveformInitializer ();
  InitTHzSpectrumWaveAll = sf->AllTHzSpectrumWaveformInitializer ();
  InitTHzPulseSpectrumWaveform = sf->THzPulseSpectrumWaveformInitializer ();
  m_txPsd = sf->CreatePulsePowerSpectralDensity (1, m_pulseDuration.ToDouble (Time::S), txPowerW);
  m_numberOfSamples = sf->m_numsample;
  m_numberOfSubBands = sf->m_numsb;
  m_subBandBandwidth = sf->m_sbw;
}
bool
THzPhyNano::SendPacket (Ptr<Packet> packet, bool rate)
{
  NS_LOG_FUNCTION ("packet" << packet << "now" << Simulator::Now ());

  Time txDuration;
  Ts = FemtoSeconds (m_beta * m_pulseDuration.ToDouble (Time::FS)); //Symbol duration
  NS_LOG_INFO ("Ts:" << Ts);

  if (rate) // transmit packet with data rate
    {
      txDuration = CalTxDuration (0, packet->GetSize ());
    }
  else // transmit packets (e.g. RTS, CTS) with basic rate
    {
      txDuration = CalTxDuration (packet->GetSize (), 0);
    }

  OngoingTx ot;  //Record the current transmissions, schedule to erase them after their duration

  if (m_ongoingTx.size () != 0 || m_ongoingRx.size () != 0) //the m_ongoingTx is not empty and less than Ts/m_pulse
    {
      NS_LOG_INFO ("Size of the transmission list:" << m_ongoingTx.size () << "Size of the receive list:" << m_ongoingRx.size ());

      Time now = Simulator::Now ();
      double TsD = Ts.ToDouble (Time::FS);
      double nowD = now.ToDouble (Time::FS);

      double now_Ts = (now + Ts).ToDouble (Time::FS);

      std::vector<double> nextPulse (m_ongoingTx.size () + m_ongoingRx.size () + 2);

      NS_LOG_INFO ("Size of nextPulse: " << nextPulse.size ());
      int size = nextPulse.size ();
      nextPulse[0] = nowD;
      nextPulse[size - 1] = now_Ts;

      std::list<OngoingTx>::iterator it = m_ongoingTx.begin ();
      int txCount = 1; //Keeps track of the no of simultaneous transmissions
//******************Map all the ongoing Txs and Rxs into one symbol duration************************
      for (; it != m_ongoingTx.end (); ++it)
        {
          double mod = fmod ((now - it->m_txStart).ToDouble (Time::FS),TsD);
          NS_LOG_INFO ("mod:" << mod << "m_pulse:" << m_pulseDuration);

          if (mod < m_pulseDuration)
            {
              NS_LOG_INFO ("Start time of the existing transmission is:" << it->m_txStart);
              nextPulse[txCount] = (nowD - mod);
              NS_LOG_INFO ("now:" << now << "The next pulse inside if:" << nextPulse[txCount]);
              txCount++;
            }
          else
            {
              NS_LOG_INFO ("Start time of the existing transmission is:" << it->m_txStart);
              nextPulse[txCount] = (nowD + TsD - mod);
              NS_LOG_INFO ("now:" << now << "The next pulse inside else:" << nextPulse[txCount]);
              txCount++;
            }
        }

      std::list<OngoingRx>::iterator itr = m_ongoingRx.begin ();

      for (; itr != m_ongoingRx.end (); ++itr)
        {
          double mod = fmod ((now - itr->m_rxStart).ToDouble (Time::FS),TsD);
          NS_LOG_INFO ("mod:" << mod << "m_pulse:" << m_pulseDuration);
          if (mod < m_pulseDuration)
            {
              NS_LOG_INFO ("Start time of the existing reception is:" << it->m_txStart);
              nextPulse[txCount] = (nowD - mod);
              NS_LOG_INFO ("now:" << now << "The next pulse inside if:" << nextPulse[txCount]);
              txCount++;
            }
          else
            {
              NS_LOG_INFO ("Start time of the existing reception is:" << itr->m_rxStart);
              NS_LOG_INFO ("nowD:" << nowD << " TsD" << TsD << " fmod" << fmod ((now - itr->m_rxStart).ToDouble (Time::FS),TsD));
              nextPulse[txCount] =  (nowD + TsD - mod);
              NS_LOG_INFO ("now:" << now << "The next pulse of reception will appear at:" << FemtoSeconds (nextPulse[txCount]));
              txCount++;
            }
        }

//****************************sort the array in the ascending order of pulse times****************************
      SortArray (&nextPulse[0],size);
      NS_LOG_INFO ("Size of nextPulse: " << sizeof(nextPulse) / sizeof(nextPulse[0]) << "NextPulse[0] " << FemtoSeconds (nextPulse[0]));
      for (int j = 1; j <= size; j++)
        {
          if (nextPulse[j - 1] != (nowD + TsD) && nextPulse[j - 1] + 2 * m_pulseDuration.ToDouble (Time::FS) <= nextPulse[j])
            {
              NS_LOG_INFO ("NextPulse[j-1] " << FemtoSeconds (nextPulse[j - 1]));
              ot.m_txStart = FemtoSeconds (nextPulse[j - 1] + m_pulseDuration.ToDouble (Time::FS));
              NS_LOG_INFO ("schedule in " << ot.m_txStart - Simulator::Now () << " at: " << ot.m_txStart);

              Simulator::Schedule (ot.m_txStart - Simulator::Now (), &THzPhyNano::ScheduleSendPacket, this,packet, txDuration );

              ot.m_txDuration = txDuration;
              m_ongoingTx.push_back (ot);

              NS_LOG_DEBUG ("Tx interleaved and will finish at " << (ot.m_txStart + txDuration).GetFemtoSeconds () << "fs txPower" << m_txPower);
              Simulator::Schedule ((ot.m_txStart - Simulator::Now ()) + txDuration, &THzPhyNano::DeleteOngoingTx, this, ot);
              return true;
            }
        }
      NS_LOG_DEBUG ("Transmission abort:no interleaving possible ");
      return false;
    }

  NS_LOG_INFO ("Tx and Rx lists are empty");

  NS_LOG_DEBUG ("Tx not interleaved and will finish at " << (Simulator::Now () + txDuration).GetFemtoSeconds () << "fs txPower" << m_txPower);
  Simulator::Schedule (Seconds (0), &THzPhyNano::ScheduleSendPacket, this,packet, txDuration );

  ot.m_txStart = Simulator::Now ();
  ot.m_txDuration = txDuration;
  m_ongoingTx.push_back (ot);

  Simulator::Schedule (txDuration, &THzPhyNano::DeleteOngoingTx, this, ot);

  return true;
}

void
THzPhyNano::ScheduleSendPacket (Ptr<Packet> packet, Time txDuration)
{
  NS_LOG_FUNCTION ("now" << Simulator::Now ());
  Ptr<THzSpectrumSignalParameters> txParams = Create<THzSpectrumSignalParameters> ();
  txParams->txDuration = txDuration;
  txParams->txPower = m_txPower;
  txParams->numberOfSamples = m_numberOfSamples;
  txParams->numberOfSubBands = m_numberOfSubBands;
  txParams->subBandBandwidth = m_subBandBandwidth;
  txParams->txPhy = Ptr<THzPhyNano> (this);
  txParams->txPsd = m_txPsd;
  txParams->packet = packet;

  m_channel->SendPacket (txParams);
}


void
THzPhyNano::DeleteOngoingTx (OngoingTx ot)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("now" << Simulator::Now ());
  std::list<OngoingTx>::iterator it = m_ongoingTx.begin ();
  for (; it != m_ongoingTx.end (); ++it)
    {
      if (it->m_txStart == ot.m_txStart && it->m_txDuration == ot.m_txDuration)
        {
          m_ongoingTx.erase (it);
          break;
        }
    }
}


void
THzPhyNano::SendPacketDone (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION ("");
  m_mac->SendPacketDone (packet);
}

void
THzPhyNano::ReceivePacket (Ptr<Packet> packet, Time txDuration, double_t rxPower)
{
  NS_LOG_FUNCTION ("rxPower" << rxPower << "packet" << packet << "now" << Simulator::Now ());
  Time now = Simulator::Now ();
  OngoingRx ot;
  Ts = FemtoSeconds (m_beta * m_pulseDuration.ToDouble (Time::FS));
  ot.m_rxStart = Simulator::Now ();
  ot.m_rxDuration = txDuration;
  ot.packet = packet;
  ot.rxPower = rxPower;

  double nowD = now.ToDouble (Time::FS);
  double TsD = Ts.ToDouble (Time::FS);

  if (m_ongoingTx.size () != 0) //the m_ongoingTx is not empty
    {
      std::list<OngoingTx>::iterator it = m_ongoingTx.begin ();
      for (; it != m_ongoingTx.end (); ++it)
        {
          if (FemtoSeconds (fmod ((nowD - (it->m_txStart - m_pulseDuration).ToDouble (Time::FS)),TsD)) < 2 * m_pulseDuration)
            {
              NS_LOG_INFO ("Drop packet due to half-duplex");
              return;
            }
        }
    }

  if (m_ongoingRx.size () != 0) //the m_ongoingRx is not empty, calculate SINR, Should we add noise?********************
    {
      std::list<OngoingRx>::iterator it = m_ongoingRx.begin ();
      for (; it != m_ongoingRx.end (); ++it)
        {
          if (FemtoSeconds (fmod ((nowD - (it->m_rxStart - m_pulseDuration).ToDouble (Time::FS)),TsD)) < 2 * m_pulseDuration)
            {
              ot.interference += DbmToW (it->rxPower);
              it->interference = 0;
              it->interference += DbmToW (rxPower); //current signal is interfernce to the old one as well, this is because we have simultaneous Txs%
              std::list<OngoingRx>::iterator it1 = m_ongoingRx.begin ();

              for (; it1 != m_ongoingRx.end (); ++it1)
                {
                  if (it1->packet != it->packet)
                    {
                      if (FemtoSeconds (fmod ((nowD - (it1->m_rxStart - m_pulseDuration).ToDouble (Time::FS)),TsD)) < 2 * m_pulseDuration)
                        {
                          it->interference += DbmToW (it1->rxPower);
                        }
                    }
                }

              double noisePlusInterference = m_channel->GetNoiseW (it->interference); // noise plus interference
              double rxPowerW = DbmToW (it->rxPower);
              double sinr = rxPowerW / noisePlusInterference;
              NS_LOG_INFO ("SINR" << sinr);
              if (sinr < m_sinrTh)
                {
                  it->m_collided = true;                //packet should be dropped
                }
              else
                {
                  ot.m_collided = false;  //reception is still good
                }
            }
        }
    }

  double noisePlusInterference = m_channel->GetNoiseW (ot.interference); // noise plus interference
  double rxPowerW = DbmToW (ot.rxPower);
  double sinr = rxPowerW / noisePlusInterference;
  NS_LOG_INFO ("SINR" << sinr);
  if (sinr < m_sinrTh)
    {
      ot.m_collided = true;
    }
  else
    {
      ot.m_collided = false;
    }
  m_ongoingRx.push_back (ot);

}

void
THzPhyNano::ReceivePacketDone (Ptr<Packet> packet, double rxPower)
{
  NS_LOG_FUNCTION ("now" << Simulator::Now ());

  NS_LOG_INFO ("Size of the receive list:" << m_ongoingRx.size ());
  std::list<OngoingRx>::iterator it = m_ongoingRx.begin ();
  for (; it != m_ongoingRx.end (); ++it)
    {
      if (it->packet == packet)
        {
          if ( it->m_collided == false)
            {
              NS_LOG_INFO ("Packet hasn't collided!");
              m_mac->ReceivePacketDone (this, packet, true);
              m_ongoingRx.erase (it);
              return;
            }

          else
            {
              NS_LOG_INFO ("Packet has collided");
              m_mac->ReceivePacketDone (this, packet, false);
              m_ongoingRx.erase (it);
              return;
            }

        }

    }

}

Time
THzPhyNano::CalTxDuration (uint32_t basicSize, uint32_t dataSize)
{
  NS_LOG_FUNCTION ("");
  Ts = FemtoSeconds (m_beta * m_pulseDuration.ToDouble (Time::FS)); //Symbol duration
  double Rate = 1 / Ts.ToDouble (Time::S);
  double_t controlPacketTime = (double) basicSize * 8.0 / Rate;
  double_t dataPacketTime = (double)dataSize * 8.0 / Rate;
  return Seconds (controlPacketTime) + Seconds (dataPacketTime);
}

double*
THzPhyNano::SortArray (double timeArray[], int n)
{
  NS_LOG_FUNCTION ("");
  int i, j;
  double a;

  for (i = 0; i < n; ++i)
    {
      for (j = i + 1; j < n; ++j)
        {
          if (timeArray[i] > timeArray[j])
            {
              a =  timeArray[i];
              timeArray[i] = timeArray[j];
              timeArray[j] = a;
            }
        }
    }

  return timeArray;
}

double
THzPhyNano::DbmToW (double dbm)
{
  double mw = pow (10.0,dbm / 10.0);
  return mw / 1000.0;
}

} // namespace ns3
