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
 */

#ifndef THZ_UDP_CLIENT_SERVER_HELPER_H
#define THZ_UDP_CLIENT_SERVER_HELPER_H

#include <stdint.h>
#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/thz-udp-server.h"
#include "ns3/thz-udp-client.h"
namespace ns3 {
/**
 * \ingroup thzudpclientserver
 * \brief Create a server application which waits for input UDP packets
 *        and uses the information carried into their payload to compute
 *        delay and to determine if some packets are lost.
 */
class THzUdpServerHelper
{
public:
  /**
   * Create UdpServerHelper which will make life easier for people trying
   * to set up simulations with udp-client-server application.
   *
   */
  THzUdpServerHelper ();

  /**
   * Create UdpServerHelper which will make life easier for people trying
   * to set up simulations with udp-client-server application.
   *
   * \param port The port the server will wait on for incoming packets
   */
  THzUdpServerHelper (uint16_t port);

  /**
   * Record an attribute to be set in each Application after it is is created.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);

  /**
   * Create one UDP server application on each of the Nodes in the
   * NodeContainer.
   *
   * \param c The nodes on which to create the Applications.  The nodes
   *          are specified by a NodeContainer.
   * \returns The applications created, one Application per Node in the
   *          NodeContainer.
   */
  ApplicationContainer Install (NodeContainer c);

  /**
   * \brief Return the last created server.
   *
   * This function is mainly used for testing.
   *
   * \returns a Ptr to the last created server application
   */
  Ptr<THzUdpServer> GetServer (void);
private:
  ObjectFactory m_factory; //!< Object factory.
  Ptr<THzUdpServer> m_server; //!< The last created server application
};

/**
 * \ingroup udpclientserver
 * \brief Create a client application which sends UDP packets carrying
 *  a 32bit sequence number and a 64 bit time stamp.
 *
 */
class THzUdpClientHelper
{

public:
  /**
   * Create THzUdpClientHelper which will make life easier for people trying
   * to set up simulations with udp-client-server.
   *
   */
  THzUdpClientHelper ();

  /**
   *  Create THzUdpClientHelper which will make life easier for people trying
   * to set up simulations with thz-udp-client-server.
   *
   * \param ip The IPv4 address of the remote UDP server
   * \param port The port number of the remote UDP server
   */

  THzUdpClientHelper (Ipv4Address ip, uint16_t port);
  /**
   *  Create THzUdpClientHelper which will make life easier for people trying
   * to set up simulations with thz-udp-client-server.
   *
   * \param ip The IPv6 address of the remote UDP server
   * \param port The port number of the remote UDP server
   */

  THzUdpClientHelper (Ipv6Address ip, uint16_t port);
  /**
   *  Create THzUdpClientHelper which will make life easier for people trying
   * to set up simulations with thz-udp-client-server.
   *
   * \param ip The IP address of the remote UDP server
   * \param port The port number of the remote UDP server
   */

  THzUdpClientHelper (Address ip, uint16_t port);

  /**
   * Record an attribute to be set in each Application after it is is created.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);

  /**
     * \param c the nodes
     *
     * Create one UDP client application on each of the input nodes
     *
     * \returns the applications created, one application per input node.
     */
  ApplicationContainer Install (NodeContainer c);

private:
  ObjectFactory m_factory; //!< Object factory.
};
/**
 * \ingroup thzudpclientserver
 * Create THzUdpTraceClient application which sends UDP packets based on a trace
 * file of an MPEG4 stream. Trace files could be downloaded form :
 * http://www2.tkn.tu-berlin.de/research/trace/ltvt.html (the 2 first lines of
 * the file should be removed)
 * A valid trace file is a file with 4 columns:
 * \li -1- the first one represents the frame index
 * \li -2- the second one indicates the type of the frame: I, P or B
 * \li -3- the third one indicates the time on which the frame was generated by the encoder
 * \li -4- the fourth one indicates the frame size in byte
*/
class THzUdpTraceClientHelper
{
public:
  /**
   * Create THzUdpTraceClientHelper which will make life easier for people trying
   * to set up simulations with thz-udp-client-server.
   *
   */
  THzUdpTraceClientHelper ();

  /**
   * Create THzUdpTraceClientHelper which will make life easier for people trying
   * to set up simulations with thz-udp-client-server.
   *
   * \param ip The IP address of the remote UDP server
   * \param port The port number of the remote UDP server
   * \param filename the file from which packet traces will be loaded
   */
  THzUdpTraceClientHelper (Address ip, uint16_t port, std::string filename);
  /**
   * Create THzUdpTraceClientHelper which will make life easier for people trying
   * to set up simulations with thz-udp-client-server.
   *
   * \param ip The IPv4 address of the remote UDP server
   * \param port The port number of the remote UDP server
   * \param filename the file from which packet traces will be loaded
   */
  THzUdpTraceClientHelper (Ipv4Address ip, uint16_t port, std::string filename);
  /**
   * Create THzUdpTraceClientHelper which will make life easier for people trying
   * to set up simulations with thz-udp-client-server.
   *
   * \param ip The IPv6 address of the remote UDP server
   * \param port The port number of the remote UDP server
   * \param filename the file from which packet traces will be loaded
   */
  THzUdpTraceClientHelper (Ipv6Address ip, uint16_t port, std::string filename);

  /**
    * Record an attribute to be set in each Application after it is is created.
    *
    * \param name the name of the attribute to set
    * \param value the value of the attribute to set
    */
  void SetAttribute (std::string name, const AttributeValue &value);

  /**
    * \param c the nodes
    *
    * Create one UDP trace client application on each of the input nodes
    *
    * \returns the applications created, one application per input node.
    */
  ApplicationContainer Install (NodeContainer c);

private:
  ObjectFactory m_factory; //!< Object factory.
};

} // namespace ns3

#endif /* THZ_UDP_CLIENT_SERVER_H */
