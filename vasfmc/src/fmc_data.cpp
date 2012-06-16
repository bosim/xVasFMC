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

#include <QObject>

#include "assert.h"
#include "config.h"
#include "logger.h"
#include "waypoint.h"

#include "flightstatus.h"

#include "defines.h"
#include "fmc_data.h"

/////////////////////////////////////////////////////////////////////////////

QString FMCData::CHANGE_FLAG_FMC_DATA = "FMCData";

/////////////////////////////////////////////////////////////////////////////

FMCData::FMCData(Config* cfg, const FlightStatus* flightstatus) : 
    m_cfg(cfg), m_flightstatus(flightstatus), m_normal_route(flightstatus), m_alternate_route(flightstatus), 
    m_secondary_route(flightstatus), m_temporary_route(flightstatus),
    m_distance_to_active_wpt_nm(0.0), m_true_track_to_active_wpt(0.0), m_hours_to_active_wpt(0.0), 
    m_distance_from_previous_wpt_nm(0.0), m_cross_track_distance_nm(0.0), m_distance_to_tod_nm(0.0),
    m_turn_radius_nm(0.0), m_v1_kts(0), m_vr_kts(0), m_v2_kts(0), m_takeoff_trim(0.0), 
    m_takeoff_flaps_notch(1), m_landing_flaps_notch(99), m_trans_altitude_adep(0), m_trans_level_ades(0), 
    m_fmc_takeoff_thrust_n1(90.5), 
    m_fmc_climb_thrust_n1_initial(84.0), m_fmc_climb_thrust_n1_maximum(86.0), m_climb_speed_kts(250), 
    m_climb_mach(0.78), m_cruise_mach(0.78), m_descent_mach(0.78), m_descent_speed_kts(280), 
    m_approach_speed_kts(0), m_minimum_descent_altitude_ft(0), m_decision_height_ft(0), m_approach_phase_active(false)
{
    MYASSERT(m_cfg != 0);
    MYASSERT(m_flightstatus != 0);

    m_normal_route.setFlag(Route::FLAG_NORMAL, true);
    m_alternate_route.setFlag(Route::FLAG_ALTERNATE, true);
    m_secondary_route.setFlag(Route::FLAG_SECONDARY, true);
    m_temporary_route.setFlag(Route::FLAG_TEMPORARY, true);
 
    m_change_delay_timer.setSingleShot(true);
    MYASSERT(connect(&m_change_delay_timer, SIGNAL(timeout()), this, SLOT(slotEmitDataChanged())));

    MYASSERT(connect(&m_normal_route, SIGNAL(signalChanged(const QString&, bool, const QString&)), 
                     this, SLOT(slotRouteChanged(const QString&, bool, const QString&))));
    MYASSERT(connect(&m_alternate_route, SIGNAL(signalChanged(const QString&, bool, const QString&)), 
                     this, SLOT(slotRouteChanged(const QString&, bool, const QString&))));
    MYASSERT(connect(&m_secondary_route, SIGNAL(signalChanged(const QString&, bool, const QString&)), 
                     this, SLOT(slotRouteChanged(const QString&, bool, const QString&))));
    MYASSERT(connect(&m_temporary_route, SIGNAL(signalChanged(const QString&, bool, const QString&)), 
                     this, SLOT(slotRouteChanged(const QString&, bool, const QString&))));

    MYASSERT(connect(&m_normal_route, SIGNAL(signalDestinationAirportChanged()),
                     this, SLOT(slotDestinationAirportChanged())));
}

/////////////////////////////////////////////////////////////////////////////

FMCData::~FMCData() {}

/////////////////////////////////////////////////////////////////////////////

