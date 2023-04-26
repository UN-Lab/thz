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

#include "thz-spectrum-propagation-loss.h"

#include <ns3/angles.h>
#include <ns3/antenna-model.h>
#include <ns3/core-module.h>
#include <ns3/cosine-antenna-model.h>
#include <ns3/double.h>
#include <ns3/log.h>
#include <ns3/mobility-model.h>
#include <ns3/object.h>
#include <ns3/thz-spectrum-waveform.h>

#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

NS_LOG_COMPONENT_DEFINE("THzSpectrumPropagationLoss");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(THzSpectrumPropagationLoss);

THzSpectrumPropagationLoss::THzSpectrumPropagationLoss()
{
}

THzSpectrumPropagationLoss::~THzSpectrumPropagationLoss()
{
}

Ptr<SpectrumValue>
THzSpectrumPropagationLoss::CalcRxPowerSpectralDensity(Ptr<const SpectrumValue> txPsd,
                                                       Ptr<const MobilityModel> a,
                                                       Ptr<const MobilityModel> b)
{
    Ptr<SpectrumValue> rxPsd = Copy<SpectrumValue>(txPsd); //[W]
    Values::iterator vit = rxPsd->ValuesBegin();
    Bands::const_iterator fit = rxPsd->ConstBandsBegin();

    NS_ASSERT(a);
    NS_ASSERT(b);
    double d = a->GetDistanceFrom(b);
    while (vit != rxPsd->ValuesEnd())
    {
        NS_ASSERT(fit != rxPsd->ConstBandsEnd());
        double lossDb = 10 * std::log10(CalculateSpreadLoss(fit->fc, d)) +
                        10 * std::log10(CalculateAbsLoss(fit->fc, d));
        double lossW = std::pow(10.0, lossDb / 10.0);
        *vit /= lossW;
        ++vit;
        ++fit;
    }
    return rxPsd; //[W]
}

double
THzSpectrumPropagationLoss::CalcRxPowerDA(Ptr<THzSpectrumSignalParameters> txParams,
                                          Ptr<MobilityModel> a,
                                          Ptr<MobilityModel> b,
                                          double RxTxGainDb)
{
    double RxTxGainW = std::pow(10.0, (RxTxGainDb) / 10.0);
    Ptr<SpectrumValue> rxPsd = Copy<SpectrumValue>(txParams->txPsd); // [W]
    Values::iterator vit = rxPsd->ValuesBegin();
    Bands::const_iterator fit = rxPsd->ConstBandsBegin();

    NS_ASSERT(a);
    NS_ASSERT(b);
    double d = a->GetDistanceFrom(b);
    NS_LOG_INFO("Distance = " << d);
    double rxPsd_inte = 0.0;
    while (vit != rxPsd->ValuesEnd())
    {
        NS_ASSERT(fit != rxPsd->ConstBandsEnd());
        double lossDb = 10 * std::log10(CalculateSpreadLoss(fit->fc, d)) +
                        10 * std::log10(CalculateAbsLoss(fit->fc, d));
        double lossW = std::pow(10.0, lossDb / 10.0);
        *vit /= lossW;
        rxPsd_inte += *vit;
        ++vit;
        ++fit;
    }
    NS_LOG_INFO("rxPsd_inte = " << rxPsd_inte << " RxTxGainW " << RxTxGainW);
    double rxPower = rxPsd_inte * txParams->subBandBandwidth *
                     (txParams->numberOfSubBands / txParams->numberOfSamples) * RxTxGainW;
    double rxPowerDbm = 10 * std::log10(rxPower * 1000.0);
    NS_LOG_INFO("Number of samples: " << txParams->numberOfSamples << " RxPower = " << rxPower << " W ");
    NS_LOG_INFO("RxPowerDbm = " << rxPowerDbm << " Dbm ");
    return rxPowerDbm;
}

double
THzSpectrumPropagationLoss::CalculateSpreadLoss(double f, double d) const
{
    NS_ASSERT(d >= 0);

    if (d == 0)
    {
        return 0;
    }

    NS_ASSERT(f > 0);
    double loss_sqrt = (4 * M_PI * f * d) / 299792458;
    double loss = loss_sqrt * loss_sqrt;
    return loss;
}

double
THzSpectrumPropagationLoss::CalculateAbsLoss(double f, double d)
{
    double kf = 0.0;
    double loss = 0.0;

    if (mapContainsKey(m_freqMap, f))
    {
        kf = m_freqMap[f];
    }
    else
    {
        std::ifstream AbsCoefile;
        AbsCoefile.open("contrib/thz/model/data_AbsCoe.txt", std::ifstream::in);
        if (!AbsCoefile.is_open())
        {
            NS_FATAL_ERROR("THzSpectrumPropagationLoss::CalculateAbsLoss: open data_AbsCoe.txt failed 1");
        }

        std::ifstream frequencyfile;
        frequencyfile.open("contrib/thz/model/data_frequency.txt", std::ifstream::in);
        if (!frequencyfile.is_open())
        {
            NS_FATAL_ERROR("THzSpectrumPropagationLoss::CalculateAbsLoss: open data_frequency.txt failed");
        }
        double f_ite;
        double k_ite;
        int i = 0;
        int j = 0;

        while (frequencyfile >> f_ite)
        {
            if (f_ite < f - 9.894e8 || f_ite > f + 9.894e8)
            {
                j++;
            }
            else
            {
                break;
            }
        }
        while (AbsCoefile >> k_ite)
        {
            if (i != j)
            {
                i++;
            }
            else
            {
                kf = k_ite;
                break;
            }
            NS_ASSERT(d >= 0);

            if (d == 0)
            {
                return 0;
            }
        }
        NS_ASSERT(f > 0);
        m_freqMap.insert(std::pair<double, double>(f, kf));
        NS_LOG_UNCOND("inserted to map f: " << f << " kf: " << kf);
    } // if f != m_previousFc

    loss = exp(kf * d);
    return loss;
}

Ptr<SpectrumValue>
THzSpectrumPropagationLoss::LoadedAbsCoe(int s,
                                         int j,
                                         double f,
                                         double d,
                                         Ptr<const SpectrumValue> txPsd) const
{
    std::ifstream AbsCoefile;
    AbsCoefile.open("contrib/thz/model/data_AbsCoe.txt", std::ifstream::in);
    if (!AbsCoefile.is_open())
    {
        NS_FATAL_ERROR("THzSpectrumPropagationLoss::LoadedAbsCoe: open data_AbsCoe.txt failed");
    }
    double k;
    Ptr<SpectrumValue> kf_store = Copy<SpectrumValue>(txPsd);
    int i = 1;

    while (AbsCoefile >> k)
    {
        if (i < s)
        {
            i++;
        }
        else if (i > j)
        {
            break;
        }
        else
        {
            (*kf_store)[i - s] = k;
            i++;
        }
    }
    return kf_store;
}

bool
THzSpectrumPropagationLoss::mapContainsKey(std::map<double, double>& map, double key)
{
    if (map.find(key) == map.end())
        return false;
    return true;
}

} // namespace ns3
