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

#ifndef THZ_MAC_MACRO_AP_H
#define THZ_MAC_MACRO_AP_H

#include "thz-mac.h"
#include "thz-net-device.h"
#include "thz-phy.h"

#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/traced-value.h"

#include <list>
#include <map>
#include <vector>

namespace ns3
{

/**
 * \ingroup thz
 * \class THzMacMacroAp
 * \brief THzMacMacroAp models the MAC layer for macroscale THz communication.
 *
 * This file describes the link-layer MAC protocol for the centralized macro-scale scenario.
 * In this scenario, we consider that there is only one access point operating as a receiver,
 * which is surrounded by the randomly distributed transmitters. The transmitters pointing their
 * directional antennas towards the receiver and the receiver periodically steer its directional
 * antenna to sweep the entire area. Two handshake protocols, i.e., zero-way handshake and two-
 * way handshake, are enabled by setting the "EnableRts" parameter to false or true.
 * The functions and reactions of any node seen from the link-layer interact with the node's
 * performances from the Physical layer. For the performance test, we calculate the average
 * throughtput and discarding probability of the nodes in this file.
 */

class THzMacMacroAp : public THzMac
{
    /**
     * Encapsulate results for output
     */
    /*typedef struct
    {
      uint32_t nodeid;
      uint16_t Psize;
      Time delay;
      bool success;
      bool discard;
    } Result;*/
  public:
    THzMacMacroAp();
    virtual ~THzMacMacroAp();

    /**
     * Register this type.
     * \return the type ID.
     */
    static TypeId GetTypeId(void);

    /**
     * \param duration the slot duration
     *
     * This function is to set the slot duration
     */
    virtual void SetSlotTime(Time duration);

    /**
     * \return the slot time
     */
    virtual Time GetSlotTime(void);

    /**
     * Attach THz PHY layer to this MAC.
     *
     * \param phy Phy layer to attach to this MAC.
     */
    virtual void AttachPhy(Ptr<THzPhy> phy);

    /**
     * \brief Attach the given netdevice to this MAC
     * \param dev pointer to the netdevice to attach to the MAC
     */
    virtual void SetDevice(Ptr<THzNetDevice> dev);

    /** Clears all pointer references. */
    virtual void Clear(void);

    /**
     * \ brief set up an EUI-48 MAC address
     *
     * \param addr Address for this MAC.
     */
    virtual void SetAddress(Mac48Address addr);

    /**
     * \ brief get the broadcast address
     */
    virtual Mac48Address GetBroadcast(void) const;

    /**
     * \return the MAC address associated to this MAC layer.
     */
    virtual Mac48Address GetAddress() const;

    /**
     * \brief PHY has finished sending a packet.
     *
     * \param packet The Packet sent
     */
    virtual void SendPacketDone(Ptr<Packet> packet);

    /**
     * \ brief set the callback to forward packtes up to higher layers
     *
     * \param cb The callback.
     */
    virtual void SetForwardUpCb(Callback<void, Ptr<Packet>, Mac48Address, Mac48Address> cb);

    /**
     * \brief PHY has started recieving a packet.
     *
     * \param packet The Packet to receive
     * \param phy The PHY attached to this MAC and receiving the packet
     */
    virtual void ReceivePacket(Ptr<THzPhy> phy, Ptr<Packet> packet);

    /**
     * \brief PHY has finished recieving a packet.
     *
     * \param packet The Packet received
     * \param phy The PHY attached to this MAC and received the packet
     * \param collision The true value means packet was received successfully
     */
    virtual void ReceivePacketDone(Ptr<THzPhy> phy,
                                   Ptr<Packet> packet,
                                   bool collision,
                                   double rxPower);

  private:
    typedef enum
    {
        IDLE,
        BACKOFF,
        WAIT_TX,
        TX,
        WAIT_ACK,
        RX,
        COLL
    } State;

    Time GetSifs(void) const;
    Time GetDifs(void) const;
    Time GetCtrlDuration(uint16_t type);
    Time GetDataDuration(uint32_t size, uint8_t mcs);
    std::string StateToString(State state);

    /**
     * \brief receive the DATA packet
     *
     * \param packet the DATA packet.
     *
     * Receive DATA packet and schedule sending ACK packet.
     */
    void ReceiveData(Ptr<Packet> packet);

    /**
     * \brief send ACK packet
     *
     * \param dest the MAC address of the transmitter.
     * \param sequence the sequence number recored in the header of the received DATA packet.
     *
     * Updata local nav and send out ACK packet with corresponding sequence number in its header.
     */
    void SendAck();

