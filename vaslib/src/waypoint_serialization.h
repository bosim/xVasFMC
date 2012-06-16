///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2008 Alexander Wemmer 
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

/*! \file    waypoint_serialization.h
    \author  Alexander Wemmer, alex@wemmer.at
*/


#ifndef __WAYPOINT_SERIALIZATION_H__
#define __WAYPOINT_SERIALIZATION_H__

#include "waypoint.h"
#include "ils.h"
#include "runway.h"
#include "waypoint_hdg_to_alt.h"
#include "waypoint_hdg_to_intercept.h"

/////////////////////////////////////////////////////////////////////////////

inline QDataStream &operator<<(QDataStream &stream, const WaypointPtrList& list)
{
    qint32 count = list.count();
    stream << count;

    for (int index=0; index < count; ++index) 
    {
        stream << list.at(index)->type();
        *list.at(index) >> stream;
    }

    return stream;
}

/////////////////////////////////////////////////////////////////////////////

inline QDataStream &operator>>(QDataStream &stream,  WaypointPtrList& list)
{
    qint32 count;
    stream >> count;

    QString wpt_type;

    list.clear();
    for (int index=0; index < count; ++index)
    {
        stream >> wpt_type;

        Waypoint* wpt = 0;

        if (wpt_type == Waypoint::TYPE_WAYPOINT)
            wpt = new Waypoint;
        else if (wpt_type == Waypoint::TYPE_AIRPORT)
            wpt = new Airport;
        else if (wpt_type == Waypoint::TYPE_RUNWAY)
            wpt = new Runway;
        else if (wpt_type == Waypoint::TYPE_INTERSECTION)
            wpt = new Intersection;
        else if (wpt_type == Waypoint::TYPE_NDB)
            wpt = new Ndb;
        else if (wpt_type == Waypoint::TYPE_VOR)
            wpt = new Vor;
        else if (wpt_type == Waypoint::TYPE_ILS)
            wpt = new Ils;
        else if (wpt_type == Waypoint::TYPE_HDG_TO_ALT)
            wpt = new WaypointHdgToAlt(QString::null, 0);
        else if (wpt_type == Waypoint::TYPE_HDG_TO_INTERCEPT)
            wpt = new WaypointHdgToIntercept(QString::null, 0.0, 0.0, Waypoint(), 0, 0, Navcalc::TURN_AUTO);

        if (wpt == 0)
        {
            Logger::log(QString("Waypoint:operator>>: got unknown waypoint type (%1) (%2 - %3 overall wpts) - aborting").
                        arg(wpt_type).arg(index).arg(count));
            list.clear();
            return stream;
        }

        list.append(wpt);
        *wpt << stream;

//         Logger::log(wpt->toString());
    }

    return stream;
}

#endif /* __WAYPOINT_SERIALIZATION_H__ */

// End of file

