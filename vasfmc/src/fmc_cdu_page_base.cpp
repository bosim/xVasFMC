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

/*! \file    fmc_cdu_page_base.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "waypoint.h"
#include "airport.h"
#include "sid.h"
#include "star.h"
#include "flightroute.h"

#include "fmc_control.h"
#include "fmc_cdu_page_manager.h"

#include "fmc_cdu_defines.h"
#include "fmc_cdu_page_base.h"

/////////////////////////////////////////////////////////////////////////////

const QString FMCCDUPageBase::ACTION_PREV = "PREV";
const QString FMCCDUPageBase::ACTION_NEXT = "NEXT";
const QString FMCCDUPageBase::ACTION_UP   = "UP";
const QString FMCCDUPageBase::ACTION_DOWN = "DOWN";

const QString FMCCDUPageBase::ACTION_LLSK1 = "LLSK1";
const QString FMCCDUPageBase::ACTION_LLSK2 = "LLSK2";
const QString FMCCDUPageBase::ACTION_LLSK3 = "LLSK3";
const QString FMCCDUPageBase::ACTION_LLSK4 = "LLSK4";
const QString FMCCDUPageBase::ACTION_LLSK5 = "LLSK5";
const QString FMCCDUPageBase::ACTION_LLSK6 = "LLSK6";

const QString FMCCDUPageBase::ACTION_RLSK1 = "RLSK1";
const QString FMCCDUPageBase::ACTION_RLSK2 = "RLSK2";
const QString FMCCDUPageBase::ACTION_RLSK3 = "RLSK3";
const QString FMCCDUPageBase::ACTION_RLSK4 = "RLSK4";
const QString FMCCDUPageBase::ACTION_RLSK5 = "RLSK5";
const QString FMCCDUPageBase::ACTION_RLSK6 = "RLSK6";

const QString FMCCDUPageBase::ACTION_OVERFLY = "OVERFLY";
const QString FMCCDUPageBase::ACTION_CLR = "CLR";
const QString FMCCDUPageBase::ACTION_CLRALL = "CLRALL";
const QString FMCCDUPageBase::ACTION_CLIPBOARD = "CLIPBOARD";
const QString FMCCDUPageBase::ACTION_PLUSMINUS = "PLUSMINUS";

const QString FMCCDUPageBase::ACTION_TOGGLE_KEYBOARD = "TOGGLE_KEYBOARD";

const QString FMCCDUPageStyleAWaypointSelectBase::ACTION_CALLBACK_WPT_SELECT = "CB_WPT_SELECT";
const QString FMCCDUPageStyleAFlightplan::ACTION_CALLBACK_DEPARTURE_SELECT = "CB_DEP_SELECT";

/////////////////////////////////////////////////////////////////////////////

FMCCDUPageBase::FMCCDUPageBase(const QString& page_name, FMCCDUPageManager* page_manager) : 
    QObject(page_manager), m_page_name(page_name), m_page_manager(page_manager), 
    m_max_lines(14), m_max_columns(24), 
    m_column_width(0.0), m_line_height(0.0),
    m_active(false), m_vertical_scroll_offset(0), m_horizontal_scroll_offset(0),
    m_draw_horizontal_scroll_page_counter(true), m_fmc_data(m_page_manager->fmcControl().fmcData()),
    m_flightstatus(m_page_manager->fmcControl().flightStatus())
{
    MYASSERT(!m_page_name.isEmpty());
    MYASSERT(m_page_manager != 0);

    MYASSERT(connect(&m_page_manager->fmcControl(), SIGNAL(signalDataChanged(const QString&)), 
                     this, SLOT(slotDataChanged(const QString&))));
}

/////////////////////////////////////////////////////////////////////////////

FMCCDUPageBase::~FMCCDUPageBase()
{
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageBase::setFont(QPainter& painter, double font_scale, int font_weight) const
{
    if (m_column_width == 0.0)
    {
        m_column_width = (double)painter.window().width() / m_max_columns;
        m_line_height =  (double)painter.window().height() / m_max_lines;
    }

    QFont current_font = QFont(m_page_manager->cduConfig().getValue(CFG_CDU_FONTNAME));

    if (fmcControl().cduDisplayOnlyMode())
        current_font.setPixelSize(
            (int)(m_page_manager->cduConfig().getIntValue(CFG_CDU_DISPLAY_ONLY_MODE_FONTSIZE) * font_scale));
    else
        current_font.setPixelSize(
            (int)(m_page_manager->cduConfig().getIntValue(CFG_CDU_FONTSIZE) * font_scale));

    current_font.setWeight(font_weight);
    current_font.setStyleStrategy(QFont::PreferQuality);
    painter.setFont(current_font);
}

/////////////////////////////////////////////////////////////////////////////

QPointF FMCCDUPageBase::getPoint(QPainter& painter, double column, double line) const
{
    if (m_column_width == 0.0)
    {
        m_column_width = (double)painter.window().width() / m_max_columns;
        m_line_height =  (double)painter.window().height() / m_max_lines;
    }

    return QPointF(1.0 + ((column-1) * m_column_width), -1.0 + (line * m_line_height));
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageBase::drawTextLeft(QPainter& painter, double column, double line,
                                  const QString& text, const QColor& color) const
{
    //TODO place letters directly via a loop here for extact placement !?

    painter.setPen(color);
    QPointF top_left = getPoint(painter, column, line-1);
    QPointF bottom_right = getPoint(painter, m_max_columns+1, line);
    painter.drawText(QRectF(top_left.x(), 
                            top_left.y(),
                            bottom_right.x()-top_left.x(),
                            bottom_right.y()-top_left.y()+2),
                     Qt::AlignLeft|Qt::AlignVCenter, text);
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageBase::drawTextRight(QPainter& painter, double column, double line, 
                                   const QString& text, const QColor& color) const
{
    //TODO place letters directly via a loop here for extact placement !?

    painter.setPen(color);
    QPointF top_left = getPoint(painter, 0, line-1);
    QPointF bottom_right = getPoint(painter, m_max_columns+1-column+0.5, line);
    painter.drawText(QRectF(top_left.x(), 
                            top_left.y(),
                            bottom_right.x()-top_left.x(),
                            bottom_right.y()-top_left.y()+2),
                     Qt::AlignRight|Qt::AlignVCenter, text);
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageBase::drawTextCenter(QPainter& painter, double line,
                                    const QString& text, const QColor& color) const
{
    //TODO place letters directly via a loop here for extact placement !?

    painter.setPen(color);
    QPointF top_left = getPoint(painter, 0, line-1);
    QPointF bottom_right = getPoint(painter, m_max_columns+0.5, line);
    painter.drawText(QRectF(top_left.x(), 
                            top_left.y(),
                            bottom_right.x()-top_left.x(),
                            bottom_right.y()-top_left.y()+2),
                     Qt::AlignHCenter|Qt::AlignVCenter, text);
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageBase::setActive(bool active)
{
    m_active = active;
    m_vertical_scroll_offset = 0;
    m_horizontal_scroll_offset = 0;
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageBase::advanceVerticalScroll(int steps)
{
    if (steps < 0)
        m_vertical_scroll_offset = qMax(0, m_vertical_scroll_offset+steps);
    else
        m_vertical_scroll_offset = qMin((int)maxVerticalScrollOffset(), m_vertical_scroll_offset+steps);
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageBase::advanceHorizontalScroll(int steps)
{
//    Logger::log(QString("advanceHorizontalScroll: cur=%1 st=%2").arg(m_horizontal_scroll_offset).arg(steps));
    m_horizontal_scroll_offset += steps;
//    Logger::log(QString("advanceHorizontalScroll: cur=%1 st=%2").arg(m_horizontal_scroll_offset).arg(steps));

    if (maxHorizontalScrollOffset() <= 0) 
        m_horizontal_scroll_offset = 0;
    else if (m_horizontal_scroll_offset < 0) 
        m_horizontal_scroll_offset = ((int)maxHorizontalScrollOffset()) + m_horizontal_scroll_offset + 1;
    else if (m_horizontal_scroll_offset > (int)maxHorizontalScrollOffset())
        m_horizontal_scroll_offset -= maxHorizontalScrollOffset() + 1;
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageBase::drawUpDownArrows(QPainter& painter) const
{
    QPen old_pen = painter.pen();
    QPen pen(WHITE);
    fmcControl().cduDisplayOnlyMode() ? pen.setWidth(2) : pen.setWidth(1);
    painter.setPen(pen);

    int length = Navcalc::round(m_line_height-3);
    int arrow_height = Navcalc::round(m_line_height / 5.0);
    int arrow_width = Navcalc::round(m_line_height / 6.0);

    int x = painter.window().width() - arrow_width - 3; 
    int y = painter.window().height() - length - 3;

    if (m_vertical_scroll_offset < (int)maxVerticalScrollOffset())
    {
        painter.drawLine(x, y, x, y+length);
        painter.drawLine(x, y, x+arrow_width, y+arrow_height);
        painter.drawLine(x, y, x-arrow_width, y+arrow_height);
        x -= (arrow_width * 2) + 2;
    }
    
    if (m_vertical_scroll_offset > 0)
    {
        painter.drawLine(x, y, x, y+length);
        painter.drawLine(x, y+length, x+arrow_width, y+length-arrow_height);
        painter.drawLine(x, y+length, x-arrow_width, y+length-arrow_height);
    }

    painter.setPen(old_pen);
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageBase::drawLeftRightArrows(QPainter& painter) const
{
    QPen old_pen = painter.pen();
    QPen pen(WHITE);
    fmcControl().cduDisplayOnlyMode() ? pen.setWidth(2) : pen.setWidth(1);
    painter.setPen(pen);

    int length = Navcalc::round(m_line_height-3);
    int arrow_height = Navcalc::round(m_line_height / 5.0);
    int arrow_width = Navcalc::round(m_line_height / 6.0);

    if (maxHorizontalScrollOffset() > 0)
    {
        int x = painter.window().width() - 3; 
        int y = arrow_width + 3;

        painter.drawLine(x, y, x-length, y);
        painter.drawLine(x, y, x-arrow_height, y-arrow_width);
        painter.drawLine(x, y, x-arrow_height, y+arrow_width);

//        y += arrow_width * 1.5;;
//        y = Navcalc::round(m_line_height - arrow_width - 3);
        y += (arrow_width * 2) + 2;
        painter.drawLine(x, y, x-length, y);
        x -= length;
        painter.drawLine(x, y, x+arrow_height, y-arrow_width);
        painter.drawLine(x, y, x+arrow_height, y+arrow_width);

        if (m_draw_horizontal_scroll_page_counter)
        {
            setFont(painter, SMALL_FONT);
            drawTextRight(painter, 4, 1, QString("%1/%2").
                          arg(m_horizontal_scroll_offset+1).arg(maxHorizontalScrollOffset()+1));
        }
    }

    painter.setPen(old_pen);
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageBase::drawNextPageArrow(QPainter& painter) const
{
    painter.setPen(WHITE);

    int length = 9;
    int arrow_height = 3;
    int arrow_width = 2;

    int x = painter.window().width() - length - 3;
    int y = arrow_width + 3;

    painter.drawLine(x, y, x-length, y);
    painter.drawLine(x, y, x-arrow_height, y-arrow_width);
    painter.drawLine(x, y, x-arrow_height, y+arrow_width);
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageBase::drawKbdActiveIndicator(QPainter& painter) const
{
    setFont(painter, SMALL_FONT);
    drawTextLeft(painter, 6, 1, "KBD", RED);
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageBase::slotDataChanged(const QString& flag)
{
    if (flag == Route::FLAG_TEMPORARY) return;

    if (m_active && !m_data_change_page_name_to_switch.isEmpty())
    {
        m_page_manager->scratchpad().setOverrideText("ROUTE CHANGED");
        m_page_manager->setCurrentPage(m_data_change_page_name_to_switch);
    }
}

/////////////////////////////////////////////////////////////////////////////

const FMCControl& FMCCDUPageBase::fmcControl() const { return m_page_manager->fmcControl(); }
FMCControl& FMCCDUPageBase::fmcControl() { return m_page_manager->fmcControl(); }

/////////////////////////////////////////////////////////////////////////////

bool FMCCDUPageBase::parsePBDSyntax(const QString& text, QString& wpt_name, double& pbd_bearing, double& pbd_distance_nm)
{
    wpt_name = text;
    pbd_bearing = 0.0;
    pbd_distance_nm = 0.0;

    if (text.contains("/") != 2) return false;

    wpt_name = text.section('/', 0, 0);
    if (wpt_name.isEmpty()) return false;
    
    bool convok = false;
    pbd_bearing = text.section('/', 1, 1).toDouble(&convok);
    if (!convok) return false;
    if (pbd_bearing < 0.0 || pbd_bearing > 360.0) return false;

    pbd_distance_nm = text.section('/', 2, 2).toDouble(&convok);
    if (!convok) return false;
    if (pbd_distance_nm <= 0.0) return false;

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool FMCCDUPageBase::processBasicActions(const QString& action)
{
    if (action == ACTION_UP)
    {
        advanceVerticalScroll(+1);
        return true;
    }
    else if (action == ACTION_DOWN)
    {
        advanceVerticalScroll(-1);
        return true;
    }
    else if (action == ACTION_PREV)
    {
        advanceHorizontalScroll(-1);
        return true;
    }
    else if (action == ACTION_NEXT)
    {
        advanceHorizontalScroll(+1);
        return true;
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////

bool FMCCDUPageBase::getLSKIndex(const QString& action, int &llsk_index, int &rlsk_index)
{
    llsk_index = -1;
    rlsk_index = -1;

    if (action.startsWith("LLSK"))
    {
        bool convok = false;
        llsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);
        return true;
    }
    else if (action.startsWith("RLSK"))
    {
        bool convok = false;
        rlsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);
        return true;
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////

bool FMCCDUPageBase::airborneAndRouteNotEmpty() const
{
    return (m_flightstatus->isValid() && !m_flightstatus->onground &&  fmcControl().normalRoute().count() >= 1);
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageBase::drawRouteWaypoints(QPainter& painter, const Route& route, uint start_line, uint end_line) const
{
    uint current_line = start_line;
    for(int index=m_vertical_scroll_offset*2; index<route.count(); index+=2)
    {
        const Waypoint* wpt = route.waypoint(index);
        if (wpt != 0)
        {
            if (!wpt->parent().isEmpty())
            {
                setFont(painter, SMALL_FONT);
                drawTextLeft(painter, 1, current_line, wpt->parent());
            }
            
            setFont(painter, NORM_FONT);
            drawTextLeft(painter, 6, current_line, wpt->id(), GREEN);
        }
        
        wpt = route.waypoint(index+1);
        if (wpt != 0)
        {
            if (!wpt->parent().isEmpty())
            {
                setFont(painter, SMALL_FONT);
                drawTextLeft(painter, 13, current_line, wpt->parent());
                }
            
            setFont(painter, NORM_FONT);
            drawTextLeft(painter, 18, current_line, wpt->id(), GREEN);
        }
        
        ++current_line;
        if (current_line > end_line) break;
    }
}

// End of file
