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

/*! \file    chart.elem.root.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "chart.xml.defines.h"

#include "chart.elem.root.h"

/////////////////////////////////////////////////////////////////////////////

ChartElemRoot::ChartElemRoot(ChartModel* chart_model) : 
    TreeBaseXMLChart(0, chart_model) 
{};

/////////////////////////////////////////////////////////////////////////////

ChartElemRoot::~ChartElemRoot() {};

/////////////////////////////////////////////////////////////////////////////

bool ChartElemRoot::loadFromDomElement(QDomElement& dom_element, 
                                       QDomDocument& dom_doc,
                                       QString& err_msg)
{
    MYASSERT(!dom_element.isNull());
    TreeBaseXML::loadFromDomElement(dom_element, dom_doc, err_msg);

    if (m_dom_element.tagName() != CHART_NODE_NAME_ROOT)
    {
        err_msg = QString("Unknown ROOT tag name (%s)").arg(m_dom_element.tagName());
        return false;
    }

    return true;
}
 
// End of file
