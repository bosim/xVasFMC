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

/*! \file    chart.elem.text.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __CHART_ELEM_TEXT_H__
#define __CHART_ELEM_TEXT_H__

#include <QPointF>

#include "ge_textrect.h"
#include "treebase_xml_chart.h"

/////////////////////////////////////////////////////////////////////////////

//! text element for charts
class ChartElemText : public TreeBaseXMLChart
{
    Q_OBJECT

public:
    //! Standard Constructor
    ChartElemText(TreeBaseXMLChart* parent, ChartModel* chart_model);

    //! Destructor
    virtual ~ChartElemText();

    void init(QDomElement dom_element, 
              const QString& text, 
              QPointF lefttop_point,
              bool lefttop_is_latlon,
              GETextRect::AbsoluteRefCorner abs_ref_corner,
              bool draw_border,
              QString err_msg);

    virtual bool loadFromDomElement(QDomElement& dom_element, 
                                    QDomDocument& dom_doc,
                                    QString& err_msg);
    
    virtual bool saveToDomElement();
    
    GETextRect* getGETextRect() const { return m_ge_textrect; }

protected:

    void setTextNode(QDomElement& dom_element);

protected:

    GETextRect* m_ge_textrect;
    QDomText m_text_node;
    
private:
    //! Hidden copy-constructor
    ChartElemText(const ChartElemText&);
    //! Hidden assignment operator
    const ChartElemText& operator = (const ChartElemText&);
};

#endif /* __CHART_ELEM_TEXT_H__ */

// End of file

