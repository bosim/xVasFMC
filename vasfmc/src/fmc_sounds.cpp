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

/*! \file    fmc_sounds.cpp
  \author  Alexander Wemmer, alex@wemmer.at
*/

#include "assert.h"
#include "config.h"
#include "flightstatus.h"
#include "fmc_control.h"
#include "aircraft_data.h"
#include "fmc_autothrottle.h"
#include "vas_path.h"

#include "fmc_flightstatus_checker_base.h"

#include "fmc_sounds.h"

#define REACHING_TOD_NM_CALLOUT_LIMIT 20.0

/////////////////////////////////////////////////////////////////////////////

#ifdef USE_QSOUND

FMCSound::FMCSound(const QString & filename, SOUND_SOURCE sound_source) :
    FMCSoundBase(sound_source), QSound(filename), m_play_looped(false)
{
    m_play_dt.start();
}

FMCSound::~FMCSound()
{
}

void FMCSound::play()
{
    m_has_played = true;
    setLoops(m_play_looped ? -1 : 1);
    QSound::play();
    m_play_dt.start();
}

void FMCSound::stop()
{
    m_has_played = false;
    QSound::stop();
}

bool FMCSound::isFinished() const
{

#ifdef Q_OS_WIN32
   // Warning: On Windows this function always returns true for unlooped sounds.
   // So we delay the return for 3 seconds.
   return m_play_dt.elapsed() > 3000;
#else
   return QSound::isFinished();
#endif

}

#endif

/////////////////////////////////////////////////////////////////////////////

#ifdef USE_FMOD

FMCSound::FMCSound(const QString & filename, SOUND_SOURCE sound_source, FMOD_SYSTEM *fmod_system) :
    FMCSoundBase(sound_source), m_filename(filename), m_fmod_system(fmod_system), m_fmod_sound(0), m_fmod_channel(0)
{
    MYASSERT(!filename.isEmpty());
    MYASSERT(m_fmod_system != 0);
    MYASSERT(FMOD_System_CreateSound(m_fmod_system, filename.toLatin1().data(), FMOD_HARDWARE, 0, &m_fmod_sound) == FMOD_OK);
}

FMCSound::~FMCSound()
{
    MYASSERT(FMOD_Sound_Release(m_fmod_sound) == FMOD_OK);
}

void FMCSound::play()
{
    m_has_played = true;
    FMOD_System_PlaySound(m_fmod_system, FMOD_CHANNEL_FREE, m_fmod_sound, 0, &m_fmod_channel);
}

void FMCSound::stop()
{
    m_has_played = false;

    FMOD_System_Update(m_fmod_system);
    if (m_fmod_channel == 0) return;

    FMOD_Channel_Stop(m_fmod_channel);
}

void FMCSound::setPlayLooped(bool yes)
{
    MYASSERT(FMOD_Sound_SetMode(m_fmod_sound, yes ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF) == FMOD_OK);
}

bool FMCSound::isFinished() const
{
    FMOD_System_Update(m_fmod_system);
    if (m_fmod_channel == 0) return true;

    int playing = 0;
    FMOD_RESULT result = FMOD_Channel_IsPlaying(m_fmod_channel, &playing);
    if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE) && (result != FMOD_ERR_CHANNEL_STOLEN))
    {
        MYASSERT(result != FMOD_OK);
    }

    return playing == 0;
}

#endif

/////////////////////////////////////////////////////////////////////////////

#ifdef USE_OPENAL

ALCcontext* AlContext::m_alContext;
ALCdevice* AlContext::m_alDevice;
ALCcontext* AlContext::m_oldContext;

FMCSound::FMCSound(const QString & filename, SOUND_SOURCE sound_source) :
    FMCSoundBase(sound_source), m_filename(filename)
{
    ALfloat sourcePosition[3] = {0.0f, 0.0f, 0.0f};
    ALfloat sourceVelocity[3] = {0.0f, 0.0f, 0.0f};
    ALfloat listenerPosition[3] = {0.0f, 0.0f, 0.0f};
    ALfloat listenerVelocity[3] = {0.0f, 0.0f, 0.0f};
    ALfloat listenerOrientation[6] = {0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f};
    MYASSERT(!filename.isEmpty());
    ALenum format;
    ALsizei size;
    ALvoid* data;
    ALsizei freq;
    ALboolean loop = AL_FALSE;
    alGetError();
    //alGenBuffers(1, &m_buffer);
    //alutLoadWAVFile((ALbyte*)filename.toLatin1().data(), &format, &data, &size, &freq, &loop);
    //alBufferData(m_buffer, format, data, size, freq);
    //alutUnloadWAV(format, data, size, freq);
    m_buffer = alutCreateBufferFromFile(filename.toLatin1().data());

    if( m_buffer == AL_NONE)
    {
        Logger::log("ALUT: Buffer creation failed:");
        Logger::log(alutGetErrorString(alutGetError()));
    } 
    else 
    {
        MYASSERT(alGetError() == AL_NO_ERROR);
        alGenSources(1, &m_source);
        MYASSERT(alGetError() == AL_NO_ERROR);
        alSourcei (m_source, AL_BUFFER,   m_buffer   );
        alSourcef (m_source, AL_PITCH,    1.0      );
        alSourcef (m_source, AL_GAIN,     1.0      );
        alSourcefv(m_source, AL_POSITION, sourcePosition);
        alSourcefv(m_source, AL_VELOCITY, sourceVelocity);
        alSourcei (m_source, AL_LOOPING,  loop     );
        MYASSERT(alGetError() == AL_NO_ERROR);
        alListenerfv(AL_POSITION,    listenerPosition);
        alListenerfv(AL_VELOCITY,    listenerVelocity);
        alListenerfv(AL_ORIENTATION, listenerOrientation);
    }
}

