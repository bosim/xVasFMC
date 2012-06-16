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

/*! \file    fmc_cdu_page_menu.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __FMC_CDU_PAGE_MENU_H__
#define __FMC_CDU_PAGE_MENU_H__

#include <QTimer>
#include <QTime>

#include "weather.h"
#include "vroute.h"

#include "fmc_cdu_page_base.h"

class FlightRoute;

/////////////////////////////////////////////////////////////////////////////

//! Menu page
class FMCCDUPageStyleAMenu : public FMCCDUPageBase
{
    Q_OBJECT

public:

    static const QString SYSTEM_FMCS;
    static const QString SYSTEM_ACARS;
    static const QString SYSTEM_WEATHER;
    static const QString SYSTEM_VROUTE;
    static const QString SYSTEM_ICAORTE;
    static const QString SYSTEM_OOOI;
    static const QString SYSTEM_SBOX;
    static const QString SYSTEM_SETTING;
    static const QString SYSTEM_FBW_BANK;
    static const QString SYSTEM_FBW_PITCH;
    static const QString SYSTEM_PUSHBACK;
    static const QString SYSTEM_CHECKLIST;
    static const QString SYSTEM_INTERFACES;
    static const QString SYSTEM_DISPLAY1;
    static const QString SYSTEM_DISPLAY2;

    FMCCDUPageStyleAMenu(const QString& page_name, FMCCDUPageManager* page_manager);
    virtual ~FMCCDUPageStyleAMenu();

    virtual void paintPage(QPainter& painter) const;
    virtual void processAction(const QString& action);

protected slots:

    void slotGotWeather(const QString& airport, const QString& date_string, const QString& weather_string);
    void slotGotWeatherError(const QString& error_string);

    void slotGotRoute(const CompactRouteList& compact_route_list);
    void slotGotRouteError(const QString& error_string);

    void slotSetVerticalScrollOffsetForChecklist();

protected:

    virtual void setActive(bool active);
    virtual uint maxVerticalScrollOffset() const;
    virtual uint maxHorizontalScrollOffset() const;
    
    void selectCompactRoute(uint index);

    void clearICAORoute();

protected:

    QString m_selected_system;

    Weather m_weather;
    QString m_weather_airport;
    QString m_weather_date;
    QStringList m_weather_text_list;

    VRoute m_vroute;

    QString m_fp_adep;
    QString m_fp_ades;
    CompactRouteList m_compact_route_list;
    uint m_selected_compact_route_index;
    QStringList m_route_text_list;

    FlightRoute *m_icao_route;

    uint m_pushback_dist_before_turn_m;
    bool m_pushback_turn_direction_clockwise;
    uint m_pushback_turn_degrees;
    uint m_pushback_dist_after_turn_m;

    QTime m_aircraft_data_blink_timer;

    bool m_aircraft_data_load_page;
    QStringList m_aircraft_data_file_list;

    QTimer m_checklist_view_timer;
};

#endif /* __FMC_CDU_PAGE_MENU_H__ */

// End of file

