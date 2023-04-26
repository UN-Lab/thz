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

#ifndef THZ_UDP_SERVER_H
#define THZ_UDP_SERVER_H

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/packet-loss-counter.h"
#include "ns3/ptr.h"
#include "ns3/thz-dir-antenna.h"

namespace ns3
{
/**
 * \ingroup applications
 * \defgroup thzudpclientserver THzUdpClientServer
 */
/**
 * \ingroup thzudpclientserver
 * \class THzUdpServer
 * \brief A UDP server, receives UDP packets from a remote host.
 *
 * UDP packets carry a 32bits sequence number followed by a 64bits time
 * stamp in their payloads. The application uses the sequence number
 * to determine if a packet is lost, and the time stamp to compute the delay.
 */
class THzUdpServer : public Application
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId(void);
    THzUdpServer();
    virtual ~THzUdpServer();
    /**
     * \brief Returns the number of lost packets
     * \return the number of lost packets
     */
    uint32_t GetLost(void) const;

    /**
     * \brief Returns the number of received packets
     * \return the number of received packets
     */
    uint32_t GetReceived(void) const;

    /**
     * \brief Returns the size of the window used for checking loss.
     * \return the size of the window used for checking loss.
     */
    uint16_t GetPacketWindowSize() const;

    /**
     * \brief Set the size of the window used for checking loss. This value should
     *  be a multiple of 8
     * \param size the size of the window used for checking loss. This value should
     *  be a multiple of 8
     */
    void SetPacketWindowSize(uint16_t size);

  protected:
    virtual void DoDispose(void);

  private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);

    /**
     * \brief Handle a packet reception.
     *
     * This function is called by lower layers.
     *
     * \param socket the socket the packet was received to.
     */
    void HandleRead(Ptr<Socket> socket);

    uint16_t m_port;                 //!< Port on which we listen for incoming packets.
    Ptr<Socket> m_socket;            //!< IPv4 Socket
    Ptr<Socket> m_socket6;           //!< IPv6 Socket
    uint32_t m_received;             //!< Number of received packets
    PacketLossCounter m_lossCounter; //!< Lost packet counter

    Ptr<THzDirectionalAntenna> m_antenna;
    Ptr<THzNetDevice> m_thznetdev;
};

} // namespace ns3

#endif /* THZ_UDP_SERVER_H */
