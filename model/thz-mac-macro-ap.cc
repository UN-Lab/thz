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

#include "thz-mac-macro-ap.h"

#include "thz-dir-antenna.h"
#include "thz-mac-header.h"
#include "thz-net-device.h"
#include "thz-phy-macro.h"

#include "ns3/attribute.h"
#include "ns3/boolean.h"
#include "ns3/core-module.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdio.h>
#include <string>
#include <time.h>
#include <vector>

NS_LOG_COMPONENT_DEFINE("THzMacMacroAp");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(THzMacMacroAp);

THzMacMacroAp::THzMacMacroAp()
    : THzMac(),
      m_phy(0),
      m_state(IDLE),
      m_ccaTimeoutEvent(),
      m_backoffTimeoutEvent(),
      m_ctsTimeoutEvent(),
      m_ackTimeoutEvent(),
      m_sendCtsEvent(),
      m_sendDataEvent(),
      m_retry(0),
      m_pktData(0)

{
    m_nav = Simulator::Now();
    m_localNav = Simulator::Now();
    m_backoffRemain = PicoSeconds(0);
    m_backoffStart = PicoSeconds(0);
    m_ite = 0;
    m_retry = 0;
    m_throughputAll = 0;
    m_state = IDLE;
    m_discard = 0;
    m_send = 0;
    m_angle = 0;
    m_ackList.clear();
    m_expectedData = 0;
    m_dummyCycles = 0;
    Simulator::ScheduleNow(&THzMacMacroAp::Init, this);
}

THzMacMacroAp::~THzMacMacroAp()
{
    Clear();
}

void
THzMacMacroAp::Clear()
{
    m_pktTx = 0;
    m_pktData = 0;
    m_pktQueue.clear();
    m_seqList.clear();
    m_pktRec = 0;
    m_throughput = 0;
    m_throughputAll = 0;
}

