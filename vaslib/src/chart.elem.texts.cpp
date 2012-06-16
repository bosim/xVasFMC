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

/*! \file    chart.elem.texts.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "projection.h"
#include "chart.xml.defines.h"
#include "ge_layers.h"

#include "chartmodel.h"
#include "chart.elem.text.h"

#include "chart.elem.texts.h"

/////////////////////////////////////////////////////////////////////////////

ChartElemTexts::ChartElemTexts(TreeBaseXMLChart* parent, ChartModel* chart_model) : 
    TreeBaseXMLChart(parent, chart_model), m_text_counter(0)
{
}

/////////////////////////////////////////////////////////////////////////////

ChartElemTexts::~ChartElemTexts()
{
}

/////////////////////////////////////////////////////////////////////////////

bool ChartElemTexts::loadFromDomElement(QDomElement& dom_element, 
                                        QDomDocument& dom_doc,
                                        QString& err_msg)
{
    MYASSERT(!dom_element.isNull());
    TreeBaseXML::loadFromDomElement(dom_element, dom_doc, err_msg);

    if (m_dom_element.tagName() != CHART_NODE_NAME_TEXTS)
    {
        err_msg = QString("Wrong node name (%s), expected (%s)").
                  arg(m_dom_element.tagName()).arg(CHART_NODE_NAME_TEXTS);
        return false;
    }

    // loop through all texts

    QDomNode node = m_dom_element.firstChild();
    for(; !node.isNull(); node = node.nextSibling())
    {
        QDomElement element = node.toElement();
        if (element.isNull()) continue;

//         // check for double entries
//         if (containsLeaf(title)) continue; //FIXXME remove the node !?

        // process the text
        
        ChartElemText *chart_elem_text = new ChartElemText(this, m_chart_model);
        MYASSERT(chart_elem_text != 0);
        
        if (!chart_elem_text->loadFromDomElement(element, dom_doc, err_msg)) 
        {
            delete chart_elem_text;
            return false;
        }
        
        addLeaf(QString::number(m_text_counter++), chart_elem_text);
    }
    
    return true;
}

/////////////////////////////////////////////////////////////////////////////
 
bool ChartElemTexts::addNewText(const QString& text, 
                                QPointF lefttop_point,
                                bool lefttop_is_latlon,
                                GETextRect::AbsoluteRefCorner abs_ref_corner,
                                bool draw_border,
                                QString err_msg)
{
    // create a new node

    QDomDocument dom_doc = m_dom_element.ownerDocument();
    QDomElement new_element = dom_doc.createElement(CHART_NODE_NAME_TEXTS_TEXT);
    m_dom_element.appendChild(new_element);

    // process the text

    ChartElemText *chart_elem_text = new ChartElemText(this, m_chart_model);
    MYASSERT(chart_elem_text != 0);
    chart_elem_text->init(new_element, text, lefttop_point, 
                          lefttop_is_latlon, abs_ref_corner, draw_border, err_msg);
    
    addLeaf(QString::number(m_text_counter++), chart_elem_text);
    return true;
}

// End of file
