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


#include "ns3/pointer.h"
#include "ns3/thz-dir-antenna.h"
#include "thz-directional-antenna-helper.h"

#include <sstream>
#include <string>

namespace ns3 {

THzDirectionalAntennaHelper::THzDirectionalAntennaHelper ()
{
}

THzDirectionalAntennaHelper::~THzDirectionalAntennaHelper ()
{
}

THzDirectionalAntennaHelper
THzDirectionalAntennaHelper::Default (void)
{
  THzDirectionalAntennaHelper helper;
  helper.SetType ("ns3::THzDirectionalAntenna");
  return helper;
}

void
THzDirectionalAntennaHelper::SetType (std::string type,
                                      std::string n0, const AttributeValue &v0,
                                      std::string n1, const AttributeValue &v1,
                                      std::string n2, const AttributeValue &v2,
                                      std::string n3, const AttributeValue &v3,
                                      std::string n4, const AttributeValue &v4,
                                      std::string n5, const AttributeValue &v5,
                                      std::string n6, const AttributeValue &v6,
                                      std::string n7, const AttributeValue &v7)
{
  m_dirantenna.SetTypeId (type);
  m_dirantenna.Set (n0, v0);
  m_dirantenna.Set (n1, v1);
  m_dirantenna.Set (n2, v2);
  m_dirantenna.Set (n3, v3);
  m_dirantenna.Set (n4, v4);
  m_dirantenna.Set (n5, v5);
  m_dirantenna.Set (n6, v6);
  m_dirantenna.Set (n7, v7);
}
void
THzDirectionalAntennaHelper::Set (std::string n, const AttributeValue &v)
{
  m_dirantenna.Set (n, v);
}

Ptr<THzDirectionalAntenna>
THzDirectionalAntennaHelper::Create (void) const
{
  Ptr<THzDirectionalAntenna> dirantenna = m_dirantenna.Create<THzDirectionalAntenna> ();
  return dirantenna;
}

} //namespace ns3
