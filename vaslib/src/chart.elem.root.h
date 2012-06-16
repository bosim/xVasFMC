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

/*! \file    chart.elem.root.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __CHART_ELEM_ROOT_H__
#define __CHART_ELEM_ROOT_H__

#include <QDate>

#include "treebase_xml_chart.h"

//! root element for charts
class ChartElemRoot : public TreeBaseXMLChart
{
    Q_OBJECT

public:
    //! Standard Constructor
    ChartElemRoot(ChartModel* chart_model);

    //! Destructor
    virtual ~ChartElemRoot();

    virtual bool loadFromDomElement(QDomElement& dom_element, 
                                    QDomDocument& dom_doc,
                                    QString& err_msg);

private:
    //! Hidden copy-constructor
    ChartElemRoot(const ChartElemRoot&);
    //! Hidden assignment operator
    const ChartElemRoot& operator = (const ChartElemRoot&);
};

#endif /* __CHART_ELEM_ROOT_H__ */

// End of file

