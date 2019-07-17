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

#include <ns3/attribute.h>
#include <ns3/node.h>
#include <ns3/double.h>
#include <ns3/log.h>
#include <ns3/mobility-model.h>
#include <ns3/mobility-helper.h>
#include <ns3/angles.h>
#include "thz-dir-antenna.h"
#include "thz-net-device.h"
#include "thz-phy.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/event-id.h"
#include <vector>
#include <list>
#include <iostream>
#include <cmath>



NS_LOG_COMPONENT_DEFINE ("THzDirectionalAntenna");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (THzDirectionalAntenna);

TypeId
THzDirectionalAntenna::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::THzDirectionalAntenna")
    .SetParent<Object> ()
    .AddConstructor<THzDirectionalAntenna> ()
    .AddAttribute ("TuneRxTxMode",
                   "If 0, device is a Directional Transmitter; 1, Directional Receiver; 2, Omni-directional Tranceiver",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&THzDirectionalAntenna::m_RxTxMode),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("BeamWidth",
                   "The 3dB beamwidth (degrees)",
                   DoubleValue (40),
                   MakeDoubleAccessor (&THzDirectionalAntenna::m_beamwidthDegrees),
                   MakeDoubleChecker<double> (0, 180))
    .AddAttribute ("MaxGain",
                   "The gain (dB) at the antenna boresight (the direction of maximum gain)",
                   DoubleValue (14.12),
                   MakeDoubleAccessor (&THzDirectionalAntenna::m_maxGain),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TurningSpeed",
                   "The turning speed of the Rx antenna unit in circles per second",
                   DoubleValue (57708.85),
                   MakeDoubleAccessor (&THzDirectionalAntenna::m_turnSpeed),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("InitialAngle",
                   "Initial Angle of  Rx antenna",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&THzDirectionalAntenna::m_RxIniAngle),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}
THzDirectionalAntenna::THzDirectionalAntenna ()
{
}
THzDirectionalAntenna::~THzDirectionalAntenna ()
{
}
void
THzDirectionalAntenna::Clear ()
{
}

void
THzDirectionalAntenna::SetDevice (Ptr<THzNetDevice> device)
{
  m_device = device;
  NS_LOG_FUNCTION ("   DEV " << m_device->GetAddress ());
}


//------------------------------------SET UP ANTENNA PARAMETER-----------------------------------//
double
THzDirectionalAntenna::CheckAntennaMode (void)
{
  NS_LOG_FUNCTION ( " at node: " << m_device->GetNode ()->GetId () << "Antenna Mode: " << m_RxTxMode << " (1--Receiver; 0--Transmitter) ");
  return m_RxTxMode;
}

void
THzDirectionalAntenna::SetRxTurningSpeed (double turnSpeed)
{
  NS_LOG_FUNCTION (turnSpeed << " circles/sec ");
  m_turnSpeed = turnSpeed;
}

double
THzDirectionalAntenna::GetRxTurningSpeed () const
{
  NS_LOG_FUNCTION (m_turnSpeed << " circles/sec " << " at node: " << m_device->GetNode ()->GetId ());
  return m_turnSpeed;
}

void
THzDirectionalAntenna::SetMaxGain (double maxGain)
{
  m_maxGain = maxGain;
}

double
THzDirectionalAntenna::GetMaxGain () const
{
  NS_LOG_FUNCTION ( m_maxGain << " dB " << " at node: " << m_device->GetNode ()->GetId () );
  return m_maxGain;
}

void
THzDirectionalAntenna::SetBeamwidth (double beamwidthDegrees)
{
  m_beamwidthDegrees = beamwidthDegrees;
  m_beamwidthRadians = DegreesToRadians (beamwidthDegrees);
  m_exponent = -3.0 / (20 * std::log10 (std::cos (m_beamwidthRadians / 4.0)));
}

double
THzDirectionalAntenna::GetBeamwidth () const
{
  NS_LOG_FUNCTION ( m_beamwidthDegrees << " Degrees " << " at node: " << m_device->GetNode ()->GetId ());
  return m_beamwidthDegrees;
}

//------------------------------------------RX DA ---------------------------------------//

void
THzDirectionalAntenna::TuneRxOrientation (double phi_zero)
{
  double phi_rx = phi_zero;
  while (phi_rx <= -360)
    {
      phi_rx += 360;
    }
  while (phi_rx > 360)
    {
      phi_rx -= 360;
    }
  double phi_rx_rad = phi_rx * M_PI / 180.0;
  m_RxorientationDegrees = phi_rx;
  m_RxorientationRadians = phi_rx_rad;
  NS_LOG_DEBUG("THzDirectionalAntenna::TuneRxOrientation: " << m_RxorientationDegrees);
}

double
THzDirectionalAntenna::CheckRxOrientation ()
{
  return m_RxorientationRadians;
  NS_LOG_DEBUG("THzDirectionalAntenna::CheckRxOrientation: " << RadiansToDegrees(m_RxorientationRadians));
}



