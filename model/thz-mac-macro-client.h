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
 *         Josep Miquel Jornet <jmjornet@buffalo.edu>
 *         Daniel Morales <danimoralesbrotons@gmail.com>
 */

#ifndef THZ_MAC_MACRO_CLIENT_H
#define THZ_MAC_MACRO_CLIENT_H

#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/event-id.h"
#include "ns3/traced-value.h"
#include "thz-net-device.h"
#include "thz-mac.h"
#include "thz-phy.h"
#include <list>

namespace ns3 {

/**
 * \ingroup thz
 * \class THzMacMacroClient
 * \brief THzMacMacroClient models the MAC layer for macroscale THz communication.
 *
 * This file describes the link-layer MAC protocol for the centralized macro-scale scenario.
 * In this scenario, we consider that there is only one access point operating as a receiver,
 * which is surrounded by the randomly distributed transmitters. The transmitters pointing their
 * directional antennas towards the receiver and the receiver periodically steer its directional
 * antenna to sweep the entire area. Two handshake protocols, i.e., zero-way handshake and two-
 * way handshake, are enabled by setting the "EnableRts" parameter to false or true.
 * The functions and reactions of any node seen from the link-layer interact with the node's
 * performances from the Physical layer. For the performance test, we calculate the average throughtput
 * and discarding probability of the nodes in this file.
 */

class THzMacMacroClient : public THzMac
{
  typedef struct
  {
    uint16_t sequence;
    EventId m_ackTimeoutEvent;
  } AckTimeouts;

  typedef struct
  {
    uint16_t sequence;
    EventId m_ctsTimeoutEvent;
  } CtsTimeouts;

  /**
    * Record packet information since it got enqueued
    */
  typedef struct
  {
    uint16_t RecSeq;            //!< data packet's sequence number
    Time RecTime;               //!< recording time
    uint16_t RecSize;           //!< size of the data packet
    uint16_t RecRetry;          //!< number of retransmittion
    Ptr<Packet> Recpacket;      //!< the data packet been recorded
    uint16_t BackoffLife;    //Number of CTS that the packet has to see before can be sent
  } Rec;

  /**
    * Encapsulate results for output
    */
  typedef struct
  {
    uint32_t nodeid;
    uint16_t Psize;
    Time delay;
    bool success;
    bool discard;
  } Result;
public:
  THzMacMacroClient ();
  virtual ~THzMacMacroClient ();

  /**
   * Register this type.
   * \return the type ID.
   */
  static TypeId GetTypeId (void);

  /**
   * \param duration the slot duration
   *
   * This function is to set the slot duration
   */
  virtual void SetSlotTime (Time duration);

  /**
   * \return the slot time
   */
  virtual Time GetSlotTime (void);

  /**
   * Attach THz PHY layer to this MAC.
   *
   * \param phy Phy layer to attach to this MAC.
   */
  virtual void AttachPhy (Ptr<THzPhy> phy);

  /**
   * \brief Attach the given netdevice to this MAC
   * \param dev pointer to the netdevice to attach to the MAC
   */
  virtual void SetDevice (Ptr<THzNetDevice> dev);


  /** Clears all pointer references. */
  virtual void Clear (void);

  /**
   * \ brief set up an EUI-48 MAC address
   *
   * \param addr Address for this MAC.
   */
  virtual void SetAddress (Mac48Address addr);

  /**
   * \ brief get the broadcast address
   */
  virtual Mac48Address GetBroadcast (void) const;

  /**
   * \return the MAC address associated to this MAC layer.
   */
  virtual Mac48Address GetAddress () const;


  /**
    * \ brief enqueue a data packet
    *
    * \param pkt  Packet to be transmitted.
    * \param dest Destination address.
    *
    * \return True if packet was successfully enqueued.
    *
    * When a packet been enqueued, it operates as the transmitter, thus the corresponding setting of
    * the directional antenna need to been accomplished.
    */
  virtual bool Enqueue (Ptr<Packet> pkt, Mac48Address dest);

  /**
    * \brief PHY has finished sending a packet.
    *
    * \param packet The Packet sent
    */
  virtual void SendPacketDone (Ptr<Packet> packet);

  /**
    * \ brief set the callback to forward packtes up to higher layers
    *
    * \param cb The callback.
    */
  virtual void SetForwardUpCb (Callback<void, Ptr<Packet>, Mac48Address, Mac48Address> cb);

  /**
    * \brief PHY has started recieving a packet.
    *
    * \param packet The Packet to receive
    * \param phy The PHY attached to this MAC and receiving the packet
    */
  virtual void ReceivePacket (Ptr<THzPhy> phy, Ptr<Packet> packet);

