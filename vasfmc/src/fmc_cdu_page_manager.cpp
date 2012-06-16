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

/*! \file    fmc_cdu_page_manager.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "config.h"
#include "assert.h"
#include "logger.h"

#include "fmc_control.h"
#include "fmc_cdu_page_menu.h"
#include "fmc_cdu_page_perf.h"
#include "fmc_cdu_page_atc.h"

#include "fmc_cdu_page_manager.h"

/////////////////////////////////////////////////////////////////////////////

FMCCDUPageManager::FMCCDUPageManager(Config* cdu_config, FMCControl* fmc_control, bool left_side) : 
    m_cdu_config(cdu_config), m_fmc_control(fmc_control), m_current_page(0), m_interim_page(0), 
    m_scratchpad_page("SRATCH", this), m_wpt_select_page(0), 
    m_departure_select_page(0), m_destination_select_page(0), m_wpt_page(0), 
    m_holding_page(0), m_flightplan_page(0), m_fpload_page(0), m_menu_page(0),
    m_left_side(left_side)
{
    MYASSERT(m_cdu_config != 0);
    MYASSERT(fmc_control != 0);
    MYASSERT(m_fmc_control != 0);
}

/////////////////////////////////////////////////////////////////////////////

FMCCDUPageManager::~FMCCDUPageManager()
{

}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageManager::resetAllPages()
{
    QMapIterator<QString, FMCCDUPageBase*> iter(m_page_map);
    while(iter.hasNext()) 
    {
        iter.next();
        iter.value()->reset();
    }

    m_scratchpad_page.reset();
}

/////////////////////////////////////////////////////////////////////////////

bool FMCCDUPageManager::setCurrentPage(const QString& page_name)
{
    if (!m_page_map.contains(page_name)) return false;

    if (m_current_page != 0 && !m_fmc_control->areAircraftDataConfirmed())
    {
        scratchpad().setOverrideText("CONFIRM ACFT DATA");
        return false;
    }

    if (!m_page_map[page_name]->allowSwitchToPage()) 
    {
        Logger::log(QString("FMCCDUPageManager:setCurrentPage: switch to %1 not allowed").arg(page_name));
        return false;
    }

    if (m_current_page != 0) 
    {
        m_current_page->setActive(false);
        m_previous_page_name = m_current_page->name();
    }
    else
    {
        m_previous_page_name = page_name;
    }

    if (m_interim_page != 0) m_interim_page->setActive(false);
    m_interim_page = 0;

    m_current_page = m_page_map[page_name];
    m_current_page->setActive(true);
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool FMCCDUPageManager::setInterimPage(const QString& page_name)
{
    if (page_name.isEmpty()) 
    {
        m_interim_page = 0;
        return true;
    }

    if (!m_page_map.contains(page_name)) return false;
    m_interim_page = m_page_map[page_name];
    return true;
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageManager::addPage(FMCCDUPageBase* page)
{
    MYASSERT(page != 0);
    MYASSERT(!page->name().isEmpty());
    MYASSERT(!m_page_map.contains(page->name()));
    
    m_page_map.insert(page->name(), page);
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageManager::setMenuPage(FMCCDUPageStyleAMenu* page)
{
    MYASSERT(m_menu_page == 0);
    m_menu_page = page;
    
    MYASSERT(!m_page_map.contains(page->name()));
    m_page_map.insert(m_menu_page->name(), page);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

const QString FMCCDUPageManagerStyleA::PAGE_TEST = "TEST";
const QString FMCCDUPageManagerStyleA::PAGE_FPLAN = "FPLAN";
const QString FMCCDUPageManagerStyleA::PAGE_DIRECT = "DIRECT";
const QString FMCCDUPageManagerStyleA::PAGE_INIT = "INIT";
const QString FMCCDUPageManagerStyleA::PAGE_NAV = "NAV";
const QString FMCCDUPageManagerStyleA::PAGE_AIRPORT = "AIRPORT";
const QString FMCCDUPageManagerStyleA::PAGE_MENU = "MENU";
const QString FMCCDUPageManagerStyleA::PAGE_DATA = "DATA";
const QString FMCCDUPageManagerStyleA::PAGE_FPLOAD = "FPLOAD";
const QString FMCCDUPageManagerStyleA::PAGE_FPSAVE = "FPSAVE";
const QString FMCCDUPageManagerStyleA::PAGE_PROG = "PROG";
const QString FMCCDUPageManagerStyleA::PAGE_PERF = "PERF";
const QString FMCCDUPageManagerStyleA::PAGE_FPRED = "FPRED";
const QString FMCCDUPageManagerStyleA::PAGE_SECFP = "SECFP";
const QString FMCCDUPageManagerStyleA::PAGE_ATC = "ATC";

/////////////////////////////////////////////////////////////////////////////

FMCCDUPageManagerStyleA::FMCCDUPageManagerStyleA(Config* cdu_config, FMCControl* fmc_control, bool left_side) : 
    FMCCDUPageManager(cdu_config, fmc_control, left_side)
{
    MYASSERT(fmc_control != 0);

    addPage(new FMCCDUPageStyleATest(PAGE_TEST, this));
    addPage(new FMCCDUPageStyleADirect(PAGE_DIRECT, this));
    addPage(new FMCCDUPageStyleAInit(PAGE_INIT, this));
    addPage(new FMCCDUPageStyleANavigation(PAGE_NAV, this));
    addPage(new FMCCDUPageStyleAAirport(PAGE_AIRPORT, this));
    addPage(new FMCCDUPageStyleAData(PAGE_DATA, this));
    addPage(new FMCCDUPageStyleAFPSave(PAGE_FPSAVE, this));
    addPage(new FMCCDUPageStyleAProgress(PAGE_PROG, this));
    addPage(new FMCCDUPageStyleAPerformance(PAGE_PERF, this));
    addPage(new FMCCDUPageStyleAFuelPred(PAGE_FPRED, this));
    addPage(new FMCCDUPageStyleASecFlightplan(PAGE_SECFP, this));
    addPage(new FMCCDUPageStyleAATC(PAGE_ATC, this));
    setWptPage(new FMCCDUPageStyleAWaypoint("WPT", this));
    setHoldingPage(new FMCCDUPageStyleAHolding("HOLD", this));
    setWptSelectPage(new FMCCDUPageStyleAWaypointSelect("WPTSEL", this));
    setDepartureSelectPage(new FMCCDUPageStyleADepartureSelect("DEPSEL", this));
    setDestinationSelectPage(new FMCCDUPageStyleADestinationSelect("DESTSEL", this));
    setFlightPlanPage(new FMCCDUPageStyleAFlightplan(PAGE_FPLAN, this));
    setFlightPlanLoadPage(new FMCCDUPageStyleAFPLoad(PAGE_FPLOAD, this));
    setMenuPage(new FMCCDUPageStyleAMenu(PAGE_MENU, this));
    setCurrentPage(PAGE_MENU);
}

/////////////////////////////////////////////////////////////////////////////

FMCCDUPageManagerStyleA::~FMCCDUPageManagerStyleA()
{

}

// End of file