double
THzDirectionalAntenna::GetRxOrientation ()
{
  double phi_rx = m_RxIniAngle + m_turnSpeed * 360 * Simulator::Now ().GetSeconds ();
  while (phi_rx <= -360)
    {
      phi_rx += 360;
    }
  while (phi_rx > 360)
    {
      phi_rx -= 360;
    }
  double phi_rx_rad = phi_rx * M_PI / 180.0;
  m_RxorientationDegrees = phi_rx;
  m_RxorientationRadians = phi_rx_rad;
  NS_LOG_FUNCTION ( " Node " << m_device->GetNode ()->GetId () << " Current Orientation of Rx Directional Antenna " << m_RxorientationDegrees << " degrees ");
  return m_RxorientationRadians;
}

double
THzDirectionalAntenna::GetRxGainDb (Ptr<MobilityModel> senderMobility, Ptr<MobilityModel> recvMobility)
{
  Angles rxAngles (senderMobility->GetPosition (), recvMobility->GetPosition ());
  double phi_rx = rxAngles.phi - m_RxorientationRadians;
  while (phi_rx <= -M_PI)
    {
      phi_rx += M_PI + M_PI;
    }
  while (phi_rx > M_PI)
    {
      phi_rx -= M_PI + M_PI;
    }
  double ef = std::pow (std::cos (phi_rx / 2.0), m_exponent);
  double m_rxgainDb = 20 * std::log10 (ef);
  NS_LOG_FUNCTION ("   GetRxGainDb " << m_rxgainDb + m_maxGain);

  m_RxGain = m_rxgainDb + m_maxGain;
  return m_rxgainDb + m_maxGain;
}

//------------------------------------------TX DA ---------------------------------------//

double
THzDirectionalAntenna::GetTxGainDb (Ptr<MobilityModel> senderMobility, Ptr<MobilityModel> recvMobility)
{
  Angles txAngles (recvMobility->GetPosition (), senderMobility->GetPosition ());
  double m_TxorientationRadians = txAngles.phi;
  double phi_tx = txAngles.phi - m_TxorientationRadians;
  while (phi_tx <= -M_PI)
    {
      phi_tx += M_PI + M_PI;
    }
  while (phi_tx > M_PI)
    {
      phi_tx -= M_PI + M_PI;
    }
  m_TxorientationDegrees = phi_tx * 180.0 / M_PI;
  m_TxorientationRadians = phi_tx;
  // element factor: amplitude gain of a single antenna element in linear units
  double ef = std::pow (std::cos (phi_tx / 2.0), m_exponent);
  double gainDb = 20 * std::log10 (ef);
  NS_LOG_FUNCTION ("   GetTxGainDb " << gainDb + m_maxGain);

  m_TxGain = gainDb + m_maxGain;
  return gainDb + m_maxGain;
}