TypeId
THzMacMacroAp::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::THzMacMacroAp")
            .SetParent<THzMac>()
            .AddConstructor<THzMacMacroAp>()
            .AddAttribute("HandshakeWays",
                          "Number of control packets interchanged as handshake",
                          UintegerValue(3),
                          MakeUintegerAccessor(&THzMacMacroAp::m_ways),
                          MakeUintegerChecker<uint16_t>())
            .AddAttribute("BoSlots",
                          "Slots for Start Backoff",
                          UintegerValue(5),
                          MakeUintegerAccessor(&THzMacMacroAp::m_boSlots),
                          MakeUintegerChecker<uint32_t>())
            .AddAttribute("SlotTime",
                          "Time slot duration for MAC backoff",
                          TimeValue(NanoSeconds(2)),
                          MakeTimeAccessor(&THzMacMacroAp::m_slotTime),
                          MakeTimeChecker())
            .AddAttribute("SlotTime3way",
                          "Time slot duration for MAC backoff for 3-way",
                          TimeValue(NanoSeconds(2)),
                          MakeTimeAccessor(&THzMacMacroAp::m_slotTime_3way),
                          MakeTimeChecker())
            .AddAttribute("SifsTime",
                          "Short Inter-frame Space",
                          TimeValue(PicoSeconds(0)),
                          MakeTimeAccessor(&THzMacMacroAp::m_sifs),
                          MakeTimeChecker())
            .AddAttribute("DifsTime",
                          "DFS Inter-frame Space",
                          TimeValue(PicoSeconds(0)),
                          MakeTimeAccessor(&THzMacMacroAp::m_difs),
                          MakeTimeChecker())
            .AddAttribute("OutputFile",
                          "name of the output file",
                          StringValue("result_macro-central.txt"),
                          MakeStringAccessor(&THzMacMacroAp::outputFile),
                          MakeStringChecker())
            .AddAttribute("PacketSize",
                          "Minimum packet size",
                          UintegerValue(15000),
                          MakeUintegerAccessor(&THzMacMacroAp::m_packetSize),
                          MakeUintegerChecker<uint32_t>())
            .AddAttribute("PropDelay",
                          "default time of propagation for r=10m",
                          TimeValue(PicoSeconds(33356)),
                          MakeTimeAccessor(&THzMacMacroAp::m_tProp),
                          MakeTimeChecker())
            .AddAttribute("UseWhiteList",
                          "Activate the use of a white list for sectors",
                          BooleanValue(true),
                          MakeBooleanAccessor(&THzMacMacroAp::m_useWhiteList),
                          MakeBooleanChecker())
            .AddAttribute("UseAdaptMCS",
                          "Activate the use of a adaptive MCS mechanism",
                          BooleanValue(true),
                          MakeBooleanAccessor(&THzMacMacroAp::m_useAdaptMCS),
                          MakeBooleanChecker())
            .AddAttribute("CS_BPSK",
                          "Carrier sense threshold for this MCS",
                          DoubleValue(-48),
                          MakeDoubleAccessor(&THzMacMacroAp::csth_BPSK),
                          MakeDoubleChecker<double>())
            .AddAttribute("CS_QPSK",
                          "Carrier sense threshold for this MCS",
                          DoubleValue(-45),
                          MakeDoubleAccessor(&THzMacMacroAp::csth_QPSK),
                          MakeDoubleChecker<double>())
            .AddAttribute("CS_8PSK",
                          "Carrier sense threshold for this MCS",
                          DoubleValue(-42),
                          MakeDoubleAccessor(&THzMacMacroAp::csth_8PSK),
                          MakeDoubleChecker<double>())
            .AddAttribute("CS_16QAM",
                          "Carrier sense threshold for this MCS",
                          DoubleValue(-38),
                          MakeDoubleAccessor(&THzMacMacroAp::csth_16QAM),
                          MakeDoubleChecker<double>())
            .AddAttribute("CS_64QAM",
                          "Carrier sense threshold for this MCS",
                          DoubleValue(-32),
                          MakeDoubleAccessor(&THzMacMacroAp::csth_64QAM),
                          MakeDoubleChecker<double>())
            .AddTraceSource("CtsTimeout",
                            "Trace Hookup for CTS Timeout",
                            MakeTraceSourceAccessor(&THzMacMacroAp::m_traceCtsTimeout),
                            "ns3::THzMac::TimeTracedCallback")
            .AddTraceSource("AckTimeout",
                            "Trace Hookup for ACK Timeout",
                            MakeTraceSourceAccessor(&THzMacMacroAp::m_traceAckTimeout),
                            "ns3::THzMac::TimeTracedCallback")
            .AddTraceSource("SendDataDone",
                            "Trace Hookup for sending a data",
                            MakeTraceSourceAccessor(&THzMacMacroAp::m_traceSendDataDone),
                            "ns3::THzMac::SendDataDoneTracedCallback")
            .AddTraceSource("Enqueue",
                            "Trace Hookup for enqueue a data",
                            MakeTraceSourceAccessor(&THzMacMacroAp::m_traceEnqueue),
                            "ns3::THzMac::TimeTracedCallback")
            .AddTraceSource("Throughput",
                            "Trace Hookup for Throughput",
                            MakeTraceSourceAccessor(&THzMacMacroAp::m_traceThroughput),
                            "ns3::THzMac::ThroughputTracedCallback");
    return tid;
}

bool
THzMacMacroAp::Enqueue(Ptr<Packet> packet, Mac48Address dest)
{
    return false;
}

