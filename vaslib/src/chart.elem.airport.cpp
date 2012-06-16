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

/*! \file    chart.elem.airport.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "airport.h"
#include "ge_airport.h"
#include "chart.xml.defines.h"
#include "chartmodel.h"
#include "ge_layers.h"

#include "chart.elem.airport.h"

/////////////////////////////////////////////////////////////////////////////

ChartElemAirport::ChartElemAirport(TreeBaseXMLChart* parent, 
                                   ChartModel* chart_model, 
                                   const Airport& airport) : 
    ChartElemNavaid(parent, chart_model)
{
    m_ge_airport = new GEAirport(m_chart_model->getProjection(), (Airport*)(airport.copy()));
    MYASSERT(m_ge_airport != 0);

    m_chart_model->getGELayerList()->addElementByLayer(
        CHART_GE_LAYER_NAVAIDS, m_ge_airport);
    m_chart_model->getGELayerList()->addElementByLayer(
        CHART_GE_LAYER_NAVAIDS, m_ge_airport->getLabel());
    
    m_ge_airport->setDataElement(this);
    m_ge_airport->getLabel()->setDataElement(this);
}

/////////////////////////////////////////////////////////////////////////////

ChartElemAirport::~ChartElemAirport()
{
    saveToDomElement();
    delete m_ge_airport;
}

/////////////////////////////////////////////////////////////////////////////

bool ChartElemAirport::loadFromDomElement(QDomElement& dom_element, 
                                          QDomDocument& dom_doc,
                                          QString& err_msg)
{
    MYASSERT(!dom_element.isNull());
    TreeBaseXML::loadFromDomElement(dom_element, dom_doc, err_msg);
    
    if (m_dom_element.tagName() != CHART_NODE_NAME_NAVAIDS_AIRPORT)
    {
        err_msg = QString("Wrong node name (%s), expected (%s)").
                  arg(m_dom_element.tagName()).arg(CHART_NODE_NAME_NAVAIDS_AIRPORT);
        return false;
    }

    bool ok1 = false, ok2 = false;
    int x = dom_element.attribute(CHART_ATTR_LABEL_X_DIFF).toInt(&ok1);
    int y = dom_element.attribute(CHART_ATTR_LABEL_Y_DIFF).toInt(&ok2);
    if (ok1 && ok2) m_ge_airport->getLabel()->setDiffXY(QPoint(x, y));

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool ChartElemAirport::saveToDomElement()
{
    m_dom_element.setAttribute(CHART_ATTR_ID, m_ge_airport->getAirport()->getId());
    m_dom_element.setAttribute(CHART_ATTR_NAME, m_ge_airport->getAirport()->getName());
    m_dom_element.setAttribute(
        CHART_ATTR_LAT, QString::number(m_ge_airport->getAirport()->getLat(), 'f', 8));
    m_dom_element.setAttribute(
        CHART_ATTR_LON, QString::number(m_ge_airport->getAirport()->getLon(), 'f', 8));
    m_dom_element.setAttribute(
        CHART_ATTR_ELEV, QString::number(m_ge_airport->getAirport()->getElevationFt()));

    writeDOMIntAttribute(m_dom_element, CHART_ATTR_LABEL_X_DIFF, 
                         (int)m_ge_airport->getLabel()->getDiffXY().x());
    writeDOMIntAttribute(m_dom_element, CHART_ATTR_LABEL_Y_DIFF, 
                         (int)m_ge_airport->getLabel()->getDiffXY().y());
    return true;
}

/////////////////////////////////////////////////////////////////////////////
 
// End of file