//------------------------------------------TOTAL DA GAIN---------------------------------------//
double
THzDirectionalAntenna::GetAntennaGain (Ptr<MobilityModel> XnodeMobility, Ptr<MobilityModel> YnodeMobility, bool XnodeMode, bool YnodeMode, double RxorientationRadians)
{
  NS_LOG_FUNCTION (" XnodeMobility: " << XnodeMobility->GetPosition () << " YnodeMobility: " << YnodeMobility->GetPosition () << " XnodeMode " << XnodeMode << " YnodeMode " << YnodeMode << "RecvOrientation" << RxorientationRadians * 180.0 / M_PI << " CurrentTime: " << Simulator::Now ().GetSeconds ());
  m_RxorientationRadians = RxorientationRadians;
  if (XnodeMode == 1 && YnodeMode == 0) // (1--Directional Receiver; 0--Directional Transmitter)
    {
      Angles rxAngles (YnodeMobility->GetPosition (), XnodeMobility->GetPosition ());
      double phi_rx = rxAngles.phi - m_RxorientationRadians;
      while (phi_rx <= -M_PI)
        {
          phi_rx += M_PI + M_PI;
        }
      while (phi_rx > M_PI)
        {
          phi_rx -= M_PI + M_PI;
        }     
      double ef_rx = std::pow (std::cos (phi_rx / 2.0), m_exponent);
      double m_rxgainDb = 20 * std::log10 (ef_rx);
      NS_LOG_DEBUG ("   GetRxGainDb " << m_rxgainDb + m_maxGain);
      m_RxGain = m_rxgainDb + m_maxGain;
      Angles txAngles (XnodeMobility->GetPosition (), YnodeMobility->GetPosition ());
      double m_TxorientationRadians = txAngles.phi;
      double phi_tx = txAngles.phi - m_TxorientationRadians;
      Simulator::ScheduleNow (&THzDirectionalAntenna::RecTxOrientation, this, txAngles.phi * 180.0 / M_PI);
      while (phi_tx <= -M_PI)
        {
          phi_tx += M_PI + M_PI;
        }
      while (phi_tx > M_PI)
        {
          phi_tx -= M_PI + M_PI;
        }
      m_TxorientationDegrees = phi_tx * 180.0 / M_PI;
      m_TxorientationRadians = phi_tx;
      NS_LOG_DEBUG ("1-Rx = "<< m_RxorientationRadians * 180.0 / M_PI<<" Tx = "<<txAngles.phi* 180.0 / M_PI<<" NOW: "<<Simulator::Now());
      double ef_tx = std::pow (std::cos (phi_tx / 2.0), m_exponent);
      double gainDb = 20 * std::log10 (ef_tx);
      m_TxGain = gainDb + m_maxGain;
      NS_LOG_DEBUG ("   GetTxGainDb " << gainDb + m_maxGain);
    }
  else if (XnodeMode == 0 && YnodeMode == 1) //  (1--Directional Receiver; 0--Directional Transmitter)
    {
      Angles rxAngles (XnodeMobility->GetPosition (), YnodeMobility->GetPosition ());
      double phi_rx = rxAngles.phi - m_RxorientationRadians;
      while (phi_rx <= -M_PI)
        {
          phi_rx += M_PI + M_PI;
        }
      while (phi_rx > M_PI)
        {
          phi_rx -= M_PI + M_PI;
        }
      double ef_rx = std::pow (std::cos (phi_rx / 2.0), m_exponent);
      double m_rxgainDb = 20 * std::log10 (ef_rx);
      NS_LOG_DEBUG ("   GetRxGainDb " << m_rxgainDb + m_maxGain);
      m_RxGain = m_rxgainDb + m_maxGain;
      Angles txAngles (YnodeMobility->GetPosition (), XnodeMobility->GetPosition ());
      double m_TxorientationRadians = txAngles.phi;
      double phi_tx = txAngles.phi - m_TxorientationRadians;
      Simulator::ScheduleNow (&THzDirectionalAntenna::RecTxOrientation, this, txAngles.phi * 180.0 / M_PI);
      while (phi_tx <= -M_PI)
        {
          phi_tx += M_PI + M_PI;
        }
      while (phi_tx > M_PI)
        {
          phi_tx -= M_PI + M_PI;
        }
      m_TxorientationDegrees = phi_tx * 180.0 / M_PI;
      m_TxorientationRadians = phi_tx;
      NS_LOG_DEBUG ("2-Rx = "<< m_RxorientationRadians * 180.0 / M_PI<<" Tx = "<<txAngles.phi* 180.0 / M_PI<<" NOW: "<<Simulator::Now());
      double ef_tx = std::pow (std::cos (phi_tx / 2.0), m_exponent);
      double gainDb = 20 * std::log10 (ef_tx);
      m_TxGain = gainDb + m_maxGain;
      NS_LOG_DEBUG ("   GetTxGainDb " << gainDb + m_maxGain);
    }
  else if (XnodeMode != 0 && XnodeMode != 1 && YnodeMode != 0 && YnodeMode != 1) //  (Omni-Directional Transmitter and receiver)
    {
      Angles rxAngles (XnodeMobility->GetPosition (), YnodeMobility->GetPosition ());
      double phi_rx = rxAngles.phi - m_RxorientationRadians;
      while (phi_rx <= -M_PI)
        {
          phi_rx += M_PI + M_PI;
        }
      while (phi_rx > M_PI)
        {
          phi_rx -= M_PI + M_PI;
        }
      double ef_rx = std::pow (std::cos (phi_rx / 2.0), m_exponent);
      double m_rxgainDb = 20 * std::log10 (ef_rx);
      NS_LOG_DEBUG ("   GetRxGainDb " << m_rxgainDb + m_maxGain);
      m_RxGain = m_rxgainDb + m_maxGain;
      Angles txAngles (XnodeMobility->GetPosition (), YnodeMobility->GetPosition ());
      double m_TxorientationRadians = txAngles.phi;
      double phi_tx = txAngles.phi - m_TxorientationRadians;
      Simulator::ScheduleNow (&THzDirectionalAntenna::RecTxOrientation, this, txAngles.phi * 180.0 / M_PI);
      while (phi_tx <= -M_PI)
        {
          phi_tx += M_PI + M_PI;
        }
      while (phi_tx > M_PI)
        {
          phi_tx -= M_PI + M_PI;
        }
      m_TxorientationDegrees = phi_tx * 180.0 / M_PI;
      m_TxorientationRadians = phi_tx;
      double ef_tx = std::pow (std::cos (phi_tx / 2.0), m_exponent);
      double gainDb = 20 * std::log10 (ef_tx);
      m_TxGain = gainDb + m_maxGain;
      NS_LOG_DEBUG ("   GetTxGainDb " << gainDb + m_maxGain);
    }
  else
    {
      m_TxGain = 0;
      m_RxGain = 0;
    }
  return m_RxGain + m_TxGain;
}


void
THzDirectionalAntenna::RecTxOrientation (double phi_tx)
{
  m_phi_tx = phi_tx;
}
double
THzDirectionalAntenna::CheckTxOrientation ()
{
  return m_phi_tx;
}

}