void
THzMacMacroAp::Init(void)
{
    m_thzAD = m_device->GetDirAntenna();
    m_beamwidth = m_thzAD->GetBeamwidth();
    m_thzAD->SetBeamwidth(m_beamwidth);                    // to set m_exponent
    m_thzAD->SetAttribute("TuneRxTxMode", DoubleValue(1)); // set as receiver
    m_thzAD->SetAttribute("InitialAngle", DoubleValue(0));

    m_tData = GetDataDuration(m_packetSize, 0);

    m_tSector = GetCtrlDuration(THZ_PKT_TYPE_CTS) + m_tProp + GetSifs() + GetMaxBackoff() +
                m_tData + m_tProp + GetSifs() + GetCtrlDuration(THZ_PKT_TYPE_ACK) + NanoSeconds(10);
    m_nSector = 360 / m_beamwidth;
    m_tMaxCircle = m_nSector * m_tSector;
    m_turningSpeed = ((double)1 / (double)m_tMaxCircle.GetNanoSeconds()) * 1e9;
    m_thzAD->SetRxTurningSpeed(m_turningSpeed);
    NS_LOG_DEBUG("tSector: " << m_tSector
                             << " tCircle: " << m_tMaxCircle
                             << " turning speed " << m_turningSpeed);
    m_nodeId = m_device->GetNode()->GetId();

    m_recordNodeSector = false;
    if (m_useWhiteList)
    {
        m_recordNodeSector = true; // If using White List (WL), record the sector/s of each node
    }

    Simulator::ScheduleNow(&THzMacMacroAp::TurnRxAntenna, this);
}

void
THzMacMacroAp::TurnRxAntenna(void)
{
    m_angle = m_angle + m_beamwidth; // add degrees for next turn
    while (m_angle <= -360)
    {
        m_angle += 360;
    }
    while (m_angle > 360)
    {
        m_dummyCycles++;
        m_angle -= 360;
        CycleRecord();
    }

    m_thzAD->TuneRxOrientation(m_angle); // turn to next sector

    if (m_ways == 3)
    {
        SendCta3(); // 3-way
    }
    else
    {
        m_sectorTimeoutEvent = Simulator::Schedule(m_tSector, &THzMacMacroAp::SectorTimeout, this);
        SendCta1(); // 1-way
    }
}

void
THzMacMacroAp::SendCta1()
{
    Ptr<Packet> packet = Create<Packet>(0);
    THzMacHeader ctaHeader = THzMacHeader(m_address, GetBroadcast(), THZ_PKT_TYPE_CTA);
    ctaHeader.SetFlags(0);
    Time dataTimeout = GetCtrlDuration(THZ_PKT_TYPE_CTS) + m_tProp + GetSifs() + GetMaxBackoff() +
                       m_tProp + NanoSeconds(1);
    m_dataTimeoutEvent = Simulator::Schedule(dataTimeout, &THzMacMacroAp::DataTimeout, this);

    NS_LOG_UNCOND(Simulator::Now() << " - AP - CTS generated"
                                   << " at node " << m_nodeId << " Data T/O in: " << dataTimeout);

    packet->AddHeader(ctaHeader);
    SendPacket(packet, 0);
}

void
THzMacMacroAp::SendCta3()
{
    Ptr<Packet> packet = Create<Packet>(0);
    THzMacHeader ctaHeader = THzMacHeader(m_address, GetBroadcast(), THZ_PKT_TYPE_CTA);
    ctaHeader.SetSector(m_angle);

    if (m_recordNodeSector)
    {
        ctaHeader.SetFlags(1); // Flags = 1: Request answer from all nodes
        NS_LOG_DEBUG(Simulator::Now() << " - AP - CTA Flags = " << ctaHeader.GetFlags());
    }
    else
    {
        ctaHeader.SetFlags(0);
    }

    Time waitTime = GetCtrlDuration(THZ_PKT_TYPE_CTA) + m_tProp + GetSifs() + GetMaxBackoff() +
                    m_tProp + GetCtrlDuration(THZ_PKT_TYPE_RTS) + NanoSeconds(1);
    m_waitTimeEvent = Simulator::Schedule(waitTime, &THzMacMacroAp::WaitTimeExpired, this);
    packet->AddHeader(ctaHeader);
    SendPacket(packet, 0);
    NS_LOG_UNCOND(Simulator::Now()
                  << " - AP - CTA sent. RTS Timeout started, expires in " << waitTime);
}

