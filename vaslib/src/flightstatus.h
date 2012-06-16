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

#ifndef FLIGHTSTATUS_H
#define FLIGHTSTATUS_H

#include <QString>
#include <QDateTime>
#include <QMap>
#include <QTime>

#include "logger.h"
#include "waypoint.h"
#include "ils.h"
#include "navcalc.h"
#include "smoothing.h"

/////////////////////////////////////////////////////////////////////////////

class TcasEntry
{
public:

    TcasEntry() : m_valid(false), m_id(0), m_altitude_ft(0), m_true_heading(0), m_groundspeed_kts(0), m_vs_fpm(0) {};
    virtual ~TcasEntry() {};

public:

    bool m_valid;
    int m_id;
    Waypoint m_position;
    int m_altitude_ft;
    int m_true_heading;
    int m_groundspeed_kts;
    int m_vs_fpm;
};

typedef QList<TcasEntry> TcasEntryValueList;
typedef QListIterator<TcasEntry> TcasEntryValueListIterator;

/////////////////////////////////////////////////////////////////////////////

//! EngineData
class EngineData
{
public:
    //! Standard Constructor
    EngineData() :
        smoothed_n1(false, false, 10, m_smooth_delay_ms, true),
        n2_percent(0.0), egt_degrees(0), ff_kg_per_hour(0), anti_ice_on(false),
        throttle_lever_percent(0.0), reverser_percent(0.0), throttle_input_percent(0.0)
    {}

    //! Destructor
    virtual ~EngineData() {}

    inline double smoothedN1(double* trend_per_s = 0) const { return smoothed_n1.value(trend_per_s); }

public:

    SmoothedValueWithDelay<double> smoothed_n1;
    double n2_percent;
    double egt_degrees;
    double ff_kg_per_hour;

    bool anti_ice_on;
    double throttle_lever_percent;
    double reverser_percent;
    
    double throttle_input_percent;

    static uint m_smooth_delay_ms;
};

/////////////////////////////////////////////////////////////////////////////

class FlightStatus
{
public:

    enum ENGINE_TYPE { ENGINE_TYPE_INVALID = 0,
                       ENGINE_TYPE_PISTON,
                       ENGINE_TYPE_TURBOPROP,
                       ENGINE_TYPE_JET
    };

//TODO
//     enum TIME_OF_DAY { TIME_OF_DAY_INVALID = 0,
//                        TIME_OF_DAY_DAY,
//                        TIME_OF_DAY_DUSK_OR_DAWN,
//                        TIME_OF_DAY_NIGHT
//     };

    static const int TO;
    static const int FROM;

    FlightStatus(uint smooth_delay_ms);
    ~FlightStatus() {};

    inline bool isValid() const { return m_valid; }
    inline void invalidate() 
    {
        m_valid = false; 
        //TODO debugging
        smoothed_altimeter_readout.setName(QString::null);
    }

    void clear();
    QString toString() const;

    inline void recalcAndSetValid()
    {
        recalc();
        m_valid = true;
    }

    // TCAS stuff

    void clearTcasEntryList() { m_tcas_entry_list.clear(); }
    void addTcasEntry(const TcasEntry& entry) { m_tcas_entry_list.append(entry); }
    const TcasEntryValueList& tcasEntryList() const { return m_tcas_entry_list; }

    // smoothing

    void advanceMeasurements();
    void updateSmoothedValueWithMeans();

    // accessors

    inline void setTrueHeading(const double& hdg) { m_true_heading = hdg; }
    inline void setMagneticTrack(const double& trk) 
    {
        m_magnetic_track = trk; 
        m_magnetic_track_set = true; // for FSs other than MSFS
    }

    inline double smoothedTrueHeading() const { return m_true_heading.value(); }
    inline double smoothedMagneticHeading() const { return Navcalc::trimHeading(m_true_heading.value() - magvar); }

    inline double smoothedTrueTrack() const { return smoothedMagneticTrack() + magvar; }
    inline double smoothedMagneticTrack() const 
    {
        if (m_magnetic_track_set) return m_magnetic_track.value(); 
        // If the track is not directly received from the flightsim, calculate it.
        // This is less accurate especially on takeoff and landing!
        return smoothedMagneticHeading() - wind_correction_angle_deg;
    }

    inline double smoothedVS() const { return smoothed_vs.value(); }

    inline double smoothedPitch(double* trend_per_s = 0) const { return pitch.value(trend_per_s); }
    inline double smoothedBank(double* trend_per_s = 0) const { return bank.value(trend_per_s); }

