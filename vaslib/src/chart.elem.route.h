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

/*! \file    chart.elem.route.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __CHART_ELEM_ROUTE_H__
#define __CHART_ELEM_ROUTE_H__

#include "treebase_xml_chart.h"

class GERoute;

/////////////////////////////////////////////////////////////////////////////

//! route element for charts
class ChartElemRoute : public TreeBaseXMLChart
{
    Q_OBJECT

public:
    //! Standard Constructor
    ChartElemRoute(TreeBaseXMLChart* parent, ChartModel* chart_model);

    //! Destructor
    virtual ~ChartElemRoute();

    virtual bool loadFromDomElement(QDomElement& dom_element, 
                                    QDomDocument& dom_doc,
                                    QString& err_msg);

    virtual bool saveToDomElement();

    GERoute* getGERoute() const { return m_ge_route; }

    const QString getName() const;
    bool setName(const QString& name, QString& err_msg);

    const QString getType() const;

protected:

    GERoute* m_ge_route;
    
private:
    //! Hidden copy-constructor
    ChartElemRoute(const ChartElemRoute&);
    //! Hidden assignment operator
    const ChartElemRoute& operator = (const ChartElemRoute&);
};

#endif /* __CHART_ELEM_ROUTE_H__ */

// End of file