void
THzMacMacroAp::SendCts(Mac48Address dest, uint16_t sequence, Time duration, uint16_t flag)
{
    Ptr<Packet> packet = Create<Packet>(0);
    THzMacHeader ctsHeader = THzMacHeader(m_address, dest, THZ_PKT_TYPE_CTS);
    ctsHeader.SetSequence(sequence);
    ctsHeader.SetDuration(duration);
    ctsHeader.SetFlags(flag);

    packet->AddHeader(ctsHeader);
    SendPacket(packet, 0);
    NS_LOG_UNCOND(Simulator::Now() << " - AP - Sending CTS to: " << dest << ". MCS " << flag);
}

void
THzMacMacroAp::DataTimeout()
{
    m_sectorTimeoutEvent.Cancel();
    NS_LOG_UNCOND(Simulator::Now() << " - AP - DATA TIMEOUT. Turning to next sector");
    TurnRxAntenna();
}

void
THzMacMacroAp::WaitTimeExpired() // Wait Time will always expire. Check how many RTS were received
{
    // If Zero RTS have been received, turn to the next sector
    if (m_rtsList.empty())
    {
        NS_LOG_UNCOND(Simulator::Now()
                      << " - AP - ---------- No RTS received, turning to next sector at "
                      << m_angle + m_beamwidth << " ----------");
        TurnRxAntenna();
        return;
    }

    // If creating the white list, record node-sector information
    if (m_recordNodeSector)
    {
        NS_LOG_UNCOND(Simulator::Now() << " - AP - Wait Time Expired. Received " << m_rtsList.size()
                                       << ". Sector: " << m_angle);
        if (m_sectorMap.find(m_angle) == m_sectorMap.end())
        {
            m_sectorMap.insert(std::make_pair(
                m_angle,
                std::vector<std::pair<Mac48Address, double>>())); // Create map entry for the sector
        }

        THzMacHeader header;
        Ptr<Packet> rts;
        std::list<std::pair<Ptr<Packet>, double>>::iterator it = m_rtsList.begin();
        for (; it != m_rtsList.end(); it++) // RTS from all nodes in the sector have been received, because it was indicated in CTA flag
        {
            rts = it->first;
            rts->PeekHeader(header);
            bool already_exists = false;
            std::vector<std::pair<Mac48Address, double>>::iterator it2 = m_sectorMap[m_angle].begin();
            for (; it2 != m_sectorMap[m_angle].end(); it2++)
            {
                if (it2->first == header.GetSource())
                {
                    already_exists = true;
                    break;
                }
            }
            if (!already_exists)
            {
                m_sectorMap[m_angle].push_back(std::make_pair(header.GetSource(), it->second));
            }
        }

        std::vector<std::pair<Mac48Address, double>>::iterator it3 = m_sectorMap[m_angle].begin();
        for (; it3 != m_sectorMap[m_angle].end(); it3++)
        {
            NS_LOG_UNCOND(it3->first << " with power " << it3->second);
        }
        m_rtsList.clear();

        if (m_dummyCycles >= 3)
        {
            m_recordNodeSector = false;
            InitNodeMap();
        }
        else
        {
            TurnRxAntenna();
        }
        return;
    }

    // BASE STATION LOGIC: Answer All RTS. If Adaptive MCS enabled, check power to adapt MCS

    NS_LOG_UNCOND(Simulator::Now()
                  << " - AP - Wait Time Expired. Received " << m_rtsList.size() << " RTSs.");

    std::list<std::pair<Ptr<Packet>, double>>::iterator it = m_rtsList.begin();
    int i = 0;
    Time wait = m_rtsList.size() * GetCtrlDuration(THZ_PKT_TYPE_CTS); // wait time to send DATA

    // Iterate through all received RTS. Answer a CTS with information on MCS and wait time until transmission
    for (; it != m_rtsList.end(); it++)
    {
        Ptr<Packet> rts = it->first;
        THzMacHeader header;
        rts->PeekHeader(header);

        int flag = 0;
        if (m_useAdaptMCS)
        {
            flag = SelectMCS(it->second); // it->second contains the RTS received power. Select MCS depending on power
        }

        Time sendAfter = (GetCtrlDuration(THZ_PKT_TYPE_CTS) + PicoSeconds(1)) * i; // wait time to send CTS
        i++;
        Simulator::Schedule(sendAfter,
                            &THzMacMacroAp::SendCts,
                            this,
                            header.GetSource(),
                            header.GetSequence(),
                            wait,
                            flag);
        wait = wait + GetDataDuration(m_packetSize, flag) + GetMaxBackoff(); // wait time to send DATA for the next node
    }
    m_expectedData = i;
    Time sectorTime = 2 * m_tProp + GetSifs() +
                      (GetCtrlDuration(THZ_PKT_TYPE_CTS) + m_tData + GetMaxBackoff() +
                       GetCtrlDuration(THZ_PKT_TYPE_ACK)) * m_expectedData + GetSifs();
    m_sectorTimeoutEvent = Simulator::Schedule(
        sectorTime,
        &THzMacMacroAp::SectorTimeout,
        this); // start a sector T/O as a maximum sector time just in case DATA is not received. If DATA is received successfully, it will be cancelled

    NS_LOG_DEBUG(Simulator::Now() << " - AP - Sector timeout event scheduled in " << sectorTime);
    m_rtsList.clear();
}

