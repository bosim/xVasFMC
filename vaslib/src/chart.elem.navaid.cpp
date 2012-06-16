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

/*! \file    chart.elem.navaid.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "chart.xml.defines.h"
#include "chart.elem.navaid.h"

/////////////////////////////////////////////////////////////////////////////

ChartElemNavaid::ChartElemNavaid(TreeBaseXMLChart* parent, ChartModel* chart_model) : 
    TreeBaseXMLChart(parent, chart_model) {}

/////////////////////////////////////////////////////////////////////////////

ChartElemNavaid::~ChartElemNavaid() {}

/////////////////////////////////////////////////////////////////////////////
 
QString ChartElemNavaid::convertWptTypeToXMLType(Waypoint::WptType wpt_type)
{
    switch(wpt_type)
    {
        case(Waypoint::VOR): return CHART_NODE_NAME_NAVAIDS_VOR;
        case(Waypoint::NDB): return CHART_NODE_NAME_NAVAIDS_NDB;
        case(Waypoint::AIRPORT): return CHART_NODE_NAME_NAVAIDS_AIRPORT;
        case(Waypoint::INTERSECTION): return CHART_NODE_NAME_NAVAIDS_INTERSECTION;
        default: break;
    }

    return "unknown";
}

/////////////////////////////////////////////////////////////////////////////

// End of file