    inline double smoothedIAS(double* ias_trend_per_s = 0) const { return smoothed_ias.value(ias_trend_per_s); }

    inline double smoothedAltimeterReadout() const { return smoothed_altimeter_readout.value(); }

    inline void updateSmoothedPosition()
    { current_position_smoothed = Waypoint("FSPOS", "", lat_smoothed.value(), lon_smoothed.value()); }

    inline double smoothedFPVVertical(double* fpv_trend_per_s = 0) const
    { return fpv_vertical.value(fpv_trend_per_s); }

    inline double smoothedFlightDirectorPitch() const { return m_fd_pitch.value(); }
    inline double smoothedFlightDirectorBank() const { return m_fd_bank.value(); }

    //----- flaps

    inline bool flapsAreUp() const { return flaps_degrees <= 0.0 && slats_degrees <= 0.0; }
    inline bool areFlapsInTransit() const { return m_flaps_transit_timer.elapsed() < 500; }
    inline double flapsPercent() const { return qMax(flaps_percent_left, flaps_percent_right); }

    //----- RA

    inline double radarAltitude() const
    {
        if (onground) return 0.0;
        return alt_ft - ground_alt_ft;
    }

    //----- gear

    inline bool isGearUp() const
    { return (gear_nose_position_percent + gear_left_position_percent + gear_right_position_percent) <= 1; }

    inline bool isGearDown() const
    { return (gear_nose_position_percent + gear_left_position_percent + gear_right_position_percent) >= 299.0; }

    //----- fuel

    inline uint fuelOnBoard() const
    {
        uint fob = total_weight_kg - zero_fuel_weight_kg;
        if (fob > 1000000) fob = 0;
        return fob;
    }

    inline double fuelOnBoardPercentage() const
    {
        return (100.0 * fuelOnBoard()) / total_fuel_capacity_kg;
    }

    //----- engine

    inline bool isAtLeastOneEngineOn() const
    {
        for(int i=1; i <= nr_of_engines; ++i)
            if (engine_data[i].smoothed_n1.lastValue() > 0.1) return true;
        return false;
    }

    double maxN1OfAllEngines() const;

//TODO
//     inline bool isAtLeastOneGeneratorOn() const
//     {
//         switch(nr_of_engines)
//         {
//             case(0): return engine_generator_1;
//             case(1): return engine_generator_1 || engine_generator_2;
//             case(2): return engine_generator_1 || engine_generator_2 || engine_generator_3;
//             case(3): return engine_generator_1 || engine_generator_2 || engine_generator_3 || engine_generator_4;
//         }

//         return false;
//     }

    inline bool isEngineAntiIceOn() const
    {
        for(int i=1; i <=nr_of_engines; ++i)
            if (engine_data[i].anti_ice_on) return true;
        return false;
    }

    inline bool isReverserOn() const
    {
        for(int i=1; i <=nr_of_engines; ++i)
            if (engine_data[i].reverser_percent >= 0.01) return true;
        return false;
    }

    inline void setAllThrottleLeversInputPercent(const double& percent)
    { for(int i=1; i <=nr_of_engines; ++i) engine_data[i].throttle_input_percent = percent; }

    inline double getMaximumThrottleLeverInputPercent() const
    {
        double max_throttle_input_percent = -1000.0;

        for(int i=1; i <=nr_of_engines; ++i) 
            max_throttle_input_percent = qMax(max_throttle_input_percent, engine_data[i].throttle_input_percent);

        return max_throttle_input_percent;
    }

    //-----

    bool localizerEstablished() const;
    bool localizerAlive() const;
    bool glideslopeAlive() const;

    int APSpd() const { return m_ap_spd; }
    const double& APMach() const { return m_ap_mach; }
    int APHdg() const { return m_ap_hdg; }
    int APAlt() const { return m_ap_alt; }
    int APVs() const { return m_ap_vs; }
    double AltPressureSettingHpa() const { return m_altimeter_pressure_setting_hpa; }
    
    bool isFlightDirectorPitchFromExternal() const { return m_fd_pitch_input_from_external; }
    void setFlightDirectorPitchInternal(const double& pitch);
    void setFlightDirectorPitchExternal(const double& pitch);
    void setFlightDirectorPitchInputFromExternal(bool yes) 
    {
        if (yes != m_fd_pitch_input_from_external) 
            Logger::log(QString("FlightStatus:setFlightDirectorPitchInputFromExternal: yes=%1").arg(yes));
        m_fd_pitch_input_from_external = yes; 
    }

