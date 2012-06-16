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

/*! \file    fmc_cdu_page.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "airport.h"
#include "ndb.h"
#include "flightstatus.h"
#include "navdata.h"
#include "navcalc.h"
#include "fsaccess.h"
#include "flight_mode_tracker.h"

#include "fmc_autopilot.h"
#include "fmc_data.h"
#include "fmc_control.h"

#include "fmc_cdu_defines.h"
#include "fmc_cdu_page_manager.h"
#include "fmc_cdu_page.h"

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleATest::paintPage(QPainter& painter) const
{
    painter.setBackground(QBrush(BLACK));
    painter.fillRect(painter.window(), painter.background());

    setFont(painter, SMALL_FONT);
    for(uint line=2; line < m_max_lines; line+=2)
    {
        drawTextLeft(painter, 2, line, QString("LABEL%1").arg(line-1));
        drawTextRight(painter, 2, line, QString("LABEL%1").arg(line-1));
    }

    setFont(painter, NORM_FONT);
    for(uint line=3; line < m_max_lines; line+=2)
    {
        drawTextLeft(painter, 1, line, QString("<LEFT %1").arg(line-1), GREEN);
        drawTextRight(painter, 1, line, QString("RIGHT %1>").arg(line-1), GREEN);
    }

    QString title = "Y";
    for(uint col=1; col <= m_max_columns-2; ++col) title += "X";
    title += "Y";
    drawTextLeft(painter, 1, 1, title);

    //drawTextCenter(painter, 1, "TITLE FIELD");
    //drawTextLeft(painter, 1, m_max_lines, "SCRATCHPAD FIELD");

//    unsigned char c = 0;
//     for(uint line=1; line < m_max_lines; ++line)
//         for(uint col=1; col <= m_max_columns; ++col)
//         {
//             //drawTextLeft(painter, 1+col, line, QString('A'+col));
//             //drawTextLeft(painter, col, line, QString::number(col%10));
//             drawTextLeft(painter, col, line, "A");
// //             drawTextLeft(painter, col, line, QChar(c));
// //             c++;
//         }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAScratchpad::paintPage(QPainter& painter) const
{
    setFont(painter, NORM_FONT);

    if (!m_override_text.isEmpty()) 
    {
        drawTextLeft(painter,  1, 14, m_override_text, ORANGE);
    }
    else if (!m_action.isEmpty())   
    {
        if (m_action == ACTION_CLR)          drawTextLeft(painter,  1, 14, "CLR");
        else if (m_action == ACTION_OVERFLY) drawTextLeft(painter,  1, 14, "*");
        else 
        {MYASSERT(false);}
    }
    else
    {                            
        drawTextLeft(painter,  1, 14, m_text);
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAScratchpad::processAction(const QString& action)
{
    // if we have an override text, the user must clear the text at first
    if (!m_override_text.isEmpty() && action != ACTION_CLR) return;

    if (action == ACTION_CLR)
    {
        if (!m_override_text.isEmpty())  
        {
            m_action.clear();
            m_override_text = QString::null;
        }
        else if (m_action == ACTION_CLR) m_action = QString::null;
        else if (m_text.isEmpty())       m_action = ACTION_CLR;
        else                             m_text = m_text.left(m_text.length() - 1);
        return;
    }    
    else if (action == ACTION_CLRALL)
    {
        m_text.clear();
        m_override_text.clear();
        m_action.clear();
        return;
    }

    if (m_action == ACTION_CLR) return;

    if (action == ACTION_OVERFLY)
    {
        m_text.clear();
        if (m_action == ACTION_OVERFLY)  m_action.clear();
        else                             m_action = action;
    }
    
    if (m_text.length() > (int)m_max_columns) return;

    if (action == ACTION_CLIPBOARD) 
    {
        m_action.clear();
        m_text += qApp->clipboard()->text();
    }
    else if (action == ACTION_PLUSMINUS)
    {
        m_action.clear();
        if (m_text.endsWith("+")) 
        {
            m_text.chop(1);
            m_text += "-";
        }
        else if (m_text.endsWith("-")) 
        {
            m_text.chop(1);
            m_text += "+";
        }
        else
        {
            m_text += "-";
        }
    }
    else if (action.length() == 1)
    {
        m_action.clear();
        m_text += action;
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAWaypointSelectBase::setWptSelectPage(const WaypointPtrList& wpt_selection_list, uint wpt_insert_index,
                                                    double pbd_bearing, double pbd_distance_nm)
{
    MYASSERT(wpt_selection_list.count() > 0);
    delete m_wpt_selection_list;
    m_wpt_selection_list = wpt_selection_list.deepCopy();

    m_wpt_insert_index = wpt_insert_index;
    MYASSERT(m_wpt_insert_index <= (int)fmcControl().normalRoute().count());

    m_pbd_bearing = pbd_bearing;
    m_pbd_distance_nm = pbd_distance_nm;
    
    m_wpt_select_page = m_page_manager->wptSelectPage();
    MYASSERT(m_wpt_select_page != 0);
    m_wpt_select_page->setForSelection(m_wpt_selection_list, this, ACTION_CALLBACK_WPT_SELECT);
    
    MYASSERT(m_page_manager->setInterimPage(m_wpt_select_page->name()));
    m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAWaypointSelectBase::processWptSelectCallback()
{
    if (m_wpt_selection_list == 0 || m_wpt_select_page == 0) return;
    
    int wpt_selection_index = m_wpt_select_page->selectedIndex();
    if (wpt_selection_index >= 0 && wpt_selection_index < m_wpt_selection_list->count())
    {
        Waypoint* wpt = m_wpt_selection_list->at(wpt_selection_index);

        if (m_pbd_distance_nm > 0.0)
        {
            *wpt = fmcControl().getPBDWaypoint(*wpt, m_pbd_bearing, m_pbd_distance_nm);
        }

        wptSelectCallback(*wpt, m_wpt_insert_index);
    }
    
    // clear the wpt selection stuff
    m_wpt_select_page = 0;
    delete m_wpt_selection_list;
    m_wpt_selection_list = 0;
    m_wpt_insert_index = 0;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

FMCCDUPageStyleAWaypoint::FMCCDUPageStyleAWaypoint(const QString& page_name, FMCCDUPageManager* page_manager) : 
    FMCCDUPageStyleAWaypointSelectBase(page_name, page_manager), m_wpt(0), m_wpt_route_index(0),
    m_is_departure_airport(false), m_is_destination_airport(false)
{
    m_data_change_page_name_to_switch = FMCCDUPageManagerStyleA::PAGE_FPLAN;
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAWaypoint::paintPage(QPainter& painter) const
{
    if (m_wpt == 0) return;

    painter.setBackground(QBrush(BLACK));
    painter.fillRect(painter.window(), painter.background());

    setFont(painter, NORM_FONT, QFont::Bold);
    //TODO write waypoint id in green!?
    drawTextCenter(painter, 1, QString("LAT REV FROM %1").arg(m_wpt->id()));

    setFont(painter, 0.70);

    if (!m_wpt->isDependendWaypoint())
    {
        drawTextCenter(painter, 2, QString("%1/%2").arg(m_wpt->latStringDegMinSec()).arg(m_wpt->lonStringDegMinSec()), GREEN);

        if (m_wpt->type() != Waypoint::TYPE_WAYPOINT)
        {
            QString name = QString("%1/%2").arg(m_wpt->type()).arg(m_wpt->name().left(18));
            if (m_wpt->name().length() > 18) name += "...";
            drawTextCenter(painter, 3, name);
        }
    }

    if (m_is_departure_airport || m_is_destination_airport)
    {
        setFont(painter, NORM_FONT);
        if (m_is_departure_airport)   drawTextLeft(painter, 1, 5, "< DEPARTURE");
        if (m_is_destination_airport) drawTextRight(painter, 1, 5, "ARRIVAL >");
    }
    else
    {
        setFont(painter, SMALL_FONT);
        drawTextRight(painter, 2,  6, "VIA/GO TO");
        
        setFont(painter, NORM_FONT);
        drawTextLeft(painter,  1,  7, "< HOLD");
        drawTextRight(painter,  1,  7, "[   ]/[     ]", CYAN);
    }

    setFont(painter, SMALL_FONT);
    drawTextRight(painter, 2,  8, "NEXT WPT");
    if (!m_is_destination_airport) drawTextRight(painter, 2,  10, "NEW DEST");

    setFont(painter, NORM_FONT);
    
    drawTextRight(painter,  1, 9, "[     ]", CYAN);
    if (!m_is_destination_airport) drawTextRight(painter,  1, 11, "[     ]", CYAN);
    drawTextLeft(painter,  1, 13, "< RETURN");        
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAWaypoint::processAction(const QString& action)
{
    if (m_wpt == 0) return;

    if (action == ACTION_CALLBACK_WPT_SELECT)
    {
        processWptSelectCallback();
        return;
    }
    else if (action == ACTION_UP)
    {
        advanceVerticalScroll(+1);
        if ((int)m_wpt_route_index != m_vertical_scroll_offset) setWaypoint(m_vertical_scroll_offset);
        return;
    }
    else if (action == ACTION_DOWN)
    {
        advanceVerticalScroll(-1);
        if ((int)m_wpt_route_index != m_vertical_scroll_offset) setWaypoint(m_vertical_scroll_offset);
        return;
    }

    int llsk_index = -1;
    int rlsk_index = -1;
    
    if (action.startsWith("LLSK"))
    {
        bool convok = false;
        llsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);
    }
    else if (action.startsWith("RLSK"))
    {
        bool convok = false;
        rlsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);
    }
    else
    {
        return;
    }

    QString text = m_page_manager->scratchpad().text();

    if (llsk_index == 2 && m_is_departure_airport)
    {
        FMCCDUPageStyleADepartureSelect* departure_select_page = m_page_manager->departureSelectPage();
        MYASSERT(departure_select_page != 0);
        departure_select_page->setForSelection(m_wpt_route_index, m_wpt->asAirport(), FMCCDUPageManagerStyleA::PAGE_FPLAN);
        m_page_manager->setCurrentPage(departure_select_page->name());
    }
    else if (rlsk_index == 2 && m_is_destination_airport)
    {
        FMCCDUPageStyleADestinationSelect* destination_select_page = m_page_manager->destinationSelectPage();
        MYASSERT(destination_select_page != 0);
        destination_select_page->setForSelection(m_wpt_route_index, m_wpt->asAirport(), FMCCDUPageManagerStyleA::PAGE_FPLAN);
        m_page_manager->setCurrentPage(destination_select_page->name());
    }
    else if (rlsk_index == 3 && m_wpt->asAirport() == 0)
    {
        if (text.isEmpty()) return;
        QStringList items = text.split('/');
        if (items.count() != 2)
        {
            m_page_manager->scratchpad().setOverrideText("Invalid Format");
            return;
        }

        QString error_text;
        WaypointPtrList wpt_list;
        if (!fmcControl().getWaypointsByAirway(
                *m_wpt, items[0].trimmed(), items[1].trimmed(), wpt_list, error_text))
        {
            m_page_manager->scratchpad().setOverrideText(error_text);
            return;
        }

        uint insert_index = m_wpt_route_index;
        WaypointPtrListIterator iter(wpt_list);
        while(iter.hasNext())
        {
            const Waypoint* wpt = iter.next();
            fmcControl().normalRoute().insertWaypoint(*wpt, ++insert_index);
        }

        FMCCDUPageStyleAFlightplan* fplan_page = m_page_manager->flightPlanPage();
        MYASSERT(fplan_page != 0);
        fplan_page->presetScrollOffset(insert_index-2);
        m_page_manager->setCurrentPage(fplan_page->name());
        m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
    }
    else if (llsk_index == 3 && !m_is_departure_airport && !m_is_destination_airport)
    {
        FMCCDUPageStyleAHolding* hold_page = m_page_manager->holdingPage();
        MYASSERT(hold_page != 0);
        hold_page->setWaypoint(m_wpt_route_index);
        m_page_manager->setInterimPage(hold_page->name());
    }
    else if (rlsk_index == 4)
    {
        if (text.isEmpty()) return;

        QString wpt_name;
        double pbd_bearing = 0.0;
        double pbd_distance_nm = 0.0;
        bool got_pbd_wpt = parsePBDSyntax(text, wpt_name, pbd_bearing, pbd_distance_nm);
        if (!got_pbd_wpt) wpt_name = text;

        //----- look if we find the waypoint in the FP and delete all waypoints in between

        bool found_wpt = false;
        for(int index = m_wpt_route_index+1; index < fmcControl().normalRoute().count(); ++index)
        {
            const Waypoint*existing_wpt = fmcControl().normalRoute().waypoint(index);
            MYASSERT(existing_wpt != 0);

            if (existing_wpt->id() == wpt_name)
            {
                found_wpt = true;

                int nr_wpts_to_remove = index - (m_wpt_route_index+1);
                while(nr_wpts_to_remove > 0) 
                {
                    fmcControl().normalRoute().removeWaypoint(m_wpt_route_index+1);
                    --nr_wpts_to_remove;
                }

                FMCCDUPageStyleAFlightplan* fplan_page = m_page_manager->flightPlanPage();
                MYASSERT(fplan_page != 0);
                fplan_page->presetScrollOffset(m_wpt_route_index-1);
                m_page_manager->setCurrentPage(fplan_page->name());
                m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
            }
        }

        //----- if we did not find the waypoint, we insert it

        if (!found_wpt)
        {
            WaypointPtrList wpt_selection_list;
            fmcControl().navdata().getWaypoints(wpt_name, wpt_selection_list, LATLON_WAYPOINT_REGEXP);
        
            if (wpt_selection_list.count() <= 0)
            {
                m_page_manager->scratchpad().setOverrideText("No Waypoint found");
                return;
            }
            else if (wpt_selection_list.count() == 1)
            {
                Waypoint* wpt = wpt_selection_list.at(0);
                if (got_pbd_wpt) *wpt = fmcControl().getPBDWaypoint(*wpt, pbd_bearing, pbd_distance_nm);

                fmcControl().normalRoute().insertWaypoint(*wpt, m_wpt_route_index+1);
                FMCCDUPageStyleAFlightplan* fplan_page = m_page_manager->flightPlanPage();
                MYASSERT(fplan_page != 0);
                fplan_page->presetScrollOffset(m_wpt_route_index-1);
                m_page_manager->setCurrentPage(fplan_page->name());
                m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
            }
            else
            {
                wpt_selection_list.sortByDistance(m_flightstatus->current_position_raw);
                setWptSelectPage(wpt_selection_list, m_wpt_route_index+1, pbd_bearing, pbd_distance_nm);
                m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
            }
        }
    }
    else if (rlsk_index == 5 && !m_is_destination_airport)
    {
        if (text.isEmpty()) return;
        
        WaypointPtrList result_list;
        fmcControl().navdata().getAirports(text, result_list);
        if (result_list.count() != 1)
        {
            m_page_manager->scratchpad().setOverrideText("Airport not found");
            return;
        }

        for(int index = m_wpt_route_index+1; index < fmcControl().normalRoute().count(); ++index)
            fmcControl().normalRoute().removeWaypoint(index);

        fmcControl().normalRoute().insertWaypoint(*result_list.at(0), m_wpt_route_index+1);
        fmcControl().normalRoute().setAsDestinationAirport(m_wpt_route_index+1, QString::null);

        while(fmcControl().normalRoute().count() > (int)m_wpt_route_index+2)
            fmcControl().normalRoute().removeWaypoint(m_wpt_route_index+2);

        FMCCDUPageStyleAFlightplan* fplan_page = m_page_manager->flightPlanPage();
        MYASSERT(fplan_page != 0);
        fplan_page->presetScrollOffset(fmcControl().normalRoute().destinationAirportIndex()-2);
        m_page_manager->setCurrentPage(fplan_page->name());

        m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
    }
    else if (llsk_index == 6)
    {
        FMCCDUPageStyleAFlightplan* fplan_page = m_page_manager->flightPlanPage();
        MYASSERT(fplan_page != 0);
        fplan_page->presetScrollOffset(m_wpt_route_index-3);
        m_page_manager->setCurrentPage(fplan_page->name());

        m_wpt = 0;
        m_wpt_route_index = 0;
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAWaypoint::setWaypoint(uint wpt_route_index) 
{
    m_is_departure_airport = false;
    m_is_destination_airport = false;
    
    m_wpt_route_index = wpt_route_index;
    MYASSERT((int)wpt_route_index < fmcControl().normalRoute().count());
    m_wpt = fmcControl().normalRoute().waypoint(m_wpt_route_index);
    MYASSERT(m_wpt != 0);
    
    m_is_departure_airport = fmcControl().normalRoute().departureAirport() == m_wpt;
    m_is_destination_airport = fmcControl().normalRoute().destinationAirport() == m_wpt;
    
    MYASSERT(!(m_is_destination_airport && m_is_departure_airport));
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAWaypoint::setActive(bool active)
{
    FMCCDUPageBase::setActive(active);
    if (active) m_vertical_scroll_offset = m_wpt_route_index;
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAWaypoint::wptSelectCallback(const Waypoint& wpt, uint wpt_insert_index)
{
    Logger::log(QString("FMCCDUPageStyleAWaypoint:wptSelectCallback: selection=%1 wpt=%2").
                arg(wpt_insert_index).arg(wpt.id()));
    
    fmcControl().normalRoute().insertWaypoint(wpt, wpt_insert_index);
    FMCCDUPageStyleAFlightplan* fplan_page = m_page_manager->flightPlanPage();
    MYASSERT(fplan_page != 0);
    fplan_page->presetScrollOffset(wpt_insert_index-3);
    m_page_manager->setCurrentPage(fplan_page->name());
}

/////////////////////////////////////////////////////////////////////////////

uint FMCCDUPageStyleAWaypoint::maxVerticalScrollOffset() const
{
    return qMax(0, fmcControl().normalRoute().count() - 1);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

FMCCDUPageStyleAHolding::FMCCDUPageStyleAHolding(const QString& page_name, FMCCDUPageManager* page_manager) : 
    FMCCDUPageBase(page_name, page_manager), m_wpt_route_index(0), m_changes_made(false)
{
    m_data_change_page_name_to_switch = FMCCDUPageManagerStyleA::PAGE_FPLAN;
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAHolding::paintPage(QPainter& painter) const
{
    painter.setBackground(QBrush(BLACK));
    painter.fillRect(painter.window(), painter.background());

    setFont(painter, NORM_FONT, QFont::Bold);
    //TODO write waypoint id in green!?
    drawTextCenter(painter, 1, QString("HOLD AT %1").arg(m_wpt_id));

    setFont(painter, SMALL_FONT);
    drawTextLeft(painter, 1, 2, "INB CRS");
    drawTextLeft(painter, 1, 4, "TURN");
    drawTextLeft(painter, 1, 6, "TIME");
    
    setFont(painter, NORM_FONT);

    QColor color = CYAN;
    if (m_changes_made) color = YELLOW;

    if (!m_holding.isValid())
    {
        drawTextLeft(painter, 1, 3, "[   ]°", CYAN);
        drawTextLeft(painter, 1, 5, "[ ]", CYAN);
        drawTextLeft(painter, 1, 7, "1.0", CYAN);
    }
    else
    {
        drawTextLeft(painter, 1, 3, QString("%1°").arg(m_holding.holdingTrack()), color);
        drawTextLeft(painter, 1, 5, m_holding.holdTurnDirText(), color);
        drawTextLeft(painter, 1, 7, QString("%1").arg(m_holding.holdLegLengthMin(), 0, 'f', 1), color);
    }

    if (wptHolding().isValid() && !wptHolding().isActive())
        drawTextRight(painter,  1, 3, "REMOVE >", ORANGE);

    if (!m_changes_made || wptHolding().isActive())
    {
        drawTextLeft(painter,  1, 13, "< RETURN");
    }
    else
    {
        drawTextLeft(painter,  1, 13, "< ERASE", ORANGE);
        drawTextRight(painter, 1, 13, "INSERT*", ORANGE);
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAHolding::processAction(const QString& action)
{
    int llsk_index = -1;
    int rlsk_index = -1;
    
    if (action.startsWith("LLSK"))
    {
        bool convok = false;
        llsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);
    }
    else if (action.startsWith("RLSK"))
    {
        bool convok = false;
        rlsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);
    }
    else
    {
        return;
    }

    QString text = m_page_manager->scratchpad().text();
    bool convok = false;

    if (wptHolding().isActive() && llsk_index >= 1 && llsk_index <= 3 && !text.isEmpty())
    {
        m_page_manager->scratchpad().setOverrideText("Holding is active");
        return;
    }
    else if (llsk_index == 1 && !text.isEmpty())
    {
        uint course = text.toUInt(&convok);
        if (!convok || course > 360) 
        {
            m_page_manager->scratchpad().setOverrideText("Invalid course");
            return;
        }

        m_holding.setHoldingTrack(course);
        m_changes_made = true;
        m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
    }
    else if (llsk_index == 2 && !text.isEmpty())
    {
        if (text != "R" && text != "L") 
        {
            m_page_manager->scratchpad().setOverrideText("Invalid turn direction");
            return;
        }
        
        m_holding.setIsLeftHolding(text == "L" ? true : false);
        m_changes_made = true;
        m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
    }
    else if (llsk_index == 3 && !text.isEmpty())
    {
        double time = text.toDouble(&convok);
        if (!convok || time <= 0.1 || time > 10.0)
        {
            m_page_manager->scratchpad().setOverrideText("Invalid holding time");
            return;
        }

        m_holding.setHoldLegLengthMin(time);
        m_changes_made = true;
        m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
    }
    else if (rlsk_index == 1 && wptHolding().isValid() && !wptHolding().isActive())
    {
        m_holding = Holding();
        m_changes_made = true;
    }
    else if (llsk_index == 6)
    {
        if (!m_changes_made || wptHolding().isActive())
        {
            m_wpt_route_index = 0;
            m_page_manager->setInterimPage(QString::null);
        }
        else
        {
            MYASSERT(fmcControl().normalRoute().waypoint(m_wpt_route_index) != 0);
            m_holding = wptHolding();
            m_changes_made = false;
        }
    }
    else if (rlsk_index == 6 && m_changes_made && !wptHolding().isActive())
    {
        MYASSERT(fmcControl().normalRoute().setHolding(m_wpt_route_index, m_holding));
        m_changes_made = false;
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAHolding::setWaypoint(uint wpt_route_index)
{
    m_wpt_route_index = wpt_route_index;
    MYASSERT(fmcControl().normalRoute().waypoint(wpt_route_index) != 0);
    m_wpt_id = fmcControl().normalRoute().waypoint(wpt_route_index)->id();
    m_holding = wptHolding();
    m_changes_made = false;
}

/////////////////////////////////////////////////////////////////////////////

const Holding& FMCCDUPageStyleAHolding::wptHolding() const
{
    MYASSERT(fmcControl().normalRoute().waypoint(m_wpt_route_index) != 0);
    return fmcControl().normalRoute().waypoint(m_wpt_route_index)->holding();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAFlightplan::paintPage(QPainter& painter) const
{
    painter.setBackground(QBrush(BLACK));
    painter.fillRect(painter.window(), painter.background());

    setFont(painter, SMALL_FONT);
    drawTextLeft(painter,  2, 1, "FROM");

    if (!fmcControl().normalRoute().flightNumber().isEmpty())
        drawTextRight(painter, 1, 1, fmcControl().normalRoute().flightNumber());

    drawTextLeft(painter,  9, 2, "UTC");
    drawTextRight(painter, 5, 2, "SPD/ALT");

    drawTextLeft(painter,  2, 12, "DEST");
    drawTextLeft(painter, 9, 12, "UTC");
    drawTextLeft(painter, 15, 12, "DIST");
    drawTextRight(painter, 2, 12, "EFOB");

    const Waypoint* active_wpt = fmcControl().normalRoute().activeWaypoint();
    m_display_line_counter = 3;

    int index = getWaypointIndexByLSK(1);

    const Airport *adep = fmcControl().normalRoute().departureAirport();
    const Airport *ades = fmcControl().normalRoute().destinationAirport();
    int prev_wpt_index = fmcControl().normalRoute().previousWaypointIndex();

    for(; index < fmcControl().normalRoute().count() ; ++index)
    {
        if (m_display_line_counter > 12) break;

        bool is_overflown_waypoint = index < (int)fmcControl().normalRoute().activeWaypointIndex();

        const Waypoint* wpt = fmcControl().normalRoute().waypoint(index);
        if (wpt == 0)
        {
            m_vertical_scroll_offset = 0;
            return;
        }

        const RouteData& route_data = fmcControl().normalRoute().routeData(index);
        
        QColor color = GREEN;
        if (wpt == active_wpt) color = WHITE;
        if (index > fmcControl().normalRoute().destinationAirportIndex() &&
            fmcControl().normalRoute().destinationAirportIndex() >= 
            (int)fmcControl().normalRoute().activeWaypointIndex()) color = CYAN;

        if (m_display_line_counter > 3)
        {
            setFont(painter, SMALL_FONT);

            if (wpt->holding().isValid())
                drawTextLeft(painter, 2, m_display_line_counter-1, QString("HOLD %1").
                             arg(wpt->holding().holdTurnDirText()));
            else if (!wpt->parent().isEmpty())
                drawTextLeft(painter, 2, m_display_line_counter-1, wpt->parent());
        }

        setFont(painter, NORM_FONT);

        QString wptname = wpt->id();
        if (wpt->restrictions().hasOverflyRestriction()) wptname += "*";
        drawTextLeft(painter, 1, m_display_line_counter, wptname, color);

        setFont(painter, SMALL_FONT);
        
        if (m_flightstatus->ground_speed_kts < 10 ||
            !route_data.m_time_over_waypoint.isValid())
        {
            drawTextLeft(painter, 9,m_display_line_counter, "----", color);
        }
        else
        {
            drawTextLeft(painter, 9,m_display_line_counter, 
                         route_data.m_time_over_waypoint.toString("hhmm"), color);
        }

        if (m_display_line_counter > 3)
        {
            if (wpt == active_wpt)
            {
                drawTextLeft(painter, 9,m_display_line_counter-1, QString("BRG%1").
                             arg(Navcalc::trimHeading(m_fmc_data.trueTrackToActiveWpt() -
                                                      m_flightstatus->magvar), 3, 'f', 0, QChar('0')), color);

                drawTextRight(painter, 6, m_display_line_counter-1, 
                              QString("%1").arg(m_fmc_data.distanceToActiveWptNm(), 0, 'f', 0), color);
            }
            else
            {
                if (m_display_line_counter <= 7)
                    drawTextLeft(painter, 9,m_display_line_counter-1, QString("TRK%1").
                                 arg(Navcalc::trimHeading(route_data.m_true_track_from_prev_wpt -
                                                          m_flightstatus->magvar), 3, 'f', 0, QChar('0')), color);
                
                drawTextRight(painter, 6, m_display_line_counter-1, 
                              QString("%1").arg(route_data.m_dist_from_prev_wpt_nm, 0, 'f', 0), color);
            }
        }

        // draw speed/alt restrictions, holding exit
        
        setFont(painter, NORM_FONT);
        if (index > prev_wpt_index && wpt->holding().isValid() && wpt->holding().isActive())
        {
            if (!wpt->holding().exitHolding())
                drawTextRight(painter, 1, m_display_line_counter, "EXIT*", ORANGE);
            else
                drawTextRight(painter, 1, m_display_line_counter, "RESUME*", ORANGE);
        }
        else
        {
            if (wpt->asAirport() != 0 && (wpt == adep || wpt == ades))
            {
                setFont(painter, NORM_FONT);
                drawTextRight(painter, 7, m_display_line_counter, "---/", color);
                drawTextRight(painter, 1, m_display_line_counter, 
                              QString("%1").arg(wpt->asAirport()->elevationFt()), MAGENTA);
            }
            else
            {
                drawTextRight(painter, 7, m_display_line_counter, "/", GREEN);

                if (is_overflown_waypoint)
                {
                    setFont(painter, NORM_FONT);
                    drawTextRight(painter, 8, m_display_line_counter, 
                                  QString::number(wpt->overflownData().speedKts()), color);
                    drawTextRight(painter, 1, m_display_line_counter, 
                                  QString::number(wpt->overflownData().altitudeFt()), color);                    
                }
                else
                {
                    // spd restriction

                    if (wpt->restrictions().speedRestrictionKts() == 0) 
                    {
                        setFont(painter, SMALL_FONT);
                        drawTextRight(painter, 8, m_display_line_counter, "---", color);
                    }
                    else
                    {
                        setFont(painter, NORM_FONT);
                        drawTextRight(painter, 8, m_display_line_counter, 
                                      QString::number(wpt->restrictions().speedRestrictionKts()), MAGENTA);
                    }

                    // alt restriction
                    
                    if (wpt->restrictions().altitudeRestrictionFt() == 0)
                    {
                        setFont(painter, SMALL_FONT);
                        drawTextRight(painter, 1, m_display_line_counter, "-----", color);
                    }
                    else
                    {
                        setFont(painter, NORM_FONT);
                        QString alt_string;
                        if (wpt->restrictions().altitudeSmallerRestriction()) alt_string = "-";
                        else if (wpt->restrictions().altitudeGreaterRestriction()) alt_string = "+";
                        alt_string += QString::number(wpt->restrictions().altitudeRestrictionFt());
                        drawTextRight(painter, 1, m_display_line_counter, alt_string, MAGENTA);
                    }
                }
            }
        }

        m_display_line_counter += 2;
    }

    if (m_display_line_counter < 12)
    {
        setFont(painter, NORM_FONT);
        drawTextCenter(painter, m_display_line_counter, "------END OF F-PLN------");
        m_display_line_counter += 2;
    }

    if (m_display_line_counter < 12)
    {
        for(int index = 0; index < fmcControl().alternateRoute().count() ; ++index)
        {
            if (m_display_line_counter > 12) break;

            const Waypoint* wpt = fmcControl().alternateRoute().waypoint(index);
            if (wpt == 0) break;
            const RouteData& route_data = fmcControl().alternateRoute().routeData(index);

            QColor color = CYAN;

            if (m_display_line_counter > 3)
            {
                setFont(painter, SMALL_FONT);
                if (!wpt->parent().isEmpty())
                    drawTextLeft(painter, 2, m_display_line_counter-1, wpt->parent(), WHITE);
            }

            setFont(painter, NORM_FONT);
            QString wptname = wpt->id();
            if (wpt->restrictions().hasOverflyRestriction()) wptname += "*";
            drawTextLeft(painter, 1, m_display_line_counter, wptname, color);
        
            if (index > 0)
            {
                setFont(painter, SMALL_FONT);
                drawTextLeft(painter, 9, m_display_line_counter, "----", color);
                drawTextRight(painter, 6, m_display_line_counter-1, 
                              QString("%1").arg(route_data.m_dist_from_prev_wpt_nm, 0, 'f', 0), color);
            }

            // draw speed/alt restrictions

            setFont(painter, NORM_FONT);
            if (wpt == fmcControl().alternateRoute().destinationAirport())
            {
                drawTextRight(painter, 7, m_display_line_counter, "---/", color);
                drawTextRight(painter, 1, m_display_line_counter, 
                              QString("%1").arg(wpt->asAirport()->elevationFt()), MAGENTA);
            }
            else
            {
                drawTextRight(painter, 7, m_display_line_counter, "---/", color);
                drawTextRight(painter, 1, m_display_line_counter, "-----", color);
            }

            m_display_line_counter += 2;
        }
    }

    
    // draw bottom line destination airport info

    setFont(painter, NORM_FONT);

    if (ades != 0)
    {
        QString ades_string = ades->id();
        if (ades->hasActiveRunway()) ades_string += ades->activeRunwayId();

        drawTextLeft(painter,  1, 13, ades_string);

        setFont(painter, SMALL_FONT);

        QTime eta_dt = 
            fmcControl().normalRoute().routeData(fmcControl().normalRoute().destinationAirportIndex()).m_time_over_waypoint;

        if (m_fmc_data.hoursOverall() <= 0 || !eta_dt.isValid())
        {
            drawTextLeft(painter,  9, 13, "----");
        }
        else
        {
            drawTextLeft(painter,  9, 13, eta_dt.toString("hhmm"));
        }

        if (m_fmc_data.distanceOverallNm() <= 0)
            drawTextLeft(painter, 15, 13, "----");
        else
            drawTextLeft(painter, 15, 13, QString("%1").arg(m_fmc_data.distanceOverallNm(), 0, 'f', 0));

        drawTextRight(painter, 1, 13, "--.-");
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAFlightplan::processAction(const QString& action)
{
    if (action == ACTION_CALLBACK_WPT_SELECT)
    {
        processWptSelectCallback();
        return;
    }

    QString text = m_page_manager->scratchpad().text();
    int ades_wpt_index = fmcControl().normalRoute().destinationAirportIndex();

    if (action == ACTION_UP)
    {
        advanceVerticalScroll(+1);
        //TODO for now, only for the left side
        if (m_page_manager->isLeftSide())
            fmcControl().normalRoute().setViewWptIndex(m_vertical_scroll_offset);

        fmcControl().setMissedAppWaypointVisibleOnCDU(
            ades_wpt_index < 0 || m_vertical_scroll_offset > ades_wpt_index-4, m_page_manager->isLeftSide());
    }
    else if (action == ACTION_DOWN)
    {
        advanceVerticalScroll(-1);
        //TODO for now, only for the left side
        if (m_page_manager->isLeftSide())
            fmcControl().normalRoute().setViewWptIndex(m_vertical_scroll_offset);

        fmcControl().setMissedAppWaypointVisibleOnCDU(
            ades_wpt_index < 0 || m_vertical_scroll_offset > ades_wpt_index-4, m_page_manager->isLeftSide());
    }
    else if (action.startsWith("LLSK"))
    {
        bool convok = false;
        int llsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);
   
        if (llsk_index == 6)
        {
            const Waypoint* wpt = fmcControl().normalRoute().destinationAirport();
            if (wpt == 0) return;

            FMCCDUPageStyleAWaypoint* wpt_page = m_page_manager->wptPage();
            MYASSERT(wpt_page != 0);
            wpt_page->setWaypoint(fmcControl().normalRoute().destinationAirportIndex());
            m_page_manager->setCurrentPage(wpt_page->name());
            return;
        }
        else if (llsk_index > 0 && llsk_index < 6)
        {
            int wpt_insert_index = getWaypointIndexByLSK(llsk_index);
            MYASSERT(wpt_insert_index >= 0);

            if (wpt_insert_index > (int)fmcControl().normalRoute().count()) 
            {
                int alternate_index = wpt_insert_index - fmcControl().normalRoute().count();
                if (alternate_index > fmcControl().alternateRoute().count()) return;

                Logger::log(QString("FMCCDUPageStyleAFlightplan:processAction: "
                                    "got action for %1. alternate waypoint - "
                                    "this is not supported yet").arg(alternate_index));
                return;
            }

            const Waypoint* wpt = fmcControl().normalRoute().waypoint(getWaypointIndexByLSK(llsk_index));
       
            if (!m_flightstatus->onground &&
                wpt_insert_index == fmcControl().normalRoute().previousWaypointIndex() &&
                !text.isEmpty() )
            {
                m_page_manager->scratchpad().setOverrideText("Not allowed when airborne");
                return;
            }

            if (m_page_manager->scratchpad().action() == ACTION_OVERFLY)
            {
                if (wpt == 0) return;
                fmcControl().normalRoute().setWaypointOverflyRestriction(
                    getWaypointIndexByLSK(llsk_index), !wpt->restrictions().hasOverflyRestriction());
                m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
            }
            else if (m_page_manager->scratchpad().action() == FMCCDUPageBase::ACTION_CLR)
            {
                if (wpt == 0) return;
                fmcControl().normalRoute().removeWaypoint(wpt_insert_index);
                m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
            }
            else if (text.isEmpty())
            {
                if (wpt == 0) return;

                FMCCDUPageStyleAWaypoint* wpt_page = m_page_manager->wptPage();
                MYASSERT(wpt_page != 0);
                wpt_page->setWaypoint(wpt_insert_index);
                m_page_manager->setCurrentPage(wpt_page->name());
                return;
            }
            else
            {
                QString wpt_name;
                double pbd_bearing = 0.0;
                double pbd_distance_nm = 0.0;
                bool got_pbd_wpt = parsePBDSyntax(text, wpt_name, pbd_bearing, pbd_distance_nm);
                if (!got_pbd_wpt) wpt_name = text;

                // expand the route and process each occurence of dupliate waypoints
            
                WaypointPtrList wpt_selection_list;
                fmcControl().navdata().getWaypoints(wpt_name, wpt_selection_list, LATLON_WAYPOINT_REGEXP);
            
                if (wpt_selection_list.count() <= 0)
                {
                    m_page_manager->scratchpad().setOverrideText("No Waypoint found");
                    return;
                }
                else if (wpt_selection_list.count() == 1)
                {
                    Waypoint* wpt = wpt_selection_list.at(0);
                    if (got_pbd_wpt) 
                    {
                        *wpt = fmcControl().getPBDWaypoint(*wpt, pbd_bearing, pbd_distance_nm);
                    }

                    fmcControl().normalRoute().insertWaypoint(*wpt, wpt_insert_index);

                    if (llsk_index == 5) advanceVerticalScroll(+1);
                    else if (llsk_index == 1) advanceVerticalScroll(-1);
                    m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
                }
                else
                {
                    if (llsk_index == 5) advanceVerticalScroll(+1);
                    else if (llsk_index == 1) advanceVerticalScroll(-1);

                    wpt_selection_list.sortByDistance(m_flightstatus->current_position_raw);
                    setWptSelectPage(wpt_selection_list, wpt_insert_index, pbd_bearing, pbd_distance_nm);
                }
            }
        }
    }
    else if (action.startsWith("RLSK"))
    {
        bool convok = false;
        int rlsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);

        if (rlsk_index > 0 && rlsk_index < 6)
        {
            int wpt_insert_index = getWaypointIndexByLSK(rlsk_index);
            MYASSERT(wpt_insert_index >= 0);

            if (wpt_insert_index > (int)fmcControl().normalRoute().count()) 
            {
                int alternate_index = wpt_insert_index - fmcControl().normalRoute().count();
                if (alternate_index > fmcControl().alternateRoute().count()) return;

                Logger::log(QString("FMCCDUPageStyleAFlightplan:processAction: "
                                    "got action for %1. alternate waypoint - "
                                    "this is not supported yet").arg(alternate_index));
                return;
            }

            Waypoint* wpt = fmcControl().normalRoute().waypoint(getWaypointIndexByLSK(rlsk_index));
            if (wpt == 0) return;

            if (wpt->holding().isValid() && wpt->holding().isActive())
            {
                Holding holding = wpt->holding();
                holding.setExitHolding(!holding.exitHolding());
                fmcControl().normalRoute().setHolding(getWaypointIndexByLSK(rlsk_index), holding);
                return;
            }

            // handle speed/alt restrictions

            if (m_page_manager->scratchpad().action() == FMCCDUPageBase::ACTION_CLR)
            {
                wpt->restrictions().setSpeedRestrictionKts(0);
                wpt->restrictions().setAltitudeRestrictionFt(0);
                wpt->restrictions().setAltitudeRestrictionType(WaypointRestrictions::RESTRICTION_ALT_EQUAL);
                m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
            }
            else if (!text.isEmpty())
            {
                // assign restrictions
                if (!wpt->restrictions().setSpeedAndAltitudeRestrictionFromText(text))
                {
                    m_page_manager->scratchpad().setOverrideText("Invalid alt and/or speed");
                    return;
                }
            }
            
            m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

uint FMCCDUPageStyleAFlightplan::maxVerticalScrollOffset() const
{
    return qMax(0, ((int)fmcControl().normalRoute().count()) - ((int)fmcControl().normalRoute().activeWaypointIndex()));
}

/////////////////////////////////////////////////////////////////////////////

uint FMCCDUPageStyleAFlightplan::getWaypointIndexByLSK(uint lsk_index) const
{
    MYASSERT(lsk_index > 0);
    return qMax(0, ((int)fmcControl().normalRoute().activeWaypointIndex()) - 1) + 
        m_vertical_scroll_offset + ((int)lsk_index) - 1;
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAFlightplan::wptSelectCallback(const Waypoint& wpt, uint wpt_insert_index)
{
    Logger::log(QString("FMCCDUPageStyleAFlightplan:wptSelectCallback: selection=%1 wpt=%2").
                arg(wpt_insert_index).arg(wpt.id()));
    
    fmcControl().normalRoute().insertWaypoint(wpt, wpt_insert_index);
    m_vertical_scroll_offset = wpt_insert_index-3;
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAFlightplan::presetScrollOffset(int preset) 
{
    m_preset_scroll_offset = qMin(fmcControl().normalRoute().count(), qMax(0, preset)); 
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAFlightplan::setActive(bool active)
{
    FMCCDUPageBase::setActive(active);
    if (active) m_vertical_scroll_offset = m_preset_scroll_offset;
    m_preset_scroll_offset = 0;

    if (active) 
    {
        //TODO for now, only for the left side
        if (m_page_manager->isLeftSide())
            fmcControl().normalRoute().setViewWptIndex(m_vertical_scroll_offset);

        int ades_wpt_index = fmcControl().normalRoute().destinationAirportIndex();
        fmcControl().setMissedAppWaypointVisibleOnCDU(
            ades_wpt_index < 0 || m_vertical_scroll_offset > ades_wpt_index-4, m_page_manager->isLeftSide());
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleADirect::paintPage(QPainter& painter) const
{
    painter.setBackground(QBrush(BLACK));
    painter.fillRect(painter.window(), painter.background());

    setFont(painter, NORM_FONT, QFont::Bold);
    if (m_selected_wpt != 0) drawTextCenter(painter, 1, "DIR TO", YELLOW);
    else                     drawTextCenter(painter, 1, "DIR TO");

    setFont(painter, SMALL_FONT);
    drawTextLeft(painter, 2, 2, "WAYPOINT");
    drawTextRight(painter, 2, 2, "DIST");
    drawTextLeft(painter, 2, 4, "F-PLN WPTS");

    setFont(painter, NORM_FONT);
    if (m_selected_wpt != 0 && !m_wpt_is_in_route) drawTextLeft(painter, 1, 3, m_selected_wpt->id(), YELLOW);
    else                                           drawTextLeft(painter, 1, 3, "[     ]", CYAN);

    if (m_selected_wpt != 0)
    {
        double dist_to_wpt = Navcalc::getDistBetweenWaypoints(
            *m_selected_wpt, m_flightstatus->current_position_raw);
        
        drawTextRight(painter, 2, 3, QString("%1").arg(dist_to_wpt, 0, 'f', 0), YELLOW);
    }

    m_display_line_counter = 5;
    int index = getWaypointIndexByLSK(1);
    for(; index < fmcControl().normalRoute().count() ; ++index)
    {
        setFont(painter, NORM_FONT);

        const Waypoint* wpt = fmcControl().normalRoute().waypoint(index);
        if (wpt == 0)
        {
            m_vertical_scroll_offset = 0;
            break;
        }

        if (m_selected_wpt != 0 && m_wpt_is_in_route && *wpt == *m_selected_wpt)
            drawTextLeft(painter, 1, m_display_line_counter, QString("  ")+wpt->id(), CYAN);
        else
            drawTextLeft(painter, 1, m_display_line_counter, QString("<-")+wpt->id(), CYAN);

        m_display_line_counter += 2;
        if (m_display_line_counter > 12) break;
    }

    // direct confirm

    if (m_selected_wpt != 0)
    {
        setFont(painter, SMALL_FONT);
        drawTextLeft(painter, 2, 12, "DIR TO", ORANGE);
        drawTextRight(painter, 2, 12, "DIR TO", ORANGE);

        setFont(painter, NORM_FONT);
        drawTextLeft(painter, 1, 13, "< ERASE", ORANGE);
        drawTextRight(painter, 1, 13, "INSERT*", ORANGE);
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleADirect::processAction(const QString& action)
{
    if (action == ACTION_CALLBACK_WPT_SELECT)
    {
        processWptSelectCallback();
        return;
    }

    if (processBasicActions(action)) return;

    QString text = m_page_manager->scratchpad().text();
    int llsk_index = -1;
    int rlsk_index = -1;
    
    if (action.startsWith("LLSK"))
    {
        bool convok = false;
        llsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);
    }
    else if (action.startsWith("RLSK"))
    {
        bool convok = false;
        rlsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);
    }
    else
    {
        return;
    }
        
    if (llsk_index == 1)
    {
        if (text.isEmpty()) return;

        QString wpt_name;
        double pbd_bearing = 0.0;
        double pbd_distance_nm = 0.0;
        bool got_pbd_wpt = parsePBDSyntax(text, wpt_name, pbd_bearing, pbd_distance_nm);
        if (!got_pbd_wpt) wpt_name = text;

        WaypointPtrList wpt_selection_list;
        fmcControl().navdata().getWaypoints(wpt_name, wpt_selection_list, LATLON_WAYPOINT_REGEXP);

        if (wpt_selection_list.count() == 0)
        {
            m_page_manager->scratchpad().setOverrideText("Waypoint not found");
            return;
        }
        else if (wpt_selection_list.count() == 1)
        {
            Waypoint* wpt = wpt_selection_list.at(0);
            if (got_pbd_wpt) *wpt = fmcControl().getPBDWaypoint(*wpt, pbd_bearing, pbd_distance_nm);
            selectDirect(*wpt, fmcControl().normalRoute().activeWaypointIndex());
        }
        else
        {
            wpt_selection_list.sortByDistance(m_flightstatus->current_position_raw);
            setWptSelectPage(wpt_selection_list, fmcControl().normalRoute().activeWaypointIndex(), pbd_bearing, pbd_distance_nm);
        }

        m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
    }
    else if (llsk_index>0 && llsk_index < 6)
    {
        int wpt_index = getWaypointIndexByLSK(llsk_index-1);
        const Waypoint* wpt = fmcControl().normalRoute().waypoint(wpt_index);
        if (wpt != 0) selectDirect(*wpt, wpt_index);
    }
    else if (m_selected_wpt != 0)
    {
        if (llsk_index == 6)
        {
            delete m_selected_wpt;
            m_selected_wpt = 0;
            m_wpt_insert_index = -1;
            m_wpt_is_in_route = false;
            fmcControl().temporaryRoute().clear();
        }
        else if (rlsk_index == 6)
        {
            activateDirect(*m_selected_wpt, m_wpt_insert_index);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleADirect::setActive(bool active)
{
    fmcControl().temporaryRoute().clear();
    delete m_selected_wpt;
    m_selected_wpt = 0;
    m_wpt_insert_index = 0;
    m_wpt_is_in_route = false;
    FMCCDUPageStyleAFlightplan::setActive(active);
}

/////////////////////////////////////////////////////////////////////////////

uint FMCCDUPageStyleADirect::maxVerticalScrollOffset() const
{
    return qMax(0, ((int)fmcControl().normalRoute().count()) - ((int)fmcControl().normalRoute().activeWaypointIndex()) - 4);
}

/////////////////////////////////////////////////////////////////////////////

uint FMCCDUPageStyleADirect::getWaypointIndexByLSK(uint lsk_index) const
{
    MYASSERT(lsk_index > 0);
    return qMax(0, (int)fmcControl().normalRoute().activeWaypointIndex()) + m_vertical_scroll_offset + ((int)lsk_index) - 1;
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleADirect::wptSelectCallback(const Waypoint& wpt, uint wpt_insert_index)
{
    Logger::log(QString("FMCCDUPageStyleADirect:wptSelectCallback: selection=%1 wpt=%2").
                arg(wpt_insert_index).arg(wpt.id()));
    
    selectDirect(wpt, wpt_insert_index);
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleADirect::selectDirect(const Waypoint& dct_wpt, int wpt_insert_index)
{
    delete m_selected_wpt;
    m_selected_wpt = dct_wpt.deepCopy();
    m_wpt_insert_index = wpt_insert_index;

    fmcControl().temporaryRoute().clear();
    fmcControl().temporaryRoute() = fmcControl().normalRoute();

    uint search_index = 0;
    m_wpt_is_in_route = false;
    
    // search for the entered waypoint in the active route
    
    WaypointPtrListIterator iter(fmcControl().temporaryRoute().waypointList());
    while(iter.hasNext())
    {
        if (*iter.next() == dct_wpt) 
        {
            m_wpt_is_in_route = true;
            m_wpt_insert_index = search_index;
            break;
        }
        
        ++search_index;
    }
    
    // if the waypoint was not found in the active route we insert it
    if (!m_wpt_is_in_route) fmcControl().temporaryRoute().insertWaypoint(*m_selected_wpt, m_wpt_insert_index);

    fmcControl().temporaryRoute().goDirect(m_wpt_insert_index, m_fmc_data.turnRadiusNm(), "T-P");
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleADirect::activateDirect(const Waypoint& dct_wpt, int active_route_wpt_insert_index)
{
    Logger::log(QString("FMCCDUPageStyleADirect:activateDirect: %1, index=%2").arg(dct_wpt.id()).
                arg(active_route_wpt_insert_index));

    uint search_index = 0;
    bool found = false;
    
    // search for the entered waypoint in the active route
    
    WaypointPtrListIterator iter(fmcControl().normalRoute().waypointList());
    while(iter.hasNext())
    {
        if (*iter.next() == dct_wpt) 
        {
            found = true;
            active_route_wpt_insert_index = search_index;
            break;
        }
        
        ++search_index;
    }
    
    // if the waypoint was not found in the active route we insert it
    if (!found) fmcControl().normalRoute().insertWaypoint(dct_wpt, active_route_wpt_insert_index);
    
    // go direct

    fmcControl().normalRoute().goDirect(active_route_wpt_insert_index, m_fmc_data.turnRadiusNm(), "T-P");
    if (!m_flightstatus->onground) fmcControl().fmcAutoPilot().setNAVHold();
    m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
    m_page_manager->setCurrentPage(FMCCDUPageManagerStyleA::PAGE_FPLAN);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAAirport::setActive(bool active)
{
    if (active) 
    {
        FMCCDUPageStyleAFlightplan* fplan_page = m_page_manager->flightPlanPage();
        MYASSERT(fplan_page != 0);
        fplan_page->presetScrollOffset(fmcControl().normalRoute().destinationAirportIndex()-3);
        m_page_manager->setCurrentPage(fplan_page->name());
    }
    
    FMCCDUPageBase::setActive(active);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

FMCCDUPageStyleAInit::FMCCDUPageStyleAInit(const QString& page_name, FMCCDUPageManager* page_manager) : 
    FMCCDUPageBase(page_name, page_manager)
{
    MYASSERT(connect(&m_page_switch_check_timer, SIGNAL(timeout()), this, SLOT(slotCheckPageSwitch())));
}

/////////////////////////////////////////////////////////////////////////////

bool FMCCDUPageStyleAInit::allowSwitchToPage() const 
{
    return fmcControl().flightModeTracker().currentFlightMode() < FlightModeTracker::FLIGHT_MODE_TAKEOFF;
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAInit::slotCheckPageSwitch()
{
    if (!m_active) return;
    if (!allowSwitchToPage()) m_page_manager->setCurrentPage(FMCCDUPageManagerStyleA::PAGE_FPLAN);

    if (m_flightstatus->isAtLeastOneEngineOn() && m_horizontal_scroll_offset > 0)
        m_horizontal_scroll_offset = 0;
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAInit::setActive(bool active)
{
    if (active) m_page_switch_check_timer.start(250); 
    else m_page_switch_check_timer.stop();

    FMCCDUPageBase::setActive(active);
    slotCheckPageSwitch();
}

/////////////////////////////////////////////////////////////////////////////

uint FMCCDUPageStyleAInit::maxHorizontalScrollOffset() const 
{
    return (m_flightstatus->isAtLeastOneEngineOn()) ? 0 : 1;
} 

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAInit::paintPage(QPainter& painter) const
{
    QColor editable_color = CYAN;
    if (airborneAndRouteNotEmpty()) editable_color = GREEN;

    painter.setBackground(QBrush(BLACK));
    painter.fillRect(painter.window(), painter.background());

    MYASSERT(m_horizontal_scroll_offset >= 0);
    MYASSERT(m_horizontal_scroll_offset <= 1);

    switch(m_horizontal_scroll_offset)
    {
        case(0): {

            const Waypoint& curpos = m_flightstatus->current_position_raw;

            setFont(painter, NORM_FONT, QFont::Bold);
            drawTextCenter(painter, 1, "INIT");

            setFont(painter, SMALL_FONT);
            drawTextLeft(painter,  2,  2, "CO RTE");
            drawTextLeft(painter,  2,  6, "FLT NBR");
            drawTextLeft(painter,  2,  8, "LAT");
            drawTextLeft(painter,  2, 10, "COST INDEX");
            drawTextLeft(painter,  2, 12, "CRZ FL/TEMP");

            drawTextRight(painter, 2,  2, "FROM/TO");
            drawTextRight(painter, 2,  4, "ALTN");
            drawTextRight(painter, 2,  8, "LONG");
            drawTextRight(painter, 2, 12, "TROPO");

            setFont(painter, NORM_FONT);

            if (fmcControl().normalRoute().companyRoute().isEmpty())
                drawTextLeft(painter, 1,  3, QString(SPACER)+SPACER+SPACER+SPACER+SPACER+SPACER+SPACER+SPACER, ORANGE);
            else
                drawTextLeft(painter, 1,  3, fmcControl().normalRoute().companyRoute(), editable_color);

            if (fmcControl().normalRoute().flightNumber().isEmpty())
                drawTextLeft(painter, 1,  7,  QString(SPACER)+SPACER+SPACER+SPACER+SPACER+SPACER+SPACER+SPACER, ORANGE);
            else
                drawTextLeft(painter, 1,  7, fmcControl().normalRoute().flightNumber(), CYAN);

            if (m_flightstatus->isValid() && !fmcControl().isIRSAlignNeeded())
                drawTextLeft(painter, 1,  9, curpos.latStringDegMinSec(), CYAN);
            else
                drawTextLeft(painter, 1,  9, "----.--", CYAN);

            if (fmcControl().normalRoute().costIndex() < 0)
                drawTextLeft(painter, 1, 11, "---", CYAN);
            else
                drawTextLeft(painter, 1, 11, QString::number(fmcControl().normalRoute().costIndex()), CYAN);

            QString fl_temp_string;
            if (fmcControl().normalRoute().cruiseFl() < 0) fl_temp_string += "---";
            else fl_temp_string += QString("FL")+QString::number(fmcControl().normalRoute().cruiseFl());
            fl_temp_string += "/";
            if (fmcControl().normalRoute().cruiseTemp() < -999) fl_temp_string += "---";
            else fl_temp_string += QString::number(fmcControl().normalRoute().cruiseTemp());
            fl_temp_string += "°";
            drawTextLeft(painter, 1, 13, fl_temp_string, CYAN);

            if (!fmcControl().normalRoute().departureAirportId().isEmpty() != 0 && 
                !fmcControl().normalRoute().destinationAirportId().isEmpty())
            {       
                drawTextRight(painter, 1, 3, QString("%1/%2").
                              arg(fmcControl().normalRoute().departureAirportId()).
                              arg(fmcControl().normalRoute().destinationAirportId()), editable_color);
            }
            else
            {
                drawTextRight(painter, 1, 3, QString(SPACER)+SPACER+SPACER+SPACER+"/"+SPACER+SPACER+SPACER+SPACER, ORANGE);
            }

            if (fmcControl().alternateRoute().destinationAirport() == 0)
                drawTextRight(painter, 1,  5, "----", CYAN);
            else
                drawTextRight(painter, 1,  5, fmcControl().alternateRoute().destinationAirport()->id(), CYAN);

            if (fmcControl().isIRSAlignNeeded()) drawTextRight(painter, 1, 7, "ALIGN IRS ->", ORANGE);

            if (m_flightstatus->isValid() && !fmcControl().isIRSAlignNeeded())
                drawTextRight(painter, 1,  9, curpos.lonStringDegMinSec(), CYAN);
            else
                drawTextRight(painter, 1,  9, "-----.--", CYAN);

            drawTextRight(painter, 1, 11, "WIND >");

            if (fmcControl().normalRoute().tropoPause() < 0)
                drawTextRight(painter, 1, 13, "-----", CYAN);
            else
                drawTextRight(painter, 1, 13, QString::number(fmcControl().normalRoute().tropoPause()), CYAN);

            break;
        }

        case(1): {

            setFont(painter, NORM_FONT, QFont::Bold);
            drawTextCenter(painter, 1, "INIT B");

            setFont(painter, SMALL_FONT);
//TODO
//             drawTextLeft(painter,  2,  2, "TAXI");
//             drawTextLeft(painter,  2,  4, "TRIP/TIME");
//             drawTextLeft(painter,  2,  6, "RTE RSV/%");
//             drawTextLeft(painter,  2,  8, "ALTN/TIME");
//             drawTextLeft(painter,  2, 10, "FINAL/TIME");
//             drawTextLeft(painter,  2, 12, "EXTRA/TIME");

//TODO            drawTextRight(painter, 2,  2, "ZFWCG / ZFW");
            drawTextRight(painter, 2,  2, "ZFW");
            drawTextRight(painter, 2,  4, "BLOCK");
//             drawTextRight(painter, 2,  6, "BLOCK");
            drawTextRight(painter, 2,  8, "TOW");
//             drawTextRight(painter, 2, 10, "LW");
//             drawTextRight(painter, 2, 12, "TRIP WIND");

            setFont(painter, NORM_FONT);

            if (m_flightstatus->isValid())
            {
                drawTextRight(painter, 1,  3, QString("%1").
                             arg(m_flightstatus->zero_fuel_weight_kg / 1000.0, 0, 'f', 1), GREEN);

                drawTextRight(painter, 1,  5, QString("%1").
                             arg(m_flightstatus->fuelOnBoard() / 1000.0, 0, 'f', 1), GREEN);

                drawTextRight(painter, 1,  9, QString("%1").
                              arg(m_flightstatus->total_weight_kg / 1000.0, 0, 'f', 1), GREEN);
            }

            break;
        }
    }
}


/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAInit::processAction(const QString& action)
{
    if (processBasicActions(action)) return;

    int llsk_index = -1;
    int rlsk_index = -1;

    if (action.startsWith("LLSK"))
    {
        bool convok = false;
        llsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);
    }
    else if (action.startsWith("RLSK"))
    {
        bool convok = false;
        rlsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);
    }
    else
    {
        return;
    }

    //-----

    if (rlsk_index == 3)
    {
        fmcControl().setIRSAlignNeeded(false);
        return;
    }

    QString text = m_page_manager->scratchpad().text();
    if (text.isEmpty() && !m_page_manager->scratchpad().hasAction()) return;

    bool convok = false;
    uint text_to_uint = text.toUInt(&convok);

    //-----

    MYASSERT(m_horizontal_scroll_offset >= 0);
    MYASSERT(m_horizontal_scroll_offset <= 1);

    switch(m_horizontal_scroll_offset)
    {
        case(0): {

            if (llsk_index == 1)
            {
                if (airborneAndRouteNotEmpty())
                {
                    m_page_manager->scratchpad().setOverrideText("route not empty");
                    return;
                }

                if (m_page_manager->scratchpad().action() == FMCCDUPageBase::ACTION_CLR) return;

                // load FP by company name

                FMCCDUPageStyleAFPLoad* fpload_page = m_page_manager->flightPlanLoadPage();
                MYASSERT(fpload_page != 0);
                if (!fpload_page->setupByCompanyRoute(text))
                {
                    m_page_manager->scratchpad().setOverrideText("route not found");
                    return;
                }
        
                m_page_manager->setCurrentPage(fpload_page->name());
                m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
            }
            else if (llsk_index == 3)
            {
                if (m_page_manager->scratchpad().action() == FMCCDUPageBase::ACTION_CLR)
                {
                    fmcControl().normalRoute().setFlightNumber(QString::null);
                }
                else
                {
                    if (text.length() > 12)
                    {
                        m_page_manager->scratchpad().setOverrideText("Value too long");
                        return;
                    }
            
                    fmcControl().normalRoute().setFlightNumber(text);
                }
                m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);        
            }
            else if (llsk_index == 5)
            {
                if (m_page_manager->scratchpad().action() == FMCCDUPageBase::ACTION_CLR)
                {
                    fmcControl().normalRoute().setCostIndex(-1);
                }
                else
                {
                    if (!convok || text_to_uint > 999)
                    {
                        m_page_manager->scratchpad().setOverrideText("Invalid value");
                        return;
                    }
            
                    fmcControl().normalRoute().setCostIndex(text_to_uint);
                }
                m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);        
            }
            else if (llsk_index == 6)
            {
                if (m_page_manager->scratchpad().action() == FMCCDUPageBase::ACTION_CLR)
                {
                    fmcControl().normalRoute().setCruiseFl(-1);
                    fmcControl().normalRoute().setCruiseTemp(-1000);
                }
                else
                {
                    if (!text.contains('/')) 
                    {
                        uint cruise_fl = text.toUInt(&convok);
                        if (!convok || cruise_fl > 490)
                        {
                            m_page_manager->scratchpad().setOverrideText("Invalid cruise FL");
                            return;
                        }

                        fmcControl().normalRoute().setCruiseFl(cruise_fl);
                        fmcControl().normalRoute().setCruiseTemp(
                            (int)Navcalc::getAltTemp(m_flightstatus->oat, m_flightstatus->alt_ft, cruise_fl*100));
                        m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
                        return;
                    }

                    uint cruise_fl = text.section('/', 0, 0).toUInt(&convok);
                    if (!convok || cruise_fl > 490)
                    {
                        m_page_manager->scratchpad().setOverrideText("Invalid cruise FL");
                        return;
                    }
            
                    int cruise_temp = text.section('/', 1, 1).toInt(&convok);
                    if (!convok || cruise_temp < -100 || cruise_temp > 50)
                    {
                        m_page_manager->scratchpad().setOverrideText("Invalid cruise temperature");
                        return;
                    }
            
                    fmcControl().normalRoute().setCruiseFl(cruise_fl);
                    fmcControl().normalRoute().setCruiseTemp(cruise_temp);
                }

                m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);        
            }
            else if (rlsk_index == 1)
            {
                if (airborneAndRouteNotEmpty())
                {
                    m_page_manager->scratchpad().setOverrideText("route not empty");
                    return;
                }

                //TODO show INSERT* and <ERASE buttons on flightplan page
        
                if (m_page_manager->scratchpad().action() == FMCCDUPageBase::ACTION_CLR)
                {
                    m_fmc_data.clear();
                    m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
                    if (m_page_manager->scratchpad().action() == FMCCDUPageBase::ACTION_CLR) return;
                }

                if (text.isEmpty()) return;

                if (!text.contains('/'))
                {
                    m_page_manager->scratchpad().setOverrideText("Invalid value");
                    return;
                }
        
                QString adep_string = text.section('/', 0, 0);
                QString ades_string = text.section('/', 1, 1);

                if (text.length() > adep_string.length() + ades_string.length() + 1 ||
                    adep_string.isEmpty() || ades_string.isEmpty()) 
                {
                    m_page_manager->scratchpad().setOverrideText("Invalid value");
                    return;
                }

                m_fmc_data.clear();

                // set new route

                WaypointPtrList result_list;
                fmcControl().navdata().getAirports(adep_string, result_list);
                if (result_list.count() != 1)
                {
                    m_fmc_data.clear();
                    m_page_manager->scratchpad().setOverrideText("Dep. Airport not found");
                    return;
                }

                fmcControl().normalRoute().insertWaypoint(*result_list.at(0), 0);
                fmcControl().normalRoute().setAsDepartureAirport(0, QString::null);

                result_list.clear();
                fmcControl().navdata().getAirports(ades_string, result_list);
                if (result_list.count() != 1)
                {
                    m_fmc_data.clear();
                    m_page_manager->scratchpad().setOverrideText("Dest. Airport not found");
                    return;
                }

                fmcControl().normalRoute().insertWaypoint(*result_list.at(0), 1);
                fmcControl().normalRoute().setAsDestinationAirport(1, QString::null);
        
                fmcControl().normalRoute().setTropoPause(
                    (int)Navcalc::getTropopauseFt(m_flightstatus->oat, m_flightstatus->alt_ft));
                if (m_flightstatus->onground) fmcControl().setIRSAlignNeeded(true);

                m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
            }
            else if (rlsk_index == 2)
            {
                if (m_page_manager->scratchpad().action() == FMCCDUPageBase::ACTION_CLR)
                {
                    fmcControl().alternateRoute().clear();
                }
                else
                {
                    WaypointPtrList result_list;
                    fmcControl().navdata().getAirports(text, result_list);
                    if (result_list.count() != 1)
                    {
                        m_page_manager->scratchpad().setOverrideText("Airport not found");
                        return;
                    }

                    fmcControl().alternateRoute().clear();            
                    fmcControl().alternateRoute().insertWaypoint(*result_list.at(0)->asAirport(), 0);
                    fmcControl().alternateRoute().setAsDestinationAirport(0, QString::null);
                }
        
                m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
            }
            else if (rlsk_index == 6)
            {
                if (m_page_manager->scratchpad().action() == FMCCDUPageBase::ACTION_CLR)
                {
                    fmcControl().normalRoute().setTropoPause(-1);
                }
                else
                {
                    if (!convok || text_to_uint < 20000 || text_to_uint > 40000)
                    {
                        m_page_manager->scratchpad().setOverrideText("Invalid value");
                        return;
                    }
            
                    fmcControl().normalRoute().setTropoPause(text_to_uint);
                }

                m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
            }

            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

FMCCDUPageStyleACallbackBase::FMCCDUPageStyleACallbackBase(const QString& page_name, FMCCDUPageManager* page_manager) : 
    FMCCDUPageBase(page_name, page_manager), m_callback_page(0), m_selected_index(-1) 
{
    m_data_change_page_name_to_switch = FMCCDUPageManagerStyleA::PAGE_FPLAN;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

FMCCDUPageStyleAWaypointSelect::FMCCDUPageStyleAWaypointSelect(const QString& page_name, FMCCDUPageManager* page_manager) : 
    FMCCDUPageStyleACallbackBase(page_name, page_manager), m_wpt_list(0)
{
    m_data_change_page_name_to_switch = FMCCDUPageManagerStyleA::PAGE_FPLAN;
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAWaypointSelect::paintPage(QPainter& painter) const
{
    painter.setBackground(QBrush(BLACK));
    painter.fillRect(painter.window(), painter.background());

    setFont(painter, NORM_FONT, QFont::Bold);
    drawTextCenter(painter, 1, "DUPLICATE NAMES");

    setFont(painter, SMALL_FONT);
    drawTextCenter(painter, 2, "LAT/LONG");
    drawTextRight(painter, 2, 2, "FREQ");

    setFont(painter, NORM_FONT);
    drawTextLeft(painter, 1, 13, "< RETURN");

    if (m_wpt_list == 0 || m_callback_page == 0) return;

    const Waypoint& curpos = m_flightstatus->current_position_raw;    
    uint line_index = 3;

    int index = m_vertical_scroll_offset;
    for(; index < m_wpt_list->count(); ++index)
    {
        const Waypoint* wpt = m_wpt_list->at(index);
        MYASSERT(wpt != 0);
        
        setFont(painter, SMALL_FONT);
        double distance_to_fix = Navcalc::getDistBetweenWaypoints(curpos, *wpt);
        drawTextLeft(painter, 2, line_index-1, QString("%1NM").arg((int)distance_to_fix));

        setFont(painter, NORM_FONT);
        drawTextLeft(painter, 1, line_index, QString("* ") + wpt->id(), CYAN);
        
        QString lat_string = QString("%1").arg((int)fabs(wpt->lat()), 2, 10, QChar('0'));
        (wpt->lat() < 0) ? lat_string += "S" : lat_string += "N";
        QString lon_string = QString("%1").arg((int)fabs(wpt->lon()), 3, 10, QChar('0'));
        (wpt->lon() < 0) ? lon_string += "W" : lon_string += "E";
        drawTextCenter(painter, line_index, QString("%1/%2").arg(lat_string).arg(lon_string), GREEN);

        if (wpt->asNdb() != 0)
            drawTextRight(painter, 1, line_index, 
                          QString("%1").arg(wpt->asNdb()->freq() / 1000.0, 3, 'f', 3), GREEN);

        line_index += 2;
        if (line_index > 12) break;
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAWaypointSelect::processAction(const QString& action)
{
    if (m_wpt_list == 0 || m_callback_page == 0) return;

    m_selected_index = -1;

    if (processBasicActions(action)) return;

    if (action.startsWith("LLSK"))
    {
        bool convok = false;
        int llsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);
        m_selected_index = llsk_index - 1 + m_vertical_scroll_offset;

        if (llsk_index == 6)
        {
            m_page_manager->setInterimPage(QString::null);
            m_wpt_list = 0;
            m_callback_page = 0;
            m_callback_action = QString::null;
        }
        else  if (m_selected_index < m_wpt_list->count())
        {
            m_callback_page->processAction(m_callback_action);
            m_page_manager->setInterimPage(QString::null);
            m_wpt_list = 0;
            m_callback_page = 0;
            m_callback_action = QString::null;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

uint FMCCDUPageStyleAWaypointSelect::maxVerticalScrollOffset() const
{
    return qMax(0, ((int)m_wpt_list->count()) - 5);
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAWaypointSelect::setForSelection(const WaypointPtrList* wpt_list,
                                               FMCCDUPageBase* callback_page,
                                               const QString& callback_action)
{
    MYASSERT(wpt_list != 0);
    m_wpt_list = wpt_list;
    FMCCDUPageStyleACallbackBase::setForSelection(callback_page, callback_action);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

FMCCDUPageStyleADepartureSelect::FMCCDUPageStyleADepartureSelect(const QString& page_name, FMCCDUPageManager* page_manager) : 
    FMCCDUPageBase(page_name, page_manager), 
    m_departure_airport(0), m_filtered_sids(false), m_chosen_sid(0), m_chosen_sid_transition(0)
{
    m_data_change_page_name_to_switch = FMCCDUPageManagerStyleA::PAGE_FPLAN;
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleADepartureSelect::paintPage(QPainter& painter) const
{
    painter.setBackground(QBrush(BLACK));
    painter.fillRect(painter.window(), painter.background());

    setFont(painter, NORM_FONT, QFont::Bold);
    drawTextCenter(painter, 1, QString("DEPARTURES FROM %1").arg(m_departure_airport->id()));
    
    setFont(painter, SMALL_FONT);
    drawTextLeft(painter,  1, 2, "RWY");
    drawTextCenter(painter, 2, "SID");    
    drawTextRight(painter, 2, 2, "TRANS");

    setFont(painter, NORM_FONT);

    if (!m_chosen_dep_runway.isEmpty() || m_chosen_sid != 0)
        drawTextLeft(painter, 1, 13, "< ERASE", ORANGE);
    else
        drawTextLeft(painter, 1, 13, "< RETURN");

    if (!m_chosen_dep_runway.isEmpty()) drawTextLeft(painter,  1,  3, m_chosen_dep_runway, YELLOW);
    else                            drawTextLeft(painter,  1,  3, "---");

    if (m_chosen_sid != 0)
    {
        drawTextCenter(painter,  3, m_chosen_sid->id(), YELLOW);

        if (m_chosen_sid_transition != 0)
            drawTextRight(painter, 1,  3, m_chosen_sid_transition->id(), YELLOW);
        else
            drawTextRight(painter,  1,  3, "-----");
    }
    else
    {
        drawTextCenter(painter,  3, "-----");
        drawTextRight(painter,  1,  3, "-----");
    }

    if (!m_chosen_dep_runway.isEmpty() || m_chosen_sid != 0)
        drawTextRight(painter, 1, 13, "INSERT*", ORANGE);

    if (m_chosen_dep_runway.isEmpty())
    {
        drawTextCenter(painter, 4, "AVAILABLE RUNWAYS");

        uint line_index = 5;
        int offset = m_vertical_scroll_offset;

        RunwayMap::const_iterator iter = m_departure_airport->runwayMap().begin();
        for(; iter != m_departure_airport->runwayMap().end(); ++iter)
        {
            const Runway& runway = *iter;
            if (offset-- > 0) continue;

            setFont(painter, NORM_FONT);
            drawTextLeft(painter,  1, line_index, QString("<- %1").arg(runway.id()), CYAN);
            drawTextLeft(painter,  10, line_index, QString("%1M").arg(runway.lengthM()), CYAN);

            if (runway.hasILS())
            {
                setFont(painter, SMALL_FONT);
                drawTextLeft(painter,  6, line_index+1, QString::number(runway.ILSHdg()), CYAN);
                drawTextLeft(painter,  9, line_index+1, QString("ILS %1").arg(runway.ILSFreq()/1000.0, 3, 'f', 2), CYAN);
            }

            line_index += 2;
            if (line_index > 12) break;
        }
    }
    else
    {
        drawTextLeft(painter,  2, 4, "SIDS AVAILABLE");
        drawTextRight(painter, 1, 4, "TRANS");

        if (m_filtered_sids.isEmpty()) 
        {
            drawTextLeft(painter,  1,  5, "<- NO SID", CYAN);
        }
        else
        {
            //----- draw SIDs
            
            uint line_index = 5;
            
            int index = m_vertical_scroll_offset;
            for(; index < m_filtered_sids.count(); ++index)
            {
                const Sid* sid = m_filtered_sids.at(index);
                
                if (sid == m_chosen_sid) 
                    drawTextLeft(painter,  1, line_index, QString("  ")+sid->id(), CYAN);
                else
                    drawTextLeft(painter,  1, line_index, QString("<-")+sid->id(), CYAN);
                
                line_index += 2;
                if (line_index > 12) break;
            }
        }

        //----- draw transitions
        
        if (m_chosen_sid != 0)
        {
            bool found_transition = false;

            uint line_index = 5;
                
            for(int trans_index = 0; trans_index < m_chosen_sid->transitions().count(); ++trans_index)
            {
                if (trans_index < m_vertical_scroll_offset) continue;
                    
                const Transition* transition = m_chosen_sid->transitions().at(trans_index)->asTransition();
                MYASSERT(transition != 0);
                    
                if (transition == m_chosen_sid_transition)
                    drawTextRight(painter,  1, line_index, 
                                  transition->id()+QString("  "), CYAN);
                else
                    drawTextRight(painter,  1, line_index, 
                                  transition->id()+QString("->"), CYAN);

                found_transition = true;
                line_index += 2;
                if (line_index > 12) break;
            }

            if (!found_transition) drawTextRight(painter, 1, 5, "NO TRANS ->", CYAN);
        }
        else
        {
            drawTextRight(painter, 1, 5, "NO TRANS ->", CYAN);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleADepartureSelect::processAction(const QString& action)
{
    if (processBasicActions(action)) return;

    int llsk_index = -1;
    int rlsk_index = -1;

    if (action.startsWith("LLSK"))
    {
        bool convok = false;
        llsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);
    }
    else if (action.startsWith("RLSK"))
    {
        bool convok = false;
        rlsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);
    }
    else
    {
        return;
    }

    if (llsk_index == 1)
    {
        QString text = m_page_manager->scratchpad().text();

        if (m_page_manager->scratchpad().action() == FMCCDUPageBase::ACTION_CLR)  m_chosen_dep_runway = QString::null;
        else if (text.isEmpty()) return;

        m_chosen_dep_runway = text;
        m_chosen_sid = 0;
        m_chosen_sid_transition = 0;
        fmcControl().temporaryRoute().clear();
        m_page_manager->scratchpad().processAction(ACTION_CLRALL);
        m_vertical_scroll_offset = 0;
    }
    if (llsk_index >= 2 && llsk_index <= 5)
    {
        if (m_chosen_dep_runway.isEmpty())
        {
            int offset = ((int)(m_vertical_scroll_offset + llsk_index)) - 2;
            RunwayMap::const_iterator iter = m_departure_airport->runwayMap().begin();
            while(offset-- > 0) 
            {
                if (iter == m_departure_airport->runwayMap().end()) break;
                ++iter;
            }
            
            if (iter != m_departure_airport->runwayMap().end())
            {
                m_chosen_dep_runway = (*iter).id();
                m_vertical_scroll_offset = 0;
            }
        }
        else
        {
            int index = ((int)(m_vertical_scroll_offset + llsk_index)) - 2;
            if (index < m_filtered_sids.count())
            {
                m_chosen_sid = m_filtered_sids.at(index)->asSID();
                
                //----- set temporary route

                fmcControl().temporaryRoute() = fmcControl().normalRoute();
                fmcControl().temporaryRoute().setSid(*m_chosen_sid, m_chosen_dep_runway);

                MYASSERT(m_chosen_sid != 0);
                m_chosen_sid_transition = 0;
                m_vertical_scroll_offset = 0;
            }
        }
    }
    else if (rlsk_index >= 2 && rlsk_index <= 5)
    {
        if (!m_chosen_dep_runway.isEmpty() && m_chosen_sid != 0)
        {
            int index = ((int)(m_vertical_scroll_offset + rlsk_index)) - 2;

            if (index < m_chosen_sid->transitions().count())
            {
                m_chosen_sid_transition = m_chosen_sid->transitions().at(index)->asTransition();
            
                //----- set temporary route
                
                fmcControl().temporaryRoute() = fmcControl().normalRoute();
                fmcControl().temporaryRoute().setSid(*m_chosen_sid, m_chosen_dep_runway);
                if (m_chosen_sid_transition != 0) fmcControl().temporaryRoute().setSidTransition(*m_chosen_sid_transition);
            }
        }
    }
    else if (llsk_index == 6)
    {
        if (!m_chosen_dep_runway.isEmpty() || m_chosen_sid != 0)
        {
            m_chosen_sid = 0;
            m_chosen_sid_transition = 0;
            fmcControl().temporaryRoute().clear();
            m_chosen_dep_runway = QString::null;
            m_vertical_scroll_offset = 0;
        }
        else
        {
            FMCCDUPageStyleAWaypoint* wpt_page = m_page_manager->wptPage();
            MYASSERT(wpt_page != 0);
            wpt_page->setWaypoint(m_wpt_insert_index);
            m_page_manager->setCurrentPage(wpt_page->name());
        }
    }
    else if (rlsk_index == 6 && !m_chosen_dep_runway.isEmpty())
    {
        if (m_chosen_sid == 0)
        {
            fmcControl().normalRoute().setAsDepartureAirport(m_wpt_insert_index, m_chosen_dep_runway);
        }
        else
        {
            bool ok = true;
            FlightRoute normal_route_copy = fmcControl().normalRoute();
            uint scroll_offset_after = m_wpt_insert_index + m_chosen_sid->count();

            if (!normal_route_copy.setSid(*m_chosen_sid, m_chosen_dep_runway))
            {
                m_page_manager->scratchpad().setOverrideText("ERROR: insert SID");
                ok = false;
            }

            if (m_chosen_sid_transition != 0)
            {
                scroll_offset_after += m_chosen_sid_transition->count();

                if (!normal_route_copy.setSidTransition(*m_chosen_sid_transition))
                {
                    m_page_manager->scratchpad().setOverrideText("ERROR: insert SID TRANS");
                    ok = false;
                }
            }

            if (ok)
            {
                FMCCDUPageStyleAFlightplan* fplan_page = m_page_manager->flightPlanPage();
                MYASSERT(fplan_page != 0);
                if (fplan_page == m_page_manager->page(m_next_page))
                    fplan_page->presetScrollOffset(scroll_offset_after-2);

                fmcControl().normalRoute() = normal_route_copy;
            }
        }
        
        m_page_manager->setCurrentPage(m_next_page);
    }

    if (!m_chosen_dep_runway.isEmpty())
    {
        filterSids();
        m_vertical_scroll_offset = qMin(m_vertical_scroll_offset, m_filtered_sids.count()-4);
        m_vertical_scroll_offset = qMax(m_vertical_scroll_offset, 0);
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleADepartureSelect::setActive(bool active)
{
    if (!active) 
    {
        m_departure_airport = 0;
        m_sid_list.clear();
        m_chosen_sid = 0;
        m_chosen_sid_transition = 0;
        m_chosen_dep_runway = QString::null;
        fmcControl().temporaryRoute().clear();
    }
    
    FMCCDUPageBase::setActive(active);
}

/////////////////////////////////////////////////////////////////////////////

uint FMCCDUPageStyleADepartureSelect::maxVerticalScrollOffset() const
{
    if (m_chosen_dep_runway.isEmpty()) return qMax(0, ((int)m_departure_airport->runwayCount()) - 4);
    else                           return qMax(0, ((int)m_filtered_sids.count()) - 4);
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleADepartureSelect::setForSelection(uint wpt_insert_index,
                                                const Airport* departure_airport,
                                                const QString& next_page)
{
    MYASSERT(departure_airport != 0);

    m_wpt_insert_index = wpt_insert_index;
    m_departure_airport = departure_airport;
    m_next_page = next_page;

    m_sid_list.clear();
    m_chosen_sid = 0;
    m_chosen_sid_transition = 0;
    m_chosen_dep_runway = departure_airport->activeRunwayId();
    fmcControl().navdata().getSids(m_departure_airport->id(), m_sid_list);
    filterSids();

    //----- search SID

    if (!m_chosen_dep_runway.isEmpty() && !fmcControl().normalRoute().sidId().isEmpty())
    {
        int index = m_vertical_scroll_offset;
        for(; index < m_filtered_sids.count(); ++index)
        {
            if (m_filtered_sids.at(index)->id() == fmcControl().normalRoute().sidId())
            {
                m_chosen_sid = m_filtered_sids.at(index);
                break;
            }
        }
    }

    //----- search SID trans

    if (m_chosen_sid_transition != 0)
    {
        for(int trans_index = 0; trans_index < m_chosen_sid->transitions().count(); ++trans_index)
        {
            if (m_chosen_sid->transitions().at(trans_index)->id() == fmcControl().normalRoute().sidTransitionId())
            {
                m_chosen_sid_transition = m_chosen_sid->transitions().at(trans_index)->asTransition();
                break;
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleADepartureSelect::filterSids()
{
    m_filtered_sids.clear();
    
    for(int index = 0; index < m_sid_list.count(); ++index)
    {
        Procedure* procedure = m_sid_list.at(index);
        MYASSERT(procedure != 0);
        Sid* sid = procedure->asSID();
        if (sid == 0) continue;

        if (!m_chosen_dep_runway.isEmpty() && 
            sid->runwayList().count() > 0 && 
            !sid->runwayList().contains("ALL") && 
            !sid->runwayList().contains("All") && 
            !sid->runwayList().contains(m_chosen_dep_runway)) continue;

        m_filtered_sids.insertSorted(sid);
    }

    Logger::log(QString("FMCCDUPageStyleADepartureSelect:filterSids: "
                        "%1 of %2 SIDs after filtering").
                arg(m_filtered_sids.count()).arg(m_sid_list.count()));
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

FMCCDUPageStyleADestinationSelect::FMCCDUPageStyleADestinationSelect(const QString& page_name, FMCCDUPageManager* page_manager) : 
    FMCCDUPageBase(page_name, page_manager), m_destination_airport(0),
    m_filtered_stars(false), m_filtered_approaches(false),
    m_chosen_approach(0), m_chosen_star(0), m_chosen_app_transition(0)
{
    m_data_change_page_name_to_switch = FMCCDUPageManagerStyleA::PAGE_FPLAN;
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleADestinationSelect::paintPage(QPainter& painter) const
{
    painter.setBackground(QBrush(BLACK));
    painter.fillRect(painter.window(), painter.background());

    setFont(painter, NORM_FONT, QFont::Bold);
    drawTextCenter(painter, 1, QString("ARRIVAL TO %1").arg(m_destination_airport->id()));
    
    setFont(painter, SMALL_FONT);
    drawTextLeft(painter,  1, 2, "APPR");
    drawTextCenter(painter, 2, "VIA");    
    drawTextRight(painter, 2, 2, "STAR");
    drawTextRight(painter, 2, 4, "TRANS");

    setFont(painter, NORM_FONT);

    if (m_chosen_approach != 0 || m_chosen_star != 0)
        drawTextLeft(painter, 1, 13, "< ERASE", ORANGE);
    else
        drawTextLeft(painter, 1, 13, "< RETURN");

    //----- draw APPR

    if (m_chosen_approach != 0) drawTextLeft(painter,  1,  3, m_chosen_approach->id(), YELLOW);
    else                        drawTextLeft(painter,  1,  3, "---");

    //----- draw STAR + TRANS + VIA

    if (m_chosen_star != 0)
    {
        drawTextCenter(painter,  3, "-----");
        drawTextRight(painter, 1,  3, m_chosen_star->id(), YELLOW);

        if (m_chosen_app_transition != 0)
            drawTextRight(painter, 1,  5, m_chosen_app_transition->id(), YELLOW);
        else
            drawTextRight(painter,  1,  5, "-----");
    }
    else
    {
        drawTextCenter(painter,  3, "-----");
        drawTextRight(painter,  1,  3, "-----");
        drawTextRight(painter,  1,  5, "-----");
    }
    
    //-----

    if (m_chosen_approach != 0 || m_chosen_star != 0)
        drawTextRight(painter, 1, 13, "INSERT*", ORANGE);

    if (m_chosen_approach == 0)
    {
        drawTextCenter(painter, 6, "APPR AVAILABLE");

        //----- draw approaches

        uint line_index = 7;

        for(int index = m_vertical_scroll_offset; index < m_filtered_approaches.count(); ++index)
        {
            Procedure* procedure = m_filtered_approaches.at(index);
            MYASSERT(procedure != 0);
            Approach* approach = procedure->asApproach();
            MYASSERT(approach != 0);
            MYASSERT(approach->runwayList().count() == 1);
            MYASSERT(m_destination_airport->runwayMap().contains(approach->runwayList().at(0)));

            const Runway& runway = m_destination_airport->runway(approach->runwayList().at(0));
            
            setFont(painter, NORM_FONT);
            
            drawTextLeft(painter,  1, line_index, QString("<- %1").arg(approach->id()), CYAN);
            drawTextLeft(painter,  12, line_index, QString("%1M").arg(runway.lengthM()), CYAN);
            drawTextRight(painter, 1, line_index, QString("CRS%1").arg(runway.hdg()), CYAN);
            
            setFont(painter, SMALL_FONT);
            
            if (approach->id().startsWith("ILS") && runway.hasILS())
                drawTextCenter(painter, line_index+1, QString("ILS/%1").arg(runway.ILSFreq()/1000.0, 3, 'f', 2), CYAN);
            
            line_index += 2;
            if (line_index > 12) break;
        }
    }
    else
    {
        drawTextLeft(painter,  2, 6, "STARS AVAILABLE");
        drawTextRight(painter, 2, 6, "TRANS");

        //----- draw stars

        if (m_filtered_stars.isEmpty()) 
        {
            drawTextLeft(painter,  1,  7, "<- NO STAR", CYAN);
        }
        else
        {
            uint line_index = 7;
            
            int index = m_vertical_scroll_offset;
            for(; index < m_filtered_stars.count(); ++index)
            {
                const Star* star = m_filtered_stars.at(index);
                
                if (star == m_chosen_star)
                    drawTextLeft(painter,  1, line_index, QString("  ")+star->id(), CYAN);
                else
                    drawTextLeft(painter,  1, line_index, QString("<-")+star->id(), CYAN);
                
                line_index += 2;
                if (line_index > 12) break;
            }
        }

        //----- draw transitions

        if (m_chosen_star != 0 && m_chosen_approach->transitions().count() > 0)
        {
            uint line_index = 7;
            
            for(int trans_index = m_vertical_scroll_offset; 
                trans_index < m_chosen_approach->transitions().count(); ++trans_index)
            {
                const Transition* transition = m_chosen_approach->transitions().at(trans_index)->asTransition();
                MYASSERT(transition != 0);
                
                if (transition == m_chosen_app_transition)
                    drawTextRight(painter,  1, line_index, transition->id()+QString("  "), CYAN);
                else
                    drawTextRight(painter,  1, line_index, transition->id()+QString("->"), CYAN);
                
                line_index += 2;
                if (line_index > 12) break;
            }
        }
        else
        {
            drawTextRight(painter, 1, 7, "NO TRANS ->", CYAN);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleADestinationSelect::processAction(const QString& action)
{
    if (processBasicActions(action)) return;    

    int llsk_index = -1;
    int rlsk_index = -1;

    if (action.startsWith("LLSK"))
    {
        bool convok = false;
        llsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);
    }
    else if (action.startsWith("RLSK"))
    {
        bool convok = false;
        rlsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);
    }
    else
    {
        return;
    }

    if (llsk_index >= 3 && llsk_index <= 5)
    {
        if (m_chosen_approach == 0)
        {
            int offset = ((int)(m_vertical_scroll_offset + llsk_index)) - 3;

            m_chosen_approach = m_filtered_approaches.at(offset);
            MYASSERT(m_chosen_approach != 0);
            m_vertical_scroll_offset = 0;
        }
        else
        {
            int index = ((int)(m_vertical_scroll_offset + llsk_index)) - 3;
            if (index < m_filtered_stars.count())
            {
                m_chosen_star = m_filtered_stars.at(index)->asSTAR();
                MYASSERT(m_chosen_star != 0);

                //----- set temporary route

                fmcControl().temporaryRoute() = fmcControl().normalRoute();
                fmcControl().temporaryRoute().setStar(*m_chosen_star, m_chosen_approach->runway());

                m_vertical_scroll_offset = 0;
                m_chosen_app_transition = 0;
            }
        }
    }
    else if (llsk_index == 6)
    {
        if (m_chosen_approach != 0 || m_chosen_star != 0)
        {
            m_chosen_approach = 0;
            m_chosen_star = 0;
            m_chosen_app_transition = 0;
            fmcControl().temporaryRoute().clear();
            m_vertical_scroll_offset = 0;
        }
        else
        {
            FMCCDUPageStyleAWaypoint* wpt_page = m_page_manager->wptPage();
            MYASSERT(wpt_page != 0);
            wpt_page->setWaypoint(m_wpt_insert_index);
            m_page_manager->setCurrentPage(wpt_page->name());
        }
    }
    else if (rlsk_index >= 3 && rlsk_index <= 5)
    {
        if (m_chosen_approach != 0 && m_chosen_star != 0)
        {
            int offset = ((int)(m_vertical_scroll_offset + rlsk_index)) - 3;

            if (offset < m_chosen_approach->transitions().count())
            {
                m_chosen_app_transition = m_chosen_approach->transitions().at(offset)->asTransition();
                MYASSERT(m_chosen_app_transition != 0);
                MYASSERT(m_chosen_app_transition->parentProcedure() != 0);
                MYASSERT(m_chosen_app_transition->parentProcedure()->id() == m_chosen_approach->id());
                MYASSERT(m_chosen_app_transition->parentProcedure()->asApproach() != 0);
                
                //----- set temporary route
                
                fmcControl().temporaryRoute() = fmcControl().normalRoute();
                fmcControl().temporaryRoute().setStar(*m_chosen_star, m_chosen_approach->runway());
                
                if (m_chosen_app_transition != 0)
                    fmcControl().temporaryRoute().setAppTransition(*m_chosen_app_transition, m_chosen_approach->runway());
                
                if (m_chosen_approach != 0)
                    fmcControl().temporaryRoute().setApproach(*m_chosen_approach, m_chosen_approach->runway());
            }
        }
    }
    else if (rlsk_index == 6 && m_chosen_approach != 0)
    {
        bool ok = true;
        FlightRoute normal_route_copy = fmcControl().normalRoute();

        if (m_chosen_approach == 0 && m_chosen_star == 0)
            normal_route_copy.setAsDestinationAirport(m_wpt_insert_index,  m_chosen_approach->runway());

        if (m_chosen_star != 0)
        {
            if (!normal_route_copy.setStar(*m_chosen_star, m_chosen_approach->runway()))
            {
                m_page_manager->scratchpad().setOverrideText("ERROR: insert STAR");
                ok = false;
            }
            
            if (m_chosen_app_transition != 0 &&
                !normal_route_copy.setAppTransition(*m_chosen_app_transition, m_chosen_approach->runway()))
            {
                m_page_manager->scratchpad().setOverrideText("ERROR: insert APP TRANS");
                ok = false;
            }
        }

        if (m_chosen_approach != 0 &&
            !normal_route_copy.setApproach(*m_chosen_approach, m_chosen_approach->runway()))
        {
            m_page_manager->scratchpad().setOverrideText("ERROR: insert APPROACH");
            ok = false;
        }

        if (ok)
        {
            FMCCDUPageStyleAFlightplan* fplan_page = m_page_manager->flightPlanPage();
            MYASSERT(fplan_page != 0);
            if (fplan_page == m_page_manager->page(m_next_page))
                fplan_page->presetScrollOffset(normal_route_copy.destinationAirportIndex()-3);
            
            fmcControl().normalRoute() = normal_route_copy;
        }

        m_page_manager->setCurrentPage(m_next_page);
    }

    if (m_chosen_approach != 0)
    {
        filterStarsAndApproaches();
        int max = qMax(m_filtered_stars.count(), (int)m_chosen_approach->transitions().count()) - 2;
        m_vertical_scroll_offset = qMin(m_vertical_scroll_offset, max);
        m_vertical_scroll_offset = qMax(m_vertical_scroll_offset, 0);
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleADestinationSelect::setActive(bool active)
{
    if (!active) 
    {
        m_destination_airport = 0;
        m_star_list.clear();
        m_approach_list.clear();
        m_chosen_approach = 0;
        m_chosen_star = 0;
        m_chosen_app_transition = 0;
        fmcControl().temporaryRoute().clear();
    }
    
    FMCCDUPageBase::setActive(active);
}

/////////////////////////////////////////////////////////////////////////////

uint FMCCDUPageStyleADestinationSelect::maxVerticalScrollOffset() const
{
    if (m_chosen_approach == 0) 
        return qMax(0, ((int)m_filtered_approaches.count()) - 3);
    else
        return qMax(0, qMax(((int)m_filtered_stars.count()), (int)m_chosen_approach->transitions().count()) - 3);
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleADestinationSelect::setForSelection(uint wpt_insert_index,
                                                        const Airport* destination_airport,
                                                        const QString& next_page)
{
    MYASSERT(destination_airport != 0);

    m_wpt_insert_index = wpt_insert_index;
    m_destination_airport = destination_airport;
    m_next_page = next_page;

    m_chosen_approach = 0;
    m_chosen_app_transition = 0;
    m_approach_list.clear();
    fmcControl().navdata().getApproaches(destination_airport->id(), m_approach_list);

    m_chosen_star = 0;
    m_star_list.clear();
    fmcControl().navdata().getStars(destination_airport->id(), m_star_list);

    filterStarsAndApproaches();

    // set chosen approach

    for(int index = 0; index < m_filtered_approaches.count(); ++index)
    {
        if (m_filtered_approaches.at(index)->id() == fmcControl().normalRoute().approachId() &&
            m_filtered_approaches.at(index)->runway() == m_destination_airport->activeRunwayId())
        {
            m_chosen_approach = m_filtered_approaches.at(index);
            break;
        }
    }

    // set chosen star
    
    if (m_chosen_approach != 0 && !fmcControl().normalRoute().starId().isEmpty())
    {
        for(int index = 0; index < m_filtered_stars.count(); ++index)
        {
            if (m_filtered_stars.at(index)->id() == fmcControl().normalRoute().starId())
            {
                m_chosen_star = m_filtered_stars.at(index);
                break;
            }
        }
    }

    // set chosen app transition

    if (m_chosen_approach != 0 && m_chosen_star != 0)
    {
        for(int trans_index = 0; trans_index < m_chosen_approach->transitions().count(); ++trans_index)
        {
            if (m_chosen_approach->transitions().at(trans_index)->id() == fmcControl().normalRoute().appTransitionId())
            {
                m_chosen_app_transition = m_chosen_approach->transitions().at(trans_index)->asTransition();
                MYASSERT(m_chosen_app_transition->parentProcedure() != 0);
                MYASSERT(m_chosen_app_transition->parentProcedure()->id() == m_chosen_approach->id());
                MYASSERT(m_chosen_app_transition->parentProcedure()->asApproach() != 0);
                break;
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleADestinationSelect::filterStarsAndApproaches()
{
    m_filtered_stars.clear();
    m_filtered_approaches.clear();

    //----- filter approaches

    MYASSERT(m_destination_airport != 0);
    
    for(int index = 0; index < m_approach_list.count(); ++index)
    {
        Procedure* procedure = m_approach_list.at(index);
        MYASSERT(procedure != 0);
        Approach* approach = procedure->asApproach();
        if (approach == 0) continue;
        
        if (!m_destination_airport->runwayMap().contains(approach->runwayList().at(0))) continue;
        
        m_filtered_approaches.insertSorted(approach);
    }

    //----- generate pseudo approaches for runways without a procedure

    RunwayMap::const_iterator iter = m_destination_airport->runwayMap().begin();
    for(; iter != m_destination_airport->runwayMap().end(); ++iter)
    {
        const Runway& runway = *iter;

        bool found_approach_for_runway = false;

        for (int index = 0; index < m_filtered_approaches.count(); ++index)
        {
            const Approach* approach = m_filtered_approaches[index];

            if (approach->runway() == runway.id())
            {
                found_approach_for_runway = true;
                break;
            }
        }

        if (!found_approach_for_runway)
        {
            Logger::log(QString("FMCCDUPageStyleADestinationSelect:filterStarsAndApproaches: "
                                "generating pseudo approache for runway %1 of %2").
                        arg(runway.id()).arg(m_destination_airport->id()));;

            Approach* approach = 
                new Approach(runway.hasILS() ? QString("ILS%1").arg(runway.id()) : runway.id(),
                             QStringList(runway.id()));
            MYASSERT(approach != 0);
            m_approach_list.insertSorted(approach);
            m_filtered_approaches.insertSorted(approach);
        }
    }

    //----- filter STARs

    for(int index = 0; index < m_star_list.count(); ++index)
    {
        Procedure* procedure = m_star_list.at(index);
        MYASSERT(procedure != 0);
        Star* star = procedure->asSTAR();
        if (star == 0) continue;

        if (m_chosen_approach != 0 && 
            star->runwayList().count() > 0 && 
            !star->runwayList().contains("ALL") && 
            !star->runwayList().contains("All") && 
            !star->runwayList().contains(m_chosen_approach->runway())) continue;

        m_filtered_stars.insertSorted(star);
    }

    Logger::log(QString("FMCCDUPageStyleADestinationSelect:filterStarsAndApproaches: "
                        "%1 of %2 STARs and %3 of %4 approaches after filtering").
                arg(m_filtered_stars.count()).arg(m_star_list.count()).
                arg(m_filtered_approaches.count()).arg(m_approach_list.count()));
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleANavigation::paintPage(QPainter& painter) const
{
    painter.setBackground(QBrush(BLACK));
    painter.fillRect(painter.window(), painter.background());

    setFont(painter, NORM_FONT, QFont::Bold);
    drawTextCenter(painter, 1, "RADIO NAV");

    setFont(painter, SMALL_FONT);
    drawTextLeft(painter,  2,  2, "VOR1 / FREQ");
    drawTextLeft(painter,  2,  4, "CRS");
    drawTextLeft(painter,  2,  6, "ILS1 / FREQ");
    drawTextLeft(painter,  2,  8, "CRS");
    drawTextLeft(painter,  2, 10, "ADF1 / FREQ");

    drawTextRight(painter, 2,  2, "FREQ / VOR2");
    drawTextRight(painter, 2,  4, "CRS");
    drawTextRight(painter, 2,  6, "FREQ / ILS2");
    drawTextRight(painter, 2,  8, "CRS");
    drawTextRight(painter, 2, 10, "FREQ / ADF2");

    setFont(painter, NORM_FONT);

    QString obs = QString("%1").arg(m_flightstatus->obs1, 3, 10, QChar('0'));
    if (!m_flightstatus->nav1_has_loc)
    {
        // VOR1

        if (!m_flightstatus->nav1.id().isEmpty())
            drawTextLeft(painter, 1, 3, QString("%1/%2").arg(m_flightstatus->nav1.id()).
                         arg(m_flightstatus->nav1_freq / 1000.0, 3, 'f', 3), CYAN);
        else
            drawTextLeft(painter, 1, 3, QString("/%1").arg(m_flightstatus->nav1_freq / 1000.0, 3, 'f', 3), CYAN);

        drawTextLeft(painter, 1, 5, obs, CYAN);
    }
    else
    {
        // ILS1

        if (!m_flightstatus->nav1.id().isEmpty())
            drawTextLeft(painter, 1, 7, QString("%1/%2").arg(m_flightstatus->nav1.id()).
                         arg(m_flightstatus->nav1_freq / 1000.0, 3, 'f', 3), CYAN);
        else
            drawTextLeft(painter, 1, 7, QString("/%1").arg(m_flightstatus->nav1_freq / 1000.0, 3, 'f', 3), CYAN);

        drawTextLeft(painter, 1, 9, obs, CYAN);
    }

    if (!m_flightstatus->nav2_has_loc)
    {
        // VOR2
        
        if (!m_flightstatus->nav2.id().isEmpty())
            drawTextRight(painter, 1, 3, 
                          QString("%1/%2").arg(m_flightstatus->nav2_freq / 1000.0, 3, 'f', 3).
                          arg(m_flightstatus->nav2.id()), CYAN);
        else
            drawTextRight(painter, 1, 3, 
                          QString("%1/").arg(m_flightstatus->nav2_freq / 1000.0, 3, 'f', 3), CYAN);
        
        drawTextRight(painter, 1, 5, 
                      QString("%1").arg(m_flightstatus->obs2, 3, 10, QChar('0')), CYAN);
    }
    else
    {
        if (!m_flightstatus->nav2.id().isEmpty())
            drawTextRight(painter, 1, 7, 
                          QString("%1/%2").arg(m_flightstatus->nav2_freq / 1000.0, 3, 'f', 3).
                          arg(m_flightstatus->nav2.id()), CYAN);
        else
            drawTextRight(painter, 1, 7, 
                          QString("%1/").arg(m_flightstatus->nav2_freq / 1000.0, 3, 'f', 3), CYAN);
        
        drawTextRight(painter, 1, 9, 
                      QString("%1").arg(m_flightstatus->obs2, 3, 10, QChar('0')), CYAN);
    }

    // ADF1
    
    const Ndb &ndb1 = m_flightstatus->adf1;

    if (!ndb1.id().isEmpty())
        drawTextLeft(painter, 1, 11, QString("%1/%2").arg(ndb1.id()).arg(ndb1.freq() / 1000.0, 3, 'f', 2), CYAN);
    else
        drawTextLeft(painter, 1, 11, QString("/%2").arg(ndb1.freq() / 1000.0, 3, 'f', 2), CYAN);

    // ADF2
    
    const Ndb &ndb2 = m_flightstatus->adf2;

    if (!ndb2.id().isEmpty())
        drawTextRight(painter, 1, 11, 
                      QString("%1/%2").arg(ndb2.freq() / 1000.0, 3, 'f', 2).arg(ndb2.id()), CYAN);
    else
        drawTextRight(painter, 1, 11, 
                      QString("%1/").arg(ndb2.freq() / 1000.0, 3, 'f', 2), CYAN);

}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleANavigation::processAction(const QString& action)
{
    int llsk_index = -1;
    int rlsk_index = -1;

    if (action.startsWith("LLSK"))
    {
        bool convok = false;
        llsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);
    }
    else if (action.startsWith("RLSK"))
    {
        bool convok = false;
        rlsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);
    }
    else
    {
        return;
    }

    QString text = m_page_manager->scratchpad().text();
    if (text.isEmpty()) return;

    if (llsk_index == 1 || llsk_index == 3 || rlsk_index == 1) 
    {
        int freq = getVorFreq(text);
        if (freq == 0)
        {
            WaypointPtrList wpt_selection_list;
            fmcControl().navdata().getNavaids(text, wpt_selection_list, QString::null, Waypoint::TYPE_VOR);
            wpt_selection_list.sortByDistance(m_flightstatus->current_position_raw);

            if (wpt_selection_list.count() <= 0 || wpt_selection_list.at(0)->asVor() == 0)
            {
                m_page_manager->scratchpad().setOverrideText("No entry found or invalid");
                return;
            }
               
            freq = wpt_selection_list.at(0)->asVor()->freq();
        }

        if (llsk_index > 0) fmcControl().fsAccess().setNavFrequency(freq, 0);
        else                fmcControl().fsAccess().setNavFrequency(freq, 1);

        m_page_manager->scratchpad().processAction(ACTION_CLRALL);
    }
    else if (llsk_index == 5 || rlsk_index == 5)
    {
        int freq = getAdfFreq(text);
        if (freq == 0)
        {
            WaypointPtrList wpt_selection_list;
            fmcControl().navdata().getNavaids(text, wpt_selection_list, QString::null, Waypoint::TYPE_NDB);
            wpt_selection_list.sortByDistance(m_flightstatus->current_position_raw);

            if (wpt_selection_list.count() <= 0 || wpt_selection_list.at(0)->asNdb() == 0)
            {
                m_page_manager->scratchpad().setOverrideText("No entry found or invalid");
                return;
            }
               
            freq = wpt_selection_list.at(0)->asNdb()->freq();
        }

        if (llsk_index == 5) fmcControl().fsAccess().setAdfFrequency(freq, 0);
        else                 fmcControl().fsAccess().setAdfFrequency(freq, 1);

        m_page_manager->scratchpad().processAction(ACTION_CLRALL);
    }
    else if (llsk_index == 2 || llsk_index == 4 || rlsk_index == 2)
    {
        int hdg = getHeading(text);
        if (hdg < 0)
        {
            m_page_manager->scratchpad().setOverrideText("Invalid Course");
            return;
        }

        if (llsk_index > 0) fmcControl().fsAccess().setNavOBS(hdg, 0);
        else                fmcControl().fsAccess().setNavOBS(hdg, 1);

            m_page_manager->scratchpad().processAction(ACTION_CLRALL);
    }
}

/////////////////////////////////////////////////////////////////////////////

int FMCCDUPageStyleANavigation::getVorFreq(const QString& text) const
{
    bool convok = false;
    double freq_double = text.toDouble(&convok);
    if (!convok) return 0;
    int freq = (int)(freq_double * 1000);
    if (freq < 108000 || freq > 117950) return 0;
    return freq;
}

/////////////////////////////////////////////////////////////////////////////

int FMCCDUPageStyleANavigation::getAdfFreq(const QString& text) const
{
    bool convok = false;
    double freq_double = text.toDouble(&convok);
    if (!convok) return 0;
    int freq = (int)(freq_double * 1000);
    if (freq < 100000 || freq > 1799950) return 0;
    return freq;
}

/////////////////////////////////////////////////////////////////////////////

int FMCCDUPageStyleANavigation::getHeading(const QString& text) const
{
    bool convok = false;
    uint hdg = text.toUInt(&convok);
    if (!convok) return -1;
    if (hdg >= 360) return -1;
    return hdg;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

const QString FMCCDUPageStyleAData::MODE_WAYPOINT = "WPT";
const QString FMCCDUPageStyleAData::MODE_NAVAIDS = "NAV";
const QString FMCCDUPageStyleAData::MODE_RUNWAYS = "RWY";

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAData::paintPage(QPainter& painter) const
{
    painter.setBackground(QBrush(BLACK));
    painter.fillRect(painter.window(), painter.background());

    setFont(painter, NORM_FONT, QFont::Bold);

    if (m_mode.isEmpty())
    {
        drawTextCenter(painter, 1, "DATA INDEX");
        setFont(painter, NORM_FONT);
        
        drawTextLeft(painter, 1, 3, "<WAYPOINTS");
        drawTextLeft(painter, 1, 5, "<NAVAIDS");
        drawTextLeft(painter, 1, 7, "<RUNWAYS");
        
        drawTextRight(painter, 1, 3, "LOAD ROUTE>");
        drawTextRight(painter, 1, 5, "SAVE ROUTE>");
    }
    else if (m_mode == MODE_WAYPOINT)
    {
        drawTextCenter(painter, 1, "WAYPOINT");

        setFont(painter, SMALL_FONT);
        drawTextLeft(painter, 2, 2, "IDENT");
        
        if (m_user_wpt == 0)
        {
            setFont(painter, NORM_FONT);
            drawTextLeft(painter, 1, 3, QString(SPACER)+SPACER+SPACER+SPACER+SPACER+SPACER+SPACER, ORANGE);
        }
        else
        {
            setFont(painter, SMALL_FONT);
            drawTextLeft(painter, 2, 2, "IDENT");
            drawTextLeft(painter, 2, 4, "LAT/LONG");
            drawTextLeft(painter, 2, 6, "CTRY");

            setFont(painter, NORM_FONT);
            drawTextLeft(painter, 1, 3, m_user_wpt->id(), CYAN);
            drawTextLeft(painter, 1, 5, m_user_wpt->latStringDegMinSec()+"/"+m_user_wpt->lonStringDegMinSec(), GREEN);

            if (m_user_wpt->asIntersection() != 0)
                drawTextLeft(painter, 1, 7, m_user_wpt->asIntersection()->countryCode(), GREEN);
        }
    }
    else if (m_mode == MODE_NAVAIDS)
    {
        drawTextCenter(painter, 1, "NAVAID");

        setFont(painter, SMALL_FONT);
        drawTextLeft(painter, 2, 2, "IDENT");
        
        if (m_user_wpt == 0)
        {
            setFont(painter, NORM_FONT);
            drawTextLeft(painter, 1, 3, QString(SPACER)+SPACER+SPACER+SPACER+SPACER+SPACER+SPACER, ORANGE);
        }
        else
        {
            setFont(painter, SMALL_FONT);
            drawTextLeft(painter, 2, 2, "IDENT");
            drawTextLeft(painter, 2, 4, "CLASS/NAME");
            drawTextLeft(painter, 2, 6, "LAT/LONG");
            drawTextLeft(painter, 2, 8, "FREQ");
            drawTextLeft(painter, 2, 10, "ELV");
            drawTextLeft(painter, 2, 12, "CTRY");

            const Ndb* ndb = m_user_wpt->asNdb();
            const Vor* vor = m_user_wpt->asVor();

            QString name = m_user_wpt->type();
            if (vor != 0 && vor->hasDME()) name += "DME";
            name += "/"+m_user_wpt->name().left(18);
            if (m_user_wpt->name().length() > 18) name += "...";
                        
            setFont(painter, NORM_FONT);
            drawTextLeft(painter, 1, 3, m_user_wpt->id(), CYAN);
            drawTextLeft(painter, 1, 5, name, GREEN);
            drawTextLeft(painter, 1, 7, m_user_wpt->latStringDegMinSec()+"/"+m_user_wpt->lonStringDegMinSec(), GREEN);
            
            if (ndb != 0)
            {
                drawTextLeft(painter, 1, 9, ndb->freqString(), GREEN);
                drawTextLeft(painter, 1, 11, QString::number(ndb->elevationFt()), GREEN);
                drawTextLeft(painter, 1, 13, m_user_wpt->asIntersection()->countryCode(), GREEN);
            }
        }
    }
    else if (m_mode == MODE_RUNWAYS)
    {
        drawTextCenter(painter, 1, "RUNWAY");

        setFont(painter, SMALL_FONT);
        drawTextLeft(painter, 2, 2, "IDENT");
        
        if (m_user_wpt == 0)
        {
            setFont(painter, NORM_FONT);
            drawTextLeft(painter, 1, 3, QString(SPACER)+SPACER+SPACER+SPACER+SPACER+SPACER+SPACER, ORANGE);
        }
        else
        {
            const Airport* airport = m_user_wpt->asAirport();
            MYASSERT(airport != 0);
            Runway runway = airport->runway(m_rwy_id);

            setFont(painter, SMALL_FONT);
            drawTextLeft(painter, 2, 2, "IDENT");
            drawTextLeft(painter, 2, 4, "NAME");
            drawTextLeft(painter, 2, 6, "LAT/LONG");
            drawTextLeft(painter, 2, 8, "RWYS");
            drawTextLeft(painter, 2, 10, "LENGTH/CRS/ELV");
            drawTextLeft(painter, 2, 12, "ILS FREQ/CRS");

            drawTextLeft(painter, 1, 9, QStringList(airport->runwayMap().keys()).join(","), GREEN);

            QString name = QString("%1").arg(m_user_wpt->name().left(24));
            if (m_user_wpt->name().length() > 24) name += "...";
                
            setFont(painter, NORM_FONT);
            if (runway.isValid())
            {
                drawTextLeft(painter, 1, 3, airport->id()+m_rwy_id, CYAN);
                drawTextLeft(painter, 1, 5, name, GREEN);
                drawTextLeft(painter, 1, 7, runway.latStringDegMinSec()+"/"+runway.lonStringDegMinSec(), GREEN);
                drawTextLeft(painter, 1, 11, QString("%1M/%2°/%3").
                             arg(runway.lengthM()).arg(runway.hdg()).arg(runway.thresholdElevationFt()), GREEN);

                if (runway.hasILS())
                    drawTextLeft(painter, 1, 13, QString("%1/%2°").
                                 arg(runway.ILSFreq()/1000.0, 3, 'f', 2).arg(runway.ILSHdg()), GREEN);
                else
                    drawTextLeft(painter, 1, 13, "---.--/---", GREEN);
            }
            else
            {
                drawTextLeft(painter, 1, 3, airport->id(), CYAN);
                drawTextLeft(painter, 1, 5, name, GREEN);
                drawTextLeft(painter, 1, 7, m_user_wpt->latStringDegMinSec()+"/"+m_user_wpt->lonStringDegMinSec(), GREEN);
                drawTextLeft(painter, 1, 11, "-----/---/----", GREEN);
                drawTextLeft(painter, 1, 13, "---.--/---", GREEN);
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAData::processAction(const QString& action)
{
    if (action == ACTION_CALLBACK_WPT_SELECT)
    {
        processWptSelectCallback();
        return;
    }

    int llsk_index = -1;
    int rlsk_index = -1;

    if (action.startsWith("LLSK"))
    {
        bool convok = false;
        llsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);
    }
    else if (action.startsWith("RLSK"))
    {
        bool convok = false;
        rlsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);
    }
    else
    {
        return;
    }

    QString text = m_page_manager->scratchpad().text();

    if (m_mode.isEmpty())
    {
        if (llsk_index == 1)
        {
            m_mode = MODE_WAYPOINT;
        }
        else if (llsk_index == 2)
        {
            m_mode = MODE_NAVAIDS;
        }
        else if (llsk_index == 3)
        {
            m_mode = MODE_RUNWAYS;
        }
        if (rlsk_index == 1)
        {
            m_page_manager->setCurrentPage(FMCCDUPageManagerStyleA::PAGE_FPLOAD);
        }
        else if (rlsk_index == 2)
        {
            m_page_manager->setCurrentPage(FMCCDUPageManagerStyleA::PAGE_FPSAVE);
        }
    }
    else if (m_mode == MODE_WAYPOINT || m_mode == MODE_NAVAIDS || m_mode == MODE_RUNWAYS)
    {
        if (text.isEmpty()) return;

        WaypointPtrList wpt_selection_list;
        if (m_mode == MODE_WAYPOINT)
            fmcControl().navdata().getIntersections(text, wpt_selection_list);
        else if (m_mode == MODE_NAVAIDS)
            fmcControl().navdata().getNavaids(text, wpt_selection_list);
        else if (m_mode == MODE_RUNWAYS)
        {
            m_rwy_id = text.mid(4);
            text = text.left(4);
            fmcControl().navdata().getAirports(text, wpt_selection_list);
        }
            
        if (wpt_selection_list.count() <= 0)
        {
            m_page_manager->scratchpad().setOverrideText("No entrys found");
            return;
        }
        else if (wpt_selection_list.count() == 1)
        {
            m_user_wpt = wpt_selection_list.at(0)->deepCopy();
            m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
        }
        else
        {
            wpt_selection_list.sortByDistance(m_flightstatus->current_position_raw);
            setWptSelectPage(wpt_selection_list, 0, 0, 0);
            m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAData::setActive(bool active)
{
    FMCCDUPageBase::setActive(active);
    m_mode.clear();
    delete m_user_wpt;
    m_user_wpt = 0;
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAData::wptSelectCallback(const Waypoint& wpt, uint)
{
    delete m_user_wpt;
    m_user_wpt = wpt.deepCopy();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAFPLoad::paintPage(QPainter& painter) const
{
    painter.setBackground(QBrush(BLACK));
    painter.fillRect(painter.window(), painter.background());

    if (m_selected_fp_index < 0)
    {
        setFont(painter, NORM_FONT, QFont::Bold);
        drawTextCenter(painter, 1, "LOAD ROUTE");
        
        setFont(painter, NORM_FONT);
        
        if (m_fp_filename_list.isEmpty())
        {
            drawTextLeft(painter, 1, 3, "NO ROUTES FOUND");
            return;
        }
        
        int display_line_counter = 3;
        int index = getIndexByLSK(1);
        
        for(; index < m_fp_filename_list.count(); ++index)
        {
            QColor color = CYAN;
            if (index == m_selected_fp_index) color = YELLOW;
            
            drawTextLeft(painter, 1, display_line_counter, QString("<- %1").
                         arg(m_fp_filename_list[index].section(CFG_FLIGHTPLAN_EXTENSION, 0, 0).toUpper()), color);

            if (display_line_counter > 12) break;            
            display_line_counter += 2;
        }
    }
    else
    {
        setFont(painter, NORM_FONT, QFont::Bold);
        drawTextCenter(painter, 1, "ROUTE");
        
        setFont(painter, SMALL_FONT);
        drawTextLeft(painter, 2, 2, "CO RTE");
        drawTextRight(painter, 2, 2, "FROM/TO");

        setFont(painter, NORM_FONT);

        if (fmcControl().temporaryRoute().companyRoute().isEmpty())
            drawTextLeft(painter, 1,  3, QString(SPACER)+SPACER+SPACER+SPACER+SPACER+SPACER+SPACER+SPACER, ORANGE);
        else
            drawTextLeft(painter, 1,  3, fmcControl().temporaryRoute().companyRoute(), CYAN);

//         drawTextRight(painter, 1, 3, QString("%1/%2").
//                       arg(fmcControl().temporaryRoute().normalRoute().departureAirportId()).
//                       arg(fmcControl().temporaryRoute().destinationAirportId()), CYAN);

        drawTextRight(painter, 1, 3, QString("%1/%2"). arg(m_adep).arg(m_ades), CYAN);

        drawRouteWaypoints(painter, fmcControl().temporaryRoute(), 4, 10);

        drawTextLeft(painter, 1, 11, "<ERASE", ORANGE);

        if (!m_fp_delete_flag) drawTextRight(painter, 1, 11, "DELETE FILE>", ORANGE);
        else                   drawTextRight(painter, 1, 11, "CONFIRM>", RED);

        if (fmcControl().allowFPLoad())
            drawTextLeft(painter, 1, 13, "*TO ACTIVE", ORANGE);
        drawTextRight(painter, 1, 13, "TO SEC >", ORANGE);
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAFPLoad::processAction(const QString& action)
{
    if (processBasicActions(action)) 
    {
        if (m_selected_fp_index >= 0 && m_horizontal_scroll_offset != m_selected_fp_index) 
            selectFP(m_horizontal_scroll_offset);
        m_fp_delete_flag = false;
        return;
    }

    int llsk_index = -1;
    int rlsk_index = -1;
    if (!getLSKIndex(action, llsk_index, rlsk_index)) return;

    if (m_selected_fp_index < 0)
    {
        m_fp_delete_flag = false;

        if (llsk_index >= 1 && llsk_index <= 6)
        {
            selectFP(getIndexByLSK(llsk_index));
        }
    }
    else
    {
        if (llsk_index == 5)
        {
            clear();
        }
        else if (llsk_index == 6 || rlsk_index == 6)
        {
            m_fp_delete_flag = false;

            MYASSERT(m_selected_fp_index < m_fp_filename_list.count());
            QString fp_name = m_fp_filename_list[m_selected_fp_index];

            Logger::log(QString("FMCCDUPageStyleAFPLoad:processAction: loading FP (%1)").arg(fp_name));

            if (llsk_index == 6 && fmcControl().allowFPLoad())
            {
                if (!fmcControl().normalRoute().loadFP(m_fp_path+"/"+fp_name, &fmcControl().navdata()))
                {
                    m_page_manager->scratchpad().setOverrideText("Could not load ROUTE");
                }
                else
                {
                    m_page_manager->setCurrentPage(FMCCDUPageManagerStyleA::PAGE_INIT);
                    if (m_flightstatus->onground) fmcControl().setIRSAlignNeeded(true);
                }
            }
            else if (rlsk_index == 6)
            {
                if (!fmcControl().secondaryRoute().loadFP(m_fp_path+"/"+fp_name, &fmcControl().navdata()))
                    m_page_manager->scratchpad().setOverrideText("Could not load ROUTE");
                else
                    m_page_manager->setCurrentPage(FMCCDUPageManagerStyleA::PAGE_SECFP);
            }
        }

        // FP delete - we only delete the FP, if the DELETE entry was pressed
        // twice. on the first press only the m_fp_delete_flag will be set.        
        if (rlsk_index == 5)
        {
            if (!m_fp_delete_flag)
            {
                m_fp_delete_flag = true;
            }
            else
            {
                deleteFP(m_selected_fp_index);
                setupFPList();
                clear();
            }
        }
        else
        {
            m_fp_delete_flag = false;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

bool FMCCDUPageStyleAFPLoad::deleteFP(int index)
{
    MYASSERT(index >= 0);
    MYASSERT(index < m_fp_filename_list.count());
    if (!QFile::remove(m_fp_path+"/"+m_fp_filename_list[m_selected_fp_index]))
    {
        m_page_manager->scratchpad().setOverrideText("Could not delete ROUTE");
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool FMCCDUPageStyleAFPLoad::selectFP(int index)
{
    if (index < 0 || index >= m_fp_filename_list.count()) return false;

    m_vertical_scroll_offset = 0;
    m_selected_fp_index = index;

    QString fp_name = m_fp_filename_list[m_selected_fp_index];
    
    if (!fmcControl().temporaryRoute().loadFP(m_fp_path+"/"+fp_name, 0))
    {
        clear();
        m_page_manager->scratchpad().setOverrideText("Could not load ROUTE");
        return false;
    }

    m_adep = fp_name.left(4);
    m_ades = fp_name.mid(4, 4);

    m_horizontal_scroll_offset = m_selected_fp_index;
    m_fp_selection_count = m_fp_filename_list.count()-1;
    return true;
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAFPLoad::clear()
{
    m_selected_fp_index = -1;
    fmcControl().temporaryRoute().clear();
    m_fp_selection_count = 0;
    m_adep.clear();
    m_ades.clear();
    m_fp_delete_flag = false;
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAFPLoad::setActive(bool active)
{
    if (active) 
    {
        setupFPList();

        Logger::log(QString("FMCCDUPageStyleAFPLoad:setActive: found %1 FPs").arg(m_fp_filename_list.count()));
    }
    else
    {
        m_fp_filename_list.clear();
        clear();
    }

    m_fp_delete_flag = false;
    int horizontal_scroll_offset = m_horizontal_scroll_offset;
    FMCCDUPageBase::setActive(active);
    m_horizontal_scroll_offset = horizontal_scroll_offset;
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAFPLoad::setupFPList()
{
    m_fp_path = fmcControl().mainConfig().getValue(CFG_VASFMC_DIR)+"/"+
                fmcControl().mainConfig().getValue(CFG_FLIGHTPLAN_SUBDIR);
    
    QDir fpdir(m_fp_path);

    Logger::log(QString("FMCCDUPageStyleAFPLoad:setupFPList: before: current_dir=%1").
                arg(fpdir.currentPath()));

    if (!fpdir.exists()) 
    {
        Logger::log(QString("FMCCDUPageStyleAFPLoad:setupFPList: FP dir (%1) not found").arg(fpdir.path()));
        return;
    }
    
    fpdir.refresh();
    m_fp_filename_list = fpdir.entryList(QStringList(QString("*")+CFG_FLIGHTPLAN_EXTENSION), 
                                         QDir::Files|QDir::Readable, QDir::Name);

    Logger::log(QString("FMCCDUPageStyleAFPLoad:setupFPList: after: current_dir=%1").
                arg(fpdir.currentPath()));
}

/////////////////////////////////////////////////////////////////////////////

uint FMCCDUPageStyleAFPLoad::maxVerticalScrollOffset() const
{
    if (m_selected_fp_index < 0)
        return qMax(0, (m_fp_filename_list.count() - 6));
    else
        return qMax(0, ((fmcControl().temporaryRoute().count()/2)-5));
}

/////////////////////////////////////////////////////////////////////////////

int FMCCDUPageStyleAFPLoad::getIndexByLSK(uint lsk_index) const
{
    MYASSERT(lsk_index > 0);
    return m_vertical_scroll_offset + ((int)lsk_index) - 1;
}

/////////////////////////////////////////////////////////////////////////////

bool FMCCDUPageStyleAFPLoad::setupByCompanyRoute(const QString& company_route)
{
    setupFPList();

    int index = m_fp_filename_list.indexOf(company_route + CFG_FLIGHTPLAN_EXTENSION);

    if (index < 0)
    {
        Logger::log(QString("FMCCDUPageStyleAFPLoad:setup: company FP (%1) not found").arg(company_route));
        clear();
        return false;
    }

    selectFP(index);
    return true;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAFPSave::paintPage(QPainter& painter) const
{
    painter.setBackground(QBrush(BLACK));
    painter.fillRect(painter.window(), painter.background());

    setFont(painter, NORM_FONT, QFont::Bold);
    drawTextCenter(painter, 1, "SAVE ROUTE");

    setFont(painter, NORM_FONT);

    drawTextLeft(painter, 1, 3, "<- SAVE ACTIVE ROUTE", CYAN);
    drawTextLeft(painter, 1, 5, "<- SAVE SEC ROUTE", CYAN);
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAFPSave::processAction(const QString& action)
{
    int llsk_index = -1;
    int rlsk_index = -1;

    if (action.startsWith("LLSK"))
    {
        bool convok = false;
        llsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);
    }
    else if (action.startsWith("RLSK"))
    {
        bool convok = false;
        rlsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);
    }
    else
    {
        return;
    }

    const FlightRoute* route_to_save = 0;
    if (llsk_index == 1)      route_to_save = &fmcControl().normalRoute();
    else if (llsk_index == 2) route_to_save = &fmcControl().secondaryRoute();

    //-----

    if (route_to_save == 0) return;
    if (route_to_save->count() == 0)
    {
        m_page_manager->scratchpad().setOverrideText("F-PLN IS EMPTY");
        return;
    }
    else if (route_to_save->departureAirport() == 0)
    {
        m_page_manager->scratchpad().setOverrideText("NO DEPARTURE AIRPORT");
        return;
    }
    else if (route_to_save->destinationAirport() == 0)
    {
        m_page_manager->scratchpad().setOverrideText("NO DESTINATION AIRPORT");
        return;
    }

    updateFPList();

    QString adep_ades_string = route_to_save->departureAirportId() + route_to_save->destinationAirportId();
    MYASSERT(!adep_ades_string.isEmpty());

    // search for matching FP - do not save the same route twice
    
    QDir fpdir(m_fp_path);
    QStringList same_adep_ades_filename_list = 
        fpdir.entryList(QStringList(adep_ades_string+"*"+CFG_FLIGHTPLAN_EXTENSION), QDir::Files|QDir::Readable, QDir::Name);

    QString fp_name;

    for(int index=0; index<same_adep_ades_filename_list.count(); ++index)
    {
        FlightRoute existing_route(fmcControl().flightStatus());
        if (existing_route.loadFP(m_fp_path+"/"+same_adep_ades_filename_list[index], 0) &&
            existing_route.compareWaypoints(*route_to_save))
        {
            fp_name = same_adep_ades_filename_list[index].mid(
                0, same_adep_ades_filename_list[index].length() - QString(CFG_FLIGHTPLAN_EXTENSION).length());
            break;
        }
    }

    // generate a new filename index

    uint number = 0;
    if (fp_name.isEmpty())
    {
        do fp_name = adep_ades_string + QString("%1").arg(++number, 3, 10, QChar('0'));
        while(m_fp_filename_list.contains(fp_name+CFG_FLIGHTPLAN_EXTENSION));
    }

    // set company route
    if (route_to_save == &fmcControl().normalRoute())
        fmcControl().normalRoute().setCompanyRoute(fp_name);
    else if(route_to_save == &fmcControl().secondaryRoute())
        fmcControl().secondaryRoute().setCompanyRoute(fp_name);

    // save the FP

    Logger::log(QString("FMCCDUPageStyleAFPSave:processAction: saving FP (%1)").arg(fp_name+CFG_FLIGHTPLAN_EXTENSION));
    
    if (!route_to_save->saveFP(m_fp_path+"/"+fp_name+CFG_FLIGHTPLAN_EXTENSION))
    {
        m_page_manager->scratchpad().setOverrideText("Could not save ROUTE");
    }
    else
    {
        m_page_manager->scratchpad().setOverrideText(QString("SAVED %1").arg(fp_name));
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAFPSave::setActive(bool active)
{
    if (active) 
    {
        m_fp_path = fmcControl().mainConfig().getValue(CFG_VASFMC_DIR)+"/"+
                    fmcControl().mainConfig().getValue(CFG_FLIGHTPLAN_SUBDIR);
        updateFPList();
    }
    else
    {
        m_fp_filename_list.clear();
    }

    FMCCDUPageBase::setActive(active);
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAFPSave::updateFPList()
{
    m_fp_filename_list.clear();

    QDir fpdir(m_fp_path);
    if (!fpdir.exists()) 
    {
        Logger::log(QString("FMCCDUPageStyleAFPSave:setActive: FP dir (%1) not found").arg(fpdir.path()));
        return;
    }

    m_fp_filename_list = fpdir.entryList(QStringList(QString("*")+CFG_FLIGHTPLAN_EXTENSION), 
                                         QDir::Files|QDir::Readable, QDir::Name);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAProgress::paintPage(QPainter& painter) const
{
    painter.setBackground(QBrush(BLACK));
    painter.fillRect(painter.window(), painter.background());

    setFont(painter, SMALL_FONT);
    drawTextLeft(painter, 2, 2, "CRZ");
    drawTextCenter(painter, 2, "OPT");
    drawTextRight(painter, 2, 2, "REC MAX");

    drawTextLeft(painter, 2, 8, "BRG  /DIST");

    drawTextLeft(painter, 1, 12, "REQUIRED");
    drawTextCenter(painter, 12, "ACCUR");
    drawTextRight(painter, 1, 12, "ESTIMATED");

    //-----

    setFont(painter, NORM_FONT, QFont::Bold);
    drawTextLeft(painter, 4, 1, fmcControl().flightModeTracker().currentFlightModeText(), GREEN);
    drawTextRight(painter, 4, 1, fmcControl().normalRoute().flightNumber());

    //-----

    setFont(painter, NORM_FONT);
    QString fl_temp_string;
    if (fmcControl().normalRoute().cruiseFl() < 0) fl_temp_string += "---";
    else fl_temp_string += QString("FL")+QString::number(fmcControl().normalRoute().cruiseFl());
    drawTextLeft(painter, 1, 3, fl_temp_string, CYAN);
    drawTextCenter(painter, 3, "-----", GREEN);
    drawTextRight(painter, 1, 3, "-----", MAGENTA);

    //-----

    setFont(painter, NORM_FONT);

    if (m_wpt == 0)
    {
        drawTextLeft(painter, 1, 9, "---° /----.-");
        drawTextRight(painter, 8, 9, "TO ");
        drawTextRight(painter, 1, 9, "[     ]", CYAN);
    }
    else
    {
        double bearing = 0.0;
        double distance = 0.0;
        Navcalc::getDistAndTrackBetweenWaypoints(
            m_flightstatus->current_position_raw, *m_wpt, distance, bearing);
        bearing -= m_flightstatus->magvar;

        drawTextLeft(painter, 1, 9, QString("%1° /%2").
                     arg(bearing, 3, 'f', 0, QChar('0')).arg(distance, 0, 'f', 1), GREEN);
        drawTextRight(painter, 8, 9, "TO ");

        QString text = m_wpt->id();
        if (m_wpt->asAirport() != 0) text += m_wpt->asAirport()->activeRunwayId();
        drawTextRight(painter, 1, 9, text, CYAN);
    }

    drawTextLeft(painter, 1, 13, "1.00NM", GREEN);
    drawTextCenter(painter, 13, "HIGH", GREEN);
    drawTextRight(painter, 1, 13, "0.05NM", GREEN);
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAProgress::processAction(const QString& action)
{
    if (action == ACTION_CALLBACK_WPT_SELECT)
    {
        processWptSelectCallback();
        return;
    }

    int llsk_index = -1;
    int rlsk_index = -1;

    if (action.startsWith("LLSK"))
    {
        bool convok = false;
        llsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);
    }
    else if (action.startsWith("RLSK"))
    {
        bool convok = false;
        rlsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);
    }
    else
    {
        return;
    }

    QString text = m_page_manager->scratchpad().text();

    if (llsk_index == 1)
    {
        bool convok = false;
        uint cruise_fl = text.toUInt(&convok);
        if (!convok || cruise_fl > 490)
        {
            m_page_manager->scratchpad().setOverrideText("Invalid cruise FL");
            return;
        }
        
        fmcControl().normalRoute().setCruiseFl(cruise_fl);
        m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
    }
    else if (rlsk_index == 4)
    {
        if (m_page_manager->scratchpad().action() == FMCCDUPageBase::ACTION_CLR)
        {
            delete m_wpt;
            m_wpt = 0;
            m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
            return;
        }

        if (text.isEmpty()) return;

        QString wpt_name;
        double pbd_bearing = 0.0;
        double pbd_distance_nm = 0.0;
        bool got_pbd_wpt = parsePBDSyntax(text, wpt_name, pbd_bearing, pbd_distance_nm);
        if (!got_pbd_wpt) wpt_name = text;

        WaypointPtrList wpt_selection_list;
        fmcControl().navdata().getWaypoints(text, wpt_selection_list, LATLON_WAYPOINT_REGEXP);
            
        if (wpt_selection_list.count() <= 0)
        {
            m_page_manager->scratchpad().setOverrideText("No Waypoint found");
            return;
        }
        else if (wpt_selection_list.count() == 1)
        {
            delete m_wpt;
            m_wpt = 0;

            if (got_pbd_wpt) 
                m_wpt = fmcControl().getPBDWaypoint(*wpt_selection_list.at(0), pbd_bearing, pbd_distance_nm).deepCopy();
            else
                m_wpt = wpt_selection_list.at(0)->deepCopy();

            MYASSERT(m_wpt != 0);
        }
        else
        {
            wpt_selection_list.sortByDistance(m_flightstatus->current_position_raw);
            setWptSelectPage(wpt_selection_list, 0, pbd_bearing, pbd_distance_nm);
        }

        m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
    }

}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAProgress::wptSelectCallback(const Waypoint& wpt, uint wpt_insert_index)
{
    Q_UNUSED(wpt_insert_index);
    delete m_wpt;
    m_wpt = 0;
    m_wpt = wpt.deepCopy();
    MYASSERT(m_wpt != 0);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAFuelPred::paintPage(QPainter& painter) const
{
    painter.setBackground(QBrush(BLACK));
    painter.fillRect(painter.window(), painter.background());

    setFont(painter, NORM_FONT, QFont::Bold);
    drawTextCenter(painter, 1, "FUEL PRED");

    setFont(painter, SMALL_FONT);
    drawTextLeft(painter, 2, 6, "GW");
    drawTextRight(painter, 2, 6, "FOB");

    setFont(painter, NORM_FONT);
    drawTextLeft(painter, 1,  7, QString("%1").arg(m_flightstatus->total_weight_kg / 1000.0, 0, 'f', 1), GREEN);
    drawTextRight(painter, 1,  7, QString("%1/FF+FQ").arg(m_flightstatus->fuelOnBoard() / 1000.0, 0, 'f', 2), GREEN);
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAFuelPred::processAction(const QString& action)
{
    int llsk_index = -1;
    int rlsk_index = -1;

    if (action.startsWith("LLSK"))
    {
        bool convok = false;
        llsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);
    }
    else if (action.startsWith("RLSK"))
    {
        bool convok = false;
        rlsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);
    }
    else
    {
        return;
    }

    QString text = m_page_manager->scratchpad().text();
    if (text.isEmpty()) return;

    //TODO implement fuel page
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleASecFlightplan::paintPage(QPainter& painter) const
{
    painter.setBackground(QBrush(BLACK));
    painter.fillRect(painter.window(), painter.background());

    setFont(painter, NORM_FONT, QFont::Bold);
    drawTextCenter(painter, 1, "SEC INDEX");

    setFont(painter, NORM_FONT);
    
    drawTextLeft(painter, 1, 3, "<-COPY ACTIVE", CYAN);
    //TODOdrawTextLeft(painter, 1, 5, "<SEC F-PLN");

    if (fmcControl().secondaryRoute().count() > 0)
    {
        drawTextLeft(painter, 1, 7, "<-DELETE SEC", CYAN);
        drawTextLeft(painter, 1, 9, "*ACTIVATE SEC", ORANGE);
    }
    else
    {
        //TODOdrawTextRight(painter, 1, 3, "INIT>");
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleASecFlightplan::processAction(const QString& action)
{
    int llsk_index = -1;
    int rlsk_index = -1;

    if (action.startsWith("LLSK"))
    {
        bool convok = false;
        llsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);
    }
    else if (action.startsWith("RLSK"))
    {
        bool convok = false;
        rlsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);
    }
    else
    {
        return;
    }

    QString text = m_page_manager->scratchpad().text();
    //if (text.isEmpty()) return;

    if (llsk_index == 1)
    {
        fmcControl().secondaryRoute() = fmcControl().normalRoute();
    }
    else if (fmcControl().secondaryRoute().count() > 0)
    {
        if (llsk_index == 3)
        {
            fmcControl().secondaryRoute().clear();
        }
        else if (llsk_index == 4)
        {
            fmcControl().normalRoute() = fmcControl().secondaryRoute();
            m_page_manager->setCurrentPage(FMCCDUPageManagerStyleA::PAGE_FPLAN);
        }
    }
}

// End of file
