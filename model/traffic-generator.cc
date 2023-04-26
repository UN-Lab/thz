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

#include "traffic-generator.h"

#include "thz-energy-model.h"

#include "ns3/assert.h"
#include "ns3/double.h"
#include "ns3/ipv4.h"
#include "ns3/log.h"
#include "ns3/mac48-address.h"
#include "ns3/packet.h"
#include "ns3/random-variable-stream.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"

NS_LOG_COMPONENT_DEFINE("TrafficGenerator");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(TrafficGenerator);

TypeId
TrafficGenerator::GetTypeId(void)
{
    static TypeId tid = TypeId("TrafficGenerator")
                            .SetParent<Application>()
                            .AddConstructor<TrafficGenerator>()
                            .AddAttribute("Mean",
                                          "The mean delay between two packets (s)",
                                          DoubleValue(500.0),
                                          MakeDoubleAccessor(&TrafficGenerator::m_mean),
                                          MakeDoubleChecker<double>())
                            .AddAttribute("PacketSize",
                                          "The size of each packet (bytes)",
                                          UintegerValue(128),
                                          MakeUintegerAccessor(&TrafficGenerator::m_size),
                                          MakeUintegerChecker<uint32_t>());
    return tid;
}

TrafficGenerator::TrafficGenerator()
{
    NS_LOG_FUNCTION(this);

    m_socket = 0;
    m_sendEvent = EventId();
}

TrafficGenerator::~TrafficGenerator()
{
    NS_LOG_FUNCTION(this);
}

void
TrafficGenerator::AddNodeContainer(NodeContainer c)
{
    NS_LOG_FUNCTION("");
    m_nodes = c;
}

void
TrafficGenerator::DoGenerate(void)
{
    Ptr<ExponentialRandomVariable> x = CreateObject<ExponentialRandomVariable>();
    x->SetAttribute("Mean", DoubleValue(m_mean));
    x->SetAttribute("Bound", DoubleValue(0.0));
    m_delay = MicroSeconds(x->GetValue());
    NS_LOG_INFO("delay" << m_delay);
    m_sendEvent = Simulator::Schedule(m_delay, &TrafficGenerator::Generate, this);
}

void
TrafficGenerator::Generate(void)
{
    Ptr<UniformRandomVariable> y = CreateObject<UniformRandomVariable>();
    uint32_t nodeIndex = y->GetInteger(0, m_nodes.GetN() - 1);
    if (nodeIndex == GetNode()->GetId())
    {
        Generate();
    }
    else
    {
        NS_LOG_INFO("Selected Node Index" << nodeIndex);
        Ptr<Ipv4> ipv4 = m_nodes.Get(nodeIndex)->GetObject<Ipv4>();
        Ipv4InterfaceAddress iaddr = ipv4->GetAddress(1, 0);
        Ipv4Address remote = iaddr.GetLocal();

        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);
        m_socket->Bind();
        m_socket->ShutdownRecv();
        m_socket->Connect(InetSocketAddress(remote));

        Ptr<Packet> p = Create<Packet>(m_size);
        m_socket->SetRecvCallback(MakeCallback(&TrafficGenerator::HandleRead, this));
        m_socket->Send(p);
        Simulator::ScheduleNow(&TrafficGenerator::DoGenerate, this);
    }
}

/*
 * Private functions start here.
 */

void
TrafficGenerator::DoDispose(void)
{
    NS_LOG_FUNCTION(this);
    Application::DoDispose();
}

void
TrafficGenerator::StartApplication(void)
{
    NS_LOG_FUNCTION(this);
    Ptr<UniformRandomVariable> y = CreateObject<UniformRandomVariable>();
    uint32_t st = y->GetInteger(0, 1);
    m_sendEvent =
        Simulator::Schedule(MicroSeconds((double)st), &TrafficGenerator::DoGenerate, this);
}

void
TrafficGenerator::StopApplication(void)
{
    NS_LOG_FUNCTION(this);
    Simulator::Cancel(m_sendEvent);
    if (m_socket)
    {
        m_socket->Close();
        m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
        m_socket = 0;
    }
}

void
TrafficGenerator::HandleRead(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    Ptr<Packet> packet;
    Address from;
    packet = socket->RecvFrom(from);
    NS_LOG_INFO("At time " << Simulator::Now().GetSeconds() << "s client received "
                           << packet->GetSize() << " bytes from "
                           << InetSocketAddress::ConvertFrom(from).GetIpv4());
}

} // namespace ns3