    bool isFlightDirectorBankFromExternal() const { return m_fd_bank_input_from_external; }
    void setFlightDirectorBankInternal(const double& bank);
    void setFlightDirectorBankExternal(const double& bank);
    void setFlightDirectorBankInputFromExternal(bool yes) 
    {
        if (yes != m_fd_bank_input_from_external) 
            Logger::log(QString("FlightStatus:setFlightDirectorBankInputFromExternal: yes=%1").arg(yes));
        m_fd_bank_input_from_external = yes; 
    };

    void resetFlightDirector() 
    {
        Logger::log("FlightStatus:recalc: resetting Flight Director");
        m_fd_pitch.clear();
        m_fd_bank.clear();
    }

    void setAPSpdInternal(uint value) { m_ap_spd = value; m_ap_spd_read_delay_timer.start(); }
    void setAPSpdExternal(uint value) { if (m_ap_spd_read_delay_timer.elapsed() >= 2000) m_ap_spd = value; }

    void setAPMachInternal(const double& value) { m_ap_mach = value; m_ap_mach_read_delay_timer.start(); }
    void setAPMachExternal(const double& value) { if (m_ap_mach_read_delay_timer.elapsed() >= 2000) m_ap_mach = value; }

    void setAPHdgInternal(uint value) { m_ap_hdg = value; m_ap_hdg_read_delay_timer.start(); }
    void setAPHdgExternal(uint value) { if (m_ap_hdg_read_delay_timer.elapsed() >= 2000) m_ap_hdg = value; }

    //NOTE the +1 is for buggy 3rd party gauges
    void setAPAltInternal(uint value) { m_ap_alt = (value/100)*100+1; m_ap_alt_read_delay_timer.start(); }
    void setAPAltExternal(uint value) { if (m_ap_alt_read_delay_timer.elapsed() >= 2000) m_ap_alt = (value/100)*100+1; }

    void setAPVsInternal(int value) { m_ap_vs = value; m_ap_vs_read_delay_timer.start(); }
    void setAPVsExternal(int value) { if (m_ap_vs_read_delay_timer.elapsed() >= 2000) m_ap_vs = value; }

    void setAltPressureSettingHpaInternal(double value)
    { m_altimeter_pressure_setting_hpa = value; m_altimeter_pressure_setting_hpa_read_delay_timer.start(); }
    void setAltPressureSettingHpaExternal(double value)
    { if (m_altimeter_pressure_setting_hpa_read_delay_timer.elapsed() >= 2000) m_altimeter_pressure_setting_hpa = value; }

public:

    // position and movement data

    double lat;
    double lon;
    double tas;
    SmoothedValueWithDelay<double> smoothed_ias;
    double barber_pole_speed;
    SmoothedValueWithDelay<double> smoothed_altimeter_readout;

    double alt_ft;
    double ground_alt_ft;
    SmoothedValueWithDelay<double> smoothed_vs;
    SmoothedValueWithDelay<double> pitch;
    SmoothedValueWithDelay<double> bank;

    bool fd_active;

    double mach;

    double velocity_pitch_deg_s;
    double velocity_roll_deg_s;
    double velocity_yaw_deg_s;

    double acceleration_pitch_deg_s2;
    double acceleration_roll_deg_s2;
    double acceleration_yaw_deg_s2;

    SmoothedValueWithDelay<double> fpv_vertical;
    double fpv_vertical_previous;

    // environmental data

    double magvar;
    double wind_speed_kts;
    double wind_dir_deg_true;
    QTime fs_utc_time;
    QDate fs_utc_date;
    double tat;
    double sat;
    double oat;
    double dew;
    double qnh;
    double delta;
    double theta;

    // engine data

    unsigned char nr_of_engines;
    ENGINE_TYPE engine_type;
    bool engine_ignition_on;

    QMap<uint, EngineData> engine_data;

    // fs control axes

    double rudder_percent;
    double aileron_percent;
    double elevator_percent;
    double elevator_trim_percent;
    double elevator_trim_degrees;

    // joystick input

    double rudder_input_percent;
    double aileron_input_percent;
    double elevator_input_percent;
    //double elevator_trim_input_percent;

    // flightsim status data

    int view_dir_deg;
    bool paused;
    bool onground;
    bool slew;
    bool stall;

    // aircraft status

    //! stall speed full flaps
    uint speed_vs0_kts;
    //! stall speed clean
    uint speed_vs1_kts;
    //! designed cruise speed
    uint speed_vc_kts;
    //! Minimum drag velocity
    uint speed_min_drag_kts;

    double aoa_degrees;
    double slip_percent;

    double total_fuel_capacity_kg;
    uint zero_fuel_weight_kg;
    uint total_weight_kg;

