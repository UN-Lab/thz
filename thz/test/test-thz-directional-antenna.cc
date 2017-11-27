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
 * Author: Qing Xia <qingxia@buffalo.edu>
 *         Zahed Hossain <zahedhos@buffalo.edu>
 *         Josep Miquel Jornet <jmjornet@buffalo.edu>
 */

#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/double.h"
#include "ns3/gnuplot.h"
#include <cmath>
#include "ns3/constant-position-mobility-model.h"
#include "ns3/thz-dir-antenna.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TestTHzDirectionalAntenna");

int main (int argc, char *argv[])
{

  //--- setting up ---//
  Ptr<MobilityModel> rx_node = CreateObject<ConstantPositionMobilityModel> ();
  rx_node->SetPosition (Vector (0,0,0));
  Ptr<MobilityModel> tx_node = CreateObject<ConstantPositionMobilityModel> ();
  tx_node->SetPosition (Vector (1,0,0));


  Ptr<THzDirectionalAntenna> THzDAModel = CreateObject<THzDirectionalAntenna> ();
  double n_sector = 13;            // number of sectors in one circle
  double bw_theta = 360 / n_sector; // beamwidth in degree
  double gain_max = 17.27;         // maximum antenna gain
  THzDAModel->SetBeamwidth (bw_theta);
  THzDAModel->SetMaxGain (gain_max);

  Gnuplot2dDataset dataset;
  dataset.SetTitle ("THz Directional Antenna Gain");
  dataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);

  int i = 0;
  while (i <= n_sector)
    {
      double angle_rxda_deg = i * bw_theta;
      double angle_rxda_rad = angle_rxda_deg * M_PI / 180.0;
      double gain_total = THzDAModel->GetAntennaGain (rx_node, tx_node, 1, 0, angle_rxda_rad);
      std::printf ("Total Gain: %f <--> Orientaion of RXDA: %f \n", gain_total, angle_rxda_deg);
      dataset.Add (angle_rxda_deg, gain_total);
      i++;
    }


  //--- plot---//
  std::string fileNameWithNoExtension = "test-thz-directional-antenna";
  std::string graphicsFileName        = fileNameWithNoExtension + ".png";
  std::string plotFileName            = fileNameWithNoExtension + ".plt";
  std::string plotTitle               = "THz Directional Antenna Gain Test";

  // Instantiate the plot and set its title.
  Gnuplot plot (graphicsFileName);
  plot.SetTitle (plotTitle);

  // Make the graphics file, which the plot file will create when it
  // is used with Gnuplot, be a PNG file.
  plot.SetTerminal ("png");

  // Set the labels for each axis.
  plot.SetLegend ("Orientation of RXDA [Degree]", "Total Gain [dB]");

  plot.AddDataset (dataset);
  std::ofstream plotFile (plotFileName.c_str ());
  plot.GenerateOutput (plotFile);
  plotFile.close ();



  return 0;

}
