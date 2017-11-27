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


#ifndef TRAFFIC_GENERATOR_H
#define TRAFFIC_GENERATOR_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"
#include "ns3/node-container.h"
#include "ns3/nstime.h"
#include "ns3/node.h"

namespace ns3 {

class Socket;
class Packet;
class THzEnergyModel;
/**
 * \ingroup thz
 * \brief A random traffic generator
 *
 * This class generates packets for a random node.
 * Each destination node is selected uniformly and
 * the delay between consecutive packets follows
 * exponential distribution
 */

class TrafficGenerator : public Application
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  TrafficGenerator ();
  virtual ~TrafficGenerator ();

  void AddNodeContainer (NodeContainer c);

protected:
  virtual void DoDispose (void);
private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);
  /**
   * \brief Schedules the generation of a packet
   */
  void DoGenerate (void);
  /**
   * \brief Generates a packet and sends to
   * a random destination node
   */
  void Generate (void);
  /**
   * \brief Handle a packet reception.
   *
   * This function is called by lower layers.
   *
   * \param socket the socket the packet was received to.
   */
  void HandleRead (Ptr<Socket> socket);

  Time m_delay; //!< Packet inter-send time
  double m_mean; //!< Mean inter-send time
  uint32_t m_size; //!< Size of the sent packet
  NodeContainer m_nodes; //!< All nodes
  Ptr<Socket> m_socket; //!< Socket
  EventId m_sendEvent; //!< Event to send the next packet

};

} //namespace ns3
#endif
