/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) YEAR COPYRIGHTHOLDER
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
 */
#include <ns3/object.h>
#include <ns3/simulator.h>
#include <ns3/log.h>
#include <ns3/packet.h>
#include <ns3/node.h>
#include <ns3/double.h>
#include <ns3/mobility-model.h>
#include <ns3/mobility-helper.h>
#include <ns3/thz-dir-antenna.h>
#include <ns3/angles.h>
#include "ns3/core-module.h"
#include "ns3/config-store.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/nstime.h"
#include "ns3/thz-phy-macro.h"
#include "ns3/thz-mac-macro.h"
#include "ns3/thz-channel.h"
#include "ns3/thz-spectrum-waveform.h"
#include "ns3/thz-mac-macro-helper.h"
#include "ns3/thz-phy-macro-helper.h"
#include "ns3/thz-directional-antenna-helper.h"

#include "ns3/thz-udp-server.h"
#include "ns3/thz-udp-client.h"
#include "ns3/thz-udp-trace-client.h"
#include "ns3/thz-udp-client-server-helper.h"

#include "ns3/traffic-generator.h"
#include "ns3/traffic-generator-helper.h"

#include "ns3/random-variable-stream.h"
#include "ns3/rng-seed-manager.h"
#include <vector>
#include <iostream>
#include <cmath>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MacroCentral");

int main (int argc, char* argv[])
{

  int seed_num = 1;

  RngSeedManager seed;
  seed.SetSeed (seed_num);
  std::printf ("seed_num = %d\n", seed.GetSeed ());

  //LogComponentEnable("THzDirectionalAntenna", LOG_LEVEL_ALL);
  //LogComponentEnable("THzNetDevice", LOG_LEVEL_ALL);
  //LogComponentEnable("THzMacMacro", LOG_LEVEL_ALL);
  //LogComponentEnable("THzPhyMacro", LOG_LEVEL_ALL);
  //LogComponentEnable("THzChannel", LOG_LEVEL_ALL);
  //LogComponentEnable ("THzUdpClient", LOG_LEVEL_ALL);
  //LogComponentEnable ("THzUdpServer", LOG_LEVEL_ALL);

  int node_num = 10;
  uint8_t SNodes = 1;
  uint8_t CNodes = node_num;
  NodeContainer Servernodes;
  Servernodes.Create (SNodes);
  NodeContainer Clientnodes;
  Clientnodes.Create (CNodes);
  std::printf ("node_num = %d\n", Clientnodes.GetN ());
  NodeContainer nodes;
  nodes.Add (Servernodes);
  nodes.Add (Clientnodes);

  //---------------------------------MOBILITY-------------------------------------//
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (Servernodes);

  mobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
                                 "X", DoubleValue (0.0),
                                 "Y", DoubleValue (0.0),
                                 "rho", DoubleValue (10));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (Clientnodes);

  //------------------------------------CONNECT ALL-------------------------------------------//

  Ptr<THzChannel> thzChan = CreateObject<THzChannel> ();

  THzMacMacroHelper thzMac = THzMacMacroHelper::Default ();

  bool rtsOn = 1;
  std::printf ("rts on? %d\n", rtsOn);
  if (rtsOn == true)
    {
      thzMac.Set ("EnableRts",StringValue ("1"));
    }
  else
    {
      thzMac.Set ("EnableRts",StringValue ("0"));
    }

  THzPhyMacroHelper thzPhy = THzPhyMacroHelper::Default ();
  THzDirectionalAntennaHelper thzDirAntenna = THzDirectionalAntennaHelper::Default ();

  THzHelper thz;
  NetDeviceContainer devices = thz.Install (nodes, thzChan, thzPhy, thzMac, thzDirAntenna);

  Config::SetDefault ("ns3::THzSpectrumValueFactory::NumSubBand", DoubleValue (98));
  Config::SetDefault ("ns3::THzSpectrumValueFactory::NumSample", DoubleValue (1));



  //------------------SETUP NETWORK LAYER-------------------//
  InternetStackHelper internet;
  internet.Install (nodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer iface = ipv4.Assign (devices);


  //-----------------------------------PopulateArpCache---------------------------------------------//

  Ptr<ArpCache> arp = CreateObject<ArpCache> ();
  arp->SetAliveTimeout (Seconds (3600)); 
  for (uint16_t i = 0; i < nodes.GetN (); i++)
    {
      Ptr<Ipv4L3Protocol> ip = nodes.Get (i)->GetObject<Ipv4L3Protocol> ();
      NS_ASSERT (ip != 0);
      int ninter = (int) ip->GetNInterfaces ();
      for (int j = 0; j < ninter; j++)
        {
          Ptr<Ipv4Interface> ipIface = ip->GetInterface (j);
          NS_ASSERT (ipIface != 0);
          Ptr<NetDevice> device = ipIface->GetDevice ();
          NS_ASSERT (device != 0);
          Mac48Address addr = Mac48Address::ConvertFrom (device->GetAddress ());
          for (uint32_t k = 0; k < ipIface->GetNAddresses (); k++)
            {
              Ipv4Address ipAddr = ipIface->GetAddress (k).GetLocal ();
              if (ipAddr == Ipv4Address::GetLoopback ())
                {
                  continue;
                }
              ArpCache::Entry * entry = arp->Add (ipAddr);
              
              Ipv4Header ipHeader;
              Ptr<Packet> packet = Create<Packet> ();
              packet->AddHeader (ipHeader);
              
              entry->MarkWaitReply (ArpCache::Ipv4PayloadHeaderPair (packet, ipHeader));
              entry->MarkAlive (addr);

            }
        }
    }
  for (uint16_t i = 0; i < nodes.GetN (); i++)
    {
      Ptr<Ipv4L3Protocol> ip = nodes.Get (i)->GetObject<Ipv4L3Protocol> ();
      NS_ASSERT (ip != 0);
      int ninter = (int) ip->GetNInterfaces ();
      for (int j = 0; j < ninter; j++)
        {
          Ptr<Ipv4Interface> ipIface = ip->GetInterface (j);
          ipIface->SetArpCache (arp);
        }
    }

  //-----------------------------------End of ARP table-----------------------------------------------//

  THzUdpServerHelper Server (9);
  ApplicationContainer Apps = Server.Install (Servernodes);
  Apps.Start (Seconds (0.0));
  Apps.Stop (Seconds (10.0));


  THzUdpClientHelper Client (iface.GetAddress (0), 9);
  Client.SetAttribute ("PacketSize", UintegerValue (15000));
  Client.SetAttribute ("Mean", DoubleValue (22));
  Apps = Client.Install (Clientnodes);
  Apps.Start (Seconds (0.0));
  Apps.Stop (Seconds (10.0));

  Simulator::Stop (Seconds (10 + 0.000001));
  ConfigStore config;
  config.ConfigureDefaults ();
  config.ConfigureAttributes ();
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;

}
