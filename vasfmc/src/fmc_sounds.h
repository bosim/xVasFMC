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

/*! \file    fmc_sounds.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __FMC_SOUNDS_H__
#define __FMC_SOUNDS_H__

#ifdef USE_QSOUND
#include <QSound>
#endif

#ifdef USE_FMOD
#include "fmod.h"
#include "fmod_errors.h"
#endif

#ifdef USE_OPENAL
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
#endif

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QTime>

#include "fmc_sounds_defines.h"
#include "assert.h"

class Config;
class FMCControl;
class FlightStatus;

/////////////////////////////////////////////////////////////////////////////

//! mysound base class
class FMCSoundBase 
{
public:

    enum SOUND_SOURCE { SOUND_SOURCE_UNKNOWN,
                        SOUND_SOURCE_COMPUTER,
                        SOUND_SOURCE_MECHANICAL,
                        SOUND_SOURCE_OTHER_PILOT
    };

    FMCSoundBase(SOUND_SOURCE sound_source) : 
        m_has_played(false), m_sound_source(sound_source), m_delete_sound_after_played(false)
    {}

    virtual ~FMCSoundBase() {}

    virtual void play() = 0;
    virtual void stop() = 0;
    virtual void setPlayLooped(bool yes) = 0;
    virtual bool hasPlayed() const { return m_has_played; }
    virtual bool isFinished() const = 0;

    void setSoundSource(SOUND_SOURCE sound_source) { m_sound_source = sound_source; }
    SOUND_SOURCE soundSource() const { return m_sound_source; }

    bool doDeleteSoundAfterPlayed() const { return m_delete_sound_after_played; }
    void setDeleteSoundAfterPlayed(bool yes) { m_delete_sound_after_played = yes; }

protected:

    bool m_has_played;
    SOUND_SOURCE m_sound_source;
    bool m_delete_sound_after_played;
};

/////////////////////////////////////////////////////////////////////////////

#ifdef USE_QSOUND

class FMCSound : public FMCSoundBase, public QSound
{
public:

    FMCSound(const QString & filename, SOUND_SOURCE sound_source);
    virtual ~FMCSound();

    void play();
    void stop();
    void setPlayLooped(bool yes) { m_play_looped = yes; }
    bool isFinished() const;

protected:

    bool m_play_looped;
    QTime m_play_dt;
};

#endif

/////////////////////////////////////////////////////////////////////////////

#ifdef USE_FMOD

class FMCSound : public FMCSoundBase
{
public:

    FMCSound(const QString & filename, SOUND_SOURCE sound_source, FMOD_SYSTEM *fmod_system);
    virtual ~FMCSound();

    void play();
    void stop();
    void setPlayLooped(bool yes);
    bool isFinished() const;

protected:

    QString m_filename;
    FMOD_SYSTEM *m_fmod_system;
    FMOD_SOUND  *m_fmod_sound;
    FMOD_CHANNEL *m_fmod_channel;
};

#endif

/////////////////////////////////////////////////////////////////////////////

#ifdef USE_OPENAL

class AlContext{
public:
    static ALCcontext* getContext() {
        if (m_alContext == 0) {
            m_alDevice = alcOpenDevice(0);
            m_alContext = alcCreateContext(m_alDevice,0);
            m_oldContext = alcGetCurrentContext();
            alcMakeContextCurrent(m_alContext);
            alGetError();
            alutInitWithoutContext(NULL,0);
            MYASSERT(alutGetError()==ALUT_ERROR_NO_ERROR);
        }
        return m_alContext;
    }
    static void destroyContext() {
        alcMakeContextCurrent(m_oldContext);
        alcDestroyContext(m_alContext);
        m_alContext = 0;
        alcCloseDevice(m_alDevice);
        m_alDevice = 0;
        alutExit();
    }
private:
    static ALCcontext* m_alContext;
    static ALCcontext* m_oldContext;
    static ALCdevice* m_alDevice;
    AlContext();
    AlContext(const AlContext&);
    AlContext& operator=(const AlContext&);
};

class FMCSound : public FMCSoundBase
{
public:
    
    FMCSound(const QString & filename, SOUND_SOURCE sound_source);
    virtual ~FMCSound();
    
    void play();
    void stop();
    void setPlayLooped(bool yes);
    bool isFinished() const;

protected:
    
    QString m_filename;
    ALuint m_buffer;
    ALuint m_source;
};
#endif

/////////////////////////////////////////////////////////////////////////////

//! fmc sounds base class
class FMCSounds : public QObject
{
    Q_OBJECT

public:
    //! Standard Constructor
    FMCSounds(Config* main_config, const QString& sounds_cfg_filename, FMCControl* fmc_control);

    //! Destructor
    virtual ~FMCSounds();

    //! loads the given sound from the given sound file and sets the sound's source
    void loadSound(const QString& sound_name, 
                   const QString& sound_filename,
                   FMCSoundBase::SOUND_SOURCE sound_source);

    //! add the given sound the the play queue.
    //! if force is true, the sound is also added if there is not flightsim connection
    void addSoundToQueue(const QString& sound_name, bool force = false);

    //! generates an internal sound object from the given sound file,
    //! add the sound to the play queue and removes the object after playing.
    //! if force is true, the sound is also added if there is not flightsim connection
    void addSoundToQueueDirectly(const QString& sound_filename,
                                 FMCSoundBase::SOUND_SOURCE sound_source,
                                 bool force = false);

public slots:

    void slotCheckSoundsTimer();
    void slotPlayAltReachAlert();
    void slotPlay1000FtToGo();

protected:

    void setupDefaultBaseConfig();
    void checkBaseSounds();
    void loadBaseSounds();

    virtual void setupDefaultConfig() = 0;
    virtual void checkSounds() = 0;

    FMCSound* getSound(const QString& name)
    {
        if (!m_sound_map.contains(name)) return 0;
        return m_sound_map[name];
    }

protected:

    Config* m_main_config;
    Config* m_sounds_cfg;
    FMCControl* m_fmc_control;
    const FlightStatus* m_flightstatus;

    QTime m_refresh_timer;

    QTime m_startup_timer;

    //! map of sound files to play indexed by the sound source.
    //! sounds of different sources may be player simultaneously.
    QMap<FMCSoundBase::SOUND_SOURCE, QStringList> m_sound_queue_map;

    //-----

    double m_last_radar_alt;
    bool m_is_ap_connected;
    bool m_is_at_connected;
    bool m_had_received_ils;

    bool m_do_80kts_callout;
    bool m_do_v1_callout;
    bool m_do_vr_callout;
    bool m_do_v2_callout;
    bool m_do_positive_rate_callout;
    bool m_do_gear_up_callout;
    bool m_do_gear_down_callout;
    QTime m_loc_alive_callout_timer;
    bool m_do_loc_alive_callout;
    QTime m_gs_alive_callout_timer;
    bool m_do_gs_alive_callout;
    bool m_do_reaching_tod_callout;
    bool m_do_reverser_callout;
    bool m_do_gnd_spoiler_callout;

    QTime m_outermarker_inhibit_timer;

    uint m_prev_flaps_notch;
    QTime m_flaps_callout_delay_timer;
    QString m_flaps_next_callout;

    double m_prev_alt_ft;
    QTime m_10000ft_inhibit_timer;

    bool m_was_above_mda;

    QTime m_last_reverser_callout_timer;
    QTime m_last_gnd_spoiler_callout_timer;
    
    uint m_direct_play_sound_id;

#ifdef USE_FMOD
    FMOD_SYSTEM      *m_fmod_system;
#endif

#ifdef USE_OPENAL
    //ALCdevice* m_alDevice;
    ALCcontext* m_alContext;
    //ALCcontext* m_oldContext;
#endif

private:

    QMap<QString, FMCSound*> m_sound_map;

private:
    //! Hidden copy-constructor
    FMCSounds(const FMCSounds&);
    //! Hidden assignment operator
    const FMCSounds& operator = (const FMCSounds&);
};

#endif /* __FMC_SOUNDS_H__ */

// End of file