  /**
    * \brief PHY has finished recieving a packet.
    *
    * \param packet The Packet received
    * \param phy The PHY attached to this MAC and received the packet
    * \param collision The true value means packet was received successfully
    */
  virtual void ReceivePacketDone (Ptr<THzPhy> phy, Ptr<Packet> packet, bool collision, double rxPower);

private:
  typedef enum
  {
    IDLE, BACKOFF, WAIT_TX, TX, WAIT_ACK, RX, COLL
  } State;

  Time GetSifs (void) const;
  Time GetDifs (void) const;
  Time GetCtrlDuration (uint16_t type);
  Time GetDataDuration (Ptr<Packet> p);
  std::string StateToString (State state);

  /**
    * \brief setup clear channel assessment function
    */
  void CcaForDifs ();

  /**
    * \brief start backoff
    */
  void BackoffStart ();

  /**
    * \brief setting channel becomes busy
    *
    * Freeze remainning backoff time.
    */
  void ChannelBecomesBusy ();

  /**
    * \brief grant channel access
    */
  void ChannelAccessGranted ();
  /**
    * \brief update nav
    *
    * \param nav the nav time
    */
  void UpdateNav (Time nav);

  /**
    * \brief update local nav
    *
    * \param nav the nav time
    */
  void UpdateLocalNav (Time nav);

  /**
    * \ brief remove a data packet from the queue
    */
  void Dequeue ();

  /**
    * \brief send RTS packet.
    *
    * \param packet The DATA Packet waiting to be sent
    *
    * The node need to calculate the nav to book the channel,
    * it also scheludes the CTS time out when send out the RTS packet.
    */
  void SendRts (Ptr<Packet> pktData, uint16_t retry);

  /**
    * \brief receive RTS packet
    *
    * \param packet the RTS packet.
    *
    * Update local nav and schedule Send CTS event.
    */
  void ReceiveRts (Ptr<Packet> packet);

  /**
    * \brief send CTS packet
    *
    * \param dest MAC address of the destination
    * \param duration nav duration recorded in the received RTS packet's header
    * \param sequence the sequency number recorded in the RTS packet's header.
    *
    * Send CTS packet with the updated nav and sequence number set in the header.
    */
  void SendCts (Mac48Address dest, Time duration, uint16_t sequence);

  /**
    * \brief receive CTS packet
    *
    * \param packet the CTS packet.
    *
    * If the received CTS packet is the responding packet for the DATA packet waiting to be sent. Clear the
    * CTS time out and schedule sending the DATA packet.
    */
  void ReceiveCts (Ptr<Packet> packet);
  void ReceiveCta1 (Ptr<Packet> packet);
  void ReceiveCta3 (Ptr<Packet> packet);

  /**
    * \brief send the DATA packet
    *
    * \param packet the DATA packet.
    *
    * Send DATA packet with updated nav and ACK time out in the header.
    */
  void SendData (Ptr<Packet> packet, int mcs);

  /**
    * \brief receive the DATA packet
    *
    * \param packet the DATA packet.
    *
    * Receive DATA packet and schedule sending ACK packet.
    */
  void ReceiveData (Ptr<Packet> packet);

  /**
    * \brief send ACK packet
    *
    * \param dest the MAC address of the transmitter.
    * \param sequence the sequence number recored in the header of the received DATA packet.
    *
    * Updata local nav and send out ACK packet with corresponding sequence number in its header.
    */
  void SendAck (Mac48Address dest, uint16_t sequence);

  /**
    * \brief receive ACK packet
    *
    * \param packet the received ACK packet.
    *
    * If the received ACK packet is the corresponding ACK to the DATA packet, schedule the SendDataDone
    * function and cancel the ACK time out for this DATA packet.
    */
  void ReceiveAck (Ptr<Packet> packet);

  /**
  * \brief send Rts packet
  *
  * \param packet The DATA Packet waiting to be sent.
  * \param rate transmission rate of packets in bps.
  *
  * \return ture if the ThzPhy is not working in the transmission mode. false otherwise.
  */
  bool SendPacket (Ptr<Packet> packet, bool rate, uint16_t mcs);

  /**
    * \brief send DATA packet done and evaluate the transmission performance.
    *
    * \param success value that indicates is the corresponding ACK packet has been successfully received or not.
    * \param sequence the sequence number recorded in the received ACK packet header.
    *
    * The performance evaluation is based on the average throughput and discarding probability of the node.
    */
  void SendDataDone (bool success, uint16_t sequence);

  void StartOver ();

  /**
    * \brief CTS time out
    *
    * Retransmission is caused by the CTS time out, if number of retransmission surpass the maximum retransmission
    * limitation. The transmission of the corresponding DATA packet is considered as failed. No more retransmissions
    * of this DATA packet need to process.
    */
  void CtsTimeout (uint16_t sequence);