void
THzMacMacroAp::SectorTimeout()
{
    if (!m_ackList.empty())
    {
        NS_LOG_UNCOND(Simulator::Now() << " - AP - ---------- SECTOR TIMEOUT. Sending "
                                       << m_ackList.size() << " ACKs ----------");
        SendAck();
    }
    else
    {
        NS_LOG_UNCOND(Simulator::Now() << " - AP - ---------- SECTOR TIMEOUT. No correct data received. Turning to next sector at "
                                       << m_angle + m_beamwidth << " ----------");
        TurnRxAntenna();
    }
}

int
THzMacMacroAp::SelectMCS(double power)
{
    if (power > csth_64QAM)
    {
        return 14;
    }
    if (power > csth_16QAM)
    {
        return 13;
    }
    if (power > csth_8PSK)
    {
        return 12;
    }
    if (power > csth_QPSK)
    {
        return 11;
    }
    if (power > csth_BPSK)
    {
        return 10;
    }
    return 0;
}

Time
THzMacMacroAp::GetMaxBackoff()
{
    return GetSlotTime() * m_boSlots;
}

void
THzMacMacroAp::ReceiveData(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION("at node " << m_nodeId);
    THzMacHeader header;
    packet->RemoveHeader(header);

    if (header.GetDestination() == GetBroadcast())
    {
        NS_LOG_UNCOND("ERROR: no broadcast DATA packets should be sent");
    }
    if (header.GetDestination() != m_address) // destined not to me
    {
        NS_LOG_UNCOND("ERROR: all data should be destined to the one AP");
    }

    NS_LOG_UNCOND(Simulator::Now() << " - AP - DATA received. Seq: " << header.GetSequence());

    // Create ACK and push it into ACK List. All ACKs are to be sent in succession after all DATA transmissions are done
    Ptr<Packet> ack = Create<Packet>(0);
    THzMacHeader ackHeader = THzMacHeader(m_address, header.GetSource(), THZ_PKT_TYPE_ACK);
    ackHeader.SetSequence(header.GetSequence());
    ack->AddHeader(ackHeader);
    m_ackList.push_back(ack);
    m_state = IDLE;

    // If no more DATA is expected, send ACKs
    if (m_expectedData == m_ackList.size() || m_ways != 3)
    {
        m_sectorTimeoutEvent.Cancel();
        m_state = WAIT_TX;
        SendAck();
    }

    if (IsNewSequence(header.GetSource(), header.GetSequence()))
    {
        m_forwardUpCb(packet, header.GetSource(), header.GetDestination());
    }
}

