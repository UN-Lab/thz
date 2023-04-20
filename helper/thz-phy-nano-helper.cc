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

#include "thz-phy-nano-helper.h"
#include "ns3/thz-phy.h"

#include <sstream>
#include <string>

namespace ns3 {

THzPhyNanoHelper::THzPhyNanoHelper ()
{
  m_phy.SetTypeId ("ns3::THzPhyNano");
}

THzPhyNanoHelper::~THzPhyNanoHelper ()
{
}

THzPhyNanoHelper
THzPhyNanoHelper::Default (void)
{
  THzPhyNanoHelper helper;
  helper.SetType ("ns3::THzPhyNano");
  return helper;
}

void
THzPhyNanoHelper::SetType (std::string type,
                           std::string n0, const AttributeValue &v0,
                           std::string n1, const AttributeValue &v1,
                           std::string n2, const AttributeValue &v2,
                           std::string n3, const AttributeValue &v3,
                           std::string n4, const AttributeValue &v4,
                           std::string n5, const AttributeValue &v5,
                           std::string n6, const AttributeValue &v6,
                           std::string n7, const AttributeValue &v7)
{
  m_phy.SetTypeId (type);
  m_phy.Set (n0, v0);
  m_phy.Set (n1, v1);
  m_phy.Set (n2, v2);
  m_phy.Set (n3, v3);
  m_phy.Set (n4, v4);
  m_phy.Set (n5, v5);
  m_phy.Set (n6, v6);
  m_phy.Set (n7, v7);
}

void
THzPhyNanoHelper::SetPhyAttribute (std::string n, const AttributeValue &v)
{
  m_phy.Set (n, v);
}

Ptr<THzPhy>
THzPhyNanoHelper::Create (void) const
{
  Ptr<THzPhy> phy = m_phy.Create<THzPhy> ();
  return phy;
}

} //namespace ns3
