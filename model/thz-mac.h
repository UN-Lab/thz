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

#ifndef THZ_MAC_H
#define THZ_MAC_H

#include "ns3/address.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "thz-net-device.h"
#include "thz-phy.h"
#include "thz-channel.h"

namespace ns3 {

class THzPhy;
class THzChannel;
class THzNetDevice;

/**
 * \ingroup thz
 *
 * Virtual base class for all THz MAC protocols.
 */
class THzMac : public Object
{
public:
  /**
   * Attach THz PHY layer to this MAC.
   *
   * \param phy Phy layer to attach to this MAC.
   */
  virtual void AttachPhy (Ptr<THzPhy> phy) = 0;
  /**
   * \brief Attach the given netdevice to this MAC
   * \param dev pointer to the netdevice to attach to the MAC
   */
  virtual void SetDevice (Ptr<THzNetDevice> dev) = 0;

  /**
   * \ brief set up an EUI-48 MAC address
   *
   * \param addr Address for this MAC.
   */
  virtual void SetAddress (Mac48Address addr) = 0;

  /**
   * \return the MAC address associated to this MAC layer.
   */
  virtual Mac48Address GetAddress (void) const = 0;

  /**
   * \ brief get the broadcast address
   */
  virtual Mac48Address GetBroadcast (void) const = 0;

  /**
   * \ brief enqueue a data packet
   *
   * \param pkt Packet to be transmitted.
   * \param dest Destination address.
   *
   * \return True if packet was successfully enqueued.
   */
  virtual bool Enqueue (Ptr<Packet> pkt, Mac48Address dest) = 0;
  /**
   * \brief PHY has finished sending a packet.
   *
   * \param packet The Packet sent
   */
  virtual void SendPacketDone (Ptr<Packet> packet) = 0;
  /**
   * \brief PHY has started recieving a packet.
   *
   * \param packet The Packet to receive
   * \param phy The PHY attached to this MAC and receiving the packet
   */
  virtual void ReceivePacket (Ptr<THzPhy> phy, Ptr<Packet> packet) = 0;
  /**
   * \brief PHY has finished recieving a packet.
   *
   * \param packet The Packet received
   * \param phy The PHY attached to this MAC and received the packet
   * \param collision The true value means packet was received successfully
   */
  virtual void ReceivePacketDone (Ptr<THzPhy> phy, Ptr<Packet> packet, bool collision, double rxPower) = 0;

  /**
   * \ brief set the callback to forward packtes up to higher layers
   *
   * \param cb The callback.
   */
  virtual void SetForwardUpCb (Callback<void, Ptr<Packet>, Mac48Address, Mac48Address> cb) = 0;
  /** Clears all pointer references. */
  virtual void Clear (void) = 0;


  /**
  * TracedCallback signature for timeout.
  *
  * \param [in] node id.
  * \param [in] device index.
  */
  typedef void (* TimeTracedCallback)(uint32_t nodeID, uint32_t devIndex);


  /**
  * TracedCallback signature for SendDataDone.
  *
  * \param [in] node id.
  * \param [in] device index.
  * \param [in] Send Data Done true or false.
  */
  typedef void (* SendDataDoneTracedCallback)(uint32_t nodeID, uint32_t devIndex, bool status);


  /**
  * TracedCallback signature for throughput.
  *
  * \param [in] value of throughput.
  */
  typedef void (* ThroughputTracedCallback)(uint32_t throughput);

};

}

#endif // THZ_MAC_H