FMCSound::~FMCSound()
{
    alDeleteSources(1, &m_source);
    alDeleteBuffers(1, &m_buffer);
}

void FMCSound::play()
{
    m_has_played = true;
    alSourcePlay(m_source);
}

void FMCSound::stop()
{
    m_has_played = false;
    alSourceStop(m_source);
}

void FMCSound::setPlayLooped(bool yes)
{
    ALboolean loop = yes ? AL_TRUE : AL_FALSE;
    alSourcei (m_source, AL_LOOPING,  loop  );
}

bool FMCSound::isFinished() const
{
    ALint state;
    alGetSourcei(m_source,AL_SOURCE_STATE,&state);
    return (state == AL_STOPPED);
}

#endif

/////////////////////////////////////////////////////////////////////////////

FMCSounds::FMCSounds(Config* main_config, const QString& sounds_cfg_filename, FMCControl* fmc_control) :
    m_main_config(main_config), m_sounds_cfg(0), m_fmc_control(fmc_control), m_flightstatus(fmc_control->flightStatus()),
    m_last_radar_alt(0), m_is_ap_connected(false), m_is_at_connected(false), m_had_received_ils(false),
    m_do_80kts_callout(false), m_do_v1_callout(false), m_do_vr_callout(false), m_do_v2_callout(false),
    m_do_positive_rate_callout(false), m_do_gear_up_callout(false),
    m_do_gear_down_callout(false), m_do_loc_alive_callout(false), m_do_gs_alive_callout(false),
    m_do_reaching_tod_callout(false), m_do_reverser_callout(true), m_do_gnd_spoiler_callout(true),
    m_prev_flaps_notch(m_flightstatus->current_flap_lever_notch), 
    m_prev_alt_ft(m_flightstatus->smoothedAltimeterReadout()), m_was_above_mda(false), m_direct_play_sound_id(0)
{
    MYASSERT(m_main_config != 0);
    MYASSERT(m_fmc_control != 0);
    MYASSERT(m_flightstatus != 0);

    // setup config

    m_sounds_cfg = new Config(sounds_cfg_filename);
    MYASSERT(m_sounds_cfg != 0);
    setupDefaultBaseConfig();
    m_sounds_cfg->loadfromFile();
    m_sounds_cfg->saveToFile();

    m_startup_timer.start();
    m_outermarker_inhibit_timer.start();
    m_flaps_callout_delay_timer.start();
    m_10000ft_inhibit_timer.start();
    m_last_reverser_callout_timer.start();
    m_loc_alive_callout_timer.start();
    m_gs_alive_callout_timer.start();

    // setup refresh timer

    m_refresh_timer.start();

    //-----

#ifdef USE_QSOUND
    Logger::log("FMCSounds: Using QSound");
#endif

    //-----

#ifdef USE_FMOD

    Logger::log("FMCSounds: Using FMOD Library");

    m_fmod_system = 0;
    unsigned int fmod_version = 0;

    MYASSERT(FMOD_System_Create(&m_fmod_system) == FMOD_OK);

    MYASSERT(FMOD_System_GetVersion(m_fmod_system, &fmod_version) == FMOD_OK);

    if (fmod_version < FMOD_VERSION)
    {
        Logger::log(QString("FMCSounds: You are using an old version of FMOD "
                            "%1.  This program requires %2\n").arg(fmod_version).arg(FMOD_VERSION));
    }

    MYASSERT(FMOD_System_Init(m_fmod_system, 32, FMOD_INIT_NORMAL, NULL) == FMOD_OK);

#endif

    //-----

#ifdef USE_OPENAL
    Logger::log("FMCSounds: Using OpenAL Library");
    /*m_oldContext = alcGetCurrentContext();
    m_alDevice = alcOpenDevice(0);
    m_alContext = alcCreateContext(m_alDevice,0);
    alcMakeContextCurrent(m_alContext);
    alGetError();
    alutInitWithoutContext(NULL,0);
    MYASSERT(alutGetError()==ALUT_ERROR_NO_ERROR);*/
    m_alContext = AlContext::getContext();
#endif

     Logger::log("FMCSounds: fin");
}

