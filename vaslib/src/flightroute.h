///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2007 Alexander Wemmer 
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

/*! \file    flightroute.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef FLIGHTROUTE_H
#define FLIGHTROUTE_H

#include "logger.h"

#include "route.h"

class Navdata;
class ProjectionBase;
class QRegExp;
class FlightStatus;

/////////////////////////////////////////////////////////////////////////////

//TODO add change detection to holding and route data and connect them to our signalChanged()

//! complete route with adep, ades, etc.
class FlightRoute : public Route
{
    Q_OBJECT

public:

    //! Standard Constructor
    FlightRoute(const FlightStatus* flightstatus, const QString& id = QString::null);

    //! Destructor
    virtual ~FlightRoute();

    FlightRoute(const FlightRoute& other):Route(other)  { *this = other; }
    const FlightRoute& operator=(const FlightRoute& other);

    void setProjection(const ProjectionBase* projection);

    //! returns false if the given start index is behind the last waypoint
    virtual bool calcProjection(const ProjectionBase& projection, int start_index = 0, int end_index = -1);

    virtual void operator<<(QDataStream& in);
    virtual void operator>>(QDataStream& out) const;

    //-----

    virtual void clear();
    virtual void appendWaypoint(const Waypoint& wpt, bool remove_double_wtps = false);
    virtual void insertWaypoint(const Waypoint& wpt, int pos, bool remove_double_wtps = false);
    virtual void goDirect(const uint dct_wpt_index, double turn_radius_nm, QString direct_wpt_name);
    virtual bool removeWaypoint(int pos);

    //! applies the given holding to the waypoint with the given index
    bool setHolding(uint wpt_index, const Holding& holding);
    
    //! sets the overfly restriction of the waypoint at the given index
    bool setWaypointOverflyRestriction(uint index, bool is_overfly);

    //-----

    // departure airport
    void setAsDepartureAirport(uint wpt_index, const QString& active_runway);
    int departureAirportIndex() const;
    const Airport* departureAirport() const;
    Airport* departureAirport();
    inline const QString& departureAirportId() const { return m_adep_id; }

    // SID sets the given SID, clears any SID and SID transition points
    bool setSid(const Sid& sid, const QString& runway);
    inline const QString& sidId() const { return m_sid_id; }

    // SID TRANS set the given SID transition, make sure setSid() was called before
    bool setSidTransition(const Transition& sid_transition);
    inline const QString& sidTransitionId() const { return m_sid_transition_id; }

    // STAR set the given star, removes any STAR, transition and approach points from the route
    bool setStar(const Star& star, const QString& runway);
    inline const QString& starId() const { return m_star_id; }

    // APP TRANS sets the given APP transition, make sure setStar() was called before
    bool setAppTransition(const Transition& app_transition, const QString& runway);
    inline const QString& appTransitionId() const { return m_app_transition_id; }

    // Approach sets the given approach, make sure setStar() was called before
    bool setApproach(const Approach& approach, const QString& runway);
    inline const QString& approachId() const { return m_approach_id; }

    // destination airport
    void setAsDestinationAirport(uint wpt_index, const QString& active_runway);
    int destinationAirportIndex() const;
    const Airport* destinationAirport() const;
    Airport* destinationAirport();
    inline const QString& destinationAirportId() const { return m_ades_id; }

    const double& distanceActiveWptToDestination() const { return m_distance_from_active_wpt_to_destination; }

    //----- waypoint

    inline double trueTrackFromPreviousToActiveWpt() const 
    { 
        MYASSERT(count() > 0);
        MYASSERT(previousWaypoint() != 0);
        MYASSERT(m_active_wpt_index > 0);
        return trueTrackToNextWaypoint(m_active_wpt_index-1);
    }
    
    inline double trueTrackFromActiveToNextWpt() const 
    { 
        MYASSERT(count() > 0);
        MYASSERT(nextWaypoint() != 0);
        MYASSERT((int)m_active_wpt_index < count()-1);
        return trueTrackToNextWaypoint(m_active_wpt_index);
    }

    void switchToNextWaypoint();

    //----- previous waypoint

    //! ATTENTION: the return value will be negative if there is no previous waypoint
    inline int previousWaypointIndex() const { return m_active_wpt_index - 1; }

    inline const Waypoint* previousWaypoint() const
    {
        if ((int)m_active_wpt_index < 1 || count() == 0) return 0;
        MYASSERT((int)m_active_wpt_index <= count());
        return waypoint(m_active_wpt_index-1); 
    }

    inline Waypoint* previousWaypoint()
    {
        if ((int)m_active_wpt_index == 0 || count() == 0) return 0;
        MYASSERT((int)m_active_wpt_index <= count());
        return waypoint(m_active_wpt_index-1); 
    }

    //----- active waypoint

    inline int activeWaypointIndex() const { return m_active_wpt_index; }

    inline const Waypoint* activeWaypoint() const 
    {
        if ((int)m_active_wpt_index >= count() || count() == 0) return 0;
        MYASSERT((int)m_active_wpt_index < count());
        return waypoint(m_active_wpt_index); 
    }

    inline Waypoint* activeWaypoint() 
    {
        if ((int)m_active_wpt_index >= count() || count() == 0) return 0;
        MYASSERT((int)m_active_wpt_index < count());
        return waypoint(m_active_wpt_index); 
    }

    inline const RouteData* activeWaypointRouteData() const
    {
        if ((int)m_active_wpt_index >= count() || count() == 0) return 0;
        return &routeData(m_active_wpt_index);
    }

    //----- next waypoint

    inline const Waypoint* nextWaypoint() const
    {
        if ((int)m_active_wpt_index >= count()-1) return 0;
        return waypoint(m_active_wpt_index+1);
    }

    inline Waypoint* nextWaypoint()
    {
        if ((int)m_active_wpt_index >= count()-1) return 0;
        return waypoint(m_active_wpt_index+1);
    }

    //----- additional fields

    // getter

    const double& hoursActiveWptToDestination() const { return m_hours_from_active_wpt_to_destination; }

    int cruiseFl() const { return m_cruise_fl; }
    int cruiseTemp() const { return m_cruise_temp; }
    const QString& companyRoute() const { return m_company_route; }
    const QString& flightNumber() const { return m_flight_number; }
    int tropoPause() const { return m_tropo_pause; }
    int costIndex() const { return m_cost_index; }
    uint thrustReductionAltitudeFt() const { return m_thrust_reduction_altitude_ft; }
    uint accelerationAltitudeFt() const { return m_acceleration_altitude_ft; }

    // setter

    void setHoursActiveWptToDestination(const double& hours_from_active_wpt_to_destination)
    { m_hours_from_active_wpt_to_destination = hours_from_active_wpt_to_destination; }

    void setCruiseFl(int cruise_fl) 
    { 
        m_cruise_fl = cruise_fl; 
        emit signalChanged(m_flag, true, "FlightRoute:setCruiseFl");
    }

    void setCruiseTemp(int cruise_temp) 
    {
        m_cruise_temp = cruise_temp; 
        emit signalChanged(m_flag, true, "FlightRoute:setCruiseTemp");
    }

    void setCompanyRoute(const QString& company_route) 
    {
        m_company_route = company_route; 
        emit signalChanged(m_flag, true, "FlightRoute:setCompanyRoute");
    }

    void setFlightNumber(const QString& flight_number) 
    {
        m_flight_number = flight_number; 
        emit signalChanged(m_flag, true, "FlightRoute:setFlightNumber");
    }
    void setCostIndex(int cost_index) 
    {
        m_cost_index = cost_index; 
        emit signalChanged(m_flag, true, "FlightRoute:setCostIndex");
    }

    void setTropoPause(int tropo_pause) 
    {
        m_tropo_pause = tropo_pause; 
        emit signalChanged(m_flag, true, "FlightRoute:setTropoPause");
    }

    void setThrustReductionAltitudeFt(const uint value) 
    {
        m_thrust_reduction_altitude_ft = value; 
        emit signalChanged(m_flag, true, "FlightRoute:setThrustReductionAltitudeFt");
    }

    void setAccelerationAltitudeFt(const uint value) 
    {
        m_acceleration_altitude_ft = value; 
        emit signalChanged(m_flag, true, "FlightRoute:setAccelerationAltitudeFt");
    }

    //----- virtual waypoints

    inline const Waypoint& altReachWpt() const { return m_alt_reach_wpt; }
    inline Waypoint& altReachWpt() { return m_alt_reach_wpt; }
    inline const Waypoint& todWpt() const { return  m_tod_wpt; }
    inline Waypoint& todWpt() { return  m_tod_wpt; }
    
    //----- load/save

    bool loadFP(const QString& filename, const Navdata* navdata);
    bool saveFP(const QString& filename) const;

    //----- extract from ICAO conform input

    //! Awaits a route in ICAO format (like LOWG GRZ P978 VIW RTT LOWI).
    //! The first point of the route should be the departure airport, the last
    //! point should be the arrival airport.
    //! Clears the route and inserts the given one.
    //! Returns true on success, false otherwise, "error" will return an error
    //! text when appropriate.
    bool extractICAORoute(const QString& route, const Navdata& navdata, 
                          const QRegExp& lat_lon_wpt_regexp, QString& error);
    
    //----- 

    //TODO move the view wpt index to the FCM data, this has nothing to do with the route itself

    inline int viewWptIndex() const { return qMin((int)count()-1, (int)m_view_wpt_index); }
    inline const Waypoint* viewWpt() const { return waypoint(viewWptIndex()); }

    inline void setViewWptIndex(int index) 
    {
        int new_index = qMin(qMax(index, -1), (int)count()-1);
        if (new_index == m_view_wpt_index) return;
        m_view_wpt_index = new_index;
        emit signalChanged(m_flag, true, QString("FlightRoute:setViewWptIndex: index=%1").arg(m_view_wpt_index));
    }

signals:

    void signalDepartureAirportChanged();
    void signalDestinationAirportChanged();

protected:

    void setAsDepartureAirportInternal(uint wpt_index, const QString& active_runway);
    void setAsDestinationAirportInternal(uint wpt_index, const QString& active_runway);

    void resetCacheFields() const
    {
        m_adep_wpt_index = -2;
        m_ades_wpt_index = -2;
    }

    void calcDistanceActiveWptToDestination();
        
protected slots:

    void slotChanged(const QString&, bool, const QString&);

protected:

    const ProjectionBase* m_projection;
    
    mutable int m_adep_wpt_index;
    QString m_adep_id;

    QString m_sid_id;
    QString m_sid_transition_id;
    QString m_star_id;
    QString m_app_transition_id;
    QString m_approach_id;

    mutable int m_ades_wpt_index;
    QString m_ades_id;

    //-----

    //TODO move those into the route with according waypoint flags!
    Waypoint m_alt_reach_wpt;
    Waypoint m_tod_wpt;

    //-----

    //! index to the currently active route waypoint
    uint m_active_wpt_index;
   
    //----- additional fields

    double m_distance_from_active_wpt_to_destination;
    double m_hours_from_active_wpt_to_destination;

    int m_cruise_fl;
    int m_cruise_temp;
    QString m_company_route;
    QString m_flight_number;
    int m_cost_index;
    int m_tropo_pause;
    //! to be used when the actual waypoint to view differs from the active waypoint
    int m_view_wpt_index;

    uint m_thrust_reduction_altitude_ft;
    uint m_acceleration_altitude_ft;
};

#endif /* FLIGHTROUTE_H */

// End of file

