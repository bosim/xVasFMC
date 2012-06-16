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

/*! \file    chart.elem.texts.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __CHART_ELEM_TEXTS_H__
#define __CHART_ELEM_TEXTS_H__

#include <QDate>
#include <QPointF>

#include "treebase_xml_chart.h"
#include "ge_textrect.h"

class ChartModel;

/////////////////////////////////////////////////////////////////////////////

//! texts element for charts
class ChartElemTexts : public TreeBaseXMLChart
{
    Q_OBJECT

public:
    //! Standard Constructor
    ChartElemTexts(TreeBaseXMLChart* parent, ChartModel* chart_model);

    //! Destructor
    virtual ~ChartElemTexts();

    virtual bool loadFromDomElement(QDomElement& dom_element, 
                                    QDomDocument& dom_doc,
                                    QString& err_msg);

    //! adds the given VOR, returns true on success
    bool addNewText(const QString& text, 
                    QPointF lefttop_point,
                    bool lefttop_is_latlon,
                    GETextRect::AbsoluteRefCorner abs_ref_corner,
                    bool draw_border,
                    QString err_msg);

protected:

    unsigned int m_text_counter;
    
private:
    //! Hidden copy-constructor
    ChartElemTexts(const ChartElemTexts&);
    //! Hidden assignment operator
    const ChartElemTexts& operator = (const ChartElemTexts&);
};



#endif /* __CHART_ELEM_TEXTS_H__ */

// End of file