/////////////////////////////////////////////////////////////////////////////

FMCSounds::~FMCSounds()
{
    qDeleteAll(m_sound_map);
    m_sounds_cfg->saveToFile();
    delete m_sounds_cfg;

#ifdef USE_FMOD
    MYASSERT(FMOD_System_Close(m_fmod_system) == FMOD_OK);
    MYASSERT(FMOD_System_Release(m_fmod_system) == FMOD_OK);
#endif

#ifdef USE_OPENAL
    /*alcMakeContextCurrent(m_oldContext);
    alcDestroyContext(m_alContext);
    alcCloseDevice(m_alDevice);
    alutExit();*/
#endif
}

/////////////////////////////////////////////////////////////////////////////

void FMCSounds::slotPlay1000FtToGo()
{
    addSoundToQueue(CFG_SOUNDS_1000_TO_GO);
}

/////////////////////////////////////////////////////////////////////////////

void FMCSounds::setupDefaultBaseConfig()
{
    m_sounds_cfg->setValue(CFG_SOUNDS_REFRESH_PERIOD_MS, 200);
    m_sounds_cfg->setValue(CFG_SOUNDS_BASE_FOLDER, "");

    m_sounds_cfg->setValue(CFG_SOUNDS_EMPTY, "../common/empty.wav");

    m_sounds_cfg->setValue(CFG_SOUNDS_ALT_ALERT, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_ALT_ALERT_CONT, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_AP_DISCO, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_AT_DISCO, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_STALL, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_SYSTEM_DOWNGRADE, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_CHIME, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_OVERSPEED, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_RETARD, "");

    m_sounds_cfg->setValue(CFG_SOUNDS_2500_FT, "2500.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_1000_FT, "1000.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_500_FT, "500.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_400_FT, "400.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_300_FT, "300.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_200_FT, "200.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_100_FT, "100.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_50_FT,  "50.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_40_FT,  "40.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_30_FT,  "30.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_20_FT,  "20.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_10_FT,  "");
    m_sounds_cfg->setValue(CFG_SOUNDS_5_FT,   "");

    m_sounds_cfg->setValue(CFG_SOUNDS_FLAPS_1, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_FLAPS_2, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_FLAPS_3, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_FLAPS_4, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_FLAPS_5, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_FLAPS_6, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_TOO_LOW_GEAR, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_TOO_LOW_FLAPS, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_APP_MINIMUMS, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_MINIMUMS, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_REVERSERS_OK, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_GND_SPOILERS, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_ACARS_CALL, "");

    m_sounds_cfg->setValue(CFG_SOUNDS_80KTS, "../common/80kts.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_V1, "../common/v1.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_VR, "../common/vr.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_V2, "");
    m_sounds_cfg->setValue(CFG_SOUNDS_POSITIVE_RATE, "../common/positive_rate.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_GEAR_UP, "../common/gear_up.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_GEAR_DOWN, "../common/gear_down.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_FLAPS_UP, "../common/flaps_up.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_FLAPS_FULL, "../common/flaps_full.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_1000_TO_GO, "../common/1000_to_go.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_LOC_ALIVE, "../common/loc_alive.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_GS_ALIVE, "../common/gs_alive.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_REACHING_TOD, "../common/tod.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_10000_FT, "../common/10000ft.wav");
    m_sounds_cfg->setValue(CFG_SOUNDS_OUTERMARKER, "../common/outermarker.wav");
}

/////////////////////////////////////////////////////////////////////////////

void FMCSounds::loadBaseSounds()
{
    loadSound(CFG_SOUNDS_EMPTY, m_sounds_cfg->getValue(CFG_SOUNDS_EMPTY), FMCSoundBase::SOUND_SOURCE_COMPUTER);

    loadSound(CFG_SOUNDS_ALT_ALERT, m_sounds_cfg->getValue(CFG_SOUNDS_ALT_ALERT), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_ALT_ALERT_CONT, m_sounds_cfg->getValue(CFG_SOUNDS_ALT_ALERT_CONT), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_AP_DISCO, m_sounds_cfg->getValue(CFG_SOUNDS_AP_DISCO), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_AT_DISCO, m_sounds_cfg->getValue(CFG_SOUNDS_AT_DISCO), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_STALL, m_sounds_cfg->getValue(CFG_SOUNDS_STALL), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_SYSTEM_DOWNGRADE, m_sounds_cfg->getValue(CFG_SOUNDS_SYSTEM_DOWNGRADE), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_CHIME, m_sounds_cfg->getValue(CFG_SOUNDS_CHIME), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_OVERSPEED, m_sounds_cfg->getValue(CFG_SOUNDS_OVERSPEED), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_RETARD, m_sounds_cfg->getValue(CFG_SOUNDS_RETARD), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_2500_FT, m_sounds_cfg->getValue(CFG_SOUNDS_2500_FT), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_1000_FT, m_sounds_cfg->getValue(CFG_SOUNDS_1000_FT), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_500_FT, m_sounds_cfg->getValue(CFG_SOUNDS_500_FT), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_400_FT, m_sounds_cfg->getValue(CFG_SOUNDS_400_FT), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_300_FT, m_sounds_cfg->getValue(CFG_SOUNDS_300_FT), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_200_FT, m_sounds_cfg->getValue(CFG_SOUNDS_200_FT), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_100_FT, m_sounds_cfg->getValue(CFG_SOUNDS_100_FT), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_50_FT, m_sounds_cfg->getValue(CFG_SOUNDS_50_FT), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_40_FT, m_sounds_cfg->getValue(CFG_SOUNDS_40_FT), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_30_FT, m_sounds_cfg->getValue(CFG_SOUNDS_30_FT), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_20_FT, m_sounds_cfg->getValue(CFG_SOUNDS_20_FT), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_10_FT, m_sounds_cfg->getValue(CFG_SOUNDS_10_FT), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_5_FT, m_sounds_cfg->getValue(CFG_SOUNDS_5_FT), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_TOO_LOW_GEAR, m_sounds_cfg->getValue(CFG_SOUNDS_TOO_LOW_GEAR), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_TOO_LOW_FLAPS, m_sounds_cfg->getValue(CFG_SOUNDS_TOO_LOW_FLAPS), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_APP_MINIMUMS, m_sounds_cfg->getValue(CFG_SOUNDS_APP_MINIMUMS), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_MINIMUMS, m_sounds_cfg->getValue(CFG_SOUNDS_MINIMUMS), FMCSoundBase::SOUND_SOURCE_COMPUTER);
    loadSound(CFG_SOUNDS_ACARS_CALL, m_sounds_cfg->getValue(CFG_SOUNDS_ACARS_CALL), FMCSoundBase::SOUND_SOURCE_COMPUTER);

    loadSound(CFG_SOUNDS_80KTS, m_sounds_cfg->getValue(CFG_SOUNDS_80KTS), FMCSoundBase::SOUND_SOURCE_OTHER_PILOT);
    loadSound(CFG_SOUNDS_V1, m_sounds_cfg->getValue(CFG_SOUNDS_V1), FMCSoundBase::SOUND_SOURCE_OTHER_PILOT);
    loadSound(CFG_SOUNDS_VR, m_sounds_cfg->getValue(CFG_SOUNDS_VR), FMCSoundBase::SOUND_SOURCE_OTHER_PILOT);
    loadSound(CFG_SOUNDS_V2, m_sounds_cfg->getValue(CFG_SOUNDS_V2), FMCSoundBase::SOUND_SOURCE_OTHER_PILOT);
    loadSound(CFG_SOUNDS_POSITIVE_RATE, m_sounds_cfg->getValue(CFG_SOUNDS_POSITIVE_RATE), FMCSoundBase::SOUND_SOURCE_OTHER_PILOT);
    loadSound(CFG_SOUNDS_GEAR_UP, m_sounds_cfg->getValue(CFG_SOUNDS_GEAR_UP), FMCSoundBase::SOUND_SOURCE_OTHER_PILOT);
    loadSound(CFG_SOUNDS_GEAR_DOWN, m_sounds_cfg->getValue(CFG_SOUNDS_GEAR_DOWN), FMCSoundBase::SOUND_SOURCE_OTHER_PILOT);
    loadSound(CFG_SOUNDS_FLAPS_UP, m_sounds_cfg->getValue(CFG_SOUNDS_FLAPS_UP), FMCSoundBase::SOUND_SOURCE_OTHER_PILOT);
    loadSound(CFG_SOUNDS_FLAPS_1, m_sounds_cfg->getValue(CFG_SOUNDS_FLAPS_1), FMCSoundBase::SOUND_SOURCE_OTHER_PILOT);
    loadSound(CFG_SOUNDS_FLAPS_2, m_sounds_cfg->getValue(CFG_SOUNDS_FLAPS_2), FMCSoundBase::SOUND_SOURCE_OTHER_PILOT);
    loadSound(CFG_SOUNDS_FLAPS_3, m_sounds_cfg->getValue(CFG_SOUNDS_FLAPS_3), FMCSoundBase::SOUND_SOURCE_OTHER_PILOT);
    loadSound(CFG_SOUNDS_FLAPS_4, m_sounds_cfg->getValue(CFG_SOUNDS_FLAPS_4), FMCSoundBase::SOUND_SOURCE_OTHER_PILOT);
    loadSound(CFG_SOUNDS_FLAPS_5, m_sounds_cfg->getValue(CFG_SOUNDS_FLAPS_5), FMCSoundBase::SOUND_SOURCE_OTHER_PILOT);
    loadSound(CFG_SOUNDS_FLAPS_6, m_sounds_cfg->getValue(CFG_SOUNDS_FLAPS_6), FMCSoundBase::SOUND_SOURCE_OTHER_PILOT);

    loadSound(CFG_SOUNDS_REVERSERS_OK, m_sounds_cfg->getValue(CFG_SOUNDS_REVERSERS_OK), FMCSoundBase::SOUND_SOURCE_OTHER_PILOT);
    loadSound(CFG_SOUNDS_GND_SPOILERS, m_sounds_cfg->getValue(CFG_SOUNDS_GND_SPOILERS), FMCSoundBase::SOUND_SOURCE_OTHER_PILOT);
    loadSound(CFG_SOUNDS_FLAPS_FULL, m_sounds_cfg->getValue(CFG_SOUNDS_FLAPS_FULL), FMCSoundBase::SOUND_SOURCE_OTHER_PILOT);
    loadSound(CFG_SOUNDS_1000_TO_GO, m_sounds_cfg->getValue(CFG_SOUNDS_1000_TO_GO), FMCSoundBase::SOUND_SOURCE_OTHER_PILOT);
    loadSound(CFG_SOUNDS_LOC_ALIVE, m_sounds_cfg->getValue(CFG_SOUNDS_LOC_ALIVE), FMCSoundBase::SOUND_SOURCE_OTHER_PILOT);
    loadSound(CFG_SOUNDS_GS_ALIVE, m_sounds_cfg->getValue(CFG_SOUNDS_GS_ALIVE), FMCSoundBase::SOUND_SOURCE_OTHER_PILOT);
    loadSound(CFG_SOUNDS_REACHING_TOD, m_sounds_cfg->getValue(CFG_SOUNDS_REACHING_TOD), FMCSoundBase::SOUND_SOURCE_OTHER_PILOT);
    loadSound(CFG_SOUNDS_10000_FT, m_sounds_cfg->getValue(CFG_SOUNDS_10000_FT), FMCSoundBase::SOUND_SOURCE_OTHER_PILOT);
    loadSound(CFG_SOUNDS_OUTERMARKER, m_sounds_cfg->getValue(CFG_SOUNDS_OUTERMARKER), FMCSoundBase::SOUND_SOURCE_OTHER_PILOT);

    if (getSound(CFG_SOUNDS_STALL)) getSound(CFG_SOUNDS_STALL)->setPlayLooped(true);
    if (getSound(CFG_SOUNDS_OVERSPEED)) getSound(CFG_SOUNDS_OVERSPEED)->setPlayLooped(true);
    if (getSound(CFG_SOUNDS_ALT_ALERT_CONT)) getSound(CFG_SOUNDS_ALT_ALERT_CONT)->setPlayLooped(true);

    // we do this to init the sound system to inhibit the delay for the first sound to be played.
    addSoundToQueue(CFG_SOUNDS_EMPTY);
}

/////////////////////////////////////////////////////////////////////////////

void FMCSounds::loadSound(const QString& sound_name, 
                          const QString& sound_filename,
                          FMCSoundBase::SOUND_SOURCE sound_source)
{
    if (sound_filename.isEmpty()) return;
    if (m_sound_map.contains(sound_name)) delete m_sound_map.take(sound_name);

    QString filename = VasPath::prependPath(m_sounds_cfg->getValue(CFG_SOUNDS_BASE_FOLDER)+"/"+sound_filename);
    if (!QFile::exists(filename))
    {
        Logger::log(QString("Could not load sound %1 (%2)").arg(sound_name).arg(filename));
        return;
    }

#ifdef USE_QSOUND
    m_sound_map.insert(sound_name, new FMCSound(filename, sound_source));
#endif

#ifdef USE_FMOD
    m_sound_map.insert(sound_name, new FMCSound(filename, sound_source, m_fmod_system));
#endif

#ifdef USE_OPENAL
    m_sound_map.insert(sound_name, new FMCSound(filename, sound_source));
#endif
}

/////////////////////////////////////////////////////////////////////////////

void FMCSounds::addSoundToQueue(const QString& sound_name, bool force)
{
    if (m_flightstatus->paused || m_flightstatus->slew ||
        !m_fmc_control->soundsEnabled() || !m_sound_map.contains(sound_name)) return;

    if (!m_flightstatus->isValid() && !force) return;

    FMCSoundBase::SOUND_SOURCE sound_source = m_sound_map[sound_name]->soundSource();
    if (!m_fmc_control->soundChannelsEnabled()) sound_source = FMCSoundBase::SOUND_SOURCE_UNKNOWN;

    m_sound_queue_map[sound_source].append(sound_name);

    //Logger::log(QString("FMCSounds:addSoundToQueue: added (%1) to queue").arg(sound_name));
}

/////////////////////////////////////////////////////////////////////////////

void FMCSounds::addSoundToQueueDirectly(const QString& sound_filename,
                                        FMCSoundBase::SOUND_SOURCE sound_source,
                                        bool force)
{
    QString sound_name;

    while(sound_name.isEmpty() || m_sound_map.contains(sound_name))
        sound_name = QString("GeneratedDirectPlaySound::%1").arg(m_direct_play_sound_id++);

    loadSound(sound_name, sound_filename, sound_source);
    addSoundToQueue(sound_name, force);
}

/////////////////////////////////////////////////////////////////////////////

void FMCSounds::slotCheckSoundsTimer()
{
    if (!m_flightstatus->isValid() || m_flightstatus->paused || m_flightstatus->slew)
    {
        if (m_sound_map.contains(CFG_SOUNDS_ALT_ALERT_CONT) &&
            m_sound_map[CFG_SOUNDS_ALT_ALERT_CONT]->hasPlayed())
            m_sound_map[CFG_SOUNDS_ALT_ALERT_CONT]->stop();

        if (m_sound_map.contains(CFG_SOUNDS_STALL) &&
            m_sound_map[CFG_SOUNDS_STALL]->hasPlayed())
            m_sound_map[CFG_SOUNDS_STALL]->stop();

        if (m_sound_map.contains(CFG_SOUNDS_OVERSPEED) &&
            m_sound_map[CFG_SOUNDS_OVERSPEED]->hasPlayed())
            m_sound_map[CFG_SOUNDS_OVERSPEED]->stop();

        if (m_flightstatus->paused || m_flightstatus->slew) return;
    }

    if (m_refresh_timer.elapsed() < m_sounds_cfg->getIntValue(CFG_SOUNDS_REFRESH_PERIOD_MS)) return;
    m_refresh_timer.start();

    // check for sounds to play

    checkBaseSounds();
    checkSounds();

    // check sound queue

    int index = FMCSoundBase::SOUND_SOURCE_UNKNOWN;
    for (; index <= FMCSoundBase::SOUND_SOURCE_OTHER_PILOT; ++index)
    {
        FMCSoundBase::SOUND_SOURCE sound_source_index = (FMCSoundBase::SOUND_SOURCE)index;

        while(!m_sound_queue_map[sound_source_index].isEmpty())
        {
            QString sound_name = m_sound_queue_map[sound_source_index][0];

            if (!m_sound_map[sound_name]->hasPlayed())
            {
                //Logger::log(QString("FMCSounds:slotCheckSoundsTimer: playing (%1) from queue").
                //arg(sound_name));
                
                m_sound_map[sound_name]->play();
                break;
            }
            else
            {
                if (!m_sound_map[sound_name]->isFinished()) break;
                
                //Logger::log(QString("FMCSounds:slotCheckSoundsTimer: removing (%1) from queue").
                //arg(sound_name));
                
                m_sound_map[sound_name]->stop();
                m_sound_queue_map[sound_source_index].removeFirst();

                if (m_sound_map[sound_name]->doDeleteSoundAfterPlayed())
                {
                    delete m_sound_map[sound_name];
                    m_sound_map.remove(sound_name);
                }
            }
        }
    }

    // set history values

    m_prev_flaps_notch = m_flightstatus->current_flap_lever_notch;
    m_prev_alt_ft = m_flightstatus->smoothedAltimeterReadout();
}

/////////////////////////////////////////////////////////////////////////////

void FMCSounds::checkBaseSounds()
{
    double ias = m_flightstatus->smoothedIAS();
    double vs = m_flightstatus->smoothedVS();
    double alt = m_flightstatus->smoothedAltimeterReadout();

    //----- check for stall

    if (!m_flightstatus->onground &&
        (m_flightstatus->stall || ias < m_fmc_control->aircraftData().getStallSpeed()))
    {
        if (m_sound_map.contains(CFG_SOUNDS_STALL) &&
            !m_sound_map[CFG_SOUNDS_STALL]->hasPlayed())
        {
            m_sound_map[CFG_SOUNDS_STALL]->play();
        }
    }
    else
    {
        if (m_sound_map.contains(CFG_SOUNDS_STALL) &&
            m_sound_map[CFG_SOUNDS_STALL]->hasPlayed())
        {
            m_sound_map[CFG_SOUNDS_STALL]->stop();
        }
    }

    //----- check for overspeed

    if (!m_flightstatus->onground && ias > 50 && ias > m_fmc_control->aircraftData().getMaxSpeedKts())
    {
        if (m_sound_map.contains(CFG_SOUNDS_OVERSPEED) &&
            !m_sound_map[CFG_SOUNDS_OVERSPEED]->hasPlayed())
        {
            m_sound_map[CFG_SOUNDS_OVERSPEED]->play();
        }
    }
    else
    {
        if (m_sound_map.contains(CFG_SOUNDS_OVERSPEED) &&
            m_sound_map[CFG_SOUNDS_OVERSPEED]->hasPlayed())
        {
            m_sound_map[CFG_SOUNDS_OVERSPEED]->stop();
        }
    }

    //----- check AP disco

    if (m_is_ap_connected && !m_flightstatus->ap_enabled) addSoundToQueue(CFG_SOUNDS_AP_DISCO);
    m_is_ap_connected = m_flightstatus->ap_enabled;

    //----- altitude deviation alert

    if (m_fmc_control->flightStatusChecker().isAltitudeDeviationAlert())
    {
        if (m_sound_map.contains(CFG_SOUNDS_ALT_ALERT_CONT) &&
            !m_sound_map[CFG_SOUNDS_ALT_ALERT_CONT]->hasPlayed())
        {
            m_sound_map[CFG_SOUNDS_ALT_ALERT_CONT]->play();
        }
    }
    else
    {
        if (m_sound_map.contains(CFG_SOUNDS_ALT_ALERT_CONT) &&
            m_sound_map[CFG_SOUNDS_ALT_ALERT_CONT]->hasPlayed())
        {
            m_sound_map[CFG_SOUNDS_ALT_ALERT_CONT]->stop();
        }
    }

    //----- check AT disco

    if (!m_fmc_control->fmcAutothrottle().isAPThrottleArmed())
    {
        if (m_is_at_connected) addSoundToQueue(CFG_SOUNDS_AT_DISCO);
        m_is_at_connected = false;
    }
    else
    {
        m_is_at_connected = m_fmc_control->fmcAutothrottle().isAPThrottleArmed();
    }

    //----- takeoff/landing stuff, gear stuff, loc/gs stuff

    if (m_flightstatus->onground)
    {
        if (ias > 70)
        {
            if (m_do_80kts_callout && ias >= 77 && ias <= 83)
            {
                addSoundToQueue(CFG_SOUNDS_80KTS);
                m_do_80kts_callout = false;
            }
        }
        else if (ias < 40)
        {
            m_do_80kts_callout =
                m_do_v1_callout =
                m_do_vr_callout =
                m_do_v2_callout =
                m_do_positive_rate_callout =
                m_do_gear_up_callout =
                m_do_reaching_tod_callout = true;
        }

        if (m_do_v1_callout && qAbs(ias - m_fmc_control->fmcData().V1()) < 2)
        {
            addSoundToQueue(CFG_SOUNDS_V1);
            m_do_v1_callout = false;
        }
        if (m_do_vr_callout && qAbs(ias - m_fmc_control->fmcData().Vr()) < 2)
        {
            addSoundToQueue(CFG_SOUNDS_VR);
            m_do_vr_callout = false;
        }
        if (m_do_v2_callout && qAbs(ias - m_fmc_control->fmcData().V2()) < 2)
        {
            addSoundToQueue(CFG_SOUNDS_V2);
            m_do_v2_callout = false;
        }
    }
    else
    {
        if (ias > 100) m_do_80kts_callout = true;

        // positive rate

        if (m_do_positive_rate_callout &&
            m_flightstatus->radarAltitude() > 20 &&
            vs > 250.0)
        {
            if (m_flightstatus->radarAltitude() < 300) addSoundToQueue(CFG_SOUNDS_POSITIVE_RATE);
            m_do_positive_rate_callout = false;
        }

        // gear

        if (m_startup_timer.elapsed() > 10000)
        {
            if (!m_flightstatus->isGearDown())
            {
                if (m_do_gear_up_callout)
                {
                    if (!m_flightstatus->onground) addSoundToQueue(CFG_SOUNDS_GEAR_UP);
                    m_do_gear_up_callout = false;
                }
                
                m_do_gear_down_callout = true;
            }
            else
            {
                if (m_do_gear_down_callout)
                {
                    if (!m_flightstatus->onground) addSoundToQueue(CFG_SOUNDS_GEAR_DOWN);
                    m_do_gear_down_callout = false;
                }
                
                m_do_gear_up_callout = true;
            }
        }

        // loc + gs

        if (m_flightstatus->localizerAlive())
        {
            if (m_do_loc_alive_callout && m_flightstatus->radarAltitude() > 500)
            {
                m_do_loc_alive_callout = false;
                
                if (m_loc_alive_callout_timer.elapsed() > 100000 &&
                    (m_flightstatus->ap_app_lock || m_flightstatus->ap_nav1_lock ||
                     m_fmc_control->showPFDILS(true) || m_fmc_control->showPFDILS(false)))
                {
                    addSoundToQueue(CFG_SOUNDS_LOC_ALIVE);
                    m_loc_alive_callout_timer.start();
                }
            }
        }
        else
        {
            m_do_loc_alive_callout = true;
        }

        if (m_flightstatus->glideslopeAlive())
        {
            if (m_do_gs_alive_callout && m_flightstatus->radarAltitude() > 500)
            {
                m_do_gs_alive_callout = false;

                if (m_gs_alive_callout_timer.elapsed() > 10000 &&
                    m_flightstatus->localizerEstablished() &&
                    (m_flightstatus->ap_app_lock || m_flightstatus->ap_gs_lock ||
                     m_fmc_control->showPFDILS(true) || m_fmc_control->showPFDILS(false)))
                {
                    addSoundToQueue(CFG_SOUNDS_GS_ALIVE);
                    m_gs_alive_callout_timer.start();
                }
            }
        }
        else
        {
            m_do_gs_alive_callout = true;
        }


        if (m_do_reaching_tod_callout)
        {
            if (m_fmc_control->normalRoute().todWpt().isValid() &&
                m_fmc_control->fmcData().distanceToTODNm() > 0 &&
                m_fmc_control->fmcData().distanceToTODNm() < REACHING_TOD_NM_CALLOUT_LIMIT &&
                m_flightstatus->radarAltitude() > 1000 &&
                qAbs(vs) < 100.0 &&
                m_fmc_control->fmcData().isOnCruisFlightlevel())
            {
                m_do_reaching_tod_callout = false;
                if (m_fmc_control->fmcData().isOnCruisFlightlevel())
                    addSoundToQueue(CFG_SOUNDS_REACHING_TOD);
            }
        }
        else
        {
            if (m_fmc_control->fmcData().distanceToTODNm() > REACHING_TOD_NM_CALLOUT_LIMIT+1.0)
                m_do_reaching_tod_callout = true;
        }
    }

    //----- check flaps

    if (!m_flaps_next_callout.isEmpty() && m_flaps_callout_delay_timer.elapsed() > 1500)
    {
        addSoundToQueue(m_flaps_next_callout);
        m_flaps_next_callout.clear();
    }

    if (m_startup_timer.elapsed() < 10000)
    {
        m_prev_flaps_notch = m_flightstatus->current_flap_lever_notch;
    }
    else if (m_flightstatus->current_flap_lever_notch != m_prev_flaps_notch)
    {
        if (m_flightstatus->current_flap_lever_notch <= 0)
        {
            m_flaps_next_callout = CFG_SOUNDS_FLAPS_UP;
            m_flaps_callout_delay_timer.start();
        }
        else if (m_flightstatus->current_flap_lever_notch >= m_flightstatus->flaps_lever_notch_count-1)
        {
            m_flaps_next_callout = CFG_SOUNDS_FLAPS_FULL;
            m_flaps_callout_delay_timer.start();
        }
    }

    //----- check for outer marker

    if (m_flightstatus->outer_marker &&
        vs < -250.0 &&
        m_flightstatus->radarAltitude() < 2500 &&
        m_outermarker_inhibit_timer.elapsed() > 20000)
    {
        addSoundToQueue(CFG_SOUNDS_OUTERMARKER);
        m_outermarker_inhibit_timer.start();
    }

    //----- check 10000 ft

    if (m_flightstatus->radarAltitude() > 500 &&
        qAbs(vs) >= 250.0 &&
        m_10000ft_inhibit_timer.elapsed() > 10000 &&
        ((m_prev_alt_ft < 10000.0 && alt > 10000.0) || (m_prev_alt_ft > 10000.0 && alt < 10000.0)))
    {
        addSoundToQueue(CFG_SOUNDS_10000_FT);
        m_10000ft_inhibit_timer.start();
    }

    //----- check reversers

    if (m_flightstatus->isAtLeastOneEngineOn() && m_flightstatus->isReverserOn())
    {
        if (m_do_reverser_callout)
        {
            addSoundToQueue(CFG_SOUNDS_REVERSERS_OK);
            m_do_reverser_callout = false;
            m_last_reverser_callout_timer.start();
        }
    }
    else
    {
        if (!m_do_reverser_callout && m_last_reverser_callout_timer.elapsed() > 5000)
            m_do_reverser_callout = true;
    }

    //------ spoilers

    if (m_fmc_control->flightModeTracker().isLanding() && m_flightstatus->spoiler_lever_percent > 5.0)
    {
        if (m_do_gnd_spoiler_callout)
        {
            addSoundToQueue(CFG_SOUNDS_GND_SPOILERS);
            m_do_gnd_spoiler_callout = false;
            m_last_gnd_spoiler_callout_timer.start();
        }
    }
    else
    {
        if (!m_do_gnd_spoiler_callout && m_last_gnd_spoiler_callout_timer.elapsed() > 5000)
            m_do_gnd_spoiler_callout = true;
    }

    //------ check for MDA

    uint minimum = m_fmc_control->fmcData().minDescentAltitudeFt();
    if (minimum > 0)
    {
        if (alt > minimum)
        {
            m_was_above_mda = true;
        }
        else
        {
            if (m_was_above_mda)
            {
                m_was_above_mda = false;
                addSoundToQueue(CFG_SOUNDS_MINIMUMS);
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCSounds::slotPlayAltReachAlert()
{
    if (m_flightstatus->ap_enabled) return;
    addSoundToQueue(CFG_SOUNDS_ALT_ALERT);
}

// End of file