void
THzMacMacroAp::ReceiveRts(Ptr<Packet> packet, double rxPower)
{
    NS_LOG_DEBUG("AP - RTS Received. Now: " << Simulator::Now());
    m_rtsList.push_back(std::make_pair(packet, rxPower));
}

void
THzMacMacroAp::SendAck()
{
    std::list<Ptr<Packet>>::iterator it = m_ackList.begin();
    Ptr<Packet> ack = *it;
    THzMacHeader header;
    ack->PeekHeader(header);
    SendPacket(ack, 0); // send ACK
    m_ackList.remove(ack);
    NS_LOG_UNCOND(Simulator::Now() << " - AP - ACK sent to " << header.GetDestination()
                                   << ". Remaining " << m_ackList.size() << " ACKs");
}

bool
THzMacMacroAp::SendPacket(Ptr<Packet> packet, bool rate)
{
    NS_LOG_FUNCTION(" state " << m_state << " now " << Simulator::Now());

    if (m_state == IDLE || m_state == WAIT_TX)
    {
        if (m_phy->SendPacket(packet, rate, 0)) // AP always sends at default data rate (MCS = 0)
        {
            m_state = TX;
            m_pktTx = packet;
            return true;
        }
        else
        {
            m_state = IDLE;
        }
    }
    return false;
}

void
THzMacMacroAp::SendPacketDone(Ptr<Packet> packet)
{
    NS_LOG_DEBUG(Simulator::Now() << " - AP - SendPacketDone");
    if (m_state != TX || m_pktTx != packet)
    {
        NS_LOG_UNCOND("ERROR: Something is wrong!");
        return;
    }

    m_state = IDLE;
    THzMacHeader header;
    packet->PeekHeader(header);

    switch (header.GetType())
    {
    case THZ_PKT_TYPE_CTA:
        NS_LOG_DEBUG(Simulator::Now() << " - AP - CTA sent");
        break;

    case THZ_PKT_TYPE_RTS:
        break;

    case THZ_PKT_TYPE_CTS:
        NS_LOG_DEBUG(Simulator::Now() << " - AP - CTS sent");
        break;

    case THZ_PKT_TYPE_DATA:
        if (header.GetDestination() == GetBroadcast())
        {
            NS_LOG_UNCOND("ERROR: there should be no broadcast packets");
        }
        break;

    case THZ_PKT_TYPE_ACK:
        if (m_ackList.empty())
        {
            NS_LOG_UNCOND(Simulator::Now()
                          << " - AP - ---------- ACK sent. Turning to next sector at "
                          << m_angle + m_beamwidth << " ----------");
            Simulator::Schedule(NanoSeconds(1), &THzMacMacroAp::TurnRxAntenna, this);
        }
        else
        {
            Simulator::Schedule(PicoSeconds(1), &THzMacMacroAp::SendAck, this);
        }
        break;

    default:
        break;
    }
}

// phy:ReceivePacket -> Mac:ReceivePacket IF rxPower > threshold
void
THzMacMacroAp::ReceivePacket(Ptr<THzPhy> phy, Ptr<Packet> packet)
{
    if (m_dataTimeoutEvent.IsRunning())
    {
        m_dataTimeoutEvent.Cancel();
    }

    THzMacHeader header;
    packet->PeekHeader(header);
    NS_LOG_FUNCTION("at node " << m_nodeId << " from " << header.GetSource() << " now "
                               << Simulator::Now() << " state: " << StateToString(m_state));
    switch (m_state)
    {
    case WAIT_TX:
    case RX:
    case WAIT_ACK:
    case BACKOFF:
    case IDLE:
        m_state = RX;
        break;
    case TX:
    case COLL:
        break;
    }
}

