//////////////////////////////////////////////////////////////////////////////
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

/*! \file    fmc_cdu_page_menu.cpp
  \author  Alexander Wemmer, alex@wemmer.at
*/

#include <QHostAddress>

#include "assert.h"
#include "aircraft_data.h"
#include "fly_by_wire.h"
#include "flight_mode_tracker.h"
#include "checklist.h"

#include "fmc_data.h"
#include "fmc_control.h"
#include "fmc_autothrottle.h"

#include "fmc_cdu_defines.h"
#include "fmc_cdu_page_manager.h"
#include "fmc_cdu_page_menu.h"

const QString FMCCDUPageStyleAMenu::SYSTEM_FMCS = "FMGC";
const QString FMCCDUPageStyleAMenu::SYSTEM_ACARS = "ACARS";
const QString FMCCDUPageStyleAMenu::SYSTEM_WEATHER = "WEATHER";
const QString FMCCDUPageStyleAMenu::SYSTEM_VROUTE = "VROUTE";
const QString FMCCDUPageStyleAMenu::SYSTEM_ICAORTE = "ICAORTE";
const QString FMCCDUPageStyleAMenu::SYSTEM_OOOI = "OOOI";
const QString FMCCDUPageStyleAMenu::SYSTEM_SBOX = "SBOX";
const QString FMCCDUPageStyleAMenu::SYSTEM_SETTING = "SETTINGS";
const QString FMCCDUPageStyleAMenu::SYSTEM_FBW_BANK = "FBW_BANK";
const QString FMCCDUPageStyleAMenu::SYSTEM_FBW_PITCH = "FBW_PITCH";
const QString FMCCDUPageStyleAMenu::SYSTEM_PUSHBACK = "PUSHBACK";
const QString FMCCDUPageStyleAMenu::SYSTEM_CHECKLIST = "CHECKLIST";
const QString FMCCDUPageStyleAMenu::SYSTEM_INTERFACES = "INTERFACES";
const QString FMCCDUPageStyleAMenu::SYSTEM_DISPLAY1 = "DISPLAY1";
const QString FMCCDUPageStyleAMenu::SYSTEM_DISPLAY2 = "DISPLAY2";

#define MAX_CHECKLIST_ITEMS_PER_PAGE 10

/////////////////////////////////////////////////////////////////////////////

FMCCDUPageStyleAMenu::FMCCDUPageStyleAMenu(const QString& page_name, FMCCDUPageManager* page_manager) :
    FMCCDUPageBase(page_name, page_manager), m_weather(CFG_WEATHER_FILENAME),
    m_selected_compact_route_index(0), m_icao_route(0),
    m_pushback_dist_before_turn_m(30), m_pushback_turn_direction_clockwise(true),
    m_pushback_turn_degrees(90), m_pushback_dist_after_turn_m(5), m_aircraft_data_load_page(false)
{
    MYASSERT(connect(&m_weather, SIGNAL(signalGotWeather(const QString&, const QString&, const QString&)),
                     this, SLOT(slotGotWeather(const QString&, const QString&, const QString&))));
    MYASSERT(connect(&m_weather, SIGNAL(signalError(const QString&)), 
                     this, SLOT(slotGotWeatherError(const QString&))));

    MYASSERT(connect(&m_vroute, SIGNAL(signalGotRoute(const CompactRouteList&)),
                     this, SLOT(slotGotRoute(const CompactRouteList&))));
    MYASSERT(connect(&m_vroute, SIGNAL(signalError(const QString&)), 
                     this, SLOT(slotGotRouteError(const QString&))));

    m_selected_system = SYSTEM_FMCS;
    m_aircraft_data_blink_timer.start();

    MYASSERT(connect(&m_checklist_view_timer, SIGNAL(timeout()),
                     this, SLOT(slotSetVerticalScrollOffsetForChecklist())));
    m_checklist_view_timer.start(500);
}

/////////////////////////////////////////////////////////////////////////////

