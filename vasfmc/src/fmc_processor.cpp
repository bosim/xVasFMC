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

/*! \file    fmc_processor.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include <QDateTime>

#include "assert.h"
#include "logger.h"
#include "config.h"
#include "navcalc.h"
#include "navdata.h"
#include "airport.h"
//#include "projection_mercator.h"
#include "projection_greatcircle.h"
#include "flightstatus.h"

#ifdef Q_OS_WIN32
#include "code_timer.h"
#define DO_PROFILING 0
#endif

#include "fmc_data.h"
#include "fmc_control.h"
#include "fmc_processor.h"
#include "geodata.h"

#define CFG_PROCESSOR_REFRESH_PERIOD_MS "processor_refresh_period_milliseconds"
#define CFG_ALT_DIFF_TO_TOD_EST_FACTOR "alt_diff_to_tod_estimate_factor"
#define CFG_KTS_TO_DESCENT_FPM_FACTOR "kts_to_descent_fpm_factor"
#define CFG_MIN_TAS_KTS_SWITCH_WPT "min_tas_kts_switch_waypoints"
#define CFG_MAX_TAS_KTS_SWITCH_WPT "max_tas_kts_switch_waypoints"
#define CFG_OVERFLY_TURN_DIST_NM "overfly_turn_dist"
#define CFG_TURN_RADIUS_SPEED_FACTOR "turn_radius_speed_factor"
#define CFG_DESCENT_ESTIMATE_RECALC_PERIOD_MS "descent_estimate_recalc_period_milliseconds"
#define CFG_WPT_TIMES_RECALC_PERIOD_MS "waypoint_times_recalc_period_milliseconds"
#define CFG_ALT_REACH_RECALC_PERIOD_MS "altitude_reach_recalc_period_milliseconds"
#define CFG_PROJECTION_RECALC_DISTANCE_NM "projection_recalc_distance_nm"
#define CFG_MAX_ONGROUND_WPT_SKIP_DIST_NM "max_onground_wpt_skip_dist_nm"
#define CFG_MAX_SURROUNDING_AIRPORT_DIST_NM "max_surrounding_airport_dist_nm"
#define CFG_MAX_SURROUNDING_VOR_DIST_NM "max_surrounding_vor_dist_nm"
#define CFG_MAX_SURROUNDING_NDB_DIST_NM "max_surrounding_ndb_dist_nm"
#define CFG_MAX_SURROUNDING_GEO_DIST_NM "max_surrounding_geo_dist_nm"

#define PROJECTION_RECALC_INC 15

/////////////////////////////////////////////////////////////////////////////

FMCProcessor::FMCProcessor(ConfigWidgetProvider* config_widget_provider,
                           const QString& processor_cfg_filename, 
                           FMCControl* fmc_control,
                           Navdata* navdata) :
    m_config_widget_provider(config_widget_provider), m_fmc_data(fmc_control->fmcData()), 
    m_fmc_control(fmc_control), m_flightstatus(fmc_control->flightStatus()), m_navdata(navdata),
    m_flightstatus_was_valid_once(false), m_wpt_times_recalc_wpt_index(0), m_data_changed(true),
    m_last_toc_eod_ground_speed_kts(0), m_last_toc_eod_vs_ftmin(0), m_last_toc_eod_ap_diff_alt(0),
    m_last_descend_distance_nm(0.0),
    m_project_recalc_index_route_normal(-1), m_project_recalc_index_route_temporary(-1),
    m_project_recalc_index_route_alternate(-1), m_project_recalc_index_route_secondary(-1),
    m_project_recalc_airports(-1), m_project_recalc_vors(-1), m_project_recalc_ndbs(-1), 
    m_project_recalc_geo(-1), m_normal_route_view_wpt_index(-1), m_next_wpt_with_alt_constraint_index(-1)
{
    MYASSERT(m_config_widget_provider != 0);
    MYASSERT(m_fmc_control != 0);
    MYASSERT(m_flightstatus != 0);
    MYASSERT(m_navdata != 0);

    // setup config

    m_processor_cfg = new Config(processor_cfg_filename);
    MYASSERT(m_processor_cfg);
    setupDefaultConfig();
    m_processor_cfg->loadfromFile();
    m_processor_cfg->saveToFile();
    m_config_widget_provider->registerConfigWidget("Processor", m_processor_cfg);

    // setup projection

    //m_projection = new ProjectionMercator;
    m_projection = new ProjectionGreatCircle;
    MYASSERT(m_projection != 0);

    // connect fmc data signals

    MYASSERT(connect(&m_fmc_data, SIGNAL(signalDataChanged(const QString&, bool, const QString&)), 
                     this, SLOT(slotDataChanged(const QString&, bool, const QString&))));

    m_refresh_timer.start();
    m_descent_estimate_recalc_timer.start();
    m_wpt_times_recalc_timer.start();
    m_alt_reach_recalc_timer.start();
}

/////////////////////////////////////////////////////////////////////////////

FMCProcessor::~FMCProcessor() 
{
    m_processor_cfg->saveToFile();
    delete m_processor_cfg;
    delete m_projection;
}

/////////////////////////////////////////////////////////////////////////////

void FMCProcessor::setupDefaultConfig()
{
    m_processor_cfg->setValue(CFG_PROCESSOR_REFRESH_PERIOD_MS, 250);
    m_processor_cfg->setValue(CFG_ALT_DIFF_TO_TOD_EST_FACTOR, 0.00305);
    m_processor_cfg->setValue(CFG_KTS_TO_DESCENT_FPM_FACTOR, 5.410655);
    m_processor_cfg->setValue(CFG_MIN_TAS_KTS_SWITCH_WPT, 30.0);
    m_processor_cfg->setValue(CFG_MAX_TAS_KTS_SWITCH_WPT, 3000.0);
    m_processor_cfg->setValue(CFG_OVERFLY_TURN_DIST_NM, 0.5);
    m_processor_cfg->setValue(CFG_TURN_RADIUS_SPEED_FACTOR, 0.0125);
    m_processor_cfg->setValue(CFG_DESCENT_ESTIMATE_RECALC_PERIOD_MS, 4000);
    m_processor_cfg->setValue(CFG_WPT_TIMES_RECALC_PERIOD_MS, 5000);
    m_processor_cfg->setValue(CFG_ALT_REACH_RECALC_PERIOD_MS, 500);
    m_processor_cfg->setValue(CFG_PROJECTION_RECALC_DISTANCE_NM, 20.0);
    m_processor_cfg->setValue(CFG_MAX_ONGROUND_WPT_SKIP_DIST_NM, 5.0);

    m_processor_cfg->setValue(CFG_MAX_SURROUNDING_AIRPORT_DIST_NM, 80);
    m_processor_cfg->setValue(CFG_MAX_SURROUNDING_VOR_DIST_NM, 80);
    m_processor_cfg->setValue(CFG_MAX_SURROUNDING_NDB_DIST_NM, 80);
    m_processor_cfg->setValue(CFG_MAX_SURROUNDING_GEO_DIST_NM, 320);
}

/////////////////////////////////////////////////////////////////////////////

void FMCProcessor::clearCalculatedValues()
{
    m_fmc_data.setDistanceToActiveWptNm(0.0);
    m_fmc_data.setDistanceFromPreviousWptNm(0.0);
    m_fmc_data.setCrossTrackDistanceNm(0.0);
}

/////////////////////////////////////////////////////////////////////////////

void FMCProcessor::slotRefresh(bool force)
{
    if (!force && m_refresh_timer.elapsed() < m_processor_cfg->getIntValue(CFG_PROCESSOR_REFRESH_PERIOD_MS)) return;
    m_refresh_timer.start();

    QTime overall_timer;
    overall_timer.start();

    clearCalculatedValues();

    FlightRoute& normal_route = m_fmc_data.normalRoute();

    Waypoint* active_wpt = normal_route.activeWaypoint();
    const Waypoint* prev_wpt = normal_route.previousWaypoint();
    const Waypoint* next_wpt = normal_route.nextWaypoint();

    double vs = m_flightstatus->smoothedVS();
    double altimeter_readout = m_flightstatus->smoothed_altimeter_readout.lastValue();
    double ap_diff_alt = m_flightstatus->APAlt() - altimeter_readout;

    //----- override current position in case we are not connected to the flightsim

    if (m_flightstatus->isValid())
        m_flightstatus_was_valid_once = true;
    else if (!m_flightstatus_was_valid_once && normal_route.waypoint(0) != 0)
    {
        m_flightstatus->current_position_raw = *normal_route.waypoint(0);
        m_flightstatus->current_position_smoothed = *normal_route.waypoint(0);
    }

    //----- calc turn radius

    m_fmc_data.setTurnRadiusNm(
        m_flightstatus->ground_speed_kts * m_processor_cfg->getDoubleValue(CFG_TURN_RADIUS_SPEED_FACTOR));

    //----- waypoint switch stuff

    bool waypoints_switched = false;

    do
    {
        waypoints_switched = false;

        //----- set active stuff
        
        if (active_wpt != 0)
        {
            m_fmc_data.setDistanceToActiveWptNm(
                Navcalc::getDistBetweenWaypoints(m_flightstatus->current_position_raw, *active_wpt));
            
            m_fmc_data.setTrueTrackToActiveWpt(
                Navcalc::getTrackBetweenWaypoints(m_flightstatus->current_position_raw, *active_wpt));
            
            if (m_flightstatus->ground_speed_kts < 30.0)
                m_fmc_data.setHoursToActiveWpt(0.0);
            else
                m_fmc_data.setHoursToActiveWpt(m_fmc_data.distanceToActiveWptNm() / m_flightstatus->ground_speed_kts);
        }
        
        //----- set prev stuff
        
        if (prev_wpt != 0)
        {
            m_fmc_data.setDistanceFromPreviousWptNm(
                Navcalc::getDistBetweenWaypoints(m_flightstatus->current_position_raw, *prev_wpt));
            
            double dummy1 = 0.0;
            double dummy2 = 0.0;
            double dummy3 = 0.0;

            if (active_wpt != 0)
                m_fmc_data.setCrossTrackDistanceNm(
                    Navcalc::getCrossTrackDistance(*prev_wpt, *active_wpt, m_flightstatus->current_position_raw, 
                                                   dummy1, dummy2, dummy3));
        }

        //----- check if to switch to the next waypoint and update last removed waypoint

        // We check the TAS here in order to detect slew mode and to disable
        // recalc when standing on the ground.
        // We also check if we are inside an acive holding and if the exit 
        // holding flag ist set and we are inbound the holding fix

        if (m_flightstatus->isValid() &&
            !m_flightstatus->slew &&
            !m_fmc_control->isFMCConnectModeSlave() &&
            active_wpt != 0 && 
            (!active_wpt->holding().isValid() ||
             (active_wpt->holding().exitHolding() &&
              (active_wpt->holding().status() == Holding::STATUS_INSIDE_LEG_1 ||
               active_wpt->holding().status() == Holding::STATUS_ENTRY_TO_FIX))) &&
            m_flightstatus->ground_speed_kts > m_processor_cfg->getDoubleValue(CFG_MIN_TAS_KTS_SWITCH_WPT) &&
            m_flightstatus->ground_speed_kts < m_processor_cfg->getDoubleValue(CFG_MAX_TAS_KTS_SWITCH_WPT))
        {
            bool do_switch_waypoint = false;

            bool alt_restriction_not_met =
                (active_wpt->restrictions().altitudeGreaterRestriction() &&
                 altimeter_readout < active_wpt->restrictions().altitudeRestrictionFt())
                ||
                (active_wpt->restrictions().altitudeSmallerRestriction() &&
                 altimeter_readout > active_wpt->restrictions().altitudeRestrictionFt());

            bool alt_restriction_met =
                (active_wpt->restrictions().altitudeGreaterRestriction() &&
                 altimeter_readout >= active_wpt->restrictions().altitudeRestrictionFt())
                ||
                 (active_wpt->restrictions().altitudeSmallerRestriction() &&
                  altimeter_readout <= active_wpt->restrictions().altitudeRestrictionFt());

//             Logger::log(QString("notmet=%1  met=%2  hdg2alt=%3").
//                         arg(alt_restriction_not_met).
//                         arg(alt_restriction_met).
//                         arg(active_wpt->asWaypointHdgToAlt() != 0));

            if (active_wpt->asWaypointHdgToAlt() != 0)
            {
                if (!alt_restriction_not_met) do_switch_waypoint = true;
            }
            else
            {
                double turn_dist = m_fmc_data.turnRadiusNm();

                bool wpt_lies_behind = 
                    Navcalc::isWaypointBehind(m_fmc_data.trueTrackToActiveWpt(), m_flightstatus->smoothedTrueHeading());

                // calc the turn distance

                if (next_wpt == 0 || active_wpt->restrictions().hasOverflyRestriction())
                {
                    // override turn dist for overfly waypoint
                    turn_dist = m_processor_cfg->getDoubleValue(CFG_OVERFLY_TURN_DIST_NM);
                }
                else if (next_wpt != 0)
                {
                    // TODO check total turn value when >90 degrees
                    // TODO check left/right turn flag
                
                    // if there is another waypoint after the active waypoint
                    turn_dist = Navcalc::getPreTurnDistance(
                        m_processor_cfg->getDoubleValue(CFG_OVERFLY_TURN_DIST_NM), m_fmc_data.turnRadiusNm(),
                        (int)m_flightstatus->ground_speed_kts, m_flightstatus->smoothedTrueHeading(), 
                        normal_route.trueTrackFromActiveToNextWpt());

                    if (Navcalc::getAbsHeadingDiff(
                            m_flightstatus->smoothedTrueHeading(), normal_route.trueTrackFromActiveToNextWpt()) > 135)
                    {
                        turn_dist = m_processor_cfg->getDoubleValue(CFG_OVERFLY_TURN_DIST_NM);
                    }
                }

                // check if we are on ground and the active waypoint lies behind us
                if (m_flightstatus->onground &&
                    m_fmc_data.distanceToActiveWptNm() <
                    m_processor_cfg->getDoubleValue(CFG_MAX_ONGROUND_WPT_SKIP_DIST_NM) &&
                    wpt_lies_behind)
                {
                    Logger::log("FMCProcessor:slotRefresh: detected WPT lies behind");
                    turn_dist = m_processor_cfg->getDoubleValue(CFG_MAX_ONGROUND_WPT_SKIP_DIST_NM);
                }

                // switch to the next waypoint

                // do not switch if ALT restriction not met
                if ( ((!alt_restriction_not_met || active_wpt->asWaypointHdgToAlt() == 0) && m_fmc_data.distanceToActiveWptNm() <= turn_dist) ||
                    (alt_restriction_met && wpt_lies_behind && 
                     qAbs(m_flightstatus->smoothedTrueTrack() - 
                          normal_route.trueTrackFromPrevWaypoint(normal_route.activeWaypointIndex())) < 10.0))
                {
                    Logger::log(QString("FMCProcessor:slotRefresh: @%1 gs=%2 dist=%3 turndist=%4").
                                arg(active_wpt->id()).
                                arg(m_flightstatus->ground_speed_kts).
                                arg(m_fmc_data.distanceToActiveWptNm()).
                                arg(turn_dist));
                    
                    if (wpt_lies_behind) active_wpt->restrictions().setOverflyRestriction(true);
                    do_switch_waypoint = true;
                }
            }

            if (do_switch_waypoint)
            {
                normal_route.switchToNextWaypoint();
                active_wpt = normal_route.activeWaypoint();
                prev_wpt = normal_route.previousWaypoint();
                next_wpt = normal_route.nextWaypoint();
                waypoints_switched = true;
                m_data_changed = true;
            }
        }
    }
    while(waypoints_switched);

    //----- calculate waypoint times and overall values by iteration over the route

 	if (m_data_changed) m_wpt_times_recalc_wpt_index = 0;
    m_wpt_times_recalc_wpt_index = qMax(m_wpt_times_recalc_wpt_index, normal_route.activeWaypointIndex());

    // loop when reaching the end
    if (m_wpt_times_recalc_wpt_index >= normal_route.count()) 
        m_wpt_times_recalc_wpt_index = normal_route.activeWaypointIndex();

    if (m_wpt_times_recalc_wpt_index < normal_route.count())
    {
        if (m_wpt_times_recalc_wpt_index < (int)normal_route.activeWaypointIndex())
        {
            // do nothing for waypoints behind
        }
        else if (m_wpt_times_recalc_wpt_index == (int)normal_route.activeWaypointIndex())
        {
            if (m_flightstatus->ground_speed_kts < 30.0)
            {
                normal_route.routeData(m_wpt_times_recalc_wpt_index).m_time_over_waypoint = QTime();
                normal_route.setHoursActiveWptToDestination(0.0);
            }
            else
            {
                normal_route.routeData(m_wpt_times_recalc_wpt_index).m_time_over_waypoint = 
                    m_flightstatus->fs_utc_time.addSecs(
                        (int)((m_fmc_data.distanceToActiveWptNm() / m_flightstatus->ground_speed_kts) * 3600));
                
                normal_route.setHoursActiveWptToDestination(
                    normal_route.distanceActiveWptToDestination() / m_flightstatus->ground_speed_kts);
            }
        }
        else  if (m_wpt_times_recalc_wpt_index > 0)
        {
            RouteData& prev_wpt_data = normal_route.routeData(m_wpt_times_recalc_wpt_index-1);
            RouteData& wpt_data = normal_route.routeData(m_wpt_times_recalc_wpt_index);

            if (m_flightstatus->ground_speed_kts < 30.0)
            {
                wpt_data.m_time_over_waypoint = QTime();
            }
            else
            {
                wpt_data.m_time_over_waypoint = 
                    prev_wpt_data.m_time_over_waypoint.addSecs(
                        (int)((wpt_data.m_dist_from_prev_wpt_nm / m_flightstatus->ground_speed_kts) * 3600));
            }
        }

        ++m_wpt_times_recalc_wpt_index;
    }

    //----- calc intermediate TOC/EOD

 	if (m_data_changed ||
        (m_alt_reach_recalc_timer.elapsed() >= m_processor_cfg->getIntValue(CFG_ALT_REACH_RECALC_PERIOD_MS)
         &&
         (qAbs(ap_diff_alt - m_last_toc_eod_ap_diff_alt) > 100 ||
          qAbs(vs - m_last_toc_eod_vs_ftmin) > 50 ||
          qAbs(m_flightstatus->ground_speed_kts - m_last_toc_eod_ground_speed_kts) > 10)))
    {
#if DO_PROFILING
        CodeTimer timer;
        timer.Tick();
#endif
        normal_route.altReachWpt() = Waypoint();
        
        if (qAbs(ap_diff_alt) > 500 && 
            normal_route.previousWaypoint() != 0 && normal_route.activeWaypoint() != 0)
        {
            double abs_alt_reach_distance_nm = 
                Navcalc::getDistanceToAltitude(ap_diff_alt, m_flightstatus->ground_speed_kts, vs);
            
            if (abs_alt_reach_distance_nm > 0.0)
            {
                // search for location to set TOC/EOD to
                
                if (abs_alt_reach_distance_nm < m_fmc_data.distanceToActiveWptNm())
                {
                    if (m_fmc_data.crossTrackDistanceNm() < 1.5)
                    {
                        normal_route.altReachWpt() = 
                            Navcalc::getIntermediateWaypoint(
                                *normal_route.activeWaypoint(),
                                *normal_route.previousWaypoint(),
                                m_fmc_data.distanceToActiveWptNm() - abs_alt_reach_distance_nm);
                    }
                }
                else
                {
                    abs_alt_reach_distance_nm -=
                        m_fmc_data.distanceToActiveWptNm() + qAbs(m_fmc_data.crossTrackDistanceNm());
                    
                    for(int index = normal_route.activeWaypointIndex(); 
                        index < normal_route.count()-1; ++index)
                    {
                        Waypoint* from_wpt = normal_route.waypoint(index);
                        MYASSERT(from_wpt != 0);
                        Waypoint* to_wpt = normal_route.waypoint(index+1);
                        MYASSERT(to_wpt != 0);
                        RouteData& route_data = normal_route.routeData(index);
                        
                        if (abs_alt_reach_distance_nm < route_data.m_dist_to_next_wpt_nm)
                        {
                            normal_route.altReachWpt() = 
                                Navcalc::getIntermediateWaypoint(*from_wpt, *to_wpt, abs_alt_reach_distance_nm);
                            break;
                        }
                        
                        abs_alt_reach_distance_nm -= route_data.m_dist_to_next_wpt_nm;
                    }
                }
            }
            
            if (normal_route.altReachWpt().isValid())
            {
                m_projection->convertLatLonToXY(normal_route.altReachWpt());
                
                if (ap_diff_alt > 0)
                    normal_route.altReachWpt().setFlag(Waypoint::FLAG_TOP_OF_CLIMB);
                else
                    normal_route.altReachWpt().setFlag(Waypoint::FLAG_END_OF_DESCENT);
            }        
        }

        m_alt_reach_recalc_timer.start();
        m_last_toc_eod_ground_speed_kts = m_flightstatus->ground_speed_kts;
        m_last_toc_eod_vs_ftmin = vs;
        m_last_toc_eod_ap_diff_alt = ap_diff_alt;

#if DO_PROFILING
        Logger::log(QString("FMCProcessor:alt_reach=%1").arg(timer.Tock()));
#endif
    }

    //----- calc descent estimate and TOD

  	if (m_data_changed || 
        m_descent_estimate_recalc_timer.elapsed() >= m_processor_cfg->getIntValue(CFG_DESCENT_ESTIMATE_RECALC_PERIOD_MS))
    {
        //TODO for testing
        m_next_wpt_with_alt_constraint_index = -1;
        
        for(int index = normal_route.activeWaypointIndex(); index < normal_route.count(); ++index)
        {
            Waypoint* wpt = normal_route.waypoint(index);
            
            if (wpt->restrictions().altitudeRestrictionFt() > 0 &&
                (wpt->restrictions().altitudeEqualRestriction() || wpt->restrictions().altitudeSmallerRestriction()))
            {
                m_next_wpt_with_alt_constraint_index = index;
                break;
            }
        }

        const Waypoint* next_wpt_with_alt_constraint = normal_route.waypoint(m_next_wpt_with_alt_constraint_index);
        if (next_wpt_with_alt_constraint != 0 && 
            ( next_wpt_with_alt_constraint->restrictions().altitudeRestrictionFt() <= 0 ||
              next_wpt_with_alt_constraint->restrictions().altitudeGreaterRestriction())) 
        {
            m_next_wpt_with_alt_constraint_index = 0;
            next_wpt_with_alt_constraint = 0;
        }
        
        //-----

        // locate TOD

        //TODO take wind into account!

        //TODO for testing
        //TODO only calc when input data changed!

        double distance_to_tod_nm = 0.0;
        bool found_next_wpt_with_alt_constraint = false;
        
        if (next_wpt_with_alt_constraint != 0 && 
            next_wpt_with_alt_constraint->restrictions().altitudeRestrictionFt() < altimeter_readout)
        {
            normal_route.todWpt() = Waypoint();
            m_fmc_data.setDistanceToTODNm(0.0);

            double altitude_to_loose = altimeter_readout - next_wpt_with_alt_constraint->restrictions().altitudeRestrictionFt();

            double descend_distance_nm = 
                5.0 + (fabs(altitude_to_loose) * m_processor_cfg->getDoubleValue(CFG_ALT_DIFF_TO_TOD_EST_FACTOR));

            if (vs > -250 &&
                altitude_to_loose >= 500 &&
                normal_route.previousWaypointIndex() >= 0)
            {
                for(int index = m_next_wpt_with_alt_constraint_index-1;
                    index >= qMax(0, normal_route.previousWaypointIndex()); --index)
                {
                    Waypoint* from_wpt = normal_route.waypoint(index);
                    if (from_wpt == 0) break;
                    RouteData& route_data = normal_route.routeData(index);

                    Waypoint* to_wpt = normal_route.waypoint(index+1);
                    if (to_wpt == 0) break;
                    
                    if (!found_next_wpt_with_alt_constraint)
                    {
                        if (descend_distance_nm > 0.0 && 
                            descend_distance_nm < route_data.m_dist_to_next_wpt_nm)
                        {
                            normal_route.todWpt() = Navcalc::getIntermediateWaypoint(*to_wpt, *from_wpt, descend_distance_nm);
                            m_projection->convertLatLonToXY(normal_route.todWpt());
                            normal_route.todWpt().setFlag(Waypoint::FLAG_TOP_OF_DESCENT);
                            found_next_wpt_with_alt_constraint = true;

                            // set distance to TOD
                            if (index == normal_route.previousWaypointIndex())
                                distance_to_tod_nm = m_fmc_data.distanceToActiveWptNm() - descend_distance_nm;
                            else
                                distance_to_tod_nm = route_data.m_dist_to_next_wpt_nm - descend_distance_nm;
                        }
                        
                        descend_distance_nm -= route_data.m_dist_to_next_wpt_nm;
                    }
                    else
                    {
                        // set distance to TOD
                        if (index == normal_route.previousWaypointIndex())
                            distance_to_tod_nm += m_fmc_data.distanceToActiveWptNm();
                        else
                            distance_to_tod_nm += route_data.m_dist_to_next_wpt_nm;
                    }
                }

                //TODO if TOD wpt not found -> unable to meet contraint!!
            }
        }

        // cruise descent

        double altitude_to_loose = altimeter_readout;
        const Airport* destination_airport = normal_route.destinationAirport();
        if (destination_airport != 0) altitude_to_loose -= destination_airport->elevationFt();
            
        double descend_distance_nm = 
            fabs(altitude_to_loose) * m_processor_cfg->getDoubleValue(CFG_ALT_DIFF_TO_TOD_EST_FACTOR);

        if (!found_next_wpt_with_alt_constraint &&
            (m_data_changed || qAbs(m_last_descend_distance_nm - descend_distance_nm) > 2 || vs <= -250.0))
        {
            normal_route.todWpt() = Waypoint();

            if (vs > -250 &&
                altitude_to_loose >= 5000 &&
                (normal_route.cruiseFl() <= 0 ||
                 qAbs(altimeter_readout - (normal_route.cruiseFl()*100)) < 5000) &&
                normal_route.previousWaypointIndex() >= 0)
            {
                // start at the last waypoint of the route
                int index = (destination_airport != 0) ?
                            (((int)normal_route.destinationAirportIndex()) - 1) :
                            (((int)normal_route.count()) - 2);
                
                bool tod_found = false;

                for(; index >= qMax(0, normal_route.previousWaypointIndex()); --index)
                {
                    Waypoint* to_wpt = normal_route.waypoint(index+1);
                    if (to_wpt == 0) break;

                    Waypoint* from_wpt = normal_route.waypoint(index);
                    if (from_wpt == 0) break;
                    
                    RouteData& route_data = normal_route.routeData(index);
                    
                    if (!tod_found)
                    {
                        if (descend_distance_nm > 0.0 &&
                            descend_distance_nm < route_data.m_dist_to_next_wpt_nm)
                        {
                            normal_route.todWpt() = Navcalc::getIntermediateWaypoint(*to_wpt, *from_wpt, descend_distance_nm);
                            m_projection->convertLatLonToXY(normal_route.todWpt());
                            normal_route.todWpt().setFlag(Waypoint::FLAG_TOP_OF_DESCENT);
                            tod_found = true;

                            // set distance to TOD
                            if (index == normal_route.previousWaypointIndex())
                                distance_to_tod_nm = m_fmc_data.distanceToActiveWptNm() - descend_distance_nm;
                            else
                                distance_to_tod_nm = route_data.m_dist_to_next_wpt_nm - descend_distance_nm;
                        }
                        
                        descend_distance_nm -= route_data.m_dist_to_next_wpt_nm;
                    }
                    else
                    {
                        // set distance to TOD
                        if (index == normal_route.previousWaypointIndex())
                            distance_to_tod_nm += m_fmc_data.distanceToActiveWptNm();
                        else
                            distance_to_tod_nm += route_data.m_dist_to_next_wpt_nm;
                    }
                }
                
                m_last_descend_distance_nm = descend_distance_nm;
            }
        }

        m_fmc_data.setDistanceToTODNm(distance_to_tod_nm);
        m_descent_estimate_recalc_timer.start();
    }

    //----- recalculate the LAT/LON -> X/Y projection for all waypoints

    // we do not recalc the projection at each cycle, instead we use a current
    // position correction X/Y calculation, see drawRouteNormalMode() in FMCNavdisplayStyleA/B

#if DO_PROFILING
    CodeTimer timer;
    timer.Tick();
#endif

    double dist_to_projection_center = 0.0;
    Waypoint view_center;

    // the view wpt is e.g. used when in map mode and the wpt to view differs from the current position
    if (m_fmc_control->currentNDMode(true) == CFG_ND_DISPLAY_MODE_NAV_PLAN &&
        normal_route.viewWptIndex() >= 0 && normal_route.viewWpt() != 0)
    {
        //TODO 
        // split the projection for left/right ND in the future -
        // one ND might be set to PLAN and the other one not.
        // For now only the left ND is taken into account.
        view_center = *normal_route.viewWpt();
    }
    else
    {
        view_center = m_flightstatus->current_position_smoothed;
    }

    dist_to_projection_center = 
        Navcalc::getDistBetweenWaypoints(m_projection->getCenter(), view_center);
    
    if (m_data_changed || dist_to_projection_center >= m_processor_cfg->getDoubleValue(CFG_PROJECTION_RECALC_DISTANCE_NM))
    {
        //Logger::log(QString("FMCProcessor:refresh: projection recalc, dist=%1nm").arg(dist_to_projection_center));

        m_projection->setScaleAndCenter(view_center, 1, 1);

        // trigger processing

        m_project_recalc_index_route_normal = 0;
        m_project_recalc_index_route_temporary = 0;
        m_project_recalc_index_route_alternate = 0;
        m_project_recalc_index_route_secondary = 0;

        m_project_recalc_airports = 0;
        m_project_recalc_vors = 0;
        m_project_recalc_ndbs = 0;
        m_project_recalc_geo = 0;
    }

    //----- recalc the route projections in steps

    if (m_project_recalc_index_route_normal >= 0)
    {
        if (m_normal_route_view_wpt_index != normal_route.viewWptIndex())
        {
            normal_route.calcProjection(*m_projection);
            m_project_recalc_index_route_normal = -1;
        }
        else
        {
            if (normal_route.calcProjection(
                    *m_projection, 
                    m_project_recalc_index_route_normal,
                    m_project_recalc_index_route_normal + PROJECTION_RECALC_INC))
            {
                m_project_recalc_index_route_normal += PROJECTION_RECALC_INC + 1;
            }
            else
            {
                m_project_recalc_index_route_normal = -1;
            }
        }
    }

    if (m_project_recalc_index_route_alternate >= 0)
    {
        if (m_fmc_data.alternateRoute().calcProjection(
                *m_projection, 
                m_project_recalc_index_route_alternate,
                m_project_recalc_index_route_alternate + PROJECTION_RECALC_INC))
        {
            m_project_recalc_index_route_alternate += PROJECTION_RECALC_INC + 1;
        }
        else
        {
            m_project_recalc_index_route_alternate = -1;
        }
    }

    if (m_project_recalc_index_route_secondary >= 0)
    {
        if (m_fmc_data.secondaryRoute().calcProjection(
                *m_projection, 
                m_project_recalc_index_route_secondary,
                m_project_recalc_index_route_secondary + PROJECTION_RECALC_INC))
        {
            m_project_recalc_index_route_secondary += PROJECTION_RECALC_INC + 1;
        }
        else
        {
            m_project_recalc_index_route_secondary = -1;
        }
    }

    if (m_project_recalc_index_route_temporary >= 0)
    {
        if (m_fmc_data.temporaryRoute().calcProjection(
                *m_projection, 
                m_project_recalc_index_route_temporary,
                m_project_recalc_index_route_temporary + PROJECTION_RECALC_INC))
        {
            m_project_recalc_index_route_temporary += PROJECTION_RECALC_INC + 1;
        }
        else
        {
            m_project_recalc_index_route_temporary = -1;
        }
    }

    //----- recalc other stuff in steps

    // always calc the stuff below relative to the current position
    view_center = m_flightstatus->current_position_smoothed;

    if (m_project_recalc_airports == 0)
    {
        // process surrounding airports

        m_fmc_data.surroundingAirportList().clear();
        m_navdata->getAirportListByCoordinates(
            view_center, 2, 
            m_processor_cfg->getIntValue(CFG_MAX_SURROUNDING_AIRPORT_DIST_NM),
            m_fmc_data.surroundingAirportList());
        
        WaypointPtrListIterator airport_iter(m_fmc_data.surroundingAirportList());
        while(airport_iter.hasNext()) m_projection->convertLatLonToXY(*airport_iter.next());
    }

    if (m_project_recalc_vors == 0)
    {
        // process surrounding vors
        
        m_fmc_data.surroundingVorList().clear();        
        m_navdata->getVorListByCoordinates(view_center, 2, 
                                           m_processor_cfg->getIntValue(CFG_MAX_SURROUNDING_VOR_DIST_NM),
                                           m_fmc_data.surroundingVorList());
        
        WaypointPtrListIterator vor_iter(m_fmc_data.surroundingVorList());
        while(vor_iter.hasNext()) m_projection->convertLatLonToXY(*vor_iter.next());
    }

    if (m_project_recalc_ndbs == 0)
    {
        // process surrounding ndbs

        m_fmc_data.surroundingNdbList().clear();
        m_navdata->getNdbListByCoordinates(view_center, 2, 
                                           m_processor_cfg->getIntValue(CFG_MAX_SURROUNDING_NDB_DIST_NM),
                                           m_fmc_data.surroundingNdbList());
        
        WaypointPtrListIterator ndb_iter(m_fmc_data.surroundingNdbList());
        while(ndb_iter.hasNext()) m_projection->convertLatLonToXY(*ndb_iter.next());
    }

    if (m_project_recalc_geo == 0)
    {
        // recalc GEO data

        m_fmc_control->geoData().updateActiveRouteList(
            view_center, m_processor_cfg->getIntValue(CFG_MAX_SURROUNDING_GEO_DIST_NM));
        m_fmc_control->geoData().calcProjectionActiveRoute(*m_projection);
    }

    if (m_project_recalc_airports >= 0) --m_project_recalc_airports;
    if (m_project_recalc_vors >= 0) --m_project_recalc_vors;
    if (m_project_recalc_ndbs >= 0) --m_project_recalc_ndbs;
    if (m_project_recalc_geo >= 0) --m_project_recalc_geo;

#if DO_PROFILING
    Logger::log(QString("FMCProcessor:proj_recalc=%1").arg(timer.Tock()));
#endif

    //----- always process tuned VOR1/2 locations

    if (!m_flightstatus->nav1.id().isEmpty()) m_projection->convertLatLonToXY(m_flightstatus->nav1);
    if (!m_flightstatus->nav2.id().isEmpty()) m_projection->convertLatLonToXY(m_flightstatus->nav2);

    m_data_changed = false;
    m_normal_route_view_wpt_index = normal_route.viewWptIndex();

    emit signalTimeUsed("PR", overall_timer.elapsed());

//     Logger::log(QString("FMCProcessor: act=%1/%2nm/%3h, prev=%4/%5nm, xtrack=%6").
//                 arg(active_wpt->id()).
//                 arg(m_fmc_data.distanceToActiveWptNm()).
//                 arg(m_fmc_data.hoursToActiveWpt()).
//                 arg((prev_wpt != 0 ? prev_wpt->id() : "")).
//                 arg(m_fmc_data.distanceFromPreviousWptNm()).
//                 arg(m_fmc_data.crossTrackDistanceNm()));
}

/////////////////////////////////////////////////////////////////////////////

// End of file
