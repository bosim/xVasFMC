///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2007 Alexander Wemmer 
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

/*! \file    fmc_cdu_page_perf.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __FMC_CDU_PAGE_PERF_H__
#define __FMC_CDU_PAGE_PERF_H__

#include <QTimer>

#include "flight_mode_tracker.h"

#include "fmc_cdu_page_base.h"

/////////////////////////////////////////////////////////////////////////////

//! Perf page
class FMCCDUPageStyleAPerformance : public FMCCDUPageBase
{
    Q_OBJECT

public:

    enum PERF_PAGE { PERF_PAGE_TAKE_OFF = 0,
                     PERF_PAGE_CLIMB,
                     PERF_PAGE_CRUISE,
                     PERF_PAGE_DESCENT,
                     PERF_PAGE_APPROACH,
                     PERF_PAGE_GOAROUND
    };

    FMCCDUPageStyleAPerformance(const QString& page_name, FMCCDUPageManager* page_manager);
    virtual ~FMCCDUPageStyleAPerformance();

    virtual void paintPage(QPainter& painter) const;
    virtual void processAction(const QString& action);

protected slots:

    void slotCheckAndSetPerfPage(bool force = false);

protected:

    virtual uint maxHorizontalScrollOffset() const;

    virtual void setActive(bool active);

    //! returns 0.0 when invalid, the parsed mach number otherwise.
    virtual double parseMach(const QString& text);

    void drawPrevPhaseText(QPainter& painter) const;
    //! returns true if the APP phase action was handled, false otherwise
    bool handleAppPhaseAction();

    bool changesOnActivePerfPageAllowed() const;

protected:

    QTimer m_check_timer;

    PERF_PAGE m_current_perf_page;
    bool m_approach_phase_trigger;

    FlightModeTracker::FLIGHTMODE m_last_flight_mode;
};

#endif /* __FMC_CDU_PAGE_PERF_H__ */

// End of file

