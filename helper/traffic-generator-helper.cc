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
 * Author: Zahed Hossain <zahedhos@buffalo.edu>
 *         Qing Xia <qingxia@buffalo.edu>
 *         Josep Miquel Jornet <jmjornet@buffalo.edu>
 */

#include "ns3/log.h"
#include "ns3/config.h"
#include "ns3/simulator.h"
#include "ns3/mac48-address.h"
#include "ns3/thz-mac.h"
#include "ns3/thz-phy.h"
#include "ns3/thz-channel.h"
#include "traffic-generator-helper.h"

#include <sstream>
#include <string>

NS_LOG_COMPONENT_DEFINE ("TrafficGeneratorHelper");

namespace ns3 {

TrafficGeneratorHelper::TrafficGeneratorHelper ()
{
  m_traffic.SetTypeId (TrafficGenerator::GetTypeId ());
}

TrafficGeneratorHelper::~TrafficGeneratorHelper ()
{
}

ApplicationContainer
TrafficGeneratorHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
    {
      Ptr<Node> node = *i;
      Ptr<TrafficGenerator> tg = m_traffic.Create<TrafficGenerator> ();
      tg->AddNodeContainer (c);

      node->AddApplication (tg);
      apps.Add (tg);
      NS_LOG_DEBUG ("node=" << node);
    }
  return apps;
}

void
TrafficGeneratorHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_traffic.Set (name, value);
}

} //end namespace ns3
