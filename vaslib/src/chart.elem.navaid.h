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

/*! \file    chart.elem.navaid.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __CHART_ELEM_NAVAID_H__
#define __CHART_ELEM_NAVAID_H__

#include "waypoint.h"
#include "treebase_xml_chart.h"

class ChartModel;

/////////////////////////////////////////////////////////////////////////////

//! navaid element for charts
class ChartElemNavaid : public TreeBaseXMLChart
{
    Q_OBJECT

public:
    //! Standard Constructor
    ChartElemNavaid(TreeBaseXMLChart* parent, ChartModel* chart_model);

    //! Destructor
    virtual ~ChartElemNavaid();

    virtual const Waypoint* getNavaid() const = 0;

    static QString convertWptTypeToXMLType(Waypoint::WptType wpt_type);

private:
    //! Hidden copy-constructor
    ChartElemNavaid(const ChartElemNavaid&);
    //! Hidden assignment operator
    const ChartElemNavaid& operator = (const ChartElemNavaid&);
};

#endif /* __CHART_ELEM_NAVAID_H__ */

// End of file

