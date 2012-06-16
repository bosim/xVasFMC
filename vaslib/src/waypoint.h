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

#ifndef WAYPOINT_H
#define WAYPOINT_H

#include <QString>
#include <QList>
#include <QListIterator>
#include <QPointF>
#include <QObject>
#include <QDataStream>

#include "serialization_iface.h"
#include "holding.h"
#include "ptrlist.h"

class Airway;
class Airport;
class Ndb;
class Vor;
class Ils;
class Intersection;
class Runway;
class WaypointHdgToAlt;
class WaypointHdgToIntercept;

/////////////////////////////////////////////////////////////////////////////

class WaypointRestrictions : public SerializationIface
{
public:

    enum AltitudeResstrictionType { RESTRICTION_ALT_EQUAL = 0,
                                    RESTRICTION_ALT_GREATER,
                                    RESTRICTION_ALT_SMALLER
    };
    
    enum TurnRestrictionType { RESTRICTION_TURN_NONE = 0,
                               RESTRICTION_TURN_LEFT,
                               RESTRICTION_TURN_RIGHT
    };

    WaypointRestrictions() : 
        m_speed_restriction_kts(0), 
        m_altitude_restriction_ft(0), m_altitude_restriction_type(RESTRICTION_ALT_EQUAL),
        m_overfly_restriction(false), m_turn_restriction(RESTRICTION_TURN_NONE)
    {}

    virtual ~WaypointRestrictions() {};

    void clear()
    {
        m_speed_restriction_kts = 0; 
        m_altitude_restriction_ft = 0;
        m_altitude_restriction_type = RESTRICTION_ALT_EQUAL;
        m_overfly_restriction = false; 
        m_turn_restriction = RESTRICTION_TURN_NONE;
    }

    inline uint speedRestrictionKts() const { return m_speed_restriction_kts; }
    inline void setSpeedRestrictionKts(uint speed) 
    {
        m_speed_restriction_kts = speed; 
    }

    inline uint altitudeRestrictionFt() const { return m_altitude_restriction_ft; }
    inline void setAltitudeRestrictionFt(uint altitude) 
    {
        m_altitude_restriction_ft = altitude; 
    }
    
    inline bool altitudeEqualRestriction() const   { return m_altitude_restriction_type == RESTRICTION_ALT_EQUAL; }
    inline bool altitudeGreaterRestriction() const { return m_altitude_restriction_type == RESTRICTION_ALT_GREATER; }
    inline bool altitudeSmallerRestriction() const { return m_altitude_restriction_type == RESTRICTION_ALT_SMALLER; }
    void setAltitudeRestrictionType(AltitudeResstrictionType type) 
    {
        m_altitude_restriction_type = type; 
    }

    inline bool hasOverflyRestriction() const { return m_overfly_restriction; }
    inline void setOverflyRestriction(bool do_overfly) 
    {
        m_overfly_restriction = do_overfly; 
    }
    
    inline bool noTurnRestriction() const    { return m_turn_restriction == RESTRICTION_TURN_NONE; }
    inline bool leftTurnRestriction() const  { return m_turn_restriction == RESTRICTION_TURN_LEFT; }
    inline bool rightTurnRestriction() const { return m_turn_restriction == RESTRICTION_TURN_RIGHT; }
    inline void setTurnRestriction(TurnRestrictionType restriction) 
    {
        m_turn_restriction = restriction; 
    }

    //----- text methods

    QString speedAndAltitudeRestrictionText() const;
    QString altitudeRestrictionText() const;
    QString speedRestrictionText() const;
    bool setSpeedAndAltitudeRestrictionFromText(const QString& text);

    virtual void operator<<(QDataStream& in);
    virtual void operator>>(QDataStream& out) const;

protected:

    uint m_speed_restriction_kts;
    uint m_altitude_restriction_ft;
    AltitudeResstrictionType m_altitude_restriction_type;
    bool m_overfly_restriction;
    TurnRestrictionType m_turn_restriction;
};

/////////////////////////////////////////////////////////////////////////////

class WaypointMetaData : public SerializationIface
{
public:

    WaypointMetaData() : m_speed_kts(0), m_altitude_ft(0)
    {}

    virtual ~WaypointMetaData() {};

    void clear()
    {
        m_speed_kts = m_altitude_ft = 0;
    }

    inline uint speedKts() const { return m_speed_kts; }
    inline void setSpeedKts(uint speed) { m_speed_kts = speed; }

