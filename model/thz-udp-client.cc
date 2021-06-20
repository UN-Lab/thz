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
 *         Daniel Morales <danimoralesbrotons@gmail.com>
 */


#include "ns3/double.h"
#include "ns3/random-variable-stream.h"
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
#include "ns3/thz-udp-client.h"
#include "ns3/seq-ts-header.h"
#include <cstdlib>
#include <cstdio>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("THzUdpClient")
;
NS_OBJECT_ENSURE_REGISTERED (THzUdpClient)
;

TypeId
THzUdpClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::THzUdpClient")
    .SetParent<Application> ()
    .AddConstructor<THzUdpClient> ()

    .AddAttribute ("RemoteAddress",
                   "The destination Address of the outbound packets",
                   AddressValue (),
                   MakeAddressAccessor (&THzUdpClient::m_peerAddress),
                   MakeAddressChecker ())
    .AddAttribute ("RemotePort",
                   "The destination port of the outbound packets",
                   UintegerValue (100),
                   MakeUintegerAccessor (&THzUdpClient::m_peerPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PacketSize",
                   "Size of packets generated. The minimum packet size is 12 bytes which is the size of the header carrying the sequence number and the time stamp.",
                   UintegerValue (1024),
                   MakeUintegerAccessor (&THzUdpClient::m_size),
                   MakeUintegerChecker<uint32_t> (12,2000000))
    .AddAttribute ("Mean",
                   "The mean delay between two packets (s)",
                   DoubleValue (500.0),
                   MakeDoubleAccessor (&THzUdpClient::m_mean),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

THzUdpClient::THzUdpClient ()
{
  NS_LOG_FUNCTION (this);
  m_sent = 0;
  m_socket = 0;
  m_sendEvent = EventId ();
}

THzUdpClient::~THzUdpClient ()
{
  NS_LOG_FUNCTION (this);
}

void
THzUdpClient::SetRemote (Ipv4Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = Address (ip);
  m_peerPort = port;
}

void
THzUdpClient::SetRemote (Ipv6Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = Address (ip);
  m_peerPort = port;
}

void
THzUdpClient::SetRemote (Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = ip;
  m_peerPort = port;
}

void
THzUdpClient::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
THzUdpClient::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      if (Ipv4Address::IsMatchingType (m_peerAddress) == true)
        {
          m_socket->Bind (); //Allocate a local Ipv4 endpoint for this socket
          m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom (m_peerAddress), m_peerPort)); //Initiate a connection to a remote host
        }
      else if (Ipv6Address::IsMatchingType (m_peerAddress) == true)
        {
          m_socket->Bind6 (); //Allocate a local Ipv6 endpoint for this socket
          m_socket->Connect (Inet6SocketAddress (Ipv6Address::ConvertFrom (m_peerAddress), m_peerPort)); //Initiate a connection to a remote host
        }
    }

  // TO AVOID INITIAL TRANSITORY PHASE
  Ptr<ExponentialRandomVariable> x = CreateObject<ExponentialRandomVariable> ();
  x->SetAttribute ("Mean", DoubleValue (m_mean));
  x->SetAttribute ("Bound", DoubleValue (std::max(1000.0, m_mean*3)));
  m_delay = MicroSeconds (x->GetValue ());
  NS_LOG_UNCOND ("Generate first packet after " << m_delay);

  m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ()); //Notify application when new data is available to be read. This callback is intended to notify a socket that would have been blocked in a blocking socket model that data is available to be read.
  m_sendEvent = Simulator::Schedule (m_delay, &THzUdpClient::Send, this);
/*
  m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ()); //Notify application when new data is available to be read. This callback is intended to notify a socket that would have been blocked in a blocking socket model that data is available to be read.
  m_sendEvent = Simulator::Schedule (Seconds(0.0), &THzUdpClient::Send, this);
  */
}

void
THzUdpClient::StopApplication (void)
{
  NS_LOG_FUNCTION (this);
  Simulator::Cancel (m_sendEvent);
}

void
THzUdpClient::Send (void)
{
  NS_LOG_FUNCTION (this);
  SeqTsHeader seqTs; //Packet header for UDP client/server application
  seqTs.SetSeq (m_sent); //seq:(m_sent) the sequence number
  Ptr<Packet> p = Create<Packet> (m_size - (8 + 4)); // 8+4 : the size of the seqTs header
  p->AddHeader (seqTs);
  //NS_LOG_UNCOND("At UDP Client, packet created with size " << p->GetSize());
  std::stringstream peerAddressStringStream;
  if (Ipv4Address::IsMatchingType (m_peerAddress))
    {
      peerAddressStringStream << Ipv4Address::ConvertFrom (m_peerAddress);
    }
  else if (Ipv6Address::IsMatchingType (m_peerAddress))
    {
      peerAddressStringStream << Ipv6Address::ConvertFrom (m_peerAddress);
    }

  if ((m_socket->Send (p)) >= 0) //Send data (or dummy data) to the remote host.
    {
      Ptr<ExponentialRandomVariable> x = CreateObject<ExponentialRandomVariable> ();
      x->SetAttribute ("Mean", DoubleValue (m_mean));
      //x->SetAttribute ("Bound", DoubleValue (std::max(1000.0, m_mean*3)));  // Be careful: bounding shifts the mean to a lower value!
      m_delay = MicroSeconds (x->GetValue ());
      NS_LOG_INFO ("Generate next packet after " << m_delay);
      m_sendEvent = Simulator::Schedule (m_delay, &THzUdpClient::Send, this); //schedule next send

      NS_LOG_INFO ("from node " << GetNode ()->GetId () << " TraceDelay TX " << m_size << " bytes to "
                                << peerAddressStringStream.str () << " Uid: "
                                << p->GetUid () << " Time: "
                                << (Simulator::Now ()).GetSeconds ());

    }
  else
    {
      NS_LOG_INFO ("Error while sending " << m_size << " bytes to "
                                          << peerAddressStringStream.str ());
    }

}

} // Namespace ns3
