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
 *         Daniel Morales <danimoralesbrotons@gmail.com>
 */

#ifndef THZ_SPECTRUM_PROPAGATION_LOSS_H
#define THZ_SPECTRUM_PROPAGATION_LOSS_H

#include "thz-spectrum-signal-parameters.h"

#include <ns3/mobility-model.h>
#include <ns3/object.h>
#include <ns3/spectrum-value.h>

#include <map>

namespace ns3
{
/**
 * \defgroup Terahertz Spectrum Propagation Loss Models
 *
 */

class THzSpectrumPropagationLoss : public Object
{
  public:
    THzSpectrumPropagationLoss();
    virtual ~THzSpectrumPropagationLoss();

    virtual bool mapContainsKey(std::map<double, double>& map, double key);

    /**
     * \param txPsd the power spectral density of the transmitted signal, unit in Watt.
     * \param a the mobility of sender.
     * \param b the mobility of receiver.
     *
     * \return the power spectral density of the received signal, unit is Watt.
     *
     * This application doesn't apply the terahertz directional antenna.
     */
    virtual Ptr<SpectrumValue> CalcRxPowerSpectralDensity(Ptr<const SpectrumValue> txPsd,
                                                          Ptr<const MobilityModel> a,
                                                          Ptr<const MobilityModel> b);

    /**
     * \param txPsd the power spectral density of the transmitted signal, unit in Watt.
     * \param a the mobility of sender.
     * \param b the mobility of receiver.
     * \param RxTxGainDb the total antenna gain of both transmitter and receiver, unit in dB.
     *
     * \return the received signal power, unit in dBm.
     *
     * This application applies the terahertz directional antenna.
     */
    virtual double CalcRxPowerDA(Ptr<THzSpectrumSignalParameters> txParams,
                                 Ptr<MobilityModel> a,
                                 Ptr<MobilityModel> b,
                                 double RxTxGainDb);

    /**
     * \brief Calculate the spreading loss
     *
     * \param f the central frequency of the operation subband, unit in Hz.
     * \param d the distance between transmitter and receiver, unit in meter.
     *
     * \return the spreading loss unit in Watt.
     *
     * The reference is J. M. Jornet and I. F. Akyildiz, "Channel modeling and capacity analysis
     * of electromagnetic wireless nanonetworks in the terahertz band," IEEE
     * Transactions on Wireless Communications, vol. 10, no. 10, pp. 3211,
     * 3221, Oct. 2011
     *
     * The values of f and d are collected from HITRAN database.
     */
    virtual double CalculateSpreadLoss(double f, double d) const;

    /**
     * \brief Calculate the absorption coefficient loss.
     *
     * \param f the central frequency of the operation subband, unit in Hz.
     * \param d the distance between transmitter and receiver, unit in meter.
     *
     * \return the absorption coefficient loss, unit in Watt.
     *
     * The reference is J. M. Jornet and I. F. Akyildiz, "Channel modeling and capacity analysis
     * of electromagnetic wireless nanonetworks in the terahertz band," IEEE
     * Transactions on Wireless Communications, vol. 10, no. 10, pp. 3211,
     * 3221, Oct. 2011.
     *
     * The values of f and d are collected from HITRAN database.
     */
    virtual double CalculateAbsLoss(double f, double d);

    /**
     * \param s the starting boundary of the absorption coefficent.
     * \param j the ending boundary of the absorption coefficent.
     * \param f the central frequency of the operation subband, unit in Hz.
     * \param d the distance between transmitter and receiver, unit in meter.
     * \param txPsd the power spectral density of the transmitted signal.
     *
     * \return the list of absorption coefficent values within the specified boundaries.
     *
     * Mainly for checking purpose.
     */
    virtual Ptr<SpectrumValue> LoadedAbsCoe(int s,
                                            int j,
                                            double f,
                                            double d,
                                            Ptr<const SpectrumValue> txPsd) const;

    double m_previousFc;
    double m_kf;
    std::map<double, double> m_freqMap;
};

} // namespace ns3

#endif /* SPECTRUM_PROPAGATION_LOSS_H */
