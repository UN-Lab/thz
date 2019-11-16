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


#ifndef THZ_SPECTRUM_SIGNAL_PARAMETERS_H
#define THZ_SPECTRUM_SIGNAL_PARAMETERS_H


#include <ns3/ptr.h>
#include <ns3/nstime.h>
#include "ns3/spectrum-signal-parameters.h"

namespace ns3 {

class THzPhy;
class SpectrumValue;
class Packet;


struct THzSpectrumSignalParameters : public SpectrumSignalParameters
{
  /**
   * default constructor
   */
  THzSpectrumSignalParameters ();

  /**
   * destructor
   */
  virtual ~THzSpectrumSignalParameters ();

  // inherited from SpectrumSignalParameters
  virtual Ptr<SpectrumSignalParameters> Copy ();

  THzSpectrumSignalParameters (const THzSpectrumSignalParameters& p);
  /**
   * The power spectral density of the transmitted signal.
   */
  Ptr <SpectrumValue> txPsd;

  /**
   * The duration of the packet transmission.
   */
  Time txDuration;

  /**
   * The Phy instance that is making the transmission
   */
  Ptr<THzPhy> txPhy;

  /**
   * The data packet being transmitted with this signal
   */
  Ptr<Packet> packet;
  /**
   * The transmission power.
   */
  double txPower;
  /**
   * The number of frequency samples from the database.
   */
  double numberOfSamples;
  /**
   * The number of sub-bands from the database.
   */
  double numberOfSubBands;
  /**
   * The bandwidth of individual sub-band.
   */
  double subBandBandwidth;
  /**
   * The center frequency of the overall signal band.
   */
  double centerFrequency;
  /**
   * The total bandwidth of the signal.
   */
  double totalBandwidth;
};


}




#endif /* THZ_SPECTRUM_SIGNAL_PARAMETERS_H */
