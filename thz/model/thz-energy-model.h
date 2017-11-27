/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 UBNANO
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
   * \param amount The amount of energy to be booked.
   *
   * \return true if the amount of requested energy is available.
   */
  bool BookEnergy (double amount);
  /**
   * \brief Returns unused energy of the booked energy
   *
   * \param amount the amount of energy returned in energy frames.
   *
   */
  void ReturnEnergy (double amount);

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
  double m_initialEnergy;                 //!< initial energy, in Joules
  double m_energyTxJ;                     //!< energy consumption for transmitting one packet, in Joules
  double m_energyRxJ;                     //!< energy consumption while receiving one packet, in Joules
  double m_energyHarvestingAmount;        //!< amount of energy harvested each time
  double m_dataCallbacklEnergy;
  TracedValue<double> m_remainingEnergy;  //!< remaining energy, in Joules
  EventId m_energyUpdateEvent;            //!< energy update event
  Time m_energyUpdateInterval;            //!< energy update interval

  Callback<void>      m_energyCbData;     //!<informs MAC when energy level reaches certain threshold
};

} // namespace ns3

#endif /* BASIC_ENERGY_SOURCE_H */
