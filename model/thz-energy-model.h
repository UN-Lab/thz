/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2021 Northeastern University (https://unlab.tech/)
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
 */

#ifndef THZ_ENERGY_MODEL_H
#define THZ_ENERGY_MODEL_H

#include "ns3/traced-value.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"
#include "ns3/node.h"
#include "ns3/object.h"

namespace ns3 {

class Node;

/**
 * \ingroup energy
 * \brief THzEnergyModel models the energy harvesting and consumption by THz nodes
 *
 * This class is installed in the Node by object aggregation facility provided in
 * the classes derived from NS-3 Object. It harvests fixed amount of energy at a
 * user defined interval. It also provides interfaces to consume certain amount of
 * energy. The MAC layer can use the interface to consume energy whenever it transmits
 * or receives a packet.
 */
class THzEnergyModel : public Object
{
public:
  static TypeId GetTypeId (void);
  THzEnergyModel ();
  virtual ~THzEnergyModel ();

  /**
   * Sets the node the energy model belongs to
   */
  virtual void SetNode (Ptr<Node> node);
  /**
   *
   * Setting up the callback to inform MAC layer to send Data
   */
  virtual void SetEnergyCallback (Callback<void> energyCbData);
  /**
   * \return Initial energy stored in energy source, in Joules.
   *
   * Implements GetInitialEnergy.
   */
  virtual double GetInitialEnergy (void) const;

  /**
   * \return Remaining energy in energy source, in Joules
   *
   * Implements GetRemainingEnergy.
   */
  virtual double GetRemainingEnergy (void);

  /**
   * This function perodically adds harvested energy to remaining energy.
   */
  virtual void HarvestEnergy (void);
  /**
   * \brief Books energy for complete transmission process
   *
   * \param packetLengthTx The length of the packet being transmitted in bytes.
   *
   * \param packetLengthRx The length of the packet being received in bytes.
   *
   * \return true if the amount of requested energy is available.
   */
  bool BookEnergy (double packetLengthTx, double packetLengthRx);
  /**
   * \brief Returns unused energy of the booked energy
   *
   * \param packetLengthTx The length of the packet energy was booked for in bytes.
   *
   * \param packetLengthRx The length of the packet energy was booked for in bytes.
   *
   */
  void ReturnEnergy (double packetLengthTx, double packetLengthRx);

  /**
   * \param interval Energy update interval.
   *
   * This function sets the interval between each energy update.
   */
  void SetEnergyUpdateInterval (Time interval);

  /**
   * \returns The interval between each energy update.
   */
  Time GetEnergyUpdateInterval (void) const;


private:
  /// Defined in ns3::Object
  void DoInitialize (void);

  /// Defined in ns3::Object
  void DoDispose (void);


private:
  Ptr<Node> m_node;                       //!< Node attached to this energy model.
  double m_initialEnergy;                 //!< initial energy, in frames

  double m_energyHarvestingAmount;        //!< amount of energy harvested each time
  double m_energyConsumptionPulseTx;      //!< amount of energy consumed for transmission of a pulse in frames
  double m_energyConsumptionPulseRx;      //!< amount of energy consumed for reception of a pulse in frames
  double m_codingWeight;                  //!< Percentage of transmitting a pulse instead of being silent

  double m_dataCallbacklEnergy;           //!< remaining energy, in frames
  TracedValue<double> m_remainingEnergy;  //!< remaining energy, in frames
  EventId m_energyUpdateEvent;            //!< energy update event
  Time m_energyUpdateInterval;            //!< energy update interval

  Callback<void>      m_energyCbData;     //!<informs MAC when energy level reaches certain threshold
};

} // namespace ns3

#endif /* BASIC_ENERGY_SOURCE_H */
