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

/*! \file    fmc_flighstatus_checker_style_a.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "fmc_control.h"
#include "fmc_autothrottle.h"

#include "fmc_flightstatus_checker_style_a.h"

/////////////////////////////////////////////////////////////////////////////

FlightStatusCheckerStyleA::FlightStatusCheckerStyleA(FlightStatus* flightstatus, const FMCControl* fmc_control) : 
    FlightStatusCheckerBase(flightstatus, fmc_control),
    m_ap_alt_was_intercepted_within_750ft(false), m_ap_alt_was_intercepted_within_250ft(false),
    m_last_flaps_lever_notch(1000)
{

}

/////////////////////////////////////////////////////////////////////////////

void FlightStatusCheckerStyleA::doChecks(FSAccess& fsaccess)
{
    if (!m_flightstatus->isValid() || m_flightstatus->paused || m_flightstatus->slew) return;

    FlightStatusCheckerBase::beforeChecks(fsaccess);

    //----- check altitudes

    if (m_last_ap_alt != m_flightstatus->APAlt())
    {
        // the pilot has selected a new AP ALT
        m_ap_alt_was_intercepted_within_250ft = (m_abs_alt_diff <= 250);
        m_ap_alt_was_intercepted_within_750ft = (m_abs_alt_diff <= 750);
    }

    //----- check altitude deviation

    m_is_altitude_deviation = false;

    if (!m_flightstatus->isGearDown() && !m_flightstatus->ap_gs_lock)
    {
        if (m_last_ap_alt == m_flightstatus->APAlt())
        {
            if (m_abs_alt_diff <= 250)
            {
                m_ap_alt_was_intercepted_within_250ft = true;
                m_ap_alt_was_intercepted_within_750ft = true;
            }
            else if (m_abs_alt_diff <= 750)
            {
                if (!m_ap_alt_was_intercepted_within_750ft && !m_flightstatus->ap_alt_lock) emit signalReachingAltitude();

                if (m_ap_alt_was_intercepted_within_250ft) m_is_altitude_deviation = true;
                m_ap_alt_was_intercepted_within_750ft = true;
            }
            else
            {
                if (m_ap_alt_was_intercepted_within_750ft) m_is_altitude_deviation = true;
            }
        }
    }
    else
    {
        m_ap_alt_was_intercepted_within_250ft = false;
        m_ap_alt_was_intercepted_within_750ft = false;
    }

    if (m_is_altitude_deviation)
        m_altitude_deviation_ft = m_abs_alt_diff;
    else
        m_altitude_deviation_ft = 0.0;

    //----- flap handling

    uint next_flaps_lever_notch = m_flightstatus->current_flap_lever_notch;

    if (m_fmc_control->isAirbusFlapsHandlingModeEnabled())
    {
        if (m_last_flaps_lever_notch == 0 && m_flightstatus->current_flap_lever_notch == 1)
        {
            if (m_flightstatus->smoothed_ias.lastValue() <= 100.0)
            {
                fsaccess.setFlaps(2);
                next_flaps_lever_notch = 2;
            }
        }
        else if (m_last_flaps_lever_notch == 1 && m_flightstatus->current_flap_lever_notch == 2)
        {
            fsaccess.setFlaps(3);
            next_flaps_lever_notch = 3;
        }
        else if (m_last_flaps_lever_notch == 2 && m_flightstatus->current_flap_lever_notch == 1)
        {
            fsaccess.setFlaps(0);
            next_flaps_lever_notch = 0;
        }
        else if (m_last_flaps_lever_notch == 3 && m_flightstatus->current_flap_lever_notch == 2)
        {
            if (m_flightstatus->smoothed_ias.lastValue() > 210)
            {
                fsaccess.setFlaps(1);
                next_flaps_lever_notch = 1;
            }
        }
        else if (m_flightstatus->current_flap_lever_notch == 2 &&
                 m_flightstatus->smoothed_ias.lastValue() >= 210) 
        {
            fsaccess.setFlaps(1);
            next_flaps_lever_notch = 1;
        }
    }
    
    m_last_flaps_lever_notch = next_flaps_lever_notch;

    //----- climb mode check

    if (m_fmc_control->fmcAutothrottle().useAirbusThrottleModes() &&
        m_fmc_control->fmcAutothrottle().isAPThrottleEngaged() &&
        !m_flightstatus->onground &&
        m_fmc_control->fmcAutothrottle().currentAirbusThrottleMode() != FMCAutothrottle::AIRBUS_THROTTLE_CLIMB &&
        ((m_fmc_control->normalRoute().thrustReductionAltitudeFt() > 0 &&
          m_flightstatus->smoothed_altimeter_readout.lastValue() > m_fmc_control->normalRoute().thrustReductionAltitudeFt()) 
         ||
         (m_fmc_control->normalRoute().thrustReductionAltitudeFt() <= 0 && 
          m_flightstatus->radarAltitude() > 1000)))
    {
        m_thrust_lever_climb_detent_request = true;
    }
    else
    {
        m_thrust_lever_climb_detent_request = false;
    }

    //-----

    FlightStatusCheckerBase::afterChecks(fsaccess);
}

// End of file
