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

#ifndef THZ_MAC_NANO_HELPER_H
#define THZ_MAC_NANO_HELPER_H

#include <string>
#include "thz-helper.h"

namespace ns3 {

/**
 * \brief create THz MAC layers for a ns3::THzNetDevice.
 *
 */
class THzMacNanoHelper : public THzMacHelper
{
public:
  /**
   * Create a THzMacNanoHelper
   */
  THzMacNanoHelper ();
  /**
   * \internal
   * Destroy a THzMacNanoHelper
   */
  virtual ~THzMacNanoHelper ();
  /**
   * Create a MAC helper in a default working state.
   */
  static THzMacNanoHelper Default (void);
  /**
   * Set the underlying type of the MAC and its attributes.
   *
   * \param type the type of ns3::WifiMac to create.
   * \param n0 the name of the attribute to set
   * \param v0 the value of the attribute to set
   * \param n1 the name of the attribute to set
   * \param v1 the value of the attribute to set
   * \param n2 the name of the attribute to set
   * \param v2 the value of the attribute to set
   * \param n3 the name of the attribute to set
   * \param v3 the value of the attribute to set
   * \param n4 the name of the attribute to set
   * \param v4 the value of the attribute to set
   * \param n5 the name of the attribute to set
   * \param v5 the value of the attribute to set
   * \param n6 the name of the attribute to set
   * \param v6 the value of the attribute to set
   * \param n7 the name of the attribute to set
   * \param v7 the value of the attribute to set
   *
   * All the attributes specified in this method should exist
   * in the requested MAC.
   */
  void SetType (std::string type,
                std::string n0 = "", const AttributeValue &v0 = EmptyAttributeValue (),
                std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue (),
                std::string n5 = "", const AttributeValue &v5 = EmptyAttributeValue (),
                std::string n6 = "", const AttributeValue &v6 = EmptyAttributeValue (),
                std::string n7 = "", const AttributeValue &v7 = EmptyAttributeValue ());
  /**
   * \param n the name of the attribute to set
   * \param v the value of the attribute to set
   *
   * Set these attributes on each ns3::THzMacNano created
   * by THzMacNanoHelper
   */
  void Set (std::string n = "", const AttributeValue &v = EmptyAttributeValue ());

private:
  /**
   * Create a THzMac using mac factory.
   */
  virtual Ptr<THzMac> Create (void) const;
  /** THz mac factory. */
  ObjectFactory m_mac;
};

} //namespace ns3

#endif /* THZ_MAC_CSMA_HELPER_H */
