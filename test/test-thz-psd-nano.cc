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
#include "ns3/test.h"
#include "ns3/double.h"
#include "ns3/thz-spectrum-propagation-loss.h"
#include "ns3/thz-spectrum-signal-parameters.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/thz-spectrum-waveform.h"
#include <ns3/spectrum-value.h>
#include "ns3/gnuplot.h"
#include "ns3/core-module.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("THzPsdNanoTestSuite");

class THzPsdNanoTestCase : public TestCase
{
public:
  THzPsdNanoTestCase ();
  ~THzPsdNanoTestCase ();
  void DoRun (void);
  double DbmToW (double dbm);
};

THzPsdNanoTestCase::THzPsdNanoTestCase ()
  : TestCase ("terahertz Rx PSD Nano test case")
{
}

THzPsdNanoTestCase::~THzPsdNanoTestCase ()
{
}

double
THzPsdNanoTestCase::DbmToW (double dbm)
{
  double mw = pow (10.0,dbm / 10.0);
  return mw / 1000.0;
}

void
THzPsdNanoTestCase::DoRun ()
{
  LogComponentEnable ("THzSpectrumPropagationLoss", LOG_LEVEL_ALL);
  std::string fileNameWithNoExtension = "thz-received-power-spectral-density-nano";
  std::string graphicsFileName        = fileNameWithNoExtension + ".png";
  std::string plotFileName            = fileNameWithNoExtension + ".plt";
  //std::string plotTitle               = "THz received signal power spectral density for nanoscale communication";
  Gnuplot plot (graphicsFileName);
  //plot.SetTitle (plotTitle);
  plot.SetLegend ("Frequency [THz]", "p.s.d. [Watts/Hz]");
  plot.AppendExtra ("set grid xtics ytics");

  Ptr<THzSpectrumPropagationLoss> lossModel = CreateObject<THzSpectrumPropagationLoss> ();

  Config::SetDefault ("ns3::THzSpectrumValueFactory::NumSample", DoubleValue (1000));
  Gnuplot2dDataset dataset;
  dataset.SetTitle ("Transmitted pulse p.s.d. for nanoscale");
  dataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);

  double txPowerDbm = -20;  //dBm
  double pulseDuration = 100e-15;  //100 femtoseconds
  double distance = 0.1;  //m
  Ptr<SpectrumValue> txPsd;
  Ptr<SpectrumValue> rxPsd;
  double txPowerW = DbmToW (txPowerDbm);
  Ptr<THzSpectrumValueFactory> sf = CreateObject<THzSpectrumValueFactory> ();

  Ptr<SpectrumModel> InitTHzPulseSpectrumWaveform;

  InitTHzPulseSpectrumWaveform = sf->THzPulseSpectrumWaveformInitializer ();
  txPsd = sf->CreatePulsePowerSpectralDensity (1, pulseDuration, txPowerW);

  Ptr<MobilityModel> a = CreateObject<ConstantPositionMobilityModel> ();
  a->SetPosition (Vector (0,0,0));
  Ptr<MobilityModel> b = CreateObject<ConstantPositionMobilityModel> ();
  b->SetPosition (Vector (distance, 0, 0));
  rxPsd = lossModel->CalcRxPowerSpectralDensity (txPsd, a, b);

  Values::iterator vit = txPsd->ValuesBegin ();
  Bands::const_iterator fit = txPsd->ConstBandsBegin ();

  while (vit != txPsd->ValuesEnd ())
    {
      NS_ASSERT (fit != txPsd->ConstBandsEnd ());
      dataset.Add (fit->fc / 1e12, *vit);

      ++vit;
      ++fit;
    }

  plot.AddDataset (dataset);

  std::ofstream plotFile (plotFileName.c_str ());

  plot.GenerateOutput (plotFile);
  plotFile.close ();
}

class THzPsdNanoTestSuite : public TestSuite
{
public:
  THzPsdNanoTestSuite ();
};

THzPsdNanoTestSuite::THzPsdNanoTestSuite ()
  : TestSuite ("thz-rx-psd-nano", UNIT)
{
  AddTestCase (new THzPsdNanoTestCase, TestCase::QUICK);
}
//create an instance of the test suite
static THzPsdNanoTestSuite g_thzPsdNanoTestSuite;