    double brake_left_percent;
    double brake_right_percent;
    bool parking_brake_set;

    bool spoilers_armed;
    double spoiler_lever_percent;
    double spoiler_left_percent;
    double spoiler_right_percent;

    bool lights_landing;
    bool lights_strobe;
    bool lights_beacon;
    bool lights_taxi;
    bool lights_navigation;
    bool lights_instruments;

    double gear_nose_position_percent;
    double gear_left_position_percent;
    double gear_right_position_percent;

    uint current_flap_lever_notch;
    uint flaps_lever_notch_count;
    double slats_degrees;
    double flaps_degrees;
    double flaps_percent_left;
    double flaps_percent_right;

    bool battery_on;
    bool avionics_on;

    // autopilot data

    bool ap_available;
    bool ap_enabled;

    bool ap_hdg_lock;
    bool ap_alt_lock;
    bool ap_vs_lock;
    bool ap_speed_lock;
    bool ap_mach_lock;

    bool ap_nav1_lock;
    bool ap_gs_lock;
    bool ap_app_lock;
    bool ap_app_bc_lock;

    bool at_toga;
    bool at_arm;

    bool gps_enabled;

    // avionics data

    Waypoint nav1;
    int nav1_freq;
    bool nav1_has_loc;
    uint nav1_loc_mag_heading;
    SmoothedValueWithDelay<double> nav1_bearing;
    QString nav1_distance_nm;
    int obs1;
    int obs1_to_from;
    int obs1_loc_needle;
    int obs1_gs_needle;

    Waypoint nav2;
    int nav2_freq;
    bool nav2_has_loc;
    SmoothedValueWithDelay<double> nav2_bearing;
    QString nav2_distance_nm;
    int obs2;
    int obs2_to_from;
    int obs2_loc_needle;
    int obs2_gs_needle;

    Ndb adf1;
    SmoothedValueWithDelay<double> adf1_bearing;

    Ndb adf2;
    SmoothedValueWithDelay<double> adf2_bearing;

    bool outer_marker;
    bool middle_marker;
    bool inner_marker;

    // derived and calculated fields

    double wind_correction_angle_deg;
    double ground_speed_kts;
    Waypoint current_position_raw;

    SmoothedValueWithDelay<double> lat_smoothed;
    SmoothedValueWithDelay<double> lon_smoothed;
    Waypoint current_position_smoothed;

    int avionics_status;
    QString aircraft_type;

    // FS controls

    QList<char> fsctrl_nd_left_list;
    QList<char> fsctrl_pfd_left_list;
    QList<char> fsctrl_cdu_left_list;
    QList<char> fsctrl_cdu_right_list;
    QList<char> fsctrl_fmc_list;
    QList<char> fsctrl_ecam_list;
    QList<char> fsctrl_fcu_list;
    QList<char> fsctrl_nd_right_list;
    QList<char> fsctrl_pfd_right_list;

    // misc

    bool doors_open;
    bool pitot_heat_on;
    int pushback_status;
    bool no_smoking_sign;
    bool seat_belt_sign;
//TODO    TIME_OF_DAY time_of_day;

protected:

    void recalc();

protected:

    bool m_valid;

    SmoothedValueWithDelay<double> m_true_heading;
    TcasEntryValueList m_tcas_entry_list;

    SmoothedValueWithDelay<double> m_magnetic_track;
    bool m_magnetic_track_set;

    SmoothedValueWithDelay<double> m_fd_pitch;
    bool m_fd_pitch_input_from_external;
    SmoothedValueWithDelay<double> m_fd_bank;
    bool m_fd_bank_input_from_external;
    bool m_fd_was_active;

private:

    int m_ap_hdg;
    int m_ap_alt;
    int m_ap_vs;
    int m_ap_spd;
    double m_ap_mach;
    double m_altimeter_pressure_setting_hpa;

    QTime m_altimeter_pressure_setting_hpa_read_delay_timer;
    QTime m_ap_spd_read_delay_timer;
    QTime m_ap_mach_read_delay_timer;
    QTime m_ap_hdg_read_delay_timer;
    QTime m_ap_alt_read_delay_timer;
    QTime m_ap_vs_read_delay_timer;

    double m_last_flaps_percent_left;
    double m_last_flaps_percent_right;
    QTime m_flaps_transit_timer;
};

/////////////////////////////////////////////////////////////////////////////

class FlightStatusProvider
{
public:

    FlightStatusProvider() {};
    virtual ~FlightStatusProvider() {};

    virtual const FlightStatus* flightStatus() const = 0;
};

#endif

