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

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/packet-loss-counter.h"

#include "ns3/seq-ts-header.h"
#include "ns3/thz-udp-server.h"

#include "ns3/thz-directional-antenna-helper.h"
#include "ns3/thz-dir-antenna.h"
#include "ns3/mobility-model.h"
#include "ns3/mobility-helper.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("THzUdpServer")
;
NS_OBJECT_ENSURE_REGISTERED (THzUdpServer)
;


TypeId
THzUdpServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::THzUdpServer")
    .SetParent<Application> ()
    .AddConstructor<THzUdpServer> ()
    .AddAttribute ("Port",
                   "Port on which we listen for incoming packets.",
                   UintegerValue (100),
                   MakeUintegerAccessor (&THzUdpServer::m_port),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PacketWindowSize",
                   "The size of the window used to compute the packet loss. This value should be a multiple of 8.",
                   UintegerValue (32),
                   MakeUintegerAccessor (&THzUdpServer::GetPacketWindowSize,
                                         &THzUdpServer::SetPacketWindowSize),
                   MakeUintegerChecker<uint16_t> (8,256))
  ;
  return tid;
}

THzUdpServer::THzUdpServer ()
  : m_lossCounter (0)
{
  NS_LOG_FUNCTION (this);
  m_received = 0;
}

THzUdpServer::~THzUdpServer ()
{
  NS_LOG_FUNCTION (this);
}

uint16_t
THzUdpServer::GetPacketWindowSize () const
{
  NS_LOG_FUNCTION (this);
  return m_lossCounter.GetBitMapSize ();
}

void
THzUdpServer::SetPacketWindowSize (uint16_t size)
{
  NS_LOG_FUNCTION (this << size);
  m_lossCounter.SetBitMapSize (size);
}

uint32_t
THzUdpServer::GetLost (void) const
{
  NS_LOG_FUNCTION (this);
  return m_lossCounter.GetLost ();
}

uint32_t
THzUdpServer::GetReceived (void) const
{
  NS_LOG_FUNCTION (this);
  return m_received;
}

void
THzUdpServer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
THzUdpServer::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  if (!m_socket)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);

      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (),
                                                   m_port);
      m_socket->Bind (local);
    }

  m_socket->SetRecvCallback (MakeCallback (&THzUdpServer::HandleRead, this));

  if (!m_socket6)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket6 = Socket::CreateSocket (GetNode (), tid);
      Inet6SocketAddress local = Inet6SocketAddress (Ipv6Address::GetAny (),
                                                     m_port);
      m_socket6->Bind (local);
    }

  m_socket6->SetRecvCallback (MakeCallback (&THzUdpServer::HandleRead, this));

}

void
THzUdpServer::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket)
    {
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}

void
THzUdpServer::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {
      if (packet->GetSize () > 0)
        {
          SeqTsHeader seqTs;
          packet->RemoveHeader (seqTs);
          uint32_t currentSequenceNumber = seqTs.GetSeq ();
          if (InetSocketAddress::IsMatchingType (from))
            {
              NS_LOG_INFO ("TraceDelay: RX " << packet->GetSize () <<
                           " bytes from " << InetSocketAddress::ConvertFrom (from).GetIpv4 () <<
                           " Sequence Number: " << currentSequenceNumber <<
                           " Uid: " << packet->GetUid () <<
                           " TXtime: " << seqTs.GetTs () <<
                           " RXtime: " << Simulator::Now () <<
                           " Delay: " << Simulator::Now () - seqTs.GetTs ());
            }
          else if (Inet6SocketAddress::IsMatchingType (from))
            {
              NS_LOG_INFO ("TraceDelay: RX " << packet->GetSize () <<
                           " bytes from " << Inet6SocketAddress::ConvertFrom (from).GetIpv6 () <<
                           " Sequence Number: " << currentSequenceNumber <<
                           " Uid: " << packet->GetUid () <<
                           " TXtime: " << seqTs.GetTs () <<
                           " RXtime: " << Simulator::Now () <<
                           " Delay: " << Simulator::Now () - seqTs.GetTs ());
            }

          m_lossCounter.NotifyReceived (currentSequenceNumber);
          m_received++;
        }
    }
}

} // Namespace ns3
