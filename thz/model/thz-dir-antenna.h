/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 UBNANO
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

#ifndef THZ_DIR_ANTENNA_H
#define THZ_DIR_ANTENNA_H

#include <ns3/boolean.h>
#include <ns3/simulator.h>
#include <ns3/object.h>
#include <ns3/mobility-model.h>
#include "ns3/nstime.h"
#include "thz-net-device.h"
#include "thz-phy.h"
#include "thz-dir-antenna.h"
#include "ns3/node.h"
#include <list>
#include <vector>
#include <cmath>

namespace ns3 {
/**
 * \defgroup Terahertz Directional Antenna Models
 *
 * \brief THz Directional Antenna Model
 *
 * This class implements the directional antenna with turning ability based on the cosine model as described in:
 * Li Chunjian, "Efficient Antenna Patterns for Three-Sector WCDMA Systems"
 *
 * This class is expected to be used in tandem with the THzChannel and ThzMacMacro
 */
class THzDirectionalAntenna : public Object
{
public:
  THzDirectionalAntenna ();
  virtual ~THzDirectionalAntenna ();
  static TypeId GetTypeId (void);
  void Clear ();

  /**
   * \brief enable the directional antenna on certain device
   *
   * \param device the related device 
   */
  void SetDevice (Ptr<THzNetDevice> device);
 
  /**
   * \brief check directional antenna mode
   *
   * if the mode is 0, the device is a directional transmitter; 
   * if the mode is 1, the device is a directional receiver;
   * if the mode is 2, the device is an omni-directional tranceiver;
   * Note that the setting of antenna mode between 1 and 2 is depend on a specific receiver-initiated handshake protocol,
   * the setting of mode 2 is for the terahertz nano-scale scenario. The detail MAC protocol is described in:
   * Q.Xia, Z.Hossain, M.Medley and J.M Jornet, "A Link-layer Synchronization and Medium Access Control Protocol for Terahertz-band Communication Networks,"
   */
  double CheckAntennaMode (void);

  /**
    * \param turnSpeed the turning speed of the directional antenna
    * 
    * \brief set the turning speed of the directional antenna on the receiver [circles/sec]
    *
    * in the receiver initiated handshake MAC protocol, the receiver's directional antenna periodically sweeps the entire area to avoid deafness problem.
    */
  void SetRxTurningSpeed (double turnSpeed);

  /**
   * \brief get the turning speed of the directional antenna on the receiver [circles/sec]
   */
  double GetRxTurningSpeed () const;

  /**
   * \param maxGain the maximum gain of directional antenna
   *
   * \brief set the maximum gain of the directional antennas for both transmitter and receiver [dB] 
   */
  void SetMaxGain (double maxGain);

  /**
    * \brief get the maximum gain of the directional antennas for both transmitter and receiver [dB]
    */
  double GetMaxGain () const;

  /**
    * \param beamwidthDegree the beamwidth of the directional antenna in degrees
    *
    * \brief set the beamwidth of the directional antennas for both transmitter and receiver [degrees]
    */
  void SetBeamwidth (double beamwidthDegrees);

  /**
    * \brief get the beamwidth of the directional antennas for both transmitter and receiver [degrees]
    */
  double GetBeamwidth () const;
  
  /**
    * \param phi_zero initial angle in phi-plane
    *
    * \brief tune the orientation of the receiver's directional antenna
    */
  void TuneRxOrientation (double phi_zero);

  /**
    * \brief check the orientation of the receiver's directional antenna based on user setting
    * 
    * returns a orientation of sector-by-sector turing directional antenna
    */
  double CheckRxOrientation ();

  /**
    * \brief check the orientation of the receiver's directional antenna based on time duration
    *
    * returns a orientation of smoothly turing directional antenna
    */
  double GetRxOrientation ();

  /**
    * \param phi_tx the orientation of the transmitter's directional antenna
    *
    * \brief record the orientation of the transmitter's directional antenna
    */
  void RecTxOrientation (double phi_tx);

  /**
   * \brief check the orientation of the transmitter's directional antenna 
   */   
  double CheckTxOrientation ();

  /**
    * \param senderMobility the mobility of the sender
    * \param recvMobility the mobility of the receiver
    *
    * \brief calculate the directional antenna's gain of the receiver [dB]
    */ 
  double GetRxGainDb (Ptr<MobilityModel> senderMobility, Ptr<MobilityModel> recvMobility);

  /**
    * \param senderMobility the mobility of the sender
    * \param recvMobility the mobility of the receiver
    *
    * \brief calculate the directional antenna's gain of the transmitter [dB]
    */
  double GetTxGainDb (Ptr<MobilityModel> senderMobility, Ptr<MobilityModel> recvMobility);

  /** 
    * \param XnodeMobility the mobility of one node of the node pair, which is denoted as X.
    * \param YnodeMobility the mobility of the other node of the node pair, which is denoted as Y.
    * \param XnodeMode the operation mode of the node X.
    * \param YnodeMode the operation mode of the node Y.
    * \param RxorientationRadians the orientation of the receiver node in radians.
    *
    * \brief calculate the total directional antenna's gain between transmitter and receiver [dB].
    * 
    * Antenna mode = 1 corresponding to Directional receiver.
    * Antenna mode = 0 corresponding to Directional transmitter.
    * Antenna mode = 2 corresponding to Omni-directional antenna.
    */
  double GetAntennaGain (Ptr<MobilityModel> XnodeMobility, Ptr<MobilityModel> YnodeMobility, bool XnodeMode, bool YnodeMode, double RxorientationRadians);


private:
  Ptr<THzNetDevice> m_device;
  Ptr<Node> m_node;

  double m_RxTxMode;

  double m_turnSpeed;
  double m_RxIniAngle;
  double m_exponent;
  double m_beamwidthRadians;
  double m_beamwidthDegrees;
  double m_TxorientationDegrees;
  double m_TxorientationRadians;
  double m_RxorientationDegrees;
  double m_RxorientationRadians;


  double m_phi_rx;
  double m_phi_tx;
  double m_maxGain;
  double m_rxgainDb;

  Time m_CurrentTime;
  Time m_SectorTime;

  EventId m_ScheduleTuneRxOrientation;
  EventId m_GetRxOrientation;
  EventId m_TxGetRxOrientation;
  EventId m_ScheduleNextCalc;
  EventId m_ScheduleGetRxGain;
  EventId m_ScheduleGetTxGain;
  EventId m_ScheduleAddRxGainDb;
  EventId m_ScheduleAddTxGainDb;

  double m_RxGain;
  double m_TxGain;
  double m_rxGain;
  double m_txGain;

};



} // namespace ns3


#endif // THZ_DIR_ANTENNA_H
