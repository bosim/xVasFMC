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

/*! \file    chart.elem.routes.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __CHART_ELEM_ROUTES_H__
#define __CHART_ELEM_ROUTES_H__

#include <QStringList>

#include "treebase_xml_chart.h"

class ChartElemRoute;

/////////////////////////////////////////////////////////////////////////////

//! routes element for charts
class ChartElemRoutes : public TreeBaseXMLChart
{
    Q_OBJECT

public:
    //! Standard Constructor
    ChartElemRoutes(TreeBaseXMLChart* parent, ChartModel* chart_model);

    //! Destructor
    virtual ~ChartElemRoutes();

    virtual bool loadFromDomElement(QDomElement& dom_element, 
                                    QDomDocument& dom_doc,
                                    QString& err_msg);
    
    bool containsRoute(const QString& name) const { return containsLeaf(name); }

    //! returns the added route on success, NULL an error.
    ChartElemRoute* addRoute(const QString& name, const QString& type, QString& err_msg);

    ChartElemRoute* getRoute(const QString& name);

    QStringList getRouteNames() 
    {
        QStringList list;
        TreeIterator iter = iterator();
        while (iter.hasNext()) { iter.next(); list.append(iter.key()); }
        return list;
    }

private:
    //! Hidden copy-constructor
    ChartElemRoutes(const ChartElemRoutes&);
    //! Hidden assignment operator
    const ChartElemRoutes& operator = (const ChartElemRoutes&);
};



#endif /* __CHART_ELEM_ROUTES_H__ */

// End of file

