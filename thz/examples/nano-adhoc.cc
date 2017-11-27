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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/node.h"

#include "ns3/thz-mac-nano.h"
#include "ns3/thz-channel.h"
#include "ns3/thz-mac-nano-helper.h"
#include "ns3/thz-phy-nano-helper.h"
#include "ns3/thz-directional-antenna-helper.h"

#include "ns3/traffic-generator-helper.h"
#include "ns3/thz-energy-model-helper.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("NanoAdhoc");


int main (int argc, char* argv[])
{
  Time::SetResolution (Time::FS ); //picoseconds);
  RngSeedManager seed;
  seed.SetRun (5);
  //LogComponentEnable("THzMacNano", LOG_LEVEL_ALL);
  //LogComponentEnable("THzNetDevice", LOG_LEVEL_ALL);
  //LogComponentEnable("THzPhyNano", LOG_LEVEL_ALL);
  //LogComponentEnable("THzChannel", LOG_LEVEL_ALL);
  //LogComponentEnable("THzNetDevice", LOG_LEVEL_ALL);
  //LogComponentEnable("TrafficGenerator", LOG_LEVEL_ALL);
  //LogComponentEnable("THzEnergyModel", LOG_LEVEL_ALL);
  //LogComponentEnable("THzSpectrumValueFactory", LOG_LEVEL_ALL);
  //LogComponentEnable("THzSpectrumPropagationLossModel", LOG_LEVEL_ALL);

  uint8_t numNodes = 13;
  NodeContainer nodes;
  nodes.Create (numNodes);

  //***********************************Energy**********************************//
  THzEnergyModelHelper energy;
  energy.SetEnergyModelAttribute ("THzEnergyModelInitialEnergy",StringValue ("0.0"));
  energy.SetEnergyModelAttribute ("DataCallbackEnergy",DoubleValue (65));
  energy.Install (nodes);
  //***********************************Aggregation**********************************//
  Ptr<THzChannel> thzChan = CreateObject<THzChannel> ();
  THzMacNanoHelper thzMac = THzMacNanoHelper::Default ();

  bool rtsOn = 0;
  std::printf ("rts on? %d\n", rtsOn);
  if (rtsOn == true)
    {
      thzMac.Set ("EnableRts",StringValue ("1"));
    }
  else
    {
      thzMac.Set ("EnableRts",StringValue ("0"));
    }

  Config::SetDefault ("ns3::THzSpectrumValueFactory::NumSubBand", DoubleValue (5242));
  Config::SetDefault ("ns3::THzSpectrumValueFactory::NumSample", DoubleValue (10));
  THzPhyNanoHelper thzPhy = THzPhyNanoHelper::Default ();
  thzPhy.SetPhyAttribute ("PulseDuration", TimeValue (FemtoSeconds (100)));
  thzPhy.SetPhyAttribute ("Beta", DoubleValue (100));

  THzDirectionalAntennaHelper thzDirAntenna = THzDirectionalAntennaHelper::Default ();
  THzHelper thz;
  NetDeviceContainer devices = thz.Install (nodes, thzChan, thzPhy, thzMac,thzDirAntenna);
  //***********************************Mobility**********************************//
  MobilityHelper ue1mobility;
  ue1mobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
                                    "X", DoubleValue (0.0),
                                    "Y", DoubleValue (0.0),
                                    "rho", DoubleValue (0.01));
  ue1mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  ue1mobility.Install (nodes);
  //***********************************IP**********************************//
  InternetStackHelper internet;
  internet.Install (nodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer iface = ipv4.Assign (devices);

  //******************PopulateArpCache()*************************************************************
  Ptr<ArpCache> arp = CreateObject<ArpCache> ();
  arp->SetAliveTimeout (Seconds (3600));
  for (uint16_t i = 0; i < nodes.GetN (); i++) // n nodecontainer**************************************
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
              entry->MarkWaitReply (0);
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
  //***********************End of ARP table population**********************************

  TrafficGeneratorHelper Traffic;
  Traffic.SetAttribute ("Mean", DoubleValue (300));
  Traffic.SetAttribute ("PacketSize",UintegerValue (75));
  ApplicationContainer Apps = Traffic.Install (nodes);
  Apps.Start (MicroSeconds (200));
  Apps.Stop (MilliSeconds (2000));

  Simulator::Stop (MilliSeconds (100 + 0.000001));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
