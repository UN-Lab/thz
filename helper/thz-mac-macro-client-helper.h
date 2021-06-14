/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 University at Buffalo, the State University of New York
 * (http://ubnano.tech/)
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

#ifndef THZ_MAC_MACRO_CLIENT_HELPER_H
#define THZ_MAC_MACRO_CLIENT_HELPER_H

#include <string>
#include "thz-helper.h"

namespace ns3 {

/**
 * \brief create the THz MAC layer for macro-scale scenario
 *
 * This class can create MACs of type ns3::THzMacMacro.
 */
class THzMacMacroClientHelper : public THzMacHelper
{
public:
  /**
   * Create a THzMacMacroClientHelper that is used to enable the THz MAC layer
   * protocol of the macro-scale scenario.
   */
  THzMacMacroClientHelper ();

  /**
   * \internal
   * Destroy a THzMacMacroClientHelper
   */
  virtual ~THzMacMacroClientHelper ();

  /**
   * Create a THz MAC helper for macro-scale scenario in a default working state.
   */
  static THzMacMacroClientHelper Default (void);

  /**
   * Set the underlying type of the THz MAC layer protocol for macro-scale scenario
   * and its attributes.
   *
   * \param type the type of ns3::THzMacMacro to create.
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
   * All the attributes specified in this method should exist in the requested MAC.
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
   * Set the attribute of the of the THz MAC layer protocol for macro-scale scenario.
   *
   * \param n the name of the attribute to set
   * \param v the value of the attribute to set
   */
  void Set (std::string n = "", const AttributeValue &v = EmptyAttributeValue ());

private:
  /**
   * \internal
   * \returns a newly-created THzMacMacro object.
   */
  virtual Ptr<THzMac> Create (void) const;

  /**
   * \MAC object factory.
   */
  ObjectFactory m_mac;
};

} //namespace ns3

#endif /* THZ_MAC_MACRO_CLIENT_HELPER_H */
