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

/*! \file    fmc_cdu_page.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __FMC_CDU_PAGE_H__
#define __FMC_CDU_PAGE_H__

#include <QTimer>

#include "waypoint.h"
#include "sid.h"
#include "star.h"
#include "transition.h"
#include "approach.h"

#include "fmc_cdu_page_base.h"

/////////////////////////////////////////////////////////////////////////////

//! Test page
class FMCCDUPageStyleATest : public FMCCDUPageBase
{
    Q_OBJECT

public:
    //! Standard Constructor
    FMCCDUPageStyleATest(const QString& page_name, FMCCDUPageManager* page_manager) : 
        FMCCDUPageBase(page_name, page_manager) {};

    //! Destructor
    virtual ~FMCCDUPageStyleATest() {};

    virtual void paintPage(QPainter& painter) const;

    virtual void processAction(const QString&) {};
};

/////////////////////////////////////////////////////////////////////////////

//! scratchpad page
class FMCCDUPageStyleAScratchpad : public FMCCDUPageBase
{
    Q_OBJECT

public:
    //! Standard Constructor
    FMCCDUPageStyleAScratchpad(const QString& page_name, FMCCDUPageManager* page_manager) : 
        FMCCDUPageBase(page_name, page_manager) {};

    //! Destructor
    virtual ~FMCCDUPageStyleAScratchpad() {};

    virtual void paintPage(QPainter& painter) const;
    virtual void processAction(const QString& action);

    inline const QString text() const 
    {
        if (!m_override_text.isEmpty()) return QString::null;
        return m_text; 
    }
    inline void setText(const QString& text) { m_text = text; }

    inline const QString& overrideText() const { return m_override_text; }
    inline void setOverrideText(const QString& text) { m_override_text = text.toUpper(); }

    inline bool hasAction() const { return !m_action.isEmpty(); }
    inline const QString& action() const 
    {
        if (!m_action.isEmpty()) MYASSERT(m_text.isEmpty());
        return m_action; 
    }
    inline void setAction(const QString& action) { m_action = action; }

protected:

    QString m_text;    
    QString m_override_text;
    QString m_action;
};

/////////////////////////////////////////////////////////////////////////////

//! callback page base
class FMCCDUPageStyleACallbackBase : public FMCCDUPageBase
{
    Q_OBJECT

    //TODO replace m_selected_index by m_selected_wpt pointing to an entry of the wpt_list

public:

    //! Standard Constructor
    FMCCDUPageStyleACallbackBase(const QString& page_name, FMCCDUPageManager* page_manager);
    
    //! Destructor
    virtual ~FMCCDUPageStyleACallbackBase() {};

    virtual void setActive(bool active)
    {
        if (!active)
        {
            m_callback_page = 0;
            m_callback_action = QString::null;
            m_selected_index = -1;
        }

        FMCCDUPageBase::setActive(active);
    }

    virtual void setForSelection(FMCCDUPageBase* callback_page, const QString& callback_action)
    {
        MYASSERT(callback_page != 0);
        MYASSERT(!callback_action.isEmpty());
        m_callback_page = callback_page;
        m_callback_action = callback_action;
        m_selected_index = -1;
        m_vertical_scroll_offset = 0;
        m_horizontal_scroll_offset = 0;
    }

    virtual int selectedIndex() const { return m_selected_index; }

protected:

    FMCCDUPageBase* m_callback_page;
    QString m_callback_action;
    int m_selected_index;
};

/////////////////////////////////////////////////////////////////////////////

//! DuplicateWaypoints page
class FMCCDUPageStyleAWaypointSelect : public FMCCDUPageStyleACallbackBase
{
    Q_OBJECT

public:

    //! Standard Constructor
    FMCCDUPageStyleAWaypointSelect(const QString& page_name, FMCCDUPageManager* page_manager);

    //! Destructor
    virtual ~FMCCDUPageStyleAWaypointSelect() {};

    virtual void setActive(bool active)
    {
        if (!active) m_wpt_list = 0;
        FMCCDUPageStyleACallbackBase::setActive(active);
    }

    virtual void setForSelection(const WaypointPtrList* wpt_list,
                                 FMCCDUPageBase* callback_page,
                                 const QString& callback_action);

    virtual void paintPage(QPainter& painter) const;
    virtual void processAction(const QString& action);

protected:

    virtual uint maxVerticalScrollOffset() const;
    
protected:

    const WaypointPtrList* m_wpt_list;
};

/////////////////////////////////////////////////////////////////////////////

//! DuplicateDepartures page
class FMCCDUPageStyleADepartureSelect : public FMCCDUPageBase
{
    Q_OBJECT

public:

    //! Standard Constructor
    FMCCDUPageStyleADepartureSelect(const QString& page_name, FMCCDUPageManager* page_manager);
    
    //! Destructor
    virtual ~FMCCDUPageStyleADepartureSelect() {};
    
    virtual void setActive(bool active);

    virtual void setForSelection(uint wpt_insert_index,
                                 const Airport* departure_airport,
                                 const QString& next_page);
    
    virtual void paintPage(QPainter& painter) const;
    virtual void processAction(const QString& action);

protected:

    virtual uint maxVerticalScrollOffset() const;
    void filterSids();

protected:

    uint m_wpt_insert_index;
    QString m_next_page;

    const Airport* m_departure_airport;
    ProcedurePtrList m_sid_list;
    PtrList<Sid> m_filtered_sids;

    const Sid *m_chosen_sid;
    QString m_chosen_dep_runway;
    const Transition *m_chosen_sid_transition;
};

/////////////////////////////////////////////////////////////////////////////

//! DuplicateDestinations page
class FMCCDUPageStyleADestinationSelect : public FMCCDUPageBase
{
    Q_OBJECT

public:

    //! Standard Constructor
    FMCCDUPageStyleADestinationSelect(const QString& page_name, FMCCDUPageManager* page_manager);
    
    //! Destructor
    virtual ~FMCCDUPageStyleADestinationSelect() {};
    
    virtual void setActive(bool active);

    virtual void setForSelection(uint wpt_insert_index,
                                 const Airport* destination_airport,
                                 const QString& next_page);

    virtual void paintPage(QPainter& painter) const;
    virtual void processAction(const QString& action);

protected:

    virtual uint maxVerticalScrollOffset() const;
    void filterStarsAndApproaches();

protected:

    uint m_wpt_insert_index;
    QString m_next_page;

    const Airport* m_destination_airport;
    ProcedurePtrList m_star_list;
    ProcedurePtrList m_approach_list;

    PtrList<Star> m_filtered_stars;
    PtrList<Approach> m_filtered_approaches;

    const Approach* m_chosen_approach;
    const Star *m_chosen_star;
    const Transition *m_chosen_app_transition;
};

/////////////////////////////////////////////////////////////////////////////

//! waypoint selection base class
class FMCCDUPageStyleAWaypointSelectBase : public FMCCDUPageBase
{
    Q_OBJECT
    
public:

    static const QString ACTION_CALLBACK_WPT_SELECT;

    //! Standard Constructor
    FMCCDUPageStyleAWaypointSelectBase(const QString& page_name, FMCCDUPageManager* page_manager) : 
        FMCCDUPageBase(page_name, page_manager), m_wpt_select_page(0), m_wpt_selection_list(0),
        m_wpt_insert_index(0), m_pbd_bearing(0.0), m_pbd_distance_nm(0.0) {};

    //! Destructor
    virtual ~FMCCDUPageStyleAWaypointSelectBase() {};

protected:

    virtual void setWptSelectPage(const WaypointPtrList& wpt_selection_list, uint wpt_insert_index,
                                  double pbd_bearing, double pbd_distance_nm);
    virtual void processWptSelectCallback();

    // called when a waypoint is selected
    virtual void wptSelectCallback(const Waypoint& wpt, uint wpt_insert_index) = 0;

private:
    
    FMCCDUPageStyleAWaypointSelect* m_wpt_select_page;
    WaypointPtrList* m_wpt_selection_list;
    int m_wpt_insert_index;
    double m_pbd_bearing;
    double m_pbd_distance_nm;
};

/////////////////////////////////////////////////////////////////////////////

//! Waypoint page
class FMCCDUPageStyleAWaypoint : public FMCCDUPageStyleAWaypointSelectBase
{
    Q_OBJECT

public:
    //! Standard Constructor
    FMCCDUPageStyleAWaypoint(const QString& page_name, FMCCDUPageManager* page_manager);

    //! Destructor
    virtual ~FMCCDUPageStyleAWaypoint() {}

    virtual void paintPage(QPainter& painter) const;

    virtual void processAction(const QString& action);

    void setWaypoint(uint wpt_route_index);

protected:

    virtual void wptSelectCallback(const Waypoint& wpt, uint wpt_insert_index);
    virtual uint maxVerticalScrollOffset() const;
    virtual void setActive(bool active);

protected:

    const Waypoint* m_wpt;
    uint m_wpt_route_index;

    bool m_is_departure_airport;
    bool m_is_destination_airport;
};

/////////////////////////////////////////////////////////////////////////////

//! Holding page
class FMCCDUPageStyleAHolding : public FMCCDUPageBase
{
    Q_OBJECT

public:
    //! Standard Constructor
    FMCCDUPageStyleAHolding(const QString& page_name, FMCCDUPageManager* page_manager);

    //! Destructor
    virtual ~FMCCDUPageStyleAHolding() {}

    virtual void paintPage(QPainter& painter) const;
    virtual void processAction(const QString& action);

    void setWaypoint(uint wpt_route_index);

protected:

    const Holding& wptHolding() const;

protected:

    uint m_wpt_route_index;
    QString m_wpt_id;
    Holding m_holding;
    bool m_changes_made;
};

/////////////////////////////////////////////////////////////////////////////

//! Flightplan page
class FMCCDUPageStyleAFlightplan : public FMCCDUPageStyleAWaypointSelectBase
{
    Q_OBJECT

public:

    static const QString ACTION_CALLBACK_DEPARTURE_SELECT;

    FMCCDUPageStyleAFlightplan(const QString& page_name, FMCCDUPageManager* page_manager) :
        FMCCDUPageStyleAWaypointSelectBase(page_name, page_manager), 
        m_display_line_counter(0), m_preset_scroll_offset(0) {}
    virtual ~FMCCDUPageStyleAFlightplan() {}

    virtual void paintPage(QPainter& painter) const;
    virtual void processAction(const QString& action);
    virtual void presetScrollOffset(int preset);
    virtual void setActive(bool active);

protected:

    virtual uint maxVerticalScrollOffset() const;
    virtual uint getWaypointIndexByLSK(uint lsk_index) const;    
    virtual void wptSelectCallback(const Waypoint& wpt, uint wpt_insert_index);

protected:

    mutable uint m_display_line_counter;
    int m_preset_scroll_offset;
};

/////////////////////////////////////////////////////////////////////////////

//! Direct page
class FMCCDUPageStyleADirect : public FMCCDUPageStyleAFlightplan
{
    Q_OBJECT

public:
    //! Standard Constructor
    FMCCDUPageStyleADirect(const QString& page_name, FMCCDUPageManager* page_manager) : 
        FMCCDUPageStyleAFlightplan(page_name, page_manager), 
        m_selected_wpt(0), m_wpt_insert_index(-1), m_wpt_is_in_route(false)
    {};

    //! Destructor
    virtual ~FMCCDUPageStyleADirect() {};

    virtual void paintPage(QPainter& painter) const;
    virtual void processAction(const QString& action);

    virtual void setActive(bool active);

protected:

    virtual uint maxVerticalScrollOffset() const;
    virtual uint getWaypointIndexByLSK(uint lsk_index) const;
    virtual void wptSelectCallback(const Waypoint& wpt, uint wpt_insert_index);

    virtual void selectDirect(const Waypoint& dct_wpt, int wpt_insert_index);
    virtual void activateDirect(const Waypoint& dct_wpt, int wpt_insert_index);

protected:

    Waypoint* m_selected_wpt;
    int m_wpt_insert_index;
    bool m_wpt_is_in_route;
};

/////////////////////////////////////////////////////////////////////////////

//! Airport page
class FMCCDUPageStyleAAirport : public FMCCDUPageBase
{
    Q_OBJECT

public:

    //! Standard Constructor
    FMCCDUPageStyleAAirport(const QString& page_name, FMCCDUPageManager* page_manager) :
        FMCCDUPageBase(page_name, page_manager) {};

    //! Destructor
    virtual ~FMCCDUPageStyleAAirport() {}
    
    virtual void paintPage(QPainter&) const {};
    virtual void processAction(const QString&) {};

    virtual void setActive(bool active);

protected:
    
};

/////////////////////////////////////////////////////////////////////////////

//! Init page
class FMCCDUPageStyleAInit : public FMCCDUPageBase
{
    Q_OBJECT

public:
    //! Standard Constructor
    FMCCDUPageStyleAInit(const QString& page_name, FMCCDUPageManager* page_manager);

    //! Destructor
    virtual ~FMCCDUPageStyleAInit() {}

    virtual void paintPage(QPainter& painter) const;
    virtual void processAction(const QString& action);

    virtual bool allowSwitchToPage() const;

protected slots:

    void slotCheckPageSwitch();

protected:

    virtual uint maxHorizontalScrollOffset() const;

    virtual void setActive(bool active);

protected:

    QTimer m_page_switch_check_timer;

};

/////////////////////////////////////////////////////////////////////////////

//! Navigation page
class FMCCDUPageStyleANavigation : public FMCCDUPageBase
{
    Q_OBJECT

public:
    //! Standard Constructor
    FMCCDUPageStyleANavigation(const QString& page_name, FMCCDUPageManager* page_manager) : 
        FMCCDUPageBase(page_name, page_manager) {};

    //! Destructor
    virtual ~FMCCDUPageStyleANavigation() {}

    virtual void paintPage(QPainter& painter) const;
    virtual void processAction(const QString& action);

protected:

    int getVorFreq(const QString& text) const;
    int getAdfFreq(const QString& text) const;
    int getHeading(const QString& text) const;
};

/////////////////////////////////////////////////////////////////////////////

//! Data page
class FMCCDUPageStyleAData  : public FMCCDUPageStyleAWaypointSelectBase
{
    Q_OBJECT

public:

    static const QString MODE_WAYPOINT;
    static const QString MODE_NAVAIDS;
    static const QString MODE_RUNWAYS;

    //! Standard Constructor
    FMCCDUPageStyleAData(const QString& page_name, FMCCDUPageManager* page_manager) :
        FMCCDUPageStyleAWaypointSelectBase(page_name, page_manager), m_user_wpt(0) {};

    //! Destructor
    virtual ~FMCCDUPageStyleAData() 
    {
        delete m_user_wpt;
    }

    virtual void paintPage(QPainter& painter) const;

    virtual void processAction(const QString& action);

protected:

    virtual void setActive(bool active);
    virtual void wptSelectCallback(const Waypoint& wpt, uint wpt_insert_index);

protected:

    QString m_mode;
    Waypoint* m_user_wpt;
    QString m_rwy_id;
};

/////////////////////////////////////////////////////////////////////////////

//! FPLoad page
class FMCCDUPageStyleAFPLoad : public FMCCDUPageBase
{
    Q_OBJECT

public:

    FMCCDUPageStyleAFPLoad(const QString& page_name, FMCCDUPageManager* page_manager) :
        FMCCDUPageBase(page_name, page_manager), m_selected_fp_index(-1), 
        m_fp_selection_count(0), m_fp_delete_flag(false)
    {};

    virtual ~FMCCDUPageStyleAFPLoad() {};

    virtual void paintPage(QPainter& painter) const;
    virtual void processAction(const QString& action);

    //! sets up the FP load page with the given company FP, returns true on success.
    virtual bool setupByCompanyRoute(const QString& company_route);    

protected:
    
    void clear();

protected:
    
    virtual void setupFPList();
    virtual void setActive(bool active);
    virtual uint maxVerticalScrollOffset() const;
    int getIndexByLSK(uint lsk_index) const;

    virtual uint maxHorizontalScrollOffset() const { return m_fp_selection_count; }
    bool deleteFP(int index);
    bool selectFP(int index);

protected:

    QString m_fp_path;
    QStringList m_fp_filename_list;

    int m_selected_fp_index;
    uint m_fp_selection_count;
    QString m_adep;
    QString m_ades;

    bool m_fp_delete_flag;
};

/////////////////////////////////////////////////////////////////////////////

//! FPSave page
class FMCCDUPageStyleAFPSave : public FMCCDUPageBase
{
    Q_OBJECT

public:
    //! Standard Constructor
    FMCCDUPageStyleAFPSave(const QString& page_name, FMCCDUPageManager* page_manager) :
        FMCCDUPageBase(page_name, page_manager) {};

    //! Destructor
    virtual ~FMCCDUPageStyleAFPSave() {};

    virtual void paintPage(QPainter& painter) const;

    virtual void processAction(const QString& action);

protected:
    
    virtual void setActive(bool active);

    void updateFPList();

protected:

    QString m_fp_path;
    QStringList m_fp_filename_list;
};

/////////////////////////////////////////////////////////////////////////////

//! Progress page
class FMCCDUPageStyleAProgress : public FMCCDUPageStyleAWaypointSelectBase
{
    Q_OBJECT

public:
    //! Standard Constructor
    FMCCDUPageStyleAProgress(const QString& page_name, FMCCDUPageManager* page_manager) :
        FMCCDUPageStyleAWaypointSelectBase(page_name, page_manager), m_wpt(0) {};

    //! Destructor
    virtual ~FMCCDUPageStyleAProgress() {};

    virtual void paintPage(QPainter& painter) const;

    virtual void processAction(const QString& action);

protected:
    
    virtual void wptSelectCallback(const Waypoint& wpt, uint wpt_insert_index);

protected:

    Waypoint* m_wpt;
};

/////////////////////////////////////////////////////////////////////////////

//! FuelPred page
class FMCCDUPageStyleAFuelPred : public FMCCDUPageBase
{
    Q_OBJECT

public:
    //! Standard Constructor
    FMCCDUPageStyleAFuelPred(const QString& page_name, FMCCDUPageManager* page_manager) :
        FMCCDUPageBase(page_name, page_manager) {};

    //! Destructor
    virtual ~FMCCDUPageStyleAFuelPred() {};

    virtual void paintPage(QPainter& painter) const;

    virtual void processAction(const QString& action);
};

/////////////////////////////////////////////////////////////////////////////

//! SecFlightplan page
class FMCCDUPageStyleASecFlightplan : public FMCCDUPageBase
{
    Q_OBJECT

public:
    //! Standard Constructor
    FMCCDUPageStyleASecFlightplan(const QString& page_name, FMCCDUPageManager* page_manager) :
        FMCCDUPageBase(page_name, page_manager) {};

    //! Destructor
    virtual ~FMCCDUPageStyleASecFlightplan() {};

    virtual void paintPage(QPainter& painter) const;

    virtual void processAction(const QString& action);
};

#endif /* __FMC_CDU_PAGE_H__ */

// End of file
