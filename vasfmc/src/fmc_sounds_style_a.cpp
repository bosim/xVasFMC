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

/*! \file    fmc_sounds_style_a.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "assert.h"
#include "config.h"
#include "defines.h"
#include "flightstatus.h"
#include "fmc_control.h"
#include "fmc_flightstatus_checker_base.h"

#include "fmc_sounds_defines.h"
#include "fmc_sounds_style_a.h"

/////////////////////////////////////////////////////////////////////////////

FMCSoundStyleA::FMCSoundStyleA(Config* main_config, FMCControl* fmc_control) :
    FMCSounds(main_config, CFG_SOUNDS_STYLE_A_FILENAME, fmc_control),
    m_radar_height_inhibit_time_ms(0), m_last_ils_mode(FMCAutopilot::ILS_MODE_NONE),
    m_last_airbus_throttle_mode(FMCAutothrottle::AIRBUS_THROTTLE_IDLE), m_thr_lvr_clb_request_active(false)
{
    setupDefaultConfig();
    m_sounds_cfg->saveToFile();
    
    // load sounds

    loadBaseSounds();
    loadSound(CFG_SOUNDS_ALT_ALERT, m_sounds_cfg->getValue(CFG_SOUNDS_ALT_ALERT), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_ALT_ALERT_CONT, m_sounds_cfg->getValue(CFG_SOUNDS_ALT_ALERT_CONT), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_AP_DISCO, m_sounds_cfg->getValue(CFG_SOUNDS_AP_DISCO), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_AT_DISCO, m_sounds_cfg->getValue(CFG_SOUNDS_AT_DISCO), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_STALL, m_sounds_cfg->getValue(CFG_SOUNDS_STALL), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_SYSTEM_DOWNGRADE, m_sounds_cfg->getValue(CFG_SOUNDS_SYSTEM_DOWNGRADE), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_CHIME, m_sounds_cfg->getValue(CFG_SOUNDS_CHIME), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_OVERSPEED, m_sounds_cfg->getValue(CFG_SOUNDS_OVERSPEED), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_RETARD, m_sounds_cfg->getValue(CFG_SOUNDS_RETARD), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_THRUST_LEVER, m_sounds_cfg->getValue(CFG_SOUNDS_THRUST_LEVER), FMCSoundBase::SOUND_SOURCE_MECHANICAL);
    loadSound(CFG_SOUNDS_CLIMB_THRUST, m_sounds_cfg->getValue(CFG_SOUNDS_CLIMB_THRUST), FMCSoundBase::SOUND_SOURCE_OTHER_PILOT);
    loadSound(CFG_SOUNDS_FLEX_THRUST, m_sounds_cfg->getValue(CFG_SOUNDS_FLEX_THRUST), FMCSoundBase::SOUND_SOURCE_OTHER_PILOT);
    loadSound(CFG_SOUNDS_MCT_THRUST, m_sounds_cfg->getValue(CFG_SOUNDS_MCT_THRUST), FMCSoundBase::SOUND_SOURCE_OTHER_PILOT);
    loadSound(CFG_SOUNDS_TOGA_THRUST, m_sounds_cfg->getValue(CFG_SOUNDS_TOGA_THRUST), FMCSoundBase::SOUND_SOURCE_OTHER_PILOT);
    
    if (getSound(CFG_SOUNDS_STALL)) getSound(CFG_SOUNDS_STALL)->setPlayLooped(true);
    if (getSound(CFG_SOUNDS_OVERSPEED)) getSound(CFG_SOUNDS_OVERSPEED)->setPlayLooped(true);
    if (getSound(CFG_SOUNDS_ALT_ALERT_CONT)) getSound(CFG_SOUNDS_ALT_ALERT_CONT)->setPlayLooped(true);
    
    m_ils_mode_inhibit_timer.start();
    m_thr_lvr_clb_request_timer.start();
    m_airbus_thrust_change_timer.start();
}

/////////////////////////////////////////////////////////////////////////////

FMCSoundStyleA::~FMCSoundStyleA()
{
}

/////////////////////////////////////////////////////////////////////////////

void FMCSoundStyleA::setupDefaultConfig()
{
    m_sounds_cfg->setValue(CFG_SOUNDS_BASE_FOLDER, "sounds/a");

    m_sounds_cfg->setValue(CFG_SOUNDS_ALT_ALERT, "alt_alert.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_ALT_ALERT_CONT, "alt_alert_cont.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_AP_DISCO, "apdisco.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_AT_DISCO, "chime.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_STALL, "stall_voice_only.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_SYSTEM_DOWNGRADE, "sys_downgrade.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_CHIME, "chime.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_OVERSPEED, "chime.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_RETARD, "retard.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_MINIMUMS, "min.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_APP_MINIMUMS, "app_min.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_ACARS_CALL, "acars.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_FLAPS_1, "flaps_1.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_FLAPS_2, "flaps_2.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_FLAPS_3, "flaps_3.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_FLAPS_4, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_FLAPS_5, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_FLAPS_6, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_THRUST_LEVER, "thr_lever.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_CLIMB_THRUST, "climb_thrust.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_FLEX_THRUST, "flex_thrust.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_MCT_THRUST, "mct_thrust.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_TOGA_THRUST, "toga_thrust.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_REVERSERS_OK, "reversers_ok.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_GND_SPOILERS, "gnd_spoilers.wav");
}

/////////////////////////////////////////////////////////////////////////////

void FMCSoundStyleA::checkSounds()
{
    //----- check ILS mode

    if (!m_flightstatus->onground &&
        m_fmc_control->fmcAutoPilot().ilsMode() < m_last_ils_mode &&
        m_ils_mode_inhibit_timer.elapsed() > 1000 &&
        (m_fmc_control->showPFDILS(true) || m_fmc_control->showPFDILS(false)))
    {
        addSoundToQueue(CFG_SOUNDS_SYSTEM_DOWNGRADE);
        m_ils_mode_inhibit_timer.start();
    }

    m_last_ils_mode = m_fmc_control->fmcAutoPilot().ilsMode();

    //----- check radar alt

    double radar_alt = m_flightstatus->radarAltitude();
    double vs = m_flightstatus->smoothedVS();

    if (!m_flightstatus->onground && radar_alt < 2600)
    {
        if (m_last_radar_alt > 0)
        {
            if (radar_alt <= 21 && 
                m_last_radar_alt <= 21 && 
                m_flightstatus->isGearDown() && 
                vs < 0 &&
                m_flightstatus->getMaximumThrottleLeverInputPercent() > 2.5)
            {
                if (m_radar_height_inhibit_time_ms <= 0 ||
                    m_radar_height_inhibit_timer.elapsed() > m_radar_height_inhibit_time_ms)
                {
                    Logger::log(QString("FMCSoundStyleA:checkSounds: retard: max. thr/lever=%1").
                                arg(m_flightstatus->getMaximumThrottleLeverInputPercent()));

                    addSoundToQueue(CFG_SOUNDS_RETARD);
                    m_radar_height_inhibit_timer.start();
                    m_radar_height_inhibit_time_ms = 1000;
                }
            }
            else
            {
                m_radar_height_inhibit_time_ms = 0;
                
                bool check_radar_height = true;
                double minimum = m_fmc_control->fmcData().decisionHeightFt();

                if (minimum > 0.0)
                {
                    if (radar_alt <= minimum+105 && m_last_radar_alt > minimum+105) 
                    {
                        addSoundToQueue(CFG_SOUNDS_APP_MINIMUMS);
                        check_radar_height = false;
                    }
                    else if (radar_alt <= minimum+5 && m_last_radar_alt > minimum+5) 
                    {
                        addSoundToQueue(CFG_SOUNDS_MINIMUMS);
                        check_radar_height = false;
                    }
                }

                if (check_radar_height)
                {
                    if (radar_alt <= 6 && m_last_radar_alt > 6) addSoundToQueue(CFG_SOUNDS_5_FT);
                    else if (radar_alt <= 15 && m_last_radar_alt > 15) addSoundToQueue(CFG_SOUNDS_10_FT);
                    else if (radar_alt <= 25 && m_last_radar_alt > 25) addSoundToQueue(CFG_SOUNDS_20_FT);
                    else if (radar_alt <= 35 && m_last_radar_alt > 35) addSoundToQueue(CFG_SOUNDS_30_FT);
                    else if (radar_alt <= 45 && m_last_radar_alt > 45) addSoundToQueue(CFG_SOUNDS_40_FT);
                    else if (radar_alt <= 55 && m_last_radar_alt > 55) addSoundToQueue(CFG_SOUNDS_50_FT);
                    else if (radar_alt <= 105 && m_last_radar_alt > 105) addSoundToQueue(CFG_SOUNDS_100_FT);
                    else if (radar_alt <= 205 && m_last_radar_alt > 205) addSoundToQueue(CFG_SOUNDS_200_FT);
                    else if (radar_alt <= 305 && m_last_radar_alt > 305) addSoundToQueue(CFG_SOUNDS_300_FT);
                    else if (radar_alt <= 405 && m_last_radar_alt > 405) addSoundToQueue(CFG_SOUNDS_400_FT);
                    else if (radar_alt <= 505 && m_last_radar_alt > 505) addSoundToQueue(CFG_SOUNDS_500_FT);
                    else if (radar_alt <= 1005 && m_last_radar_alt > 1005) addSoundToQueue(CFG_SOUNDS_1000_FT);
                    else if (radar_alt <= 2505 && m_last_radar_alt > 2505) addSoundToQueue(CFG_SOUNDS_2500_FT);
                }
            }
        }

        m_last_radar_alt = m_flightstatus->radarAltitude();
    }

    //------ check for lost ILS signal

    if (!m_flightstatus->onground)
    {
        if (!m_flightstatus->nav1.id().isEmpty() && m_flightstatus->nav1_has_loc)
        {
            m_had_received_ils = true;
        }
        else
        {
            if (m_had_received_ils && m_flightstatus->isGearDown() &&
                (m_fmc_control->showPFDILS(true) || m_fmc_control->showPFDILS(false)))
                addSoundToQueue(CFG_SOUNDS_SYSTEM_DOWNGRADE);
            m_had_received_ils = false;
        }
    }

    //----- check flaps

    if (m_startup_timer.elapsed() < 10000)
    {
        m_prev_flaps_notch = m_flightstatus->current_flap_lever_notch;
    }
    else if (m_flightstatus->current_flap_lever_notch != m_prev_flaps_notch &&
             m_flightstatus->current_flap_lever_notch < m_flightstatus->flaps_lever_notch_count-1)
    {
        QString play_sound;

        switch(m_flightstatus->current_flap_lever_notch)
        {
            case(1):
                if (m_prev_flaps_notch != 2) play_sound = CFG_SOUNDS_FLAPS_1; 
                break;
            case(2): 
                if (m_prev_flaps_notch != 1) play_sound = CFG_SOUNDS_FLAPS_1; 
                break;
            case(3): play_sound = CFG_SOUNDS_FLAPS_2; break;
            case(4): play_sound = CFG_SOUNDS_FLAPS_3; break;
        }

        if (!play_sound.isEmpty())
        {
            m_flaps_next_callout = play_sound;
            m_flaps_callout_delay_timer.start();
        }
    }

    //----- check airbus throttle modes

    if (m_fmc_control->fmcAutothrottle().useAirbusThrottleModes())
    {
        FMCAutothrottle::AIRBUS_THROTTLE_MODE current_airbus_throttle_mode = 
            m_fmc_control->fmcAutothrottle().currentAirbusThrottleMode();

        //----- check for THR CL detent request
        
        if (m_fmc_control->flightStatusChecker().thrustLeverClimbDetentRequest())
        {
            if (!m_thr_lvr_clb_request_active)
            {
                m_thr_lvr_clb_request_active = true;
                m_thr_lvr_clb_request_timer.start();
            }
            else if (m_thr_lvr_clb_request_timer.elapsed() >= 5000)
            {
                addSoundToQueue(CFG_SOUNDS_CHIME);
                m_thr_lvr_clb_request_timer.start();            
            }
        }
        else
        {
            m_thr_lvr_clb_request_active = false;
        }
        
        if (m_last_airbus_throttle_mode != current_airbus_throttle_mode)
        {
            if (!m_flightstatus->isReverserOn())
            {
                //----- check airbus throttle detents
                
                // play the sound for each detent passed
                uint count = m_fmc_control->fmcAutothrottle().airbusDetentCount(
                    m_last_airbus_throttle_mode, current_airbus_throttle_mode);
                
                while(count > 0)
                {
                    addSoundToQueue(CFG_SOUNDS_THRUST_LEVER);
                    --count;
                }
                
                //----- set changed thrust mode callout

                switch(current_airbus_throttle_mode)
                {
                    case(FMCAutothrottle::AIRBUS_THROTTLE_CLIMB):
                        m_airbus_thrust_next_callout = CFG_SOUNDS_CLIMB_THRUST;
                        break;
                    case(FMCAutothrottle::AIRBUS_THROTTLE_FLEX):
                        m_airbus_thrust_next_callout = CFG_SOUNDS_FLEX_THRUST;
                        break;
                    case(FMCAutothrottle::AIRBUS_THROTTLE_MCT):
                        m_airbus_thrust_next_callout = CFG_SOUNDS_MCT_THRUST;
                        break;
                    case(FMCAutothrottle::AIRBUS_THROTTLE_TOGA):
                        m_airbus_thrust_next_callout = CFG_SOUNDS_TOGA_THRUST;
                        break;
                    default:
                        m_airbus_thrust_next_callout.clear();
                        break;
                }

                m_airbus_thrust_change_timer.start();
            }
            else if (!m_airbus_thrust_next_callout.isEmpty())
            {
                m_airbus_thrust_next_callout.clear();
            }
        }

        //----- play changed thrust mode callout after a delay
        
        if (!m_airbus_thrust_next_callout.isEmpty() && m_airbus_thrust_change_timer.elapsed() > 1500)
        {
            addSoundToQueue(m_airbus_thrust_next_callout);
            m_airbus_thrust_next_callout.clear();
        }
        
        m_last_airbus_throttle_mode = current_airbus_throttle_mode;
    }
}

// End of file
