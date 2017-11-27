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
 * Author: Zahed Hossain <zahedhos@buffalo.edu>
 *         Qing Xia <qingxia@buffalo.edu>
 *         Josep Miquel Jornet <jmjornet@buffalo.edu>
 */

#ifndef TRAFFIC_GENERATOR_HELPER_H
#define TRAFFIC_GENERATOR_HELPER_H

#include <string>
#include "ns3/attribute.h"
#include "ns3/object-factory.h"
#include "ns3/node-container.h"
#include "ns3/traffic-generator.h"
#include "ns3/application-container.h"

namespace ns3 {
/**
 * \ingroup thz
 * \brief A helper to make it easier to instantiate an ns3::TrafficGenerator
 * on a set of nodes.
 */
class TrafficGeneratorHelper
{
public:
  /**
   * Create a TrafficGeneratorHelper
   */
  TrafficGeneratorHelper ();
  /**
   * Destroy a TrafficGeneratorHelper
   */
  virtual ~TrafficGeneratorHelper ();
  /**
   * Helper function used to set the underlying application attributes.
   *
   * \param name the name of the application attribute to set
   * \param value the value of the application attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);
  /**
   * Install an ns3::TrafficGenerator on each node of the input container
   * configured with all the attributes set with SetAttribute.
   *
   * \param c NodeContainer of the set of nodes on which an TrafficGenerator
   * will be installed.
   * \returns Container of Ptr to the applications installed.
   */
  ApplicationContainer Install (NodeContainer c);

private:
  ObjectFactory m_traffic;
};


} //end namespace ns3

#endif /* THZ_HELPER_H_ */
