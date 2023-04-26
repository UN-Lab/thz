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

#include "ns3/config-store.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/core-module.h"
#include "ns3/double.h"
#include "ns3/gnuplot.h"
#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/thz-spectrum-propagation-loss.h"
#include "ns3/thz-spectrum-signal-parameters.h"
#include "ns3/thz-spectrum-waveform.h"
#include <ns3/spectrum-value.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("THzPsdMacroTestSuite");

class THzPsdMacroTestCase : public TestCase
{
  public:
    THzPsdMacroTestCase();
    ~THzPsdMacroTestCase();
    void DoRun(void);
    double DbmToW(double dbm);
};

THzPsdMacroTestCase::THzPsdMacroTestCase()
    : TestCase("terahertz PSD Macro test case")
{
}

THzPsdMacroTestCase::~THzPsdMacroTestCase()
{
}

double
THzPsdMacroTestCase::DbmToW(double dbm)
{
    double mw = pow(10.0, dbm / 10.0);
    return mw / 1000.0;
}

void
THzPsdMacroTestCase::DoRun()
{
    LogComponentEnable("THzSpectrumPropagationLoss", LOG_LEVEL_ALL);
    std::string fileNameWithNoExtension = "thz-received-power-spectral-density-macro";
    std::string graphicsFileName = fileNameWithNoExtension + ".png";
    std::string plotFileName = fileNameWithNoExtension + ".plt";

    Gnuplot plot(graphicsFileName);
    plot.SetLegend("Frequency [THz]", "p.s.d. [Watts/Hz]");
    plot.AppendExtra("set grid xtics ytics");

    Ptr<THzSpectrumPropagationLoss> lossModel = CreateObject<THzSpectrumPropagationLoss>();
    Config::SetDefault("ns3::THzSpectrumValueFactory::TotalBandWidth", DoubleValue(7.476812e10));
    Config::SetDefault("ns3::THzSpectrumValueFactory::NumSample", DoubleValue(1));

    Gnuplot2dDataset dataset;
    dataset.SetTitle("Transmitted signal p.s.d. for macroscale");
    dataset.SetStyle(Gnuplot2dDataset::LINES_POINTS);

    double txPowerDbm = -20; // [dBm] Transmit power
    double gain = 17.27;     // [dB] Gain
    double distance = 10;    // [m] Distance

    double txPowerW = DbmToW(txPowerDbm);
    gain = std::pow(10.0, gain / 10.0);

    Ptr<SpectrumValue> txPsd;
    Ptr<SpectrumValue> rxPsd;
    Ptr<THzSpectrumValueFactory> sf = CreateObject<THzSpectrumValueFactory>();
    Ptr<SpectrumModel> InitTHzSpectrumWave;
    Ptr<SpectrumModel> InitTHzSpectrumWaveAll;
    InitTHzSpectrumWave = sf->THzSpectrumWaveformInitializer();
    InitTHzSpectrumWaveAll = sf->AllTHzSpectrumWaveformInitializer();
    txPsd = sf->CreateTxPowerSpectralDensityMask(txPowerW);

    Ptr<MobilityModel> a = CreateObject<ConstantPositionMobilityModel>();
    a->SetPosition(Vector(0, 0, 0));
    Ptr<MobilityModel> b = CreateObject<ConstantPositionMobilityModel>();
    b->SetPosition(Vector(distance, 0, 0));
    rxPsd = lossModel->CalcRxPowerSpectralDensity(txPsd, a, b);

    Values::iterator vit = txPsd->ValuesBegin();
    Bands::const_iterator fit = txPsd->ConstBandsBegin();

    while (vit != txPsd->ValuesEnd())
    {
        NS_ASSERT(fit != txPsd->ConstBandsEnd());
        dataset.Add(fit->fc / 1e12, std::log10(*vit * 2 * gain));

        ++vit;
        ++fit;
    }
    plot.AddDataset(dataset);

    std::ofstream plotFile(plotFileName.c_str());

    plot.GenerateOutput(plotFile);
    plotFile.close();
}

class THzPsdMacroTestSuite : public TestSuite
{
  public:
    THzPsdMacroTestSuite();
};

THzPsdMacroTestSuite::THzPsdMacroTestSuite()
    : TestSuite("thz-psd-macro", UNIT)
{
    AddTestCase(new THzPsdMacroTestCase, TestCase::QUICK);
}

// Create an instance of the test suite
static THzPsdMacroTestSuite g_thzPsdMacroTestSuite;