    /**
     * \brief send Rts packet
     *
     * \param packet The DATA Packet waiting to be sent.
     * \param rate transmission rate of packets in bps.
     *
     * \return ture if the ThzPhy is not working in the transmission mode. false otherwise.
     */
    bool SendPacket(Ptr<Packet> packet, bool rate);

    /**
     * \brief check if it is a new sequence
     *
     * \param addr MAC address of the DATA source.
     * \param seq the sequence number of the DATA packet.
     *
     * \return true if this is a new sequence number. False otherwise.
     */
    bool IsNewSequence(Mac48Address addr, uint16_t seq);

    virtual bool Enqueue(Ptr<Packet> pkt, Mac48Address dest);

    /**
     * \brief record the cycle time into output file
     */
    void CycleRecord();

    Callback<void, Ptr<Packet>, Mac48Address, Mac48Address> m_forwardUpCb;
    Mac48Address m_address;
    Ptr<THzPhy> m_phy;
    Ptr<THzNetDevice> m_device;

    State m_state;

    Ptr<THzDirectionalAntenna> m_thzAD;

    EventId m_ccaTimeoutEvent;
    EventId m_backoffTimeoutEvent;
    EventId m_ctsTimeoutEvent;
    EventId m_ackTimeoutEvent;
    EventId m_sendCtsEvent;
    EventId m_sendDataEvent;
    EventId m_SetRxAntennaEvent;

    // Mac parameters
    uint16_t m_boSlots;
    uint16_t m_retry;

    Time m_slotTime;
    Time m_slotTime_3way;
    Time m_sifs;
    Time m_difs;
    Ptr<Packet> m_pktTx;
    Ptr<Packet> m_pktData;

    uint16_t m_send;
    uint16_t m_discard;

    Time m_tData;          //!< transmission duration of the DATA packet
    double m_angle;        //!< initial angle of the receiver antenna
    uint32_t m_packetSize; //!< the minimum DATA packet size needed to enqueue the packet

    Time m_nav;
    Time m_localNav;
    Time m_backoffRemain;
    Time m_boRemain;
    Time m_backoffStart;

    Time m_tstart;
    Time m_tend;
    uint16_t m_pktRec;
    Time m_timeRec;
    double m_throughput;
    double m_throughputAll;
    Mac48Address m_addRecS;
    int m_ite;

    std::list<Ptr<Packet>> m_pktQueue;
    std::list<Ptr<Packet>> m_ackList;
    std::list<std::pair<Mac48Address, uint16_t>> m_seqList;
    std::list<std::pair<uint16_t, Time>> m_pktTxlist;

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

    std::list<CtsLife> m_ctsLifeTrack;
    void SendCta1();
    void SendCta3();
    void SendCts(Mac48Address dest, uint16_t sequence, Time duration, uint16_t flag);
    void ReceiveRts(Ptr<Packet> packet, double rxPower);
    void InitNodeMap();

    uint16_t m_ways;
    Ptr<MobilityModel> m_clientMobility;
    Ptr<UniformRandomVariable> m_uniRand;
    double m_beamwidth;
    void Init();
    void TurnRxAntenna();
    void DataTimeout();
    void WaitTimeExpired();
    void SectorTimeout();
    void SendFeedbackCTA(double angle, Mac48Address dest);
    int SelectMCS(double power);
    Time GetMaxBackoff();
    EventId m_dataTimeoutEvent;
    EventId m_waitTimeEvent;
    EventId m_sectorTimeoutEvent;
    double m_nSector;
    Time m_tMaxCircle;
    Time m_tSector;
    uint16_t m_nodeId;
    std::string outputFile;
    double m_turningSpeed;
    Time m_tProp;
    uint16_t m_expectedData;
    bool m_useWhiteList;
    bool m_useAdaptMCS;

    std::list<std::pair<Ptr<Packet>, double>> m_rtsList;
    std::map<double, std::vector<std::pair<Mac48Address, double>>> m_sectorMap;
    std::map<Mac48Address, std::vector<std::pair<double, double>>> m_nodeMap;
    std::map<double, std::vector<Mac48Address>> m_whiteList;
    bool m_recordNodeSector;
    uint16_t m_dummyCycles;

    double csth_BPSK;
    double csth_QPSK;
    double csth_8PSK;
    double csth_16QAM;
    double csth_64QAM;

  protected:
};

} // namespace ns3

#endif /* THZ_MAC_MACRO_AP_H */