FMCCDUPageStyleAMenu::~FMCCDUPageStyleAMenu()
{
    delete m_icao_route;
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAMenu::paintPage(QPainter& painter) const
{
    painter.setBackground(QBrush(BLACK));
    painter.fillRect(painter.window(), painter.background());

    if (m_selected_system.isEmpty())
    {
        setFont(painter, NORM_FONT, QFont::Bold);
        drawTextCenter(painter, 1, "MCDU MENU");
        setFont(painter, NORM_FONT);
        drawTextLeft(painter, 1, 3, "<FMGC");
        drawTextLeft(painter, 1, 5, "<ACARS");
        drawTextLeft(painter, 1, 7, "<SQUAWKBOX");
        if (fmcControl().isMSFSActive()) drawTextLeft(painter, 1, 9, "<PUSHBACK");
        drawTextLeft(painter, 1, 11, "<CHECKLIST");

        drawTextRight(painter, 1, 3, "SETTINGS>");
        drawTextRight(painter, 1, 5, "FBW BANK>");
        drawTextRight(painter, 1, 7, "FBW PITCH>");
        drawTextRight(painter, 1, 9, "DISPLAY1>");
        drawTextRight(painter, 1, 11, "DISPLAY2>");
        drawTextRight(painter, 1, 13, "INTERFACES>");
    }
    else if (m_selected_system == SYSTEM_FMCS)
    {
        if (!m_aircraft_data_load_page)
        {
            setFont(painter, SMALL_FONT, QFont::Bold);

            QString actype= m_flightstatus->aircraft_type.left(m_max_columns);
            if (m_flightstatus->aircraft_type.length() > (int)m_max_columns) actype += "...";
            drawTextCenter(painter, 1, actype);
        
            setFont(painter, SMALL_FONT);
            drawTextLeft(painter, 2, 2, "FMC VERSION");
            drawTextLeft(painter, 2, 4, "ACFT DATA");

            if (!fmcControl().areAircraftDataConfirmed())
                drawTextRight(painter, 2, 4, "CONFIRM ACFT DATA");

            drawTextLeft(painter, 2, 6, "COPYRIGHT");
            drawTextLeft(painter, 2, 8, "HOMEPAGE");
            drawTextLeft(painter, 2, 10, "ACTIVE NAV DATA BASE");
                
            setFont(painter, NORM_FONT);
            drawTextLeft(painter, 1, 3, QString("%1 %2").arg(PROCNAME).arg(VERSION), GREEN);

            if (!fmcControl().areAircraftDataConfirmed() &&
                m_aircraft_data_blink_timer.elapsed() < 20000 && 
                (m_aircraft_data_blink_timer.elapsed() / 500) % 2 == 0)
                drawTextLeft(painter, 1, 5, "CHECK", RED);
            else
                drawTextLeft(painter, 1, 5, QString("<- %1").arg(fmcControl().aircraftData().name()), CYAN);

            if (!fmcControl().areAircraftDataConfirmed())
                drawTextRight(painter, 1, 5, "CONFIRM ->", RED);

            drawTextLeft(painter, 1, 7, COPYRIGHT_SHORT, GREEN);
            drawTextLeft(painter, 1, 9, HOMEPAGE, GREEN);

            if (fmcControl().navdata().isValid())
            {
                drawTextLeft(painter, 1, 11, QString("AIRAC %1").
                             arg(fmcControl().navdata().getAiracCycleTitle()), GREEN);
                drawTextLeft(painter, 1, 12, QString("%1").
                             arg(fmcControl().navdata().getAiracCycleDates()), GREEN);
            }
            else
            {
                drawTextLeft(painter, 1, 11, "NAVDATA NOT VALID", RED);
            }
        }
        else
        {
            setFont(painter, NORM_FONT, QFont::Bold);
            drawTextCenter(painter, 1, "AIRCRAFT DATA FILES");

            setFont(painter, NORM_FONT);

            int line = 3;
            for(int index = m_vertical_scroll_offset; index < m_aircraft_data_file_list.count(); ++index)
            {
                drawTextLeft(painter, 1, line, QString("<- %1").arg(m_aircraft_data_file_list[index]).toUpper(), CYAN);
                line += 2;
                if (line > 11) break;
            }

            drawTextLeft(painter, 1, 13, "< RETURN");
        }
    }
    else if (m_selected_system == SYSTEM_ACARS)
    {
        setFont(painter, NORM_FONT, QFont::Bold);
        drawTextCenter(painter, 1, "ACARS");
        
        setFont(painter, NORM_FONT);
        drawTextLeft(painter, 1, 3, "<- WEATHER", CYAN);
        drawTextLeft(painter, 1, 5, "<- VROUTE FLIGHTPLANS", CYAN);
        drawTextLeft(painter, 1, 7, "<- ICAO RTE", CYAN);
        drawTextLeft(painter, 1, 9, "<- OOOI TIMES", CYAN);
    }
    else if (m_selected_system == SYSTEM_WEATHER)
    {
        setFont(painter, NORM_FONT, QFont::Bold);
        drawTextCenter(painter, 1, "WEATHER");

        setFont(painter, SMALL_FONT);
        drawTextLeft(painter, 2, 2, "AIRPORT");
        
        setFont(painter, NORM_FONT);
        drawTextLeft(painter, 1, 5, "<- REQ METAR", CYAN);
        drawTextRight(painter, 1, 3, "REQ S-TAF ->", CYAN);
        drawTextRight(painter, 1, 5, "REQ TAF ->", CYAN);

        if (m_weather_airport.isEmpty()) drawTextLeft(painter, 1, 3, QString(SPACER)+SPACER+SPACER+SPACER, ORANGE);
        else drawTextLeft(painter, 1, 3, m_weather_airport, GREEN);

        if (!m_weather_text_list.isEmpty())
        {
            int line_index = 0;
            int line = 7;
            while(line < 14 && (line_index + m_vertical_scroll_offset) < m_weather_text_list.count())
            {
                drawTextLeft(painter, 1, line, m_weather_text_list[line_index + m_vertical_scroll_offset], GREEN);
                ++line;
                ++line_index;
            }
        }
    }
    else if (m_selected_system == SYSTEM_VROUTE)
    {
        setFont(painter, NORM_FONT, QFont::Bold);
        drawTextCenter(painter, 1, "VROUTE");
        
        setFont(painter, SMALL_FONT);
        drawTextRight(painter, 2, 2, "FROM/TO");

        drawTextLeft(painter, 2, 4, "ACTIVE AIRAC ONLY");

        setFont(painter, NORM_FONT);
        drawTextLeft(painter, 1, 3, "<- REQ FP", CYAN);

        (fmcControl().isVrouteAiracRestrictionSet()) ?
            drawTextLeft(painter, 1, 5, "<- YES", CYAN) :
            drawTextLeft(painter, 1, 5, "<- NO", CYAN);
        
        if (m_fp_adep.isEmpty() || m_fp_ades.isEmpty())
            drawTextRight(painter, 1, 3, QString(SPACER)+SPACER+SPACER+SPACER+"/"+SPACER+SPACER+SPACER+SPACER, ORANGE);
        else
            drawTextRight(painter, 1, 3, QString("%1/%2").arg(m_fp_adep).arg(m_fp_ades), CYAN);

        if (!m_route_text_list.isEmpty())
        {
            MYASSERT((int)m_selected_compact_route_index < m_compact_route_list.count());
            const CompactRoute& route = m_compact_route_list[m_selected_compact_route_index];

            drawTextLeft(painter, 1, 7, QString("%1nm").arg(route.m_distance), GREEN);
            drawTextRight(painter, 1,7, QString("%1 / %2").arg(route.m_min_fl).arg(route.m_max_fl), GREEN);

            int line_index = 0;
            int line = 9;
            while(line < 12 && (line_index + m_vertical_scroll_offset) < m_route_text_list.count())
            {
                drawTextLeft(painter, 1, line, m_route_text_list[line_index + m_vertical_scroll_offset], GREEN);
                ++line;
                ++line_index;
            }

            if (fmcControl().allowFPLoad())            
                drawTextLeft(painter, 1, 13, "*TO ACTIVE", ORANGE);
            drawTextRight(painter, 1, 13, "TO SEC >", ORANGE);

            setFont(painter, SMALL_FONT);
            drawTextLeft(painter, 2, 6, "DIST");
            drawTextRight(painter, 2, 6, "MIN FL / MAX FL");
            drawTextLeft(painter, 2, 8, "ROUTE");
        }
    }
    else if (m_selected_system == SYSTEM_ICAORTE)
    {
        setFont(painter, NORM_FONT, QFont::Bold);
        drawTextCenter(painter, 1, "ICAO RTE");
    
        setFont(painter, SMALL_FONT);
        drawTextRight(painter, 2, 2, "FROM/TO", WHITE);
        drawTextLeft(painter, 2, 4, "RTE", WHITE);

        setFont(painter, NORM_FONT);

        if (m_fp_adep.isEmpty() || m_fp_ades.isEmpty())
        {
            drawTextRight(painter, 1, 3, QString(SPACER)+SPACER+SPACER+SPACER+"/"+SPACER+SPACER+SPACER+SPACER, ORANGE);
        }
        else
        {
            drawTextRight(painter, 1, 3, QString("%1/%2").arg(m_fp_adep).arg(m_fp_ades), CYAN);
            drawTextLeft(painter, 1, 3, "<- INSERT", CYAN);
        }

        if (m_icao_route != 0)
        {
            if (fmcControl().allowFPLoad())
                drawTextLeft(painter, 1, 13, "*TO ACTIVE", ORANGE);
            drawTextRight(painter, 1, 13, "TO SEC >", ORANGE);

            drawRouteWaypoints(painter, *m_icao_route, 5, 11);
        }
    }
    else if (m_selected_system == SYSTEM_OOOI)
    {
        setFont(painter, NORM_FONT, QFont::Bold);
        drawTextCenter(painter, 1, "OOOI UTC TIMES");
    
        setFont(painter, SMALL_FONT);
        drawTextLeft(painter, 2, 2, "OUT", WHITE);
        drawTextLeft(painter, 2, 4, "OFF", WHITE);
        drawTextLeft(painter, 2, 6, "ON", WHITE);
        drawTextLeft(painter, 2, 8, "IN", WHITE);

        setFont(painter, NORM_FONT);

        if (fmcControl().flightModeTracker().timeOut().isValid())
            drawTextLeft(painter, 1, 3, fmcControl().flightModeTracker().timeOut().toString("hhmm"), GREEN);
        if (fmcControl().flightModeTracker().timeOff().isValid())
            drawTextLeft(painter, 1, 5, fmcControl().flightModeTracker().timeOff().toString("hhmm"), GREEN);
        if (fmcControl().flightModeTracker().timeOn().isValid())
            drawTextLeft(painter, 1, 7, fmcControl().flightModeTracker().timeOn().toString("hhmm"), GREEN);
        if (fmcControl().flightModeTracker().timeIn().isValid())
            drawTextLeft(painter, 1, 9, fmcControl().flightModeTracker().timeIn().toString("hhmm"), GREEN);

    }
    else if (m_selected_system == SYSTEM_SBOX)
    {
        setFont(painter, NORM_FONT, QFont::Bold);
        drawTextCenter(painter, 1, "SQUAWKBOX");
        
        setFont(painter, SMALL_FONT);
        drawTextLeft(painter, 2, 2, "HANDLING MODE", WHITE);

        setFont(painter, NORM_FONT);
        drawTextLeft(painter, 1, 3, fmcControl().transponderHandlingModeString(), GREEN);
        
        drawTextLeft(painter, 1, 5, "<- MANUAL", CYAN);
        drawTextLeft(painter, 1, 7, "<- AUTOMATIC", CYAN);
        drawTextLeft(painter, 1, 9, "<- ON", CYAN);
        drawTextLeft(painter, 1, 11, "<- OFF", CYAN);

        drawTextRight(painter, 1, 3, "IDENT ->", CYAN);
    }
    else if (m_selected_system == SYSTEM_SETTING)
    {
        setFont(painter, NORM_FONT, QFont::Bold);
        drawTextCenter(painter, 1, "SETTINGS");

        setFont(painter, SMALL_FONT);
        drawTextLeft(painter, 2, 2, "INPUTAREAS");
        drawTextLeft(painter, 2, 4, "SOUNDS");
        drawTextLeft(painter, 2, 6, "SOUND CHANNELS");
        drawTextLeft(painter, 2, 8, "TIME SYNC");
        drawTextLeft(painter, 2, 10, "DATE SYNC");

        drawTextRight(painter, 2, 2, "TCAS");
        drawTextRight(painter, 2, 4, "FLY BY WIRE");
        drawTextRight(painter, 2, 6, "A/THR MODE");
        drawTextRight(painter, 2, 8, "AIRBUS A/THR");
        
        if (fmcControl().mainConfig().getIntValue(CFG_STYLE) == CFG_STYLE_A)
            drawTextRight(painter, 2, 10, "AIRBUS FLAPS");

        drawTextRight(painter, 2, 12, "SEPARATE THR LEVER");

        setFont(painter, NORM_FONT);
        drawTextLeft(painter, 1, 3, fmcControl().showInputAreas() ? "<- ON" : "<- OFF", CYAN);
        drawTextLeft(painter, 1, 5, fmcControl().soundsEnabled() ? "<- ON" : "<- OFF", CYAN);
        drawTextLeft(painter, 1, 7, fmcControl().soundChannelsEnabled() ? "<- ON" : "<- OFF", CYAN);
        drawTextLeft(painter, 1, 9, fmcControl().timeSyncEnabled() ? "<- ON" : "<- OFF", CYAN);
        drawTextLeft(painter, 1, 11, fmcControl().dateSyncEnabled() ? "<- ON" : "<- OFF", CYAN);

        drawTextRight(painter, 1, 3, fmcControl().isTCASOn() ? "ON ->" : 
                      (fmcControl().isTCASStandby() ? "STANDBY ->" : "OFF ->"), CYAN);
        drawTextRight(painter, 1, 5, fmcControl().fbwEnabled() ? "ON ->" : "OFF ->", CYAN);

        drawTextRight(painter, 1, 7, fmcControl().fmcAutothrustEnabled() ? "VASFMC ->" : "FLIGHTSIM ->", CYAN);
        drawTextRight(painter, 1, 9, 
                      (fmcControl().fmcAutothrottle().useAirbusThrottleModes()) ? "YES ->" : "NO ->", CYAN);

        if (fmcControl().mainConfig().getIntValue(CFG_STYLE) == CFG_STYLE_A)
            drawTextRight(painter, 1, 11, (fmcControl().isAirbusFlapsHandlingModeEnabled()) ? "YES ->" : "NO ->", CYAN);

        drawTextRight(painter, 1, 13, (fmcControl().isSeperateThrottleLeverInputModeEnabled()) ? "YES ->" : "NO ->", CYAN);
    }
    else if (m_selected_system == SYSTEM_FBW_BANK)
    {
        setFont(painter, NORM_FONT, QFont::Bold);
        drawTextCenter(painter, 1, "FLY BY WIRE BANK");

        setFont(painter, SMALL_FONT);
        drawTextLeft(painter, 2, 2, "MAX IDLE BANK");
        drawTextLeft(painter, 2, 4, "MAX FORCED BANK");
        drawTextLeft(painter, 2, 6, "MAX BANK RATE");
        drawTextLeft(painter, 2, 8, "P GAIN");
        drawTextLeft(painter, 2, 10, "I GAIN");
        drawTextLeft(painter, 2, 12, "D GAIN");

        drawTextRight(painter, 2, 4, "I-P RESPONSE");

        drawTextRight(painter, 2, 12, "DO STATISTICS");

        setFont(painter, NORM_FONT);
        drawTextLeft(painter, 1, 3, QString::number(fmcControl().bankController()->maxIdleBank(), 'f', 2), CYAN);
        drawTextLeft(painter, 1, 5, QString::number(fmcControl().bankController()->maxForcedBank(), 'f', 2), CYAN);
        drawTextLeft(painter, 1, 7, QString::number(fmcControl().bankController()->maxBankrate(), 'f', 2), CYAN);
        drawTextLeft(painter, 1, 9, QString::number(fmcControl().bankController()->pGain(), 'f', 2), CYAN);
        drawTextLeft(painter, 1, 11, QString::number(fmcControl().bankController()->iGain(), 'f', 2),CYAN);
        drawTextLeft(painter, 1, 13, QString::number(fmcControl().bankController()->dGain(), 'f', 2), CYAN);

        drawTextRight(painter, 1, 5, 
                      QString::number(fmcControl().bankController()->IToPPartResponseFactor(), 'f', 2), CYAN);

        drawTextRight(painter, 1, 13, fmcControl().bankController()->doStatistics() ? "YES" : "NO", CYAN);
    }
    else if (m_selected_system == SYSTEM_FBW_PITCH)
    {
        setFont(painter, NORM_FONT, QFont::Bold);
        drawTextCenter(painter, 1, "FLY BY WIRE PITCH");

        setFont(painter, SMALL_FONT);
        drawTextLeft(painter, 2, 2, "MAX NEG PITCH");
        drawTextLeft(painter, 2, 4, "MAX POS PITCH");
        drawTextLeft(painter, 2, 6, "MAX PITCH RATE");
        drawTextLeft(painter, 2, 8, "P GAIN");
        drawTextLeft(painter, 2, 10, "I GAIN");
        drawTextLeft(painter, 2, 12, "D GAIN");

        drawTextRight(painter, 2, 2, "GOOD TREND DAMP");
        drawTextRight(painter, 2, 4, "I-P RESPONSE");
        drawTextRight(painter, 2, 6, "BANK RATE BOOST");
        drawTextRight(painter, 2, 8, "STABLE FPV DAMP");
        drawTextRight(painter, 2, 10, "TRANSITION BOOST");
        drawTextRight(painter, 2, 12, "DO STATISTICS");

        setFont(painter, NORM_FONT);
        drawTextLeft(painter, 1, 3, QString::number(fmcControl().pitchController()->maxNegativePitch(), 'f', 2), CYAN);
        drawTextLeft(painter, 1, 5, QString::number(fmcControl().pitchController()->maxPositivePitch(), 'f', 2), CYAN);
        drawTextLeft(painter, 1, 7, QString::number(fmcControl().pitchController()->maxPitchrate(), 'f', 2), CYAN);
        drawTextLeft(painter, 1, 9, QString::number(fmcControl().pitchController()->pGain(), 'f', 2), CYAN);
        drawTextLeft(painter, 1, 11, QString::number(fmcControl().pitchController()->iGain(), 'f', 2),CYAN);
        drawTextLeft(painter, 1, 13, QString::number(fmcControl().pitchController()->dGain(), 'f', 2), CYAN);

        drawTextRight(painter, 1, 3, 
                      QString::number(fmcControl().pitchController()->PIDGoodTrendDampingFactor(), 'f', 2), CYAN);
        drawTextRight(painter, 1, 5, 
                      QString::number(fmcControl().pitchController()->IToPPartResponseFactor(), 'f', 2), CYAN);
        drawTextRight(painter, 1, 7, 
                      QString::number(fmcControl().pitchController()->bankRateBoostFactor(), 'f', 2), CYAN);
        drawTextRight(painter, 1, 9, 
                      QString::number(fmcControl().pitchController()->stableFPVDampFactor(), 'f', 2), CYAN);
        drawTextRight(painter, 1, 11, 
                      QString::number(fmcControl().pitchController()->transitionBoostFactor(), 'f', 2), CYAN);

        drawTextRight(painter, 1, 13, fmcControl().pitchController()->doStatistics() ? "YES" : "NO", CYAN);
    }
    else if (m_selected_system == SYSTEM_PUSHBACK)
    {
        setFont(painter, NORM_FONT, QFont::Bold);
        drawTextCenter(painter, 1, "PUSHBACK");

        if (fmcControl().isMSFSActive())
        {        
            setFont(painter, SMALL_FONT);
            drawTextLeft(painter, 2, 2, "DISTANCE BEFORE TURN", WHITE);
            drawTextLeft(painter, 2, 4, "NOSE TURN DIRECTION", WHITE);
            drawTextLeft(painter, 2, 6, "NOSE TURN DEGREES", WHITE);
            drawTextLeft(painter, 2, 8, "DISTANCE AFTER TURN", WHITE);

            setFont(painter, NORM_FONT);
            drawTextLeft(painter, 1, 3, QString("%1 M").arg(m_pushback_dist_before_turn_m), CYAN);
            drawTextLeft(painter, 1, 5, 
                         m_pushback_turn_direction_clockwise ? "<- CLOCKWISE" : "<- COUNTER CLOCKWISE", CYAN);
            drawTextLeft(painter, 1, 7, QString("%1 °").arg(m_pushback_turn_degrees), CYAN);
            drawTextLeft(painter, 1, 9, QString("%1 M").arg(m_pushback_dist_after_turn_m), CYAN);

            if (m_flightstatus->pushback_status == FSAccess::PUSHBACK_STOP)
            {
                if (!m_flightstatus->onground)              drawTextLeft(painter, 1, 13, "N/A WHILE AIRBORNE", GREEN);
                else if (m_flightstatus->parking_brake_set) drawTextLeft(painter, 1, 13, "RELEASE PARKING BRAKE", GREEN);
                else                                        drawTextLeft(painter, 1, 13, "<- START PUSHBACK", CYAN);
            }
            else
            {
                drawTextLeft(painter, 1, 13, "<- STOP PUSHBACK", CYAN);
            }
        }
        else
        {
            drawTextCenter(painter, 3, "ONLY FOR MS FS YET");
        }
    }
    else if (m_selected_system == SYSTEM_CHECKLIST)
    {
        setFont(painter, NORM_FONT, QFont::Bold);

        if (fmcControl().checklistManager().count() <= 0)
        {
            drawTextCenter(painter, 1, "NO CHECKLISTS LOADED");
        }
        else
        {
            if (!fmcControl().checklistManager().currentChecklist().name().isEmpty())
            {
                drawTextCenter(painter, 1, fmcControl().checklistManager().currentChecklist().name().toUpper());
                setFont(painter, SMALL_FONT);
                drawTextCenter(painter, 2, "CHECKLIST");
            }
            else
            {
                drawTextCenter(painter, 1, "CHECKLISTS");
                setFont(painter, SMALL_FONT);
                drawTextCenter(painter, 4, "YOU CAN ALSO USE THE");
                drawTextCenter(painter, 5, "HIDDEN CLICKSPOT ON THE FCU");
                drawTextCenter(painter, 6, "BELOW THE SPD/MACH BUTTON");
                drawTextCenter(painter, 7, "TO ADVANCE THROUGH THE");
                drawTextCenter(painter, 8, "CHECKLISTS!");
            }

            int cur_item_index = fmcControl().checklistManager().currentChecklist().currentItemIndex();

            //----- item list

            int y_text = 2;
            for(int index=0; index < MAX_CHECKLIST_ITEMS_PER_PAGE; ++index)
            {
                ++y_text;

                int item_index = index + m_vertical_scroll_offset;
                bool active = item_index == cur_item_index;
                bool done = item_index < cur_item_index;
                QColor color = done ? GREEN : (active ? MAGENTA : WHITE);

                const ChecklistItem& item = fmcControl().checklistManager().currentChecklist().item(item_index);

                if (item.resultText().isEmpty() && item.checkText().startsWith("--"))
                {
                    drawTextCenter(painter, y_text, item.checkText().toUpper(), color);
                }
                else
                {
                    drawTextLeft(painter, 1, y_text, item.checkText().toUpper(), color);
                    drawTextRight(painter, 1, y_text, item.resultText().toUpper(), color);
                }
            }

            setFont(painter, NORM_FONT);

            //----- prev arrow
            
            if (fmcControl().checklistManager().currentChecklist().isAtFirstItem())
            {
                if (!fmcControl().checklistManager().isAtFirstChecklist() > 0)
                    drawTextLeft(painter, 1, 13, "<- PREV CL", CYAN);
                else
                    drawTextLeft(painter, 1, 13, "<- RESET", CYAN);
            }
            else
            {
                drawTextLeft(painter, 1, 13, "<- BACK", CYAN);
            }

            //----- next arrow

            if (fmcControl().checklistManager().currentChecklist().isAtLastItem())
            {
                if (!fmcControl().checklistManager().isAtLastChecklist())
                    drawTextRight(painter, 1, 13, "NEXT CL ->", CYAN);
                else
                    drawTextRight(painter, 1, 13, "RESET ->", CYAN);
            }
            else
            {
                drawTextRight(painter, 1, 13, "FORWARD ->", CYAN);
            }
        }
    }
    else if (m_selected_system == SYSTEM_INTERFACES)
    {
        setFont(painter, NORM_FONT, QFont::Bold);
        drawTextCenter(painter, 1, "INTERFACES");
        
        setFont(painter, SMALL_FONT);
        drawTextLeft(painter, 2, 2, "FMC MODE");

        drawTextRight(painter, 2, 2, "IOCP SERVER");
#ifdef SERIAL
        drawTextRight(painter, 2, 4, "CPFLIGHT MCP");
#endif

        setFont(painter, NORM_FONT);
        drawTextLeft(painter, 1, 3, QString("<- ")+fmcControl().getFMCConnectMode().toUpper(), CYAN);

        if (fmcControl().isFMCConnectModeMaster())
        {
            setFont(painter, SMALL_FONT);
            drawTextLeft(painter, 2, 4, "MASTER PORT");
            drawTextLeft(painter, 2, 8, "CONN STATUS");

            setFont(painter, NORM_FONT);
            drawTextLeft(painter, 1, 5, QString::number(fmcControl().getFMCConnectModeMasterPort()), CYAN);
            drawTextLeft(painter, 1, 9, fmcControl().isFMCConnectRemoteConnected() ? 
                         (QString("SERVING (%1)").arg(fmcControl().getFMCConnectModeMasterNrClients())) :
                         "FAILURE", GREEN);
        }
        else if (fmcControl().isFMCConnectModeSlave())
        {
            setFont(painter, SMALL_FONT);
            drawTextLeft(painter, 2, 4, "MASTER PORT");
            drawTextLeft(painter, 2, 6, "MASTER IP");
            drawTextLeft(painter, 2, 8, "CONN STATUS");

            setFont(painter, NORM_FONT);
            drawTextLeft(painter, 1, 5, QString::number(fmcControl().getFMCConnectModeSlaveMasterPort()), CYAN);
            drawTextLeft(painter, 1, 7, fmcControl().getFMCConnectModeSlaveMasterIP().toUpper(), CYAN);
            drawTextLeft(painter, 1, 9, fmcControl().isFMCConnectRemoteConnected() ? 
                         "CONNECTED" : "DISCONNECTED", GREEN);
        }

        drawTextRight(painter, 1, 3, fmcControl().useIOCPServer() ? "ON ->" : "OFF ->", CYAN);
#ifdef SERIAL
        drawTextRight(painter, 1, 5, fmcControl().useCPFlight() ? "ON ->" : "OFF ->", CYAN);
#endif
    }
    else if (m_selected_system == SYSTEM_DISPLAY1)
    {
        setFont(painter, NORM_FONT, QFont::Bold);
        drawTextCenter(painter, 1, "DISPLAY1");

        setFont(painter, SMALL_FONT);
        drawTextCenter(painter, 2, "PFD/ND/ECAM FONT");
        drawTextCenter(painter, 4, "FCU FONT");
        drawTextCenter(painter, 6, "MCDU FONT");

        drawTextLeft(painter, 2, 8, "PFD/ND");
        drawTextLeft(painter, 2, 10, "ECAM/EICAS");
        drawTextLeft(painter, 2, 12, "MCDU/FCU");

        drawTextRight(painter, 2, 8, "SHOW FPS");

        setFont(painter, NORM_FONT);
        drawTextLeft(painter, 1, 3, "<- SMALLER", CYAN);
        drawTextRight(painter, 1, 3, "BIGGER ->", CYAN);

        drawTextLeft(painter, 1, 5, "<- SMALLER", CYAN);
        drawTextRight(painter, 1, 5, "BIGGER ->", CYAN);

        drawTextLeft(painter, 1, 7, "<- SMALLER", CYAN);
        drawTextRight(painter, 1, 7, "BIGGER ->", CYAN);

        drawTextLeft(painter, 1, 9, QString("%1ms").arg(fmcControl().getPFDNDRefreshRateMs()), CYAN);
        drawTextLeft(painter, 8, 9, QString("%1fps").arg(Navcalc::round(1000 / fmcControl().getPFDNDRefreshRateMs())), GREEN);

        drawTextLeft(painter, 1, 11, QString("%1ms").arg(fmcControl().getECAMRefreshRateMs()), CYAN);
        drawTextLeft(painter, 8, 11, QString("%1fps").arg(Navcalc::round(1000 / fmcControl().getECAMRefreshRateMs())), GREEN);

        drawTextLeft(painter, 1, 13, QString("%1ms").arg(fmcControl().getCDUFCURefreshRateMs()), CYAN);
        drawTextLeft(painter, 8, 13, QString("%1fps").arg(Navcalc::round(1000 / fmcControl().getCDUFCURefreshRateMs())), GREEN);

        drawTextRight(painter, 1, 9, QString("%1 ->").arg(fmcControl().showFps() ? "YES" : "NO"), CYAN);
    }
    else if (m_selected_system == SYSTEM_DISPLAY2)
    {
        setFont(painter, NORM_FONT, QFont::Bold);
        drawTextCenter(painter, 1, "DISPLAY2");

        // SMALL
        setFont(painter, SMALL_FONT);

        drawTextLeft(painter, 2, 2, "ND SCROLL");
        drawTextRight(painter, 2, 2, "CDU SCROLL");

        drawTextLeft(painter, 2, 4, "ND WIND CORR");

#if !VASFMC_GAUGE
        drawTextRight(painter, 2, 4, "CDU DISP ONLY");
        drawTextLeft(painter, 2, 6, "EFIS F/O");
        drawTextRight(painter, 2, 6, "KEEP ONTOP");
#endif

        // NORM
        setFont(painter, NORM_FONT);

        drawTextLeft(painter, 1, 3, fmcControl().ndNormalScrollMode() ? "<- NORMAL": "<- INVERSE", CYAN);
        drawTextRight(painter, 1, 3, fmcControl().cduNormalScrollMode() ? "NORMAL ->": "INVERSE ->", CYAN);

        if (fmcControl().mainConfig().getIntValue(CFG_STYLE) != CFG_STYLE_A)
            drawTextLeft(painter, 1, 5, fmcControl().ndWindCorrection() ? "<- YES": "<- NO", CYAN);
        else
            drawTextLeft(painter, 1, 5, "N/A IN A-STYLE", GREEN);

#if !VASFMC_GAUGE
        drawTextRight(painter, 1, 5, fmcControl().cduDisplayOnlyMode() ? "ON ->": "OFF ->", CYAN);
        drawTextLeft(painter, 1, 7, fmcControl().fcuLeftOnlyMode() ? "<- OFF" : "<- ON", CYAN);
        drawTextRight(painter, 1, 7, QString("%1 ->").arg(fmcControl().doKeepOnTop() ? "YES" : "NO"), CYAN);
#endif
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAMenu::slotSetVerticalScrollOffsetForChecklist()
{
    if (m_selected_system != SYSTEM_CHECKLIST) return;

    int cur_item_index = fmcControl().checklistManager().currentChecklist().currentItemIndex();
    
    if (cur_item_index >= MAX_CHECKLIST_ITEMS_PER_PAGE + m_vertical_scroll_offset)
    {
        m_vertical_scroll_offset = 
            qMax(0, qMin((int)maxVerticalScrollOffset(), cur_item_index - MAX_CHECKLIST_ITEMS_PER_PAGE + 1));
    }
    else if (cur_item_index < m_vertical_scroll_offset)
    {
        m_vertical_scroll_offset = qMin(qMax(0, cur_item_index), (int)maxVerticalScrollOffset());
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAMenu::processAction(const QString& action)
{
    int horizontal_scroll_offset = m_horizontal_scroll_offset;

    if (m_selected_system == SYSTEM_CHECKLIST)
    {
        if (action == ACTION_NEXT)
        {
            fmcControl().checklistManager().incChecklistIndex();
            slotSetVerticalScrollOffsetForChecklist();
            return;
        }
        else if (action == ACTION_PREV)
        {
            fmcControl().checklistManager().decChecklistIndex();
            slotSetVerticalScrollOffsetForChecklist();
            return;
        }
    }

    if (processBasicActions(action)) 
    {
        if (m_selected_system == SYSTEM_VROUTE)
        {
            if (horizontal_scroll_offset != m_horizontal_scroll_offset)
            {
                selectCompactRoute(m_horizontal_scroll_offset);
                m_vertical_scroll_offset = 0;
            }
        }

        return;
    }

    QString text = m_page_manager->scratchpad().text();
    int llsk_index = -1;
    int rlsk_index = -1;
    if (!getLSKIndex(action, llsk_index, rlsk_index)) return;

    //-----

    if (m_selected_system.isEmpty())
    {
        setDrawHorizontalScrollPageCounter(true);
        
        switch(llsk_index)
        {
            case(1): m_selected_system = SYSTEM_FMCS; break;
            case(2): m_selected_system = SYSTEM_ACARS; break;
            case(3): m_selected_system = SYSTEM_SBOX; break;
            case(4): {
                if (fmcControl().isMSFSActive()) m_selected_system = SYSTEM_PUSHBACK; 
                break;
            }
            case(5): 
                m_selected_system = SYSTEM_CHECKLIST; 
                setDrawHorizontalScrollPageCounter(false);
                slotSetVerticalScrollOffsetForChecklist();
                break;
        }

        switch(rlsk_index)
        {
            case(1): m_selected_system = SYSTEM_SETTING; break;
            case(2): m_selected_system = SYSTEM_FBW_BANK; break;
            case(3): m_selected_system = SYSTEM_FBW_PITCH; break;
            case(4): m_selected_system = SYSTEM_DISPLAY1; break;
            case(5): m_selected_system = SYSTEM_DISPLAY2; break;
            case(6): m_selected_system = SYSTEM_INTERFACES; break;
        }
    }
    else if (m_selected_system == SYSTEM_FMCS)
    {
        if (!m_aircraft_data_load_page)
        {
            if (llsk_index == 2)
            {
                fmcControl().setAircraftDataConfirmed();
                m_aircraft_data_load_page = true;

                // setup aircraft data list
                
                QDir fpdir(fmcControl().aircraftDataPath());
                
                if (!fpdir.exists()) 
                {
                    Logger::log(QString("FMCCDUPageStyleAMenu:processAction: "
                                        "aircraft data dir (%1) not found").arg(fpdir.path()));
                    
                    m_page_manager->scratchpad().setOverrideText("ACFT DATA DIR NOT FOUND");
                    return;
                }
                
                fpdir.refresh();
                m_aircraft_data_file_list = fpdir.entryList(QStringList(QString("*")+CFG_AIRCRAFT_DATA_EXTENSION),
                                                            QDir::Files|QDir::Readable, QDir::Name|QDir::IgnoreCase);
            }
            else if (rlsk_index == 2 && !fmcControl().areAircraftDataConfirmed())
            {
                fmcControl().setAircraftDataConfirmed();
            }
        }
        else
        {
            if (llsk_index == 6)
            {
                m_aircraft_data_load_page = false;
                return;
            }
            else if (llsk_index > 0)
            {
                int index = ((int)llsk_index) - 1 + m_vertical_scroll_offset;
                
                if (index < m_aircraft_data_file_list.count())
                {
                    fmcControl().loadAircraftData(m_aircraft_data_file_list[index]);
                    m_aircraft_data_load_page = false;
                }
            }
        }

        return;        
    }
    else if (m_selected_system == SYSTEM_ACARS)
    {
        setDrawHorizontalScrollPageCounter(true);
        if (llsk_index == 1) m_selected_system = SYSTEM_WEATHER;
        else if (llsk_index == 2) m_selected_system = SYSTEM_VROUTE;
        else if (llsk_index == 3) m_selected_system = SYSTEM_ICAORTE;
        else if (llsk_index == 4) m_selected_system = SYSTEM_OOOI;
    }
    else if (m_selected_system == SYSTEM_WEATHER)
    {
        if (llsk_index == 1)
        {
            if (text.isEmpty()) 
            {
                m_page_manager->scratchpad().setOverrideText("ENTER AIRPORT CODE");
                return;
            }

            m_weather_airport = text;
            m_weather_date.clear();
            m_weather_text_list.clear();
            m_vertical_scroll_offset = 0;
            m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
        }
        else
        {
            if (m_weather_airport.isEmpty())
            {
                m_page_manager->scratchpad().setOverrideText("ENTER AIRPORT CODE");
                return;
            }
            else
            {
                bool do_request_weather = false;

                m_weather_date.clear();
                m_weather_text_list.clear();
                m_vertical_scroll_offset = 0;
             
                if (llsk_index == 2) 
                {
                    m_weather.requestMetar(m_weather_airport);
                    do_request_weather = true;
                }
                else if (rlsk_index == 1) 
                {
                    m_weather.requestSTAF(m_weather_airport);
                    do_request_weather = true;
                }
                else if (rlsk_index == 2) 
                {
                    m_weather.requestTAF(m_weather_airport);
                    do_request_weather = true;
                }
                
                if (do_request_weather)
                {
                    m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
                    m_page_manager->scratchpad().setOverrideText("REQUESTING...");
                }
            }
        }
    }
    else if (m_selected_system == SYSTEM_VROUTE)
    {
        if (llsk_index == 1) 
        {
            if (m_fp_adep.isEmpty() || m_fp_ades.isEmpty())
            {
                m_page_manager->scratchpad().setOverrideText("ENTER AIRPORT CODES");
                return;
            }

            m_compact_route_list.clear();
            m_selected_compact_route_index = 0;
            m_route_text_list.clear();

            (fmcControl().isVrouteAiracRestrictionSet()) ?
                m_vroute.setAiracRestiction(fmcControl().navdata().getAiracCycleTitle().trimmed()) :
                m_vroute.setAiracRestiction(QString::null);

            m_vroute.requestFP(m_fp_adep, m_fp_ades);
            m_page_manager->scratchpad().setOverrideText("REQUESTING...");
        }
        else if (rlsk_index == 1)
        {
            if (text.isEmpty()) return;

            if (!text.contains('/'))
            {
                m_page_manager->scratchpad().setOverrideText("Invalid value");
                return;
            }
            
            m_fp_adep = text.section('/', 0, 0);
            m_fp_ades = text.section('/', 1, 1);
            if (text.length() > m_fp_adep.length() + m_fp_ades.length() + 1) 
            {
                m_fp_adep.clear();
                m_fp_ades.clear();
                clearICAORoute();
                m_page_manager->scratchpad().setOverrideText("Invalid value");
                return;
            }

            m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
        }
        else if (llsk_index == 2)
        {
            fmcControl().setVrouteAiracRestriction(!fmcControl().isVrouteAiracRestrictionSet());
        }
        else if (!m_route_text_list.isEmpty() && (llsk_index == 6 || rlsk_index == 6))
        {
            if (llsk_index == 6 && airborneAndRouteNotEmpty())
            {
                m_page_manager->scratchpad().setOverrideText("route not empty");
                return;
            }
            
            m_page_manager->scratchpad().setOverrideText("PROCESSING ROUTE...");

            MYASSERT((int)m_selected_compact_route_index < m_compact_route_list.count());
            const CompactRoute& route = m_compact_route_list[m_selected_compact_route_index];

            QString error;
            FlightRoute new_route(m_flightstatus);
            if (!new_route.extractICAORoute(m_fp_adep + " " + route.m_route + " " + m_fp_ades, 
                                            fmcControl().navdata(), LATLON_WAYPOINT_REGEXP, error))
            {
                m_page_manager->scratchpad().setOverrideText(error);
                return;
            }

            if (new_route.departureAirport() == 0 || new_route.destinationAirport() == 0)
            {
                m_page_manager->scratchpad().setOverrideText("No ADEP/ADES found");
                return;
            }

            // activate the new route

            if (llsk_index == 6 && fmcControl().allowFPLoad())
            {
                fmcControl().normalRoute() = new_route;
                if (m_flightstatus->onground) fmcControl().setIRSAlignNeeded(true);
                m_page_manager->setCurrentPage(FMCCDUPageManagerStyleA::PAGE_INIT);
                m_page_manager->scratchpad().setOverrideText(QString::null);
            }
            else if (rlsk_index == 6)
            {
                fmcControl().secondaryRoute() = new_route;
                m_page_manager->setCurrentPage(FMCCDUPageManagerStyleA::PAGE_SECFP);
                m_page_manager->scratchpad().setOverrideText(QString::null);
            }   

            m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
        }
    }
    else if (m_selected_system == SYSTEM_ICAORTE)
    {
        if (llsk_index == 1) 
        {
            if (text.isEmpty() || m_fp_adep.isEmpty() || m_fp_ades.isEmpty()) return;

            if (m_fp_adep.isEmpty() || m_fp_ades.isEmpty())
            {
                m_page_manager->scratchpad().setOverrideText("ENTER AIRPORT CODES");
                return;
            }

            clearICAORoute();
            m_icao_route = new FlightRoute(m_flightstatus);
            MYASSERT(m_icao_route != 0);

            QString error;
            if (!m_icao_route->extractICAORoute(m_fp_adep + " " + text + " " + m_fp_ades, 
                                                fmcControl().navdata(), LATLON_WAYPOINT_REGEXP, error))
            {
                clearICAORoute();
                m_page_manager->scratchpad().setOverrideText(error);
                return;
            }

            if (m_icao_route->departureAirport() == 0 || m_icao_route->destinationAirport() == 0)
            {
                clearICAORoute();
                m_page_manager->scratchpad().setOverrideText("No ADEP/ADES found");
                return;
            }

            MYASSERT(m_icao_route != 0);
            fmcControl().temporaryRoute() = *m_icao_route;

            m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
        }
        else if (rlsk_index == 1)
        {
            if (text.isEmpty()) return;

            clearICAORoute();

            if (!text.contains('/'))
            {
                m_page_manager->scratchpad().setOverrideText("Invalid value");
                return;
            }

            m_fp_adep = text.section('/', 0, 0);
            m_fp_ades = text.section('/', 1, 1);
            if (text.length() > m_fp_adep.length() + m_fp_ades.length() + 1) 
            {
                m_fp_adep.clear();
                m_fp_ades.clear();
                clearICAORoute();
                m_page_manager->scratchpad().setOverrideText("Invalid value");
                return;
            }

            m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
        }
        else if (llsk_index == 6 && fmcControl().allowFPLoad())
        {
            if (m_icao_route == 0) 
            {
                Logger::log("FMCCDUPageStyleAMenu:processAction: m_icao_route not set");
                return;
            }

            fmcControl().normalRoute() = *m_icao_route;
            if (m_flightstatus->onground) fmcControl().setIRSAlignNeeded(true);
            fmcControl().temporaryRoute().clear();
            m_page_manager->setCurrentPage(FMCCDUPageManagerStyleA::PAGE_INIT);
            m_page_manager->scratchpad().setOverrideText(QString::null);
        }
        else if (rlsk_index == 6)
        {
            if (m_icao_route == 0) 
            {
                Logger::log("FMCCDUPageStyleAMenu:processAction: m_icao_route not set");
                return;
            }

            fmcControl().secondaryRoute() = *m_icao_route;
            fmcControl().temporaryRoute().clear();
            m_page_manager->setCurrentPage(FMCCDUPageManagerStyleA::PAGE_SECFP);
            m_page_manager->scratchpad().setOverrideText(QString::null);
        }   
    }
    else if (m_selected_system == SYSTEM_SBOX)
    {
        switch(llsk_index)
        {
            case(2): fmcControl().setTransponderHandlingDisabled(); break;
            case(3): fmcControl().setTransponderHandlingAutomatic(); break;
            case(4): fmcControl().setTransponderHandlingOn(); break;
            case(5): fmcControl().setTransponderHandlingOff(); break;
        }

        switch(rlsk_index)
        {
            case(1): fmcControl().setTransponderIdent(); break;
        }
    }
    else if (m_selected_system == SYSTEM_SETTING)
    {
        switch(llsk_index)
        {
            case(1): fmcControl().slotToggleShowInputAreas(); break;
            case(2): fmcControl().slotToggleSounds(); break;
            case(3): fmcControl().slotToggleSoundChannels(); break;
            case(4): fmcControl().slotToggleTimeSync(); break;
            case(5): fmcControl().slotToggleDateSync(); break;
        }
         
        switch(rlsk_index)
        {
            case(1): fmcControl().toggleTCAS(); break;
            case(2): fmcControl().slotToggleFbw(); break;
            case(3): fmcControl().slotToggleFmcAutothrustEnabled(); break;
            case(4): fmcControl().fmcAutothrottle().setUseAirbusThrottleModes(
                !fmcControl().fmcAutothrottle().useAirbusThrottleModes());
                break;
            case(5): {
                if (fmcControl().mainConfig().getIntValue(CFG_STYLE) == CFG_STYLE_A)
                    fmcControl().setAirbusFlapsHandlingMode(!fmcControl().isAirbusFlapsHandlingModeEnabled());
                break;
            }
            case(6):
                fmcControl().setSeparateThrottleLeverInputMode(!fmcControl().isSeperateThrottleLeverInputModeEnabled());
                break;
        }
    }
    else if (m_selected_system == SYSTEM_FBW_BANK)
    {
        if (rlsk_index == 6)
        {
            fmcControl().bankController()->setDoStatistics(
                !fmcControl().bankController()->doStatistics());
        }

        if (text.isEmpty()) return;

        bool convok = false;
        double value = text.toDouble(&convok);
        if (!convok)
        {
            m_page_manager->scratchpad().setOverrideText("INVALID VALUE");
            return;
        }

        switch(llsk_index)
        {
            case(1): 
                if (value < 5.0 || value >= fmcControl().bankController()->maxForcedBank()) 
                {
                    m_page_manager->scratchpad().setOverrideText("VALUE OUT OF RANGE");
                    return;
                }
                fmcControl().bankController()->setBankLimits(
                    value, fmcControl().bankController()->maxForcedBank()); break;
            case(2):
                if (value < 15.0 || value <= fmcControl().bankController()->maxIdleBank()) 
                {
                    m_page_manager->scratchpad().setOverrideText("VALUE OUT OF RANGE");
                    return;
                }
                fmcControl().bankController()->setBankLimits(
                    fmcControl().bankController()->maxIdleBank(), value); break;
            case(3): 
                if (value <= 3.0) 
                {
                    m_page_manager->scratchpad().setOverrideText("VALUE OUT OF RANGE");
                    return;
                }
                fmcControl().bankController()->setPIDParams(
                    fmcControl().bankController()->pGain(),
                    fmcControl().bankController()->iGain(),
                    fmcControl().bankController()->dGain(),
                    value,
                    fmcControl().bankController()->IToPPartResponseFactor());
                break;
            case(4):
                if (value < 0.0) 
                {
                    m_page_manager->scratchpad().setOverrideText("VALUE OUT OF RANGE");
                    return;
                }
                fmcControl().bankController()->setPIDParams(
                    value,
                    fmcControl().bankController()->iGain(),
                    fmcControl().bankController()->dGain(),
                    fmcControl().bankController()->maxBankrate(),
                    fmcControl().bankController()->IToPPartResponseFactor());
                break;
            case(5):
                if (value < 0.0) 
                {
                    m_page_manager->scratchpad().setOverrideText("VALUE OUT OF RANGE");
                    return;
                }
                fmcControl().bankController()->setPIDParams(
                    fmcControl().bankController()->pGain(),
                    value,
                    fmcControl().bankController()->dGain(),
                    fmcControl().bankController()->maxBankrate(),
                    fmcControl().bankController()->IToPPartResponseFactor());
                break;
            case(6): 
                if (value < 0.0) 
                {
                    m_page_manager->scratchpad().setOverrideText("VALUE OUT OF RANGE");
                    return;
                }
                fmcControl().bankController()->setPIDParams(
                    fmcControl().bankController()->pGain(),
                    fmcControl().bankController()->iGain(),
                    value,
                    fmcControl().bankController()->maxBankrate(),
                    fmcControl().bankController()->IToPPartResponseFactor());
                break;
        }

        switch(rlsk_index)
        {
            case(2):
                if (value < 0.1)
                {
                    m_page_manager->scratchpad().setOverrideText("VALUE OUT OF RANGE");
                    return;
                }
                fmcControl().bankController()->setPIDParams(
                    fmcControl().bankController()->pGain(),
                    fmcControl().bankController()->iGain(),
                    fmcControl().bankController()->dGain(),
                    fmcControl().bankController()->maxBankrate(),
                    value);
                break;
        }

        m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
    }
    else if (m_selected_system == SYSTEM_FBW_PITCH)
    {
        if (rlsk_index == 6)
        {
            fmcControl().pitchController()->setDoStatistics(
                !fmcControl().pitchController()->doStatistics());
            return;
        }

        if (text.isEmpty()) return;

        bool convok = false;
        double value = text.toDouble(&convok);
        if (!convok)
        {
            m_page_manager->scratchpad().setOverrideText("INVALID VALUE");
            return;
        }

        switch(llsk_index)
        {
            case(1): 
                if (value >= -10.0 ) 
                {
                    m_page_manager->scratchpad().setOverrideText("VALUE OUT OF RANGE");
                    return;
                }
                fmcControl().pitchController()->setPitchLimits(
                    value, fmcControl().pitchController()->maxPositivePitch()); break;
                break;
            case(2):
                if (value <= 10.0) 
                {
                    m_page_manager->scratchpad().setOverrideText("VALUE OUT OF RANGE");
                    return;
                }
                fmcControl().pitchController()->setPitchLimits(
                    fmcControl().pitchController()->maxNegativePitch(), value); break;
                break;
            case(3): 
                if (value <= 0.5) 
                {
                    m_page_manager->scratchpad().setOverrideText("VALUE OUT OF RANGE");
                    return;
                }
                fmcControl().pitchController()->setPIDParams(
                    fmcControl().pitchController()->pGain(),
                    fmcControl().pitchController()->iGain(),
                    fmcControl().pitchController()->dGain(),
                    value,
                    fmcControl().pitchController()->PIDGoodTrendDampingFactor(),
                    fmcControl().pitchController()->IToPPartResponseFactor(),
                    fmcControl().pitchController()->bankRateBoostFactor(),
                    fmcControl().pitchController()->stableFPVDampFactor(),
                    fmcControl().pitchController()->transitionBoostFactor());
                break;
            case(4):
                if (value < 0.0) 
                {
                    m_page_manager->scratchpad().setOverrideText("VALUE OUT OF RANGE");
                    return;
                }
                fmcControl().pitchController()->setPIDParams(
                    value,
                    fmcControl().pitchController()->iGain(),
                    fmcControl().pitchController()->dGain(),
                    fmcControl().pitchController()->maxPitchrate(),
                    fmcControl().pitchController()->PIDGoodTrendDampingFactor(),
                    fmcControl().pitchController()->IToPPartResponseFactor(),
                    fmcControl().pitchController()->bankRateBoostFactor(),
                    fmcControl().pitchController()->stableFPVDampFactor(),
                    fmcControl().pitchController()->transitionBoostFactor());
                break;
            case(5):
                if (value < 0.0) 
                {
                    m_page_manager->scratchpad().setOverrideText("VALUE OUT OF RANGE");
                    return;
                }
                fmcControl().pitchController()->setPIDParams(
                    fmcControl().pitchController()->pGain(),
                    value,
                    fmcControl().pitchController()->dGain(),
                    fmcControl().pitchController()->maxPitchrate(),
                    fmcControl().pitchController()->PIDGoodTrendDampingFactor(),
                    fmcControl().pitchController()->IToPPartResponseFactor(),
                    fmcControl().pitchController()->bankRateBoostFactor(),
                    fmcControl().pitchController()->stableFPVDampFactor(),
                    fmcControl().pitchController()->transitionBoostFactor());
                break;
            case(6): 
                if (value < 0.0) 
                {
                    m_page_manager->scratchpad().setOverrideText("VALUE OUT OF RANGE");
                    return;
                }
                fmcControl().pitchController()->setPIDParams(
                    fmcControl().pitchController()->pGain(),
                    fmcControl().pitchController()->iGain(),
                    value,
                    fmcControl().pitchController()->maxPitchrate(),
                    fmcControl().pitchController()->PIDGoodTrendDampingFactor(),
                    fmcControl().pitchController()->IToPPartResponseFactor(),
                    fmcControl().pitchController()->bankRateBoostFactor(),
                    fmcControl().pitchController()->stableFPVDampFactor(),
                    fmcControl().pitchController()->transitionBoostFactor());
                break;
        }

        switch(rlsk_index)
        {
            case(1): 
                if (value < 0.0) 
                {
                    m_page_manager->scratchpad().setOverrideText("VALUE OUT OF RANGE");
                    return;
                }
                fmcControl().pitchController()->setPIDParams(
                    fmcControl().pitchController()->pGain(),
                    fmcControl().pitchController()->iGain(),
                    fmcControl().pitchController()->dGain(),
                    fmcControl().pitchController()->maxPitchrate(),
                    value,
                    fmcControl().pitchController()->IToPPartResponseFactor(),
                    fmcControl().pitchController()->bankRateBoostFactor(),
                    fmcControl().pitchController()->stableFPVDampFactor(),
                    fmcControl().pitchController()->transitionBoostFactor());
                break;
            case(2):
                if (value < 0.1) 
                {
                    m_page_manager->scratchpad().setOverrideText("VALUE OUT OF RANGE");
                    return;
                }
                fmcControl().pitchController()->setPIDParams(
                    fmcControl().pitchController()->pGain(),
                    fmcControl().pitchController()->iGain(),
                    fmcControl().pitchController()->dGain(),
                    fmcControl().pitchController()->maxPitchrate(),
                    fmcControl().pitchController()->PIDGoodTrendDampingFactor(),
                    value,
                    fmcControl().pitchController()->bankRateBoostFactor(),
                    fmcControl().pitchController()->stableFPVDampFactor(),
                    fmcControl().pitchController()->transitionBoostFactor());
                break;
            case(3): 
                if (value < 0.0) 
                {
                    m_page_manager->scratchpad().setOverrideText("VALUE OUT OF RANGE");
                    return;
                }
                fmcControl().pitchController()->setPIDParams(
                    fmcControl().pitchController()->pGain(),
                    fmcControl().pitchController()->iGain(),
                    fmcControl().pitchController()->dGain(),
                    fmcControl().pitchController()->maxPitchrate(),
                    fmcControl().pitchController()->PIDGoodTrendDampingFactor(),
                    fmcControl().pitchController()->IToPPartResponseFactor(),
                    value,
                    fmcControl().pitchController()->stableFPVDampFactor(),
                    fmcControl().pitchController()->transitionBoostFactor());
                break;
            case(4): 
                if (value < 0.1)
                {
                    m_page_manager->scratchpad().setOverrideText("VALUE OUT OF RANGE");
                    return;
                }
                fmcControl().pitchController()->setPIDParams(
                    fmcControl().pitchController()->pGain(),
                    fmcControl().pitchController()->iGain(),
                    fmcControl().pitchController()->dGain(),
                    fmcControl().pitchController()->maxPitchrate(),
                    fmcControl().pitchController()->PIDGoodTrendDampingFactor(),
                    fmcControl().pitchController()->IToPPartResponseFactor(),
                    fmcControl().pitchController()->bankRateBoostFactor(),
                    value,
                    fmcControl().pitchController()->transitionBoostFactor());
                break;
            case(5): 
                if (value < 1.0)
                {
                    m_page_manager->scratchpad().setOverrideText("VALUE OUT OF RANGE");
                    return;
                }
                fmcControl().pitchController()->setPIDParams(
                    fmcControl().pitchController()->pGain(),
                    fmcControl().pitchController()->iGain(),
                    fmcControl().pitchController()->dGain(),
                    fmcControl().pitchController()->maxPitchrate(),
                    fmcControl().pitchController()->PIDGoodTrendDampingFactor(),
                    fmcControl().pitchController()->IToPPartResponseFactor(),
                    fmcControl().pitchController()->bankRateBoostFactor(),
                    fmcControl().pitchController()->stableFPVDampFactor(),
                    value);
                break;
        }

        m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
    }
    else if (m_selected_system == SYSTEM_PUSHBACK && fmcControl().isMSFSActive())
    {
        bool convok = false;
        int value = text.toInt(&convok);

        switch(llsk_index)
        {
            case(1): {

                if (text.isEmpty()) return;

                if (!convok)
                {
                    m_page_manager->scratchpad().setOverrideText("INVALID VALUE");
                    return;
                }

                if (value < 0 || value >1000)
                {
                    m_page_manager->scratchpad().setOverrideText("VALUE OUT OF RANGE");
                    return;
                }

                m_pushback_dist_before_turn_m = value;

                break;
            }

            case(2): {
               
                m_pushback_turn_direction_clockwise = !m_pushback_turn_direction_clockwise;
                break;
            }

            case(3): {

                if (text.isEmpty()) return;

                if (!convok)
                {
                    m_page_manager->scratchpad().setOverrideText("INVALID VALUE");
                    return;
                }

                if (value < 0 || value >90)
                {
                    m_page_manager->scratchpad().setOverrideText("VALUE OUT OF RANGE");
                    return;
                }

                m_pushback_turn_degrees = value;

                break;
            }

            case(4): {

                if (text.isEmpty()) return;

                if (!convok)
                {
                    m_page_manager->scratchpad().setOverrideText("INVALID VALUE");
                    return;
                }

                if (value < 0 || value >1000)
                {
                    m_page_manager->scratchpad().setOverrideText("VALUE OUT OF RANGE");
                    return;
                }

                m_pushback_dist_after_turn_m = value;

                break;
            }

            case(6): {

                if (m_flightstatus->pushback_status == FSAccess::PUSHBACK_STOP)
                {
                    if (!m_flightstatus->parking_brake_set && m_flightstatus->onground)
                        fmcControl().startPushBack(m_pushback_dist_before_turn_m,
                                                   m_pushback_turn_direction_clockwise,
                                                   m_pushback_turn_degrees,
                                                   m_pushback_dist_after_turn_m);
                }
                else
                {
                    fmcControl().stopPushBack();
                }
                break;
            }
        }

        m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
    }
    else if (m_selected_system == SYSTEM_CHECKLIST)
    {
        //----- process key input

        if (llsk_index == 6) 
        {
            if (fmcControl().checklistManager().decChecklistItemIndex()) m_vertical_scroll_offset = 0;
        }
        else if (rlsk_index == 6)
        {
            if (fmcControl().checklistManager().incChecklistItemIndex()) m_vertical_scroll_offset = 0;
        }

        //----- correct scroll if current checklist item is out of view

        slotSetVerticalScrollOffsetForChecklist();
    }
    else if (m_selected_system == SYSTEM_DISPLAY1)
    {
        bool convok = false;
        uint value = text.toUInt(&convok);

        switch(llsk_index)
        {
            case(1): fmcControl().setGLFontSize(fmcControl().glFontIndex()-1); break;
            case(2): fmcControl().decFCUFontSize(); break;
            case(3): fmcControl().decCDUFontSize(); break;

            case(4): {
                if (!convok)
                {
                    m_page_manager->scratchpad().setOverrideText("INVALID VALUE");
                    return;
                }

                fmcControl().setPFDNDRefreshRateMs(value);
                m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
                break;
            }

            case(5): {
                if (!convok)
                {
                    m_page_manager->scratchpad().setOverrideText("INVALID VALUE");
                    return;
                }

                fmcControl().setECAMRefreshRateMs(value);
                m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
                break;
            }

            case(6): {
                if (!convok)
                {
                    m_page_manager->scratchpad().setOverrideText("INVALID VALUE");
                    return;
                }

                fmcControl().setCDUFCURefreshRateMs(value);
                m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
                break;
            }
        }
         
        switch(rlsk_index)
        {
            case(1): fmcControl().setGLFontSize(fmcControl().glFontIndex()+1); break;
            case(2): fmcControl().incFCUFontSize(); break;
            case(3): fmcControl().incCDUFontSize(); break;
            case(4): fmcControl().setShowFps(!fmcControl().showFps()); break;
        }
    }
    else if (m_selected_system == SYSTEM_DISPLAY2)
    {
        switch(llsk_index)
        {
            case(1): {
                (fmcControl().ndNormalScrollMode()) ?
                    fmcControl().setNdScrollModeInverse() : fmcControl().setNdScrollModeNormal();
                break;
            }

            case(2): fmcControl().toggleNdWindCorrection(); break;
#if !VASFMC_GAUGE
            case(3): fmcControl().slotToggleFcuLeftOnlyMode(); break;
#endif
        }

        switch(rlsk_index)
        {
            case(1): {
                (fmcControl().cduNormalScrollMode()) ?
                    fmcControl().setCduScrollModeInverse() : fmcControl().setCduScrollModeNormal();
                break;
            }

#if !VASFMC_GAUGE
            case(2): fmcControl().slotToggleCduDisplayOnlyMode(false); break;
            case(3): fmcControl().setKeepOnTop(!fmcControl().doKeepOnTop()); break;
#endif
        }
    }
    else if (m_selected_system == SYSTEM_INTERFACES)
    {
        bool convok = false;
        int value = text.toInt(&convok);

        switch(llsk_index)
        {
            case(1): {
                fmcControl().switchToNextFMCConnectMode();
                break;
            }
            case(2): {

                if (text.isEmpty() || fmcControl().isFMCConnectModeSingle()) return;

                if (!convok || value <= 0 || value > 65535)
                {
                    m_page_manager->scratchpad().setOverrideText("INVALID VALUE");
                    return;
                }

                if (fmcControl().isFMCConnectModeSlave())
                    fmcControl().setFMCConnectModeSlaveMasterPort(value);
                else if (fmcControl().isFMCConnectModeMaster())
                    fmcControl().setFMCConnectModeMasterPort(value);

                m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
                break;
            }
            case(3): {

                if (text.isEmpty() || !fmcControl().isFMCConnectModeSlave()) return;

                QHostAddress ip(text);

                if (ip.protocol() != QAbstractSocket::IPv4Protocol)
                {
                    m_page_manager->scratchpad().setOverrideText("INVALID IP");
                    return;
                }

                fmcControl().setFMCConnectModeSlaveMasterIP(ip.toString());

                m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
                break;
            }
        }

        switch(rlsk_index)
        {
            case(1): fmcControl().slotToggleUseIOCPServer(); break;
#ifdef SERIAL
            case(2): fmcControl().slotToggleUseCPFlight(); break;
#endif
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAMenu::setActive(bool active)
{
    FMCCDUPageBase::setActive(active);
    if (!active) m_selected_system.clear();
    if (fmcControl().temporaryRoute().count() > 0) fmcControl().temporaryRoute().clear();
    setDrawHorizontalScrollPageCounter(true);
    m_aircraft_data_load_page = false;
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAMenu::clearICAORoute()
{
    delete m_icao_route;
    m_icao_route = 0;
    fmcControl().temporaryRoute().clear();
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAMenu::slotGotWeather(const QString& airport, const QString& date_string, const QString& weather_string)
{
    m_vertical_scroll_offset = 0;
    m_weather_airport = airport;
    m_weather_date = date_string;

    //Logger::log(weather_string);
    
    QString text = weather_string.simplified();
    while(text.length() > 0)
    {
	    QString cut_text = text.left(m_max_columns);
        m_weather_text_list.append(cut_text);
        text = text.mid(cut_text.length());
    }

    if (m_active) m_page_manager->scratchpad().setOverrideText(QString::null);
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAMenu::slotGotWeatherError(const QString& error_string)
{
    Logger::log(QString("FMCCDUPageStyleAMenu:slotGotWeatherError: act=%1 (%2)").arg(m_active).arg(error_string));
    m_weather_date.clear();
    m_weather_text_list.clear();
    m_vertical_scroll_offset = 0;
    if (m_active) m_page_manager->scratchpad().setOverrideText(error_string);
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAMenu::slotGotRoute(const CompactRouteList& compact_route_list)
{
    m_vertical_scroll_offset = 0;
    m_selected_compact_route_index = 0;
    m_compact_route_list = compact_route_list;
    m_route_text_list.clear();

    if (m_compact_route_list.count() == 0)
    {
        m_page_manager->scratchpad().setOverrideText("No FPs found");        
        return;
    }

    selectCompactRoute(0);
    
    if (m_active) m_page_manager->scratchpad().setOverrideText(QString::null);
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAMenu::slotGotRouteError(const QString& error_string)
{
    Logger::log(QString("FMCCDUPageStyleAMenu:slotGotRouteError: act=%1 (%2)").arg(m_active).arg(error_string));
    m_vertical_scroll_offset = 0;
    m_selected_compact_route_index = 0;
    m_route_text_list.clear();
    m_compact_route_list.clear();
    if (m_active) m_page_manager->scratchpad().setOverrideText(error_string);
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAMenu::selectCompactRoute(uint index)
{
    m_route_text_list.clear();
    m_selected_compact_route_index = 0;

    if ((int)index >= m_compact_route_list.count()) return;
    m_selected_compact_route_index = index;

    QString text = m_compact_route_list[m_selected_compact_route_index].m_route.simplified();
    while(text.length() > 0)
    {
	    QString cut_text = text.left(m_max_columns);
        m_route_text_list.append(cut_text);
        text = text.mid(cut_text.length());
    }
}

/////////////////////////////////////////////////////////////////////////////

uint FMCCDUPageStyleAMenu::maxVerticalScrollOffset() const 
{
    if (m_selected_system == SYSTEM_WEATHER) return qMax(0, m_weather_text_list.count() - 7); 
    if (m_selected_system == SYSTEM_VROUTE)  return qMax(0, m_route_text_list.count() - 3);

    if (m_selected_system == SYSTEM_ICAORTE) 
    {
        if (m_icao_route == 0) return 0;
        else return qMax(0, (m_icao_route->count()/2)-5);
    }

    if (m_selected_system == SYSTEM_FMCS && m_aircraft_data_load_page)
    {
        return qMax(0, m_aircraft_data_file_list.count() - 5);
    }

    if (m_selected_system == SYSTEM_CHECKLIST)
    {
        return qMax(0, fmcControl().checklistManager().currentChecklist().count() - MAX_CHECKLIST_ITEMS_PER_PAGE);
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////

uint FMCCDUPageStyleAMenu::maxHorizontalScrollOffset() const 
{
    if (m_selected_system == SYSTEM_VROUTE) return qMax(0, m_compact_route_list.count()-1);
    if (m_selected_system == SYSTEM_CHECKLIST) return qMax(0, fmcControl().checklistManager().count()-1);
    return 0;
}

// End of file
