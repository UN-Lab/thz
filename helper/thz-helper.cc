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

#include "ns3/mobility-model.h"
#include "ns3/log.h"
#include "ns3/config.h"
#include "ns3/simulator.h"
#include "ns3/mac48-address.h"
#include "thz-helper.h"
#include "ns3/thz-mac.h"
#include "ns3/thz-phy.h"
#include "ns3/thz-channel.h"
#include "ns3/thz-dir-antenna.h"

#include <sstream>
#include <string>

NS_LOG_COMPONENT_DEFINE ("THzHelper");

namespace ns3 {

THzMacHelper::~THzMacHelper ()
{
}

THzPhyHelper::~THzPhyHelper ()
{
}

THzDirAntennaHelper::~THzDirAntennaHelper ()
{
}

THzHelper::THzHelper ()
{
}

THzHelper::~THzHelper ()
{
}

NetDeviceContainer
THzHelper::Install (NodeContainer c, Ptr<THzChannel> channel, const THzPhyHelper &phyHelper, const THzMacHelper &macHelper, const THzDirAntennaHelper &dirantennaHelper) const
{
  NetDeviceContainer devices;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
    {
      Ptr<Node> node = *i;
      Ptr<THzNetDevice> device = CreateObject<THzNetDevice> ();

      Ptr<THzMac> mac = macHelper.Create ();
      Ptr<THzPhy> phy = phyHelper.Create ();
      Ptr<THzDirectionalAntenna> dirantenna = dirantennaHelper.Create ();
      mac->SetAddress (Mac48Address::Allocate ());
      device->SetMac (mac);
      device->SetPhy (phy);
      device->SetChannel (channel);
      device->SetDirAntenna (dirantenna);

      node->AddDevice (device);
      devices.Add (device);

      NS_LOG_DEBUG ("node=" << node << ", mob=" << node->GetObject<MobilityModel> ());
    }
  return devices;
}

} //end namespace ns3