    inline uint altitudeFt() const { return m_altitude_ft; }
    inline void setAltitudeFt(uint altitude) { m_altitude_ft = altitude; }
    
    virtual void operator<<(QDataStream& in);
    virtual void operator>>(QDataStream& out) const;

protected:

    uint m_speed_kts;
    uint m_altitude_ft;
};

/////////////////////////////////////////////////////////////////////////////

class Waypoint : public SerializationIface
{
public:

    static const double LAT_LON_COMPARE_EPSILON;

    Waypoint();
    Waypoint(const QString& id, const QString& name, const double &lat, const double& lon);
    virtual ~Waypoint() {};

    virtual QString toString() const;
    
    //! Returns true, if the id, lat and lon values are equal
    virtual bool operator==(const Waypoint& other) const;
    //! inverted operator==()
    virtual bool operator!=(const Waypoint& other) const;

    const Waypoint& operator=(const Waypoint& other);

    virtual bool operator<(const Waypoint& other) const { return id() < other.id(); }

    virtual Waypoint* deepCopy() const { return new Waypoint(*this); }

    virtual void operator<<(QDataStream& in);
    virtual void operator>>(QDataStream& out) const;

    /////////////////////////////////////////////////////////////////////////////
    
    static QString TYPE_WAYPOINT;
    static QString TYPE_AIRPORT;
    static QString TYPE_RUNWAY;
    static QString TYPE_INTERSECTION;
    static QString TYPE_NDB;
    static QString TYPE_VOR;
    static QString TYPE_ILS;
    static QString TYPE_ALL;
    static QString TYPE_HDG_TO_ALT;
    static QString TYPE_HDG_TO_INTERCEPT;
    
    //TODO make flags enums!
    static QString FLAG_ADEP;
    static QString FLAG_SID;
    static QString FLAG_SID_TRANS;
    static QString FLAG_TOP_OF_CLIMB;
    static QString FLAG_TOP_OF_DESCENT;
    static QString FLAG_END_OF_DESCENT;
    static QString FLAG_STAR;
    static QString FLAG_APP_TRANS;
    static QString FLAG_APPROACH;
    static QString FLAG_ADES;
    static QString FLAG_MISSED_APPROACH;
    static QString FLAG_DISCONTINUITY;
    static QString FLAG_DCT;

    /////////////////////////////////////////////////////////////////////////////

    inline bool isAdep() const { return m_flag == FLAG_ADEP; }
    inline bool isSid() const { return m_flag == FLAG_SID; }
    inline bool isSidTransition() const { return m_flag == FLAG_SID_TRANS; }
    inline bool isTopOfClimb() const { return m_flag == FLAG_TOP_OF_CLIMB; }
    inline bool isTopOfDescent() const { return m_flag == FLAG_TOP_OF_DESCENT; }
    inline bool isEndOfDescent() const { return m_flag == FLAG_END_OF_DESCENT; }
    inline bool isStar() const { return m_flag == FLAG_STAR; }
    inline bool isAppTransition() const { return m_flag == FLAG_APP_TRANS; }
    inline bool isApproach() const { return m_flag == FLAG_APPROACH; }
    inline bool isAdes() const { return m_flag == FLAG_ADES; }
    inline bool isMissedApproach() const { return m_flag == FLAG_MISSED_APPROACH; }
    inline bool isDiscontinuity() const { return m_flag == FLAG_DISCONTINUITY; }
    inline bool isDirect() const { return m_flag == FLAG_DCT; }

    /////////////////////////////////////////////////////////////////////////////

    virtual const Waypoint* asWaypoint() const { return this; }
    virtual const Airport* asAirport() const { return 0; }
    virtual const Runway* asRunway() const { return 0; }
    virtual const Intersection* asIntersection() const { return 0; }
    virtual const Ndb* asNdb() const { return 0; }
    virtual const Vor* asVor() const { return 0; }
    virtual const Ils* asIls() const { return 0; }
    virtual const WaypointHdgToAlt* asWaypointHdgToAlt() const { return 0; }
    virtual const WaypointHdgToIntercept* asWaypointHdgToIntercept() const { return 0; }
    
    virtual Waypoint* asWaypoint() { return this; }
    virtual Airport* asAirport() { return 0; }
    virtual Runway* asRunway() { return 0; }
    virtual Intersection* asIntersection() { return 0; }
    virtual Ndb* asNdb() { return 0; }
    virtual Vor* asVor() { return 0; }
    virtual Ils* asIls() { return 0; }
    virtual WaypointHdgToAlt* asWaypointHdgToAlt() { return 0; }
    virtual WaypointHdgToIntercept* asWaypointHdgToIntercept() { return 0; }

