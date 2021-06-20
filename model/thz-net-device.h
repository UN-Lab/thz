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
 */

#ifndef THZ_NET_DEVICE_H
#define THZ_NET_DEVICE_H

#include "ns3/net-device.h"
#include "ns3/pointer.h"
#include "ns3/traced-callback.h"
#include "ns3/node.h"
#include "ns3/node-container.h"
#include "ns3/mac48-address.h"
#include "vector"

namespace ns3 {
/**
 * \defgroup Terahertz Net Device Models
 *
 */

class THzChannel;
class THzPhy;
class THzMac;
class THzDirectionalAntenna;

class THzNetDevice : public NetDevice
{
public:
  /**
    * \brief Get the type ID.
    * \return the object TypeID
    */
  static TypeId GetTypeId (void);

  THzNetDevice ();
  virtual ~THzNetDevice ();
  /**
    * \ brief attach to terahertz MAC layer
    */
  virtual void SetMac (Ptr<THzMac> mac);

  /**
    * \ brief attach to terahertz physical layer
    */
  virtual void SetPhy (Ptr<THzPhy> phy);

  /**
    * \ brief attach to terahertz channel
    */
  virtual void SetChannel (Ptr<THzChannel> channel);

  /**
    * \ brief set directional antenna
    */
  virtual void SetDirAntenna (Ptr<THzDirectionalAntenna> dirantenna);

  virtual Ptr<THzMac> GetMac (void) const;
  virtual Ptr<THzPhy> GetPhy (void) const;
  virtual Ptr<THzDirectionalAntenna> GetDirAntenna (void) const;

  void Clear (void);

  // Purely virtual functions from base class

  /**
    * \ brief set index of interface if given
    * \ param index ifIndex of the device
    */
  virtual void SetIfIndex (const uint32_t index);
  /**
    * \ return index ifIndex of the device
    */
  virtual uint32_t GetIfIndex (void) const;

  /**
    * \ return the channel this NetDevice is connected to. The value returned can be zero if folder
    *   the NetDevice is not yet connected to any channel or if the underlying NetDevice has no concept
    *   of a channel.
    */
  virtual Ptr<Channel> GetChannel (void) const;

  /**
    * \ return the current Address of this interface
    */
  virtual Address GetAddress (void) const;

  /**
    * \ param mtu MTU (Maximum Transmission Unit) value, in bytes, to set for the device
    * \ return whether the MTU value was within legal bounds
    *
    * Override for default MTU defined on a per-type basis.
    */
  virtual bool SetMtu (const uint16_t mtu);

  /**
    * \return the link MTU in bytes for this interface
    *
    * This value is typically used by the IP layer to perform
    * IP fragmentation when needed.
    */
  virtual uint16_t GetMtu (void) const;

  /**
    * \ return true if link is up; false otherwise
    */
  virtual bool IsLinkUp (void) const;

  /**
    * \ return true if this interface supports a broadcast address, false otherwise
    */
  virtual bool IsBroadcast (void) const;

  /**
    * \ return the broadcast address supported by this netdevice
    *
    * Calling this methed is invalid if IsBroadcast returns not true.
    */
  virtual Address GetBroadcast (void) const;

  /**
    * \ return value of m_isMulticast flag
    */
  virtual bool IsMulticast (void) const;

  /**
    * \brief Make and return a MAC multicast address using the provided
    *        multicast group
    *
    * \RFC{1112} says that an Ipv4 host group address is mapped to an Ethernet
    * multicast address by placing the low-order 23-bits of the IP address into
    * the low-order 23 bits of the Ethernet multicast address
    * 01-00-5E-00-00-00 (hex).  Similar RFCs exist for Ipv6 and Eui64 mappings.
    * This method performs the multicast address creation function appropriate
    * to the underlying MAC address of the device.  This MAC address is
    * encapsulated in an abstract Address to avoid dependencies on the exact
    * MAC address format.
    *
    * In the case of net devices that do not support
    * multicast, clients are expected to test NetDevice::IsMulticast and avoid
    * attempting to map multicast packets.  Subclasses of NetDevice that do
    * support multicasting are expected to override this method and provide an
    * implementation appropriate to the particular device.
    *
    * \param multicastGroup The IP address for the multicast group destination
    * of the packet.
    * \return The MAC multicast Address used to send packets to the provided
    * multicast group.
    *
    * \warning Calling this method is invalid if IsMulticast returns not true.
    * \see Ipv4Address
    * \see Address
    * \see NetDevice::IsMulticast
    */
  virtual Address GetMulticast (Ipv4Address multicastGroup) const;

