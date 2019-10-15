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


#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cmath>
#include <ns3/object.h>
#include <ns3/core-module.h>
#include "ns3/log.h"
#include "thz-spectrum-waveform.h"

NS_LOG_COMPONENT_DEFINE ("THzSpectrumValueFactory");

namespace ns3 {
const double PULSE_START_FREQUENCY = 0.1e12; //Hz
const double PULSE_END_FREQUENCY = 4e12;

NS_OBJECT_ENSURE_REGISTERED (THzSpectrumValueFactory);

TypeId
THzSpectrumValueFactory::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::THzSpectrumValueFactory")
    .SetParent<Object> ()
    .AddAttribute ("NumSubBand", 
                   "The number of sub-bands containing in the selected 3dB frequency window",
                   DoubleValue (98), 
                   MakeDoubleAccessor (&THzSpectrumValueFactory::m_numsb),
                   MakeDoubleChecker<int> ())
    .AddAttribute ("SubBandWidth", 
                   "The bandwidth of each sub-band",
                   DoubleValue (7.6294e8),
                   MakeDoubleAccessor (&THzSpectrumValueFactory::m_sbw),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TotalBandWidth", 
                   "The total bandwidth of the selected 3dB frequency window",
                   DoubleValue (7.4768e10),
                   MakeDoubleAccessor (&THzSpectrumValueFactory::m_tbw),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("CentralFrequency", 
                   "The central frequency of the selected 3dB frequency window",
                   DoubleValue (1.0345e+012),
                   MakeDoubleAccessor (&THzSpectrumValueFactory::m_fc),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("NumSample", 
                   "The number of sample bands of the selected 3dB frequency window",
                   DoubleValue (100),
                   MakeDoubleAccessor (&THzSpectrumValueFactory::m_numsample),
                   MakeDoubleChecker<int> ())

  ;
  return tid;
}
THzSpectrumValueFactory::THzSpectrumValueFactory ()
{
}
THzSpectrumValueFactory::~THzSpectrumValueFactory ()
{
}

Ptr<SpectrumModel>
THzSpectrumValueFactory::THzSpectrumWaveformInitializer ()
{
  m_numsb = m_tbw/m_sbw;
  m_fstart = m_fc - (m_numsb / 2) * m_sbw;
  NS_LOG_DEBUG("CHECK: THzSpectrumWaveformInitializer: m_numsb = "<< m_numsb);

  std:: ifstream frequencyfile;
  frequencyfile.open ("contrib/thz/model/data_frequency.txt", std::ifstream::in);
  if (!frequencyfile.is_open ())
    {
	  NS_LOG_UNCOND("THzSpectrumValueFactory::THzSpectrumWaveformInitializer: open data_frequency.txt failed");
    }

  double f_starV;                      // Detected frequency from the frequency database
  double f;
  int i = 1;                           // record the sequency of selected frequency value in database.
                                       //(used for finding the correspond absorption coefficient in another file)
  while (frequencyfile >> f)
    {
      if (f < m_fstart)
        {
          i++;
        }
      else
        {
          break;                        // if customer defined value is not corresponding to any value stored in database,
        }
      //just select the first value next to the customer defined frequency.
    }
  f_starV = f;

  Bands bands;

  for (int j = 0; j <= m_numsb; j++)
    {
      BandInfo bi;
      bi.fl = f_starV + (j - 0.5) * m_sbw;
      bi.fh = f_starV + (j + 0.5) * m_sbw;
      bi.fc = (bi.fl +  bi.fh) / 2;
      bands.push_back (bi);
    }
  m_THzSpectrumWaveform = Create<SpectrumModel> (bands);
  return m_THzSpectrumWaveform;
}

Ptr<SpectrumModel>
THzSpectrumValueFactory::AllTHzSpectrumWaveformInitializer ()
{
  std:: ifstream frequencyfile;
  frequencyfile.open ("contrib/thz/model/data_frequency.txt", std::ifstream::in);
  if (!frequencyfile.is_open ())
    {
      NS_LOG_UNCOND ( "THzSpectrumValueFactory::AllTHzSpectrumWaveformInitializer: open data_frequency.txt failed" );
    }

  double f;
  int i = 0;

  while (frequencyfile >> f)
    {
      i++;
    }
  Bands bands;

  for (int j = 0; j <= i; j++)
    {
      BandInfo bi;
      bi.fl = 8.99378e+010 + (j - 0.5) * m_sbw;
      bi.fh = 8.99378e+010 + (j + 0.5) * m_sbw;
      bi.fc = (bi.fl +  bi.fh) / 2;
      bands.push_back (bi);
    }
  m_AllTHzSpectrumWaveform = Create<SpectrumModel> (bands);
  return m_AllTHzSpectrumWaveform;
}
Ptr<SpectrumModel>
THzSpectrumValueFactory::THzPulseSpectrumWaveformInitializer ()
{
  std:: ifstream frequencyfile;
  frequencyfile.open ("contrib/thz/model/data_frequency.txt", std::ifstream::in);
  if (!frequencyfile.is_open ())
    {
	  NS_LOG_UNCOND ( "THzSpectrumValueFactory::THzPulseSpectrumWaveformInitializer: open data_frequency.txt failed" );
    }


  Bands bands;
  double pulseStartingSample = PULSE_START_FREQUENCY/m_sbw;
  m_numsb = (PULSE_END_FREQUENCY - PULSE_START_FREQUENCY)/m_sbw;

  for (int j = 0; j < m_numsample; j++)
    {
      BandInfo bi;
      bi.fl =  (pulseStartingSample + (j - 0.5) * m_numsb / m_numsample) * m_sbw;
      bi.fh =  (pulseStartingSample + (j + 0.5) * m_numsb / m_numsample) * m_sbw;
      bi.fc = (bi.fl +  bi.fh) / 2;
      bands.push_back (bi);
    }
  m_THzPulseSpectrumWaveform = Create<SpectrumModel> (bands);
  return m_THzPulseSpectrumWaveform;
}
int
THzSpectrumValueFactory::BandNum ()
{
  return m_numsb;
}

//-------------------------------------------------------------------------------------//

int
THzSpectrumValueFactory::FreqSeqStart ()    // return sequence number of the first frequency band
{
  std:: ifstream frequencyfile;
  frequencyfile.open ("contrib/thz/model/data_frequency.txt", std::ifstream::in);
  if (!frequencyfile.is_open ())
    {
	  NS_LOG_UNCOND ( "THzSpectrumValueFactory::FreqSeqStart: open data_frequency.txt failed" );
    }

  double f;
  int i = 1;
  while (frequencyfile >> f)
    {
      if (f < m_fstart)
        {
          i++;
        }
      else
        {
          break;
        }
    }

  return i;
}

double
THzSpectrumValueFactory::FreqStartValue ()
{
  std:: ifstream frequencyfile;
  frequencyfile.open ("contrib/thz/model/data_frequency.txt", std::ifstream::in);
  if (!frequencyfile.is_open ())
    {
	  NS_LOG_UNCOND ( "THzSpectrumValueFactory::FreqStartValue: open data_frequency.txt failed" );
    }

  double f;
  while (frequencyfile >> f)
    {
      if (f >= m_fstart)
        {
          break;
        }
    }
  return f;
}

Ptr<SpectrumValue>
THzSpectrumValueFactory::FreqBands ()
{
  Ptr<SpectrumValue> f_store = Create <SpectrumValue> (m_THzSpectrumWaveform);
  std:: ifstream frequencyfile;
  frequencyfile.open ("contrib/thz/model/data_frequency.txt", std::ifstream::in);
  if (!frequencyfile.is_open ())
    {
	  NS_LOG_UNCOND ( "THzSpectrumValueFactory::FreqBands: open data_frequency.txt failed" );
    }

  double f;
  int i = 0;
  while (frequencyfile >> f)
    {
      (*f_store)[i] = f;

      if (f >= m_fstart)
        {
          i++;
          if (i == m_numsb)
            {
              break;
            }
        }
    }
  return f_store;
}


int
THzSpectrumValueFactory::FreqSeqEnd ()    // return the sequence number of the last frequency band
{
  Ptr<SpectrumValue> f_store = Create <SpectrumValue> (m_THzSpectrumWaveform);
  std:: ifstream frequencyfile;
  frequencyfile.open ("contrib/thz/model/data_frequency.txt", std::ifstream::in);
  if (!frequencyfile.is_open ())
    {
	  NS_LOG_UNCOND ( "THzSpectrumValueFactory::FreqSeqEnd: open data_frequency.txt failed" );
    }

  double f;
  int i = 0;
  int j = 0;
  while (frequencyfile >> f)
    {
      if (f < m_fstart)
        {
          j++;
        }
      else
        {
          break;
        }
    }
  while (frequencyfile >> f)
    {
      (*f_store)[i] = f;

      if (f >= m_fstart)
        {
          i++;
          if (i == m_numsb)
            {
              break;
            }
        }
    }
  return i + j;
}


Ptr<SpectrumValue>
THzSpectrumValueFactory::CreateConstant (double v)
{
  Ptr<SpectrumValue> c = Create <SpectrumValue> (m_THzSpectrumWaveform);
  (*c) = v;
  return c;
}


Ptr<SpectrumValue>
THzSpectrumValueFactory::CreateTxPowerSpectralDensity (double txPower)
{
  std:: ifstream frequencyfile;
  frequencyfile.open ("contrib/thz/model/data_frequency.txt", std::ifstream::in);
  if (!frequencyfile.is_open ())
    {
	  NS_LOG_UNCOND ( "THzSpectrumValueFactory::CreateTxPowerSpectralDensity: open data_frequency.txt failed" );
    }


  double f_starV;
  double f;
  while (frequencyfile >> f)
    {
      if (f < m_fstart)
        {
        }
      else
        {
          break;
        }
    }
  f_starV = f;

  Bands bands;

  m_numsb = m_tbw/m_sbw;

  for (int j = 0; j < m_numsample; j++)
    {
      BandInfo bi;
      bi.fl = f_starV - 0.5 * m_sbw + j * m_sbw * (m_numsb / m_numsample);
      bi.fh = f_starV - 0.5 * m_sbw + (j + 1) * m_sbw * (m_numsb / m_numsample);
      bi.fc = (bi.fl +  bi.fh) / 2;
      bands.push_back (bi);
    }

  NS_LOG_DEBUG("CHECK:CreateTxPowerSpectralDensity: m_numsb = "<< m_numsb);

  Ptr<SpectrumModel> txBand = Create <SpectrumModel> (bands);
  Ptr<SpectrumValue> txPsd = Create <SpectrumValue> (txBand);
  double txPowerDensity = txPower / m_tbw;

  for (int g = 0; g < m_numsample; g++)
    {
      (*txPsd)[g] = txPowerDensity;
    }

  return txPsd;
}


Ptr<SpectrumValue>
THzSpectrumValueFactory::CreateTxPowerSpectralDensityMask (double txPower)
{
  Ptr<SpectrumValue> txPsd = Create <SpectrumValue> (m_THzSpectrumWaveform);
  double txPowerDensity = txPower / 16.0269584;
  int g = 7; // group #1-6 and #19-24 don't has psd value, so start from group #7, totally 12 out of 24 groups have psd value
  for (int g1 = (m_numsb / 24) * g; g1 < (m_numsb / 24) * (g + 1); g1++)
    {
      (*txPsd)[g1] = txPowerDensity * 1e-4 / m_sbw;      // -40dB
    }
  g++;
  for (int g2 = (m_numsb / 24) * g; g2 < (m_numsb / 24) * (g + 1); g2++)
    {
      (*txPsd)[g2] = txPowerDensity * 1e-4 / m_sbw;      // -40dB
    }
  g++;
  for (int g3 = (m_numsb / 24) * g; g3 < (m_numsb / 24) * (g + 1); g3++)
    {
      (*txPsd)[g3] = txPowerDensity * 0.0015849 / m_sbw; // -28dB
    }
  g++;
  for (int g4 = (m_numsb / 24) * g; g4 < (m_numsb / 24) * (g + 1); g4++)
    {
      (*txPsd)[g4] = txPowerDensity * 0.0015849 / m_sbw; // -28dB
    }
  g++;
  for (int g5 = (m_numsb / 24) * g; g5 < (m_numsb / 24) * (g + 1); g5++)
    {
      (*txPsd)[g5] = txPowerDensity / m_sbw;
    }
  g++;
  for (int g6 = (m_numsb / 24) * g; g6 < (m_numsb / 24) * (g + 1); g6++)
    {
      (*txPsd)[g6] = txPowerDensity / m_sbw;
    }
  g++;
  for (int g7 = (m_numsb / 24) * g; g7 < (m_numsb / 24) * (g + 1); g7++)
    {
      (*txPsd)[g7] = txPowerDensity / m_sbw;
    }
  g++;
  for (int g8 = (m_numsb / 24) * g; g8 < (m_numsb / 24) * (g + 1); g8++)
    {
      (*txPsd)[g8] = txPowerDensity / m_sbw;
    }
  g++;
  for (int g9 = (m_numsb / 24) * g; g9 < (m_numsb / 24) * (g + 1); g9++)
    {
      (*txPsd)[g9] = txPowerDensity * 0.0015849 / m_sbw; // -28dB
    }
  g++;
  for (int g10 = (m_numsb / 24) * g; g10 < (m_numsb / 24) * (g + 1); g10++)
    {
      (*txPsd)[g10] = txPowerDensity * 0.0015849 / m_sbw; // -28dB
    }
  g++;
  for (int g11 = (m_numsb / 24) * g; g11 < (m_numsb / 24) * (g + 1); g11++)
    {
      (*txPsd)[g11] = txPowerDensity * 1e-4 / m_sbw;      // -40dB
    }
  g++;
  for (int g12 = (m_numsb / 24) * g; g12 < (m_numsb / 24) * (g + 1); g12++)
    {
      (*txPsd)[g12] = txPowerDensity * 1e-4 / m_sbw;      // -40dB
    }

  std::stringstream txtname;
  txtname << "scratch/PSD-MASK.txt";
  std::string filename = txtname.str ();

  std::ofstream resultfile;
  resultfile.open (filename.c_str ());
  resultfile << "txPower: " << std::endl << txPower << std::endl << "PSD: " << std::endl << (*txPsd) << std::endl << "FreqSeqstart: " << std::endl << FreqSeqStart () << std::endl << "FreqSeqEnd: " << std::endl << FreqSeqEnd () << std::endl << " FreqStartValue " << std::endl << FreqStartValue () << std::endl;
  resultfile.close ();
  return txPsd;
}



Ptr<SpectrumValue>
THzSpectrumValueFactory::CreateAllPowerSpectralDensity (double n, double r, double a0) const
{
  Ptr<SpectrumValue> allPsd = Create <SpectrumValue> (m_AllTHzSpectrumWaveform);

  std:: ifstream frequencyfile;
  std:: ofstream myfile;
  frequencyfile.open ("contrib/thz/model/data_frequency.txt", std::ifstream::in);
  if (!frequencyfile.is_open ())
    {
	  NS_LOG_UNCOND ( "THzSpectrumValueFactory::CreateAllPowerSpectralDensity: open data_frequency.txt failed" );
    }
  double f;
  int i = 0;

  while (frequencyfile >> f)
    {
      double f1 = f / 1e12;
      (*allPsd)[i] = std::pow ((2 * M_PI * f1), (2 * n)) * std::pow (a0, 2) * std::exp (-std::pow ((2 * M_PI * f1), 2) * r);
      myfile << (*allPsd)[i] << std::endl;
      i++;
    }


  return allPsd;
}

double
THzSpectrumValueFactory::CalculateEnergyConstant (double n, double r, double txPowerWatts) const
{
  NS_LOG_FUNCTION ("");
  Ptr<SpectrumValue> allPsd = Create <SpectrumValue> (m_THzPulseSpectrumWaveform);

  Bands::const_iterator fit = allPsd->ConstBandsBegin ();

  double integral = 0.0;
  int i = 0;
  while (fit != allPsd->ConstBandsEnd ())
    {
      (*allPsd)[i] = std::pow ((2 * M_PI * fit->fc), (2 * n)) * std::exp (-std::pow ((2 * M_PI * fit->fc * r), 2));

      integral += (*allPsd)[i];
      i++;
      ++fit;
    }
  NS_LOG_INFO ("value of i:" << i);
  integral *= m_sbw * (m_numsb / m_numsample);
  double a02 = txPowerWatts / integral;
  NS_LOG_INFO ("value of a0:" << a02);
  return a02;
}

Ptr<SpectrumValue>
THzSpectrumValueFactory::CreatePulsePowerSpectralDensity (double n, double r, double txPowerWatts) const
{
  NS_LOG_FUNCTION ("tx power" << txPowerWatts);
  Ptr<SpectrumValue> allPsd = Create <SpectrumValue> (m_THzPulseSpectrumWaveform);

  Bands::const_iterator fit = allPsd->ConstBandsBegin ();
  double txPsd_inte = 0.0;
  int i = 0;

  double a02 = CalculateEnergyConstant (n, r, txPowerWatts);
  while (fit != allPsd->ConstBandsEnd ())
    {
      (*allPsd)[i] = std::pow ((2 * M_PI * fit->fc), (2 * n)) * a02 * std::exp (-std::pow ((2 * M_PI * fit->fc * r), 2));
      txPsd_inte += (*allPsd)[i];
      i++;
      ++fit;
    }
  double txPower = txPsd_inte * m_sbw * (m_numsb / m_numsample);
  NS_LOG_UNCOND ("tx power from PSD" << txPower);

  return allPsd;
}


} // namespace ns3
