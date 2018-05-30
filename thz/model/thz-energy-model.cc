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

#include "traffic-generator.h"
#include "ns3/application.h"
#include "thz-energy-model.h"
#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/double.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/simulator.h"
#include "ns3/node.h"
#include "ns3/random-variable-stream.h"

NS_LOG_COMPONENT_DEFINE ("THzEnergyModel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (THzEnergyModel);
// 1 attoJoule = 0.125 energy frames
TypeId
THzEnergyModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::THzEnergyModel")
    .SetParent<Object> ()
    .AddConstructor<THzEnergyModel> ()
    .AddAttribute ("THzEnergyModelInitialEnergy",
                   "Initial energy stored in basic energy source.",
                   DoubleValue (0.0),  // in frames
                   MakeDoubleAccessor (&THzEnergyModel::m_initialEnergy),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("DataCallbackEnergy",
                   "Lets the MAC layer know that it has harvested enough to transmit one packet.",
                   DoubleValue (65.0),  // in frames
                   MakeDoubleAccessor (&THzEnergyModel::m_dataCallbacklEnergy),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("EnergyHarvestingAmount",
                   "Amount of Energy Harvested in each time.",
                   DoubleValue (1.0), // in frames
                   MakeDoubleAccessor (&THzEnergyModel::m_energyHarvestingAmount),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("PeriodicEnergyUpdateInterval",
                   "Time between two consecutive periodic energy updates.",
                   TimeValue (MicroSeconds (8.0)),
                   MakeTimeAccessor (&THzEnergyModel::m_energyUpdateInterval),
                   MakeTimeChecker ())
    .AddAttribute ("EnergyConsumptionPulseTx",
                   "Energy consumption for the transmission of a pulse.",
                   DoubleValue (0.125),
                   MakeDoubleAccessor (&THzEnergyModel::m_energyConsumptionPulseTx),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("EnergyConsumptionPulseRx",
                   "Energy consumption for the transmission of a pulse.",
                   DoubleValue (12.5e-3),
                   MakeDoubleAccessor (&THzEnergyModel::m_energyConsumptionPulseRx),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("CodingWeight",
                   "Percentage of transmitting a pulse instead of being silent.",
                   DoubleValue (0.5),
                   MakeDoubleAccessor (&THzEnergyModel::m_codingWeight),
                   MakeDoubleChecker<double> ())
    .AddTraceSource ("RemainingEnergy",
                     "Remaining energy at THzEnergyModel.",
                     MakeTraceSourceAccessor (&THzEnergyModel::m_remainingEnergy))
  ;
  return tid;
}

THzEnergyModel::THzEnergyModel ()
{
  NS_LOG_FUNCTION (this);
  Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
  uint32_t energyInitTime = uv->GetInteger (0, m_dataCallbacklEnergy); //used to randomize the transmission start times
  Simulator::Schedule (MicroSeconds (8.0 * energyInitTime), &THzEnergyModel::DoInitialize, this);
}

THzEnergyModel::~THzEnergyModel ()
{
  NS_LOG_FUNCTION (this);
}

void
THzEnergyModel::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this);
  m_node = node;
}
void
THzEnergyModel::SetEnergyCallback ( Callback<void> energyCbData)
{
  NS_LOG_FUNCTION (this << &energyCbData);
  m_energyCbData = energyCbData;
}
void
THzEnergyModel::SetEnergyUpdateInterval (Time interval)
{
  NS_LOG_FUNCTION (this << interval);
  m_energyUpdateInterval = interval;
}

Time
THzEnergyModel::GetEnergyUpdateInterval (void) const
{
  NS_LOG_FUNCTION (this);
  return m_energyUpdateInterval;
}

double
THzEnergyModel::GetInitialEnergy (void) const
{
  NS_LOG_FUNCTION (this);
  return m_initialEnergy;
}

double
THzEnergyModel::GetRemainingEnergy (void)
{
  NS_LOG_FUNCTION (this);
  return m_remainingEnergy;
}

void
THzEnergyModel::HarvestEnergy (void)
{
  NS_LOG_FUNCTION (this);

  if (Simulator::IsFinished ())
    {
      return;
    }

  m_remainingEnergy += m_energyHarvestingAmount;
  if (m_remainingEnergy == m_dataCallbacklEnergy)
    {
      m_energyCbData ();                                 
    }
  m_energyUpdateEvent = Simulator::Schedule (m_energyUpdateInterval,
                                             &THzEnergyModel::HarvestEnergy,
                                             this);
  NS_LOG_DEBUG ("node id: " << m_node->GetId () << " remaining energy: " << m_remainingEnergy << " now: " << Simulator::Now ());
}

void
THzEnergyModel::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  m_remainingEnergy = m_initialEnergy;
  HarvestEnergy ();  // start periodic update
  return;
}

void
THzEnergyModel::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
}

bool
THzEnergyModel::BookEnergy (double packetLengthTx, double packetLengthRx)
{
  NS_LOG_FUNCTION ("node id" << m_node->GetId () << " now: " << Simulator::Now ());

  double energyBook = packetLengthTx * 8 * m_energyConsumptionPulseTx * m_codingWeight
                    + packetLengthRx * 8 * m_energyConsumptionPulseRx;

  if ((m_remainingEnergy - energyBook) >= 0)
    {
      m_remainingEnergy -= energyBook;
      NS_LOG_UNCOND (m_remainingEnergy);
      NS_LOG_DEBUG ("THzEnergyModel:Remaining energy = " << m_remainingEnergy);
      return true;
    }
  NS_LOG_DEBUG ("THzEnergyModel:Remaining energy is not sufficient!" << m_remainingEnergy);
  return false;
}

void
THzEnergyModel::ReturnEnergy (double packetLengthTx, double packetLengthRx)
{
  NS_LOG_FUNCTION (this);
  double energyReturn = packetLengthTx * 8 * m_energyConsumptionPulseTx * m_codingWeight
                      + packetLengthRx * 8 * m_energyConsumptionPulseRx;
  m_remainingEnergy += energyReturn;
  NS_LOG_UNCOND (m_remainingEnergy);
  NS_LOG_DEBUG ("THzEnergyModel:Remaining energy = " << m_remainingEnergy << " now: " << Simulator::Now ());
}

} // namespace ns3