  /**
    * \brief ACK time out
    *
    * Retransmission is caused by the ACK time out, if number of retransmission surpass the maximum retransmission
    * limitation. The transmission of the corresponding DATA packet is considered as failed. No more retransmissions
    * of this DATA packet need to process.
    */
  void AckTimeout (uint16_t sequence);

  /**
    * \brief Backoff Time
    *
    * The backoff time calculated based on the number of retransmission of the DATA packet.
    */
  void Backoff (uint32_t retry);

  /**
    * \brief check if it is a new sequence
    *
    * \param addr MAC address of the DATA source.
    * \param seq the sequence number of the DATA packet.
    *
    * \return true if this is a new sequence number. False otherwise.
    */
  bool IsNewSequence (Mac48Address addr, uint16_t seq);

  /**
    * \brief record the results into output file
    */
  void ResultsRecord ();
  void CollisionsRecord(uint16_t retry);

  /**
    * \brief record the positions into output file
    */
  void PositionsRecord ();

  Callback <void, Ptr<Packet>, Mac48Address, Mac48Address> m_forwardUpCb;
  Mac48Address m_address;
  Ptr<THzPhy> m_phy;
  Ptr<THzNetDevice> m_device;

  State m_state;

  Ptr<THzDirectionalAntenna> m_thzAD;

  EventId m_ccaTimeoutEvent;
  EventId m_backoffTimeoutEvent;
  EventId m_ctsTimeoutEvent;
  EventId m_ctsDTimeoutEvent;
  EventId m_ackTimeoutEvent;
  EventId m_sendCtsEvent;
  EventId m_sendAckEvent;
  EventId m_sendDataEvent;
  EventId m_SetRxAntennaEvent;

  // Mac parameters
  uint16_t m_boSlots;
  uint16_t m_rtsRetryLimit;
  uint16_t m_dataRetryLimit;
  uint16_t m_retry;
  uint16_t m_sequence;

  Time m_slotTime;
  Time m_slotTime_3way;
  Time m_sifs;
  Time m_difs;
  Ptr<Packet> m_pktTx;
  Ptr<Packet> m_pktData;


  uint16_t m_send;
  uint16_t m_discard;

  Time m_tData;             //!< transmission duration of the DATA packet
  double m_rxIniAngle;      //!< initial angle of the receiver antenna
  uint32_t m_MinEnquePacketSize;    //!< the minimum DATA packet size needed to enqueue the packet
  uint16_t m_probDiscard;   //!< the DATA packet discarding probability

  Time m_nav;
  Time m_localNav;
  Time m_backoffRemain;
  Time m_boRemain;
  Time m_backoffStart;

  Time m_tstart;
  Time m_tend;
  uint16_t m_seqRec;
  uint16_t m_pktRec;
  Time m_timeRec;
  double m_throughput;
  double m_throughputAll;
  double m_throughputavg;
  Mac48Address m_addRecS;
  int m_ite;

  uint32_t m_queueLimit;
  std::list<Ptr<Packet> > m_pktQueue;
  std::list<std::pair<Mac48Address, uint16_t> > m_seqList;
  std::list<std::pair<uint16_t, Time> > m_pktTxlist;
  std::list<Rec> m_rec;
  std::list<Result> m_result;

  std::list<AckTimeouts> m_ackTimeouts;
  std::list<CtsTimeouts> m_ctsTimeouts;


  TracedCallback<uint32_t, uint32_t> m_traceCtsTimeout;
  TracedCallback<uint32_t, uint32_t> m_traceAckTimeout;
  TracedCallback<uint32_t, uint32_t> m_traceEnqueue;
  TracedCallback<uint32_t, uint32_t, bool> m_traceSendDataDone;
  TracedCallback<double> m_traceThroughput;

  // *** for 1-way ***
  // should be formatted before going into app store
  typedef struct
  {
  Time m_ctsLife;
  Mac48Address m_ctsSource;
  } CtsLife;

  uint16_t m_ways;
  std::list<CtsLife> m_ctsLifeTrack;
  void InitCts ();
  bool m_backoffActive;
  uint16_t m_backoffSeq;
  Ptr<MobilityModel> m_clientMobility;
  double m_beamwidth;
  void InitVariables();
  double m_nSector;
  Time m_tCircle;
  Time m_tSector;
  uint16_t m_nodeId;
  std::string outputFile;
  uint16_t m_ctsReceived;
  Time GetMaxBackoff();
  uint16_t m_lastSeq;
  void DecreaseBackoff ();
  double m_dataRate;
  Time m_tProp;
  Time m_timeCTSrx;
  double m_sector;
  bool m_rtsAnswered;
  void StateRecord(uint16_t state);
  
protected:
};

}

#endif /* THZ_MAC_MACRO_CLIENT_H */
