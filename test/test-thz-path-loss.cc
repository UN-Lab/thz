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


#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/double.h"
#include "ns3/thz-spectrum-propagation-loss.h"
#include "ns3/thz-spectrum-signal-parameters.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/thz-spectrum-waveform.h"
#include <ns3/spectrum-value.h>
#include "ns3/gnuplot.h"



using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("THzPathLossTestSuite");

class THzPathLossTestCase : public TestCase
{
public:
  THzPathLossTestCase();
  ~THzPathLossTestCase();
  void DoRun (void);
  double DbmToW (double dbm);
};

THzPathLossTestCase::THzPathLossTestCase()
  : TestCase("Terahertz Path Loss test case")
{  
}
THzPathLossTestCase::~THzPathLossTestCase()
{
}


double
THzPathLossTestCase::DbmToW (double dbm)
{
  double mw = pow (10.0,dbm / 10.0);
  return mw / 1000.0;
}

void
THzPathLossTestCase::DoRun()
{

  LogComponentEnable ("THzSpectrumPropagationLoss", LOG_LEVEL_ALL);
  std::string fileNameWithNoExtension = "thz-path-loss-for-nanoscale-pulse-based-waveform";
  std::string graphicsFileName        = fileNameWithNoExtension + ".png";
  std::string plotFileName            = fileNameWithNoExtension + ".plt";
  //std::string plotTitle               = "THz propagation loss vs distance for nanoscale communication";

  Gnuplot plot (graphicsFileName);
  //plot.SetTitle (plotTitle);
  plot.SetLegend ("Distance (m)", "Recieved Power (dBm)");
  plot.AppendExtra("set grid xtics ytics");

  Ptr<THzSpectrumPropagationLoss> lossModel = CreateObject<THzSpectrumPropagationLoss> ();

  Gnuplot2dDataset dataset;
  dataset.SetTitle ("THz propagation loss for nanoscale pulse based waveform");
  dataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);

  double txPowerDbm = -20;  //dBm
  double totalGainDb = 0;  //dB
  double pulseDuration = 100e-15;  //100 femtoseconds
  Ptr<SpectrumValue> m_txPsd;  //transmitted signal power spectral density

  double txPowerW = DbmToW (txPowerDbm);
  Ptr<THzSpectrumValueFactory> sf = CreateObject<THzSpectrumValueFactory> ();
  Ptr<SpectrumModel> InitTHzPulseSpectrumWaveform;

  InitTHzPulseSpectrumWaveform = sf->THzPulseSpectrumWaveformInitializer ();
  m_txPsd = sf->CreatePulsePowerSpectralDensity (1, pulseDuration, txPowerW);

  Ptr<THzSpectrumSignalParameters> txParams = Create<THzSpectrumSignalParameters> ();
  txParams->txDuration = Seconds (0);
  txParams->txPower = txPowerW;
  txParams->numberOfSamples = sf->m_numsample;
  txParams->numberOfSubBands = sf->m_numsb;
  txParams->subBandBandwidth = sf->m_sbw;
  //txParams->txPhy = 0;//GetObject<THzPhy> ();
  txParams->txPsd = m_txPsd;
  //txParams->packet = 0;

  Ptr<MobilityModel> a = CreateObject<ConstantPositionMobilityModel> ();
  a->SetPosition (Vector (0,0,0));
  Ptr<MobilityModel> b = CreateObject<ConstantPositionMobilityModel> ();
  for (double distance = 0.0001; distance <= 1.0; distance *= 10.0)
    {
      b->SetPosition (Vector (distance, 0, 0));
      double rxPowerDbm = lossModel->CalcRxPowerDA (txParams, a, b, totalGainDb);

      std::printf ("Rx power for distance %fm is %f\n",distance, rxPowerDbm);
      dataset.Add (10 * std::log10 (distance), rxPowerDbm);
    }

  plot.AddDataset (dataset);

  std::ofstream plotFile (plotFileName.c_str ());

  plot.GenerateOutput (plotFile);
  plotFile.close ();

}

class THzPathLossTestSuite : public TestSuite
{
public:
  THzPathLossTestSuite();
};

THzPathLossTestSuite::THzPathLossTestSuite ()
  : TestSuite("thz-path-loss", UNIT)
{
  AddTestCase (new THzPathLossTestCase, TestCase::QUICK);
}
// create an instance of the test suite
THzPathLossTestSuite g_thzPathLossTestSuite;
