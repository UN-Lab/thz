/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 University at Buffalo, the State University of New York
 * (http://ubnano.tech/)
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

#include "ns3/address-utils.h"
#include "thz-mac-header.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("THzMacHeader");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (THzMacHeader);

THzMacHeader::THzMacHeader ()
{
}

THzMacHeader::THzMacHeader (const Mac48Address srcAddr, const Mac48Address dstAddr, uint8_t type)
  : Header (),
    m_srcAddr (srcAddr),
    m_dstAddr (dstAddr),
    m_type (type)
{
}

TypeId
THzMacHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::THzMacHeader")
    .SetParent<Header> ()
    .AddConstructor<THzMacHeader> ()
  ;
  return tid;
}

TypeId
THzMacHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

THzMacHeader::~THzMacHeader ()
{
}

void
THzMacHeader::SetSource (Mac48Address addr)
{
  m_srcAddr = addr;
}
void
THzMacHeader::SetDestination (Mac48Address addr)
{
  m_dstAddr = addr;
}
void
THzMacHeader::SetType (uint8_t type)
{
  m_type = type;
}
void
THzMacHeader::SetDuration (Time duration)
{
  int64_t duration_us = duration.GetMicroSeconds ();
  m_duration = static_cast<uint16_t> (duration_us);
}
void
THzMacHeader::SetSequence (uint16_t seq)
{
  m_sequence = seq;
}

Mac48Address
THzMacHeader::GetSource (void) const
{
  return m_srcAddr;
}
Mac48Address
THzMacHeader::GetDestination (void) const
{
  return m_dstAddr;
}
uint8_t
THzMacHeader::GetType (void) const
{
  return m_type;
}
Time
THzMacHeader::GetDuration (void) const
{
  return MicroSeconds (m_duration);
}
uint32_t
THzMacHeader::GetSize (void) const
{
  uint32_t size = 0;
  switch (m_type)
    {
    case THZ_PKT_TYPE_RTS:
    case THZ_PKT_TYPE_CTS:
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
THzMacHeader::GetSequence (void) const
{
  return m_sequence;
}

// Inherrited methods

uint32_t
THzMacHeader::GetSerializedSize (void) const
{
  return GetSize ();
}

void
THzMacHeader::Serialize (Buffer::Iterator i) const
{
  i.WriteU8 (m_type);
  i.WriteHtolsbU16 (m_duration);
  switch (m_type)
    {
    case THZ_PKT_TYPE_RTS:
    case THZ_PKT_TYPE_CTS:
    case THZ_PKT_TYPE_ACK:
      WriteTo (i, m_srcAddr);
      WriteTo (i, m_dstAddr);
      i.WriteU16 (m_sequence);
      break;
    case THZ_PKT_TYPE_DATA:
      WriteTo (i, m_srcAddr);
      WriteTo (i, m_dstAddr);
      i.WriteU16 (m_sequence);
      break;
    }
}

uint32_t
THzMacHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_type = i.ReadU8 ();
  m_duration = i.ReadLsbtohU16 ();
  switch (m_type)
    {
    case THZ_PKT_TYPE_RTS:
    case THZ_PKT_TYPE_CTS:
    case THZ_PKT_TYPE_ACK:
      ReadFrom (i, m_srcAddr);
      ReadFrom (i, m_dstAddr);
      m_sequence = i.ReadU16 ();
      break;
    case THZ_PKT_TYPE_DATA:
      ReadFrom (i, m_srcAddr);
      ReadFrom (i, m_dstAddr);
      m_sequence = i.ReadU16 ();
      break;
    }

  return i.GetDistanceFrom (start);
}

void
THzMacHeader::Print (std::ostream &os) const
{
  os << "THZ src=" << m_srcAddr << " dest=" << m_dstAddr << " type=" << (uint32_t) m_type;
}


} // namespace ns3