void FMCData::clear()
{
    m_normal_route.clear();
    m_alternate_route.clear();
    m_secondary_route.clear();
    m_temporary_route.clear();

    m_v1_kts = 0;
    m_vr_kts = 0;
    m_v2_kts = 0;
    m_takeoff_trim = 0.0;
    m_takeoff_flaps_notch = 1;
    m_landing_flaps_notch = 5;
    m_trans_altitude_adep = 0;
    m_trans_level_ades = 0;

    m_fmc_takeoff_thrust_n1 = 90.5;
    m_fmc_climb_thrust_n1_initial = 84;
    m_fmc_climb_thrust_n1_maximum = 86;
    m_climb_speed_kts = 250;
    m_climb_mach = 0.78;
    m_cruise_mach = 0.78;
    m_descent_mach = 0.78;
    m_descent_speed_kts = 280;
    m_approach_speed_kts = 0;
    m_minimum_descent_altitude_ft = 0;
    m_decision_height_ft = 0;
    m_approach_phase_active = false;

    m_distance_to_active_wpt_nm = 0;
    m_true_track_to_active_wpt = 0;
    m_hours_to_active_wpt = 0;
    m_distance_from_previous_wpt_nm = 0;
    m_cross_track_distance_nm = 0;
 	m_distance_to_tod_nm = 0.0;

    emit signalDataChanged(CHANGE_FLAG_FMC_DATA, true, "FMCData:clear");
}

/////////////////////////////////////////////////////////////////////////////

void FMCData::setApproachPhaseActive(bool yes)
{
    if (yes && m_flightstatus->onground) return;
    m_approach_phase_active = yes;
    emit signalDataChanged(
        CHANGE_FLAG_FMC_DATA, true, 
        QString("FMCData:setApproachPhaseActive(appr=%1)").arg(yes));
    if (yes) { emit signalApproachPhaseActivated(); }
}

/////////////////////////////////////////////////////////////////////////////

void FMCData::slotDestinationAirportChanged()
{
    Logger::log("FMCData:slotDestinationAirportChanged");
    setApproachPhaseActive(false);
}

/////////////////////////////////////////////////////////////////////////////

bool FMCData::isOnCruisFlightlevel() const
{
    return 
        m_normal_route.cruiseFl() > 1 &&
        qAbs(m_flightstatus->smoothed_altimeter_readout.lastValue() - (100 * m_normal_route.cruiseFl())) < 100;
}

/////////////////////////////////////////////////////////////////////////////

void FMCData::setTakeoffFlapsNotch(uint takeoff_flaps_notch)
{
    m_takeoff_flaps_notch = qMin(takeoff_flaps_notch, m_flightstatus->flaps_lever_notch_count-1);
    emit signalDataChanged(CHANGE_FLAG_FMC_DATA, true, "FMCData:setTakeoffFlapsNotch");
}

/////////////////////////////////////////////////////////////////////////////

void FMCData::setLandingFlapsNotch(uint landing_flaps_notch)
{
    m_landing_flaps_notch = qMin(landing_flaps_notch, m_flightstatus->flaps_lever_notch_count-1);
    emit signalDataChanged(CHANGE_FLAG_FMC_DATA, true, "FMCData:setLandingFlapsNotch");
}

/////////////////////////////////////////////////////////////////////////////

void FMCData::operator>>(QDataStream& out) const
{
    out << m_v1_kts
        << m_vr_kts
        << m_v2_kts
        << m_takeoff_trim
        << m_takeoff_flaps_notch
        << m_landing_flaps_notch
        << m_trans_altitude_adep
        << m_trans_level_ades
        << m_fmc_takeoff_thrust_n1
        << m_fmc_climb_thrust_n1_initial
        << m_fmc_climb_thrust_n1_maximum
        << m_climb_speed_kts
        << m_climb_mach
        << m_cruise_mach
        << m_descent_mach
        << m_descent_speed_kts
        << m_approach_speed_kts
        << m_minimum_descent_altitude_ft
        << m_decision_height_ft
        << m_approach_phase_active;
}

/////////////////////////////////////////////////////////////////////////////

void FMCData::operator<<(QDataStream& in)
{
    in  >> m_v1_kts
        >> m_vr_kts
        >> m_v2_kts
        >> m_takeoff_trim
        >> m_takeoff_flaps_notch
        >> m_landing_flaps_notch
        >> m_trans_altitude_adep
        >> m_trans_level_ades
        >> m_fmc_takeoff_thrust_n1
        >> m_fmc_climb_thrust_n1_initial
        >> m_fmc_climb_thrust_n1_maximum
        >> m_climb_speed_kts
        >> m_climb_mach
        >> m_cruise_mach
        >> m_descent_mach
        >> m_descent_speed_kts
        >> m_approach_speed_kts
        >> m_minimum_descent_altitude_ft
        >> m_decision_height_ft
        >> m_approach_phase_active;

    emit signalDataChanged(CHANGE_FLAG_FMC_DATA, false, "FMCData:operator<<");
}

/////////////////////////////////////////////////////////////////////////////
