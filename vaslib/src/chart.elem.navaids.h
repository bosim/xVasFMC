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

/*! \file    chart.elem.navaids.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __CHART_ELEM_NAVAIDS_H__
#define __CHART_ELEM_NAVAIDS_H__

#include <QDate>

#include "vor.h"
#include "ndb.h"
#include "airport.h"
#include "intersection.h"

#include "treebase_xml_chart.h"

class ChartModel;

/////////////////////////////////////////////////////////////////////////////

//! navaids element for charts
class ChartElemNavaids : public TreeBaseXMLChart
{
    Q_OBJECT

public:
    //! Standard Constructor
    ChartElemNavaids(TreeBaseXMLChart* parent, ChartModel* chart_model);

    //! Destructor
    virtual ~ChartElemNavaids();

    virtual bool loadFromDomElement(QDomElement& dom_element, 
                                    QDomDocument& dom_doc,
                                    QString& err_msg);

    //! adds the given VOR, returns true on success
    bool addVor(const Vor& vor, QDomElement& element, 
                QDomDocument& dom_doc, QString& err_msg);
    
    //! adds the given VOR, returns true on success
    bool addNdb(const Ndb& vor, QDomElement& element, 
                QDomDocument& dom_doc, QString& err_msg);

    //! adds the given VOR, returns true on success
    bool addAirport(const Airport& vor, QDomElement& element, 
                    QDomDocument& dom_doc, QString& err_msg);

    //! adds the given INTERSECTION, returns true on success
    bool addIntersection(const Intersection& fix, QDomElement& element, 
                         QDomDocument& dom_doc, QString& err_msg);
    
    //-----

    //! returns a pointer to the navaid with the given ID and type.
    const Waypoint* getNavaid(const QString& id, Waypoint::WptType type) const;

    //! returns a pointer to the navaid with the given ID and type.
    const Waypoint* getNavaid(const QString& id, const QString& type) const;

    //-----

    //! adds the navaid with the given ID and type.
    bool addNewNavaid(const QString& id, const QString& type_string, QString& err_msg);

    //! adds the given VOR, returns true on success
    bool addNewVor(const QString& vor_id, QString& err_msg);

    //! adds the given NDB, returns true on success
    bool addNewNdb(const QString& ndb_id, QString& err_msg);

    //! adds the given airport, returns true on success
    bool addNewAirport(const QString& airport_id, QString& err_msg);

    //! adds the given intersection, returns true on success
    bool addNewIntersection(const QString& intersection_id, QString& err_msg);

    //! adds the given lat/lon point, returns true on success
    bool addNewLatLonPoint(const QString& id, double lat, double lon, QString& err_msg);

protected:

    //! Returns the lead ID for the given waypoint
    QString getLeafID(const Waypoint& wpt) const { return wpt.getId()+":"+wpt.getTypeString(); }

    //! Returns the lead ID for the given waypoint
    QString getLeafID(const QString& id, Waypoint::WptType type) const
    { return id+":"+Waypoint::getTypeString(type); }
    
    //! Sets the center of the projection, e.g. when the first navaid item is
    //! added to the navaid list.
    void setProjectionCenterWhenAppropriate(const Waypoint* wpt);

private:
    //! Hidden copy-constructor
    ChartElemNavaids(const ChartElemNavaids&);
    //! Hidden assignment operator
    const ChartElemNavaids& operator = (const ChartElemNavaids&);
};



#endif /* __CHART_ELEM_NAVAIDS_H__ */

// End of file