void
THzMacMacroAp::ReceivePacketDone(Ptr<THzPhy> phy, Ptr<Packet> packet, bool success, double rxPower)
{
    NS_LOG_FUNCTION("at node " << m_nodeId << " success? " << success);
    m_state = IDLE;
    THzMacHeader header;
    packet->PeekHeader(header);
    NS_LOG_DEBUG(" AP - rxPower: " << rxPower);

    if (!success) // success: SINR > threshold
    {
        NS_LOG_DEBUG("The packet is not encoded correctly. Drop it!");
        return;
    }
    switch (header.GetType())
    {
    case THZ_PKT_TYPE_RTS:
        ReceiveRts(packet, rxPower);
        break;
    case THZ_PKT_TYPE_CTA:
    case THZ_PKT_TYPE_CTS:
    case THZ_PKT_TYPE_ACK:
        NS_LOG_UNCOND("ERROR: Received packed different than RTS or DATA");
        break;
    case THZ_PKT_TYPE_DATA:
        ReceiveData(packet);
        break;
    default:
        break;
    }
}

void
THzMacMacroAp::InitNodeMap()
{
    // For every sector
    std::map<double, std::vector<std::pair<Mac48Address, double>>>::iterator it;
    for (it = m_sectorMap.begin(); it != m_sectorMap.end(); it++)
    {
        // it->first is the sector
        // it->second is the vector of pairs <nodes, rxPower>
        // it2->first is the Mac48Address of the node
        // it2->second is the rxPower

        // Iterate thru nodes in the sector
        std::vector<std::pair<Mac48Address, double>>::iterator it2;
        for (it2 = it->second.begin(); it2 != it->second.end(); it2++)
        {
            // create map entry if non existant
            if (m_nodeMap.find(it2->first) == m_nodeMap.end())
            {
                m_nodeMap.insert(std::make_pair(it2->first, std::vector<std::pair<double, double>>()));
            }

            // push sector (value) into node (key)
            m_nodeMap[it2->first].push_back(std::make_pair(it->first, it2->second));
        }
    }

    // For every node
    int i = 0;
    std::map<Mac48Address, std::vector<std::pair<double, double>>>::iterator it4;
    for (it4 = m_nodeMap.begin(); it4 != m_nodeMap.end(); it4++)
    {
        NS_LOG_UNCOND("--- Node " << it4->first << " ---");

        double bestSector = 0;
        Mac48Address node_mac = it4->first;
        double rxPower = -200;
        std::vector<std::pair<double, double>>::iterator it5;
        for (it5 = it4->second.begin(); it5 != it4->second.end(); it5++)
        {
            // Select best sector
            if (it5->second > rxPower)
            {
                rxPower = it5->second;
                bestSector = it5->first;
            }
            NS_LOG_UNCOND(it5->first << ", " << it5->second);
        }

        // Create map entry if non existant
        if (m_whiteList.find(bestSector) == m_whiteList.end())
        {
            m_whiteList.insert(std::make_pair(bestSector, std::vector<Mac48Address>()));
        }
        // Assign best sector to node
        m_whiteList[bestSector].push_back(node_mac);
        NS_LOG_UNCOND("Inserted node " << node_mac << " into " << bestSector << " white list");

        // Notify node in which sector has to send
        i++;
        Simulator::Schedule((i - 1) * (GetCtrlDuration(THZ_PKT_TYPE_CTS) + NanoSeconds(1)),
                            &THzMacMacroAp::SendFeedbackCTA,
                            this,
                            bestSector,
                            node_mac);
    }
    Simulator::Schedule(i * (GetCtrlDuration(THZ_PKT_TYPE_CTS) + NanoSeconds(1)),
                        &THzMacMacroAp::TurnRxAntenna,
                        this);
}

