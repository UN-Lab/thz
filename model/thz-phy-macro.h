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

#ifndef THZ_PHY_MACRO_H
#define THZ_PHY_MACRO_H

#include "thz-mac.h"
#include "thz-phy.h"

#include "ns3/event-id.h"
#include "ns3/mobility-model.h"
#include "ns3/simulator.h"
#include "ns3/thz-spectrum-waveform.h"
#include "ns3/traced-value.h"

#include <list>

namespace ns3
{
/**
 * \ingroup thz
 * \class THzPhyMacro
 * \brief THzPhyMacro models the physical layer of the terahertz communication networks
 * in the macro-scale scenario.
 *
 * This class represents the physical layer of macro-scale terahertz communication networks,
 * it mainly interacts with the terahertz link layer MAC protocol and the terahertz channel.
 * The physical layer status got updated during the communication process. In this physical layer,
 * we do consider the interference caused by the neighbors.
 */

class THzPhyMacro : public THzPhy
{
    typedef struct
    {
        Time rxStart;       //!< start time of the ongoing transmission
        Time rxDuration;    //!< duration of the ongoing transmission
        Ptr<Packet> packet; //!< the ongoing packet
        double_t rxPower;   //!< the receiving power
        double interference;
    } OngoingRx;

  public:
    THzPhyMacro();
    virtual ~THzPhyMacro();

    /** Clears all pointer references. */
    void Clear();

    /**
     * \brief Get the type ID.
     * \return the object TypeID
     */
    static TypeId GetTypeId(void);

    /**
     * \ brief calculate the power spectral density of the transmitted signal
     */
    void CalTxPsd();

    /**
     * \ brief attach the terahertz net device to this terahertz physical layer
     */
    void SetDevice(Ptr<THzNetDevice> device);

    /**
     * \ return the value of m_device flag.
     */
    Ptr<THzNetDevice> GetDevice();

    /**
     * \ brief attach the terahertz MAC layer to this terahertz phisical layer.
     */
    void SetMac(Ptr<THzMac> mac);

    /**
     * \ brief attach the terahertz channel to this terahertz physical layer
     */
    void SetChannel(Ptr<THzChannel> channel);

    /**
     * \ brief set up transmission power
     *
     * \ param dBm the tansmission power unit in dBm
     */
    void SetTxPower(double dBm);

    /**
     * \brief get terahertz channel
     *
     * \ return the value of m_channel flag.
     */
    Ptr<THzChannel> GetChannel();

    /**
     * \brief get MAC address
     *
     * \ return the Mac48Address
     *
     * The format of the string is "xx:xx:xx:xx:xx:xx"
     */
    Mac48Address GetAddress();

    /**
     * \brief get the transmission power
     *
     * \ return the transmission power in dBm
     */
    double GetTxPower();

    /**
     * \brief get the data rate of the control packet
     */
    uint32_t GetBasicRate();

    /**
     * \brief get the data rate of the DATA packet
     */
    double GetDataRate(int mcs);

    /**
     * \param packet packet sent from above down to terahertz physical layer.
     * \param rate transmission rate of packets in bps.
     *
     * Called from higher layer (MAC layer) to send packet into physical layer to the specified
     * destination Address. Pass the packet to lower layer (channel).
     */
    bool SendPacket(Ptr<Packet> packet, bool rate, uint16_t mcs);

    /**
     * \param packet packet sent out from the terahertz channel.
     *
     * Called from terahertz channel to indicate the packet has been sent out.
     * Terahertz physical layer need to pass this message to the upper layer (terahertz MAC layer).
     */
    void SendPacketDone(Ptr<Packet> packet);

    /**
     * \param packet packet received from lower layer (terahertz channel).
     * \param txDuration transmission duration of this packet.
     * \param rxPower power strength of the received packet.
     *
     * Called from terahertz channel to indicate the packet is been receiving by the receiver.
     * Terahertz physical layer need to pass this message to the upper layer (terahertz MAC layer)
     */
    void ReceivePacket(Ptr<Packet> packet, Time txDuration, double_t rxPower);

    /**
     * \param packet packet received from lower layer (terahertz channel).
     * \param rxPower power strength of the received packet.
     *
     * Called from terahertz channel to indicate the packet is been completely received by the
     * receiver. Terahertz physical layer need to pass this message to the upper layer (terahertz
     * MAC layer)
     */
    void ReceivePacketDone(Ptr<Packet> packet, double rxPower);

    /**
     * \return true if the physical layer is in a IDLE state and sense carrier not busy, false other
     * wise.
     */
    bool IsIdle();

    /**
     * \param basicSize the size of the control packet
     * \param dataSize the size of the DATA packet
     * \param mcs the Modulation Coding Scheme chosen. 0 for default
     *
     * \return the time duration for transmitting a packet.
     */
    Time CalTxDuration(uint32_t basicSize, uint32_t dataSize, uint8_t mcs);

    /**
     * \param dbm input value in dBm.
     *
     * Transfer the unit of input value from dBm to Watt.
     * \return the value in Watt.
     */
    double DbmToW(double dbm);

  private:
    typedef enum
    {
        IDLE,
        TX,
        RX,
        COLL
    } State;

    State m_state;
    std::string StateToString(State state);
    Ptr<THzNetDevice> m_device;
    Ptr<THzMac> m_mac;
    Ptr<THzChannel> m_channel;
    Ptr<SpectrumValue> m_txPsd;

    Ptr<Packet> m_pktRx;
    Time m_preambleDuration; //!< Duration (us) of Preamble of PHY Layer
    uint32_t m_trailerSize;  //!< Size of Trailer (e.g. FCS) (bytes)
    uint32_t m_headerSize;   //!< Size of Header (bytes)

    double m_txPower;          //!< transmission power (dBm)
    double m_numberOfSamples;  //!< number of frequency samples of the 3dB frequency band
    double m_numberOfSubBands; //!< uumber of the sub-bands within the 3dB frequency band
    double m_subBandBandwidth; //!< band width of the sub-bands
    double m_sinrTh;           //!< SINR threshold
    double m_csTh;             //!< carrier sense threshold (dBm)
    double m_basicRate;        //!< data rate of the control packet (bps)
    double m_dataRate;         //!< data rate of the DATA packet (bps)

    bool m_csBusy;
    Time m_csBusyEnd;

    bool m_daEnable;
    double m_dataRateBSPK;
    double m_dataRateQSPK;
    double m_dataRate8SPK;
    double m_dataRate16QAM;
    double m_dataRate64QAM;

    std::list<OngoingRx> m_ongoingRx;

  protected:
};

} // namespace ns3

#endif // THZ_PHY_MACRO_H
