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
 *         Daniel Morales <danimoralesbrotons@gmail.com>
 */

#include "thz-mac-header.h"

#include "ns3/address-utils.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE("THzMacHeader");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(THzMacHeader);

THzMacHeader::THzMacHeader()
{
}

THzMacHeader::THzMacHeader(const Mac48Address srcAddr, const Mac48Address dstAddr, uint8_t type)
    : Header(),
      m_srcAddr(srcAddr),
      m_dstAddr(dstAddr),
      m_type(type)
{
}

TypeId
THzMacHeader::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::THzMacHeader").SetParent<Header>().AddConstructor<THzMacHeader>();
    return tid;
}

TypeId
THzMacHeader::GetInstanceTypeId(void) const
{
    return GetTypeId();
}

THzMacHeader::~THzMacHeader()
{
}

void
THzMacHeader::SetSource(Mac48Address addr)
{
    m_srcAddr = addr;
}

void
THzMacHeader::SetDestination(Mac48Address addr)
{
    m_dstAddr = addr;
}

void
THzMacHeader::SetType(uint8_t type)
{
    m_type = type;
}

void
THzMacHeader::SetRetry(uint8_t retry)
{
    m_retry = retry;
}

/*
--- Flag values (RTS/CTS/CTA) ---
  0 (CTS/RTS/CTA):  Nothing to indicate. Normal packet

  -- Beam sounding --
  1 (CTA):  CTA requests RTS (dummy) from all nodes.
  1 (RTS):  Dummy RTS
  2 (CTA):  Feedback CTA announcing sector assigned

  -- Adaptive MCS --
  10 (CTS/RTS): BPSK
  11 (CTS/RTS): QPSK
  12 (CTS/RTS): 8-PSK
  13 (CTS/RTS): 16-QAM
  14 (CTS/RTS): 64-QAM
*/
void
THzMacHeader::SetFlags(uint16_t flags)
{
    m_flags = flags;
}

void
THzMacHeader::SetSector(uint16_t sector)
{
    m_sector = sector;
}

void
THzMacHeader::SetDuration(Time duration)
{
    int64_t duration_ns = duration.GetNanoSeconds();
    m_duration = static_cast<uint16_t>(duration_ns);
}

void
THzMacHeader::SetSequence(uint16_t seq)
{
    m_sequence = seq;
}

Mac48Address
THzMacHeader::GetSource(void) const
{
    return m_srcAddr;
}

Mac48Address
THzMacHeader::GetDestination(void) const
{
    return m_dstAddr;
}

uint8_t
THzMacHeader::GetType(void) const
{
    return m_type;
}

uint8_t
THzMacHeader::GetRetry(void) const
{
    return m_retry;
}

uint16_t
THzMacHeader::GetFlags(void) const
{
    return m_flags;
}

uint16_t
THzMacHeader::GetSector(void) const
{
    return m_sector;
}

Time
THzMacHeader::GetDuration(void) const
{
    return NanoSeconds(m_duration);
}

uint32_t
THzMacHeader::GetSize(void) const
{
    uint32_t size = 0;
    switch (m_type)
    {
    case THZ_PKT_TYPE_CTA:
        size = sizeof(m_type) + sizeof(m_sector) + sizeof(m_flags) + sizeof(Mac48Address) * 2;
        break;
    case THZ_PKT_TYPE_RTS:
        size = sizeof(m_type) + sizeof(m_flags) + sizeof(m_retry) + sizeof(Mac48Address) * 2 + sizeof(m_sequence);
        break;
    case THZ_PKT_TYPE_CTS:
        size = sizeof(m_type) + sizeof(m_flags) + sizeof(m_duration) + sizeof(Mac48Address) * 2 + sizeof(m_sequence);
        break;
    case THZ_PKT_TYPE_ACK:
        size = sizeof(m_type) + sizeof(m_duration) + sizeof(Mac48Address) * 2 + sizeof(m_sequence);
        break;
    case THZ_PKT_TYPE_DATA:
        size = sizeof(m_type) + sizeof(m_duration) + sizeof(Mac48Address) * 2 + sizeof(m_sequence);
        break;
    }
    return size;
}

uint16_t
THzMacHeader::GetSequence(void) const
{
    return m_sequence;
}

// Inherrited methods

uint32_t
THzMacHeader::GetSerializedSize(void) const
{
    return GetSize();
}

void
THzMacHeader::Serialize(Buffer::Iterator i) const
{
    i.WriteU8(m_type);
    switch (m_type)
    {
    case THZ_PKT_TYPE_CTA:
        i.WriteU16(m_sector);
        i.WriteU16(m_flags);
        WriteTo(i, m_srcAddr);
        WriteTo(i, m_dstAddr);
        break;
    case THZ_PKT_TYPE_RTS:
        i.WriteU16(m_flags);
        i.WriteU8(m_retry);
        WriteTo(i, m_srcAddr);
        WriteTo(i, m_dstAddr);
        i.WriteU16(m_sequence);
        break;
    case THZ_PKT_TYPE_CTS:
        i.WriteU16(m_flags);
        i.WriteHtolsbU16(m_duration);
        WriteTo(i, m_srcAddr);
        WriteTo(i, m_dstAddr);
        i.WriteU16(m_sequence);
        break;
    case THZ_PKT_TYPE_ACK:
        i.WriteHtolsbU16(m_duration);
        WriteTo(i, m_srcAddr);
        WriteTo(i, m_dstAddr);
        i.WriteU16(m_sequence);
        break;
    case THZ_PKT_TYPE_DATA:
        i.WriteHtolsbU16(m_duration);
        WriteTo(i, m_srcAddr);
        WriteTo(i, m_dstAddr);
        i.WriteU16(m_sequence);
        break;
    }
}

uint32_t
THzMacHeader::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;

    m_type = i.ReadU8();
    switch (m_type)
    {
    case THZ_PKT_TYPE_CTA:
        m_sector = i.ReadU16();
        m_flags = i.ReadU16();
        ReadFrom(i, m_srcAddr);
        ReadFrom(i, m_dstAddr);
        break;
    case THZ_PKT_TYPE_RTS:
        m_flags = i.ReadU16();
        m_retry = i.ReadU8();
        ReadFrom(i, m_srcAddr);
        ReadFrom(i, m_dstAddr);
        m_sequence = i.ReadU16();
        break;
    case THZ_PKT_TYPE_CTS:
        m_flags = i.ReadU16();
        m_duration = i.ReadLsbtohU16();
        ReadFrom(i, m_srcAddr);
        ReadFrom(i, m_dstAddr);
        m_sequence = i.ReadU16();
        break;
    case THZ_PKT_TYPE_ACK:
        m_duration = i.ReadLsbtohU16();
        ReadFrom(i, m_srcAddr);
        ReadFrom(i, m_dstAddr);
        m_sequence = i.ReadU16();
        break;
    case THZ_PKT_TYPE_DATA:
        m_duration = i.ReadLsbtohU16();
        ReadFrom(i, m_srcAddr);
        ReadFrom(i, m_dstAddr);
        m_sequence = i.ReadU16();
        break;
    }

    return i.GetDistanceFrom(start);
}

void
THzMacHeader::Print(std::ostream& os) const
{
    os << "THZ src=" << m_srcAddr << " dest=" << m_dstAddr << " type=" << (uint32_t)m_type;
}

} // namespace ns3
