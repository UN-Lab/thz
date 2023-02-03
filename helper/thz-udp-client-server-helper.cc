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

#include "ns3/thz-udp-client-server-helper.h"
#include "ns3/thz-udp-server.h"
#include "ns3/thz-udp-client.h"
#include "ns3/thz-udp-trace-client.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"

namespace ns3 {

THzUdpServerHelper::THzUdpServerHelper ()
{
}

THzUdpServerHelper::THzUdpServerHelper (uint16_t port)
{
  m_factory.SetTypeId (THzUdpServer::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
}

void
THzUdpServerHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
THzUdpServerHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;

      m_server = m_factory.Create<THzUdpServer> ();
      node->AddApplication (m_server);
      apps.Add (m_server);

    }
  return apps;
}

Ptr<THzUdpServer>
THzUdpServerHelper::GetServer (void)
{
  return m_server;
}

THzUdpClientHelper::THzUdpClientHelper ()
{
}

THzUdpClientHelper::THzUdpClientHelper (Address address, uint16_t port)
{
  m_factory.SetTypeId (THzUdpClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
}

THzUdpClientHelper::THzUdpClientHelper (Ipv4Address address, uint16_t port)
{
  m_factory.SetTypeId (THzUdpClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (Address (address)));
  SetAttribute ("RemotePort", UintegerValue (port));
}

THzUdpClientHelper::THzUdpClientHelper (Ipv6Address address, uint16_t port)
{
  m_factory.SetTypeId (THzUdpClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (Address (address)));
  SetAttribute ("RemotePort", UintegerValue (port));
}

void
THzUdpClientHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
THzUdpClientHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<THzUdpClient> client = m_factory.Create<THzUdpClient> ();
      node->AddApplication (client);
      apps.Add (client);
    }
  return apps;
}

THzUdpTraceClientHelper::THzUdpTraceClientHelper ()
{
}

THzUdpTraceClientHelper::THzUdpTraceClientHelper (Address address, uint16_t port, std::string filename)
{
  m_factory.SetTypeId (THzUdpTraceClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
  SetAttribute ("TraceFilename", StringValue (filename));
}

THzUdpTraceClientHelper::THzUdpTraceClientHelper (Ipv4Address address, uint16_t port, std::string filename)
{
  m_factory.SetTypeId (THzUdpTraceClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (Address (address)));
  SetAttribute ("RemotePort", UintegerValue (port));
  SetAttribute ("TraceFilename", StringValue (filename));
}

THzUdpTraceClientHelper::THzUdpTraceClientHelper (Ipv6Address address, uint16_t port, std::string filename)
{
  m_factory.SetTypeId (THzUdpTraceClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (Address (address)));
  SetAttribute ("RemotePort", UintegerValue (port));
  SetAttribute ("TraceFilename", StringValue (filename));
}

void
THzUdpTraceClientHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
THzUdpTraceClientHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<THzUdpTraceClient> client = m_factory.Create<THzUdpTraceClient> ();
      node->AddApplication (client);
      apps.Add (client);
    }
  return apps;
}

} // namespace ns3
