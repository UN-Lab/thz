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

#ifndef THZ_MAC_HEADER_H
#define THZ_MAC_HEADER_H

#include "ns3/header.h"
#include "ns3/mac48-address.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"

#define THZ_PKT_TYPE_CTA 0
#define THZ_PKT_TYPE_RTS 1
#define THZ_PKT_TYPE_CTS 2
#define THZ_PKT_TYPE_ACK 3
#define THZ_PKT_TYPE_DATA 4

namespace ns3
{
/**
 * \defgroup Terahertz MAC Headers including RTS, CTS, ACK and DATA
 *
 */

class THzMacHeader : public Header
{
  public:
    THzMacHeader();

    THzMacHeader(const Mac48Address srcAddr, const Mac48Address dstAddr, uint8_t type);
    virtual ~THzMacHeader();

    static TypeId GetTypeId(void);
    /**
     * \ brief set the source address
     */
    void SetSource(Mac48Address addr);

    /**
     * \ brief set the destination address
     */
    void SetDestination(Mac48Address addr);

    /**
     * \ brief set the packet type, i.e., RTS, CTS, ACK or DATA
     */
    void SetType(uint8_t type);
    void SetRetry(uint8_t retry);
    void SetFlags(uint16_t flags);
    void SetSector(uint16_t sector);
    /**
     * \ brief set the duration field with the given duration
     *
     * The method converts the given time to microseconds
     */
    void SetDuration(Time duration);

    /**
     * \ brief set the sequence number of the header
     */
    void SetSequence(uint16_t seq);

    /**
     * \ brief get the source address
     */
    Mac48Address GetSource() const;

    /**
     * \ brief get the destination address
     */
    Mac48Address GetDestination() const;

    /**
     * \ brief get the packet type, i.e., RTS, CTS, ACK or DATA
     */
    uint8_t GetType() const;
    uint8_t GetRetry() const;
    uint16_t GetFlags() const;
    uint16_t GetSector() const;

    /**
     * \ brief get the duration field with the given duration
     *
     */
    Time GetDuration() const;

    /**
     * \ brief get the packet size
     */
    uint32_t GetSize() const;

    /**
     * \ brief get the sequence number of the header
     */
    uint16_t GetSequence() const;

    // Inherrited methods
    virtual uint32_t GetSerializedSize(void) const;
    virtual void Serialize(Buffer::Iterator start) const;
    virtual uint32_t Deserialize(Buffer::Iterator start);
    virtual void Print(std::ostream& os) const;
    virtual TypeId GetInstanceTypeId(void) const;

  private:
    Mac48Address m_srcAddr;
    Mac48Address m_dstAddr;
    uint8_t m_type;
    uint16_t m_duration;
    uint16_t m_sequence;
    uint8_t m_retry;
    uint16_t m_flags;
    uint16_t m_sector;
};

} // namespace ns3

#endif /* THZ_MAC_HEADER_H */
