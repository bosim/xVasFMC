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

/*! \file    fmc_sounds_style_a.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __FMC_SOUNDS_STYLE_A_H__
#define __FMC_SOUNDS_STYLE_A_H__

#include <QTime>

#include "fmc_autopilot.h"
#include "fmc_autothrottle.h"

#include "fmc_sounds.h"

class QSound;

/////////////////////////////////////////////////////////////////////////////

//! A style sounds
class FMCSoundStyleA : public FMCSounds
{
    Q_OBJECT

public:
    //! Standard Constructor
    FMCSoundStyleA(Config* main_config, FMCControl* fmc_control);

    //! Destructor
    virtual ~FMCSoundStyleA();

protected:

    virtual void setupDefaultConfig();
    virtual void checkSounds();

protected:

    QTime m_radar_height_inhibit_timer;
    int m_radar_height_inhibit_time_ms;
    FMCAutopilot::ILS_MODE m_last_ils_mode;
    QTime m_ils_mode_inhibit_timer;

    FMCAutothrottle::AIRBUS_THROTTLE_MODE m_last_airbus_throttle_mode;
    bool m_thr_lvr_clb_request_active;
    QTime m_thr_lvr_clb_request_timer;
    QTime m_airbus_thrust_change_timer;
    QString m_airbus_thrust_next_callout;

private:
    //! Hidden copy-constructor
    FMCSoundStyleA(const FMCSoundStyleA&);
    //! Hidden assignment operator
    const FMCSoundStyleA& operator = (const FMCSoundStyleA&);
};

#endif /* __FMC_SOUNDS_STYLE_A_H__ */

// End of file

