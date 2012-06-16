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

/*! \file    flight_mode_tracker.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/


#include "flight_mode_tracker.h"

#include "logger.h"
#include "assert.h"

#include "flightstatus.h"
#include "fmc_data_provider.h"
#include "flightroute.h"

/////////////////////////////////////////////////////////////////////////////

FlightModeTracker::FlightModeTracker(const FlightStatus* flightstatus, const FMCDataProvider* fmc_data_provider) :
    m_flightstatus(flightstatus), m_fmc_data_provider(fmc_data_provider), 
    m_current_flight_mode(FLIGHT_MODE_UNKNOWN), m_prev_flight_mode(FLIGHT_MODE_UNKNOWN)
{
    MYASSERT(m_flightstatus != 0);
    MYASSERT(m_fmc_data_provider != 0);

    m_check_timer.start();
}

/////////////////////////////////////////////////////////////////////////////

FlightModeTracker::~FlightModeTracker()
{
}

/////////////////////////////////////////////////////////////////////////////

QString FlightModeTracker::flightModeText(FLIGHTMODE mode) const
{
    switch(mode)
    {
        case(FLIGHT_MODE_UNKNOWN):   return "UNKNOWN";
        case(FLIGHT_MODE_PREFLIGHT): return "PREFLIGHT";
        case(FLIGHT_MODE_TAXIING):    return "TAXIING";
        case(FLIGHT_MODE_TAKEOFF):   return "TAKEOFF";
        case(FLIGHT_MODE_CLIMB):     return "CLIMB";
        case(FLIGHT_MODE_CRUISE):    return "CRUISE";
        case(FLIGHT_MODE_DESCENT):   return "DESCENT";
        case(FLIGHT_MODE_APPROACH):  return "APPROACH";
        case(FLIGHT_MODE_LANDING):   return "LANDING";
        case(FLIGHT_MODE_GOAROUND):  return "GOAROUND";
    }

    return "INVALID";
}

/////////////////////////////////////////////////////////////////////////////

void FlightModeTracker::slotCheckAndSetFlightMode()
{
    if (!m_flightstatus->isValid() || m_flightstatus->slew || m_flightstatus->paused) return;

    // only call the check routine 10 times per second at max.
    if (m_current_flight_mode != FLIGHT_MODE_UNKNOWN && m_check_timer.elapsed() < 100) return;
    m_check_timer.start();

    FLIGHTMODE prev_mode = m_current_flight_mode;

    if (m_flightstatus->onground)
    {
        if (!m_flightstatus->isAtLeastOneEngineOn())
        {
            m_current_flight_mode = FLIGHT_MODE_PREFLIGHT;
        }
        else
        {
            if (prev_mode == FLIGHT_MODE_LANDING)
            {
                if (m_flightstatus->ground_speed_kts < 30)
                    m_current_flight_mode = FLIGHT_MODE_TAXIING;
            }
            else
            {
                if (m_flightstatus->ground_speed_kts <= 90 && m_flightstatus->maxN1OfAllEngines() <= 85.0)
                    m_current_flight_mode = FLIGHT_MODE_TAXIING;
                else if (prev_mode == FLIGHT_MODE_APPROACH || m_flightstatus->flapsPercent() > 99) 
                    m_current_flight_mode = FLIGHT_MODE_LANDING;
                else
                    m_current_flight_mode = FLIGHT_MODE_TAKEOFF;
            }
        }
    }
    else
    {
        if (m_fmc_data_provider->approachPhaseActive())
        {
            m_current_flight_mode = FLIGHT_MODE_APPROACH;
        }
        else if (m_flightstatus->radarAltitude() < 500)
        {
            if (prev_mode == FLIGHT_MODE_DESCENT || m_flightstatus->smoothed_vs.lastValue() < -100) 
                m_current_flight_mode = FLIGHT_MODE_APPROACH;
            else if (prev_mode != FLIGHT_MODE_APPROACH)
                m_current_flight_mode = FLIGHT_MODE_CLIMB;
        }
        else if (qAbs(m_flightstatus->smoothed_altimeter_readout.lastValue() - 
                      m_fmc_data_provider->normalRoute().cruiseFl() * 100.0) < 250)
        {
            m_current_flight_mode = FLIGHT_MODE_CRUISE;
        }
        else if ((!m_flightstatus->isGearUp() || !m_flightstatus->flapsAreUp()) && prev_mode == FLIGHT_MODE_DESCENT)
        {
            m_current_flight_mode = FLIGHT_MODE_APPROACH;
        }
        else
        {
            if (m_flightstatus->smoothed_vs.lastValue() < -400)
            {
                if (prev_mode != FLIGHT_MODE_APPROACH) m_current_flight_mode = FLIGHT_MODE_DESCENT;
            }
            else if (m_flightstatus->smoothed_vs.lastValue() > 400)
            {
                m_current_flight_mode = FLIGHT_MODE_CLIMB;
            }
            else
            {
                if (prev_mode == FLIGHT_MODE_TAKEOFF || prev_mode == FLIGHT_MODE_LANDING) 
                    m_current_flight_mode = FLIGHT_MODE_CLIMB;
                else if (prev_mode == FLIGHT_MODE_CRUISE)
                    m_current_flight_mode = FLIGHT_MODE_DESCENT;
            }
        }

        if (m_current_flight_mode == FLIGHT_MODE_UNKNOWN) m_current_flight_mode = FLIGHT_MODE_CLIMB;
    }

    if (prev_mode != m_current_flight_mode)
    {
        m_prev_flight_mode = prev_mode;

        // determine OOOI times

        switch(m_current_flight_mode)
        {
            case(FLIGHT_MODE_TAXIING): {
                if (m_prev_flight_mode == FLIGHT_MODE_PREFLIGHT) 
                {
                    m_time_out = m_flightstatus->fs_utc_time;
                    m_time_off = m_time_on = m_time_in = QTime();
                }
                break;
            }
            case(FLIGHT_MODE_TAKEOFF): {
                m_time_off = m_flightstatus->fs_utc_time;
                break;
            }
            case(FLIGHT_MODE_LANDING): {
                m_time_on = m_flightstatus->fs_utc_time;
                break;
            }
            case(FLIGHT_MODE_PREFLIGHT): {
                if (m_prev_flight_mode == FLIGHT_MODE_TAXIING) m_time_in = m_flightstatus->fs_utc_time;
                break;
            }
            default: 
                break;
        }

        emit signalFlightModeChanged(m_current_flight_mode);
    }
}

// End of file
