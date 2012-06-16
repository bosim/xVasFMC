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

/*! \file    fmc_cdu_page_manager.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __FMC_CDU_PAGE_MANAGER_H__
#define __FMC_CDU_PAGE_MANAGER_H__

#include <QObject>
#include <QMap>
#include <QMapIterator>

#include "ptrlist.h"

#include "fmc_cdu_page.h"

class Config;
class FMCControl;
class FMCCDUPageStyleAMenu;

/////////////////////////////////////////////////////////////////////////////

//! FMC CDU page manager
class FMCCDUPageManager : public QObject
{
    Q_OBJECT

public:

    //! Standard Constructor
    FMCCDUPageManager(Config* cdu_config, FMCControl* fmc_control, bool left_side);

    //! Destructor
    virtual ~FMCCDUPageManager();

    inline const Config& cduConfig() const { return *m_cdu_config; }

    //! access to the FMC data
    inline FMCControl& fmcControl() { return *m_fmc_control; }

    //! whether we are the left or right side CDU
    inline bool isLeftSide() const { return m_left_side; }

    //! resets all pages
    virtual void resetAllPages();

    //! sets the current page to the given one. returns true on success.
    bool setCurrentPage(const QString& page_name);
    //! sets the interim page to the given one. returns true on success.
    bool setInterimPage(const QString& page_name);

    //! returns the interim page, or if the interim page is NULL, the current page
    inline FMCCDUPageBase* activePage()
    {
        if (m_interim_page != 0) return m_interim_page;
        return m_current_page;
    }
    //! will return the scratchpad
    inline FMCCDUPageStyleAScratchpad& scratchpad() { return m_scratchpad_page; }
    
    //! returns the page with the given name
    inline FMCCDUPageBase* page(const QString& page_name)
    {
        if (!m_page_map.contains(page_name)) return 0;
        return m_page_map[page_name];
    }                                              

    inline const QString& previousPageName() const { return m_previous_page_name; }

    //-----

    inline FMCCDUPageStyleAWaypoint* wptPage() { return m_wpt_page; }
    inline void setWptPage(FMCCDUPageStyleAWaypoint* page)
    {
        MYASSERT(m_wpt_page == 0);
        m_wpt_page = page;
        
        MYASSERT(!m_page_map.contains(page->name()));
        m_page_map.insert(m_wpt_page->name(), page);
    }

    inline FMCCDUPageStyleAHolding* holdingPage() { return m_holding_page; }
    inline void setHoldingPage(FMCCDUPageStyleAHolding* page)
    {
        MYASSERT(m_holding_page == 0);
        m_holding_page = page;
        
        MYASSERT(!m_page_map.contains(page->name()));
        m_page_map.insert(m_holding_page->name(), page);
    }

    inline FMCCDUPageStyleAWaypointSelect* wptSelectPage() { return m_wpt_select_page; }
    inline void setWptSelectPage(FMCCDUPageStyleAWaypointSelect* page)
    {
        MYASSERT(m_wpt_select_page == 0);
        m_wpt_select_page = page;
        
        MYASSERT(!m_page_map.contains(page->name()));
        m_page_map.insert(m_wpt_select_page->name(), page);
    }

    inline FMCCDUPageStyleADepartureSelect* departureSelectPage() { return m_departure_select_page; }
    inline void setDepartureSelectPage(FMCCDUPageStyleADepartureSelect* page)
    {
        MYASSERT(m_departure_select_page == 0);
        m_departure_select_page = page;
        
        MYASSERT(!m_page_map.contains(page->name()));
        m_page_map.insert(m_departure_select_page->name(), page);
    }

    inline FMCCDUPageStyleADestinationSelect* destinationSelectPage() { return m_destination_select_page; }
    inline void setDestinationSelectPage(FMCCDUPageStyleADestinationSelect* page)
    {
        MYASSERT(m_destination_select_page == 0);
        m_destination_select_page = page;
        
        MYASSERT(!m_page_map.contains(page->name()));
        m_page_map.insert(m_destination_select_page->name(), page);
    }

    inline FMCCDUPageStyleAFlightplan* flightPlanPage() { return m_flightplan_page; }
    inline void setFlightPlanPage(FMCCDUPageStyleAFlightplan* page)
    {
        MYASSERT(m_flightplan_page == 0);
        m_flightplan_page = page;
        
        MYASSERT(!m_page_map.contains(page->name()));
        m_page_map.insert(m_flightplan_page->name(), page);
    }

    inline FMCCDUPageStyleAFPLoad* flightPlanLoadPage() { return m_fpload_page; }
    inline void setFlightPlanLoadPage(FMCCDUPageStyleAFPLoad* page)
    {
        MYASSERT(m_fpload_page == 0);
        m_fpload_page = page;
        
        MYASSERT(!m_page_map.contains(page->name()));
        m_page_map.insert(m_fpload_page->name(), page);
    }

    inline FMCCDUPageStyleAMenu* menuPage() { return m_menu_page; }
    inline void setMenuPage(FMCCDUPageStyleAMenu* page);

protected:

    void addPage(FMCCDUPageBase* page);

protected:

    Config* m_cdu_config;
    FMCControl* m_fmc_control;
    FMCCDUPageBase* m_current_page;
    FMCCDUPageBase* m_interim_page;
    QString m_previous_page_name;

    FMCCDUPageStyleAScratchpad m_scratchpad_page;
    FMCCDUPageStyleAWaypointSelect* m_wpt_select_page;
    FMCCDUPageStyleADepartureSelect* m_departure_select_page;
    FMCCDUPageStyleADestinationSelect* m_destination_select_page;
    FMCCDUPageStyleAWaypoint* m_wpt_page;
    FMCCDUPageStyleAHolding* m_holding_page;
    FMCCDUPageStyleAFlightplan* m_flightplan_page;
    FMCCDUPageStyleAFPLoad* m_fpload_page;
    FMCCDUPageStyleAMenu* m_menu_page;

private:
    
    QMap<QString, FMCCDUPageBase*> m_page_map;
    bool m_left_side;

private:
    //! Hidden copy-constructor
    FMCCDUPageManager(const FMCCDUPageManager&);
    //! Hidden assignment operator
    const FMCCDUPageManager& operator = (const FMCCDUPageManager&);
};

/////////////////////////////////////////////////////////////////////////////

//! FMC CDU page manager style A
class FMCCDUPageManagerStyleA : public FMCCDUPageManager
{
    Q_OBJECT

public:

    static const QString PAGE_TEST;
    static const QString PAGE_FPLAN;
    static const QString PAGE_DIRECT;
    static const QString PAGE_INIT;
    static const QString PAGE_NAV;
    static const QString PAGE_AIRPORT;
    static const QString PAGE_MENU;
    static const QString PAGE_DATA;
    static const QString PAGE_FPLOAD;
    static const QString PAGE_FPSAVE;
    static const QString PAGE_PROG;
    static const QString PAGE_PERF;
    static const QString PAGE_FPRED;
    static const QString PAGE_SECFP;
    static const QString PAGE_ATC;

    //! Standard Constructor
    FMCCDUPageManagerStyleA(Config* cdu_config, FMCControl* fmc_control, bool left_side);

    //! Destructor
    virtual ~FMCCDUPageManagerStyleA();

private:
    //! Hidden copy-constructor
    FMCCDUPageManagerStyleA(const FMCCDUPageManagerStyleA&);
    //! Hidden assignment operator
    const FMCCDUPageManagerStyleA& operator = (const FMCCDUPageManagerStyleA&);
};

#endif /* __FMC_CDU_PAGE_MANAGER_H__ */

// End of file

