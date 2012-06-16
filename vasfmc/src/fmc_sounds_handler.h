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

/*! \file    fmc_sounds_handler.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __FMC_SOUNDS_HANDLER_H__
#define __FMC_SOUNDS_HANDLER_H__

#include <QTimer>
#include <QMutex>

#include "assert.h"
#include "config.h"
#include "logger.h"
#include "defines.h"

#include "fmc_sounds_style_a.h"
#include "fmc_sounds_style_b.h"

/////////////////////////////////////////////////////////////////////////////

//! FMC sounds handler (restarter)
class FMCSoundsHandler : public QObject
{
    Q_OBJECT

public:
    //! Standard Constructor
    FMCSoundsHandler(Config* main_config, FMCControl* fmc_control);    

    //! Destructor
    virtual ~FMCSoundsHandler()
    {
        delete m_sounds;
    }

    FMCSounds *fmcSounds()
    {
        // Protect m_sounds from being accessed during a restart (when it
        // might be invalid)
#if VASFMC_GAUGE
        QMutexLocker locker(&m_sounds_mutex);
#endif
        return m_sounds;
    }

    void restartSounds();

signals:

protected slots:

    void slotTriggerSoundsRestart() { QTimer::singleShot(1, this, SLOT(slotRestartSounds())); }

    void slotRestartSounds();

protected:

    FMCSounds* createSounds()
    {
        if (m_main_config->getIntValue(CFG_STYLE) == CFG_STYLE_A)
            return new FMCSoundStyleA(m_main_config, m_fmc_control);
        else
            return new FMCSoundStyleB(m_main_config, m_fmc_control);
    }

protected:

    Config* m_main_config;
    FMCControl* m_fmc_control;
    FMCSounds* m_sounds;
#if VASFMC_GAUGE
    QMutex m_sounds_mutex;
#endif

private:
    //! Hidden copy-constructor
    FMCSoundsHandler(const FMCSoundsHandler&);
    //! Hidden assignment operator
    const FMCSoundsHandler& operator = (const FMCSoundsHandler&);
};

#endif /* __FMC_SOUNDS_HANDLER_H__ */

// End of file

