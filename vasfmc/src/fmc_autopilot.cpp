//////////////////////////////////////////////////////////////////////////////
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

/*! \file    fmc_autopilot.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "logger.h"
#include "navcalc.h"
#include "waypoint.h"
#include "fmc_control.h"
#include "fmc_data.h"
#include "flightstatus.h"
#include "fsaccess.h"
#include "flight_mode_tracker.h"
#include "aircraft_data.h"

#include "defines.h"
#include "fmc_autopilot.h"
#include "fmc_autothrottle.h"

#include "fly_by_wire.h"

#define ALT_CAPTURE_VS_INHIBIT 500
#define ALT_CAPTURE_MARGIN 50
#define ALT_CRZ_MARGIN 50

/////////////////////////////////////////////////////////////////////////////

FMCAutopilot::FMCAutopilot(ConfigWidgetProvider* config_widget_provider,
                           Config* main_config, 
                           const QString& autopilot_cfg_filename,
                           FMCControl* fmc_control) :
    m_main_config(main_config), m_values_initialized(false), m_fmc_control(fmc_control), 
    m_fmc_data(fmc_control->fmcData()), m_flightstatus(fmc_control->flightStatus()), m_ils_mode(ILS_MODE_NONE),
    m_lateral_mode_active(LATERAL_MODE_NONE), m_lateral_mode_armed(LATERAL_MODE_NONE),
    m_vertical_mode_active(VERTICAL_MODE_NONE), m_vertical_mode_armed(VERTICAL_MODE_NONE),
    m_flightpath_angle(0), m_vertical_speed(0), m_mode_fl_change_active(false), m_fl_change_cmd_vs(0.0),
    m_do_set_vs_to_ap(true), m_do_reset_after_landing(true),
    m_takeoff_active_lateral(false), m_takeoff_lateral_mode_armed(LATERAL_MODE_NONE),
    m_takeoff_active_vertical(false), m_takeoff_vertical_mode_armed(VERTICAL_MODE_NONE), 
    m_takeoff_lateral_target_track(0.0), m_takeoff_vertical_speed_hold_kts(0.0)
{
    MYASSERT(config_widget_provider != 0);
    MYASSERT(m_main_config != 0);
    MYASSERT(m_fmc_control != 0);
    MYASSERT(m_flightstatus != 0);

    // setup config
    
    m_autopilot_config = new Config(autopilot_cfg_filename);
    MYASSERT(m_autopilot_config != 0);
    setupDefaultConfig();
    m_autopilot_config->loadfromFile();
    m_autopilot_config->saveToFile();
    config_widget_provider->registerConfigWidget("Autopilot", m_autopilot_config);

    // mode stuff

    m_lateral_mode_calc_timer = QTime::currentTime().addSecs(-60);
    m_lateral_mode_active_changed_time = QTime::currentTime().addSecs(-60);

    m_vertical_mode_calc_timer = QTime::currentTime().addSecs(-60);
    m_vertical_mode_active_changed_time = QTime::currentTime().addSecs(-60);

    // takeoff stuff

    m_lateral_fd_source_reset_timer.setSingleShot(true);
    m_vertical_fd_source_reset_timer.setSingleShot(true);
    MYASSERT(connect(&m_lateral_fd_source_reset_timer, SIGNAL(timeout()),
                     this, SLOT(slotSwitchLateralFlightDirectorToExternalSource())));
    MYASSERT(connect(&m_vertical_fd_source_reset_timer, SIGNAL(timeout()),
                     this, SLOT(slotSwitchVerticalFlightDirectorToExternalSource())));
}

/////////////////////////////////////////////////////////////////////////////

FMCAutopilot::~FMCAutopilot() 
{
    m_autopilot_config->saveToFile();
    delete m_autopilot_config;
};

/////////////////////////////////////////////////////////////////////////////

void FMCAutopilot::setupDefaultConfig()
{
    m_autopilot_config->setValue(CFG_DIRECT_TURNDIST_FACTOR, 2.0);
    m_autopilot_config->setValue(CFG_INTERCEPT_ANGLE_FACTOR, 90.0);
    m_autopilot_config->setValue(CFG_MAX_INTERCEPT_ANGLE, 80.0);
    m_autopilot_config->setValue(CFG_HOLDING_OVERFLY_TURN_DIST_NM, 0.3);
    m_autopilot_config->setValue(CFG_TURN_DIST_HDG_DIFF_FACTOR, 90.0);
    m_autopilot_config->setValue(CFG_INTERCEPT_HDG_DIFF_MULTIPLY_FACTOR, 0.5);
    m_autopilot_config->setValue(CFG_INTERCEPT_BOOST_DIST_NM, 0.01);

    m_autopilot_config->setValue(CFG_HOLDING_HDG_DIFF_LEG_DETECTION, 18.0);
    m_autopilot_config->setValue(CFG_HOLDING_TEARDROP_ANGLE_OFFSET, 45.0);
    m_autopilot_config->setValue(CFG_HOLDING_ENTRY_OUTBOUND_TIME_MIN, 1.0);
    m_autopilot_config->setValue(CFG_HOLDING_INBD_REF_TRACK_WPT_DIST_NM, 30.0);

    m_autopilot_config->setValue(CFG_AP_NAV_COUPLE, 0);
    m_autopilot_config->setValue(CFG_AP_MODE_FLIGHTPATH_ENABLED, 0);
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutopilot::initValues()
{
    if (!m_flightstatus->isValid()) return;
    m_values_initialized = true;

    setFlightPathAngleInternal(m_flightstatus->fpv_vertical.value()); 
    setVerticalSpeedInternal((int)m_flightstatus->smoothedVS());
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutopilot::slotRefresh()
{
    if (!m_flightstatus->isValid() || 
        m_flightstatus->slew || 
        m_flightstatus->paused) return;

    calculateFlightDirector();

    if (m_fmc_control->isFMCConnectModeSlave()) return;

    if (!m_values_initialized) initValues();

    // ILS mode processing

    if (!m_flightstatus->nav1.id().isEmpty() && m_flightstatus->nav1_has_loc)
    {
        m_ils_mode = 
            (m_flightstatus->ap_enabled && m_flightstatus->ap_app_lock && m_flightstatus->fd_active) ?
            ILS_MODE_CAT2 : ILS_MODE_CAT1;
    }
    else
    {
        m_ils_mode = ILS_MODE_NONE;
    }

    // FBW processing

    if (m_fmc_control->fbwEnabled())
    {
        m_fmc_control->bankController()->execBankRate(m_fmc_control->fsAccess());
        m_fmc_control->pitchController()->execPitchRate(m_fmc_control->fsAccess());
    }
    else
    {
        m_fmc_control->fsAccess().freeControlAxes();
        m_fmc_control->bankController()->reset();
        m_fmc_control->pitchController()->reset();
    }
    
    // flight director and autopilot checks

    if (m_flightstatus->ap_enabled && 
        m_flightstatus->radarAltitude() < 100 &&
        !m_fmc_control->flightModeTracker().isApproach() && 
        !m_fmc_control->flightModeTracker().isLanding())
    {
        m_fmc_control->fsAccess().setAPOnOff(false);
    }

    if (m_flightstatus->onground &&
        !m_flightstatus->isAtLeastOneEngineOn() &&
        m_flightstatus->fd_active)
    {
        m_fmc_control->fsAccess().setFDOnOff(false);
    }
    
    if (m_flightstatus->ground_speed_kts < 30 && m_fmc_control->flightModeTracker().wasLanding())
    {
        if (m_do_reset_after_landing)
        {
            m_do_reset_after_landing = false;

            if (m_flightstatus->ap_enabled) m_fmc_control->fsAccess().setAPOnOff(false);
            if (m_flightstatus->fd_active) m_fmc_control->fsAccess().setFDOnOff(false);
            m_fmc_control->fsAccess().setElevatorTrimPercent(0.0);
        }
    }
    else if (!m_flightstatus->onground)
    {
        m_do_reset_after_landing = true;
    }

    // calculate lateral & vertical AP

    calculateLateral();
    calculateLateralMode();

    calculateVertical();
    calculateVerticalMode();
}

/////////////////////////////////////////////////////////////////////////////

double FMCAutopilot::getInterceptHeadingToTrack(Waypoint& from_wpt, const Waypoint& to_wpt) const
{
    // TODO check for special cases (true hdg vs. track, etc.)

    double track_between_waypoints = 0.0;
    double track_from_to_current = 0.0;
    double dist_from_to_current = 0.0;

    // calculate the corrected track in respect to the progress on the current leg

    double cross_track_distance = Navcalc::getCrossTrackDistance(
        from_wpt, to_wpt, m_flightstatus->current_position_raw, 
        track_between_waypoints, track_from_to_current, dist_from_to_current);

    // if the wpt lies behind -> keep the track
    if (Navcalc::isWaypointBehind(m_fmc_data.trueTrackToActiveWpt(), m_flightstatus->smoothedTrueHeading()))
    {
        return track_between_waypoints;
    }

    // go direct if the last waypoint had an overfly restriction

    if (from_wpt.restrictions().hasOverflyRestriction())
    {
        return m_fmc_data.trueTrackToActiveWpt();
    }

    double overall_dist = Navcalc::getDistBetweenWaypoints(from_wpt, to_wpt);
    double progress = qMax(0.0, qMin(1.0, dist_from_to_current / overall_dist));

    double reverse_track_between_waypoints = Navcalc::trimHeading(180+Navcalc::getTrackBetweenWaypoints(to_wpt, from_wpt));
    double track_diff = reverse_track_between_waypoints - track_between_waypoints;
    double intermediate_track = track_between_waypoints + track_diff * progress;

    double ground_speed_boost_factor  = (1.0 + (m_flightstatus->ground_speed_kts / 250.0));

    // scale the boost
    ground_speed_boost_factor = m_flightstatus->ground_speed_kts * ((ground_speed_boost_factor - 0.5) / 500.0) + 0.5;
    
    double intercept_hdg = ((-cross_track_distance/ground_speed_boost_factor) * 
                            m_autopilot_config->getDoubleValue(CFG_INTERCEPT_ANGLE_FACTOR));

    double boost_factor = m_fmc_data.turnRadiusNm();
    if (fabs(cross_track_distance) <= m_autopilot_config->getDoubleValue(CFG_INTERCEPT_BOOST_DIST_NM))
    {
        boost_factor = 1.0 + (fabs(cross_track_distance) * (m_fmc_data.turnRadiusNm() - 1) / 
                              m_autopilot_config->getDoubleValue(CFG_INTERCEPT_BOOST_DIST_NM));
    }
    
    intercept_hdg /= boost_factor;

    if (intercept_hdg > 90.0) intercept_hdg = 90.0;
    else if (intercept_hdg < -90.0) intercept_hdg = -90.0;

    // if we are already on an heading smaller than the intercept heading within a
    // certain distance from the track to intercept, we keep that heading
    // until the intercept heading catches the current heading (inhibit slingering)
    
    double hdg_track_diff = Navcalc::getSignedHeadingDiff(intermediate_track, m_flightstatus->smoothedTrueHeading());
    
    if (fabs(hdg_track_diff) < 90.0 && 
        fabs(hdg_track_diff) > 2.0 && 
        dist_from_to_current <= m_fmc_data.turnRadiusNm()*2.0 &&
        (((intercept_hdg > 0.0 && hdg_track_diff > 0.0 && hdg_track_diff < intercept_hdg) ||
          (intercept_hdg < 0.0 && hdg_track_diff < 0.0 && hdg_track_diff > intercept_hdg))) &&
        !Navcalc::isWaypointBehind(Navcalc::trimHeading(track_from_to_current+180), m_flightstatus->smoothedTrueHeading()))
    {
        intercept_hdg = hdg_track_diff;
    }
    
    double result_hdg = Navcalc::trimHeading(intermediate_track + intercept_hdg);

//     Logger::log(QString("icept: %1 => %2 track=%3, xtrack=%4, icept=%5, result=%6").
//                 arg(from_wpt.id()).arg(to_wpt.id()).arg(track_between_wpts).
//                 arg(cross_track_distance).arg(intercept_hdg).arg(result_hdg));
    
    if (from_wpt.restrictions().leftTurnRestriction())
    {
        // clear the turn restriction if we are close to the target heading in
        // order to avoid never ending circling 
        if (Navcalc::getAbsHeadingDiff(m_flightstatus->smoothedTrueHeading(), intermediate_track) < 45.0)
            from_wpt.restrictions().setTurnRestriction(WaypointRestrictions::RESTRICTION_TURN_NONE);

        return Navcalc::getTurnHeading(m_flightstatus->smoothedTrueHeading(), result_hdg, true);
    }
    
    if (from_wpt.restrictions().rightTurnRestriction())
    {
        // clear the turn restriction if we are close to the target heading in
        // order to avoid never ending circling 
        if (Navcalc::getAbsHeadingDiff(m_flightstatus->smoothedTrueHeading(), intermediate_track) < 45.0)
            from_wpt.restrictions().setTurnRestriction(WaypointRestrictions::RESTRICTION_TURN_NONE);
        
        return Navcalc::getTurnHeading(m_flightstatus->smoothedTrueHeading(), result_hdg, false);
    }

    // we go direct if the distance is too short for turning
    if (m_fmc_data.distanceToActiveWptNm() < 
        Navcalc::getPreTurnDistance(
            0.1, m_fmc_data.turnRadiusNm(), (int)m_flightstatus->ground_speed_kts,
            m_flightstatus->smoothedTrueHeading(), m_fmc_data.trueTrackToActiveWpt()))
        return m_fmc_data.trueTrackToActiveWpt();

    return result_hdg;
}

/////////////////////////////////////////////////////////////////////////////

double FMCAutopilot::getHeadingForHolding(Waypoint& to_wpt) const
{
    double true_track_result = -10;

    Holding& holding = to_wpt.holding();
    MYASSERT(holding.holdLegLengthNm() == 0.0);
    MYASSERT(holding.holdLegLengthMin() > 0.0);
    MYASSERT(holding.isInboundFixTrack());

    double holding_track = Navcalc::trimHeading(holding.holdingTrack() + m_flightstatus->magvar);
    double holding_out_track = Navcalc::trimHeading(holding_track + 180.0);
    
    double track_to_wpt = 0.0;
    double dist_to_wpt = 0.0;
    
    MYASSERT(Navcalc::getDistAndTrackBetweenWaypoints(
                 m_flightstatus->current_position_raw, to_wpt, dist_to_wpt, track_to_wpt));

    // Calc the WCA for the case we would fly the holding leg 1.
    // We always use this WCA to correct other holding legs for wind.
    double holding_track_wind_corr_angle = 0.0;
    double gs_dummy = 0.0;
    Navcalc::getWindCorrAngleAndGS(m_flightstatus->tas, holding_track,
                                   m_flightstatus->wind_speed_kts, m_flightstatus->wind_dir_deg_true,
                                   holding_track_wind_corr_angle, gs_dummy);
    
    // process the holding depending on its status

    switch(holding.status())
    {
        case(Holding::STATUS_INACTIVE): {
           
            // calc inbd reference track waypoint

            if (holding.inbdTrackLat() == 0.0 && holding.inbdTrackLon() == 0.0)
            {
                Waypoint to_fix_ref_track_wpt =
                    Navcalc::getPBDWaypoint(
                        to_wpt, (int)(holding_out_track+0.5),
                        m_autopilot_config->getDoubleValue(CFG_HOLDING_INBD_REF_TRACK_WPT_DIST_NM),
                        Declination::globalDeclination());
                
                holding.setInbdTrackLatLon(to_fix_ref_track_wpt.lat(), to_fix_ref_track_wpt.lon());
            }

            // determine holding entry

            double track_to_airplane = Navcalc::trimHeading(track_to_wpt + 180.0);
            double diff_track = track_to_airplane - holding_track;
            while(diff_track < -180.0) diff_track += 360.0;
            while(diff_track > 180.0) diff_track -= 360.0;

//             printf("entry detection: t2a:%.2lf ht:%.2lf dt:%.2lf\n",
//                    track_to_airplane, holding_track, diff_track);
//             fflush(stdout);

            if (holding.isInboundFixTrack())
            {
                if (!holding.isLeftHolding())
                {
                    if (diff_track >= 0.0 && diff_track < 110.0)
                        holding.setEntryType(Holding::ENTRY_PARALLEL);
                    else if (diff_track < 0.0 && diff_track > -70.0)
                        holding.setEntryType(Holding::ENTRY_TEARDROP);
                    else if (diff_track >= 110.0 || diff_track <= -150.0)
                        holding.setEntryType(Holding::ENTRY_DIRECT_DCT);
                    else
                        holding.setEntryType(Holding::ENTRY_DIRECT_INTERCEPT);
                }
                else
                {
                    if (diff_track >= 0.0 && diff_track < 70.0)
                        holding.setEntryType(Holding::ENTRY_TEARDROP);
                    else if (diff_track < 0.0 && diff_track > -110.0)
                        holding.setEntryType(Holding::ENTRY_PARALLEL);
                    else if (diff_track <= -110.0 || diff_track >= 150.0)
                        holding.setEntryType(Holding::ENTRY_DIRECT_DCT);
                    else
                        holding.setEntryType(Holding::ENTRY_DIRECT_INTERCEPT);
                }
            }
            else
            {
                MYASSERT(0);
            }

            holding.resetTimer();
            holding.setStatus(Holding::STATUS_ENTRY_TO_FIX);
            true_track_result = track_to_wpt;

//             printf("INACTIVE: %s\n", holding.toString().toLatin1().data());
//             fflush(stdout);
            
            break;
        }

        case(Holding::STATUS_ENTRY_TO_FIX): {

            // calc heading of next status

            double hdg_for_next_status = holding_track;

            if (holding.entryType() == Holding::ENTRY_TEARDROP)
            {
                if (holding.isLeftHolding())
                    hdg_for_next_status = Navcalc::trimHeading(
                        holding_out_track + 
                        m_autopilot_config->getDoubleValue(CFG_HOLDING_TEARDROP_ANGLE_OFFSET));
                else
                    hdg_for_next_status = Navcalc::trimHeading(
                        holding_out_track - 
                        m_autopilot_config->getDoubleValue(CFG_HOLDING_TEARDROP_ANGLE_OFFSET));
            }
            else if (holding.entryType() == Holding::ENTRY_PARALLEL)
            {
                hdg_for_next_status = holding_out_track;
            }

            double turn_dist = Navcalc::getPreTurnDistance(
				m_autopilot_config->getDoubleValue(CFG_HOLDING_OVERFLY_TURN_DIST_NM), m_fmc_data.turnRadiusNm(), 
				(int)m_flightstatus->ground_speed_kts, m_flightstatus->smoothedTrueHeading(), hdg_for_next_status);

//            printf("ENTRY_TO_FIX: ");

            // process status

            //check for special intercept for DIRECT ENTRY
            if (holding.entryType() == Holding::ENTRY_DIRECT_INTERCEPT)
            {
                if (holding.isLeftHolding())
                {
                    if (!Navcalc::isAfterAbeamFix(track_to_wpt, holding_track, 180.0))
                    {
                        // we did not cross the fix yet
                        if (dist_to_wpt > m_autopilot_config->getDoubleValue(CFG_HOLDING_OVERFLY_TURN_DIST_NM))
                        {
//                            printf(" not abeam - to fix");
                            holding.setTrueTargetTrack(track_to_wpt);
                        }
                        else
                        {
//                            printf(" not abeam - right angle");
                            holding.setTrueTargetTrack(
                                Navcalc::trimHeading(holding_track-90.0));
                        }
                    }
                    else
                    {
                        if (dist_to_wpt < m_fmc_data.turnRadiusNm()*0.5)
                        { 
                            // we crossed the fix and wait for the outbound leg
                            //printf(" abeam - intercept dist:%.02lf", dist_to_wpt);
                            
                            holding.setTrueTargetTrack(
                                Navcalc::trimHeading(holding_track-90.0));
                        }
                        else
                        {
                            // turn to outound leg
//                            printf(" abeam - turn");
                            holding.setTrueTargetTrack(holding_out_track);
                            holding.setStatus(Holding::STATUS_INSIDE_TURN_1);
                            holding.setTurnToTargetTrackWithLeftTurn(
                                holding.isLeftHolding());
                        }
                    }
                }
                else //--- right holding
                {
                    if (!Navcalc::isAfterAbeamFix(track_to_wpt, holding_out_track, 180.0))
                    {
                        // we did not cross the fix yet
                        if (dist_to_wpt > m_autopilot_config->getDoubleValue(CFG_HOLDING_OVERFLY_TURN_DIST_NM))
                        {
//                            printf(" not abeam - to fix");
                            holding.setTrueTargetTrack(track_to_wpt);
                        }
                        else
                        {
//                            printf(" not abeam - right angle");
                            holding.setTrueTargetTrack(
                                Navcalc::trimHeading(holding_track+90.0));
                        }
                    }
                    else
                    {
                        if (dist_to_wpt < m_fmc_data.turnRadiusNm()*0.5)
                        { 
                            // we crossed the fix and wait for the outbound leg
                            //printf(" abeam - intercept dist:%.02lf", dist_to_wpt);
                            
                            holding.setTrueTargetTrack(
                                Navcalc::trimHeading(holding_track+90.0));
                        }
                        else
                        {
                            // turn to outound leg
//                            printf(" abeam - turn");
                            holding.setTrueTargetTrack(holding_out_track);
                            holding.setStatus(Holding::STATUS_INSIDE_TURN_1);
                            holding.setTurnToTargetTrackWithLeftTurn(
                                !holding.isLeftHolding());
                        }
                    }
                }

                true_track_result = holding.trueTargetTrack();
            }
            else if (dist_to_wpt > turn_dist) // ! DIRECT_INTERCEPT
            {
                // go direct to the fix
                true_track_result = track_to_wpt;
            }
            else 
            {
                // change to entry when we reached the fix

                switch(holding.entryType())
                {
                    case(Holding::ENTRY_DIRECT_DCT): {

                        holding.setTrueTargetTrack(hdg_for_next_status);
                        holding.setStatus(Holding::STATUS_INSIDE_TURN_1);
                        holding.setTurnToTargetTrackWithLeftTurn(holding.isLeftHolding());
                        break;
                    }
                    case(Holding::ENTRY_TEARDROP): {

                        holding.setTrueTargetTrack(hdg_for_next_status);
                        holding.setStatus(Holding::STATUS_ENTRY_LEG_1);
                        holding.setTurnToTargetTrackWithLeftTurn(true);
                        break;
                    }
                    case(Holding::ENTRY_PARALLEL): {

                        holding.setTrueTargetTrack(hdg_for_next_status);
                        holding.setStatus(Holding::STATUS_ENTRY_LEG_1);
                        holding.setTurnToTargetTrackWithLeftTurn(!holding.isLeftHolding());
                        break;
                    }
                    default: {
                        MYASSERT(0);
                    }
     
                }

                true_track_result = Navcalc::getTurnHeading(
                    m_flightstatus->smoothedTrueHeading(),
                    holding.trueTargetTrack(),
                    holding.turnToTargetTrackWithLeftTurn());
                
                holding.resetTimer();
            }

//             printf(" turndist:%.2lf : %s\n", 
//                    turn_dist, holding.toString().toLatin1().data());
//             fflush(stdout);

            break;
        }

            //----- entering holding

        case(Holding::STATUS_ENTRY_LEG_1): {

            double speed_diff = m_flightstatus->ground_speed_kts - m_flightstatus->tas;

            double corrected_outbound_time = 
            ((m_autopilot_config->getDoubleValue(CFG_HOLDING_ENTRY_OUTBOUND_TIME_MIN) *
              (m_flightstatus->tas - speed_diff)) - (speed_diff * 1.5)) / m_flightstatus->ground_speed_kts;
            
//             printf("corrected time:%.02lf -> %.02lf ", 
//                    holding.holdLegLengthMin(), corrected_outbound_time);

            if ((holding.timerSeconds()/60.0) < corrected_outbound_time)
            {
                // continue leg
                true_track_result = holding.trueTargetTrack();
                
                // add wind correction for the next turn
                if (holding.entryType() == Holding::ENTRY_TEARDROP)
                    true_track_result -= holding_track_wind_corr_angle;

                // reset timer until we passed abeam the fix
                switch(holding.entryType())
                {
                    case(Holding::ENTRY_TEARDROP): {
                        if (!Navcalc::isAfterAbeamFix(track_to_wpt, holding_track-90.0, 180.0))
                            holding.resetTimer();
                        break;
                    }
                    case(Holding::ENTRY_PARALLEL): {
                        if (!Navcalc::isAfterAbeamFix(track_to_wpt, holding_track-45.0, 90.0))
                            holding.resetTimer();
                        break;
                    }
                    default: {
                        MYASSERT(0);
                    }
                }
                
//                printf("ENTRY_L1: ");
            }
            else if (holding.entryType() == Holding::ENTRY_TEARDROP)
            {
                // the leg is over - turn to fix
                
//                printf("ENTRY_L1: turn to fix tear: ");
                
                holding.resetTimer();
                holding.setStatus(Holding::STATUS_INSIDE_TURN_2);
                
                true_track_result = Navcalc::getTurnHeading(
                    m_flightstatus->smoothedTrueHeading(), holding_track,
                    holding.isLeftHolding());
            }            
            else if (holding.entryType() == Holding::ENTRY_PARALLEL)
            {
                // the leg is over - turn to fix
                
//                printf("ENTRY_L1: turn to fix para: ");
                
                holding.resetTimer();
                holding.setStatus(Holding::STATUS_ENTRY_LEG_2);
                
                true_track_result = Navcalc::getTurnHeading(
                    m_flightstatus->smoothedTrueHeading(), track_to_wpt,
                    !holding.isLeftHolding());
            }            
            else
            {
                MYASSERT(0);
            }
            
//             printf("%s\n", holding.toString().toLatin1().data());
//             fflush(stdout);
            break;
        }

        case(Holding::STATUS_ENTRY_LEG_2): {

            MYASSERT(holding.entryType() == Holding::ENTRY_PARALLEL);

            double turn_dist = 
                Navcalc::getPreTurnDistance(
					m_autopilot_config->getDoubleValue(CFG_HOLDING_OVERFLY_TURN_DIST_NM), m_fmc_data.turnRadiusNm(), 
                    (int)m_flightstatus->ground_speed_kts, 
                    m_flightstatus->smoothedTrueHeading() + m_flightstatus->wind_correction_angle_deg,
                    holding_track);

            double heading_diff = Navcalc::getAbsHeadingDiff(
				m_flightstatus->smoothedTrueHeading(), track_to_wpt + m_flightstatus->wind_correction_angle_deg);

            if (dist_to_wpt > turn_dist)
            {
                if (heading_diff < m_autopilot_config->getDoubleValue(CFG_MAX_INTERCEPT_ANGLE))
                {
                    // go direct to the fix
//                    printf("ENTRY_L2: to fix (dct): ");
                    true_track_result = track_to_wpt;
                }
                else
                {
                    // calc hdg to the fix                    
//                    printf("ENTRY_L2: to fix: ");
                    
                    true_track_result = Navcalc::getTurnHeading(
                        m_flightstatus->smoothedTrueHeading(), track_to_wpt,
                        !holding.isLeftHolding());
                }
            }
            else
            {
                // change to first holding turn
//                printf("ENTRY_L2: exit: ");

                holding.resetTimer();
                holding.setStatus(Holding::STATUS_INSIDE_TURN_1);
                    
                true_track_result = Navcalc::getTurnHeading(
                    m_flightstatus->smoothedTrueHeading(), holding_out_track,
                    holding.isLeftHolding());
            }
            
//             printf("turndist:%.2lf : %s\n", 
//                    turn_dist, holding.toString().toLatin1().data());
//             fflush(stdout);
            
            break;
        }
            
            //----- inside holding

        case(Holding::STATUS_INSIDE_LEG_1): {
            
            if (holding.exitHolding())
            {
//                printf("INSIDE_L1: to fix - direct (exit) ");
                true_track_result = track_to_wpt;
            }
            else if (dist_to_wpt <= m_autopilot_config->getDoubleValue(CFG_HOLDING_OVERFLY_TURN_DIST_NM)) 
            {
                // we reached the fix - turn to 2nd leg

//                printf("INSIDE_L1: to fix - turn: ");

                holding.resetTimer();
                holding.setStatus(Holding::STATUS_INSIDE_TURN_1);
                    
                true_track_result = Navcalc::getTurnHeading(
                    m_flightstatus->smoothedTrueHeading(),
                    holding_out_track - (2*holding_track_wind_corr_angle),
                    holding.isLeftHolding());
            }
            else if (dist_to_wpt <= m_fmc_data.turnRadiusNm())
            {
                // we are too near to fetch the inbound track - go direct

//                printf("INSIDE_L1: to fix - direct ");
                true_track_result = track_to_wpt;
            }
            else
            {
                //we are heading to the fix - construct inbound track
                    
//                printf("INSIDE_L1: to fix - track: ");

                Waypoint to_fix_fmc_wpt("HoldInbdTrack", QString::null, 
                                        holding.inbdTrackLat(), holding.inbdTrackLon());
                true_track_result = getInterceptHeadingToTrack(to_fix_fmc_wpt, to_wpt);
            }

//             printf("ttr:%.02lf: %s\n", true_track_result, holding.toString().toLatin1().data());
//             fflush(stdout);
            
            break;
        }

        case(Holding::STATUS_INSIDE_TURN_1): {

            holding.resetTimer();
            
            double target_track = holding_out_track - (2*holding_track_wind_corr_angle);
            
            double heading_diff = Navcalc::getAbsHeadingDiff(
                m_flightstatus->smoothedTrueHeading(), target_track + m_flightstatus->wind_correction_angle_deg);
            
            // exit the turn and go into the leg
            
            if (heading_diff > m_autopilot_config->getDoubleValue(CFG_TURN_DIST_HDG_DIFF_FACTOR))
            {
                true_track_result = Navcalc::getTurnHeading(
                    m_flightstatus->smoothedTrueHeading(),
                    target_track,
                    holding.isLeftHolding());
            }
            else
            {
                true_track_result = target_track;
            }
            
            // change to leg 2 if the fix is abeam 
	    
            if (Navcalc::isAfterAbeamFix(
                    track_to_wpt, holding_track - 90.0, 180.0) &&
                heading_diff <= m_autopilot_config->getDoubleValue(CFG_HOLDING_HDG_DIFF_LEG_DETECTION))
            {
                holding.setStatus(Holding::STATUS_INSIDE_LEG_2);
            }
	    
//             printf("INSIDE_T1: hdgdiff:%.02lf, %s\n", heading_diff, holding.toString().toLatin1().data());
//             fflush(stdout);
            
            break;
        }

        case(Holding::STATUS_INSIDE_LEG_2): {

//            printf("INSIDE_L2: ");

            // calc correction for wind to make the inbound leg as long as the
            // given time

            double speed_diff = m_flightstatus->ground_speed_kts - m_flightstatus->tas;

            double corrected_outbound_time = 
                ((holding.holdLegLengthMin() * (m_flightstatus->tas - speed_diff)) -
                 (speed_diff * 1.5)) / m_flightstatus->ground_speed_kts;
	    
//             printf("corrected time:%.02lf -> %.02lf ", 
//                    holding.holdLegLengthMin(), corrected_outbound_time);

            if ((holding.timerSeconds()/60.0) < corrected_outbound_time)
            {
                // continue second holding leg

                true_track_result = holding_out_track;
                
                // for normal holdings we triple the WCA
                if (holding.holdLegLengthMin() < 2.0)
                    true_track_result -= 2*holding_track_wind_corr_angle;
                
//                printf("from fix: ");
            }
            else
            {
                // the second leg is over - turn to fix

//                printf("from fix - turn: ");
                    
                holding.resetTimer();
                holding.setStatus(Holding::STATUS_INSIDE_TURN_2);
                    
                true_track_result = Navcalc::getTurnHeading(
                    m_flightstatus->smoothedTrueHeading(), holding_track,
                    holding.isLeftHolding());
            }            

//             printf("%s\n", holding.toString().toLatin1().data());
//             fflush(stdout);
            
            break;
        }

        case(Holding::STATUS_INSIDE_TURN_2): {

            double heading_diff = Navcalc::getAbsHeadingDiff(
		m_flightstatus->smoothedTrueHeading(), holding_track + m_flightstatus->wind_correction_angle_deg);
	    
            if (heading_diff > m_autopilot_config->getDoubleValue(CFG_MAX_INTERCEPT_ANGLE))
            {
                // continue the turn inside the holding
                true_track_result = Navcalc::getTurnHeading(
                    m_flightstatus->smoothedTrueHeading(), holding_track,
                    holding.isLeftHolding());
            }
            else
            {
                // take intercept heading to inbd track into account
                Waypoint to_fix_fmc_wpt("HoldInbdTrack", QString::null, 
                                        holding.inbdTrackLat(), holding.inbdTrackLon());
				
				double target_track = getInterceptHeadingToTrack(to_fix_fmc_wpt, to_wpt);
			                
                true_track_result = Navcalc::getTurnHeading(
                    m_flightstatus->smoothedTrueHeading(), target_track, holding.isLeftHolding());
				
				heading_diff = Navcalc::getAbsHeadingDiff(
					m_flightstatus->smoothedTrueHeading(), target_track + m_flightstatus->wind_correction_angle_deg);
				
                if (heading_diff <= m_autopilot_config->getDoubleValue(CFG_HOLDING_HDG_DIFF_LEG_DETECTION))
                {
                    holding.resetTimer();
                    holding.setStatus(Holding::STATUS_INSIDE_LEG_1);
                }
            }

//             printf("INSIDE_T2: hdgdiff:%.02lf ", heading_diff);
//             printf("%s\n", holding.toString().toLatin1().data());
//             fflush(stdout);
            
            break;
        }

        default: {
            MYASSERT(0);
        }
    }

    to_wpt.setHolding(holding);

    MYASSERT(true_track_result != -10.0);

    return true_track_result;
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutopilot::calculateLateralMode()
{
    // only calculate the mode 4 times per second
    if (m_lateral_mode_calc_timer.elapsed() < 250) return;
    m_lateral_mode_calc_timer.start();

    LATERAL_MODE last_lateral_mode_active = m_lateral_mode_active;
    m_lateral_mode_active = m_lateral_mode_armed = LATERAL_MODE_NONE;
    
    if (m_flightstatus->ap_app_lock || m_flightstatus->ap_app_bc_lock)
    {
        if (m_flightstatus->ap_hdg_lock || m_flightstatus->ap_nav1_lock)
        {
            if (m_flightstatus->ap_hdg_lock)
            {
                m_lateral_mode_active = (isNAVCoupled()) ? LATERAL_MODE_LNAV : LATERAL_MODE_HDG;
            }
            else if (m_flightstatus->ap_nav1_lock)
            {
                if (!m_flightstatus->nav1.id().isEmpty())
                    m_lateral_mode_active = 
                        (!m_flightstatus->nav1_has_loc) ? LATERAL_MODE_VOR : LATERAL_MODE_LOC;
            }
            
            m_lateral_mode_armed = LATERAL_MODE_LOC;
        }
        else
        {
            m_lateral_mode_active = LATERAL_MODE_LOC;
        }
    }
    else if (m_flightstatus->ap_nav1_lock)
    {
        if (m_flightstatus->ap_hdg_lock)
        {
            m_lateral_mode_active = (isNAVCoupled()) ? LATERAL_MODE_LNAV : LATERAL_MODE_HDG;
            
            if (!m_flightstatus->nav1.id().isEmpty())
                m_lateral_mode_armed = 
                    (m_flightstatus->nav1_has_loc) ? LATERAL_MODE_LOC : LATERAL_MODE_VOR;
        }
        else
        {
            if (!m_flightstatus->nav1.id().isEmpty())
                m_lateral_mode_active = 
                    (m_flightstatus->nav1_has_loc) ? LATERAL_MODE_LOC : LATERAL_MODE_VOR;
        }
    }
    else if (m_flightstatus->ap_hdg_lock)
    {
        m_lateral_mode_active = (isNAVCoupled()) ? LATERAL_MODE_LNAV : LATERAL_MODE_HDG;
    }

    // override for takeoff mode

    if ((m_lateral_mode_active == LATERAL_MODE_NONE || m_flightstatus->smoothed_ias.lastValue() < 10.0) &&
        m_flightstatus->onground && 
        m_flightstatus->fd_active)
    {
        if (!isTakeoffModeActiveLateral()) setTakeoffModeLateral(true);
    }

    if (m_takeoff_active_lateral)
    {
        m_lateral_mode_active = LATERAL_MODE_TAKEOFF;
        m_lateral_mode_armed = m_takeoff_lateral_mode_armed;
    }

    // override LOC mode

    if (m_lateral_mode_active == LATERAL_MODE_LOC && !m_flightstatus->localizerAlive())
        m_lateral_mode_active = LATERAL_MODE_LOC_CAPTURE;

    // override landing/flare/rollout mode

    if (m_flightstatus->fd_active && m_flightstatus->localizerEstablished())
    {
        if (m_fmc_control->flightModeTracker().isApproach())
        {
            if (m_flightstatus->radarAltitude() < 400 &&
                !m_flightstatus->isGearUp() &&
                !m_flightstatus->flapsAreUp())
            {
                if (m_flightstatus->radarAltitude() < 50) m_lateral_mode_active = LATERAL_MODE_FLARE;
                else                                           m_lateral_mode_active = LATERAL_MODE_LANDING;
            }
        }
        else if (m_fmc_control->flightModeTracker().isLanding())
        {
            if (!m_flightstatus->isGearUp() &&
                !m_flightstatus->flapsAreUp() &&
                m_flightstatus->onground) m_lateral_mode_active = LATERAL_MODE_ROLLOUT;
        }
    }

    // override on the ground
    if (m_fmc_control->flightModeTracker().isPreflight() || m_fmc_control->flightModeTracker().isTaxiing())
        m_lateral_mode_active = LATERAL_MODE_NONE;

    // detect mode change

    if (m_lateral_mode_active != last_lateral_mode_active) m_lateral_mode_active_changed_time.start();
}

/////////////////////////////////////////////////////////////////////////////

uint FMCAutopilot::lateralModeActiveChangeTimeMs() const 
{
    if (m_fmc_control->isFMCConnectModeSlave()) return m_master_fmc_lateral_mode_active_change_time_ms;
    return m_lateral_mode_active_changed_time.elapsed(); 
}

/////////////////////////////////////////////////////////////////////////////

uint FMCAutopilot::verticalModeActiveChangeTimeMs() const 
{
    if (m_fmc_control->isFMCConnectModeSlave()) return m_master_fmc_vertical_mode_active_change_time_ms;
    return m_vertical_mode_active_changed_time.elapsed(); 
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutopilot::slotSwitchLateralFlightDirectorToExternalSource()
{
    Logger::log("FMCAutopilot:slotSwitchLateralFlightDirectorToExternalSource:");
    m_flightstatus->setFlightDirectorBankInputFromExternal(true);
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutopilot::slotSwitchVerticalFlightDirectorToExternalSource()
{
    Logger::log("FMCAutopilot:slotSwitchVerticalFlightDirectorToExternalSource:");
    m_flightstatus->setFlightDirectorPitchInputFromExternal(true);
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutopilot::setTakeoffModeLateral(bool yes)
{
    Logger::log(QString("FMCAutopilot:setTakeoffModeLateral: %1").arg(yes));

    if (yes)
    {
        m_takeoff_active_lateral = true;
        m_takeoff_lateral_mode_armed = LATERAL_MODE_LNAV;
        m_takeoff_lateral_target_track = -99.0;
        m_lateral_fd_source_reset_timer.stop();
        m_flightstatus->setFlightDirectorBankInputFromExternal(false);
        m_flightstatus->resetFlightDirector();
        setNAVCouple(true);

    }
    else
    {
        m_takeoff_active_lateral = false;
        m_takeoff_lateral_mode_armed = LATERAL_MODE_NONE;
        m_lateral_fd_source_reset_timer.start(1000);
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutopilot::setTakeoffModeVertical(bool yes)
{
    Logger::log(QString("FMCAutopilot:setTakeoffModeVertical: %1").arg(yes));

    if (yes)
    {
        m_takeoff_active_vertical = true;
        m_takeoff_vertical_mode_armed = VERTICAL_MODE_FLCH;
        m_takeoff_vertical_speed_hold_kts = 0.0;
        m_vertical_fd_source_reset_timer.stop();
        m_flightstatus->setFlightDirectorPitchInputFromExternal(false);
        m_flightstatus->resetFlightDirector();
    }
    else
    {
        m_takeoff_active_vertical = false;
        m_takeoff_vertical_mode_armed = VERTICAL_MODE_NONE;
        m_vertical_fd_source_reset_timer.start(1000);
    }
}

/////////////////////////////////////////////////////////////////////////////

bool FMCAutopilot::isNAVCoupled() const 
{
    if (m_fmc_control->isFMCConnectModeSlave()) return m_master_fmc_nav_coupled;
    return m_autopilot_config->getIntValue(CFG_AP_NAV_COUPLE) != 0; 
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutopilot::setNAVCouple(bool yes) 
{
    Logger::log(QString("FMCAutopilot:setNAVCouple: %1, takeoff: %2").
                arg(yes).arg(m_takeoff_active_lateral));

    if (m_takeoff_active_lateral) 
        m_takeoff_lateral_mode_armed = yes ? LATERAL_MODE_LNAV : LATERAL_MODE_HDG;

    m_autopilot_config->setValue(CFG_AP_NAV_COUPLE, yes ? 1:0);
    calculateLateralMode();
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutopilot::setNAVHold()
{
    Logger::log(QString("FMCAutopilot:setNAVHold: takeoff: %1").arg(m_takeoff_active_lateral));

    if (m_takeoff_active_lateral) 
    {
        m_takeoff_lateral_mode_armed = LATERAL_MODE_LNAV;
    }
    else
    {
        setNAVCouple(true);
        m_fmc_control->fsAccess().setAPHeadingHold(true);
    }
}

/////////////////////////////////////////////////////////////////////////////

bool FMCAutopilot::isNAVHoldActive() const 
{
    if (m_fmc_control->isFMCConnectModeSlave()) return m_master_fmc_nav_hold_active;
    return isNAVCoupled() && m_flightstatus->ap_hdg_lock; 
}

/////////////////////////////////////////////////////////////////////////////

bool FMCAutopilot::isHeadingHoldActive() const 
{
    if (m_fmc_control->isFMCConnectModeSlave()) return m_master_fmc_hdg_hold_active;
    return !isNAVCoupled() && m_flightstatus->ap_hdg_lock; 
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutopilot::setHeadingHold(bool set_current_hdg)
{
    Logger::log(QString("FMCAutopilot:setHeadingHold: set_cur=%1, takeoff: %2").
                arg(set_current_hdg).arg(m_takeoff_active_lateral));

    if (m_takeoff_active_lateral) 
    {
        m_takeoff_lateral_mode_armed = LATERAL_MODE_HDG;
    }
    else
    {
        setNAVCouple(false);
        m_fmc_control->fsAccess().setAPHeadingHold(true);
        if (set_current_hdg)
            m_fmc_control->fsAccess().setAPHeading(Navcalc::round(m_flightstatus->smoothedMagneticHeading()));
    }
}

/////////////////////////////////////////////////////////////////////////////

bool FMCAutopilot::isAPPHoldActive() const
{
    return m_flightstatus->ap_app_lock;
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutopilot::setAPPHold()
{
    Logger::log("FMCAutopilot:setAPPHold");
    if (m_takeoff_active_lateral || m_takeoff_active_vertical) return;
    if (!m_flightstatus->nav1.id().isEmpty() && m_flightstatus->nav1_has_loc) m_fmc_control->fsAccess().setAPPArm(true);
}

/////////////////////////////////////////////////////////////////////////////

bool FMCAutopilot::isLOCHoldActive() const
{
    return m_flightstatus->ap_nav1_lock && !m_flightstatus->nav1.id().isEmpty() && m_flightstatus->nav1_has_loc;
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutopilot::setLOCHold()
{
    Logger::log("FMCAutopilot:setLOCHold");
    if (m_takeoff_active_lateral) return;
    if (!m_flightstatus->nav1.id().isEmpty()) m_fmc_control->fsAccess().setNAV1Arm(true);
}

/////////////////////////////////////////////////////////////////////////////

bool FMCAutopilot::isALTHoldActive() const
{
    return m_flightstatus->ap_alt_lock;
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutopilot::setALTHold(bool set_current_alt)
{
    Logger::log(QString("FMCAutopilot:setLOCHold: set_cur=%1").arg(set_current_alt));

    if (m_takeoff_active_vertical) return;

    if (isFLChangeModeActive()) setFLChangeMode(false, false);
    m_fmc_control->fsAccess().setAPAltHold(true);
    if (set_current_alt)
        m_fmc_control->fsAccess().setAPAlt((int)m_flightstatus->smoothed_altimeter_readout.lastValue());
}

/////////////////////////////////////////////////////////////////////////////

bool FMCAutopilot::isFlightPathModeEnabled() const
{
    if (m_fmc_control->isFMCConnectModeSlave()) return m_master_fmc_fpa_mode_enabled;
    return m_autopilot_config->getIntValue(CFG_AP_MODE_FLIGHTPATH_ENABLED) != 0;
}

/////////////////////////////////////////////////////////////////////////////

bool FMCAutopilot::isFlightPathModeActive() const 
{
    if (m_fmc_control->isFMCConnectModeSlave()) return m_master_fmc_fpa_mode_active;

    return ((m_flightstatus->ap_vs_lock ||
             (m_flightstatus->ap_alt_lock &&
              qAbs(m_flightstatus->smoothed_altimeter_readout.lastValue() - m_flightstatus->APAlt()) > 
              2*ALT_CAPTURE_MARGIN)) &&
            isFlightPathModeEnabled());
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutopilot::enableFlightPathMode(bool yes) 
{
    m_autopilot_config->setValue(CFG_AP_MODE_FLIGHTPATH_ENABLED, (yes ? 1:0)); 
    if (yes) setFlightPathAngleInternal(m_flightstatus->fpv_vertical.lastValue());
    calculateVerticalMode();
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutopilot::setFlightPathAngle(const double& fpv)
{ 
    if (m_takeoff_active_vertical) return;
    if (!isFlightPathModeActive()) return;
    m_flightpath_angle = LIMIT(((Navcalc::round(fpv * 10.0)) / 10.0), 30.0); 
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutopilot::setFlightPathAngleInternal(const double& fpv)
{ 
    m_flightpath_angle = LIMIT(((Navcalc::round(fpv * 10.0)) / 10.0), 30.0); 
}

/////////////////////////////////////////////////////////////////////////////

bool FMCAutopilot::isVsModeEnabled() const
{
    if (m_fmc_control->isFMCConnectModeSlave()) return m_master_fmc_vs_mode_enabled;
    return m_autopilot_config->getIntValue(CFG_AP_MODE_FLIGHTPATH_ENABLED) == 0;
}

/////////////////////////////////////////////////////////////////////////////

bool FMCAutopilot::isVsModeActive() const
{
    if (m_fmc_control->isFMCConnectModeSlave()) return m_master_fmc_vs_mode_active;
    return ((m_flightstatus->ap_vs_lock ||
             (m_flightstatus->ap_alt_lock &&
              qAbs(m_flightstatus->smoothed_altimeter_readout.lastValue() - m_flightstatus->APAlt()) > 
              2*ALT_CAPTURE_MARGIN)) &&
            isVsModeEnabled());
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutopilot::setVSFPAHold(bool zero_vs)
{
    Logger::log(QString("FMCAutopilot:setVSFPAHold: set_zero=%1").arg(zero_vs));

    if (m_takeoff_active_vertical) return;

    if (isFLChangeModeActive()) setFLChangeMode(false, false);
    m_fmc_control->fsAccess().setAPAltHold(true);

    if (isFlightPathModeActive())
    {
        zero_vs ? 
            setFlightPathAngle(0.0) : 
            setFlightPathAngle(m_flightstatus->fpv_vertical.value());
    }
    else
    {
        zero_vs ? 
            setVerticalSpeed(0) : 
            setVerticalSpeed((Navcalc::round(m_flightstatus->smoothedVS())/100)*100);
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutopilot::setVerticalSpeed(int vs) 
{
    if (m_takeoff_active_vertical) return;

    if (!isVsModeActive()) return;
    m_vertical_speed = LIMIT(vs / 100 * 100, 6000);
    m_do_set_vs_to_ap = true;
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutopilot::setVerticalSpeedInternal(int vs)
{
    m_vertical_speed = LIMIT(vs / 100 * 100, 6000);    
}

/////////////////////////////////////////////////////////////////////////////

bool FMCAutopilot::isFLChangeModeActive() const
{
    return m_mode_fl_change_active;
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutopilot::setFLChangeMode(bool yes, bool set_spd_to_current)
{
    Logger::log(QString("FMCAutopilot:setFLChangeMode: %1 set_spd=%2").
                arg(yes).arg(set_spd_to_current));

    bool was_active = m_mode_fl_change_active;

    if (qAbs(m_flightstatus->smoothed_altimeter_readout.lastValue() - m_flightstatus->APAlt()) < ALT_CAPTURE_MARGIN)
        yes = false;

    m_mode_fl_change_active = yes;

    if (isFLChangeModeActive())
    {
        m_fl_change_cmd_vs = m_flightstatus->smoothedVS();
        m_fmc_control->fsAccess().setAPAltHold(true);

        if (set_spd_to_current) 
            m_fmc_control->fsAccess().setAPAirspeed(Navcalc::round(m_flightstatus->smoothed_ias.lastValue()));
    
        if (m_fmc_control->fmcAutothrottle().isAPThrottleArmed() ||
            m_fmc_control->fmcAutothrottle().isAPThrottleEngaged())
        {
            m_fmc_control->fmcAutothrottle().engageAPThrottleN1Hold();
        }
    }
    else if (was_active)
    {
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutopilot::calculateFlightDirector()
{
    bool slave = m_fmc_control->isFMCConnectModeSlave();
    
    //----- lateral

    if (isTakeoffModeActiveLateral())
    {
        if (slave) m_flightstatus->setFlightDirectorBankInputFromExternal(false);

        // calc flight director

        double fd_bank = 0.0;        
        if (m_takeoff_lateral_target_track > -1.0)
            fd_bank = -2.0 * LIMIT(Navcalc::getSignedHeadingDiff(
                                       m_flightstatus->smoothedMagneticTrack(), m_takeoff_lateral_target_track), 12.5);

        m_flightstatus->setFlightDirectorBankInternal(fd_bank);
    }
    else
    {
        if (slave) m_flightstatus->setFlightDirectorBankInputFromExternal(true);
    }

    //----- vertical

    if (isTakeoffModeActiveVertical())
    {
        if (slave) m_flightstatus->setFlightDirectorPitchInputFromExternal(false);

        // calc flight director
        
        if (m_takeoff_vertical_speed_hold_kts > 0.0 &&
            m_takeoff_vertical_speed_hold_engaged_dt.elapsed() > 2000)
        {
            m_flightstatus->setFlightDirectorPitchInputFromExternal(true);
        }
        else
        {
            m_flightstatus->setFlightDirectorPitchInternal(m_flightstatus->smoothedPitch());
        }
    }
    else if (m_lateral_mode_active == LATERAL_MODE_ROLLOUT)
    {
        m_flightstatus->setFlightDirectorPitchInputFromExternal(false);
        m_flightstatus->setFlightDirectorPitchInternal(m_flightstatus->smoothedPitch());
        m_vertical_fd_source_reset_timer.start(1000);
    }
    else
    {
        if (slave) m_flightstatus->setFlightDirectorPitchInputFromExternal(true);
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutopilot::calculateLateral()
{
    if (!m_flightstatus->fd_active && !m_flightstatus->ap_enabled)
    {
        if (isTakeoffModeActiveLateral()) 
        {
            Logger::log("FMCAutopilot:calculateLateral: no FD and no AP - disabling takoff mode");
            setTakeoffModeLateral(false);
        }
    }
    else if (isTakeoffModeActiveLateral())
    {
        // calc takeoff mode

        if (m_flightstatus->radarAltitude() > 30)
        {
            Logger::log("FMCAutopilot:calculateLateral: reached RA 30ft - switching lateral mode");
            
            m_takeoff_active_lateral = false;

            switch(m_takeoff_lateral_mode_armed)
            {
                case(LATERAL_MODE_LNAV):
                    Logger::log("FMCAutopilot:lateralRecalc: setting NAV after takeoff");
                    setNAVHold();
                    break;

                case(LATERAL_MODE_HDG):
                    Logger::log("FMCAutopilot:lateralRecalc: setting HDG after takeoff");
                    setHeadingHold(true);
                    break;

                default:
                    Logger::log(QString("FMCAutopilot:lateralRecalc: "
                                        "invalid armed mode after takeoff mode: %1").
                                arg(m_takeoff_lateral_mode_armed));
                    break;
            }

            setTakeoffModeLateral(false);
        }
        else
        {
            if (m_flightstatus->radarAltitude() < 30)
            {
                m_takeoff_lateral_target_track = -99;
            }
            else if (m_takeoff_lateral_target_track < -1.0)
            {
                m_takeoff_lateral_target_track = m_flightstatus->smoothedMagneticTrack();
            }

            if (!isHeadingHoldActive())
            {
                m_autopilot_config->setValue(CFG_AP_NAV_COUPLE, 0);
                m_fmc_control->fsAccess().setAPHeadingHold(true);
            }

            if (m_takeoff_lateral_target_track < 0)
                m_fmc_control->fsAccess().setAPHeading(m_flightstatus->smoothedMagneticHeading());
            else
                m_fmc_control->fsAccess().setAPHeading(m_takeoff_lateral_target_track + 
                                                       m_flightstatus->wind_correction_angle_deg);
        }
    }

    double autopilot_heading = m_flightstatus->APHdg();
    m_last_nav_calculated_heading = autopilot_heading;

    //----- calc autopilot heading from the normal route
    
    Waypoint* active_wpt = m_fmc_data.normalRoute().activeWaypoint();
    Waypoint* prev_wpt = m_fmc_data.normalRoute().previousWaypoint();

    if (active_wpt == 0) 
    {
        if (isNAVCoupled())
        {
            Logger::log("FMCAutopilot:lateralRecalc: no more waypoint, uncoupling");
            setNAVCouple(false);
        }

        return;
    }
    
    //-----

    if (active_wpt->holding().isValid() &&
		( (active_wpt->holding().status() != Holding::STATUS_INACTIVE) ||
          m_fmc_data.distanceToActiveWptNm() < 
          m_autopilot_config->getDoubleValue(CFG_DIRECT_TURNDIST_FACTOR) * m_fmc_data.turnRadiusNm()))
    {
        // we have a holding
        autopilot_heading = getHeadingForHolding(*active_wpt);
	}
    else if (active_wpt->asWaypointHdgToAlt() != 0)
    {
        autopilot_heading = active_wpt->asWaypointHdgToAlt()->hdgToHold() + m_flightstatus->magvar;
    }
    else if (active_wpt->asWaypointHdgToIntercept() != 0)
    {
        //TODO correct?
        autopilot_heading = m_fmc_data.trueTrackToActiveWpt();

//         //TODO do wind correcation later when getting a track instead of a heading
//         autopilot_heading = active_wpt->asWaypointHdgToIntercept()->hdgUntilIntercept();
    }
    else if (prev_wpt != 0)
    {
        autopilot_heading = getInterceptHeadingToTrack(*prev_wpt, *active_wpt);
    }
    else
    {
        autopilot_heading = m_fmc_data.trueTrackToActiveWpt();
    }

    //-----

	// correct the AP heading for wind

	autopilot_heading = Navcalc::trimHeading(autopilot_heading + m_flightstatus->wind_correction_angle_deg);

 	// correct the AP heading depending on the aircraft's current heading for faster turns
    
    double heading_diff = Navcalc::getSignedHeadingDiff(m_flightstatus->smoothedTrueHeading(), autopilot_heading);
    
    if (fabs(heading_diff) < m_autopilot_config->getDoubleValue(CFG_MAX_INTERCEPT_ANGLE))
        autopilot_heading += heading_diff * m_autopilot_config->getDoubleValue(CFG_INTERCEPT_HDG_DIFF_MULTIPLY_FACTOR);

    // correct the AP heading for magnetic variation

    autopilot_heading = Navcalc::trimHeading(autopilot_heading - m_flightstatus->magvar);

    // limit max hdg deviation left/right from the current heading

    heading_diff = Navcalc::getSignedHeadingDiff(m_flightstatus->smoothedTrueHeading(), autopilot_heading);    

    if (heading_diff > m_autopilot_config->getDoubleValue(CFG_MAX_INTERCEPT_ANGLE))
        autopilot_heading = Navcalc::trimHeading(
            m_flightstatus->smoothedTrueHeading() + m_autopilot_config->getDoubleValue(CFG_MAX_INTERCEPT_ANGLE));
    else if (heading_diff < -m_autopilot_config->getDoubleValue(CFG_MAX_INTERCEPT_ANGLE))
        autopilot_heading = Navcalc::trimHeading(
            m_flightstatus->smoothedTrueHeading() - m_autopilot_config->getDoubleValue(CFG_MAX_INTERCEPT_ANGLE));

    // set autopilot heading 

    m_last_nav_calculated_heading = autopilot_heading;
    if (isNAVCoupled() && !isTakeoffModeActiveLateral()) m_fmc_control->fsAccess().setAPHeading(autopilot_heading);
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutopilot::calculateVerticalMode()
{
    // only calculate the mode 4 times per second
    if (m_vertical_mode_calc_timer.elapsed() < 250) return;
    m_vertical_mode_calc_timer.start();

    double abs_ap_alt_diff = qAbs(m_flightstatus->smoothed_altimeter_readout.lastValue() - m_flightstatus->APAlt());

    VERTICAL_MODE last_vertical_mode_active = m_vertical_mode_active;
    m_vertical_mode_active = m_vertical_mode_armed = VERTICAL_MODE_NONE;

    double m_alt_readout = m_flightstatus->smoothedAltimeterReadout();

    // detect current states

    if (m_flightstatus->ap_gs_lock || m_flightstatus->ap_app_lock)
    {
        if (m_flightstatus->ap_alt_lock)
        {
            if (isVsModeActive())              m_vertical_mode_active = VERTICAL_MODE_VS;
            else if (isFlightPathModeActive()) m_vertical_mode_active = VERTICAL_MODE_FPV;
            else                               m_vertical_mode_active = VERTICAL_MODE_ALT;
            
            m_vertical_mode_armed = VERTICAL_MODE_GS;
        }
        else
        {
            m_vertical_mode_active = VERTICAL_MODE_GS;
        }
    }
    else if (isVsModeActive())
    {
        m_vertical_mode_active = VERTICAL_MODE_VS;
        if (m_flightstatus->ap_alt_lock) m_vertical_mode_armed = VERTICAL_MODE_ALT;
    }
    else if (isFlightPathModeActive())
    {
        m_vertical_mode_active = VERTICAL_MODE_FPV;
        if (m_flightstatus->ap_alt_lock) m_vertical_mode_armed = VERTICAL_MODE_ALT;
    }
    else if (m_flightstatus->ap_alt_lock)
    {
        m_vertical_mode_active = 
            (fabs(m_alt_readout - (100*m_fmc_control->normalRoute().cruiseFl())) < ALT_CRZ_MARGIN) ? 
            VERTICAL_MODE_ALTCRZ : VERTICAL_MODE_ALT;
    }

    // override V/S

    if (!isTakeoffModeActiveVertical() && 
        !isFLChangeModeActive() && 
        (isVsModeActive() || isFlightPathModeActive()) && 
        qAbs(m_flightstatus->APVs()) < 10)
    {
        m_vertical_mode_active = (fabs(m_alt_readout - (100*m_fmc_control->normalRoute().cruiseFl())) < ALT_CRZ_MARGIN) ? 
            VERTICAL_MODE_ALTCRZ : VERTICAL_MODE_ALT;
        if (m_vertical_mode_armed == VERTICAL_MODE_ALT) m_vertical_mode_armed = VERTICAL_MODE_NONE;
    }

    // override V/S

    if (m_vertical_mode_active == VERTICAL_MODE_VS &&
        abs_ap_alt_diff < ALT_CAPTURE_VS_INHIBIT &&
        qAbs(m_flightstatus->APVs()) < 50)
    {
        m_vertical_mode_active = (fabs(m_alt_readout - (100*m_fmc_control->normalRoute().cruiseFl())) < ALT_CRZ_MARGIN) ? 
                                 VERTICAL_MODE_ALTCRZ : VERTICAL_MODE_ALT;
    }

    // override alt hold with alt capture

    if ((m_vertical_mode_active == VERTICAL_MODE_ALT || m_vertical_mode_active == VERTICAL_MODE_ALTCRZ) &&
        abs_ap_alt_diff > ALT_CAPTURE_MARGIN && 
        abs_ap_alt_diff < ALT_CAPTURE_VS_INHIBIT)
    {
        m_vertical_mode_active = VERTICAL_MODE_ALT_CAPTURE;
        if (m_vertical_mode_armed == VERTICAL_MODE_ALT) m_vertical_mode_armed = VERTICAL_MODE_NONE;
    }

    // override for FLCH mode

    if (isFLChangeModeActive())
    {
        if (abs_ap_alt_diff > ALT_CAPTURE_VS_INHIBIT)
        {
            m_vertical_mode_active = VERTICAL_MODE_FLCH;
        }
        else if (abs_ap_alt_diff > ALT_CAPTURE_MARGIN)
        {
            m_vertical_mode_active = VERTICAL_MODE_ALT_CAPTURE;
            if (m_vertical_mode_armed == VERTICAL_MODE_ALT) m_vertical_mode_armed = VERTICAL_MODE_NONE;
        }
    }

    // override for TAKEOFF mode

    if ((m_vertical_mode_active == VERTICAL_MODE_NONE || m_flightstatus->smoothed_ias.lastValue() < 10.0) &&
        m_flightstatus->onground && 
        m_flightstatus->fd_active)
    {
        if (!isTakeoffModeActiveVertical()) setTakeoffModeVertical(true);
    }

    if (m_takeoff_active_vertical)
    {
        m_vertical_mode_active = VERTICAL_MODE_TAKEOFF;
        m_vertical_mode_armed = m_takeoff_vertical_mode_armed;
    }
 
    // override GS mode

    if (m_vertical_mode_active == VERTICAL_MODE_GS && !m_flightstatus->glideslopeAlive()) 
        m_vertical_mode_active = VERTICAL_MODE_GS_CAPTURE;

    // override landing/flare/rollout mode

    if (m_flightstatus->fd_active && m_flightstatus->localizerEstablished())
    {
        if (m_fmc_control->flightModeTracker().currentFlightMode() == FlightModeTracker::FLIGHT_MODE_APPROACH)
        {
            if (m_flightstatus->radarAltitude() < 400 &&
                !m_flightstatus->isGearUp() &&
                !m_flightstatus->flapsAreUp())
            {
                if (m_flightstatus->radarAltitude() < 50) m_vertical_mode_active = VERTICAL_MODE_FLARE;
                else                                           m_vertical_mode_active = VERTICAL_MODE_LANDING;
            }
        }
        else if (m_fmc_control->flightModeTracker().currentFlightMode() == FlightModeTracker::FLIGHT_MODE_LANDING)
        {
            if (!m_flightstatus->isGearUp() &&
                !m_flightstatus->flapsAreUp() &&
                m_flightstatus->onground) m_vertical_mode_active = VERTICAL_MODE_ROLLOUT;
        }
    }

    // override on the ground
    if (m_fmc_control->flightModeTracker().isPreflight() || m_fmc_control->flightModeTracker().isTaxiing())
        m_vertical_mode_active = VERTICAL_MODE_NONE;

    // detect mode change
   
    if (m_vertical_mode_active != last_vertical_mode_active) m_vertical_mode_active_changed_time.start();
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutopilot::calculateVertical()
{
    double ap_alt_diff = m_flightstatus->smoothed_altimeter_readout.lastValue() - m_flightstatus->APAlt();
    double abs_ap_alt_diff = qAbs(ap_alt_diff);
    double vs = m_flightstatus->smoothedVS();

    if (!m_flightstatus->fd_active && !m_flightstatus->ap_enabled)
    {
        if (isTakeoffModeActiveVertical()) setTakeoffModeVertical(false);
        if (isFLChangeModeActive()) setFLChangeMode(false, false);
    }
    else if (isTakeoffModeActiveVertical())
    {
        // takeoff mode

        if ((m_fmc_data.normalRoute().accelerationAltitudeFt() > 0 &&
             m_flightstatus->smoothed_altimeter_readout.lastValue() >= m_fmc_data.normalRoute().accelerationAltitudeFt())
            ||
            (m_fmc_data.normalRoute().accelerationAltitudeFt() <= 0 &&
             m_flightstatus->radarAltitude() > 1000))
        {
            Logger::log("FMCAutopilot:verticalRecalc: acceleration alt reached");

            switch(m_takeoff_vertical_mode_armed)
            {
                case(VERTICAL_MODE_FLCH):
                    Logger::log("FMCAutopilot:verticalRecalc: setting FLCH mode after takeoff");
                    setFLChangeMode(true, false);
                    break;

                default:
                    Logger::log(QString("FMCAutopilot:verticalRecalc: "
                                        "invalid armed mode after takeoff mode: %1").
                                arg(m_takeoff_vertical_mode_armed));
                    break;
            }

            setTakeoffModeVertical(false);
        }
        else if (m_flightstatus->radarAltitude() > 200 && !isFLChangeModeActive())
        {
            if (m_takeoff_vertical_speed_hold_kts <= 0.0)
            {
                m_takeoff_vertical_speed_hold_kts = m_flightstatus->smoothedIAS();

                if (m_fmc_control->fmcData().V2() > 0)
                {
                    m_takeoff_vertical_speed_hold_kts =
                        qMax((double)m_fmc_control->fmcData().V2(), 
                             qMin(m_takeoff_vertical_speed_hold_kts, m_fmc_control->fmcData().V2() + 15.0));
                }

                Logger::log(QString("FMCAutopilot:verticalRecalc: takeoff_ias_hold_target=%1").
                            arg(m_takeoff_vertical_speed_hold_kts));

                m_takeoff_vertical_speed_hold_engaged_dt.start();
                setFLChangeMode(true, false);
            }
        }
    }

    if (isFLChangeModeActive())
    {
        if (m_flightstatus->onground)
        {
            setFLChangeMode(false, false);
            m_fmc_control->fsAccess().setAPAltHold(false);
        }
        else
        {
            if (!m_flightstatus->ap_alt_lock) 
                m_fmc_control->fsAccess().setAPAltHold(true);
        
            if (abs_ap_alt_diff < ALT_CAPTURE_MARGIN)
            {
                setFLChangeMode(false, false);

                if (m_fmc_control->fmcAutothrottle().isAPThrottleEngaged() &&
                    m_fmc_control->fmcAutothrottle().isAPThrottleModeN1Engaged())
                {
                    m_fmc_control->fmcAutothrottle().engageAPThrottle(false);
                }
            }
            else if (abs_ap_alt_diff < ALT_CAPTURE_VS_INHIBIT)
            {
                if (m_fmc_control->fmcAutothrottle().isAPThrottleEngaged() &&
                    m_fmc_control->fmcAutothrottle().isAPThrottleModeN1Engaged())
                {
                    m_fmc_control->fmcAutothrottle().engageAPThrottle(false);
                }

                if (m_flightstatus->APVs() > 2000)
                {
                    Logger::log("FMCAutopilot:calculateVertical: Reducing climb rate");
                    m_fmc_control->fsAccess().setAPVs(2000);
                }
                else if (m_flightstatus->APVs() < -2000)
                {
                    Logger::log("FMCAutopilot:calculateVertical: Reducing descent rate");
                    m_fmc_control->fsAccess().setAPVs(-2000);
                }
            }
            else
            {
                if (m_fmc_control->fmcAutothrottle().isAPThrottleEngaged() &&
                    !m_fmc_control->fmcAutothrottle().isAPThrottleModeN1Engaged())
                {
                    setFLChangeMode(false, false);
                }

                //----- gather input for climb/descent mode (OP CLB/DES, FLCH mode)

                double ias_trend = 0.0;
                double ias = m_flightstatus->smoothedIAS(&ias_trend);
                double n1_trend = 0.0;
                m_flightstatus->engine_data[1].smoothedN1(&n1_trend);
                bool climb = ap_alt_diff < 0;

                m_flch_mode_ias_trend_target = 0.0;

                if (isTakeoffModeActiveVertical())
                {
                    m_flch_mode_ias_trend_target = m_takeoff_vertical_speed_hold_kts;
                    if (m_flch_mode_ias_trend_target <= 0.0) m_flch_mode_ias_trend_target = m_flightstatus->smoothedIAS();
                }
                else if (m_fmc_control->fmcAutothrottle().isAPThrottleModeSpeedSet())
                {
                    m_flch_mode_ias_trend_target = m_flightstatus->APSpd();
                }
                else
                {
                    m_flch_mode_ias_trend_target = 
                        Navcalc::getIasFromMach(
                            m_flightstatus->APMach() , m_flightstatus->oat, ias, m_flightstatus->tas, m_flightstatus->mach);
                }
                
                //----- calculate the commanded vertical speed for climb/descent mode (OP CLB/DES, FLCH mode)

                const AircraftData& aircraft_data = m_fmc_control->aircraftData();

                m_flch_mode_ias_trend_target -= 
                    ias + 
                    (ias_trend * aircraft_data.flchControllerIASTrendFactor()) + 
                    (n1_trend * aircraft_data.flchControllerN1TrendFactor());

                m_flch_mode_ias_trend_target = 
                    LIMIT(m_flch_mode_ias_trend_target, aircraft_data.flchControllerIASTrendTargetLimit()) / 10.0;
                
                double vs_change = (ias_trend - m_flch_mode_ias_trend_target) * aircraft_data.flchControllerVSChangeFactor();
                
                m_fl_change_cmd_vs += 
                    LIMIT(vs_change, aircraft_data.flchControllerVSChangeLimit()) / 
                    (1000.0 / qMin(1000, m_refresh_detector.elapsed()));

                (climb) ?
                    m_fl_change_cmd_vs = LIMITMINMAX(m_fl_change_cmd_vs, 0.0, aircraft_data.flchControllerVSCmdLimit()) :
                    m_fl_change_cmd_vs = LIMITMINMAX(m_fl_change_cmd_vs, -aircraft_data.flchControllerVSCmdLimit(), 0.0);

                // limit the commanded VS by the current VS
                m_fl_change_cmd_vs = LIMITMINMAX(m_fl_change_cmd_vs, 
                                                 vs - aircraft_data.flchControllerVSCmdDiffLimit(), 
                                                 vs + aircraft_data.flchControllerVSCmdDiffLimit());

//                 Logger::log(QString("FLCH: ias=%1 trend=%2/%3 trend_tgt=%4 vs_change=%5 cmd=%6 vs=%7 fd_p=%8").
//                             arg(ias).arg(ias_trend, 0, 'f', 0).arg(n1_trend, 0, 'f', 2).arg(m_flch_mode_ias_trend_target, 0, 'f', 2).
//                             arg(vs_change, 0, 'f', 0).arg(m_fl_change_cmd_vs, 0, 'f', 0).arg(vs, 0, 'f', 0).
//                             arg(m_flightstatus->smoothedFlightDirectorPitch(), 0, 'f', 2));
            
                m_fmc_control->fsAccess().setAPVs(Navcalc::round(m_fl_change_cmd_vs));
            }
        }
    }
    else if (isFlightPathModeActive())
    {
//         Logger::log(QString("fpv:vs/altlock=: %1/%2/%3").arg(m_flightstatus->ap_vs_lock).arg(m_flightstatus->ap_alt_lock).
//                     arg(m_flightstatus->APVs()));

        setVerticalSpeedInternal(
            Navcalc::round(Navcalc::getVsByFlightPath(m_flightstatus->ground_speed_kts, m_flightpath_angle)));

        if (m_flightstatus->APVs() != m_vertical_speed && abs_ap_alt_diff > ALT_CAPTURE_VS_INHIBIT)
            m_fmc_control->fsAccess().setAPVs(m_vertical_speed);
    }
    else if (isVsModeActive())
    {
//         Logger::log(QString("vs:vs/altlock=: %1/%2/%3").arg(m_flightstatus->ap_vs_lock).arg(m_flightstatus->ap_alt_lock).
//                     arg(m_flightstatus->APVs()));

        // set the AP VS when the VS mode is engaged

        if (m_do_set_vs_to_ap)
            m_fmc_control->fsAccess().setAPVs(m_vertical_speed);
        else if (m_flightstatus->APVs() != m_vertical_speed && abs_ap_alt_diff > ALT_CAPTURE_VS_INHIBIT)
            setVerticalSpeedInternal(m_flightstatus->APVs());
    }
    else
    {
//         Logger::log(QString("alt:vs/altlock=: %1/%2/%3").arg(m_flightstatus->ap_vs_lock).arg(m_flightstatus->ap_alt_lock).
//                     arg(m_flightstatus->APVs()));

        if (abs_ap_alt_diff < ALT_CAPTURE_MARGIN)
        {
            setFlightPathAngleInternal(0.0);
            setVerticalSpeedInternal(0);
        }
    }

    m_do_set_vs_to_ap = !isVsModeActive();
    m_refresh_detector.start();
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutopilot::slotToggleFlightDirector()
{ 
    Logger::log(QString("FMCAutopilot:slotToggleFlightDirector: cur_active=%1").arg(m_flightstatus->fd_active));
    // inhibit switching the FD to on the ground with engines off
    if (!m_flightstatus->isAtLeastOneEngineOn() && !m_flightstatus->fd_active) return;
    m_fmc_control->fsAccess().setFDOnOff(!m_flightstatus->fd_active); 
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutopilot::slotToggleAutopilot()
{ 
    m_fmc_control->fsAccess().setAPOnOff(!m_flightstatus->ap_enabled); 
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutopilot::operator>>(QDataStream& out) const
{
    out << (qint16) m_ils_mode
        << (qint16) m_lateral_mode_active
        << (qint16) m_lateral_mode_armed
        << lateralModeActiveChangeTimeMs()
        << isNAVCoupled()
        << m_last_nav_calculated_heading
        << isNAVHoldActive()
        << isHeadingHoldActive()
        << (qint16) m_vertical_mode_active
        << (qint16) m_vertical_mode_armed
        << verticalModeActiveChangeTimeMs()
        << isVsModeEnabled()
        << isFlightPathModeEnabled()
        << isVsModeActive()
        << isFlightPathModeActive()
        << m_flightpath_angle
        << m_vertical_speed
        << m_mode_fl_change_active
        << m_fl_change_cmd_vs
        << m_do_set_vs_to_ap
        << m_takeoff_active_lateral
        << (qint16) m_takeoff_lateral_mode_armed
        << m_takeoff_active_vertical
        << (qint16) m_takeoff_vertical_mode_armed
        << m_takeoff_lateral_target_track
        << m_takeoff_vertical_speed_hold_kts;
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutopilot::operator<<(QDataStream& in)
{
    qint16 ils_mode, lateral_mode_active, lateral_mode_armed, vertical_mode_active, vertical_mode_armed;
    qint16 takeoff_lateral_mode_armed, takeoff_vertical_mode_armed;

    in >> ils_mode
       >> lateral_mode_active
       >> lateral_mode_armed
       >> m_master_fmc_lateral_mode_active_change_time_ms
       >> m_master_fmc_nav_coupled
       >> m_last_nav_calculated_heading
       >> m_master_fmc_nav_hold_active
       >> m_master_fmc_hdg_hold_active
       >> vertical_mode_active
       >> vertical_mode_armed
       >> m_master_fmc_vertical_mode_active_change_time_ms
       >> m_master_fmc_vs_mode_enabled
       >> m_master_fmc_fpa_mode_enabled
       >> m_master_fmc_vs_mode_active
       >> m_master_fmc_fpa_mode_active
       >> m_flightpath_angle
       >> m_vertical_speed
       >> m_mode_fl_change_active
       >> m_fl_change_cmd_vs
       >> m_do_set_vs_to_ap
       >> m_takeoff_active_lateral
       >> takeoff_lateral_mode_armed
       >> m_takeoff_active_vertical
       >> takeoff_vertical_mode_armed
       >> m_takeoff_lateral_target_track
       >> m_takeoff_vertical_speed_hold_kts;

    m_ils_mode = (ILS_MODE)ils_mode;
    m_lateral_mode_active = (LATERAL_MODE)lateral_mode_active;
    m_lateral_mode_armed = (LATERAL_MODE)lateral_mode_armed;
    m_vertical_mode_active = (VERTICAL_MODE)vertical_mode_active;
    m_vertical_mode_armed = (VERTICAL_MODE)vertical_mode_armed;
    m_takeoff_lateral_mode_armed = (LATERAL_MODE)takeoff_lateral_mode_armed;
    m_takeoff_vertical_mode_armed = (VERTICAL_MODE)takeoff_vertical_mode_armed;
}

/////////////////////////////////////////////////////////////////////////////

// End of file
