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
 * Author: Zahed Hossain <zahedhos@buffalo.edu>
 *         Qing Xia <qingxia@buffalo.edu>
 *         Josep Miquel Jornet <jmjornet@buffalo.edu>
 */


#ifndef THZ_MAC_NANO_H
#define THZ_MAC_NANO_H

#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/event-id.h"
#include "ns3/traced-value.h"
#include "thz-mac.h"
#include "thz-phy.h"
#include <list>

namespace ns3 {

/**
 * \ingroup thz
 * \class THzMacNano
 * \brief THzMacNano models the MAC layer for nanoscale THz communication.
 *
 * When a packet is enqueued in the THzMacNano, it checks if it has
 * enough energy. If it does have sufficient energy to complete a
 * transmission, it passes the frame over to PHY layer for 0-way
 * or sends an RTS packet for 2-way handshake protocol. Otherwise, it just
 * puts the packet in the queue and waits for energy model to call back.
 */
class THzMacNano : public THzMac
{
/** Data structure for tracking ACK timeout of transmitted packets */
  typedef struct
  {
    uint16_t sequence;    //!< Packet sequence number.
    EventId m_ackTimeoutEvent;
    Ptr<Packet> packet;
  }   AckTimeouts;
  /** Data structure for tracking CTS timeout for sent RTS packets */
  typedef struct
  {
    uint16_t sequence;  //!< Packet sequence number.
    EventId m_ctsTimeoutEvent;
  } CtsTimeouts;
  /** Data structure for tracking Data timeout for sent CTS packets */
  typedef struct
  {
    uint16_t sequence;  //!< Packet sequence number.
    EventId m_dataTimeoutEvent;
  } DataTimeouts;
  /** Data structure for keeping track of enqueued packets */
  typedef struct
  {
    uint16_t sequence;  //!< Packet sequence number.
    uint16_t retry;     //!< Retry number.
    Ptr<Packet> packet;
    Time tstart;        //!< Transmission start time.
    Mac48Address destination; //!< Transmission start time.
    bool backoff;
  } PktTx;
public:
  /**
   * \brief Create a THzPhyNano
   */
  THzMacNano ();
  /**
   * \brief Destroy a THzPhyNano
   */
  virtual ~THzMacNano ();
  static TypeId GetTypeId (void);
  /**
   * \brief Sets the antenna parameters
   *
   * Sets the antenna parameters so it works as an Omnidirectional
   * antenna as for the nanoscale communication.
   */
  virtual void SetAntenna ();
  /**
   * \brief Ask THzEnergyModel to set callback.
   *
   * This callback will let the MAC layer know when it harvests
   * enough energy to transmit a packet.
   */
  virtual void InitEnergyCallback ();
  /**
   * Set slot duration for backoff.
   *
   * \param slotTime slot duration
   */
  virtual void SetSlotTime (Time duration);
  /**
   * Return slot duration for backoff.
   *
   * \return slot duration
   */
  virtual Time GetSlotTime (void);

  // Inherited methods from THzMac
  virtual void AttachPhy (Ptr<THzPhy> phy);
  virtual void SetDevice (Ptr<THzNetDevice> dev);
  virtual void SetAddress (Mac48Address addr);

  virtual Mac48Address GetBroadcast (void) const;
  virtual Mac48Address GetAddress () const;

