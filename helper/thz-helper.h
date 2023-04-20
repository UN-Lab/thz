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


#ifndef THZ_HELPER_H_
#define THZ_HELPER_H_

#include <string>
#include "ns3/attribute.h"
#include "ns3/object-factory.h"
#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/thz-net-device.h"

namespace ns3 {
class THzMac;
class THzPhy;
class THzChannel;
class THzDirectionalAntenna;

/**
 * \brief create MAC objects
 *
 * This base class must be implemented by new MAC implementation which wish to integrate
 * with the \ref ns3::THzHelper class.
 */
class THzMacHelper
{
public:
  virtual ~THzMacHelper ();
  /**
   * \returns a new MAC object.
   *
   * Subclasses must implement this method to allow the ns3::THzHelper class
   * to create MAC objects from ns3::THzHelper::Install.
   */
  virtual Ptr<THzMac> Create (void) const = 0;
};
/**
 * \brief create PHY objects
 *
 * This base class must be implemented by new PHY implementation which wish to integrate
 * with the \ref ns3::THzHelper class.
 */
class THzPhyHelper
{
public:
  virtual ~THzPhyHelper ();
  /**
   * \returns a new PHY object.
   *
   * Subclasses must implement this method to allow the ns3::THzHelper class
   * to create PHY objects from ns3::THzHelper::Install.
   */
  virtual Ptr<THzPhy> Create (void) const = 0;
};
/**
 * \brief create Antenna objects
 *
 * This base class must be implemented by new Antenna implementation which wish to integrate
 * with the \ref ns3::THzHelper class.
 */
class THzDirAntennaHelper
{
public:
  virtual ~THzDirAntennaHelper ();
  /**
   * \returns a new Antenna object.
   *
   * Subclasses must implement this method to allow the ns3::THzHelper class
   * to create Antenna objects from ns3::THzHelper::Install.
   */
  virtual Ptr<THzDirectionalAntenna> Create (void) const = 0;
};

/**
 * \brief helps to create THzNetDevice objects
 *
 * This class can help to create a large set of similar
 * THzNetDevice objects and to configure a large set of
 * their attributes during creation.
 */
class THzHelper
{
public:
  /**
   * Create a THz helper in an empty state: all its parameters
   * must be set before calling ns3::THzHelper::Install
   */
  THzHelper ();
  virtual ~THzHelper ();
  /**
   * \param c the set of nodes on which a THz device must be created
   * \param channel the channel helper to create channel objects
   * \param phyHelper the PHY helper to create PHY objects
   * \param macHelper the MAC helper to create MAC objects
   * \param dirantennaHelper the antenna helper to create antenna
   * \returns a device container which contains all the devices created by this method.
   */
  NetDeviceContainer Install (NodeContainer c, Ptr<THzChannel> channel, const THzPhyHelper &phyHelper, const THzMacHelper &macHelper, const THzDirAntennaHelper &dirantennaHelper) const;
private:
  ObjectFactory m_mac;
  ObjectFactory m_phy;
  ObjectFactory m_dirantenna;
};


} //end namespace ns3

#endif /* THZ_HELPER_H_ */
