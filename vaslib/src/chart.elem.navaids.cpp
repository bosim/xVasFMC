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

/*! \file    chart.elem.navaids.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "projection.h"
#include "navdata.h"
#include "chart.xml.defines.h"
#include "ge_layers.h"

#include "ge_vor.h"
#include "ge_ndb.h"
#include "ge_airport.h"
#include "ge_intersection.h"

#include "chartmodel.h"
#include "chart.elem.vor.h"
#include "chart.elem.ndb.h"
#include "chart.elem.airport.h"
#include "chart.elem.intersection.h"

#include "chart.elem.navaids.h"

/////////////////////////////////////////////////////////////////////////////

ChartElemNavaids::ChartElemNavaids(TreeBaseXMLChart* parent, ChartModel* chart_model) : 
    TreeBaseXMLChart(parent, chart_model)
{
}

/////////////////////////////////////////////////////////////////////////////

ChartElemNavaids::~ChartElemNavaids()
{
}

/////////////////////////////////////////////////////////////////////////////

bool ChartElemNavaids::loadFromDomElement(QDomElement& dom_element, 
                                          QDomDocument& dom_doc,
                                          QString& err_msg)
{
    MYASSERT(!dom_element.isNull());
    TreeBaseXML::loadFromDomElement(dom_element, dom_doc, err_msg);

    if (m_dom_element.tagName() != CHART_NODE_NAME_NAVAIDS)
    {
        err_msg = QString("Wrong node name (%s), expected (%s)").
                  arg(m_dom_element.tagName()).arg(CHART_NODE_NAME_NAVAIDS);
        return false;
    }

    // loop through all navaids

    QDomNode node = m_dom_element.firstChild();
    for(; !node.isNull(); node = node.nextSibling())
    {
        QDomElement element = node.toElement();
        if (element.isNull()) continue;

        // extract info from node

        QString id = element.attribute(CHART_ATTR_ID);
        QString ctry = element.attribute(CHART_ATTR_CTRY);
        QString name = element.attribute(CHART_ATTR_NAME);
        double lat = element.attribute(CHART_ATTR_LAT).toDouble();
        double lon = element.attribute(CHART_ATTR_LON).toDouble();
        int freq = element.attribute(CHART_ATTR_FREQ).toUInt();
        bool has_dme = element.attribute(CHART_ATTR_DME).toInt();
        int elevation = element.attribute(CHART_ATTR_ELEV).toInt();

        // check info

        if (id.isEmpty())
        {
            err_msg = QString("Navaids: Missing ID at tag named (%1)").arg(element.tagName());
            return false;
        }

        // process element

        if (element.tagName() == CHART_NODE_NAME_NAVAIDS_VOR)
        {
            Vor* vor = 0;

            if (!name.isEmpty() && lat != 0.0 && lon != 0.0 && freq != 0)
                vor = new Vor(id, name, lat, lon, freq, has_dme, 0, 0, ctry);
            else
                vor = (Vor*)m_chart_model->getNavdata()->getElementsWithSignal(
                    id, ctry, Waypoint::VOR);

            if (!vor)
            {
                err_msg = QString("Navaids:VOR: unknown VOR with ID (%1)").arg(id);
                return false;
            }

            if (!addVor(*vor, element, dom_doc, err_msg))
            {
                node = node.previousSibling();
                m_dom_element.removeChild(element);
            }
            
            delete vor;
        }
        else if (element.tagName() == CHART_NODE_NAME_NAVAIDS_NDB)
        {
            Ndb* ndb = 0;

            if (!name.isEmpty() && lat != 0.0 && lon != 0.0 && freq != 0)
                ndb = new Ndb(id, name, lat, lon, freq, 0, 0, ctry);
            else
                ndb = (Ndb*)m_chart_model->getNavdata()->getElementsWithSignal(
                    id, ctry, Waypoint::NDB);

            if (!ndb)
            {
                err_msg = QString("Navaids:NDB: unknown NDB with ID (%1)").arg(id);
                return false;
            }

            if (!addNdb(*ndb, element, dom_doc, err_msg))
            {
                node = node.previousSibling();
                m_dom_element.removeChild(element);
            }
            
            delete ndb;
        }
        else if (element.tagName() == CHART_NODE_NAME_NAVAIDS_AIRPORT)
        {
            Airport* airport = 0;

            if (!name.isEmpty() && lat != 0.0 && lon != 0.0 && 
                element.hasAttribute(CHART_ATTR_ELEV))
                airport = new Airport(id, name, lat, lon, elevation);
            else
                airport = (Airport*)m_chart_model->getNavdata()->getElementsWithSignal(
                    id, ctry, Waypoint::AIRPORT);

            if (!airport)
            {
                err_msg = QString("Navaids:AIRPORT: unknown AIRPORT with ID (%1)").arg(id);
                return false;
            }

            if (!addAirport(*airport, element, dom_doc, err_msg))
            {
                node = node.previousSibling();
                m_dom_element.removeChild(element);
            }

            delete airport;
        }
        else if (element.tagName() == CHART_NODE_NAME_NAVAIDS_INTERSECTION)
        {
            Intersection* intersection = 0;
            
            if (lat != 0.0 && lon != 0.0)
                intersection = new Intersection(id, lat, lon, ctry);
            else
                intersection = (Intersection*)m_chart_model->getNavdata()->getElementsWithSignal(
                    id, ctry, Waypoint::INTERSECTION);

            if (!intersection)
            {
                err_msg = 
                    QString("Navaids:INTERSECTION: unknown INTERSECTION with ID (%1)").arg(id);
                return false;
            }

            if (!addIntersection(*intersection, element, dom_doc, err_msg))
            {
                node = node.previousSibling();
                m_dom_element.removeChild(element);
            }

            delete intersection;
        }
    }
    
    return true;
}

/////////////////////////////////////////////////////////////////////////////
 
bool ChartElemNavaids::addVor(const Vor& vor, 
                              QDomElement& element, 
                              QDomDocument& dom_doc,
                              QString& err_msg)
{
    QString leaf_id = getLeafID(vor);

    // check for double entries
    if (containsLeaf(leaf_id)) 
    {
        err_msg = QString("Double VOR entry detected: (%1)").arg(vor.getId());
        return false;
    }
 
    // process the VOR

    ChartElemVor *chart_elem_vor = new ChartElemVor(this, m_chart_model, vor);
    MYASSERT(chart_elem_vor != 0);

    if (!chart_elem_vor->loadFromDomElement(element, dom_doc, err_msg)) 
    {
        delete chart_elem_vor;
        return false;
    }

    addLeaf(leaf_id, chart_elem_vor);
    return true;
}

/////////////////////////////////////////////////////////////////////////////

void ChartElemNavaids::setProjectionCenterWhenAppropriate(const Waypoint* wpt)
{
    if (count() == 1)
        m_chart_model->getProjection()->setCenter(*wpt);
}

/////////////////////////////////////////////////////////////////////////////

bool ChartElemNavaids::addNewVor(const QString& vor_id, QString& err_msg)
{
    const Waypoint* existing_vor = getNavaid(vor_id, Waypoint::VOR);
    if (existing_vor)
    {
        err_msg = QString("Navaids:VOR: VOR with ID (%1) already exists").arg(vor_id);
        printf("***\n");fflush(stdout);
        return false;
    }

    Vor *vor = (Vor*)m_chart_model->getNavdata()->getElementsWithSignal(vor_id, "", Waypoint::VOR);
    
    if (!vor)
    {
        err_msg = QString("Navaids:VOR: unknown VOR with ID (%1)").arg(vor_id);
        delete vor;
        return false;
    }

    QDomDocument dom_doc = m_dom_element.ownerDocument();
    QDomElement new_element = dom_doc.createElement(CHART_NODE_NAME_NAVAIDS_VOR);
    m_dom_element.appendChild(new_element);

    bool added = addVor(*vor, new_element, dom_doc, err_msg);
    if (!added) m_dom_element.removeChild(new_element);
    setProjectionCenterWhenAppropriate(vor);
    delete vor;
    return added;
}

/////////////////////////////////////////////////////////////////////////////

bool ChartElemNavaids::addNdb(const Ndb& ndb,
                              QDomElement& element, 
                              QDomDocument& dom_doc,
                              QString& err_msg)
{
    QString leaf_id = getLeafID(ndb);

    // check for double entries
    if (containsLeaf(leaf_id)) 
    {
        err_msg = QString("Double NDB entry detected: (%1)").arg(ndb.getId());
        return false;
    }
    
    // process the NDB

    ChartElemNdb* chart_elem_ndb = new ChartElemNdb(this, m_chart_model, ndb);
    MYASSERT(chart_elem_ndb != 0);

    if (!chart_elem_ndb->loadFromDomElement(element, dom_doc, err_msg)) 
    {
        delete chart_elem_ndb;
        return false;
    }

    addLeaf(leaf_id, chart_elem_ndb);
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool ChartElemNavaids::addNewNdb(const QString& ndb_id, QString& err_msg)
{
    const Waypoint* existing_ndb = getNavaid(ndb_id, Waypoint::NDB);
    if (existing_ndb)
    {
        err_msg = QString("Navaids:NDB: NDB with ID (%1) already exists").arg(ndb_id);
        return false;
    }

    Ndb *ndb = (Ndb*)m_chart_model->getNavdata()->getElementsWithSignal(ndb_id, "", Waypoint::NDB);
    
    if (!ndb)
    {
        err_msg = QString("Navaids:NDB: unknown NDB with ID (%1)").arg(ndb_id);
        delete ndb;
        return false;
    }

    QDomDocument dom_doc = m_dom_element.ownerDocument();
    QDomElement new_element = dom_doc.createElement(CHART_NODE_NAME_NAVAIDS_NDB);
    m_dom_element.appendChild(new_element);

    bool ret = addNdb(*ndb, new_element, dom_doc, err_msg);
    setProjectionCenterWhenAppropriate(ndb);
    delete ndb;
    return ret;
}

/////////////////////////////////////////////////////////////////////////////

bool ChartElemNavaids::addAirport(const Airport& airport,
                                  QDomElement& element, 
                                  QDomDocument& dom_doc,
                                  QString& err_msg)
{
    QString leaf_id = getLeafID(airport);

    // check for double entries
    if (containsLeaf(leaf_id)) 
    {
        err_msg = QString("Double Aiport entry detected: (%1)").arg(airport.getId());
        return false;
    }

    // process the AIRPORT

    ChartElemAirport* chart_elem_airport = new ChartElemAirport(this, m_chart_model, airport);
    MYASSERT(chart_elem_airport);

    if (!chart_elem_airport->loadFromDomElement(element, dom_doc, err_msg)) 
    {
        delete chart_elem_airport;
        return false;
    }

    addLeaf(leaf_id, chart_elem_airport);
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool ChartElemNavaids::addNewAirport(const QString& airport_id, QString& err_msg)
{
    const Waypoint* existing_airport = getNavaid(airport_id, Waypoint::AIRPORT);
    if (existing_airport)
    {
        err_msg = QString("Navaids:AIRPORT: Airport with ID (%1) already exists").arg(airport_id);
        return false;
    }
    
    Airport *airport = (Airport*)m_chart_model->getNavdata()->getElementsWithSignal(
        airport_id, "", Waypoint::AIRPORT);
    
    if (!airport)
    {
        err_msg = QString("Navaids:AIRPORT: unknown airport with ID (%1)").arg(airport_id);
        delete airport;
        return false;
    }

    QDomDocument dom_doc = m_dom_element.ownerDocument();
    QDomElement new_element = dom_doc.createElement(CHART_NODE_NAME_NAVAIDS_AIRPORT);
    m_dom_element.appendChild(new_element);

    bool ret = addAirport(*airport, new_element, dom_doc, err_msg);
    setProjectionCenterWhenAppropriate(airport);
    delete airport;
    return ret;
}

/////////////////////////////////////////////////////////////////////////////   

bool ChartElemNavaids::addIntersection(const Intersection& intersection, QDomElement& element, 
                                       QDomDocument& dom_doc, QString& err_msg)
{
    QString leaf_id = getLeafID(intersection);

    // check for double entries
    if (containsLeaf(leaf_id)) 
    {
        err_msg = QString("Double Intersection entry detected: (%1)").arg(intersection.getId());
        return false;
    }
    
    // process the INTERSECTION

    ChartElemIntersection* chart_elem_intersection = 
        new ChartElemIntersection(this, m_chart_model, intersection);
    MYASSERT(chart_elem_intersection);
    
    if (!chart_elem_intersection->loadFromDomElement(element, dom_doc, err_msg)) 
    {
        delete chart_elem_intersection;
        return false;
    }

    addLeaf(leaf_id, chart_elem_intersection);
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool ChartElemNavaids::addNewIntersection(const QString& intersection_id, QString& err_msg)
{
    const Waypoint* existing_intersection = getNavaid(intersection_id, Waypoint::INTERSECTION);
    if (existing_intersection)
    {
        err_msg = QString("Navaids:INTERSECTION: Intersection with ID (%1) already exists").
                  arg(intersection_id);
        return false;
    }

    Intersection *intersection = (Intersection*)m_chart_model->getNavdata()->getElementsWithSignal(
        intersection_id, "", Waypoint::INTERSECTION);
    
    if (!intersection)
    {
        err_msg = QString("Navaids:INTERSECTION: unknown intersection with ID (%1)").
                  arg(intersection_id);
        delete intersection;
        return false;
    }

    QDomDocument dom_doc = m_dom_element.ownerDocument();
    QDomElement new_element = dom_doc.createElement(CHART_NODE_NAME_NAVAIDS_INTERSECTION);
    m_dom_element.appendChild(new_element);

    bool ret = addIntersection(*intersection, new_element, dom_doc, err_msg);
    setProjectionCenterWhenAppropriate(intersection);
    delete intersection;
    return ret;
}

/////////////////////////////////////////////////////////////////////////////

bool ChartElemNavaids::addNewLatLonPoint(const QString& id, double lat, double lon, QString& err_msg)
{
    MYASSERT(!id.isEmpty());

    const Waypoint* existing_intersection = getNavaid(id, Waypoint::INTERSECTION);
    if (existing_intersection)
    {
        err_msg = 
            QString("Navaids:LATLONPOINT: Intersection with ID (%1) already exists").arg(id);
        return false;
    }

    Intersection *intersection = new Intersection(id, lat, lon, QString::null);
    MYASSERT(intersection != 0);

    QDomDocument dom_doc = m_dom_element.ownerDocument();
    QDomElement new_element = dom_doc.createElement(CHART_NODE_NAME_NAVAIDS_INTERSECTION);
    m_dom_element.appendChild(new_element);

    bool ret = addIntersection(*intersection, new_element, dom_doc, err_msg);
    setProjectionCenterWhenAppropriate(intersection);
    delete intersection;
    return ret;
}

/////////////////////////////////////////////////////////////////////////////

const Waypoint* ChartElemNavaids::getNavaid(const QString& id, Waypoint::WptType type) const
{
    if (id.isEmpty()) return 0;
    QString leaf_id = getLeafID(id, type);
    const TreeBase* leaf = getLeaf(leaf_id);

    if (!leaf) return 0;
    //MYASSERT(leaf->inherits("ChartElemNavaid"));
    return ((ChartElemNavaid*)leaf)->getNavaid();
}

/////////////////////////////////////////////////////////////////////////////

const Waypoint* ChartElemNavaids::getNavaid(const QString& id, const QString& type) const
{
    return getNavaid(id, Waypoint::getTypeByText(type));
}

/////////////////////////////////////////////////////////////////////////////

bool ChartElemNavaids::addNewNavaid(const QString& id, const QString& type_string, QString& err_msg)
{
    Waypoint::WptType type = Waypoint::getTypeByText(type_string);

    switch(type)
    {
        case(Waypoint::AIRPORT): return addNewAirport(id, err_msg);
        case(Waypoint::INTERSECTION): return addNewIntersection(id, err_msg);
        case(Waypoint::NDB): return addNewNdb(id, err_msg);
        case(Waypoint::VOR): return addNewVor(id, err_msg);
        default: err_msg = QString("Unknown navaid type %1 (ID:%2").arg(type).arg(id); break;
    }

    return false;
}

// End of file
