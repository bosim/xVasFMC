///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2006 Alexander Wemmer 
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
///////////////////////////////////////////////////////////////////////////////

/*! \file    treebase_xml_chart.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "chartmodel.h"

#include "treebase_xml_chart.h"

/////////////////////////////////////////////////////////////////////////////

TreeBaseXMLChart::TreeBaseXMLChart(TreeBaseXMLChart* parent, ChartModel* chart_model) : 
    TreeBaseXML(parent), m_chart_model(chart_model)
{
    MYASSERT(m_chart_model != 0);
    MYASSERT(connect(m_chart_model, SIGNAL(signalBeforeSave()), this, SLOT(slotDoSave())));
}

/////////////////////////////////////////////////////////////////////////////

TreeBaseXMLChart::~TreeBaseXMLChart()
{
}

// End of file
