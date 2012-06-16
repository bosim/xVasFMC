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

/*! \file    chart.elem.projection.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "projection.h"
#include "waypoint.h"
#include "chart.xml.defines.h"
#include "treebase_xml_chart.h"

#include "chartmodel.h"
#include "chart.elem.projection.h"

/////////////////////////////////////////////////////////////////////////////

ChartElemProjection::ChartElemProjection(TreeBaseXMLChart* parent, ChartModel* chart_model) : 
    TreeBaseXMLChart(parent, chart_model)
{
}

/////////////////////////////////////////////////////////////////////////////

ChartElemProjection::~ChartElemProjection()
{
    saveToDomElement();
}

/////////////////////////////////////////////////////////////////////////////

bool ChartElemProjection::loadFromDomElement(QDomElement& dom_element, 
                                             QDomDocument& dom_doc,
                                             QString& err_msg)
{
    MYASSERT(!dom_element.isNull());
    TreeBaseXMLChart::loadFromDomElement(dom_element, dom_doc, err_msg);

    if (m_dom_element.tagName() != CHART_NODE_NAME_PROJECTION)
    {
        err_msg = QString("Wrong node name (%s), expected (%s)").
                  arg(m_dom_element.tagName()).arg(CHART_NODE_NAME_PROJECTION);
        return false;
    }

    // center node

    m_center_element = dom_element.firstChildElement(CHART_NODE_NAME_PROJECTION_CENTER);
    if (m_center_element.isNull())
    {
        m_center_element = dom_doc.createElement(CHART_NODE_NAME_PROJECTION_CENTER);
        dom_element.appendChild(m_center_element);
    }
   
    MYASSERT(!m_center_element.isNull());

    if (!m_center_element.attribute(CHART_ATTR_LAT).isEmpty() &&
        !m_center_element.attribute(CHART_ATTR_LON).isEmpty())
    {
        Waypoint center_wpt;

        bool convok1 = false, convok2 = false;
        center_wpt.setLatLon(m_center_element.attribute(CHART_ATTR_LAT).toDouble(&convok1),
                             m_center_element.attribute(CHART_ATTR_LON).toDouble(&convok2));
        if (!convok1 || !convok2)
        {
            err_msg = QString("Could not parse projection center (%1/%2)").
                      arg(m_center_element.attribute(CHART_ATTR_LAT)).
                      arg(m_center_element.attribute(CHART_ATTR_LON));
            return false;
        }
        
        m_chart_model->getProjection()->setCenter(center_wpt);
    }

    // range node
    
    m_range_element = dom_element.firstChildElement(CHART_NODE_NAME_PROJECTION_RANGE);
    if (m_range_element.isNull())
    {
        m_range_element = dom_doc.createElement(CHART_NODE_NAME_PROJECTION_RANGE);
        dom_element.appendChild(m_range_element);
    }

    MYASSERT(!m_range_element.isNull());

    if (!m_range_element.attribute(CHART_ATTR_VALUE).isEmpty())
    {
        bool convok = false;
        unsigned int range_nm = m_range_element.attribute(CHART_ATTR_VALUE).toUInt(&convok);
        if (!convok)
        {
            err_msg = QString("Could not parse range (%1)").
                      arg(m_range_element.attribute(CHART_ATTR_VALUE));
            return false;
        }
        
        m_chart_model->getProjection()->setScale(
            range_nm, m_chart_model->getProjection()->getDrawingDistAtMaxRange());
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool ChartElemProjection::saveToDomElement()
{
    TreeBaseXML::saveToDomElement();

    m_center_element.setAttribute(
        CHART_ATTR_LAT, QString::number(
            m_chart_model->getProjection()->getCenter().getLat(), 'f', 8));

    m_center_element.setAttribute(
        CHART_ATTR_LON, QString::number(
            m_chart_model->getProjection()->getCenter().getLon(), 'f', 8));

    m_range_element.setAttribute(
        CHART_ATTR_VALUE, QString::number(
            m_chart_model->getProjection()->getRangeNm()));

    return true;
}

/////////////////////////////////////////////////////////////////////////////

void ChartElemProjection::setCenter(const Waypoint& wpt)
{
    m_chart_model->getProjection()->setCenter(wpt);
}
 
// End of file
