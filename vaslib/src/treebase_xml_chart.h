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

/*! \file    treebase_xml_chart.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __TREEBASE_XML_CHART_H__
#define __TREEBASE_XML_CHART_H__

#include <QDomDocument>
#include <QDomElement>

#include "treebase_xml.h"

class ChartModel;

/////////////////////////////////////////////////////////////////////////////

//! Base class of a tree with a DOM element member
class TreeBaseXMLChart  : public TreeBaseXML
{
    Q_OBJECT

public:
    //! Standard Constructor
    TreeBaseXMLChart(TreeBaseXMLChart* parent, ChartModel* chart_model);

    //! Destructor
    virtual ~TreeBaseXMLChart();

    ChartModel* getChartModel() { return m_chart_model; }

protected slots:

    void slotDoSave() { saveToDomElement(); }

protected:

    ChartModel* m_chart_model;

private:
    //! Hidden copy-constructor
    TreeBaseXMLChart(const TreeBaseXMLChart&);
    //! Hidden assignment operator
    const TreeBaseXMLChart& operator = (const TreeBaseXMLChart&);
};

#endif /* __TREEBASE_XML_CHART_H___ */

// End of file

