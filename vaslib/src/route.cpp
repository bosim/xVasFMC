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

#include "logger.h"
#include "navcalc.h"
#include "navdata.h"
#include "projection.h"
#include "declination.h"
#include "flightstatus.h"

#include "route.h"
#include "waypoint_serialization.h"

/////////////////////////////////////////////////////////////////////////////

void RouteData::operator>>(QDataStream& out) const
{
    out << m_true_track_to_next_wpt
        << m_dist_to_next_wpt_nm
        << m_true_track_from_prev_wpt
        << m_dist_from_prev_wpt_nm
        << m_time_over_waypoint;
}

/////////////////////////////////////////////////////////////////////////////

void RouteData::operator<<(QDataStream& in)
{
    in >> m_true_track_to_next_wpt
       >> m_dist_to_next_wpt_nm
       >> m_true_track_from_prev_wpt
       >> m_dist_from_prev_wpt_nm
       >> m_time_over_waypoint;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

QString Route::TYPE_ROUTE = "ROUTE";
QString Route::TYPE_AIRWAY = "AIRWAY";
QString Route::TYPE_PROCEDURE = "PROCEDURE";
QString Route::TYPE_SID = "SID";
QString Route::TYPE_STAR = "STAR";
QString Route::TYPE_APPROACH = "APP";
QString Route::TYPE_TRANSITION = "TRANS";

QString Route::FLAG_NORMAL = "NORM";
QString Route::FLAG_ALTERNATE = "ALT";
QString Route::FLAG_SECONDARY = "SEC";
QString Route::FLAG_TEMPORARY = "TEMP";

/////////////////////////////////////////////////////////////////////////////

Route::Route(const QString& id, const FlightStatus* flightstatus) : 
    m_flightstatus(flightstatus), m_type(TYPE_ROUTE), m_flag_fixed(false), m_id(id) 
{
};

/////////////////////////////////////////////////////////////////////////////

Route::Route(const Route& other) : QObject(), SerializationIface()
{
    *this = other;
}

/////////////////////////////////////////////////////////////////////////////

const Route& Route::operator=(const Route& other)
{
    if (&other == this) return *this;
    m_type = other.m_type;
    if (!m_flag_fixed) m_flag = other.m_flag;
    m_id = other.m_id;
    m_wpt_list = other.m_wpt_list;
    m_routedata_list = other.m_routedata_list;
    m_flightstatus = other.m_flightstatus;
    emit signalChanged(m_flag, true, "Route:operator=");
    return *this;
}

/////////////////////////////////////////////////////////////////////////////

bool Route::compareWaypoints(const Route& other) const
{
    if (other.count() != count()) return false;

    for(int index=0; index<count(); ++index)
        if (*other.waypoint(index) != *waypoint(index)) return false;

    return true;
}

/////////////////////////////////////////////////////////////////////////////

void Route::clear()
{
    m_wpt_list.clear();
    m_routedata_list.clear();
    emit signalChanged(m_flag, true, "Route:clear");
}

/////////////////////////////////////////////////////////////////////////////

void Route::appendWaypoint(const Waypoint& wpt)
{
    Waypoint* wpt_copy = wpt.deepCopy();
    MYASSERT(wpt_copy != 0);
    MYASSERT(wpt_copy->type() == wpt.type());
    wpt_copy->resetIfDependendWaypoint();
    m_wpt_list.append(wpt_copy);
    m_routedata_list.append(RouteData());

    recalcWaypointData(count()-2);
    recalcWaypointData(count()-1);

    checkAndSetSpecialWaypoint(count()-1, false);

    emit signalChanged(m_flag, true, "Route:appendWaypoint");
};

/////////////////////////////////////////////////////////////////////////////

void Route::insertWaypoint(const Waypoint& wpt, int pos)
{
//     Logger::log(QString("Route:insertWaypoint: %1 (%2) [%3] / %4/%5/%6").
//                 arg(wpt.id()).arg(wpt.type()).arg(pos).
//                 arg(wpt.toString()).arg(wpt.lat()).arg(wpt.lon()));

    MYASSERT(pos >= 0);
    MYASSERT(pos <= count());
    Waypoint* wpt_copy = wpt.deepCopy();
    MYASSERT(wpt_copy != 0);
    MYASSERT(wpt_copy->type() == wpt.type());
    wpt_copy->resetIfDependendWaypoint();
    m_wpt_list.insert(pos, wpt_copy);
    m_routedata_list.insert(pos, RouteData());

    recalcWaypointData(pos-1);
    recalcWaypointData(pos);
    recalcWaypointData(pos+1);
    
    checkAndSetSpecialWaypoint(pos, false);

    emit signalChanged(m_flag, true, "Route:insertWaypoint");
};

/////////////////////////////////////////////////////////////////////////////

bool Route::removeWaypoint(int pos)
{
    MYASSERT(pos >= 0);
    MYASSERT(pos < count());
    m_wpt_list.removeAt(pos);
    m_routedata_list.removeAt(pos);

    recalcWaypointData(pos-1);
    recalcWaypointData(pos);
    
    emit signalChanged(m_flag, true, "Route:removeWaypoint");

    return true;
}

/////////////////////////////////////////////////////////////////////////////

void Route::removeDoubleWaypoints()
{
    for(int index=0; index < m_wpt_list.count()-1; )
    {
        const Waypoint* cur_wpt = m_wpt_list[index];
        const Waypoint* next_wpt = m_wpt_list[index+1];

        if (*cur_wpt == *next_wpt) 
        {
            removeWaypoint(index+1);
            continue;
        }

        ++index;
    }
}

/////////////////////////////////////////////////////////////////////////////

bool Route::containsWaypoint(const Waypoint& wpt) const
{
    WaypointPtrListIterator iter(m_wpt_list);
    while(iter.hasNext())
    {
        const Waypoint* existing_wpt = iter.next();
        MYASSERT(existing_wpt != 0);
        if (*existing_wpt == wpt) return true;
    }
    
    return false;
}

/////////////////////////////////////////////////////////////////////////////

void Route::recalcWaypointData(int pos)
{
    if (pos < 0 || pos >= count()) return;

    const Waypoint* prev_wpt = waypoint(pos-1);
    const Waypoint* wpt = waypoint(pos);
    const Waypoint* next_wpt = waypoint(pos+1);
    MYASSERT(wpt != 0);
    
    RouteData& route_data = routeData(pos);

    if (prev_wpt != 0)
    {
        MYASSERT(Navcalc::getDistAndTrackBetweenWaypoints(
                     *prev_wpt, *wpt, route_data.m_dist_from_prev_wpt_nm, route_data.m_true_track_from_prev_wpt));
    }
    else
    {
        route_data.m_dist_from_prev_wpt_nm = 0.0;
        route_data.m_true_track_from_prev_wpt = 0.0;
    }

    if (next_wpt != 0)
    {
        MYASSERT(Navcalc::getDistAndTrackBetweenWaypoints(
                     *wpt, *next_wpt, route_data.m_dist_to_next_wpt_nm, route_data.m_true_track_to_next_wpt));
    }
    else
    {
        route_data.m_dist_to_next_wpt_nm = 0.0;
        route_data.m_true_track_to_next_wpt = 0.0;
    }

    MYASSERT(m_wpt_list.count() == m_routedata_list.count());
}

/////////////////////////////////////////////////////////////////////////////

void Route::scanForWaypointInformation(const Navdata& navdata)
{
    // go through the waypoint list and look for navaids and airports in 
    // order to be able to display more information

    QTime start_time;
    start_time.start();

    for(int index=0; index < count(); ++index)
    {
        Waypoint* route_wpt = waypoint(index);
		MYASSERT(route_wpt != 0);
		
		if (route_wpt->type() != Waypoint::TYPE_WAYPOINT || route_wpt->id().length() > 4) continue;
		
        bool found = false;

        for(int count=0; count < 2; ++count)
        {
            WaypointPtrList possible_navdata_wpt_list;
            possible_navdata_wpt_list.clear();

            switch(count) {
                case(0): {
                    navdata.getNavaids(route_wpt->id(), possible_navdata_wpt_list);
                    break;
                }
                case(1): {
                    navdata.getAirports(route_wpt->id(), possible_navdata_wpt_list);
                    break;
                }
                case(2): {
                    navdata.getIntersections(route_wpt->id(), possible_navdata_wpt_list);
                    break;
                }
                default: {
                    MYASSERT(false);
                    break;
                }
            }

            if (possible_navdata_wpt_list.count() <= 0) continue;
            
            // scan through the list of found waypoint from the navdatabase
            
            WaypointPtrListIterator iter(possible_navdata_wpt_list);
            for(; iter.hasNext();)
            {
                Waypoint* navdata_wpt = iter.next();

                double diff_distance = Navcalc::getDistBetweenWaypoints(*navdata_wpt, *route_wpt);

                // for airport we must also search through the runway positions

                Airport* airport = navdata_wpt->asAirport();

                if(airport != 0)
                {
					RunwayMap::ConstIterator iter = airport->runwayMap().begin();
					for(; iter != airport->runwayMap().end(); ++iter)
					{
						const Runway& rwy = *iter;
						
						if (rwy.lat() == route_wpt->lat() && rwy.lon() == route_wpt->lon())
						{
//                             Logger::log(QString("Route:scanForWaypointInformation: rwy match: %1 - %2").
//                                         arg(rwy.toString()).arg(navdata_wpt->toString()));

							airport->setActiveRunwayId(rwy.id());
                            Waypoint* new_waypoint = airport->deepCopy();
                            new_waypoint->setParent(route_wpt->parent());
                            new_waypoint->setFlag(route_wpt->flag());
                            new_waypoint->restrictions() = route_wpt->restrictions();
                            Route::removeWaypoint(index);
                            Route::insertWaypoint(*new_waypoint, index);
                            delete new_waypoint;
							found = true;
							break;
						}
					}
	
					if (!found && diff_distance < 1.0)
					{
//                         Logger::log(QString("Route:scanForWaypointInformation: airport match: %1 - %2").
//                                     arg(airport->toString()).arg(route_wpt->id()));
                    
                        Waypoint* new_waypoint = airport->deepCopy();
                        new_waypoint->setParent(route_wpt->parent());
                        new_waypoint->setFlag(route_wpt->flag());
                        new_waypoint->restrictions() = route_wpt->restrictions();
                        Route::removeWaypoint(index);
                        Route::insertWaypoint(*new_waypoint, index);
                        delete new_waypoint;
						found = true;
                        break;
					}

                    if (found) break;
                }

                // if the waypoints are the same, make a new FMC waypoint
                // out of the waypoint from the navdatabase and the previous fmc waypoint.
                
                if (!found && diff_distance < 1.0)
                {
//                    Logger::log(QString("Route:scanForWaypointInformation: match %1").arg(navdata_wpt->toString()));

                    Waypoint* new_waypoint = navdata_wpt->deepCopy();
                    new_waypoint->setParent(route_wpt->parent());
                    new_waypoint->setFlag(route_wpt->flag());
                    new_waypoint->restrictions() = route_wpt->restrictions();
                    Route::removeWaypoint(index);
                    Route::insertWaypoint(*new_waypoint, index);
                    delete new_waypoint;
                    found = true;
                    break;
                }

                MYASSERT(!found);
            }

            if (found) break;
        }
    }

    Logger::log(QString("Route:scanForWaypointInformation: scanned route in %1ms").arg(start_time.elapsed()));
}

/////////////////////////////////////////////////////////////////////////////

bool Route::calcProjection(const ProjectionBase& projection, int start_index, int end_index)
{
    if (count() == 0 || start_index < 0 || start_index >= count()) return false;

    if (end_index < 0) end_index = count() - 1;
    else if (end_index >= count()) end_index = count() - 1;
    
    for(int index = start_index; index <= end_index; ++index)
    {
        Waypoint* wpt = waypoint(index);
        
        checkAndSetSpecialWaypoint(index, false);
        projection.convertLatLonToXY(*wpt);

        if (wpt->asAirport() != 0)
        {
            QMapIterator<QString, Runway> rwy_iter(wpt->asAirport()->runwayMap());
            while(rwy_iter.hasNext())
            {
                rwy_iter.next();
                projection.convertLatLonToXY(wpt->asAirport()->runwayMap()[rwy_iter.key()]);
            }
        }
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////

void Route::resetAndRecalcSpecialWaypoints()
{
    int index;
    for (index=0; index < count(); ++index) waypoint(index)->resetIfDependendWaypoint();
    for (index=0; index < count(); ++index) checkAndSetSpecialWaypoint(index, false);
}

/////////////////////////////////////////////////////////////////////////////

bool Route::checkAndSetSpecialWaypoint(int pos, bool take_current_flightdata_as_reference)
{
    Waypoint* wpt = waypoint(pos);
    if (wpt == 0) return false;

//     if (wpt->id().length() > 1)
//         Logger::log(QString("Route:checkAndSetSpecialWaypoint: ----------------------- %1 ----------------------").
//                     arg(wpt->id()));

    bool changed = false;

    bool prev_waypoint_location_ok = false;
    if (pos > 0) prev_waypoint_location_ok = waypoint(pos-1)->lat() != 0.0 && waypoint(pos-1)->lon() != 0.0;

    if (wpt->lat() == 0.0 && wpt->lon() == 0.0 && pos > 0 && pos < count() && prev_waypoint_location_ok)
    {
        const Waypoint* prev_wpt = waypoint(pos-1);

        if (wpt->asWaypointHdgToAlt() != 0)
        {
            Logger::log("Route:checkAndSetSpecialWaypoint: hdg2alt wpt");

            //TODO calc this later based on the estimated climb/descent rate (VNAV)
            Waypoint new_wpt = 
                Navcalc::getPBDWaypoint(*prev_wpt, wpt->asWaypointHdgToAlt()->hdgToHold(), 1.0, 
                                        Declination::globalDeclination());
    
            Logger::log(QString("Route:checkAndSetSpecialWaypoint: got hdg2alt wpt: %1 "
                                " -> %2/%3").arg(wpt->toString()).arg(new_wpt.lat()).arg(new_wpt.lon()));
            
            wpt->setLat(new_wpt.lat());
            wpt->setLon(new_wpt.lon());
            changed = true;
        }
        else if (wpt->asWaypointHdgToIntercept() != 0)
        {
            Logger::log(QString("Route:checkAndSetSpecialWaypoint: hdg2intercept: start: wpt %1/%2 <-> %3/%4").
                        arg(prev_wpt->toString()).arg(prev_wpt->parent()).arg(wpt->toString()).arg(wpt->parent()));

            const WaypointHdgToIntercept* icpt_wpt = wpt->asWaypointHdgToIntercept();
            
            Waypoint new_wpt;
            
            Waypoint from_wpt;

            //TODO take the set route speed, not this default
            double speed = 100.0;

            // determine the hdg before the turn to intercept
            double true_hdg_before_turn = trueTrackFromPrevWaypoint(pos-1);

            if (take_current_flightdata_as_reference && m_flightstatus != 0) 
            {
                speed = qMax(speed, m_flightstatus->ground_speed_kts);
                true_hdg_before_turn = m_flightstatus->smoothedTrueHeading();
            }

            // determine the point after we have turned to the intercept heading
            Navcalc::getPointAfterTurn(
                *prev_wpt, 
                true_hdg_before_turn,
                speed,
                24.0,
                icpt_wpt->hdgUntilIntercept() - Declination::globalDeclination()->declination(icpt_wpt->fixToIntercept()),
                icpt_wpt->turnDirection(),
                from_wpt);

            if (Navcalc::getPBPBWaypoint(
                    from_wpt, Navcalc::trimHeading(icpt_wpt->hdgUntilIntercept()),
                    icpt_wpt->fixToIntercept(), icpt_wpt->radialToIntercept(),
                    Declination::globalDeclination(), new_wpt))
            {
                Logger::log(QString("Route:checkAndSetSpecialWaypoint: got hdg2intercept wpt: %1 "
                                    " -> %2/%3").arg(wpt->id()).arg(new_wpt.lat()).arg(new_wpt.lon()));
                
                wpt->setLat(new_wpt.lat());
                wpt->setLon(new_wpt.lon());
            }
            else
            {
                Logger::log(QString("Route:checkAndSetSpecialWaypoint: hdg2intercept wpt not found - taking PBD wpt"));
                wpt->setLat(from_wpt.lat());
                wpt->setLon(from_wpt.lon());
            }
            
            changed = true;
        }
    }

    if (changed)
    {
        recalcWaypointData(pos-1);
        recalcWaypointData(pos);
        recalcWaypointData(pos+1);
        emit signalChanged(m_flag, true, "Route:checkAndSetSpecialWaypoint");
        return true;
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////

void Route::operator>>(QDataStream& out) const
{
    out << m_type
        << m_flag
        << m_flag_fixed
        << m_id
        << m_wpt_list
        << m_routedata_list;
}

/////////////////////////////////////////////////////////////////////////////

void Route::operator<<(QDataStream& in)
{
    in >> m_type
       >> m_flag
       >> m_flag_fixed
       >> m_id
       >> m_wpt_list
       >> m_routedata_list;

    emit signalChanged(m_flag, false, "Route:operator<<");
}

/////////////////////////////////////////////////////////////////////////////

// End of file
