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

#include "thz-net-device.h"

#include "thz-channel.h"
#include "thz-dir-antenna.h"
#include "thz-mac.h"
#include "thz-phy.h"

#include "ns3/assert.h"
#include "ns3/llc-snap-header.h"
#include "ns3/log.h"
#include "ns3/mac48-address.h"
#include "ns3/node.h"
#include "ns3/pointer.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/traced-callback.h"

#include <vector>

NS_LOG_COMPONENT_DEFINE("THzNetDevice");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(THzNetDevice);

THzNetDevice::THzNetDevice()
    : NetDevice(),
      m_mtu(60000),
      m_arp(true)
{
}

THzNetDevice::~THzNetDevice()
{
}

void
THzNetDevice::Clear()
{
    m_node = 0;
    if (m_mac)
    {
        m_mac->Clear();
        m_mac = 0;
    }
    if (m_phy)
    {
        m_phy->Clear();
        m_phy = 0;
    }
    if (m_channel)
    {
        m_channel->Clear();
        m_channel = 0;
    }
    if (m_dirantenna)
    {
        m_dirantenna->Clear();
        m_dirantenna = 0;
    }
}

void
THzNetDevice::DoDispose()
{
    Clear();
    NetDevice::DoDispose();
}

TypeId
THzNetDevice::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::THzNetDevice")
            .SetParent<NetDevice>()
            .AddAttribute(
                "Channel",
                "The channel attached to this device",
                PointerValue(),
                MakePointerAccessor(&THzNetDevice::DoGetChannel, &THzNetDevice::SetChannel),
                MakePointerChecker<THzChannel>())
            .AddAttribute(
                "DirAntenna",
                "The Directional Antenna attached to this device.",
                PointerValue(),
                MakePointerAccessor(&THzNetDevice::GetDirAntenna, &THzNetDevice::SetDirAntenna),
                MakePointerChecker<THzDirectionalAntenna>())
            .AddAttribute("Phy",
                          "The PHY layer attached to this device.",
                          PointerValue(),
                          MakePointerAccessor(&THzNetDevice::GetPhy, &THzNetDevice::SetPhy),
                          MakePointerChecker<THzPhy>())
            .AddAttribute("Mac",
                          "The MAC layer attached to this device.",
                          PointerValue(),
                          MakePointerAccessor(&THzNetDevice::GetMac, &THzNetDevice::SetMac),
                          MakePointerChecker<THzMac>())
            .AddTraceSource("Rx",
                            "Received payload from the MAC layer.",
                            MakeTraceSourceAccessor(&THzNetDevice::m_rxLogger),
                            "ns3::Packet::Mac48AddressTracedCallback")
            .AddTraceSource("Tx",
                            "Send payload to the MAC layer.",
                            MakeTraceSourceAccessor(&THzNetDevice::m_txLogger),
                            "ns3::Packet::Mac48AddressTracedCallback");
    return tid;
}

void
THzNetDevice::SetNode(Ptr<Node> node)
{
    m_node = node;
    NS_LOG_FUNCTION(m_node->GetId());
}

void
THzNetDevice::SetMac(Ptr<THzMac> mac)
{
    if (mac)
    {
        m_mac = mac;
        NS_LOG_DEBUG("Set MAC");

        if (m_phy)
        {
            m_phy->SetMac(m_mac);
            m_mac->AttachPhy(m_phy);
            m_mac->SetDevice(this);
            NS_LOG_DEBUG("Attached MAC to PHY");
        }
        m_mac->SetForwardUpCb(MakeCallback(&THzNetDevice::ForwardUp, this));
    }
}

void
THzNetDevice::SetPhy(Ptr<THzPhy> phy)
{
    if (phy)
    {
        m_phy = phy;
        m_phy->SetDevice(Ptr<THzNetDevice>(this));
        NS_LOG_DEBUG("Set PHY");
        if (m_mac)
        {
            m_mac->AttachPhy(phy);
            m_mac->SetDevice(this);
            m_phy->SetMac(m_mac);
            NS_LOG_DEBUG("Attached PHY to MAC");
        }
    }
}

void
THzNetDevice::SetChannel(Ptr<THzChannel> channel)
{
    if (channel)
    {
        m_channel = channel;
        NS_LOG_DEBUG("Set CHANNEL");
        if (m_phy)
        {
            m_channel->AddDevice(this, m_phy);
            m_phy->SetChannel(channel);
            NS_LOG_DEBUG("Attach CH to PHY");
        }
    }
}

void
THzNetDevice::SetDirAntenna(Ptr<THzDirectionalAntenna> dirantenna)
{
    if (dirantenna)
    {
        m_dirantenna = dirantenna;
        m_dirantenna->SetDevice(this);
        NS_LOG_DEBUG("Set DIRECTIONAL ANTENNA");
    }
}

void
THzNetDevice::SetIfIndex(uint32_t index)
{
    m_ifIndex = index;
}

void
THzNetDevice::SetAddress(Address address)
{
    m_mac->SetAddress(Mac48Address::ConvertFrom(address));
}

