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

/*! \file    fmc_autothrottle.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __FMC_AUTOTHROTTLE_H__
#define __FMC_AUTOTHROTTLE_H__

#include <QObject>

#include "config.h"
#include "serialization_iface.h"
#include "flight_mode_tracker.h"

#include "controller_throttle.h"
#include "controller_speed.h"

#include "fmc_autothrottle_defines.h"

class FMCControl;
class FMCData;
class FlightStatus;
class Waypoint;
class ConfigWidgetProvider;
class Damping;

/////////////////////////////////////////////////////////////////////////////

//! FMC Autothrottle Control
class FMCAutothrottle : public QObject, public SerializationIface
{
    Q_OBJECT

public:

    enum SPEED_MODE { SPEED_MODE_NONE = 0,
                      SPEED_MODE_SPEED,
                      SPEED_MODE_MACH,
                      SPEED_MODE_TOGA,
                      SPEED_MODE_FLEX,
                      SPEED_MODE_MCT,
                      SPEED_MODE_THRCLB,
                      SPEED_MODE_THRLVR,
                      SPEED_MODE_THRIDLE
    };

    enum AIRBUS_THROTTLE_MODE { AIRBUS_THROTTLE_IDLE = 0,
                                AIRBUS_THROTTLE_NORMAL,
                                AIRBUS_THROTTLE_CLIMB,
                                AIRBUS_THROTTLE_FLEX,
                                AIRBUS_THROTTLE_MCT,
                                AIRBUS_THROTTLE_TOGA
    };

    //! Standard Constructor
    FMCAutothrottle(ConfigWidgetProvider* config_widget_provider,
                    Config* main_config,
                    const QString& autothrottle_cfg_filename,
                    FMCControl* fmc_control);    

    //! Destructor
    virtual ~FMCAutothrottle();

    virtual void operator<<(QDataStream& in);
    virtual void operator>>(QDataStream& out) const;

    //----- getter

    bool isAPThrottleArmed() const;
    bool isAPThrottleEngaged() const;
    bool isAPThrottleModeSpeedSet() const;
    bool isAPThrottleModeMachSet() const;
    bool isAPThrottleModeN1Engaged() const;
    bool isAPThrottleN1ClimbModeActive() const;
    const double& getAPThrottleN1Target() const;
    SPEED_MODE speedModeActive() const { return m_speed_mode_active; }
    SPEED_MODE speedModeArmed() const { return m_speed_mode_armed; }
    uint speedModeActiveChangeTimeMs() const;

    // TODO take max. toga thrust from aircraftdata later!
    const double& currentTakeoffThrust() const { return m_current_takeoff_thrust; }
    const double& currentMaxContinousThrust() const { return m_current_max_continous_thrust; }
    const double& currentFlexThrust() const { return m_current_flex_thrust; }
    const double& currentClimbThrust() const { return m_current_climb_thrust; }

    bool isMoreDragNecessary() const { return m_more_drag_necessary; }    
    
    bool useAirbusThrottleModes() const { return m_use_airbus_throttle_mode; }
    AIRBUS_THROTTLE_MODE currentAirbusThrottleMode() const { return m_current_airbus_throttle_mode; }
    QString currentAirbusThrottleModeAsString() const;
    
    //! returns the number of detents the lever passed from the prev_mode to the cur_mode
    uint airbusDetentCount(AIRBUS_THROTTLE_MODE prev_mode, AIRBUS_THROTTLE_MODE cur_mode);

    //! only useful when in airbus thrust mode
    double getAirbusModeN1LimitByLever();

    //----- setter

    void armAPThrottle();
    void engageAPThrottle(bool set_ap_speed_mach_to_current = true);
    void disengageAPThrottle();
    void setAPThrottleModeSpeed(bool set_ap_speed_to_current = true);
    void setAPThrottleModeMach(bool set_ap_mach_to_current = true);
    void engageAPThrottleN1Hold();
    inline void setAPThrottleN1Target(const double& target) { m_ctrl_throttle->setTarget(target); }
    inline void setAPThrottleSpeedTarget(const double& target) { m_ctrl_speed->setTarget(target); }

    void setUseAirbusThrottleModes(bool yes);

public slots:

    void slotRefresh();

    void slotToggleAutothrottleArm();
    void slotToggleAutothrottleSpeedMach();
    void slotAutothrottleEngageSpeed();
    void slotAutothrottleEngageMach();

protected slots:

    void slotAircraftDataChanged();

protected:

    void setupDefaultConfig();

    //! calculates and sets the autothrottle
    void calculateThrottle();

    //! calculates the ATHR mode
    void calculateMode();

    //! calculates the requested FLEX thrust
    double calcMaxContinousThrust();

    //! calculates the requested FLEX thrust
    double calcFlexThrust();

    //! calculates the requested climb thrust based on the altitude
    double calcClimbThrust();
    
    //! calculates the current airbus throttle mode when enabled
    void calcAirbusMode();

    //! checks if we need more drag when the engines are idle
    void checkForMoreDragNecessary();

protected:

    Config* m_main_config;
    Config* m_autothrottle_config;

    FMCControl* m_fmc_control;
    FMCData& m_fmc_data;
    FlightStatus* m_flightstatus;

    ControllerThrottle *m_ctrl_throttle;
    ControllerSpeed *m_ctrl_speed;

    FlightModeTracker::FLIGHTMODE m_last_flight_mode;

    bool m_was_acceleration_set;

    QTime m_speed_mode_calc_timer;
    SPEED_MODE m_speed_mode_active;
    SPEED_MODE m_speed_mode_armed;
    QTime m_speed_mode_active_changed_time;

    bool m_idle_thrust_timer_triggered;
    QTime m_idle_thrust_timer;

    double m_current_takeoff_thrust;
    double m_current_flex_thrust;
    double m_current_max_continous_thrust;
    QTime m_climb_thrust_calculate_timer;
    double m_current_climb_thrust;

    bool m_more_drag_necessary;
    bool m_more_drag_necessary_timer_started;
    QTime m_more_drag_necessary_timer;
    bool m_more_drag_necessary_end_timer_started;
    QTime m_more_drag_necessary_end_timer;

    bool m_use_airbus_throttle_mode;
    AIRBUS_THROTTLE_MODE m_current_airbus_throttle_mode;
    bool m_was_airborne_with_thr_lever_below_flex_at_least_once;

    //----- master FMC values (values not listed here will be set directly from the master when in slave mode)

    bool m_master_fmc_armed;
    bool m_master_fmc_engaged;
    bool m_master_fmc_speed_mode;
    bool m_master_fmc_mach_mode;
    bool m_master_fmc_n1_hold_mode;
    bool m_master_fmc_n1_climb_mode;
    double m_master_fmc_n1_target;
    uint m_master_fmc_mode_active_change_time_ms;

private:
    //! Hidden copy-constructor
    FMCAutothrottle(const FMCAutothrottle&);
    //! Hidden assignment operator
    const FMCAutothrottle& operator = (const FMCAutothrottle&);
};



#endif /* __FMC_AUTOTHROTTLE_H__ */

// End of file

