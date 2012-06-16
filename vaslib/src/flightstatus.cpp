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

/*! \file    flightstatus.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "navcalc.h"
#include "fsaccess.h"
#include "flightstatus.h"

/////////////////////////////////////////////////////////////////////////////

uint EngineData::m_smooth_delay_ms = 0;

const int FlightStatus::TO = 1;
const int FlightStatus::FROM = 2;

/////////////////////////////////////////////////////////////////////////////

FlightStatus::FlightStatus(uint smooth_delay_ms) : 
    smoothed_ias(false, false, 40, smooth_delay_ms, true),
    smoothed_altimeter_readout(false, false, 40, smooth_delay_ms),
    smoothed_vs(false, false, 40, smooth_delay_ms),
    pitch(false, false, 40, smooth_delay_ms, true),
    bank(false, false, 40, smooth_delay_ms, true),
    fpv_vertical(false, false, 40, smooth_delay_ms, true),
    nav1_bearing(true, false, 40, smooth_delay_ms), 
    nav2_bearing(true, false, 40, smooth_delay_ms), 
    adf1_bearing(true, false, 40, smooth_delay_ms), 
    adf2_bearing(true, false, 40, smooth_delay_ms),
    lat_smoothed(false, true, 40, smooth_delay_ms), 
    lon_smoothed(false, true, 40, smooth_delay_ms),
    m_true_heading(true, false, 40, smooth_delay_ms),
    m_magnetic_track(true, false, 40, smooth_delay_ms),
    m_fd_pitch(false, false, 40, smooth_delay_ms), 
    m_fd_pitch_input_from_external(true),
    m_fd_bank(false, false, 40, smooth_delay_ms), 
    m_fd_bank_input_from_external(true),
    m_fd_was_active(false)
{
    EngineData::m_smooth_delay_ms = smooth_delay_ms;

    smoothed_ias.doTrendLowPass(true, 0.6);
    m_fd_pitch.doLowPass(true, 0.5);
    m_fd_bank.doLowPass(true, 0.5);

//TODO for debugging
//     smoothed_ias.setName("IAS");
//     smoothed_altimeter_readout.setName("ALT");
//     smoothed_vs.setName("VS");
//     pitch.setName("Pitch");
//     bank.setName("Bank");
//     fpv_vertical.setName("FPV");
//     nav1_bearing.setName("NAV1");
//     nav2_bearing.setName("NAV2");
//     adf1_bearing.setName("ADF1");
//     adf2_bearing.setName("ADF2");
//     lat_smoothed.setName("LAT");
//     lon_smoothed.setName("LON");
//     m_true_heading.setName("THDG");
//     m_magnetic_track.setName("MTRACK");
//    m_fd_pitch.setName("FDPitch");
//    m_fd_bank.setName("FDBank");

    clear();

    m_ap_spd_read_delay_timer.start();
    m_ap_hdg_read_delay_timer.start();
    m_ap_alt_read_delay_timer.start();
    m_ap_vs_read_delay_timer.start();
}

/////////////////////////////////////////////////////////////////////////////

void FlightStatus::clear()
{
    invalidate();

    //-----

    smoothed_altimeter_readout.clear();
    m_true_heading.clear();
    m_magnetic_track.clear();
    m_magnetic_track_set = false;
    tas = 0;
    barber_pole_speed = 0.0;
    smoothed_ias.clear();
    smoothed_ias = 30.0;
    pitch.clear();
    bank.clear();
    fd_active = false;
    m_fd_pitch.clear();
    m_fd_bank.clear();
    fpv_vertical.clear();
    fpv_vertical_previous = 0.0;

    acceleration_pitch_deg_s2 = acceleration_roll_deg_s2 = acceleration_yaw_deg_s2 = 0.0;

    //-----

    alt_ft = 0;
    ground_alt_ft = 0;
    smoothed_vs.clear();
    lat = 0.0;
    lon = 0.0;
    magvar = 0.0;
    m_true_heading = 0.0;
    m_magnetic_track = 0.0;
    wind_speed_kts = 0;
    wind_dir_deg_true = 0;
    view_dir_deg = -1;
    paused = false;
    fs_utc_time = QTime();
    fs_utc_date = QDate();
    tat = 0.0;
    sat = 0.0;
    oat = 0.0;
    dew = 0.0;
    qnh = 0.0;
    delta = 0.0;
    theta = 0.0;
    onground = true;
    slew = false;
    mach = 0;
    stall = 0;

    current_flap_lever_notch = flaps_lever_notch_count = 0;
    slats_degrees = flaps_degrees = flaps_percent_left = flaps_percent_right = 0.0;
    m_last_flaps_percent_left = m_last_flaps_percent_right = 0.0;

    speed_vs0_kts = speed_vs1_kts = speed_vc_kts = speed_min_drag_kts = 0;
    slip_percent = 0.0;
    total_fuel_capacity_kg = zero_fuel_weight_kg = total_weight_kg = 0;

    brake_left_percent = brake_right_percent = parking_brake_set = 0.0;
    spoilers_armed = false;
    spoiler_lever_percent = spoiler_left_percent = spoiler_right_percent = 0.0;

    battery_on = avionics_on = lights_landing = lights_strobe = lights_beacon = false;    
    lights_taxi = lights_navigation = lights_instruments = false;

    gear_nose_position_percent = gear_left_position_percent = gear_right_position_percent = 0.0;

    ap_available = false;
    ap_enabled = false;
    ap_hdg_lock = false;
    ap_alt_lock = false;
    ap_vs_lock = false;
    ap_speed_lock = false;
    ap_mach_lock = false;

    ap_nav1_lock = false;
    ap_gs_lock = false;
    ap_app_lock = false;
    ap_app_bc_lock = false;
    at_toga = false;
    at_arm = false;

    gps_enabled = false;

    nav1 = Waypoint();
    nav1_freq = 0;
    nav1_has_loc = false;
    nav1_loc_mag_heading = 0;
    nav1_bearing.clear();
    nav1_distance_nm = QString::null;
    obs1 = 0;
    obs1_loc_needle = 0;
    obs1_gs_needle = 0;
    obs1_to_from = 0;

    nav2 = Waypoint();
    nav2_freq = 0;
    nav2_has_loc = false;
    nav2_bearing.clear();
    nav2_distance_nm = QString::null;
    obs2 = 0;
    obs2_to_from = 0;
    obs2_loc_needle = 0;
    obs2_gs_needle = 0;

    adf1 = Ndb();
    adf1_bearing.clear();
    
    adf2 = Ndb();
    adf2_bearing.clear();

    outer_marker = false;
    middle_marker = false;;
    inner_marker = false;;

    wind_correction_angle_deg = 0.0;
    ground_speed_kts = 0.0;
    current_position_raw = Waypoint();

    lat_smoothed.clear();
    lon_smoothed.clear();
    current_position_smoothed = Waypoint();

    avionics_status = 0;
    aircraft_type.clear();

    fsctrl_nd_left_list.clear();
    fsctrl_pfd_left_list.clear();
    fsctrl_cdu_left_list.clear();
    fsctrl_cdu_right_list.clear();
    fsctrl_fmc_list.clear();
    fsctrl_ecam_list.clear();
    fsctrl_fcu_list.clear();
    fsctrl_nd_right_list.clear();
    fsctrl_pfd_right_list.clear();

    nr_of_engines = 0;
    engine_type = ENGINE_TYPE_INVALID;
    engine_ignition_on = false;

    rudder_percent = aileron_percent = elevator_percent = elevator_trim_percent = elevator_trim_degrees = 0.0;
    rudder_input_percent = aileron_input_percent = elevator_input_percent = 0.0;
    //elevator_trim_input_percent = 0.0;

    m_ap_spd = m_ap_hdg = m_ap_alt = m_ap_vs = 0;
    m_ap_mach = 0.0;
    m_altimeter_pressure_setting_hpa = 0.0;

    doors_open = pitot_heat_on = false;
    m_flaps_transit_timer = QTime::currentTime().addSecs(-10);
    pushback_status = FSAccess::PUSHBACK_STOP;
    //TODOtime_of_day = TIME_OF_DAY_INVALID;

    no_smoking_sign = seat_belt_sign = false;
}

/////////////////////////////////////////////////////////////////////////////

QString FlightStatus::toString() const
{
    return QString("valid=%1/paused=%2/lat=%3/lon=%4/thdg=%5/ias=%6").
        arg(m_valid).arg(paused).arg(lat).arg(lon).arg(m_true_heading.value()).arg(smoothed_ias.value());
}

/////////////////////////////////////////////////////////////////////////////

void FlightStatus::recalc()
{
    // setup current position
    current_position_raw = Waypoint("FSPOS", "", lat, lon);
    lat_smoothed = lat;
    lon_smoothed = lon;
    updateSmoothedPosition();

    // calc wind correction
    
    double eff_wind_speed = wind_speed_kts;
    if (tas < 30.0 || onground) 
    {
        eff_wind_speed = 0.0;
        wind_correction_angle_deg = 0.0;
    }

    double calculated_gs;
    Navcalc::getWindCorrAngleAndGS(tas, smoothedTrueHeading(), eff_wind_speed, wind_dir_deg_true, 
                                   wind_correction_angle_deg, calculated_gs);
    
    if (onground && ground_speed_kts < 30) tas = ground_speed_kts;

    if (flaps_percent_right != m_last_flaps_percent_right ||
        flaps_percent_left != m_last_flaps_percent_left) m_flaps_transit_timer.start();

    m_last_flaps_percent_left = flaps_percent_left;
    m_last_flaps_percent_right = flaps_percent_right;

    fpv_vertical_previous = fpv_vertical.lastValue();
    fpv_vertical = Navcalc::getFlightPath(smoothedVS(), ground_speed_kts);

    if (fd_active)
    {
        if (!m_fd_was_active)
        {
            m_fd_was_active = true;
            resetFlightDirector();
        }
    }
    else
    {
        m_fd_was_active = false;
    }
}

/////////////////////////////////////////////////////////////////////////////

double FlightStatus::maxN1OfAllEngines() const
{
    double max_n1 = 0.0;
    for(int i=1; i <= nr_of_engines; ++i) max_n1 = qMax(max_n1, engine_data[i].smoothed_n1.lastValue());
    return max_n1;
}

/////////////////////////////////////////////////////////////////////////////

bool FlightStatus::localizerEstablished() const
{
    return (nav1_has_loc && !nav1.id().isEmpty() && (ap_app_lock || ap_nav1_lock) && qAbs(obs1_loc_needle) < 50);
}

/////////////////////////////////////////////////////////////////////////////

bool FlightStatus::localizerAlive() const
{
    return (nav1_has_loc && !nav1.id().isEmpty() && qAbs(obs1_loc_needle) < 110);
}

/////////////////////////////////////////////////////////////////////////////

bool FlightStatus::glideslopeAlive() const
{
    return (nav1_has_loc && !nav1.id().isEmpty() && qAbs(obs1_gs_needle) < 110);
}

/////////////////////////////////////////////////////////////////////////////

void FlightStatus::setFlightDirectorPitchInternal(const double& pitch)
{
    if (m_fd_pitch_input_from_external) return;
    //Logger::log(QString("FlightStatus:setFlightDirectorPitchInternal: %1").arg(pitch));
    m_fd_pitch = pitch;
}

/////////////////////////////////////////////////////////////////////////////

void FlightStatus::setFlightDirectorPitchExternal(const double& pitch)
{
    if (!m_fd_pitch_input_from_external) return;
    //Logger::log(QString("FlightStatus:setFlightDirectorPitchExternal: %1").arg(pitch));
    m_fd_pitch = pitch;
}

/////////////////////////////////////////////////////////////////////////////

void FlightStatus::setFlightDirectorBankInternal(const double& bank)
{
    if (m_fd_bank_input_from_external) return;
    //Logger::log(QString("FlightStatus:setFlightDirectorBankInternal: %1").arg(bank));
    m_fd_bank = bank;
}

/////////////////////////////////////////////////////////////////////////////

void FlightStatus::setFlightDirectorBankExternal(const double& bank)
{
    if (!m_fd_bank_input_from_external) return;
    //Logger::log(QString("FlightStatus:setFlightDirectorBankExternal: %1").arg(bank));
    m_fd_bank = bank;
}

// End of file