bool
THzNetDevice::SetMtu(uint16_t mtu)
{
    m_mtu = mtu;
    return true;
}

bool
THzNetDevice::NeedsArp() const
{
    return m_arp;
}

bool
THzNetDevice::SupportsSendFrom(void) const
{
    return false;
}

Ptr<THzChannel>
THzNetDevice::DoGetChannel(void) const
{
    return m_channel;
}

Ptr<THzMac>
THzNetDevice::GetMac() const
{
    return m_mac;
}

Ptr<THzPhy>
THzNetDevice::GetPhy() const
{
    return m_phy;
}

Ptr<THzDirectionalAntenna>
THzNetDevice::GetDirAntenna() const
{
    return m_dirantenna;
}

uint32_t
THzNetDevice::GetIfIndex() const
{
    return m_ifIndex;
}

Ptr<Channel>
THzNetDevice::GetChannel() const
{
    return m_channel;
}

Address
THzNetDevice::GetAddress() const
{
    return m_mac->GetAddress();
}

uint16_t
THzNetDevice::GetMtu() const
{
    return m_mtu;
}

bool
THzNetDevice::IsLinkUp() const
{
    return (m_linkup && m_phy);
}

bool
THzNetDevice::IsBroadcast() const
{
    return true;
}

Address
THzNetDevice::GetBroadcast() const
{
    return m_mac->GetBroadcast();
}

Ptr<Node>
THzNetDevice::GetNode() const
{
    return m_node;
}

bool
THzNetDevice::IsMulticast() const
{
    return false;
}

Address
THzNetDevice::GetMulticast(Ipv4Address multicastGroup) const
{
    NS_FATAL_ERROR("THzNetDevice does not support multicast");
    return m_mac->GetBroadcast();
}

Address
THzNetDevice::GetMulticast(Ipv6Address addr) const
{
    NS_FATAL_ERROR("THzNetDevice does not support multicast");
    return m_mac->GetBroadcast();
}

bool
THzNetDevice::IsBridge(void) const
{
    return false;
}

bool
THzNetDevice::IsPointToPoint() const
{
    return false;
}

bool
THzNetDevice::Send(Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
    NS_LOG_FUNCTION("pkt" << packet << "dest" << dest);
    NS_ASSERT(Mac48Address::IsMatchingType(dest));
    Mac48Address destAddr = Mac48Address::ConvertFrom(dest);

    LlcSnapHeader llc;
    llc.SetType(protocolNumber);
    packet->AddHeader(llc);

    if (packet->GetSize() == 60004) // fix to be able to send packets of 65535 Bytes (UDP max). For some reason it is fragmented at 60004 B
    {
        Ptr<Packet> packet2 = Create<Packet>(65000); // 8+4 : the size of the seqTs header
        packet2->AddHeader(llc);
        m_mac->Enqueue(packet2, destAddr);
        return true;
    }

    m_mac->Enqueue(packet, destAddr);
    return true;
}

bool
THzNetDevice::SendFrom(Ptr<Packet> packet,
                       const Address& src,
                       const Address& dest,
                       uint16_t protocolNumber)
{
    NS_LOG_DEBUG("SendFrom Device " << this->GetAddress());
    NS_LOG_FUNCTION(src << dest);
    NS_ASSERT(Mac48Address::IsMatchingType(dest));
    NS_ASSERT(Mac48Address::IsMatchingType(src));
    Mac48Address destAddr = Mac48Address::ConvertFrom(dest);

    LlcSnapHeader llc;
    llc.SetType(protocolNumber);
    packet->AddHeader(llc);

    m_mac->Enqueue(packet, destAddr);
    return true;
}

void
THzNetDevice::ForwardUp(Ptr<Packet> packet, Mac48Address src, Mac48Address dest)
{
    LlcSnapHeader llc;
    packet->RemoveHeader(llc);
    enum NetDevice::PacketType type;

    if (dest.IsBroadcast())
    {
        type = NetDevice::PACKET_BROADCAST;
    }
    else if (dest.IsGroup())
    {
        type = NetDevice::PACKET_MULTICAST;
    }
    else if (dest == m_mac->GetAddress())
    {
        type = NetDevice::PACKET_HOST;
    }
    else
    {
        type = NetDevice::PACKET_OTHERHOST;
    }

    if (type != NetDevice::PACKET_OTHERHOST)
    {
        m_rxLogger(packet, src);
        m_forwardUp(this, packet, llc.GetType(), src);
    }
}

void
THzNetDevice::SetReceiveCallback(NetDevice::ReceiveCallback cb)
{
    m_forwardUp = cb;
}

void
THzNetDevice::AddLinkChangeCallback(Callback<void> callback)
{
    m_linkChanges.ConnectWithoutContext(callback);
}

void
THzNetDevice::SetPromiscReceiveCallback(PromiscReceiveCallback cb)
{
    // Not implemented yet
    NS_ASSERT_MSG(0, "Not yet implemented");
}

} // namespace ns3
