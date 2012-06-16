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

/*! \file    fmc_autothrottle.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "logger.h"
#include "navcalc.h"
#include "airport.h"
#include "fmc_control.h"
#include "fmc_data.h"
#include "flightstatus.h"
#include "fsaccess.h"
#include "aircraft_data.h"


#include "defines.h"
#include "fmc_autothrottle.h"
#include "fmc_autopilot.h"

#include "fly_by_wire.h"

/////////////////////////////////////////////////////////////////////////////

FMCAutothrottle::FMCAutothrottle(ConfigWidgetProvider* config_widget_provider,
                                 Config* main_config, 
                                 const QString& autothrottle_cfg_filename,
                                 FMCControl* fmc_control) :
    m_main_config(main_config), m_fmc_control(fmc_control), m_fmc_data(fmc_control->fmcData()), 
    m_flightstatus(fmc_control->flightStatus()), m_ctrl_throttle(0), m_ctrl_speed(0), 
    m_last_flight_mode(FlightModeTracker::FLIGHT_MODE_UNKNOWN),
    m_was_acceleration_set(false), m_speed_mode_active(SPEED_MODE_NONE), 
    m_speed_mode_armed(SPEED_MODE_NONE), 
    m_current_takeoff_thrust(98.0), m_current_flex_thrust(95.0),
    m_current_max_continous_thrust(95.0), m_current_climb_thrust(90.0),
    m_more_drag_necessary(false), m_more_drag_necessary_timer_started(false),
    m_more_drag_necessary_end_timer_started(false), m_use_airbus_throttle_mode(false),
    m_current_airbus_throttle_mode(AIRBUS_THROTTLE_IDLE), m_was_airborne_with_thr_lever_below_flex_at_least_once(false)
{
    MYASSERT(config_widget_provider != 0);
    MYASSERT(m_main_config != 0);
    MYASSERT(m_fmc_control != 0);
    MYASSERT(m_flightstatus != 0);

    // setup config
    
    m_autothrottle_config = new Config(autothrottle_cfg_filename);
    MYASSERT(m_autothrottle_config != 0);
    setupDefaultConfig();
    m_autothrottle_config->loadfromFile();
    m_autothrottle_config->saveToFile();
    config_widget_provider->registerConfigWidget("Autothrottle", m_autothrottle_config);
    setUseAirbusThrottleModes(m_autothrottle_config->getIntValue(CFG_AP_AIRBUS_THROTTLE_MODE) != 0);

    // setup controller

    MYASSERT(connect(&m_fmc_control->aircraftData(), SIGNAL(signalChanged()), this, SLOT(slotAircraftDataChanged())));

    m_ctrl_throttle = new ControllerThrottle(*m_flightstatus);
    MYASSERT(m_ctrl_throttle != 0);
    m_ctrl_throttle->setTarget(m_fmc_data.fmcTakeoffThrustN1());
    m_ctrl_throttle->setMaxRate(m_fmc_control->aircraftData().throttleControllerMaxRate());
    m_ctrl_throttle->setTrendBoostFactor(m_fmc_control->aircraftData().throttleControllerTrendBoostFactor());
    m_ctrl_throttle->setRateFactor(m_fmc_control->aircraftData().throttleControllerRateFactor());

    m_ctrl_speed = new ControllerSpeed(*m_flightstatus, m_ctrl_throttle);
    MYASSERT(m_ctrl_speed != 0);
    m_ctrl_speed->setMaxTrend(m_fmc_control->aircraftData().speedControllerMaxTrend());
    m_ctrl_speed->setTrendBoostFactor(m_fmc_control->aircraftData().speedControllerTrendBoostFactor());
    m_ctrl_speed->setWrongDirectionTrendBoostFactor(m_fmc_control->aircraftData().speedControllerWrongDirectionTrendBoostFactor());
    m_ctrl_speed->setRateFactor(m_fmc_control->aircraftData().speedControllerRateFactor());

    // mode stuff

    m_speed_mode_calc_timer = QTime::currentTime().addSecs(-60);
    m_speed_mode_active_changed_time = QTime::currentTime().addSecs(-60);
    m_idle_thrust_timer_triggered = false;

    // climb thrust

    m_climb_thrust_calculate_timer.start();
}

/////////////////////////////////////////////////////////////////////////////

FMCAutothrottle::~FMCAutothrottle() 
{
    delete m_ctrl_throttle;
    delete m_ctrl_speed;

    m_autothrottle_config->setValue(CFG_AP_AIRBUS_THROTTLE_MODE, useAirbusThrottleModes() ? 1 : 0);
    m_autothrottle_config->saveToFile();
    delete m_autothrottle_config;
    m_fmc_control->fsAccess().freeThrottleAxes();
};

/////////////////////////////////////////////////////////////////////////////

void FMCAutothrottle::setupDefaultConfig()
{
    m_autothrottle_config->setValue(CFG_AP_THROTTLE_ARMED, 0);
    m_autothrottle_config->setValue(CFG_AP_THROTTLE_ENGAGED, 0);
    m_autothrottle_config->setValue(CFG_AP_THROTTLE_N1_MODE, CFG_AP_THROTTLE_N1_MODE_OFF);
    m_autothrottle_config->setValue(CFG_AP_THROTTLE_SPDMACH, CFG_AP_THROTTLE_SPDMACH_SPEED);
    m_autothrottle_config->setValue(CFG_AP_AIRBUS_THROTTLE_MODE, 0);
    m_autothrottle_config->setValue(CFG_AP_AIRBUS_THROTTLE_IDLE_DETENT_PERCENT, 2.0);
    m_autothrottle_config->setValue(CFG_AP_AIRBUS_THROTTLE_CL_DETENT_PERCENT, 81.0);
    m_autothrottle_config->setValue(CFG_AP_AIRBUS_THROTTLE_FLXMCT_DETENT_PERCENT, 92.0);
    m_autothrottle_config->setValue(CFG_AP_AIRBUS_THROTTLE_TOGA_DETENT_PERCENT, 99.0);
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutothrottle::slotAircraftDataChanged()
{
    m_ctrl_throttle->setMaxRate(m_fmc_control->aircraftData().throttleControllerMaxRate());
    m_ctrl_throttle->setTrendBoostFactor(m_fmc_control->aircraftData().throttleControllerTrendBoostFactor());
    m_ctrl_throttle->setRateFactor(m_fmc_control->aircraftData().throttleControllerRateFactor());

    m_ctrl_speed->setMaxTrend(m_fmc_control->aircraftData().speedControllerMaxTrend());
    m_ctrl_speed->setTrendBoostFactor(m_fmc_control->aircraftData().speedControllerTrendBoostFactor());
    m_ctrl_speed->setWrongDirectionTrendBoostFactor(m_fmc_control->aircraftData().speedControllerWrongDirectionTrendBoostFactor());
    m_ctrl_speed->setRateFactor(m_fmc_control->aircraftData().speedControllerRateFactor());
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutothrottle::slotRefresh()
{
    if (!m_flightstatus->isValid() ||
        m_flightstatus->slew || 
        m_flightstatus->paused ||
        m_fmc_control->isFMCConnectModeSlave()) return;

    calculateThrottle();
    calculateMode();
    checkForMoreDragNecessary();
}

/////////////////////////////////////////////////////////////////////////////

const double& FMCAutothrottle::getAPThrottleN1Target() const 
{
    if (m_fmc_control->isFMCConnectModeSlave()) return m_master_fmc_n1_target;
    return m_ctrl_throttle->target(); 
}

/////////////////////////////////////////////////////////////////////////////

uint FMCAutothrottle::speedModeActiveChangeTimeMs() const 
{
    if (m_fmc_control->isFMCConnectModeSlave()) return m_master_fmc_mode_active_change_time_ms;
    return m_speed_mode_active_changed_time.elapsed(); 
}

/////////////////////////////////////////////////////////////////////////////

bool FMCAutothrottle::isAPThrottleArmed() const
{ 
    if (m_fmc_control->isFMCConnectModeSlave()) return m_master_fmc_armed;

    if (m_fmc_control->fmcAutothrustEnabled())
        return m_autothrottle_config->getIntValue(CFG_AP_THROTTLE_ARMED) != 0;

    if (m_flightstatus->at_arm) return true;
    
    return m_autothrottle_config->getIntValue(CFG_AP_THROTTLE_ARMED) != 0 &&
        m_autothrottle_config->getIntValue(CFG_AP_THROTTLE_ENGAGED) != 0 &&
        m_autothrottle_config->getIntValue(CFG_AP_THROTTLE_N1_MODE) ==  CFG_AP_THROTTLE_N1_MODE_ON;
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutothrottle::armAPThrottle() 
{
    Logger::log("FMCAutothrottle:armAPThrottle");

    if (isAPThrottleArmed()) return;

    if (m_fmc_control->fmcAutothrustEnabled())
    {
        Logger::log("FMCAutothrottle:armAPThrottle - FMC A/THR");
        m_fmc_control->fsAccess().setAutothrustArm(false);
        m_autothrottle_config->setValue(CFG_AP_THROTTLE_ARMED, true);
    }
    else
    {
        Logger::log("FMCAutothrottle:armAPThrottle - flightsim A/THR");
        m_fmc_control->fsAccess().setAutothrustArm(true);
        m_autothrottle_config->setValue(CFG_AP_THROTTLE_ARMED, false);
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutothrottle::disengageAPThrottle()
{
    if (useAirbusThrottleModes())
    {
        switch(m_current_airbus_throttle_mode)
        {
            case(AIRBUS_THROTTLE_CLIMB):
            case(AIRBUS_THROTTLE_FLEX):
            case(AIRBUS_THROTTLE_MCT):
            case(AIRBUS_THROTTLE_TOGA):
                // inhibit A/THR disengage at certain detents
                return;
            default:
                // set throttle controller output
                m_ctrl_throttle->setOutput(m_flightstatus->engine_data[1].throttle_lever_percent);
                break;
        }
    }

    // disengage both, the flightsim A/THR and the vasFMC one.

    m_fmc_control->fsAccess().setAutothrustArm(false);
    //if (!m_fmc_control->fmcAutothrustEnabled()) setAPThrottleModeSpeed();

    m_autothrottle_config->setValue(CFG_AP_THROTTLE_ARMED, false);
    m_autothrottle_config->setValue(CFG_AP_THROTTLE_ENGAGED, false);
    m_autothrottle_config->setValue(CFG_AP_THROTTLE_N1_MODE, CFG_AP_THROTTLE_N1_MODE_OFF);
    if (!useAirbusThrottleModes()) m_fmc_control->fsAccess().freeThrottleAxes();
}

/////////////////////////////////////////////////////////////////////////////

bool FMCAutothrottle::isAPThrottleEngaged() const
{
    if (m_fmc_control->isFMCConnectModeSlave()) return m_master_fmc_engaged;

    if (m_fmc_control->fmcAutothrustEnabled())
        return m_autothrottle_config->getIntValue(CFG_AP_THROTTLE_ENGAGED) != 0;

    if (m_flightstatus->at_arm)
        return (m_flightstatus->ap_speed_lock || m_flightstatus->ap_mach_lock);

    return m_autothrottle_config->getIntValue(CFG_AP_THROTTLE_ARMED) != 0 &&
        m_autothrottle_config->getIntValue(CFG_AP_THROTTLE_ENGAGED) != 0 &&
        m_autothrottle_config->getIntValue(CFG_AP_THROTTLE_N1_MODE) ==  CFG_AP_THROTTLE_N1_MODE_ON;
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutothrottle::engageAPThrottleN1Hold()
{
    Logger::log("FMCAutothrottle:engageAPThrottleN1Hold");
    m_fmc_control->fsAccess().setAutothrustArm(false);
    m_autothrottle_config->setValue(CFG_AP_THROTTLE_ARMED, true);
    m_autothrottle_config->setValue(CFG_AP_THROTTLE_ENGAGED, true);
    m_autothrottle_config->setValue(CFG_AP_THROTTLE_N1_MODE, CFG_AP_THROTTLE_N1_MODE_ON);
    m_ctrl_throttle->setOutput(m_flightstatus->engine_data[1].throttle_lever_percent);
}

/////////////////////////////////////////////////////////////////////////////

bool FMCAutothrottle::isAPThrottleModeN1Engaged() const
{
    if (m_fmc_control->isFMCConnectModeSlave()) return m_master_fmc_n1_hold_mode;

    return m_autothrottle_config->getIntValue(CFG_AP_THROTTLE_ARMED) != 0 &&
        m_autothrottle_config->getIntValue(CFG_AP_THROTTLE_ENGAGED) != 0 &&
        m_autothrottle_config->getIntValue(CFG_AP_THROTTLE_N1_MODE) ==  CFG_AP_THROTTLE_N1_MODE_ON;
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutothrottle::engageAPThrottle(bool set_ap_speed_mach_to_current)
{
    Logger::log("FMCAutothrottle:engageAPThrottle");

    if (m_flightstatus->onground) return;

    if (isAPThrottleModeN1Engaged())
    {
        Logger::log("FMCAutothrottle:engageAPThrottle: N1 engaged");
        m_autothrottle_config->setValue(CFG_AP_THROTTLE_N1_MODE, CFG_AP_THROTTLE_N1_MODE_OFF);

        if (!m_fmc_control->fmcAutothrustEnabled())
        {
            m_autothrottle_config->setValue(CFG_AP_THROTTLE_ARMED, false);
            m_autothrottle_config->setValue(CFG_AP_THROTTLE_ENGAGED, false);
            if (!useAirbusThrottleModes()) m_fmc_control->fsAccess().freeThrottleAxes();
        }

        if (!isAPThrottleArmed()) armAPThrottle();
    }
    else 
    {
        if (!isAPThrottleArmed()) return;
    }
    
    if (!m_fmc_control->fmcAutothrustEnabled())
    {
        if (m_autothrottle_config->getIntValue(CFG_AP_THROTTLE_SPDMACH) == CFG_AP_THROTTLE_SPDMACH_MACH)
        {
            m_fmc_control->fsAccess().setAutothrustMachHold(true);
            m_fmc_control->fsAccess().setAutothrustSpeedHold(false);
            if (set_ap_speed_mach_to_current)
                m_fmc_control->fsAccess().setAPMach(m_flightstatus->mach);
        }
        else // if (m_autothrottle_config->getIntValue(CFG_AP_THROTTLE_SPDMACH) == CFG_AP_THROTTLE_SPDMACH_SPEED)
        {
            m_fmc_control->fsAccess().setAutothrustMachHold(false);
            m_fmc_control->fsAccess().setAutothrustSpeedHold(true);
            if (set_ap_speed_mach_to_current)
                m_fmc_control->fsAccess().setAPAirspeed(Navcalc::round(m_flightstatus->smoothedIAS()));
        }
    }
    else
    {
        m_autothrottle_config->setValue(CFG_AP_THROTTLE_ENGAGED, true);

        m_ctrl_speed->setOutput(m_flightstatus->engine_data[1].throttle_lever_percent);
        m_ctrl_throttle->setOutput(m_flightstatus->engine_data[1].throttle_lever_percent);
        
        if (m_autothrottle_config->getIntValue(CFG_AP_THROTTLE_SPDMACH) == CFG_AP_THROTTLE_SPDMACH_MACH)
        {
            if (set_ap_speed_mach_to_current)
                m_fmc_control->fsAccess().setAPMach(m_flightstatus->mach);
        }
        else // if (m_autothrottle_config->getIntValue(CFG_AP_THROTTLE_SPDMACH) == CFG_AP_THROTTLE_SPDMACH_SPEED)
        {
            if (set_ap_speed_mach_to_current)
                m_fmc_control->fsAccess().setAPAirspeed(Navcalc::round(m_flightstatus->smoothedIAS()));
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutothrottle::setAPThrottleModeSpeed(bool set_ap_speed_to_current)
{
    Logger::log("FMCAutothrottle:setAPThrottleModeSpeed");

    m_autothrottle_config->setValue(CFG_AP_THROTTLE_SPDMACH, CFG_AP_THROTTLE_SPDMACH_SPEED);
    m_ctrl_speed->setOutput(m_flightstatus->engine_data[1].throttle_lever_percent);

    if (set_ap_speed_to_current)
        m_fmc_control->fsAccess().setAPAirspeed(
            Navcalc::round(
                Navcalc::getIasFromMach(
                    m_flightstatus->APMach(), m_flightstatus->oat, 
                    m_flightstatus->smoothedIAS(), m_flightstatus->tas, m_flightstatus->mach)));
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutothrottle::setAPThrottleModeMach(bool set_ap_mach_to_current)
{
    Logger::log("FMCAutothrottle:setAPThrottleModeMach");

    m_autothrottle_config->setValue(CFG_AP_THROTTLE_SPDMACH, CFG_AP_THROTTLE_SPDMACH_MACH);
    m_ctrl_speed->setOutput(m_flightstatus->engine_data[1].throttle_lever_percent);

    if (set_ap_mach_to_current)
        m_fmc_control->fsAccess().setAPMach(
            Navcalc::getMachFromIas(
                m_flightstatus->APSpd(), m_flightstatus->oat, 
                m_flightstatus->smoothedIAS(), m_flightstatus->tas, m_flightstatus->mach));
}

/////////////////////////////////////////////////////////////////////////////

bool FMCAutothrottle::isAPThrottleModeSpeedSet() const
{
    if (m_fmc_control->isFMCConnectModeSlave()) return m_master_fmc_speed_mode;

//     if (!m_fmc_control->fmcAutothrustEnabled())
//         return !m_flightstatus->ap_mach_lock;
//     else
        return m_autothrottle_config->getIntValue(CFG_AP_THROTTLE_SPDMACH) == CFG_AP_THROTTLE_SPDMACH_SPEED; 
}

/////////////////////////////////////////////////////////////////////////////

bool FMCAutothrottle::isAPThrottleModeMachSet() const
{
    if (m_fmc_control->isFMCConnectModeSlave()) return m_master_fmc_mach_mode;

//     if (!m_fmc_control->fmcAutothrustEnabled())
//         return m_flightstatus->ap_mach_lock;
//     else
        return m_autothrottle_config->getIntValue(CFG_AP_THROTTLE_SPDMACH) == CFG_AP_THROTTLE_SPDMACH_MACH; 
}

/////////////////////////////////////////////////////////////////////////////

bool FMCAutothrottle::isAPThrottleN1ClimbModeActive() const
{
    if (m_fmc_control->isFMCConnectModeSlave()) return m_master_fmc_n1_climb_mode;

    if (useAirbusThrottleModes())
    {
        if (m_current_airbus_throttle_mode == AIRBUS_THROTTLE_CLIMB) return true;
        else return false;
    }
    else
    {
        if (m_fmc_data.normalRoute().thrustReductionAltitudeFt() > 0)
            return m_flightstatus->smoothed_altimeter_readout.lastValue() >= m_fmc_data.normalRoute().thrustReductionAltitudeFt();

        return m_flightstatus->radarAltitude() > 1000;
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutothrottle::checkForMoreDragNecessary()
{
    if (speedModeActive() == SPEED_MODE_THRIDLE)
    {
        double ias_trend;
        double ias = m_flightstatus->smoothedIAS(&ias_trend);
        
        double speed_diff = ias - m_flightstatus->APSpd();

        if (speed_diff >= 10.0 && ias_trend > 0.1)
        {
            if (!m_more_drag_necessary_timer_started)
            {
                m_more_drag_necessary_timer_started = true;
                m_more_drag_necessary_timer.start();
            }    
            else if (m_more_drag_necessary_timer.elapsed() > 4000) 
            {
                m_more_drag_necessary = true;
                m_more_drag_necessary_end_timer_started = false;
            }
        }
        else if (isMoreDragNecessary())
        {
            if (!m_more_drag_necessary_end_timer_started)
            {
                m_more_drag_necessary_end_timer_started = true;
                m_more_drag_necessary_end_timer.start();
            }
            else if (m_more_drag_necessary_end_timer.elapsed() > 4000)
            {
                m_more_drag_necessary = false;
                m_more_drag_necessary_timer_started = false;
            }
        }
    }
    else
    {
        m_more_drag_necessary = false;
        m_more_drag_necessary_timer_started = false;
        m_more_drag_necessary_end_timer_started = false;
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutothrottle::calculateThrottle()
{
    if (!m_fmc_control->fmcAutothrustEnabled()) 
    {
        if (useAirbusThrottleModes())
        {
            Logger::log("FMCAutothrottle:calculateThrottle: "
                        "vasFMC A/THR disabled - disabling airbus thrust mode");
            setUseAirbusThrottleModes(false);
            m_ctrl_throttle->setRateReduceFactor(1.0);
            m_fmc_control->fsAccess().freeThrottleAxes();
        }
    }
    else if (useAirbusThrottleModes())
    {
        calcAirbusMode();
        
        switch(m_current_airbus_throttle_mode)
        {
            case(AIRBUS_THROTTLE_IDLE): 
            case(AIRBUS_THROTTLE_NORMAL): 
                break;
            case(AIRBUS_THROTTLE_CLIMB): {
                if (!m_flightstatus->onground)
                {
                    if (!isAPThrottleArmed()) armAPThrottle();
                    if (!isAPThrottleEngaged()) engageAPThrottle(false);
                }
                break;
            }
            case(AIRBUS_THROTTLE_MCT):
            case(AIRBUS_THROTTLE_FLEX):
            case(AIRBUS_THROTTLE_TOGA):
                if (!isAPThrottleModeN1Engaged()) engageAPThrottleN1Hold();
                break;
        }
    }

    //-----

    if (!m_flightstatus->isValid() || m_flightstatus->paused || m_flightstatus->slew) return;

    double ias = m_flightstatus->smoothed_ias.lastValue();
    double vs = m_flightstatus->smoothed_vs.lastValue();

    //TODO experimental, add managed mode later - do the stuff below only in managed mode then!
    if (!m_flightstatus->onground)
    {
        // handle acceleration alt and climb mach

        if (m_fmc_control->flightModeTracker().isClimb() && vs > 100)
        {
            if (!m_was_acceleration_set &&
                m_fmc_data.climbSpeedKts() > 0 &&
                m_fmc_data.normalRoute().accelerationAltitudeFt() > 0 &&
                m_flightstatus->smoothed_altimeter_readout.lastValue() >= m_fmc_data.normalRoute().accelerationAltitudeFt())
            {
                
                if (qAbs(m_flightstatus->smoothed_altimeter_readout.lastValue() - m_fmc_data.normalRoute().accelerationAltitudeFt()) < 500)
                {
                    Logger::log(QString("FMCAutothrottle:calculateThrottle: reached accel alt %1").
                                arg(m_fmc_data.normalRoute().accelerationAltitudeFt()));
                    setAPThrottleModeSpeed(false);
                    m_fmc_control->fsAccess().setAPAirspeed(m_fmc_data.climbSpeedKts());
                    m_was_acceleration_set = true;
                }
            }

            if (isAPThrottleN1ClimbModeActive() &&
                !isAPThrottleModeMachSet() &&
                m_fmc_data.climbMach() > 0.0 &&
                m_flightstatus->mach + 0.01 >= m_fmc_data.climbMach())
            {
                Logger::log(QString("FMCAutothrottle:calculateThrottle: setting climb mach %1").arg(m_fmc_data.climbMach()));
                setAPThrottleModeMach(false);
                m_fmc_control->fsAccess().setAPMach(m_fmc_data.climbMach());
            }
        }

        // handle cruise mach

        if (m_fmc_control->flightModeTracker().isCruise() && m_last_flight_mode == FlightModeTracker::FLIGHT_MODE_CLIMB)
        {
            if (isAPThrottleN1ClimbModeActive() &&
                m_fmc_data.cruiseMach() > 0.0 &&
                m_flightstatus->mach + 0.01 >= m_fmc_data.cruiseMach())
            {
                Logger::log(QString("FMCAutothrottle:calculateThrottle: setting cruise mach %1").arg(m_fmc_data.cruiseMach()));
                if (!isAPThrottleModeMachSet()) setAPThrottleModeMach(false);
                m_fmc_control->fsAccess().setAPMach(m_fmc_data.cruiseMach());                    
                engageAPThrottle(false);
            }
        }

        // handle descent mach and speed

        if (m_fmc_control->flightModeTracker().isDescent() && isAPThrottleModeMachSet())
        {
            if (m_last_flight_mode == FlightModeTracker::FLIGHT_MODE_CRUISE)
            {
                Logger::log(QString("FMCAutothrottle:calculateThrottle: setting descent mach %1").arg(m_fmc_data.descentMach()));
                m_fmc_control->fsAccess().setAPMach(m_fmc_data.descentMach());
            }
            
            if (ias + 3 > m_fmc_data.descentSpeedKts())
            {
                Logger::log(QString("FMCAutothrottle:calculateThrottle: setting descent speed %1").arg(m_fmc_data.descentSpeedKts()));
                if (!isAPThrottleModeSpeedSet()) setAPThrottleModeSpeed(false);
                m_fmc_control->fsAccess().setAPAirspeed(m_fmc_data.descentSpeedKts());
            }
        }
    }
    else
    {
        m_was_acceleration_set = false;
    }

    //----- sanity checks

    if (!isAPThrottleArmed()) m_autothrottle_config->setValue(CFG_AP_THROTTLE_ENGAGED, false);
    if (!m_flightstatus->isAtLeastOneEngineOn() && (isAPThrottleArmed() || isAPThrottleEngaged())) disengageAPThrottle();

    //-----

    if (isAPThrottleModeN1Engaged())
    {
        // N1 hold mode

        m_ctrl_throttle->setN1Limit(currentTakeoffThrust());
        m_ctrl_throttle->setRateReduceFactor(1.0);

        double ap_alt_diff = m_flightstatus->smoothed_altimeter_readout.lastValue() - m_flightstatus->APAlt();

        if (m_fmc_control->fmcAutoPilot().isFLChangeModeActive())
        {
            if (ap_alt_diff > 0)
            {
                m_ctrl_throttle->setRateReduceFactor(6.0);
                m_ctrl_throttle->setTarget(0.0);
            }
            else
            {
                m_ctrl_throttle->setRateReduceFactor(5.0);
                (isAPThrottleN1ClimbModeActive()) ?
                    m_ctrl_throttle->setTarget(calcClimbThrust()) :
                    m_ctrl_throttle->setTarget(m_fmc_data.fmcTakeoffThrustN1());
            }
        }
        else
        {
            (isAPThrottleN1ClimbModeActive()) ?
                m_ctrl_throttle->setTarget(calcClimbThrust()) :
                m_ctrl_throttle->setTarget(m_fmc_data.fmcTakeoffThrustN1());
        }
        
        // override by airbus throttle mode

        bool set_throttle = true;

        if (useAirbusThrottleModes())
        {
            switch(m_current_airbus_throttle_mode)
            {
                case(AIRBUS_THROTTLE_IDLE):
                    m_ctrl_throttle->setN1Limit(calcClimbThrust());
                    m_ctrl_throttle->setTarget(0.0);

                    if (m_flightstatus->onground)
                    {
                        set_throttle = false;
                        m_fmc_control->fsAccess().setThrottle(0.0);
                        disengageAPThrottle();
                    }
                    break;

                case(AIRBUS_THROTTLE_NORMAL): 
                    m_ctrl_throttle->setN1Limit(calcClimbThrust());
                    if (m_fmc_control->fmcAutoPilot().isFLChangeModeActive() && ap_alt_diff > 0)
                        m_ctrl_throttle->setTarget(0.0);
                    else
                        m_ctrl_throttle->setTarget(getAirbusModeN1LimitByLever());
                    break;

                case(AIRBUS_THROTTLE_CLIMB):
                    m_ctrl_throttle->setN1Limit(calcClimbThrust());
                    if (m_fmc_control->fmcAutoPilot().isFLChangeModeActive())
                    {
                        if (ap_alt_diff > 0) 
                        {
                            m_ctrl_throttle->setRateReduceFactor(6.0);
                            m_ctrl_throttle->setTarget(0.0);
                        }
                        else
                        {
                            m_ctrl_throttle->setRateReduceFactor(5.0);
                            m_ctrl_throttle->setTarget(calcClimbThrust());
                        }
                    }
                    else if (m_fmc_control->fmcAutoPilot().isTakeoffModeActiveVertical())
                    {
                        m_ctrl_throttle->setTarget(calcClimbThrust());
                    }
                    else if (!m_flightstatus->onground)
                    {
                        Logger::log("FMCAutothrottle:calculateThrottle: changing to spd/mach hold");
                        engageAPThrottle(false);
                    }
                    break;

                case(AIRBUS_THROTTLE_MCT):
                    m_ctrl_throttle->setN1Limit(100.0);
                    m_ctrl_throttle->setTarget(calcMaxContinousThrust());
                    break;
                case(AIRBUS_THROTTLE_FLEX):
                    m_ctrl_throttle->setN1Limit(100.0);
                    m_ctrl_throttle->setTarget(calcFlexThrust());
                    break;
                case(AIRBUS_THROTTLE_TOGA):
                    m_ctrl_throttle->setN1Limit(100.0);
                    m_ctrl_throttle->setTarget(currentTakeoffThrust());
                    break;
            }
        }
   
        // let the autothrust controller set the throttle levers

        if (set_throttle)
        {
            m_ctrl_throttle->setDoFastIdling(false);
            m_fmc_control->fsAccess().setThrottle(m_ctrl_throttle->output(m_flightstatus->engine_data[1].smoothedN1()));
        }
    }
    else if (!m_flightstatus->at_arm && m_autothrottle_config->getIntValue(CFG_AP_THROTTLE_ARMED) != 0)
    {
        // normal speed/mach hold mode

        if (useAirbusThrottleModes() &&
            (m_current_airbus_throttle_mode == AIRBUS_THROTTLE_NORMAL ||
             m_current_airbus_throttle_mode == AIRBUS_THROTTLE_IDLE))
        {
            m_ctrl_speed->setN1Limit(getAirbusModeN1LimitByLever());
        }
        else
        {
            m_ctrl_speed->setN1Limit(calcClimbThrust());
        }

        switch(m_autothrottle_config->getIntValue(CFG_AP_THROTTLE_SPDMACH))
        {
            case(CFG_AP_THROTTLE_SPDMACH_SPEED): {

                if (m_flightstatus->onground)
                {
                    Logger::log("FMCAutothrottle:calculateThrottle: on ground - disengage A/THR");
                    m_fmc_control->fsAccess().setThrottle(0.0);
                    disengageAPThrottle();
                }
                else
                {
                    if (!m_fmc_control->fmcAutothrustEnabled())
                    {
                        if (m_flightstatus->ap_mach_lock)
                        {
                            m_fmc_control->fsAccess().setAutothrustMachHold(false);
                            m_fmc_control->fsAccess().setAutothrustSpeedHold(true);
                        }
                    }
                    else
                    {
                        m_ctrl_speed->setTarget(
                            qMax(qMin((int)m_flightstatus->barber_pole_speed-5, m_flightstatus->APSpd()),
                                 (int)m_fmc_control->aircraftData().getMinimumSelectableSpeed()));
                        m_fmc_control->fsAccess().setThrottle(m_ctrl_speed->output(ias));
                    }
                }
                break;
            }

            case(CFG_AP_THROTTLE_SPDMACH_MACH): {

                if (m_flightstatus->onground)
                {
                    Logger::log("FMCAutothrottle:calculateThrottle: on ground - disengage A/THR");
                    m_fmc_control->fsAccess().setThrottle(0.0);
                    disengageAPThrottle();
                }
                else
                {
                    if (!m_fmc_control->fmcAutothrustEnabled())
                    {
                        if (m_flightstatus->ap_speed_lock)
                        {
                            m_fmc_control->fsAccess().setAutothrustSpeedHold(false);
                            m_fmc_control->fsAccess().setAutothrustMachHold(true);
                        }
                    }
                    else
                    {
                        double effective_ap_speed = 
                            Navcalc::getIasFromMach(
                                m_flightstatus->APMach(), m_flightstatus->oat, 
                                ias, m_flightstatus->tas, m_flightstatus->mach);
                        
                        m_ctrl_speed->setTarget(
                            qMax(qMin(m_flightstatus->barber_pole_speed-5, effective_ap_speed),
                                 m_fmc_control->aircraftData().getMinimumSelectableSpeed()));

                        m_fmc_control->fsAccess().setThrottle(m_ctrl_speed->output(ias));
                    }
                }
                break;
            }
        }
    }
    else if (useAirbusThrottleModes())
    {
        // A/THR off mode and airbus throttle mode on - manual throttle

        m_ctrl_throttle->setRateReduceFactor(1.0);
        m_ctrl_throttle->setDoFastIdling(true);

        // if the A/THR is neither armed or engaged and the airbus throttle is
        // enabled we set the N1 directly from the thrust lever position

        switch(m_current_airbus_throttle_mode)
        {
            case(AIRBUS_THROTTLE_IDLE):
                m_ctrl_throttle->setN1Limit(calcClimbThrust());
                m_ctrl_throttle->setTarget(0.0);
                break;
            case(AIRBUS_THROTTLE_NORMAL): 
                m_ctrl_throttle->setN1Limit(calcClimbThrust());
                m_ctrl_throttle->setTarget(getAirbusModeN1LimitByLever());
                break;
            case(AIRBUS_THROTTLE_CLIMB):
                m_ctrl_throttle->setN1Limit(calcClimbThrust());
                m_ctrl_throttle->setTarget(calcClimbThrust());
                break;
            case(AIRBUS_THROTTLE_MCT):
                m_ctrl_throttle->setN1Limit(100.0);
                m_ctrl_throttle->setTarget(calcMaxContinousThrust());
                break;
            case(AIRBUS_THROTTLE_FLEX):
                m_ctrl_throttle->setN1Limit(100.0);
                m_ctrl_throttle->setTarget(calcFlexThrust());
                break;
            case(AIRBUS_THROTTLE_TOGA):
                m_ctrl_throttle->setN1Limit(100.0);
                m_ctrl_throttle->setTarget(currentTakeoffThrust());
                break;
        }

        if (!m_flightstatus->isReverserOn())
            m_fmc_control->fsAccess().setThrottle(m_ctrl_throttle->output(m_flightstatus->engine_data[1].smoothedN1()));
        else
            m_fmc_control->fsAccess().freeThrottleAxes();

//         Logger::log(QString("thrlvr=%1 ablvr=%2 tgt=%3  cldet=%4  cthr=%5").
//                     arg(m_flightstatus->throttle_input_percent).
//                     arg(getAirbusModeN1LimitByLever()).
//                     arg(m_ctrl_throttle->target()).
//                     arg(m_autothrottle_config->getDoubleValue(CFG_AP_AIRBUS_THROTTLE_CL_DETENT_PERCENT)).
//                     arg(calcClimbThrust()));
    }


    m_last_flight_mode = m_fmc_control->flightModeTracker().currentFlightMode();
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutothrottle::calculateMode()
{
    // only calculate the mode 4 times per second
    if (m_speed_mode_calc_timer.elapsed() < 250) return;
    m_speed_mode_calc_timer.start();

    SPEED_MODE last_speed_mode_active = m_speed_mode_active;
    m_speed_mode_active = m_speed_mode_armed = SPEED_MODE_NONE;

    if (isAPThrottleEngaged())
    {
        if (!isAPThrottleModeN1Engaged() &&
            m_flightstatus->engine_data[1].throttle_lever_percent <= 0 && 
            m_flightstatus->engine_data[2].throttle_lever_percent <= 0)
        {
            if (!m_idle_thrust_timer_triggered && m_speed_mode_active != SPEED_MODE_THRIDLE)
            {
                m_idle_thrust_timer.start();
                m_idle_thrust_timer_triggered = true;
            }

            if (isAPThrottleModeSpeedSet())
                m_speed_mode_active = SPEED_MODE_SPEED;
            else if (isAPThrottleModeMachSet())
                m_speed_mode_active = SPEED_MODE_MACH;

            if (m_idle_thrust_timer.elapsed() > 2000)
                m_speed_mode_active = SPEED_MODE_THRIDLE;
        }
        else 
        {
            m_idle_thrust_timer_triggered = false;

            if (isAPThrottleModeN1Engaged())
            {
                if (useAirbusThrottleModes())
                {
                    switch(m_current_airbus_throttle_mode)
                    {
                        case(AIRBUS_THROTTLE_IDLE):
                            m_speed_mode_active = SPEED_MODE_THRIDLE;
                            break;
                        case(AIRBUS_THROTTLE_NORMAL):
                            m_speed_mode_active = SPEED_MODE_THRLVR;
                            if (m_ctrl_throttle->target() < 0.1) m_speed_mode_active = SPEED_MODE_THRIDLE;
                            break;
                        case(AIRBUS_THROTTLE_CLIMB):
                            m_speed_mode_active = SPEED_MODE_THRCLB;
                            if (m_ctrl_throttle->target() < 0.1) m_speed_mode_active = SPEED_MODE_THRIDLE;
                            break;
                        case(AIRBUS_THROTTLE_MCT):
                            m_speed_mode_active = SPEED_MODE_MCT;
                            break;
                        case(AIRBUS_THROTTLE_FLEX):
                            m_speed_mode_active = SPEED_MODE_FLEX;
                            break;
                        case(AIRBUS_THROTTLE_TOGA):
                            m_speed_mode_active = SPEED_MODE_TOGA;
                            break;
                    }
                }
                else
                {
                    if (m_ctrl_throttle->target() < 0.1)
                        m_speed_mode_active = SPEED_MODE_THRIDLE;
                    else if (isAPThrottleN1ClimbModeActive())
                        m_speed_mode_active = SPEED_MODE_THRCLB;
                    else
                        m_speed_mode_active = SPEED_MODE_FLEX;
                }
            }
            else
            {
                if (isAPThrottleModeSpeedSet())
                {
                    m_speed_mode_active = SPEED_MODE_SPEED;
                }
                if (isAPThrottleModeMachSet())
                {
                    m_speed_mode_active = SPEED_MODE_MACH;
                }

                // override when in airbus mode

                if (useAirbusThrottleModes())
                {
                    switch(m_current_airbus_throttle_mode)
                    {
                        case(AIRBUS_THROTTLE_IDLE):
                            m_speed_mode_active = SPEED_MODE_THRIDLE;
                            break;
                        case(AIRBUS_THROTTLE_NORMAL):
                            m_speed_mode_active = SPEED_MODE_THRLVR;
                            if (m_ctrl_throttle->target() < 0.1) m_speed_mode_active = SPEED_MODE_THRIDLE;
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }
    else
    {
        m_idle_thrust_timer_triggered = false;
    }

    if (m_speed_mode_active != last_speed_mode_active) m_speed_mode_active_changed_time.start();
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutothrottle::slotToggleAutothrottleArm()
{
    if (isAPThrottleArmed()) disengageAPThrottle();
    else                     armAPThrottle();
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutothrottle::slotToggleAutothrottleSpeedMach()
{
    if (isAPThrottleModeSpeedSet())     setAPThrottleModeMach();
    else if (isAPThrottleModeMachSet()) setAPThrottleModeSpeed();
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutothrottle::slotAutothrottleEngageSpeed()
{
    Logger::log("FMCAutothrottle:slotAutothrottleEngageSpeed");

    if (isAPThrottleArmed())
    {
        engageAPThrottle();
        setAPThrottleModeSpeed();
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutothrottle::slotAutothrottleEngageMach()
{
    Logger::log("FMCAutothrottle:slotAutothrottleEngageMach");

    if (isAPThrottleArmed())
    {
        engageAPThrottle();
        setAPThrottleModeMach();
    }
}

/////////////////////////////////////////////////////////////////////////////

double FMCAutothrottle::calcMaxContinousThrust()
{
    //TODO get from aircraft config later!
    m_current_max_continous_thrust = m_fmc_control->fmcData().fmcTakeoffThrustN1();
    return m_current_max_continous_thrust;
}

/////////////////////////////////////////////////////////////////////////////

double FMCAutothrottle::calcFlexThrust()
{
    //TODO calc by temp later!
    m_current_flex_thrust = m_fmc_control->fmcData().fmcTakeoffThrustN1();
    return m_current_flex_thrust;
}

/////////////////////////////////////////////////////////////////////////////

double FMCAutothrottle::calcClimbThrust()
{
    if (m_climb_thrust_calculate_timer.elapsed() > 2000)
    {
        m_climb_thrust_calculate_timer.start();

        double init_alt = 
            (m_fmc_control->normalRoute().departureAirport() != 0) ?
            qMax(0, m_fmc_control->normalRoute().departureAirport()->elevationFt()) : 0.0;

        double washout_alt = 30000;
        MYASSERT(init_alt < washout_alt - 1000.0);

        if (m_flightstatus->smoothed_altimeter_readout.lastValue() >= washout_alt) 
            return m_fmc_data.fmcMaxClimbThrustN1();

        double k = qMax(0.0, 
                        (m_fmc_data.fmcMaxClimbThrustN1() - 
                         m_fmc_data.fmcInitialClimbThrustN1()) /(washout_alt - init_alt));

        double d = m_fmc_data.fmcMaxClimbThrustN1() - (k * washout_alt);

        m_current_climb_thrust = k * m_flightstatus->smoothed_altimeter_readout.lastValue() + d;

//         Logger::log(QString("calcClimbThrust: %1/%2 k=%3 d=%4 cur=%5").
//                     arg(m_fmc_control->initialClimbThrustN1()).
//                     arg(m_fmc_control->maximumClimbThrustN1()).
//                     arg(k).
//                     arg(d).
//                     arg(m_current_climb_thrust));
    }

    return m_current_climb_thrust;
}

/////////////////////////////////////////////////////////////////////////////

QString FMCAutothrottle::currentAirbusThrottleModeAsString() const
{
    switch(m_current_airbus_throttle_mode)
    {
        case(AIRBUS_THROTTLE_IDLE): return "IDLE";
        case(AIRBUS_THROTTLE_NORMAL): return "NORMAL";
        case(AIRBUS_THROTTLE_CLIMB): return "CLIMB";
        case(AIRBUS_THROTTLE_MCT): return "MCT";
        case(AIRBUS_THROTTLE_FLEX): return "FLEX";
        case(AIRBUS_THROTTLE_TOGA): return "TOGA";
    }

    return "INVALID";
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutothrottle::calcAirbusMode()
{
    double max_throttle_lever_input_percent = m_flightstatus->getMaximumThrottleLeverInputPercent();

    if (m_flightstatus->onground)
    {
        m_was_airborne_with_thr_lever_below_flex_at_least_once = false;
    }
    else if (m_current_airbus_throttle_mode != AIRBUS_THROTTLE_TOGA &&
             m_current_airbus_throttle_mode != AIRBUS_THROTTLE_FLEX)
    {
        m_was_airborne_with_thr_lever_below_flex_at_least_once = true;
    }

    //TOGA/MAX THRUST
    if (max_throttle_lever_input_percent >= 
        m_autothrottle_config->getDoubleValue(CFG_AP_AIRBUS_THROTTLE_TOGA_DETENT_PERCENT))
    {
        m_current_airbus_throttle_mode = AIRBUS_THROTTLE_TOGA;
    }
    // FLX/MCT
    else if (max_throttle_lever_input_percent >=
             m_autothrottle_config->getDoubleValue(CFG_AP_AIRBUS_THROTTLE_FLXMCT_DETENT_PERCENT))
    {
        if (m_was_airborne_with_thr_lever_below_flex_at_least_once)
            m_current_airbus_throttle_mode = AIRBUS_THROTTLE_MCT;
        else
            m_current_airbus_throttle_mode = AIRBUS_THROTTLE_FLEX;
    }
    // CL
    else if (max_throttle_lever_input_percent >=
             m_autothrottle_config->getDoubleValue(CFG_AP_AIRBUS_THROTTLE_CL_DETENT_PERCENT))
    {
        m_current_airbus_throttle_mode = AIRBUS_THROTTLE_CLIMB;
    }
    // IDLE
    else if (max_throttle_lever_input_percent <= 
             m_autothrottle_config->getDoubleValue(CFG_AP_AIRBUS_THROTTLE_IDLE_DETENT_PERCENT))
    {
        m_current_airbus_throttle_mode = AIRBUS_THROTTLE_IDLE;
    }
    else 
    {
        m_current_airbus_throttle_mode = AIRBUS_THROTTLE_NORMAL;
    }
}

/////////////////////////////////////////////////////////////////////////////

double FMCAutothrottle::getAirbusModeN1LimitByLever()
{
    double throttle_lever_input_percent = m_flightstatus->getMaximumThrottleLeverInputPercent();

    if (!useAirbusThrottleModes()) return throttle_lever_input_percent;

    switch(m_current_airbus_throttle_mode)
    {
        case(AIRBUS_THROTTLE_IDLE):   return throttle_lever_input_percent;
        case(AIRBUS_THROTTLE_NORMAL): {
            // scale the lever N1 command to reach climb thrust at the CL detent
            return 
                m_fmc_control->aircraftData().idleThrust() + 
                (throttle_lever_input_percent * (calcClimbThrust() - m_fmc_control->aircraftData().idleThrust()) /
                 m_autothrottle_config->getDoubleValue(CFG_AP_AIRBUS_THROTTLE_CL_DETENT_PERCENT));
        }
        case(AIRBUS_THROTTLE_CLIMB):  return calcClimbThrust();
        case(AIRBUS_THROTTLE_MCT):    return calcMaxContinousThrust();
        case(AIRBUS_THROTTLE_FLEX):   return calcFlexThrust();
        case(AIRBUS_THROTTLE_TOGA):   return m_current_takeoff_thrust;
    }

    return throttle_lever_input_percent;
}

/////////////////////////////////////////////////////////////////////////////

uint FMCAutothrottle::airbusDetentCount(AIRBUS_THROTTLE_MODE prev_mode, AIRBUS_THROTTLE_MODE cur_mode)
{
    int count =qAbs(cur_mode - prev_mode);
    if (count == 0) return count;
    
    if (cur_mode == AIRBUS_THROTTLE_IDLE || prev_mode == AIRBUS_THROTTLE_IDLE) --count;
    if ((prev_mode <= AIRBUS_THROTTLE_FLEX && cur_mode > AIRBUS_THROTTLE_FLEX ||
         prev_mode > AIRBUS_THROTTLE_FLEX && cur_mode <= AIRBUS_THROTTLE_FLEX)) --count;

    MYASSERT(count >= 0);
    return count;
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutothrottle::operator>>(QDataStream& out) const
{
    out << isAPThrottleArmed()
        << isAPThrottleEngaged()
        << isAPThrottleModeSpeedSet() 
        << isAPThrottleModeMachSet()
        << isAPThrottleModeN1Engaged()
        << isAPThrottleN1ClimbModeActive()
        << getAPThrottleN1Target()
        << (qint16)m_speed_mode_active
        << (qint16)m_speed_mode_armed
        << speedModeActiveChangeTimeMs()
        << m_current_climb_thrust
        << m_more_drag_necessary
        << (qint8)m_use_airbus_throttle_mode
        << (qint16)m_current_airbus_throttle_mode;
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutothrottle::operator<<(QDataStream& in)
{
    qint16 speed_mode_active, speed_mode_armed, current_airbus_throttle_mode;

    in >> m_master_fmc_armed
       >> m_master_fmc_engaged
       >> m_master_fmc_speed_mode
       >> m_master_fmc_mach_mode
       >> m_master_fmc_n1_hold_mode
       >> m_master_fmc_n1_climb_mode
       >> m_master_fmc_n1_target
       >> speed_mode_active
       >> speed_mode_armed
       >> m_master_fmc_mode_active_change_time_ms
       >> m_current_climb_thrust
       >> m_more_drag_necessary 
       >> m_use_airbus_throttle_mode
       >> current_airbus_throttle_mode;

    m_speed_mode_active = (SPEED_MODE)speed_mode_active;
    m_speed_mode_armed = (SPEED_MODE)speed_mode_armed;
    m_current_airbus_throttle_mode = (AIRBUS_THROTTLE_MODE)current_airbus_throttle_mode;
}

/////////////////////////////////////////////////////////////////////////////

void FMCAutothrottle::setUseAirbusThrottleModes(bool yes)
{
    m_use_airbus_throttle_mode = yes; 
    if (!yes) disengageAPThrottle();
}

/////////////////////////////////////////////////////////////////////////////

// End of file
