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

/*! \file    fmc_flightstatus_checker_base.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "fmc_control.h"
#include "flight_mode_tracker.h"

#include "fmc_flightstatus_checker_base.h"

/////////////////////////////////////////////////////////////////////////////

FlightStatusCheckerBase::FlightStatusCheckerBase(FlightStatus* flightstatus, const FMCControl* fmc_control) : 
    m_init(false), m_flightstatus(flightstatus), m_fmc_control(fmc_control),
    m_last_ap_alt(999999), m_is_altitude_deviation(false),
    m_altitude_deviation_ft(0.0), m_abs_alt_diff(0.0), m_do_1000_to_go_callout(false),
    m_was_airborne_since_last_engine_off(false), m_was_above_1000ft_since_last_onground_taxi(false),
    m_thrust_lever_climb_detent_request(false), m_altimeter_has_wrong_setting_left(false),
    m_altimeter_has_wrong_setting_right(false), m_was_approaching_and_spoilers_armed(false),
    m_spoilers_are_managed(false)
{
    MYASSERT(m_flightstatus != 0);
    MYASSERT(m_fmc_control != 0);
    m_ap_alt_last_change_time.start();
}

/////////////////////////////////////////////////////////////////////////////

void FlightStatusCheckerBase::beforeChecks(FSAccess& fsaccess)
{
    if (!m_flightstatus->isValid() || m_flightstatus->paused || m_flightstatus->slew) return;

    if (!m_init)
    {
        m_init = true;
        m_was_airborne_since_last_engine_off =  !m_flightstatus->onground;
        m_was_above_1000ft_since_last_onground_taxi = m_flightstatus->radarAltitude() > 1000;
    }
    
    //-----

    if (m_flightstatus->onground && m_flightstatus->smoothed_ias.lastValue() < 40.0)
        m_do_1000_to_go_callout = true;

    if (m_last_ap_alt != m_flightstatus->APAlt())
    {
        m_ap_alt_last_change_time.start();
        m_do_1000_to_go_callout = true;
    }

    m_abs_alt_diff = qAbs(m_flightstatus->APAlt() - m_flightstatus->smoothed_altimeter_readout.lastValue());

    //------

    if (!m_flightstatus->isAtLeastOneEngineOn()) 
    {
        m_time_since_last_engine_start.start();
    }

    if (m_flightstatus->onground)
    {
        if (!m_flightstatus->isAtLeastOneEngineOn()) m_was_airborne_since_last_engine_off = false;
        if (m_flightstatus->smoothed_ias.lastValue() < 40) m_was_above_1000ft_since_last_onground_taxi = false;
    }
    else
    {
        m_was_airborne_since_last_engine_off = true;
        if (m_flightstatus->radarAltitude() > 1000) m_was_above_1000ft_since_last_onground_taxi = true;
    }

    //----- check altimeter setting left

    double alt_readout = m_flightstatus->smoothed_altimeter_readout.lastValue();
    
    if (!m_fmc_control->isAltimeterSetToSTD(true))
    {
        if (m_fmc_control->flightModeTracker().isClimb() &&
            m_fmc_control->fmcData().transitionAltitudeAdep() > 0 &&
            alt_readout > m_fmc_control->fmcData().transitionAltitudeAdep())
        {
            m_altimeter_has_wrong_setting_left = true;
        }
        else
        {
            m_altimeter_has_wrong_setting_left = false;
        }
    }
    else
    {
        if (m_flightstatus->radarAltitude() < 2000 ||
            (m_fmc_control->fmcData().transitionLevelAdes() > 0 &&
             (m_fmc_control->flightModeTracker().isDescent() || m_fmc_control->flightModeTracker().isApproach()) &&
             alt_readout < m_fmc_control->fmcData().transitionLevelAdes()*100))
        {
            m_altimeter_has_wrong_setting_left = true;
        }
        else
        {
            m_altimeter_has_wrong_setting_left = false;
        }
    }

    //----- check altimeter setting right

    if (!m_fmc_control->isAltimeterSetToSTD(false))
    {
        if (m_fmc_control->flightModeTracker().isClimb() &&
            m_fmc_control->fmcData().transitionAltitudeAdep() > 0 &&
            alt_readout > m_fmc_control->fmcData().transitionAltitudeAdep())
        {
            m_altimeter_has_wrong_setting_right = true;
        }
        else
        {
            m_altimeter_has_wrong_setting_right = false;
        }
    }
    else
    {
        if (m_flightstatus->radarAltitude() < 2000 ||
            (m_fmc_control->fmcData().transitionLevelAdes() > 0 &&
             (m_fmc_control->flightModeTracker().isDescent() || m_fmc_control->flightModeTracker().isApproach()) &&
             alt_readout < m_fmc_control->fmcData().transitionLevelAdes()*100))
        {
            m_altimeter_has_wrong_setting_right = true;
        }
        else
        {
            m_altimeter_has_wrong_setting_right = false;
        }
    }

    //----- check armed spoilers

    switch(m_fmc_control->flightModeTracker().currentFlightMode())
    {
        case(FlightModeTracker::FLIGHT_MODE_APPROACH):
            m_was_approaching_and_spoilers_armed = m_flightstatus->spoilers_armed;
            break;

        case(FlightModeTracker::FLIGHT_MODE_UNKNOWN): 
        case(FlightModeTracker::FLIGHT_MODE_PREFLIGHT): 
        case(FlightModeTracker::FLIGHT_MODE_TAXIING):
        case(FlightModeTracker::FLIGHT_MODE_TAKEOFF): 
        case(FlightModeTracker::FLIGHT_MODE_CLIMB):   
        case(FlightModeTracker::FLIGHT_MODE_CRUISE): 
        case(FlightModeTracker::FLIGHT_MODE_DESCENT): 
        case(FlightModeTracker::FLIGHT_MODE_GOAROUND):
            m_was_approaching_and_spoilers_armed = false;
            break;

        case(FlightModeTracker::FLIGHT_MODE_LANDING): {

            if (m_was_approaching_and_spoilers_armed)
            {
                if (m_flightstatus->spoiler_left_percent < 10.0 || m_flightstatus->spoiler_right_percent < 10.0)
                    Logger::log("FlightStatusCheckerBase:beforeChecks: deploying SPOILERS on landing");

                fsaccess.setSpoiler(100.0);
                m_spoilers_are_managed = true;
            }
            break; 
        }
    }

    if (m_spoilers_are_managed && 
        (!m_fmc_control->flightModeTracker().isLanding() ||
         (m_fmc_control->flightModeTracker().isLanding() && m_flightstatus->smoothed_ias.lastValue() <= 50) ||
         (m_flightstatus->getMaximumThrottleLeverInputPercent() > 10.0 && !m_flightstatus->isReverserOn())))
    {
        if (m_flightstatus->spoiler_left_percent > 5.0 || m_flightstatus->spoiler_right_percent > 5.0)
            Logger::log("FlightStatusCheckerBase:beforeChecks: retracting SPOILERS");

        fsaccess.setSpoiler(0.0);
        fsaccess.freeSpoilerAxis();
        m_spoilers_are_managed = false;
        m_was_approaching_and_spoilers_armed = false;
    }
}

/////////////////////////////////////////////////////////////////////////////

void FlightStatusCheckerBase::doChecks(FSAccess& fsaccess)
{
    beforeChecks(fsaccess);
    afterChecks(fsaccess);
}

/////////////////////////////////////////////////////////////////////////////

void FlightStatusCheckerBase::afterChecks(FSAccess& fsaccess)
{
    //----- check altitude reach

    if (m_do_1000_to_go_callout && !m_flightstatus->onground &&
        m_abs_alt_diff <= 1000 && m_abs_alt_diff > 900 && m_ap_alt_last_change_time.elapsed() > 1000)
    {
        if ((m_flightstatus->APAlt() > m_flightstatus->smoothed_altimeter_readout.lastValue() && 
             m_flightstatus->smoothed_vs.lastValue() > 100.0) ||
            (m_flightstatus->APAlt() < m_flightstatus->smoothed_altimeter_readout.lastValue() && 
             m_flightstatus->smoothed_vs.lastValue() < 100.0))
        {
            Logger::log("FlightStatusCheckerStyleA:doChecks: trigger 1000 ft to go");
            emit signal1000FtToGo();
            m_do_1000_to_go_callout = false;
        }
    }

    m_last_ap_alt = m_flightstatus->APAlt();
}

// End of file
