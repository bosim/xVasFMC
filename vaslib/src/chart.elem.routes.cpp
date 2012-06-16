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

/*! \file    chart.elem.routes.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "ge_route.h"
#include "ge_layers.h"
#include "chartmodel.h"
#include "chart.xml.defines.h"
#include "chart.elem.navaids.h"
#include "chart.elem.routes.h"
#include "chart.elem.route.h"

/////////////////////////////////////////////////////////////////////////////

ChartElemRoutes::ChartElemRoutes(TreeBaseXMLChart* parent, ChartModel* chart_model) : 
    TreeBaseXMLChart(parent, chart_model)
{
}

/////////////////////////////////////////////////////////////////////////////

ChartElemRoutes::~ChartElemRoutes()
{
}

/////////////////////////////////////////////////////////////////////////////

bool ChartElemRoutes::loadFromDomElement(QDomElement& dom_element, 
                                    QDomDocument& dom_doc,
                                    QString& err_msg)
{
    MYASSERT(!dom_element.isNull());
    TreeBaseXML::loadFromDomElement(dom_element, dom_doc, err_msg);

    if (m_dom_element.tagName() != CHART_NODE_NAME_ROUTES)
    {
        err_msg = QString("Wrong node name (%s), expected (%s)").
                  arg(m_dom_element.tagName()).arg(CHART_NODE_NAME_ROUTES);
        return false;
    }

    // loop through all routes

    QDomNode node = m_dom_element.firstChild();
    for(; !node.isNull(); node = node.nextSibling())
    {
        QDomElement element = node.toElement();
        if (element.isNull()) continue;
        if (element.tagName() != CHART_NODE_NAME_ROUTES_ROUTE) continue;

        QString name = element.attribute(CHART_ATTR_NAME);
        if (name.isEmpty()) continue;

        if (containsLeaf(name))
        {
            err_msg = QString("Routes: Double route %1").arg(name);
            return false;
        }

        ChartElemRoute *chart_elem_route = new ChartElemRoute(this, m_chart_model);
        MYASSERT(chart_elem_route != 0);
        addLeaf(name, chart_elem_route);
        if (!chart_elem_route->loadFromDomElement(element, dom_doc, err_msg)) return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////

ChartElemRoute* ChartElemRoutes::addRoute(const QString& name, 
                                          const QString& type,
                                          QString& err_msg)
{
    MYASSERT(!name.isEmpty());
    MYASSERT(!type.isEmpty());

    if (containsRoute(name))
    {
        //FIXXME
        printf("ChartElemRoutes:addRoute: route (%s) exists\n", name.toLatin1().data());
        fflush(stdout);

        ChartElemRoute* route = getRoute(name);
        MYASSERT(route != 0);
        return route;
    }
    
    m_chart_model->setDirty();

    QDomDocument dom_doc = m_dom_element.ownerDocument();
    QDomElement element = createNode(m_dom_element, CHART_NODE_NAME_ROUTES_ROUTE);
    element.setAttribute(CHART_ATTR_NAME, name);
    element.setAttribute(CHART_ATTR_TYPE, type);

    ChartElemRoute *chart_elem_route = new ChartElemRoute(this, m_chart_model);
    MYASSERT(chart_elem_route != 0);
    if (!chart_elem_route->loadFromDomElement(element, dom_doc, err_msg)) 
    {
        delete chart_elem_route;
        m_dom_element.removeChild(element);
        return 0;
    }

    MYASSERT(chart_elem_route->getName() == name);

    //FIXXME
    printf("add leaf: %s\n", chart_elem_route->getName().toLatin1().data());
    fflush(stdout);

    addLeaf(name, chart_elem_route);
    return chart_elem_route;
}

/////////////////////////////////////////////////////////////////////////////

ChartElemRoute* ChartElemRoutes::getRoute(const QString& name) 
{
    MYASSERT(!name.isEmpty());
    TreeBase* leaf = getLeaf(name);
    if (!leaf) return 0;
    MYASSERT(leaf->inherits("ChartElemRoute"));
    return (ChartElemRoute*)leaf; 
}

/////////////////////////////////////////////////////////////////////////////
 
// End of file
