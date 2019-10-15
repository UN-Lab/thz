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


#ifndef THZ_ENERGY_MODEL_HELPER_H
#define THZ_ENERGY_MODEL_HELPER_H

#include <string>
#include "ns3/attribute.h"
#include "ns3/object-factory.h"
#include "ns3/node-container.h"
#include "ns3/thz-energy-model.h"


namespace ns3
{

/**
 * \ingroup thz
 *
 * This installer installs THzEnergyModel to the nodes,
 * using ns3 object aggregation 
 */
class THzEnergyModelHelper
{
public:
  /**
   * Create a THzEnergyModelHelper
   */
  THzEnergyModelHelper();
  /**
   * \internal
   * Destroy a THzEnergyModelHelper
   */
  virtual ~THzEnergyModelHelper();
  /**
   * For each Ptr<node> in the provided container:
   * it creates an ns3::THzEnergyModel (with the attributes 
   * configured by THzEnergyModelHelper::SetEnergyModelAttribute); 
   * adds the energy model to the node;
   *
   * \param c The NodeContainer holding the nodes to be changed.
   */
  void Install (NodeContainer c) const;
  /**
   * \param n1 the name of the attribute to set
   * \param v1 the value of the attribute to set
   *
   * Set these attributes on each ns3::THzEnergyModel created
   * by THzEnergyModelHelper::Install
   */
  void SetEnergyModelAttribute (std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue ());

private:
  /** Energy model factory. */
  ObjectFactory m_energyModel;
};


} //end namespace ns3

#endif /* THZ_ENERGY_MODEL_HELPER_H */
