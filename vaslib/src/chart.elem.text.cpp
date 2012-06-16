//////////////////////////////////////////////////////////////////////////////
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

/*! \file    chart.elem.text.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "projection.h"
#include "waypoint.h"
#include "chart.xml.defines.h"
#include "ge_layers.h"

#include "chartmodel.h"
#include "chart.elem.text.h"
#include "chart.elem.texts.h"

/////////////////////////////////////////////////////////////////////////////

ChartElemText::ChartElemText(TreeBaseXMLChart* parent, ChartModel* chart_model) :
    TreeBaseXMLChart(parent, chart_model), m_ge_textrect(0)
{
}

/////////////////////////////////////////////////////////////////////////////

ChartElemText::~ChartElemText()
{
    saveToDomElement();
    delete m_ge_textrect;
}

/////////////////////////////////////////////////////////////////////////////

void ChartElemText::init(QDomElement dom_element, 
                         const QString& text, 
                         QPointF lefttop_point,
                         bool lefttop_is_latlon,
                         GETextRect::AbsoluteRefCorner abs_ref_corner,
                         bool draw_border,
                         QString err_msg)
{
    MYASSERT(!dom_element.isNull());
    QDomDocument dom_doc = dom_element.ownerDocument();
    TreeBaseXML::loadFromDomElement(dom_element, dom_doc, err_msg);
    setTextNode(dom_element);

    delete m_ge_textrect;
    m_ge_textrect = new GETextRect(m_chart_model->getProjection(), text, 
                                   lefttop_point, lefttop_is_latlon, abs_ref_corner, draw_border);
    MYASSERT(m_ge_textrect);
    m_ge_textrect->setDataElement(this);

    m_chart_model->getGELayerList()->addElementByLayer(CHART_GE_LAYER_TEXTRECT, m_ge_textrect);
}

/////////////////////////////////////////////////////////////////////////////

bool ChartElemText::saveToDomElement()
{
    TreeBaseXML::saveToDomElement();

    // write the text back to the XML DOM
    if (m_ge_textrect)
    {
        QDomDocument dom_doc = m_dom_element.ownerDocument();

        writeDOMIntAttribute(m_dom_element, CHART_ATTR_BORDER, 
                             m_ge_textrect->doDrawBorder());
        writeDOMIntAttribute(m_dom_element, CHART_ATTR_ISLATLON, 
                             m_ge_textrect->isRefPointLatLon());

        writeDOMDoubleAttribute(m_dom_element, CHART_ATTR_POINT_X, 
                                m_ge_textrect->getRefPoint().x());
        writeDOMDoubleAttribute(m_dom_element, CHART_ATTR_POINT_Y, 
                                m_ge_textrect->getRefPoint().y());

        m_dom_element.setAttribute(CHART_ATTR_REFPOINT, m_ge_textrect->absRefCornerText());

        MYASSERT(!m_text_node.isNull());
        m_text_node.setData(m_ge_textrect->getText());
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////

void ChartElemText::setTextNode(QDomElement& dom_element)
{
    QDomNode node = m_dom_element.firstChild();
    for(; !node.isNull(); node = node.nextSibling())
    {
        m_text_node = node.toText();
        if (!m_text_node.isNull()) break;
    }
    
    if (m_text_node.isNull())
    {
        QDomDocument dom_doc = dom_element.ownerDocument();
        m_text_node = dom_doc.createTextNode("");
        dom_element.appendChild(m_text_node);
    }

    MYASSERT(!m_text_node.isNull());
}

/////////////////////////////////////////////////////////////////////////////

bool ChartElemText::loadFromDomElement(QDomElement& dom_element, 
                                        QDomDocument& dom_doc,
                                        QString& err_msg)
{
    MYASSERT(!dom_element.isNull());
    TreeBaseXML::loadFromDomElement(dom_element, dom_doc, err_msg);
    
    if (m_dom_element.tagName() != CHART_NODE_NAME_TEXTS_TEXT)
    {
        err_msg = QString("Wrong node name (%s), expected (%s)").
                  arg(m_dom_element.tagName()).arg(CHART_NODE_NAME_TEXTS_TEXT);
        return false;
    }

    // extract the text from the node

    setTextNode(dom_element);
    QString text = m_text_node.data().trimmed();

    // extract other info from node
    
    bool draw_border = dom_element.attribute(CHART_ATTR_BORDER).toInt();
    bool is_latlon = dom_element.attribute(CHART_ATTR_ISLATLON).toInt();
    QString abs_ref_corner_text = dom_element.attribute(CHART_ATTR_REFPOINT);
    
    bool ok_x = false, ok_y = false;
    double x = dom_element.attribute(CHART_ATTR_POINT_X).toDouble(&ok_x);
    double y = dom_element.attribute(CHART_ATTR_POINT_Y).toDouble(&ok_y);    
    
    if (!ok_x || !ok_y)
    {
        err_msg = QString("Text: Could not parse X/Y (%1/%2) of tag named (%3)").
                  arg(dom_element.attribute(CHART_ATTR_POINT_X)).
                  arg(dom_element.attribute(CHART_ATTR_POINT_Y)).
                  arg(dom_element.tagName());
        return false;
    }

    // check info
    
    delete m_ge_textrect;
    m_ge_textrect = new GETextRect(m_chart_model->getProjection(), text, 
                                   QPointF(x,y), is_latlon, 
                                   GETextRect::getAbsRefCornerFromText(abs_ref_corner_text),
                                   draw_border);
    
    MYASSERT(m_ge_textrect);
    m_ge_textrect->setDataElement(this);

    m_chart_model->getGELayerList()->addElementByLayer(CHART_GE_LAYER_TEXTRECT, m_ge_textrect);
    return true;
}

// End of file
