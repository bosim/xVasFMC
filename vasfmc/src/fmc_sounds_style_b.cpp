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

/*! \file    fmc_sounds_style_b.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "assert.h"
#include "config.h"
#include "defines.h"
#include "flightstatus.h"
#include "fmc_control.h"
#include "fmc_autopilot.h"

#include "fmc_sounds_defines.h"
#include "fmc_sounds_style_b.h"

/////////////////////////////////////////////////////////////////////////////

FMCSoundStyleB::FMCSoundStyleB(Config* main_config, FMCControl* fmc_control) :
    FMCSounds(main_config, CFG_SOUNDS_STYLE_B_FILENAME, fmc_control)
{
    setupDefaultConfig();
    m_sounds_cfg->saveToFile();

    // load sounds

    loadBaseSounds();
    loadSound(CFG_SOUNDS_TOO_LOW_GEAR, m_sounds_cfg->getValue(CFG_SOUNDS_TOO_LOW_GEAR), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_TOO_LOW_FLAPS, m_sounds_cfg->getValue(CFG_SOUNDS_TOO_LOW_FLAPS), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_APP_MINIMUMS, m_sounds_cfg->getValue(CFG_SOUNDS_APP_MINIMUMS), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_MINIMUMS, m_sounds_cfg->getValue(CFG_SOUNDS_MINIMUMS), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_ALT_ALERT, m_sounds_cfg->getValue(CFG_SOUNDS_ALT_ALERT), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_ALT_ALERT_CONT, m_sounds_cfg->getValue(CFG_SOUNDS_ALT_ALERT_CONT), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_AP_DISCO, m_sounds_cfg->getValue(CFG_SOUNDS_AP_DISCO), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_AT_DISCO, m_sounds_cfg->getValue(CFG_SOUNDS_AT_DISCO), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_STALL, m_sounds_cfg->getValue(CFG_SOUNDS_STALL), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_CHIME, m_sounds_cfg->getValue(CFG_SOUNDS_CHIME), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_OVERSPEED, m_sounds_cfg->getValue(CFG_SOUNDS_OVERSPEED), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_ALT_ALERT_CONT, m_sounds_cfg->getValue(CFG_SOUNDS_ALT_ALERT_CONT), FMCSoundBase::SOUND_SOURCE_COMPUTER);

    if (getSound(CFG_SOUNDS_STALL)) getSound(CFG_SOUNDS_STALL)->setPlayLooped(true);
    if (getSound(CFG_SOUNDS_OVERSPEED)) getSound(CFG_SOUNDS_OVERSPEED)->setPlayLooped(true);
    if (getSound(CFG_SOUNDS_ALT_ALERT_CONT)) getSound(CFG_SOUNDS_ALT_ALERT_CONT)->setPlayLooped(true);
}

/////////////////////////////////////////////////////////////////////////////

FMCSoundStyleB::~FMCSoundStyleB()
{
}

/////////////////////////////////////////////////////////////////////////////

void FMCSoundStyleB::setupDefaultConfig()
{
    m_sounds_cfg->setValue(CFG_SOUNDS_BASE_FOLDER, "sounds/b");

    m_sounds_cfg->setValue(CFG_SOUNDS_TOO_LOW_GEAR, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_TOO_LOW_FLAPS, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_APP_MINIMUMS, "app_minimums.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_MINIMUMS, "minimums.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_ALT_ALERT, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_ALT_ALERT_CONT, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_AP_DISCO, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_AT_DISCO, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_STALL, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_CHIME, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_OVERSPEED, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_FLAPS_1, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_FLAPS_2, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_FLAPS_3, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_FLAPS_4, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_FLAPS_5, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_FLAPS_6, "");
}

/////////////////////////////////////////////////////////////////////////////

void FMCSoundStyleB::checkSounds()
{
    //----- check radar alt

    double radar_alt = m_flightstatus->radarAltitude();
    
    if (!m_flightstatus->onground && radar_alt < 2600)
    {
        if (m_last_radar_alt > 0)
        {
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
                else if (radar_alt <= 11 && m_last_radar_alt > 11) addSoundToQueue(CFG_SOUNDS_10_FT);
                else if (radar_alt <= 21 && m_last_radar_alt > 21) addSoundToQueue(CFG_SOUNDS_20_FT);
                else if (radar_alt <= 31 && m_last_radar_alt > 31) addSoundToQueue(CFG_SOUNDS_30_FT);
                else if (radar_alt <= 41 && m_last_radar_alt > 41) addSoundToQueue(CFG_SOUNDS_40_FT);
                else if (radar_alt <= 51 && m_last_radar_alt > 51) addSoundToQueue(CFG_SOUNDS_50_FT);
                else if (radar_alt <= 101 && m_last_radar_alt > 101) addSoundToQueue(CFG_SOUNDS_100_FT);
                else if (radar_alt <= 201 && m_last_radar_alt > 201) addSoundToQueue(CFG_SOUNDS_200_FT);
                else if (radar_alt <= 301 && m_last_radar_alt > 301) addSoundToQueue(CFG_SOUNDS_300_FT);
                else if (radar_alt <= 401 && m_last_radar_alt > 401) addSoundToQueue(CFG_SOUNDS_400_FT);
                else if (radar_alt <= 501 && m_last_radar_alt > 501) addSoundToQueue(CFG_SOUNDS_500_FT);
                else if (radar_alt <= 1001 && m_last_radar_alt > 1001) addSoundToQueue(CFG_SOUNDS_1000_FT);
                else if (radar_alt <= 2501 && m_last_radar_alt > 2501) addSoundToQueue(CFG_SOUNDS_2500_FT);
            }
        }

        m_last_radar_alt = m_flightstatus->radarAltitude();
    }

    //----- check flaps

    if (m_startup_timer.elapsed() < 10000)
        m_prev_flaps_notch = m_flightstatus->current_flap_lever_notch;

    if (m_flightstatus->current_flap_lever_notch != m_prev_flaps_notch &&
        m_flightstatus->current_flap_lever_notch < m_flightstatus->flaps_lever_notch_count-1)
    {
        QString play_sound;
        
        switch(m_flightstatus->current_flap_lever_notch)
        {
            case(1): play_sound = CFG_SOUNDS_FLAPS_1; break;
            case(2): play_sound = CFG_SOUNDS_FLAPS_2; break;
            case(3): play_sound = CFG_SOUNDS_FLAPS_3; break;
            case(4): play_sound = CFG_SOUNDS_FLAPS_4; break;
            case(5): play_sound = CFG_SOUNDS_FLAPS_5; break;
            case(6): play_sound = CFG_SOUNDS_FLAPS_6; break;
        }

        if (!play_sound.isEmpty())
        {
            m_flaps_next_callout = play_sound;
            m_flaps_callout_delay_timer.start();
        }
    }
}

// End of file