void
THzMacMacroAp::SendFeedbackCTA(double angle, Mac48Address dest)
{
    NS_LOG_UNCOND(Simulator::Now() << " - AP - Sending Feedback CTA to node " << dest
                                   << ". Notify that his sector is " << angle);
    m_thzAD->TuneRxOrientation(angle);

    Ptr<Packet> packet = Create<Packet>(0);
    THzMacHeader ctaHeader = THzMacHeader(m_address, dest, THZ_PKT_TYPE_CTA);
    ctaHeader.SetSector(angle);
    ctaHeader.SetFlags(2); // Sector announced. No response required

    packet->AddHeader(ctaHeader);
    SendPacket(packet, 0);
}

// ------------------------ Set Functions -----------------------------
void
THzMacMacroAp::AttachPhy(Ptr<THzPhy> phy)
{
    m_phy = phy;
}

void
THzMacMacroAp::SetDevice(Ptr<THzNetDevice> dev)
{
    m_device = dev;
}

void
THzMacMacroAp::SetAddress(Mac48Address addr)
{
    NS_LOG_FUNCTION(addr);
    m_address = addr;
    // to help each node have different random seed
    uint8_t tmp[6];
    m_address.CopyTo(tmp);
}

void
THzMacMacroAp::SetForwardUpCb(Callback<void, Ptr<Packet>, Mac48Address, Mac48Address> cb)
{
    m_forwardUpCb = cb;
}

void
THzMacMacroAp::SetSlotTime(Time duration)
{
    m_slotTime = duration;
}

// ------------------------ Get Functions -----------------------------
Time
THzMacMacroAp::GetSlotTime(void)
{
    if (m_ways == 3)
    { // 3-way
        return m_slotTime_3way;
    }
    return m_slotTime;
}

Time
THzMacMacroAp::GetSifs(void) const
{
    return m_sifs;
}

Time
THzMacMacroAp::GetDifs(void) const
{
    return m_difs;
}

Mac48Address
THzMacMacroAp::GetAddress() const
{
    return this->m_address;
}

Mac48Address
THzMacMacroAp::GetBroadcast(void) const
{
    return Mac48Address::GetBroadcast();
}

Time
THzMacMacroAp::GetCtrlDuration(uint16_t type)
{
    THzMacHeader header = THzMacHeader(m_address, m_address, type);
    return m_phy->CalTxDuration(header.GetSize(), 0, 0);
}

Time
THzMacMacroAp::GetDataDuration(uint32_t size, uint8_t mcs)
{
    return m_phy->CalTxDuration(0, size, mcs);
}

std::string
THzMacMacroAp::StateToString(State state)
{
    switch (state)
    {
    case IDLE:
        return "IDLE";
    case BACKOFF:
        return "BACKOFF";
    case WAIT_TX:
        return "WAIT_TX";
    case TX:
        return "TX";
    case WAIT_ACK:
        return "WAIT_ACK";
    case RX:
        return "RX";
    case COLL:
        return "COLL";
    default:
        return "??";
    }
}

// --------------------------- ETC -------------------------------------
bool
THzMacMacroAp::IsNewSequence(Mac48Address addr, uint16_t seq)
{
    std::list<std::pair<Mac48Address, uint16_t>>::iterator it = m_seqList.begin();
    for (; it != m_seqList.end(); ++it)
    {
        if (it->first == addr)
        {
            if (seq > it->second)
            {
                it->second = seq;
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    std::pair<Mac48Address, uint16_t> newEntry;
    newEntry.first = addr;
    newEntry.second = seq;
    m_seqList.push_back(newEntry);
    return true;
}

void
THzMacMacroAp::CycleRecord()
{
    /*----------------------------------------------------------------------------------------
     * enable cycle time printing in a .txt file by uncommenting the content in this function
     *----------------------------------------------------------------------------------------*/
    /*
       std::stringstream txtname;
       txtname << "scratch/AP_cycle_" << outputFile;
       std::string filename = txtname.str ();

       std::ofstream resultfile;
       resultfile.open (filename.c_str (), std::ios::app);
       resultfile << Simulator::Now().GetNanoSeconds() << std::endl;
       resultfile.close ();
       return;
      */
}

} /* namespace ns3 */
