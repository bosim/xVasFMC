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

#ifndef ROUTE_H
#define ROUTE_H

#include <QObject>
#include <QList>
#include <QStringList>
#include <QDateTime>

#include "serialization_iface.h"
#include "waypoint.h"
#include "ptrlist.h"

class Airway;
class Procedure;
class Sid;
class Star;
class Transition;
class Approach;
class Navdata;
class ProjectionBase;
class FlightStatus;

/////////////////////////////////////////////////////////////////////////////

class RouteData : public SerializationIface
{
public:

    RouteData() : m_true_track_to_next_wpt(0.0), m_dist_to_next_wpt_nm(0.0), 
                  m_true_track_from_prev_wpt(0.0), m_dist_from_prev_wpt_nm(0.0)
    {};

    virtual ~RouteData() {};

    virtual void operator<<(QDataStream& in);
    virtual void operator>>(QDataStream& out) const;

public:

    double m_true_track_to_next_wpt;
    double m_dist_to_next_wpt_nm;

    double m_true_track_from_prev_wpt;
    double m_dist_from_prev_wpt_nm;

    QTime m_time_over_waypoint;
};

/////////////////////////////////////////////////////////////////////////////

inline QDataStream &operator<<(QDataStream &stream, const RouteData& data)
{
    data >> stream;
    return stream;
}

/////////////////////////////////////////////////////////////////////////////

inline QDataStream &operator>>(QDataStream &stream,  RouteData& data)
{
    data << stream;
    return stream;
}

/////////////////////////////////////////////////////////////////////////////

class Route : public QObject, public SerializationIface
{
    Q_OBJECT

public:

    Route(const QString& id = QString::null, const FlightStatus* flightstatus = 0);
    virtual ~Route() {};

    Route(const Route&);
    const Route& operator=(const Route& other);

    virtual bool operator<(const Route& other) const { return id() < other.id(); }

    // compares the waypoints of the given route with this route and returns
    // true if all ID/LAT/LON values match.
    virtual bool compareWaypoints(const Route& other) const;
    
    virtual void operator<<(QDataStream& in);
    virtual void operator>>(QDataStream& out) const;

    /////////////////////////////////////////////////////////////////////////////

    static QString TYPE_ROUTE;
    static QString TYPE_AIRWAY;
    static QString TYPE_PROCEDURE;
    static QString TYPE_SID;
    static QString TYPE_STAR;
    static QString TYPE_APPROACH;
    static QString TYPE_TRANSITION;

    static QString FLAG_NORMAL;
    static QString FLAG_ALTERNATE;
    static QString FLAG_SECONDARY;
    static QString FLAG_TEMPORARY;

    /////////////////////////////////////////////////////////////////////////////

    virtual const Route* asRoute() const { return this; }
    virtual const Airway* asAirway() const { return 0; }
    virtual const Procedure* asProcedure() const { return 0; }
    virtual const Sid* asSID() const { return 0; }
    virtual const Star* asSTAR() const { return 0; }
    virtual const Transition* asTransition() const { return 0; }
    virtual const Approach* asApproach() const { return 0; }

    virtual Route* asRoute() { return this; }
    virtual Airway* asAirway() { return 0; }
    virtual Procedure* asProcedure() { return 0; }
    virtual Sid* asSID() { return 0; }
    virtual Star* asSTAR() { return 0; }
    virtual Transition* asTransition() { return 0; }
    virtual Approach* asApproach() { return 0; }

    /////////////////////////////////////////////////////////////////////////////

    const QString& type() const { return m_type; }
    void setType(QString type) { m_type = type; }

    const QString& flag() const {  return m_flag; }
    //! when fixed == true, the flag will not be overwritten on copy, etc.
    void setFlag(const QString& flag, bool fixed = false) 
    {
        m_flag = flag.trimmed(); 
        m_flag_fixed = fixed;
    }

    const QString& id() const { return m_id; }
    void setId(QString id) { m_id = id; }

