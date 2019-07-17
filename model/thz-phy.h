/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 UBNANO (http://ubnano.tech/)
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
 *         Josep Miquel Jornet <jmjornet@buffalo.edu>
 */

#ifndef THZ_PHY_H
#define THZ_PHY_H

#include "ns3/simulator.h"
#include "ns3/event-id.h"
#include "thz-mac.h"
#include "thz-phy.h"
#include "ns3/traced-value.h"
#include "ns3/mobility-model.h"
#include "ns3/thz-spectrum-waveform.h"
#include "thz-net-device.h"
#include <list>

namespace ns3 {

class THzPhy : public Object
{
public:
  /** Clears all pointer references. */
  virtual void Clear () = 0;

  /**
    * \ brief calculate the power spectral density of the transmitted signal
    */
  virtual void CalTxPsd () = 0;

  /**
    * \ brief attach the terahertz net device to this terahertz physical layer
    */
  virtual void SetDevice (Ptr<THzNetDevice> device) = 0;

  /**
    * \ brief attach the terahertz MAC layer to this terahertz phisical layer.
    */
  virtual void SetMac (Ptr<THzMac> mac) = 0;

  /**
    * \ brief attach the terahertz channel to this terahertz physical layer
    */
  virtual void SetChannel (Ptr<THzChannel> channel) = 0;

  /**
    * \ brief set up transmission power
    *
    * \ param dBm the tansmission power unit in dBm
    */
  virtual void SetTxPower (double dBm) = 0;

  /**
    * \brief get terahertz channel
    *
    * \ return the value of m_channel flag.
    */
  virtual Ptr<THzChannel> GetChannel () = 0;

  /**
    * \brief get MAC address
    *
    * \ return the Mac48Address
    *
    * The format of the string is "xx:xx:xx:xx:xx:xx"
    */
  virtual Mac48Address GetAddress () = 0;

  /**
    * \brief get the transmission power
    *
    * \ return the transmission power in dBm
    */
  virtual double GetTxPower () = 0;

  /**
    * \param packet packet sent from above down to terahertz physical layer.
    * \param rate transmission rate of packets in bps.
    *
    * Called from higher layer (MAC layer) to send packet into physical layer to the specified destination Address.
    * Pass the packet to lower layer (channel).
    */
  virtual bool SendPacket (Ptr<Packet> packet, bool rate) = 0;

  /**
    * \param packet packet sent out from the terahertz channel.
    *
    * Called from terahertz channel to indicate the packet has been sent out.
    * Terahertz physical layer need to pass this message to the upper layer (terahertz MAC layer).
    */
  virtual void SendPacketDone (Ptr<Packet> packet) = 0;

  /**
    * \param packet packet received from lower layer (terahertz channel).
    * \param txDuration transmission duration of this packet.
    * \param rxPower power strength of the received packet.
    *
    * Called from terahertz channel to indicate the packet is been receiving by the receiver.
    * Terahertz physical layer need to pass this message to the upper layer (terahertz MAC layer)
    */
  virtual void ReceivePacket (Ptr<Packet> packet, Time txDuration, double_t rxPower) = 0;

  /**
    * \param packet packet received from lower layer (terahertz channel).
    * \param rxPower power strength of the received packet.
    *
    * Called from terahertz channel to indicate the packet is been completely received by the receiver.
    * Terahertz physical layer need to pass this message to the upper layer (terahertz MAC layer)
    */
  virtual void ReceivePacketDone (Ptr<Packet> packet, double rxPower) = 0;

  /**
    * \param basicSize the size of the control packet
    * \param dataSize the size of the DATA packet
    *
    * \return the time duration for transmitting a packet.
    */

  virtual Time CalTxDuration (uint32_t basicSize, uint32_t dataSize) = 0;

};

} // namespace ns3

#endif // THZ_PHY_H
