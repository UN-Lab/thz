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
 *         Daniel Morales <danimoralesbrotons@gmail.com>
 */

#ifndef THZ_PHY_NANO_H
#define THZ_PHY_NANO_H

#include "ns3/simulator.h"
#include "ns3/event-id.h"
#include "thz-mac.h"
#include "thz-phy.h"
#include "ns3/traced-value.h"
#include "ns3/mobility-model.h"

#include "ns3/nstime.h"
#include <list>
#include <ns3/thz-spectrum-waveform.h>

namespace ns3 {
/**
 * \ingroup thz
 * \class THzPhyNano
 * \brief THzPhyNano models the physical layer for nanoscale THz communication.
 *
 * This class represents the physical layer for nanoscale THz
 * communication based on TSOOK, a modulation scheme based on the
 * transmission of hundred-femtosecond long pulses spread in time.
 * In addition, it allows the node to interleave multiple transmissions
 * and receptions. In case of collision between two receptions, it
 * calculates the SINR and accepts the packet if SINR is greater than the threshold.
 */
class THzPhyNano : public THzPhy
{
  /** Data structure for tracking ongoing transmissions */
  typedef struct
  {
    Time m_txStart; // start time of the ongoing transmission
    Time m_txDuration; //duration of the ongoing transmission
  } OngoingTx;
  /** Data structure for tracking ongoing receptions */
  typedef struct
  {
    Time m_rxStart; // start time of the ongoing transmission
    Time m_rxDuration; //duration of the ongoing transmission
    Ptr<Packet> packet;
    bool m_collided; //true if collided
    double_t rxPower;
    double interference;
  } OngoingRx;


public:
  /**
   * Register this type.
   * \return The TypeId.
   */
  static TypeId GetTypeId (void);
  /**
   * \brief Create a THzPhyNano
   */
  THzPhyNano ();
  /**
   * \brief Destroy a THzPhyNano
   */
  virtual ~THzPhyNano ();
  void Clear ();
  /**
   * \brief Create the pulse waveform that will be used in transmissions
   *
   * Generates the power spectral density of the hundred-femtosecond long pulse
   */
  void CalTxPsd ();
  /**
   * \ brief attach the terahertz net device to this terahertz physical layer.
   */
  void SetDevice (Ptr<THzNetDevice> device);

  /**
   * \ brief attach the terahertz MAC layer to this terahertz physical layer.
   */
  void SetMac (Ptr<THzMac> mac);

  /**
   * \ brief attach the terahertz channel to this terahertz physical layer.
   */
  void SetChannel (Ptr<THzChannel> channel);

  /**
   * Set the transmit power.
   *
   * \param dBm Final output transmission power, in dBm.
   */
  void SetTxPower (double dBm);
  /**
   * \ return the value of m_device flag.
   */
  Ptr<THzNetDevice> GetDevice ();
  /**
   * Get the current transmit power, in dBm.
   *
   * \return The transmit power.
   */
  double GetTxPower ();
  /**
   * \ return the value of m_channel flag.
   */
  Ptr<THzChannel> GetChannel ();
  /**
   * \ return the Mac48Address
   *
   * The format of the string is "xx:xx:xx:xx:xx:xx"
   */
  Mac48Address GetAddress ();
  /**
   *
   * \return The receive power threshold.
   */
  double GetRxPowerTh ();

  /**
   * \brief Interleave the new transmission with existing transmissions and schedule the transmission start
   *
   * \param packet A reference to the packet that will be transmitted
   * \param rate a boolean value that allows to choose between two transmission rates
   * \return True unless there is no slot left to interleave
   */
  bool SendPacket (Ptr<Packet> packet, bool rate, uint16_t mcs);
  /**
   * \brief Indicates that the Phy has finished transmitting
   * the packet over the channel
   *
   */
  void SendPacketDone (Ptr<Packet> packet);
  /**
   * \brief Indicates that the Phy has started receiving the first bit of
   * the packet from the channel
   *
   * The Phy will first check if the reception overlaps with  ongoing transmission.
   * If there is an overlap, it will drop the reception, otherwise it will keep
   * receiving the packet. Then Phy will check if it overlaps with ongoing receptions.
   * If there is no overlap, Phy will receive the packet. If there is an overlap,
   * Phy calculates the SINR.  If the SINR is greater than the threshold, Phy marks
   * the packet as collided, otherwise not collided.
   * \param packet A reference to the packet being received
   * \param txDuration Transmission time of the packet being received
   * \param rxPower Received signal power of the packet
   */
  void ReceivePacket (Ptr<Packet> packet, Time txDuration, double_t rxPower);
  /**
   * \brief Indicates that the Phy has started receiving the first bit of
   * the packet from the channel
   *
   * Passes the packet to upper layer if not collided, else drops the packet.
   *
   * \param packet A reference to the packet being received
   */
  void ReceivePacketDone (Ptr<Packet> packet, double rxPower); //transmission time is over
  /**
   * \return the time duration for transmitting a packet.
   */
  Time CalTxDuration (uint32_t basicSize, uint32_t dataSize, uint8_t mcs);
  /**
   * \brief Sort the array in ascending order
   *
   * \param timeArray The array to be sorted.
   * \param n Length of the array.
   */
  double* SortArray (double timeArray[], int n);
private:
  void DeleteOngoingTx (OngoingTx ot); //delete ot from list after its
  /**
   * \brief Schedule transmitting the packet to channel
   *
   * \param packet A reference to the packet that will be transmitted.
   * \param txDuration The transmission duration of the packet.
   */
  void ScheduleSendPacket (Ptr<Packet> packet, Time txDuration);
  /**
   * \param dbm input value in dBm.
   *
   * Convert input value from dBm to Watt.
   * \return the value in Watt.
   */
  double DbmToW (double dbm);
  Ptr<THzNetDevice> m_device;
  Ptr<THzMac> m_mac;
  Ptr<THzChannel> m_channel;
  Ptr<SpectrumValue> m_txPsd;

  Time Ts;
  Time m_pulseDuration;
  double m_beta;

  double m_txPower;  // transmission power (dBm)
  double m_numberOfSamples; // number of frequency samples in the waveform
  double m_numberOfSubBands;
  double m_subBandBandwidth;
  double m_sinrTh;   // SINR threshold

  std::list<OngoingTx> m_ongoingTx;
  std::list<OngoingRx> m_ongoingRx;

protected:
};

} // namespace ns3

#endif // THZ_PHY_H
