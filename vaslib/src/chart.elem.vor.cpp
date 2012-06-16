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

/*! \file    chart.elem.vor.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "vor.h"
#include "ge_vor.h"
#include "chart.xml.defines.h"
#include "chartmodel.h"
#include "ge_layers.h"

#include "chart.elem.vor.h"

/////////////////////////////////////////////////////////////////////////////

ChartElemVor::ChartElemVor(TreeBaseXMLChart* parent, ChartModel* chart_model, const Vor& vor) 
    : ChartElemNavaid(parent, chart_model)
{
    m_ge_vor = new GEVor(m_chart_model->getProjection(), (Vor*)(vor.copy()));
    MYASSERT(m_ge_vor != 0);

    m_chart_model->getGELayerList()->addElementByLayer(CHART_GE_LAYER_NAVAIDS, m_ge_vor);
    m_chart_model->getGELayerList()->addElementByLayer(CHART_GE_LAYER_LABELS, m_ge_vor->getLabel());

    m_ge_vor->setDataElement(this);
    m_ge_vor->getLabel()->setDataElement(this);
}

/////////////////////////////////////////////////////////////////////////////

ChartElemVor::~ChartElemVor()
{
    saveToDomElement();
    delete m_ge_vor;
}

/////////////////////////////////////////////////////////////////////////////

bool ChartElemVor::loadFromDomElement(QDomElement& dom_element, 
                                      QDomDocument& dom_doc,
                                      QString& err_msg)
{
    MYASSERT(!dom_element.isNull());
    TreeBaseXML::loadFromDomElement(dom_element, dom_doc, err_msg);
    
    if (m_dom_element.tagName() != CHART_NODE_NAME_NAVAIDS_VOR)
    {
        err_msg = QString("Wrong node name (%s), expected (%s)").
                  arg(m_dom_element.tagName()).arg(CHART_NODE_NAME_NAVAIDS_VOR);
        return false;
    }

    bool ok1 = false, ok2 = false;
    int x = dom_element.attribute(CHART_ATTR_LABEL_X_DIFF).toInt(&ok1);
    int y = dom_element.attribute(CHART_ATTR_LABEL_Y_DIFF).toInt(&ok2);
    if (ok1 && ok2) m_ge_vor->getLabel()->setDiffXY(QPoint(x, y));

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool ChartElemVor::saveToDomElement()
{
    m_dom_element.setAttribute(CHART_ATTR_ID, m_ge_vor->getVor()->getId());
    m_dom_element.setAttribute(CHART_ATTR_CTRY, m_ge_vor->getVor()->getCountryCode());
    m_dom_element.setAttribute(CHART_ATTR_NAME, m_ge_vor->getVor()->getName());

    writeDOMDoubleAttribute(m_dom_element, CHART_ATTR_LAT, m_ge_vor->getVor()->getLat());
    writeDOMDoubleAttribute(m_dom_element, CHART_ATTR_LON, m_ge_vor->getVor()->getLon());
    writeDOMIntAttribute(m_dom_element, CHART_ATTR_FREQ, m_ge_vor->getVor()->getFreq());
    writeDOMIntAttribute(m_dom_element, CHART_ATTR_DME, m_ge_vor->getVor()->hasDME());

    writeDOMIntAttribute(m_dom_element, CHART_ATTR_LABEL_X_DIFF, 
                         (int)m_ge_vor->getLabel()->getDiffXY().x());
    writeDOMIntAttribute(m_dom_element, CHART_ATTR_LABEL_Y_DIFF, 
                         (int)m_ge_vor->getLabel()->getDiffXY().y());
    return true;
}

/////////////////////////////////////////////////////////////////////////////
 
// End of file