  virtual bool Enqueue (Ptr<Packet> pkt, Mac48Address dest);
  virtual void SendPacketDone (Ptr<Packet> packet);
  virtual void ReceivePacket (Ptr<THzPhy> phy, Ptr<Packet> packet);
  virtual void ReceivePacketDone (Ptr<THzPhy> phy, Ptr<Packet> packet, bool collision);
  virtual void SetForwardUpCb (Callback<void, Ptr<Packet>, Mac48Address, Mac48Address> cb);
  virtual void Clear (void);

private:
  /**
   * Return transmission duration for a control packet.
   *
   * \param type is the type of control packet
   */
  Time GetCtrlDuration (uint16_t type);
  /**
   * Return transmission duration for a Data packet.
   *
   * \param p is the data packet
   */
  Time GetDataDuration (Ptr<Packet> p);
  /**
   * \brief Checks to see if there is enough energy for transmission
   *
   */
  void CheckResources (Ptr<Packet> packet);
  /**
   * \brief Sets the backoff time based on number of retry
   *
   */
  void Backoff (Ptr<Packet> packet, uint32_t retry);
  /**
   * \ brief remove a data packet from the queue
   */
  void Dequeue (Ptr<Packet> packet);
  /** Send RTS packet. */
  void SendRts (Ptr<Packet> packet);
  /** Send CTS packet. */
  void SendCts (Mac48Address dest, uint16_t sequence);
  /**
   * \brief Try to transmit the first packet in the queue.
   *
   * Called from the THz energy model when there is sufficient
   * energy to transmit a packet.
   */
  void TxFirstPacket (); //
  /** Send Data packet. */
  void SendData (Ptr<Packet> packet);
  /** Send ACK packet. */
  void SendAck (Mac48Address dest, uint16_t sequence);
  /**
   * Send on packet on the PHY.
   *
   * \param packet The packet.
   * \param rate boolean to determine the transmission rate between basic and data rate.
   */
  bool SendPacket (Ptr<Packet> packet, bool rate);
  /**
   * \brief Evaluate the transmission performance.
   *
   * \param success value that indicates if the corresponding ACK packet has been successfully received.
   * \param sequence the sequence number of the packet whose.
   *
   * The performance evaluation is based on the average throughput and discarding probability of the node.
   */
  void SendDataDone (bool success, Ptr<Packet> packet);
  /**
   * Process a received RTS packet.
   *
   * \param packet The RTS packet.
   */
  void ReceiveRts (Ptr<Packet> packet);
  /**
   * Process a received CTS packet.
   *
   * \param packet The CTS packet.
   */
  void ReceiveCts (Ptr<Packet> packet);
  /**
   * Process a received Data packet.
   *
   * \param packet The Data packet.
   */
  void ReceiveData (Ptr<Packet> packet);
  /**
   * Process a received ACK packet.
   *
   * \param packet The ACK packet.
   */
  void ReceiveAck (Ptr<Packet> packet);
  /**
   * \brief CTS time out occurred
   *
   * Retransmission is caused by the CTS time out. If number of retransmissions surpass the maximum retransmission
   * limit, the transmission of the corresponding DATA packet is considered as failed. No more retransmissions
   * of this DATA packet is needed.
   */
  void CtsTimeout (Ptr<Packet> packet);
  /**
   * \brief DATA time out occurred
   *
   * Retransmission is caused by the CTS time out. If number of retransmissions surpass the maximum retransmission
   * limit, the transmission of the corresponding DATA packet is considered as failed. No more retransmissions
   * of this DATA packet is needed.
   */
  void DataTimeout (uint16_t sequence);
  /**
   * \brief ACK time out occurred
   *
   * Retransmission is caused by the CTS time out. If number of retransmissions surpass the maximum retransmission
   * limit, the transmission of the corresponding DATA packet is considered as failed. No more retransmissions
   * of this DATA packet is needed.
   */
  void AckTimeout (uint16_t sequence);
  /**
   * \brief check if it is a new sequence
   *
   * \param addr MAC address of the DATA source.
   * \param seq the sequence number of the DATA packet.
   *
   * \return true if this is a new sequence number. False otherwise.
   */
  bool IsNewSequence (Mac48Address addr, uint16_t seq);

  Callback <void, Ptr<Packet>, Mac48Address, Mac48Address> m_forwardUpCb;
  Mac48Address m_address; //!< The MAC address.
  Ptr<THzPhy> m_phy;     //!< PHY layer attached to this MAC.
  Ptr<THzNetDevice> m_device;  //!< Device attached to this MAC.

  bool m_rtsEnable;  //!< Flag to enable or disable RTS.

  // Mac parameters
  uint16_t m_dataRetryLimit;    //!< Maximum number of retry before dropping the packet.
  uint16_t m_FrameLength;       //!< Packet length at the MAC layer
  uint16_t m_sequence;          //!< Enqueued packet sequence number.
  Time m_slotTime;              //!< Slot time duration.
  Ptr<Packet> m_pktData;
  Time m_backoffRemain; //
  Time m_ackTimeout; //!< ACK timeout time without interleaving delay.
  Ptr<THzDirectionalAntenna> m_thzAD; //!< Antenna for this node.

  Time m_tend; //!< Transmission end time.
  Time m_timeRec;
  double m_throughput; //!< Throughput of acked packet.
  double m_throughputAll;
  double m_throughputavg;
  int m_ite;
  int m_discarded;
  uint32_t m_queueLimit;
  std::list<Ptr<Packet> > m_pktQueue;  //!< Queue to hold the enqueued packets.
  std::list<std::pair<Mac48Address, uint16_t> > m_seqList;

  // for trace and performance evaluation
  TracedCallback<uint32_t, uint32_t> m_traceCtsTimeout;
  TracedCallback<uint32_t, uint32_t> m_traceAckTimeout;
  TracedCallback<uint32_t, uint32_t> m_traceEnqueue;
  TracedCallback<uint32_t, uint32_t, bool> m_traceSendDataDone;
  //add trace throughput
  TracedCallback<double> m_traceThroughput;

  std::list<DataTimeouts> m_dataTimeouts;
  std::list<AckTimeouts> m_ackTimeouts;
  std::list<CtsTimeouts> m_ctsTimeouts;
  std::list<PktTx> m_pktTx;

protected:
};

}

#endif // THZ_MAC_CSMA_H
