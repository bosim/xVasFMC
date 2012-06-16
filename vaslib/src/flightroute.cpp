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

/*! \file    flightroute.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include <QFile>
#include <QRegExp>
#include <QApplication>

#include "assert.h"
#include "airport.h"
#include "sid.h"
#include "star.h"
#include "navdata.h"
#include "projection.h"
#include "flightstatus.h"
#include "declination.h"

#include "flightroute.h"

/////////////////////////////////////////////////////////////////////////////

#define FIELD_SEPARATOR "|"
#define ITEM_SEPARATOR ";"
#define OVERFLY_DESIGNATOR "*"

#define TAG_CRUISE_FL "cfl"
#define TAG_CRUISE_TEMP "ctemp"
#define TAG_COMPANY_ROUTE "croute"
#define TAG_FLIGHT_NR "fnr"
#define TAG_COST_INDEX "cindex"
#define TAG_TROPOPAUSE "tropo"
#define TAG_ACT_WPT_INDEX "actwpt"
#define TAG_ADEP "adep"
#define TAG_ADES "ades"
#define TAG_ACCEL_ALT "accel_alt"
#define TAG_THRUST_RED_ALT "thrust_red_alt"

/////////////////////////////////////////////////////////////////////////////

FlightRoute::FlightRoute(const FlightStatus* flightstatus, const QString& id) :
    Route(id, flightstatus), m_projection(0),
    m_adep_wpt_index(-2), m_ades_wpt_index(-2), m_active_wpt_index(0),
    m_distance_from_active_wpt_to_destination(0.0), m_hours_from_active_wpt_to_destination(0.0),
    m_cruise_fl(-1), m_cruise_temp(-1000), m_cost_index(-1), m_tropo_pause(-1), m_view_wpt_index(0),
    m_thrust_reduction_altitude_ft(0), m_acceleration_altitude_ft(0)
{
    MYASSERT(m_flightstatus != 0);
    MYASSERT(connect(this, SIGNAL(signalChanged(const QString&, bool, const QString&)),
                     this, SLOT(slotChanged(const QString&, bool, const QString&))));

}

/////////////////////////////////////////////////////////////////////////////

FlightRoute::~FlightRoute()
{
}

/////////////////////////////////////////////////////////////////////////////

const FlightRoute& FlightRoute::operator=(const FlightRoute& other)
{
    if (&other == this) return *this;
    
    if (other.m_projection != 0) m_projection = other.m_projection;

    m_adep_wpt_index = other.m_adep_wpt_index;
    m_adep_id = other.m_adep_id;
    m_sid_id = other.m_sid_id;
    m_sid_transition_id = other.m_sid_transition_id;
    m_star_id = other.m_star_id;
    m_app_transition_id = other.m_app_transition_id;
    m_approach_id = other.m_approach_id;
    m_ades_wpt_index = other.m_ades_wpt_index;
    m_ades_id = other.m_ades_id;
    m_active_wpt_index = other.m_active_wpt_index;
    m_alt_reach_wpt = other.m_alt_reach_wpt;
    m_tod_wpt = other.m_tod_wpt;
    m_active_wpt_index = other.m_active_wpt_index;
    m_view_wpt_index = other.m_view_wpt_index;
    m_cruise_fl = other.m_cruise_fl;
    m_cruise_temp = other.m_cruise_temp;
    m_company_route = other.m_company_route;
    m_flight_number = other.m_flight_number;
    m_cost_index = other.m_cost_index;
    m_tropo_pause = other.m_tropo_pause;
    m_acceleration_altitude_ft = other.m_acceleration_altitude_ft;
    m_thrust_reduction_altitude_ft = other.m_thrust_reduction_altitude_ft;
    m_distance_from_active_wpt_to_destination = other.m_distance_from_active_wpt_to_destination;
    m_hours_from_active_wpt_to_destination = other.m_hours_from_active_wpt_to_destination;
    
    Route::operator=(other);
    return *this;
}

/////////////////////////////////////////////////////////////////////////////

void FlightRoute::slotChanged(const QString&, bool, const QString&)
{
    resetCacheFields();
}

/////////////////////////////////////////////////////////////////////////////

void FlightRoute::setProjection(const ProjectionBase* projection) 
{
    MYASSERT(projection != 0);
    m_projection = projection; 
}

/////////////////////////////////////////////////////////////////////////////

void FlightRoute::clear()
{
    resetCacheFields();
    m_adep_id = m_sid_id = m_sid_transition_id = m_star_id = m_app_transition_id = QString::null;
    m_approach_id = m_ades_id = QString::null;
    m_active_wpt_index = 0;
    m_cruise_fl = -1;
    m_cruise_temp = -1000;
    m_company_route = m_flight_number = QString::null;
    m_cost_index = m_tropo_pause = -1;
    m_thrust_reduction_altitude_ft = 0;
    m_acceleration_altitude_ft = 0;
    Route::clear();
    calcDistanceActiveWptToDestination();
}

/////////////////////////////////////////////////////////////////////////////

void FlightRoute::setAsDepartureAirportInternal(uint wpt_index, const QString& active_runway)
{
    Logger::log("FlightRoute:setAsDepartureAirportInternal");

    m_adep_id.clear();

    // set new ADEP
    Waypoint* wpt = waypoint(wpt_index);
    if (wpt != 0 && wpt->asAirport() != 0) 
    {
        wpt->setFlag(Waypoint::FLAG_ADEP);
        wpt->asAirport()->setActiveRunwayId(active_runway);
        m_adep_id = wpt->id();

        while(waypoint(++wpt_index) != 0 && waypoint(wpt_index)->isDependendWaypoint())
        {
            waypoint(wpt_index)->resetIfDependendWaypoint();
            checkAndSetSpecialWaypoint(wpt_index, false);
        }

        m_acceleration_altitude_ft = wpt->asAirport()->elevationFt() + 1000;
        m_thrust_reduction_altitude_ft = wpt->asAirport()->elevationFt() + 1000;
    }

    resetCacheFields();
    emit signalDepartureAirportChanged();
    emit signalChanged(m_flag, true, "FlightRoute:setAsDepartureAirportInternal");
}

/////////////////////////////////////////////////////////////////////////////

void FlightRoute::setAsDepartureAirport(uint wpt_index, const QString& active_runway)
{
    Logger::log("FlightRoute:setAsDepartureAirport");

    // clear existing ADEP, SID and SID_TRANS flags
    WaypointPtrListIterator iter(waypointList());
    while(iter.hasNext())
    {
        Waypoint* wpt = iter.next();
        if (wpt->flag() == Waypoint::FLAG_ADEP) wpt->setFlag(QString::null);
    }

    m_adep_id.clear();

    // set new ADEP
    Waypoint* wpt = waypoint(wpt_index);
    if (wpt != 0 && wpt->asAirport() != 0) 
    {
        wpt->setFlag(Waypoint::FLAG_ADEP);
        wpt->asAirport()->setActiveRunwayId(active_runway);
        m_adep_id = wpt->id();

        int reset_index = wpt_index;
        while(waypoint(++reset_index) != 0 && waypoint(reset_index)->isDependendWaypoint())
        {
            waypoint(reset_index)->resetIfDependendWaypoint();
            checkAndSetSpecialWaypoint(reset_index, false);
        }

        m_acceleration_altitude_ft = wpt->asAirport()->elevationFt() + 1000;
        m_thrust_reduction_altitude_ft = wpt->asAirport()->elevationFt() + 1000;
    }

    // remove all SID flagged waypoints from the route
    for(int index = 0; index < count();)
    {
        if (waypoint(index)->flag() == Waypoint::FLAG_SID ||
            waypoint(index)->flag() == Waypoint::FLAG_SID_TRANS)
        {
            removeWaypoint(index);
        }
        else
        {
            ++index;
        }
    }

    resetCacheFields();
    emit signalDepartureAirportChanged();
    emit signalChanged(m_flag, true, "FlightRoute:setAsDepartureAirport");
}

/////////////////////////////////////////////////////////////////////////////

int FlightRoute::departureAirportIndex() const
{
    if (m_adep_wpt_index >= -1) return m_adep_wpt_index;

    m_adep_wpt_index = -2;
    bool found = false;
    int adep_wpt_index = -1;
    WaypointPtrListIterator iter(waypointList());
    while(iter.hasNext())
    {
        Waypoint* wpt = iter.next();
        ++adep_wpt_index;
        if (wpt->asAirport() != 0 && wpt->flag() == Waypoint::FLAG_ADEP) 
        {
            found = true;
            break;
        }
    }

    if (found) m_adep_wpt_index = adep_wpt_index;
    return m_adep_wpt_index;
}

/////////////////////////////////////////////////////////////////////////////

const Airport* FlightRoute::departureAirport() const 
{
    const Waypoint* wpt = waypoint(departureAirportIndex());
    if (wpt != 0) return wpt->asAirport();
    return 0;
}

/////////////////////////////////////////////////////////////////////////////

Airport* FlightRoute::departureAirport() 
{
    Waypoint* wpt = waypoint(departureAirportIndex());
    if (wpt != 0) return wpt->asAirport();
    return 0;
}

/////////////////////////////////////////////////////////////////////////////

bool FlightRoute::setSid(const Sid& sid, const QString& runway)
{
    Logger::log(QString("FlightRoute:setSid: sid=%1 rwy=%2").arg(sid.id()).arg(runway));

    MYASSERT(!runway.isEmpty());
    
    if (departureAirport() == 0)
    {
        Logger::log("FlightRoute:setSid: no ADEP set");
        return false;
    }

    // remove all SID and SID_TRANS flagged waypoints

    int index = 0;
    
    for(index = 0; index < count();)
    {
        if (waypoint(index)->flag() == Waypoint::FLAG_SID ||
            waypoint(index)->flag() == Waypoint::FLAG_SID_TRANS)
        {
            removeWaypoint(index);
        }
        else
        {
            ++index;
        }
    }

    // search for the last waypoint of the SID in the FP, when found clear wpts before that point

    if (sid.count() > 0)
    {
        int remove_index = -1;
        
        for(index = departureAirportIndex()+1; index < count(); ++index)
            if (*waypoint(index) == *sid.lastWaypoint()) remove_index = index;
        
        for(index = remove_index; index >= departureAirportIndex()+1; --index)
        {
            Logger::log(QString("FlightRoute:setSid: removing wpt %1").arg(waypoint(index)->id()));
            removeWaypoint(index);
        }
    }

    // process the SID

    Airport* adep = departureAirport();
    MYASSERT(adep != 0);
    m_sid_id.clear();
    m_sid_transition_id.clear();

    // insert the SID waypoints to the route

    int insert_index = departureAirportIndex()+1;
    WaypointPtrListIterator iter(sid.waypointList());
    while(iter.hasNext())
    {
        Waypoint* wpt = iter.next();
        if (adep != 0 && wpt->id() == adep->id()) continue;
        insertWaypoint(*wpt, insert_index++);
    }

    // set the active runway to the departure airport
    MYASSERT(adep->runway(runway).isValid());
    adep->setActiveRunwayId(runway);

    int reset_index = departureAirportIndex();
    while(waypoint(++reset_index) != 0 && waypoint(reset_index)->isDependendWaypoint())
    {
        waypoint(reset_index)->resetIfDependendWaypoint();
        checkAndSetSpecialWaypoint(reset_index, false);
    }
    
    // insert the runway as a waypoint
    Waypoint* rwy_wpt = waypoint(departureAirportIndex() + 1);
    if (rwy_wpt != 0 && rwy_wpt->lat() != adep->runway(runway).lat() && rwy_wpt->lon() != adep->runway(runway).lon())
    {
        insertWaypoint(adep->runway(runway), departureAirportIndex()+1);
        waypoint(departureAirportIndex()+1)->setFlag(Waypoint::FLAG_SID);
    }

    m_sid_id = sid.id();

    removeDoubleWaypoints();

    Logger::log(QString("FlightRoute:setSid: ADEP=%1 actRwy=%2/%3").arg(adep->id()).arg(adep->activeRunwayId()).
                arg(adep->activeRunway().toString()));

    emit signalChanged(m_flag, true, "FlightRoute:setSid");
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool FlightRoute::setSidTransition(const Transition& sid_transition)
{
    Logger::log(QString("FlightRoute:setSidTransition: sid_transition=%1").arg(sid_transition.id()));

    if (departureAirport() == 0)
    {
        Logger::log("FlightRoute:setSidTransition: no ADEP set");
        return false;
    }

    // remove all SID_TRANSITION flagged waypoints from the route
    
    int index = 0;

    for(index = 0; index < count();)
    {
        if (waypoint(index)->flag() == Waypoint::FLAG_SID_TRANS)
        {
            removeWaypoint(index);
        }
        else
        {
            ++index;
        }
    }

    m_sid_transition_id.clear();

    // find waypoint after SID

    int insert_index = departureAirportIndex()+1;
    while(insert_index < count() && (waypoint(insert_index)->flag() == Waypoint::FLAG_SID)) ++insert_index;

    // search for the last waypoint of the SID_TRANSITION in the FP, when found clear wpts before that point

    if (sid_transition.count() > 0)
    {
        int remove_index = -1;
        
        for(index = insert_index; index < count(); ++index)
            if (*waypoint(index) == *sid_transition.lastWaypoint()) remove_index = index;
        
        for(index = remove_index; index >= insert_index; --index)
        {
            Logger::log(QString("FlightRoute:setSidTransition: removing wpt %1").arg(waypoint(index)->id()));
            removeWaypoint(index);
        }
    }

    // insert the SID_TRANSITION waypoints to the route

    WaypointPtrListIterator iter(sid_transition.waypointList());
    while(iter.hasNext())
    {
        Waypoint* wpt = iter.next();
        insertWaypoint(*wpt, insert_index++);
    }

    m_sid_transition_id = sid_transition.id();

    removeDoubleWaypoints();

    emit signalChanged(m_flag, true, "FlightRoute:setSidTransition");
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool FlightRoute::setStar(const Star& star, const QString& runway)
{
    Logger::log(QString("FlightRoute:setStar: star=%1 rwy=%2").arg(star.id()).arg(runway));
    
    MYASSERT(!runway.isEmpty());

    if (destinationAirport() == 0)
    {
        Logger::log("FlightRoute:setStar: no ADEP set");
        return false;
    }

    // remove all STAR flagged waypoints from the end of the route
    
    int index = 0;

    for(index = 0; index < count();)
    {
        if (waypoint(index)->flag() == Waypoint::FLAG_STAR ||
            waypoint(index)->flag() == Waypoint::FLAG_APP_TRANS)
        {
            removeWaypoint(index);
        }
        else
        {
            ++index;
        }
    }

    // search for the first waypoint of the STAR in the FP, when found clear wpts after that point

    if (star.count() > 0)
    {
        int max_index = destinationAirportIndex();
        if (max_index < 0) max_index = count();

        int remove_index = max_index;
       
        for(index = activeWaypointIndex()+1; index < max_index; ++index)
        {
            if (*waypoint(index) == *star.firstWaypoint()) 
            {
                remove_index = index;
                break;
            }
        }
        
        for(index = max_index-1; index >= remove_index; --index)
        {
            Logger::log(QString("FlightRoute:setStar: removing wpt %1").arg(waypoint(index)->id()));
            removeWaypoint(index);
        }
    }

    // process STAR
        
    Airport* ades = destinationAirport();
    MYASSERT(ades != 0);
    m_star_id.clear();
    m_app_transition_id.clear();
    MYASSERT(m_app_transition_id.isEmpty());
    m_approach_id.clear();

    // insert the STAR waypoints to the route

    int insert_index = destinationAirportIndex();
    WaypointPtrListIterator iter(star.waypointList());
    while(iter.hasNext())
    {
        Waypoint* wpt = iter.next();
        if (wpt->id() == ades->id()) continue;
        insertWaypoint(*wpt, insert_index);
        ++insert_index;
    }

    // set the active runway to the destination airport
    MYASSERT(ades->runway(runway).isValid());
    ades->setActiveRunwayId(runway);
    m_star_id = star.id();

    int reset_index = destinationAirportIndex();
    while(waypoint(++reset_index) != 0 && waypoint(reset_index)->isDependendWaypoint())
    {
        waypoint(reset_index)->resetIfDependendWaypoint();
        checkAndSetSpecialWaypoint(reset_index, false);
    }

    removeDoubleWaypoints();

    Logger::log(QString("FlightRoute:setStar: ADES=%1 actRwy=%2/%3").
                arg(ades->id()).arg(ades->activeRunwayId()).arg(ades->activeRunway().toString()));

    MYASSERT(m_app_transition_id.isEmpty());

    emit signalChanged(m_flag, true, "FlightRoute:setStar");

    MYASSERT(m_app_transition_id.isEmpty());
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool FlightRoute::setAppTransition(const Transition& app_transition, const QString& runway)
{
    Logger::log(QString("FlightRoute:setAppTransition: app_transition=%1 rwy=%2").
                arg(app_transition.id()).arg(runway));
    
    MYASSERT(!runway.isEmpty());

    if (destinationAirport() == 0)
    {
        Logger::log("FlightRoute:setAppTransition: no ADEP set");
        return false;
    }

    if (m_star_id.isEmpty())
    {
        Logger::log("FlightRoute:setAppTransition: no STAR set");
        return false;
    }

    if (!m_app_transition_id.isEmpty())
    {
        Logger::log(QString("FlightRoute:setAppTransition: APP_TRANS already set (%1), call setStar() before").
                    arg(m_app_transition_id));
        return false;
    }

    // remove all APP_TRANSITION and approach flagged waypoints from the end of the route
    
    for(int index = 0; index < count();)
    {
        if (waypoint(index)->flag() == Waypoint::FLAG_APP_TRANS)
        {
            removeWaypoint(index);
        }
        else
        {
            ++index;
        }
    }
        
    Airport* ades = destinationAirport();
    MYASSERT(ades != 0);
    m_app_transition_id.clear();
    m_approach_id.clear();

    // insert the APP_TRANSITION waypoints to the route

    int insert_index = destinationAirportIndex();
    
    if (app_transition.count() > 0)
    {
        for(int index = destinationAirportIndex()-1; index > 0; --index)
        {
            if (*waypoint(index) == *app_transition.waypoint(0))
            {
                insert_index = index;
                break;
            }
        }

        int index = insert_index;
        while(index < destinationAirportIndex())
        {
            if (!waypoint(index)->isApproach())removeWaypoint(index);
            else ++index;
        }
    }

    WaypointPtrListIterator iter(app_transition.waypointList());
    while(iter.hasNext())
    {
        Waypoint* wpt = iter.next();
        if (wpt->id() == ades->id()) continue;
        insertWaypoint(*wpt, insert_index);
        ++insert_index;
    }
    
    // set the active runway to the destination airport
    MYASSERT(ades->runway(runway).isValid());
    ades->setActiveRunwayId(runway);
    m_app_transition_id = app_transition.id();

    int reset_index = destinationAirportIndex();
    while(waypoint(++reset_index) != 0 && waypoint(reset_index)->isDependendWaypoint())
    {
        waypoint(reset_index)->resetIfDependendWaypoint();
        checkAndSetSpecialWaypoint(reset_index, false);
    }

    removeDoubleWaypoints();

    Logger::log(QString("FlightRoute:setAppTransition: ADES=%1 actRwy=%2/%3").
                arg(ades->id()).arg(ades->activeRunwayId()).arg(ades->activeRunway().toString()));

    emit signalChanged(m_flag, true, "FlightRoute:setAppTransition");
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool FlightRoute::setApproach(const Approach& approach, const QString& runway)
{
    Logger::log(QString("FlightRoute:setApproach: approach=%1 rwy=%2").
                arg(approach.id()).arg(runway));
    
    MYASSERT(!runway.isEmpty());

    if (destinationAirport() == 0)
    {
        Logger::log("FlightRoute:setApproach: no ADEP set");
        return false;
    }

    // remove all APPROACH flagged waypoints from the route

    int index = 0;
    while(index < count())
    {
        if (waypoint(index)->flag() == Waypoint::FLAG_APPROACH)
        {
            removeWaypoint(index);
        }
        else
        {
            ++index;
        }
    }

    // remove all waypoints after the destination airport

    index = destinationAirportIndex() + 1;
    for(;index < count(); ++index)
    {
        removeWaypoint(index);
        --index;
    }

    Airport* ades = destinationAirport();
    MYASSERT(ades != 0);
    m_approach_id.clear();

    // insert the APPROACH waypoints to the route

    bool after_ades = false;
    int insert_index = destinationAirportIndex();
    WaypointPtrListIterator iter(approach.waypointList());
    while(iter.hasNext())
    {
        Waypoint* wpt = iter.next();
        if (wpt->id() == ades->id()) continue;
        if (wpt->asRunway() != 0)
        {
            after_ades = true;
            ++insert_index;
            continue;
        }

        if (after_ades) wpt->setParent(QString::null);
        insertWaypoint(*wpt, insert_index);
        ++insert_index;
    }

    // set the active runway to the destination airport
    MYASSERT(ades->runway(runway).isValid());
    ades->setActiveRunwayId(runway);
    m_approach_id = approach.id();

    int reset_index = destinationAirportIndex();
    while(waypoint(++reset_index) != 0 && waypoint(reset_index)->isDependendWaypoint())
    {
        waypoint(reset_index)->resetIfDependendWaypoint();
        checkAndSetSpecialWaypoint(reset_index, false);
    }

    removeDoubleWaypoints();

    Logger::log(QString("FlightRoute:setApproach: ADES=%1 actRwy=%2/%3").
                arg(ades->id()).arg(ades->activeRunwayId()).arg(ades->activeRunway().toString()));

    emit signalChanged(m_flag, true, "FlightRoute:setApproach");
    return true;
}

/////////////////////////////////////////////////////////////////////////////

void FlightRoute::setAsDestinationAirportInternal(uint wpt_index, const QString& active_runway)
{
    Logger::log("FlightRoute:setAsDestinationAirportInternal");

    m_ades_id.clear();

    Waypoint* wpt = waypoint(wpt_index);
    if (wpt != 0 && wpt->asAirport() != 0) 
    {
        wpt->setFlag(Waypoint::FLAG_ADES);
        wpt->asAirport()->setActiveRunwayId(active_runway);
        m_ades_id = wpt->id();

        while(waypoint(++wpt_index) != 0 && waypoint(wpt_index)->isDependendWaypoint())
        {
            waypoint(wpt_index)->resetIfDependendWaypoint();
            checkAndSetSpecialWaypoint(wpt_index, false);
        }
    }

    resetCacheFields();
    calcDistanceActiveWptToDestination();
    emit signalDestinationAirportChanged();
    emit signalChanged(m_flag, true, "FlightRoute:setAsDestinationAirportInternal");
}

/////////////////////////////////////////////////////////////////////////////

void FlightRoute::setAsDestinationAirport(uint wpt_index, const QString& active_runway)
{
    Logger::log("FlightRoute:setAsDestinationAirport");

    // clear existing ADES flags
    WaypointPtrListIterator iter(waypointList());
    while(iter.hasNext())
    {
        Waypoint* wpt = iter.next();
        if (wpt->flag() == Waypoint::FLAG_ADES) wpt->setFlag(QString::null);
    }

    m_ades_id.clear();

    Waypoint* wpt = waypoint(wpt_index);
    if (wpt != 0 && wpt->asAirport() != 0) 
    {
        wpt->setFlag(Waypoint::FLAG_ADES);
        wpt->asAirport()->setActiveRunwayId(active_runway);
        m_ades_id = wpt->id();
    }

    int reset_index = wpt_index;
    while(waypoint(++reset_index) != 0 && waypoint(reset_index)->isDependendWaypoint())
    {
        waypoint(reset_index)->resetIfDependendWaypoint();
        checkAndSetSpecialWaypoint(reset_index, false);
    }

    // remove all STAR flagged waypoints from the route
    for(int index = 0; index < count();)
    {
        if (waypoint(index)->flag() == Waypoint::FLAG_STAR ||
            waypoint(index)->flag() == Waypoint::FLAG_APP_TRANS ||
            waypoint(index)->flag() == Waypoint::FLAG_APPROACH)
        {
            removeWaypoint(index);
        }
        else
        {
            ++index;
        }
    }

    resetCacheFields();
    calcDistanceActiveWptToDestination();
    emit signalDestinationAirportChanged();
    emit signalChanged(m_flag, true, "FlightRoute:setAsDestinationAirport");
}

/////////////////////////////////////////////////////////////////////////////

int FlightRoute::destinationAirportIndex() const
{
    if (m_ades_wpt_index >= -1) return m_ades_wpt_index;

    m_ades_wpt_index = -2;
    bool found = false;
    int ades_wpt_index = count();
    WaypointPtrListIterator iter(waypointList());
    iter.toBack();
    while(iter.hasPrevious())
    {
        const Waypoint* wpt = iter.previous();
        --ades_wpt_index;
        if (wpt->asAirport() != 0 && wpt->flag() == Waypoint::FLAG_ADES) 
        {
            found = true;
            break;
        }
    }
    
    if (found) m_ades_wpt_index = ades_wpt_index;
    return m_ades_wpt_index;
}

/////////////////////////////////////////////////////////////////////////////

const Airport* FlightRoute::destinationAirport() const 
{
    const Waypoint* wpt = waypoint(destinationAirportIndex());
    if (wpt != 0) return wpt->asAirport();
    return 0;
}

/////////////////////////////////////////////////////////////////////////////

Airport* FlightRoute::destinationAirport() 
{
    Waypoint* wpt = waypoint(destinationAirportIndex());
    if (wpt != 0) return wpt->asAirport();
    return 0;
}

/////////////////////////////////////////////////////////////////////////////

void FlightRoute::switchToNextWaypoint()
{
    if ((int)m_active_wpt_index >= count()) return;
    
    Waypoint* wpt = activeWaypoint();
    MYASSERT(wpt != 0);
    Logger::log(QString("FlightRoute:switchToNextWaypoint: @%1").arg(wpt->id()));

    // clear a potential holding
    wpt->setHolding(Holding());

    // set overflown values
    routeData(activeWaypointIndex()).m_time_over_waypoint = m_flightstatus->fs_utc_time;
    wpt->overflownData().setAltitudeFt((int)m_flightstatus->smoothed_altimeter_readout.lastValue());
    wpt->overflownData().setSpeedKts((int)m_flightstatus->smoothedIAS());

    // reset departure airport

    

    // reset destination airport

    // go direct to the next waypoint if we passed a hdg2alt waypoint
    if (wpt->asWaypointHdgToAlt() != 0)
    {
        Logger::log("FlightRoute:switchToNextWaypoint: we passed a hdg2alt waypoint -> DCT");
        wpt->restrictions().setOverflyRestriction(true);
    }

    Waypoint prev_wpt_copy = *wpt;
    ++m_active_wpt_index;
    
    // remove waypoint before previous one

    bool emit_change = true;
    while(m_active_wpt_index > 1) 
    {
        removeWaypoint(0);
        emit_change = false;
    }

//     Logger::log(QString("prev=%1/%2/%3, act=%4/%5/%6, next=%7/%8/%9, copy=%10/%11/%12").
//                 arg(previousWaypoint()->id()).arg(previousWaypoint()->lat()).arg(previousWaypoint()->lon()).
//                 arg(activeWaypoint()->id()).arg(activeWaypoint()->lat()).arg(activeWaypoint()->lon()).
//                 arg(nextWaypoint()->id()).arg(nextWaypoint()->lat()).arg(nextWaypoint()->lon()).
//                 arg(prev_wpt_copy.id()).arg(prev_wpt_copy.lat()).arg(prev_wpt_copy.lon()));
                
    // jump over double waypoints
    while(activeWaypoint() != 0 &&
          activeWaypoint()->id() == prev_wpt_copy.id() &&
          qAbs(activeWaypoint()->lat() - prev_wpt_copy.lat()) < Waypoint::LAT_LON_COMPARE_EPSILON &&
          qAbs(activeWaypoint()->lon() - prev_wpt_copy.lon()) < Waypoint::LAT_LON_COMPARE_EPSILON)
    {
        Logger::log(QString("FlightRoute:switchToNextWaypoint: jumping over double waypoint (%1)").
                    arg(activeWaypoint()->id()));
        removeWaypoint(activeWaypointIndex());
    }

    if (activeWaypoint() != 0 && activeWaypoint()->asWaypointHdgToIntercept())
    {
        Logger::log(QString("FlightRoute:switchToNextWaypoint: next wpt (%1) is a hdg2intercept, recalc wpt").
                    arg(activeWaypoint()->id()));

        activeWaypoint()->setLat(0.0);
        activeWaypoint()->setLon(0.0);
        checkAndSetSpecialWaypoint(activeWaypointIndex(), true);
    }

    calcDistanceActiveWptToDestination();
    if (emit_change) emit signalChanged(m_flag, true, "FlightRoute:switchToNextWaypoint");
}

/////////////////////////////////////////////////////////////////////////////

void FlightRoute::appendWaypoint(const Waypoint& wpt, bool remove_double_wtps)
{
    Route::appendWaypoint(wpt);

    Waypoint* inserted_wpt = waypoint(count() - 1);
    MYASSERT(inserted_wpt != 0);
    if (m_projection != 0) m_projection->convertLatLonToXY(*inserted_wpt);

    if (remove_double_wtps) removeDoubleWaypoints();
    resetCacheFields();
    calcDistanceActiveWptToDestination();
}

/////////////////////////////////////////////////////////////////////////////

void FlightRoute::insertWaypoint(const Waypoint& wpt, int pos, bool remove_double_wtps)
{
    Route::insertWaypoint(wpt, pos);

    Waypoint* inserted_wpt = waypoint(pos);
    MYASSERT(inserted_wpt != 0);
    if (m_projection != 0) m_projection->convertLatLonToXY(*inserted_wpt);

    if (remove_double_wtps) removeDoubleWaypoints();
    resetCacheFields();
    calcDistanceActiveWptToDestination();
}

/////////////////////////////////////////////////////////////////////////////

bool FlightRoute::removeWaypoint(int index)
{
    MYASSERT(index >= 0);
    if ((int)index >= count()) return false;

    // remove a T/D waypoint when removeing its successor
    if (waypoint(index-1) != 0 && waypoint(index-1)->flag() == Waypoint::FLAG_DCT) 
    {
        Route::removeWaypoint(index - 1);
        --index;
    }

    //shift the active waypoint index if we remove a waypoint before the active one
    if ((int)m_active_wpt_index > index) m_active_wpt_index = qMax((uint)0, m_active_wpt_index-1);
    Route::removeWaypoint(index);
    resetCacheFields();
    calcDistanceActiveWptToDestination();
    return true;
}

/////////////////////////////////////////////////////////////////////////////

void FlightRoute::goDirect(const uint dct_wpt_index,
                           double turn_radius_nm,
                           QString direct_wpt_name)
{
    Logger::log("FlightRoute:goDirect");

    MYASSERT((int)dct_wpt_index < count());

    bool direct_to_active_waypoint = ((int)dct_wpt_index == activeWaypointIndex());
    const Waypoint* dct_wpt = waypoint(dct_wpt_index);

    if (dct_wpt_index == 0)
    {
        // if we have a holding, invalidate it
        Waypoint* first_wpt = waypoint(0);
        if (first_wpt != 0 &&
            first_wpt->holding().isValid() && 
			first_wpt->holding().status() != Holding::STATUS_INACTIVE)
        {
            first_wpt->setHolding(Holding());
        }
    }
    else
    {
        if (direct_to_active_waypoint &&
            activeWaypoint()->holding().isValid() && 
			activeWaypoint()->holding().status() != Holding::STATUS_INACTIVE)
        {
            activeWaypoint()->setHolding(Holding());
        }

        for(uint count=0; count < dct_wpt_index; ++count) removeWaypoint(0);
    }

	// insert the current position as the last removed waypoint (with the turn radius in mind)

    double turn_dist = Navcalc::getPreTurnDistance(
        0.0, turn_radius_nm, (int)m_flightstatus->ground_speed_kts, 
        m_flightstatus->smoothedTrueHeading(), 
        Navcalc::getTrackBetweenWaypoints(m_flightstatus->current_position_raw, *dct_wpt));

    Waypoint direct_from_position = 
        Navcalc::getPBDWaypoint(
            m_flightstatus->current_position_raw, m_flightstatus->smoothedMagneticHeading(), turn_dist,
            Declination::globalDeclination());

    direct_from_position.setId(direct_wpt_name);
    direct_from_position.setFlag(Waypoint::FLAG_DCT);
    insertWaypoint(direct_from_position, 0);
    if (activeWaypointIndex() == 0 && dct_wpt_index > 0) switchToNextWaypoint();

    // search for waypoints in front of the current position which lie on
    // the track to the direct point and remove them.
    // When doing this, the direction of each leg must be checked for the same
    // direction as the leg from the current position to the direct waypoint,
    // plus the leg from the waypoint to remove to the direct waypoint must
    // point into the same direction as the leg from the current position to
    // the direct waypoint.

//     Logger::log(QString("FlightRoute:goDirect: dct2active_wpt=%1  act_wpt_index=%2").
//                 arg(direct_to_active_waypoint).arg(activeWaypointIndex()));

    if (direct_to_active_waypoint && count() >= 3 && activeWaypointIndex() > 0)
    {
//        Logger::log(QString("FlightRoute:goDirect: direct to active wpt detected - checking for WPTs to remove"));

        double direct_track = trueTrackFromPreviousToActiveWpt();
        const int wpt_index_to_remove = dct_wpt_index+1;

        while(wpt_index_to_remove < count()-1)
        {
            double leg_to_direct_track_diff = 
                Navcalc::getAbsHeadingDiff(direct_track, trueTrackToNextWaypoint(wpt_index_to_remove));

//             Logger::log(QString("--%1---").arg(waypoint(wpt_index_to_remove)->id()));

//             Logger::log(QString("FlightRoute:goDirect: direct_track=%1 track_to_next=%2 (%3) diff=%4").
//                         arg(direct_track).
//                         arg(trueTrackToNextWaypoint(wpt_index_to_remove)).
//                         arg(waypoint(wpt_index_to_remove)->id()).
//                         arg(leg_to_direct_track_diff));
            
            double wpt_to_dct_wpt_track_diff = 
                Navcalc::getAbsHeadingDiff(
                    direct_track, Navcalc::getTrackBetweenWaypoints(*waypoint(wpt_index_to_remove), *dct_wpt));

//             Logger::log(QString("FlightRoute:goDirect: track_between=%1 (%2) diff=%3").
//                         arg(Navcalc::getTrackBetweenWaypoints(*waypoint(wpt_index_to_remove), *dct_wpt)).
//                         arg(waypoint(wpt_index_to_remove)->id()).
//                         arg(wpt_to_dct_wpt_track_diff));

            if (leg_to_direct_track_diff >= 90.0 || wpt_to_dct_wpt_track_diff >= 90.0) break;

            // remove the waypoint

//            Logger::log(QString("removing %1").arg(waypoint(wpt_index_to_remove)->id()));
            removeWaypoint(wpt_index_to_remove);
        }
    }

    emit signalChanged(m_flag, true, "FlightRoute:goDirect");
}

/////////////////////////////////////////////////////////////////////////////

bool FlightRoute::loadFP(const QString& filename, const Navdata* navdata)
{
    if (filename.isEmpty()) return false;
    clear();

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) 
    {
        Logger::log(QString("FlightRoute:loadFP: could not open file (%1)").arg(filename));
        return false;
    }

    QTextStream stream( &file );

    QStringList active_runway_list;
    bool loaded_adep_flag = false;
    bool loaded_ades_flag = false;

    unsigned int line_count = 0;
    while (!stream.atEnd() ) 
    {
        ++line_count;
        QString line(stream.readLine());
		line = line.trimmed();
        QStringList items = line.split(FIELD_SEPARATOR, QString::KeepEmptyParts);
        if (items.count() < 3)
        {
            Logger::log(QString("FlightRoute:loadFP: could not read line %1").arg(line_count));
            return false;
        }

        bool is_overfly_wpt = false;

        while (items[0].startsWith(OVERFLY_DESIGNATOR))
        {
            is_overfly_wpt = true;
            items[0] = items[0].mid(1);
        }

        while (items[0].endsWith(OVERFLY_DESIGNATOR))
        {
            is_overfly_wpt = true;
            items[0] = items[0].left(items[0].length()-1);
        }

        Waypoint* new_waypoint = 0;

        if (line_count > 1 && items.count() >= 8)
        {
            if (items[7] == Waypoint::TYPE_HDG_TO_ALT && items.count() >= 9)
            {
                new_waypoint = new WaypointHdgToAlt(items[0], items[8].toInt());
            }
            else if (items[7] == Waypoint::TYPE_HDG_TO_INTERCEPT && items.count() >= 13)
            {
                new_waypoint = new WaypointHdgToIntercept(
                    items[0], 0, 0, 
                    Waypoint("fix2icept", "", items[8].toDouble(), items[9].toDouble()),
                    items[10].toInt(),
                    items[11].toInt(),
                    (Navcalc::TURN_DIRECTION)items[12].toUInt());
            }
        }
            
        if (new_waypoint == 0)
            new_waypoint = new Waypoint(items[0], QString::null, items[1].toDouble(), items[2].toDouble());

        MYASSERT(new_waypoint != 0);
        new_waypoint->restrictions().setOverflyRestriction(is_overfly_wpt);

        if (items.count() >= 4)
            new_waypoint->restrictions().setSpeedAndAltitudeRestrictionFromText(items[3]);

        if (items.count() >= 5) new_waypoint->setParent(items[4]);

        if (items.count() >= 6) 
        {
            new_waypoint->setFlag(items[5]);
            if (items[5] == Waypoint::FLAG_ADEP) loaded_adep_flag = true;
            if (items[5] == Waypoint::FLAG_ADES) loaded_ades_flag = true;
        }

        if (items.count() >= 7) active_runway_list.append(items[6]);
        else                    active_runway_list.append("");

        appendWaypoint(*new_waypoint);
        delete new_waypoint;
        new_waypoint = 0;

        if (line_count == 1)
        {
            QStringList extended_item_list = items.last().split(ITEM_SEPARATOR);
            for(int index=0; index < extended_item_list.count(); ++index)
            {
                //Logger::log(QString("FlightRoute:loadFP: tag: (%1)").arg(extended_item_list[index]));

                QString tag = extended_item_list[index].section("=", 0, 0);
                QString value = extended_item_list[index].section("=", 1);

                bool convok = false;
                if (tag == TAG_CRUISE_FL)
                {
                    m_cruise_fl = value.toUInt(&convok);
                    if (!convok) m_cruise_fl = -1;
                }
                else if (tag == TAG_CRUISE_TEMP)
                {
                    m_cruise_temp = value.toInt(&convok);
                    if (!convok) m_cruise_temp = -1000;
                }
                else if (tag == TAG_COMPANY_ROUTE)
                    m_company_route = value;
                else if (tag == TAG_FLIGHT_NR)
                    m_flight_number = value;
                else if (tag == TAG_COST_INDEX)
                {
                    m_cost_index = value.toUInt(&convok);
                    if (!convok) m_cost_index = -1;
                }
                else if (tag == TAG_TROPOPAUSE)
                {
                    m_tropo_pause = value.toUInt(&convok);
                    if (!convok) m_tropo_pause = -1;
                }
                else if (tag == TAG_ACT_WPT_INDEX)
                {
                    m_active_wpt_index = value.toUInt(&convok);
                }
                else if (tag == TAG_ADEP)
                    m_adep_id = value;
                else if (tag == TAG_ADES)
                    m_ades_id = value;
                else if (tag == TAG_ACCEL_ALT)
                {
                    m_acceleration_altitude_ft = value.toUInt(&convok);
                }
                else if (tag == TAG_THRUST_RED_ALT)
                {
                    m_thrust_reduction_altitude_ft = value.toUInt(&convok);
                }
            }
        }
    }

    file.close();

    if (navdata != 0) scanForWaypointInformation(*navdata);

    // set adep/ades

    const Waypoint* ades = 0;

    // ADES
    for(int index=count()-1; index>=0; --index)
    {
        const Waypoint* wpt = waypoint(index);
        if (wpt->asAirport() != 0 && (wpt->isAdes() || !loaded_ades_flag))
        {
            setAsDestinationAirportInternal(index, active_runway_list[index]);
            ades = wpt;
            break;
        }
    }

    // ADEP
    for(int index=0; index<count(); ++index)
    {
        const Waypoint* wpt = waypoint(index);
        if (wpt->asAirport() != 0 && wpt != ades && (wpt->isAdep() || !loaded_adep_flag))
        {
            setAsDepartureAirportInternal(index, active_runway_list[index]);
            break;
        } 
    }

    // SID, SID_TRANS, STAR, APP_TRANS, APPROACH
    for(int index=0; index<count(); ++index)
    {
        const Waypoint* wpt = waypoint(index);

        if (wpt->isSid())
        {
            m_sid_id = wpt->parent();
            //Logger::log(QString("FlightRoute:loadFP: found SID (%1)").arg(m_sid_id));
        }
        else if (wpt->isSidTransition())
        {
            m_sid_transition_id = wpt->parent();
            //Logger::log(QString("FlightRoute:loadFP: found SID_TRANS (%1)").arg(m_sid_transition_id));
        }
        else if (wpt->isStar())
        {
            m_star_id = wpt->parent();
            //Logger::log(QString("FlightRoute:loadFP: found STAR (%1)").arg(m_star_id));
        }
        else if (wpt->isAppTransition())
        {
            m_app_transition_id = wpt->parent();
            //Logger::log(QString("FlightRoute:loadFP: found APP_TRANS (%1)").arg(m_app_transition_id));
        }
        else if (wpt->isApproach())
        {
            m_approach_id = wpt->parent();
            //Logger::log(QString("FlightRoute:loadFP: found APPROACH (%1)").arg(m_approach_id));
        }
    }

    calcDistanceActiveWptToDestination();
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool FlightRoute::saveFP(const QString& filename) const
{
//     Logger::log(QString("FlightRoute:saveFP: %1").arg(filename));
    if (filename.isEmpty()) return false;

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) 
    {
        Logger::log(QString("FlightRoute:saveFP: could not open file (%1)").arg(filename));
        return false;
    }

    QTextStream stream(&file);

    // Per line format: ID|LAT|LON|SPD&ALT RESTRICTION|PARENT|FLAG|ACTIVERWY|OTHER

    for(int index=0; index < m_wpt_list.count(); ++index)
    {
        const Waypoint* waypoint = m_wpt_list.at(index);
		MYASSERT(waypoint != 0);

		QString waypoint_id = waypoint->id();
        if (waypoint->restrictions().hasOverflyRestriction()) waypoint_id += OVERFLY_DESIGNATOR;
        
        stream << waypoint_id << FIELD_SEPARATOR
               << QString::number(waypoint->lat(), 'f', 10) << FIELD_SEPARATOR
               << QString::number(waypoint->lon(), 'f', 10) << FIELD_SEPARATOR
               << waypoint->restrictions().speedAndAltitudeRestrictionText() << FIELD_SEPARATOR
               << waypoint->parent() << FIELD_SEPARATOR
               << waypoint->flag() << FIELD_SEPARATOR;

        if (waypoint->asAirport() != 0)
            stream << waypoint->asAirport()->activeRunwayId();

        stream << FIELD_SEPARATOR << waypoint->type();
        if (waypoint->asWaypointHdgToAlt() != 0)
        {
            stream << FIELD_SEPARATOR << waypoint->asWaypointHdgToAlt()->hdgToHold();
        }
        else if (waypoint->asWaypointHdgToIntercept() != 0)
        {
            stream << FIELD_SEPARATOR 
                   << waypoint->asWaypointHdgToIntercept()->fixToIntercept().lat()
                   << FIELD_SEPARATOR 
                   << waypoint->asWaypointHdgToIntercept()->fixToIntercept().lon()
                   << FIELD_SEPARATOR 
                   << waypoint->asWaypointHdgToIntercept()->radialToIntercept()
                   << FIELD_SEPARATOR 
                   << waypoint->asWaypointHdgToIntercept()->hdgUntilIntercept()
                   << FIELD_SEPARATOR 
                   << waypoint->asWaypointHdgToIntercept()->turnDirection();
        }
        
        if (index == 0)
        {
            stream << FIELD_SEPARATOR
                   << TAG_CRUISE_FL   << "=" << m_cruise_fl   << ITEM_SEPARATOR
                   << TAG_CRUISE_TEMP << "=" << m_cruise_temp << ITEM_SEPARATOR
                   << TAG_COMPANY_ROUTE << "=" << m_company_route << ITEM_SEPARATOR
                   << TAG_FLIGHT_NR << "=" << m_flight_number << ITEM_SEPARATOR
                   << TAG_COST_INDEX << "=" << m_cost_index << ITEM_SEPARATOR
                   << TAG_TROPOPAUSE << "=" << m_tropo_pause << ITEM_SEPARATOR
                   << TAG_ACT_WPT_INDEX << "=" << m_active_wpt_index << ITEM_SEPARATOR
                   << TAG_ADEP << "=" << m_adep_id << ITEM_SEPARATOR
                   << TAG_ADES << "=" << m_ades_id << ITEM_SEPARATOR
                   << TAG_ACCEL_ALT << "=" << m_acceleration_altitude_ft << ITEM_SEPARATOR
                   << TAG_THRUST_RED_ALT << "=" << m_thrust_reduction_altitude_ft;
        }

        stream   << "\n";
    }

    file.close();
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool FlightRoute::setHolding(uint wpt_index, const Holding& holding)
{
    Waypoint* wpt = waypoint(wpt_index);
    if (wpt == 0) return false;
    wpt->setHolding(holding);
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool FlightRoute::setWaypointOverflyRestriction(uint index, bool is_overfly)
{
    if ((int)index >= count()) return false;
    Waypoint* wpt = waypoint(index);
    MYASSERT(wpt != 0);
    wpt->restrictions().setOverflyRestriction(is_overfly);
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool FlightRoute::extractICAORoute(const QString& route, const Navdata& navdata, 
                                   const QRegExp& lat_lon_wpt_regexp, QString& error)
{
    clear();
    error = QString::null;

    if (route.isEmpty()) 
    {
        error = "Route is empty";
        return false;
    }
    
    // decode route
    
    QStringList route_item_list = route.toUpper().trimmed().split(" ", QString::SkipEmptyParts);

    if (route_item_list.count() >= 2 && route_item_list[0] == route_item_list[1]) route_item_list.removeFirst();
    if (route_item_list.count() >= 2 && 
        route_item_list[route_item_list.count()-2] == route_item_list[route_item_list.count()-1]) 
        route_item_list.removeLast();

    if (route_item_list.count() < 2)
    {
        error = "Could not parse route";
        return false;
    }
   
    // set departure airport
            
    WaypointPtrList result_list;
    navdata.getAirports(route_item_list[0].trimmed(), result_list);
    if (result_list.count() == 1)
    {
        insertWaypoint(*result_list.at(0), 0);
        setAsDepartureAirport(0, QString::null);
        MYASSERT(departureAirport() != 0);
        route_item_list.removeFirst();
    }
    
    // set arrival airport
    
    Airport *destination_airport = 0;

    result_list.clear();
    navdata.getAirports(route_item_list[route_item_list.count()-1].trimmed(), result_list);
    if (result_list.count() == 1)
    {
        destination_airport = (Airport*)result_list.at(0)->deepCopy();
        route_item_list.removeLast();
    }

    // extract route

    int item_index = 0;
    Waypoint last_wpt;
    while(item_index < route_item_list.count())
    {
        if (route_item_list[item_index].trimmed().isEmpty())
        {
            ++item_index;
            continue;
        }

        // handle direct waypoints
        if (route_item_list[item_index].trimmed() == "DCT")
        {
            ++item_index;
            last_wpt = Waypoint();
            continue;
        }
        
        // if we got no last waypoint search for the new wpt
        if (!last_wpt.isValid() || (route_item_list.count() - (item_index+1)) < 1)
        {
            last_wpt = Waypoint();
            result_list.clear();
            
            navdata.getWaypoints(route_item_list[item_index].trimmed(), result_list, lat_lon_wpt_regexp);
            if (result_list.count() <= 0)
            {
                // we tolerate not finding the last waypoint, because
                // this could be a star or other procedure
                if (item_index+1 == route_item_list.count()) break;
                
                error = QString("%1 not found").arg(route_item_list[item_index].trimmed());
                return false;
            }
        
            if (last_wpt.isValid()) result_list.sortByDistance(last_wpt);
            else                    result_list.sortByDistance(*departureAirport());
            
            //TODO search for the right waypoint if we got an airway
            //and found more than one possible first waypoints
            appendWaypoint(*result_list.at(0));
            if ((route_item_list.count() - (item_index+1)) >= 1) last_wpt = *result_list.at(0);
            ++item_index;
        }
        else
        {
            result_list.clear();
            QString error_text;
            if (!navdata.getWaypointsByAirway(
                    last_wpt, route_item_list[item_index].trimmed(), 
                    route_item_list[item_index+1].trimmed(), result_list, error_text))
            {
                // if we got no airway we search for a waypoint in the next loop instead
                last_wpt = Waypoint();
                continue;
            }
            
            WaypointPtrListIterator iter(result_list);
            while(iter.hasNext()) appendWaypoint(*iter.next());
            last_wpt = *result_list.last()->deepCopy();
            item_index += 2;
        }
    }
    
    if (destination_airport != 0)
    {
        appendWaypoint(*destination_airport);
        setAsDestinationAirport(count()-1, QString::null);
        MYASSERT(destinationAirport() != 0);
        delete destination_airport;
        destination_airport = 0;
    }
    
    removeDoubleWaypoints();

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool FlightRoute::calcProjection(const ProjectionBase& projection, int start_index, int end_index)
{
    projection.convertLatLonToXY(altReachWpt());
    projection.convertLatLonToXY(todWpt());
    return Route::calcProjection(projection, start_index, end_index);
}

/////////////////////////////////////////////////////////////////////////////

void FlightRoute::calcDistanceActiveWptToDestination()
{
    m_distance_from_active_wpt_to_destination = 0.0;

    int end_index = destinationAirportIndex();
    if (end_index < 0) end_index = count()-1;

    for (int index=activeWaypointIndex(); index < end_index; ++index)
        m_distance_from_active_wpt_to_destination += distanceNMToNextWaypoint(index);
}

/////////////////////////////////////////////////////////////////////////////

void FlightRoute::operator<<(QDataStream& in)
{
    in >> m_adep_wpt_index
       >> m_adep_id
       >> m_sid_id
       >> m_sid_transition_id
       >> m_star_id
       >> m_app_transition_id
       >> m_approach_id
       >> m_ades_wpt_index
       >> m_ades_id
       >> m_active_wpt_index
       >> m_view_wpt_index
       >> m_cruise_fl
       >> m_cruise_temp
       >> m_company_route
       >> m_flight_number
       >> m_cost_index
       >> m_tropo_pause
       >> m_distance_from_active_wpt_to_destination
       >> m_thrust_reduction_altitude_ft
       >> m_acceleration_altitude_ft;

    m_alt_reach_wpt << in;
    m_tod_wpt << in;

    Route::operator<<(in);
}

/////////////////////////////////////////////////////////////////////////////

void FlightRoute::operator>>(QDataStream& out) const
{
    out << m_adep_wpt_index
        << m_adep_id
        << m_sid_id
        << m_sid_transition_id
        << m_star_id
        << m_app_transition_id
        << m_approach_id
        << m_ades_wpt_index
        << m_ades_id
        << m_active_wpt_index
        << m_view_wpt_index
        << m_cruise_fl
        << m_cruise_temp
        << m_company_route
        << m_flight_number
        << m_cost_index
        << m_tropo_pause
        << m_distance_from_active_wpt_to_destination
        << m_thrust_reduction_altitude_ft
        << m_acceleration_altitude_ft;

    m_alt_reach_wpt >> out;
    m_tod_wpt >> out;

    Route::operator>>(out);
}

/////////////////////////////////////////////////////////////////////////////

// End of file
