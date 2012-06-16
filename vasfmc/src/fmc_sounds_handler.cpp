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

/*! \file    fmc_sounds_handler.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "fmc_control.h"

#include "fmc_sounds_handler.h"

/////////////////////////////////////////////////////////////////////////////

FMCSoundsHandler::FMCSoundsHandler(Config* main_config, FMCControl* fmc_control) :
    m_main_config(main_config),
    m_fmc_control(fmc_control), 
    m_sounds(0)
{
    MYASSERT(m_fmc_control != 0);
    restartSounds();
    m_fmc_control->setSoundHandler(this);
}

/////////////////////////////////////////////////////////////////////////////

void FMCSoundsHandler::restartSounds()
{
#if VASFMC_GAUGE
    QMutexLocker locker(&m_sounds_mutex);
#endif
    delete m_sounds;
    m_sounds = createSounds();
    MYASSERT(m_sounds != 0);
}

/////////////////////////////////////////////////////////////////////////////

void FMCSoundsHandler::slotRestartSounds()
{
#if VASFMC_GAUGE
    QMutexLocker locker(&m_sounds_mutex);
#endif
    delete m_sounds;
    m_sounds = createSounds();
    MYASSERT(m_sounds != 0);
}

// End of file