  /**
    * \brief Get the MAC multicast address corresponding
    * to the IPv6 address provided.
    * \param addr IPv6 address
    * \return the MAC multicast address
    * \warning Calling this method is invalid if IsMulticast returns not true.
    */
  virtual Address GetMulticast (Ipv6Address addr) const;

  /**
   * \brief Return true if the net device is acting as a bridge.
   *
   * \return value of m_isBridge flag
   */
  virtual bool IsBridge (void) const;

  /**
   * \brief Return true if the net device is on a point-to-point link.
   *
   * \return value of m_isPointToPoint flag
   */
  virtual bool IsPointToPoint (void) const;

  /**
   * \param packet packet sent from above down to Network Device
   * \param dest mac address of the destination (already resolved)
   * \param protocolNumber identifies the type of payload contained in
   *        this packet. Used to call the right L3Protocol when the packet
   *        is received.
   *
   *  Called from higher layer to send packet into Network Device
   *  to the specified destination Address
   *
   * \return whether the Send operation succeeded
   */
  virtual bool Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);

/**
   * \param packet packet sent from above down to Network Device
   * \param source source mac address (so called "MAC spoofing")
   * \param dest mac address of the destination (already resolved)
   * \param protocolNumber identifies the type of payload contained in
   *        this packet. Used to call the right L3Protocol when the packet
   *        is received.
   *
   *  Called from higher layer to send packet into Network Device
   *  with the specified source and destination Addresses.
   *
   * \return whether the Send operation succeeded
   */
  virtual bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber);

  /**
   * \param node the node associated to this netdevice.
   *
   * This method is called from ns3::Node::AddDevice.
   */
  virtual void SetNode (Ptr<Node> node);

  /**
   * \returns the node base class which contains this network
   *          interface.
   *
   * When a subclass needs to get access to the underlying node
   * base class to print the nodeid for example, it can invoke
   * this method.
   */
  virtual Ptr<Node> GetNode (void) const;

  /**
   * \returns true if ARP is needed, false otherwise.
   *
   * Called by higher-layers to check if this NetDevice requires
   * ARP to be used.
   */
  virtual bool NeedsArp (void) const;

  /**
   * \param cb callback to invoke whenever a packet has been received and must
   *        be forwarded to the higher layers.
   *
   */
  virtual void SetReceiveCallback (NetDevice::ReceiveCallback cb);

  /**
   * \param cb callback to invoke whenever a packet has been received in promiscuous mode and must
   *        be forwarded to the higher layers.
   *
   * Enables netdevice promiscuous mode and sets the callback that
   * will handle promiscuous mode packets.  Note, promiscuous mode
   * packets means _all_ packets, including those packets that can be
   * sensed by the netdevice but which are intended to be received by
   * other hosts.
   */
  virtual void SetPromiscReceiveCallback (PromiscReceiveCallback cb);
  virtual bool SupportsSendFrom (void) const;

  /**
    * \ param callback the call back to invoke
    *
    * Add a callback invoked whenever the link
    * status changes to UP. This callback is typically used by the IP/ARP layer
    * to flush the ARP cache and by IPv6 stack to flush NDISC cache whenever the
    * link goes up.
    */
  virtual void AddLinkChangeCallback (Callback<void> callback);

  /**
    * Set the address of this interface
    * \param address address to set
    */
  virtual void SetAddress (Address address);

private:
  virtual void ForwardUp (Ptr<Packet> packet, Mac48Address src, Mac48Address dest);
  Ptr<THzChannel> DoGetChannel (void) const;

  Ptr<Node> m_node;

  Ptr<THzChannel> m_channel;
  Ptr<THzMac> m_mac;
  Ptr<THzPhy> m_phy;
  Ptr<THzDirectionalAntenna> m_dirantenna;


  std::string m_name;
  uint32_t m_ifIndex;
  uint16_t m_mtu;
  bool m_linkup;
  TracedCallback<> m_linkChanges;
  ReceiveCallback m_forwardUp;

  TracedCallback<Ptr<const Packet>, Mac48Address> m_rxLogger;
  TracedCallback<Ptr<const Packet>, Mac48Address> m_txLogger;

  bool m_arp;

protected:
  virtual void DoDispose ();
};

} // namespace ns3

#endif // THZ_NET_DEVICE_H
