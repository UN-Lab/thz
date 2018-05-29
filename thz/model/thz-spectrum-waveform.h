/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 UBNANO (http://ubnano.tech/)
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
#ifndef THZ_SPECTRUM_WAVEFORM_H
#define THZ_SPECTRUM_WAVEFORM_H

#include <ns3/spectrum-value.h>
#include <ns3/object.h>

namespace ns3 {
class THzSpectrumValueFactory : public Object
{
public:
  THzSpectrumValueFactory ();
  virtual ~THzSpectrumValueFactory ();

  /**
   * \brief Get the type ID.
   * \return the object TypeID
   */
  static TypeId GetTypeId (void);


  virtual Ptr<SpectrumModel> THzSpectrumWaveformInitializer ();
  virtual Ptr<SpectrumModel> AllTHzSpectrumWaveformInitializer ();
  virtual Ptr<SpectrumModel> THzPulseSpectrumWaveformInitializer ();

  /**
   * \return the number of subbands
   */
  virtual int BandNum ();

  /**
   * \return the sequence number of the first frequency band
   */
  virtual int FreqSeqStart ();

  /**
   * \return the frequency value of the first frequency band.
   */
  virtual double FreqStartValue ();

  /**
   * \return the frequency information of the operation frequecy band.
   */
  virtual Ptr<SpectrumValue> FreqBands ();

  /**
   * \return the sequence number of the last frequency band.
   */
  virtual int FreqSeqEnd ();

  /**
   * \param psd the constant power spectral density value.
   *
   * \return terahertz spectrum waveform with the constant power spectral density.
   */
  virtual Ptr<SpectrumValue> CreateConstant (double psd);

  /**
   * \param txPower transmission power
   *
   * \return power spectral density with given transmission power and frequency band information
   */
  virtual Ptr<SpectrumValue> CreateTxPowerSpectralDensity (double txPower);

  /**
   * \param txPower transmission power
   *
   * \return masked power spectral density with given transmission power and total bandwidth
   */
  virtual Ptr<SpectrumValue> CreateTxPowerSpectralDensityMask (double txPower);
  /**
   * \param n order of derivative of the Gaussian pulse
   * \param r standard deviation of the Gaussian pulse
   * \param txPowerWatts transmission power
   *
   * \return power spectral density with given transmission power and full Thz spectrum
   */
  virtual Ptr<SpectrumValue> CreateAllPowerSpectralDensity (double n, double r, double a0) const;
  /**
   * \param n order of derivative of the Gaussian pulse
   * \param r standard deviation of the Gaussian pulse
   * \param txPowerWatts transmission power
   *
   * \return power spectral density of the Gaussian pulse with given transmission power
   */
  virtual Ptr<SpectrumValue> CreatePulsePowerSpectralDensity (double n, double r, double txPowerWatts) const;
  /**
   * \param n order of derivative of the Gaussian pulse
   * \param r standard deviation of the Gaussian pulse
   * \param txPowerWatts transmission power
   *
   * \return value of the normalizing constant for the Gaussian pulse
   */
  virtual double CalculateEnergyConstant (double n, double r, double txPowerWatts) const;
  //private:
  double m_numsb; //NumSubBand
  double m_sbw;   //SubBandWidth
  double m_tbw;   //TotalBandWidth
  double m_fc;    //CentralFrequency
  double m_numsample; //NumSample

  double m_fstart; //StartingFrequency
  Ptr<SpectrumModel> m_THzSpectrumWaveform;
  Ptr<SpectrumModel> m_AllTHzSpectrumWaveform;
  Ptr<SpectrumModel> m_THzPulseSpectrumWaveform;
};

} // namespace ns3



#endif /*  THZ_SPECTRUM_WAVEFORM */
