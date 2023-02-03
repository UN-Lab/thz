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

#ifndef THZ_CHANNEL_H
#define THZ_CHANNEL_H

#include "ns3/channel.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "thz-spectrum-signal-parameters.h"
#include "ns3/thz-spectrum-propagation-loss.h"
#include "ns3/propagation-delay-model.h"
#include "thz-net-device.h"
#include "thz-phy.h"
#include "thz-dir-antenna.h"
#include "ns3/mac48-address.h"
#include <list>
#include <vector>

namespace ns3 {

class THzSpectrumPropagationLoss;
class PropagationDelayModel;

/**
 * \brief A Terahertz Channel
 * \ingroup thz
 *
 * This thz channel implements the propagation model descripted in J.M. Jornet and I.F. Akyildiz,
 * "Channel Modeling and Capacity Analysis for Electromagnetic Wireless Nanonetworks in the Terahertz Band"
 *
 * This class is expected to be used in tandem with the THzPhyMacro/Nano and ThzDirAntenna
 */

class THzChannel : public Channel
{
  /**
   * typedef for a list of NoiseEntry
   *
   * This information will be used by PHY layer to obtain SINR value.
   */
  typedef struct
  {
    Ptr<Packet> packet;   //!< The packet been transmitted
    Ptr<THzPhy> phy;      //!< The PHY supports the packet transmission
    Time txDuration;      //!< Transmission time for the packet
    Time txEnd;           //!< time when packet transmission finished
    double_t rxPower;
  } NoiseEntry;

public:
  /**
   * Create a THzChannel
   */
  THzChannel ();

  /**
   * \internal
   * Destroy a THzChannel
   */
  virtual ~THzChannel ();
  static TypeId GetTypeId ();

  virtual std::size_t GetNDevices () const;
  virtual Ptr<NetDevice> GetDevice (std::size_t i) const;
  void AddDevice (Ptr<THzNetDevice> dev, Ptr<THzPhy> phy);
  void Clear ();

  /**
   * \brief send packet from terahertz channel with directional antenna.
   *
   * \param txParams the data structure of the transmitted packet.
   *
   * in this function a NoiseEntry will be updated, which recoreds the signel strength a node will receive
   * and how long will the signal exist, this information will be used by terahertz physical layer to calculate SINR.
   * Also in this function, it checks the antenna mode for sending node and receiving node, by this process it
   * checking the receiver's orientation and thus the total antenna gain between transmitter and receiver.
   * Note that the sending node is not necessarily to be the transmitter, e.g., node sending CTS or ACK packet is actually the receiver.
   */
  bool SendPacket (Ptr<THzSpectrumSignalParameters> txParams);

  /**
   * \brief calculate the overall noise and interference.
   *
   * \param interference the value of interference.
   */
  double GetNoiseW (double interference);

  /**
   * \brief convert the value from dBm to Watt.
   *
   * \param dbm the value in dBm.
   */
  double DbmToW (double dbm);

private:
  /**
   * \brief send packet done in terahertz channel.
   *
   * \param phy the teraherz PHY related to the packet.
   * \param packet the packet been sent.
   */
  void SendPacketDone (Ptr<THzPhy> phy, Ptr<Packet> packet);

  /**
   * \brief receive packet from terahertz channel.
   *
   * \param i the recording of device ID of the receiver.
   * \param ne the noise entry.
   *
   * Upon receive packet from terahertz channel, the node updates the noiseEntry for terahertz physical layer SINR calculation.
   */
  void ReceivePacket (uint32_t i, NoiseEntry ne);

  /**
   * \brief receive packet done from terahertz channel.
   *
   * \param i the recording of device ID of the receiver.
   * \param ne the noise entry.
   *
   * Note that if concurrent transmissions end at the same time, some of them can be missed from SINR calculation, so delete
   * a noise entry a few seconds later.
   */
  void ReceivePacketDone (uint32_t i, NoiseEntry ne);

  /**
   * \brief delete the noise entry.
   *
   * \param ne the noise entry.
   */
  void DeleteNoiseEntry (NoiseEntry ne);
  double m_noiseFloor;
  double m_Rxorientation;
  double m_totalGain;

  double m_XnodeMode;  //!< Antenna mode of the Xnode of the node pair (X-Y)
  double m_YnodeMode;  //!< Antenna mode of the Ynode of the node pair (X-Y)
  Ptr<THzSpectrumPropagationLoss> m_loss;
  Ptr<ConstantSpeedPropagationDelayModel> m_delay;
  Ptr<THzDirectionalAntenna> m_thzDA;
  Ptr<THzNetDevice> m_sendDev;

  Address m_send_check;
  Mac48Address m_send_48_check;
  Address m_add_check;
  Mac48Address m_add_48_check;
  Mac48Address m_add_48_recv;
  Ptr<THzDirectionalAntenna> m_dirantenna;
  std::list<Ptr<MobilityModel> > m_recMobList;

  /**
   * A vector of pointers to THzDeviceList
   */
  typedef std::vector<std::pair<Ptr<THzNetDevice>, Ptr<THzPhy> > > THzDeviceList;
  THzDeviceList m_devList;
  std::list<NoiseEntry> m_noiseEntry;

protected:
};

}

#endif // THZ_CHANNEL_H