    //! clears the waypoints and route data, does not touch the ID, flag and type.
    virtual void clear();
    int count() const { return m_wpt_list.count(); }
    inline const WaypointPtrList& waypointList() const { return m_wpt_list; }

    //! returns false if the given start index is behind the last waypoint
    virtual bool calcProjection(const ProjectionBase& projection, int start_index = 0, int end_index = -1);

    //-----

    virtual void appendWaypoint(const Waypoint& wpt);
    virtual void insertWaypoint(const Waypoint& wpt, int pos);
    virtual bool containsWaypoint(const Waypoint& wpt) const;
    virtual bool removeWaypoint(int pos);

    virtual void removeDoubleWaypoints();
    void scanForWaypointInformation(const Navdata& navdata);

    //-----

    inline const Waypoint* waypoint(int pos) const
    {
        if (pos < 0 || pos >= count()) return 0;
        return m_wpt_list.at(pos);
    }

    inline Waypoint* waypoint(int pos)
    {
        if (pos < 0 || pos >= count()) return 0;
        return m_wpt_list.at(pos);
    }

    inline const Waypoint* lastWaypoint() const { return waypoint(count()-1); }
    inline const Waypoint* firstWaypoint() const { return waypoint(0); }

    //-----

    inline const RouteData& routeData(int pos) const 
    {
        MYASSERT(pos >= 0);
        MYASSERT(pos < count());
        return m_routedata_list[pos];
    }

    inline RouteData& routeData(int pos)
    {
        MYASSERT(pos >= 0);
        MYASSERT(pos < count());
        return m_routedata_list[pos];
    }

    //-----

    //! returns the true heading to the next waypoint from the waypoint at pos
    inline const double& trueTrackToNextWaypoint(int pos) const
    {
        MYASSERT(pos >= 0);
        MYASSERT(pos < count());
        return m_routedata_list[pos].m_true_track_to_next_wpt;
    }

    //! returns the distance in NM to the next waypoint from the waypoint at pos
    inline const double& distanceNMToNextWaypoint(int pos) const
    {
        MYASSERT(pos >= 0);
        MYASSERT(pos < count());
        return m_routedata_list[pos].m_dist_to_next_wpt_nm;
    }

    //! returns the true heading from the previous waypoint from the waypoint at pos
    inline const double& trueTrackFromPrevWaypoint(int pos) const
    {
        MYASSERT(pos >= 0);
        MYASSERT(pos < count());
        return m_routedata_list[pos].m_true_track_from_prev_wpt;
    }

    //! returns the distance in NM from the previous waypoint from the waypoint at pos
    inline const double& distanceNMFromPrevWaypoint(int pos) const
    {
        MYASSERT(pos >= 0);
        MYASSERT(pos < count());
        return m_routedata_list[pos].m_dist_from_prev_wpt_nm;
    }

    //! returns the time over the waypoint
    inline const QTime& timeOverWaypoint(int pos) const
    {
        MYASSERT(pos >= 0);
        MYASSERT(pos < count());
        return m_routedata_list[pos].m_time_over_waypoint;
    }

signals:

    void signalChanged(const QString& flag, bool direct_change, const QString& comment) const;

protected:

    // recalculates the route data for the waypoint at the given pos
    void recalcWaypointData(int pos);

    // returns true if route or waypoint changes happened
    bool checkAndSetSpecialWaypoint(int pos, bool take_current_flightdata_as_reference);

    void resetAndRecalcSpecialWaypoints();

protected:

    const FlightStatus* m_flightstatus;

    QString m_type;
    QString m_flag;
    bool m_flag_fixed;
    QString m_id;

    WaypointPtrList m_wpt_list;
    QList<RouteData> m_routedata_list;

private:
};

typedef QList<Route> RouteValueList;
typedef QListIterator<Route> RouteValueListIterator;

typedef PtrList<Route> RoutePtrList;
typedef QListIterator<Route*> RoutePtrListIterator;

#endif
