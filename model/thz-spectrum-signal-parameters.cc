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


#include "thz-spectrum-signal-parameters.h"
#include "thz-phy.h"
#include <ns3/spectrum-value.h>
#include <ns3/log.h>
#include "ns3/packet.h"


NS_LOG_COMPONENT_DEFINE ("THzSpectrumSignalParameters");

namespace ns3 {

THzSpectrumSignalParameters::THzSpectrumSignalParameters ()
{
  NS_LOG_FUNCTION (this);
}

THzSpectrumSignalParameters::~THzSpectrumSignalParameters ()
{
  NS_LOG_FUNCTION (this);
}

THzSpectrumSignalParameters::THzSpectrumSignalParameters (const THzSpectrumSignalParameters& p)
{
  NS_LOG_FUNCTION (this << &p);
  txPsd = p.txPsd->Copy ();
  txPower = p.txPower;
  txDuration = p.txDuration;
  txPhy = p.txPhy;
  packet = p.packet->Copy ();
  numberOfSamples = p.numberOfSamples;
  numberOfSubBands = p.numberOfSubBands;
  subBandBandwidth = p.subBandBandwidth;
}

Ptr<SpectrumSignalParameters>
THzSpectrumSignalParameters::Copy ()
{
  NS_LOG_FUNCTION (this);

  Ptr<THzSpectrumSignalParameters> tssp (new THzSpectrumSignalParameters (*this), false);
  return tssp;
}

} // namespace ns3
