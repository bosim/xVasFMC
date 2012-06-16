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

/*! \file    chart.elem.route.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "projection.h"
#include "waypoint.h"
#include "ge_route.h"
#include "chart.xml.defines.h"
#include "ge_layers.h"

#include "chartmodel.h"
#include "chart.elem.navaids.h"
#include "chart.elem.navaid.h"
#include "chart.elem.route.h"
#include "chart.elem.routes.h"
#include "chart.elem.info.h"

/////////////////////////////////////////////////////////////////////////////

ChartElemRoute::ChartElemRoute(TreeBaseXMLChart* parent, ChartModel* chart_model) :
    TreeBaseXMLChart(parent, chart_model), m_ge_route(0)
{
}

/////////////////////////////////////////////////////////////////////////////

ChartElemRoute::~ChartElemRoute()
{
    saveToDomElement();
    delete m_ge_route;
}

/////////////////////////////////////////////////////////////////////////////

bool ChartElemRoute::saveToDomElement()
{
    TreeBaseXML::saveToDomElement();

    // write the route back to the XML DOM
    if (m_ge_route)
    {
        QDomDocument dom_doc = m_dom_element.ownerDocument();

        // clear all fix nodes
        while(m_dom_element.hasChildNodes())
        {
            QDomNode node = m_dom_element.firstChild();
            m_dom_element.removeChild(node);
        }

        const Route& route = m_ge_route->getRoute();

        if (route.count() < 2)
        {
            // if the route is empty or invalid, remove it

            QDomNode parent = m_dom_element.parentNode();
            parent.removeChild(m_dom_element);
        }
        else
        {
            // write the current fix nodes to the XML DOM

            m_dom_element.setAttribute(CHART_ATTR_NAME, m_ge_route->getName());
            m_dom_element.setAttribute(CHART_ATTR_TYPE, m_ge_route->getType());

            unsigned int max_count = route.count();
            for (unsigned int index = 0; index < max_count; ++index)
            {
                const Waypoint* wpt = route.getFix(index);
                const RouteFixData& fixdata = route.getFixData(index);
                MYASSERT(wpt != 0);

                QDomElement element = dom_doc.createElement(CHART_NODE_NAME_ROUTES_FIX);
                m_dom_element.appendChild(element);

                element.setAttribute(CHART_ATTR_ID, wpt->getId());
                element.setAttribute(
                    CHART_ATTR_TYPE, ChartElemNavaid::convertWptTypeToXMLType(wpt->getType()));
                
                if (!fixdata.m_alt_restriction.isEmpty())
                    element.setAttribute(CHART_ATTR_ALT, fixdata.m_alt_restriction);

                element.setAttribute(
                    CHART_ATTR_LABEL, fixdata.m_label_list.join(CHART_LABEL_SEPARATOR));
            }
        }
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool ChartElemRoute::loadFromDomElement(QDomElement& dom_element, 
                                        QDomDocument& dom_doc,
                                        QString& err_msg)
{
    MYASSERT(!dom_element.isNull());
    TreeBaseXML::loadFromDomElement(dom_element, dom_doc, err_msg);
    
    if (m_dom_element.tagName() != CHART_NODE_NAME_ROUTES_ROUTE)
    {
        err_msg = QString("Wrong node name (%s), expected (%s)").
                  arg(m_dom_element.tagName()).arg(CHART_NODE_NAME_ROUTES_ROUTE);
        return false;
    }

    // extract info from node
    
    QString name = dom_element.attribute(CHART_ATTR_NAME);
    QString type = dom_element.attribute(CHART_ATTR_TYPE);
    
    // check info
    
    if (name.isEmpty())
    {
        err_msg = QString("Route: Missing NAME at tag named (%1)").arg(dom_element.tagName());
        return false;
    }
    
    if (type.isEmpty()) type = "unknown";

    // loop through all fixes

    delete m_ge_route;
    m_ge_route = new GERoute(m_chart_model->getProjection(), name, type, 
                             m_chart_model->getElemInfo()->getMagneticVariation());
    MYASSERT(m_ge_route);
    m_ge_route->setDataElement(this);

    MYASSERT(connect(m_chart_model->getElemInfo(), SIGNAL(signalNewMagneticVariation(const double&)),
                     m_ge_route, SLOT(slotNewMagneticVariation(const double&))));
    
    bool found_route_name_label = false;

    QDomNode node = m_dom_element.firstChild();
    for(; !node.isNull(); node = node.nextSibling())
    {
        QDomElement element = node.toElement();
        if (element.isNull()) continue;
        if (element.tagName() != CHART_NODE_NAME_ROUTES_FIX) continue;
        
        QString id = element.attribute(CHART_ATTR_ID);
        QString type = element.attribute(CHART_ATTR_TYPE);

        if (id.isEmpty())
        {
            err_msg = QString("Route Fix: Missing fix ID at tag named (%1)").arg(element.tagName());
            delete m_ge_route;
            m_ge_route = 0;
            return false;
        }
        
        if (type.isEmpty())
        {
            err_msg = QString("Route Fix : Missing TYPE at tag named (%1)").arg(element.tagName());
            delete m_ge_route;
            m_ge_route = 0;
            return false;
        }
        
        const Waypoint* wpt = 0;
        unsigned int count = 0;

        while(++count <= 2)
        {
            wpt = m_chart_model->getElemNavaids()->getNavaid(id, type);
            if (!wpt)
            {
                if (count == 1)
                {
                    if (!m_chart_model->getElemNavaids()->addNewNavaid(id, type, err_msg))
                    {
                        delete m_ge_route;
                        m_ge_route = 0;
                        return false;
                    }
                }
                else
                {
                    err_msg = QString("Route: Could not find fix %1 of type %2 in chart").
                              arg(id).arg(type);
                    delete m_ge_route;
                    m_ge_route = 0;
                    return false;
                }
            }
        }

        // extract extra fix data
       
        RouteFixData fix_data;

        if (element.hasAttribute(CHART_ATTR_ALT))
        {
            fix_data.m_alt_restriction = element.attribute(CHART_ATTR_ALT);
        }

        if (!element.attribute(CHART_ATTR_LABEL).isEmpty())
        {
            fix_data.m_label_list = element.attribute(CHART_ATTR_LABEL).split(CHART_LABEL_SEPARATOR);
            if (fix_data.m_label_list.contains(CHART_ATTR_LABEL_PARAM_ROUTE)) 
                found_route_name_label = true;
        }

        // add fix to route
        m_ge_route->getRoute().addFix(*wpt, fix_data);
    }

    if (m_ge_route->getRoute().count() > 1 && !found_route_name_label)
        m_ge_route->getRoute().addLabelEntryToFixData(1, CHART_ATTR_LABEL_PARAM_ROUTE);

    MYASSERT(m_ge_route);
    m_chart_model->getGELayerList()->addElementByLayer(CHART_GE_LAYER_ROUTES, m_ge_route);
    return true;
}

/////////////////////////////////////////////////////////////////////////////
 
const QString ChartElemRoute::getName() const
{
    if (!m_ge_route) return QString::null;
    return m_ge_route->getName();
}

/////////////////////////////////////////////////////////////////////////////

bool ChartElemRoute::setName(const QString& name, QString& err_msg)
{
    err_msg = QString::null;

    if (!m_ge_route)
    {
        err_msg = "Chart route element has no route set";
        return false;
    }

    MYASSERT(m_parent->inherits("ChartElemRoutes"));
    ChartElemRoutes* elem_routes = (ChartElemRoutes*)m_parent;
    
    if (elem_routes->containsRoute(name))
    {
        err_msg = QString("A route with name %1 already exists").arg(name);
        return false;
    }

    QString old_name = getName();    
    m_ge_route->setName(name);
    if (!elem_routes->renameLeaf(old_name, getName(), err_msg)) 
    {
        m_ge_route->setName(old_name);
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////

const QString ChartElemRoute::getType() const
{
    if (!m_ge_route) return QString::null;
    return m_ge_route->getType();
}

// End of file