    /////////////////////////////////////////////////////////////////////////////

    inline bool isValid() const { return m_is_valid; }

    inline const QString& type() const { return m_type; }

    inline QString id() const {  return m_id; }
    void setId(const QString& id) { m_id = id.trimmed(); }

    const QString& name() const {  return m_name; }
    void setName(const QString& name) { m_name = name.trimmed(); }
    
    const QString& flag() const { return m_flag; }
    void setFlag(const QString& flag) { m_flag = flag.trimmed(); }

    const QString& parent() const {  return m_parent; }
    void setParent(const QString& parent) { m_parent = parent.trimmed(); }

    inline double lat() const { return m_polar_coordinates.x(); }
    inline void setLat(const double& lat) { m_polar_coordinates.setX(lat); }

    inline double lon() const { return m_polar_coordinates.y(); }
    inline void setLon(const double& lon) { m_polar_coordinates.setY(lon); }

    inline QPointF pointLatLon() const { return m_polar_coordinates; }
    inline void setPointLatLon(const QPointF& point) { m_polar_coordinates = point; }

    inline double x() const { return m_cartesian_coordinates.x(); }
    inline double y() const { return m_cartesian_coordinates.y(); }

    inline QPointF pointXY() const { return m_cartesian_coordinates; }
    inline void setPointXY(const QPointF& point) { m_cartesian_coordinates = point; }

    QString latString() const;
    QString latStringDegMinSec() const;
    QString lonString() const;
    QString lonStringDegMinSec() const;
    inline QString latLonString() const { return latString() + lonString(); }

    inline const Holding& holding() const { return m_holding; }
    inline Holding& holding() { return m_holding; }
    inline void setHolding(const Holding& holding) { m_holding = holding; }
    
    inline const WaypointRestrictions& restrictions() const { return m_restrictions; }
    inline WaypointRestrictions& restrictions() { return m_restrictions; }
    inline void setRestrictions(const WaypointRestrictions& restrictions) { m_restrictions = restrictions; }

    inline const WaypointMetaData& estimatedData() const { return m_estimated_data; }
    inline WaypointMetaData& estimatedData() { return m_estimated_data; }
    inline void setEstimatedData(const WaypointMetaData& estimated_data) { m_estimated_data = estimated_data; }

    inline const WaypointMetaData& overflownData() const { return m_overflown_data; }
    inline WaypointMetaData& overflownData() { return m_overflown_data; }
    inline void setOverflownData(const WaypointMetaData& overflown_data) { m_overflown_data = overflown_data; }

    //! returns true if we got a dependend waypoint (e.g. hdg2alt, hdg2intercept, etc.)
    bool isDependendWaypoint() const;

    //! resets the waypoint coordinates if the waypoint is a dependend waypoint (see isDependendWaypoint())
    inline void resetIfDependendWaypoint() { if (isDependendWaypoint()) m_polar_coordinates = QPointF(); }

protected:

    bool m_is_valid;

    QString m_type;
    QString m_id;
    QString m_name;
    QString m_parent;
    QString m_flag;

    QPointF m_polar_coordinates;
    QPointF m_cartesian_coordinates;

    WaypointRestrictions m_restrictions;
    WaypointMetaData m_estimated_data;
    WaypointMetaData m_overflown_data;
    Holding m_holding;
};

typedef QList<Waypoint> WaypointValueList;
typedef QListIterator<Waypoint> WaypointValueListIterator;

typedef QListIterator<Waypoint*> WaypointPtrListIterator;
//typedef PtrList<Waypoint> WaypointPtrList;

/////////////////////////////////////////////////////////////////////////////

//! waypoint pointer list
class WaypointPtrList : public PtrList<Waypoint>
{
public:
    //! Standard Constructor
    WaypointPtrList() {};

    WaypointPtrList(const WaypointPtrList& other) : PtrList<Waypoint>(other)
    {
        *this = other;
    }

    const WaypointPtrList& operator=(const WaypointPtrList& other)
    {
        PtrList<Waypoint>::operator=(other);
        return *this;
    }

    //! Destructor
    virtual ~WaypointPtrList() {};

    //! sorts the list of waypoints by distance in respect to the given reference waypoint
    void sortByDistance(const Waypoint& reference_wpt);

    inline WaypointPtrList* deepCopy() const 
    { return static_cast<WaypointPtrList* >(PtrList<Waypoint>::deepCopy()); }
};

#endif
