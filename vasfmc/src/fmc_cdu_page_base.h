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

/*! \file    fmc_cdu_page_base.h
    \author  Alexander Wemmer, alex@wemmer.at
*/


#ifndef __FMC_CDU_PAGE_BASE_H__
#define __FMC_CDU_PAGE_BASE_H__

#include <QObject>
#include <QPointF>
#include <QFont>
#include <QDir>
#include <QApplication>
#include <QClipboard>
#include <QPainter>
#include <QFontMetrics>
#include <QColor>

#include "config.h"
#include "logger.h"

#include "defines.h"

class FMCCDUPageManager;
class FlightStatus;
class FMCData;
class FMCControl;
class Route;

/////////////////////////////////////////////////////////////////////////////

//! FMC CDU Page baseclass
class FMCCDUPageBase : public QObject
{
    Q_OBJECT

public:

    static const QString ACTION_PREV;
    static const QString ACTION_NEXT;
    static const QString ACTION_UP;
    static const QString ACTION_DOWN;

    static const QString ACTION_LLSK1;
    static const QString ACTION_LLSK2;
    static const QString ACTION_LLSK3;
    static const QString ACTION_LLSK4;
    static const QString ACTION_LLSK5;
    static const QString ACTION_LLSK6;

    static const QString ACTION_RLSK1;
    static const QString ACTION_RLSK2;
    static const QString ACTION_RLSK3;
    static const QString ACTION_RLSK4;
    static const QString ACTION_RLSK5;
    static const QString ACTION_RLSK6;

    static const QString ACTION_OVERFLY;
    static const QString ACTION_CLR;
    static const QString ACTION_CLRALL;
    static const QString ACTION_CLIPBOARD;
    static const QString ACTION_PLUSMINUS;

    static const QString ACTION_TOGGLE_KEYBOARD;

    //! Standard Constructor
    FMCCDUPageBase(const QString& page_name, FMCCDUPageManager* page_manager);
    
    //! Destructor
    virtual ~FMCCDUPageBase();

    //-----

    virtual void reset()
    {
        m_column_width = 0.0;
        m_line_height = 0.0;
    }

    inline const QString& name() const { return m_page_name; }

    //! may be overwritten by derived classes. If so this method shall return
    //! false if the page manager shall not switch to the page.
    virtual bool allowSwitchToPage() const { return true; }

    virtual void setActive(bool active);

    virtual void advanceVerticalScroll(int steps);
    virtual uint maxVerticalScrollOffset() const { return 0; }

    virtual void advanceHorizontalScroll(int steps);
    virtual uint maxHorizontalScrollOffset() const { return 0; }

    virtual void drawUpDownArrows(QPainter& painter) const;
    virtual void drawLeftRightArrows(QPainter& painter) const;
    virtual void drawNextPageArrow(QPainter& painter) const;

    virtual void drawKbdActiveIndicator(QPainter& painter) const;

    virtual void paintPage(QPainter& painter) const = 0;
    virtual void processAction(const QString& action) = 0;

protected slots:

    virtual void slotDataChanged(const QString& flag);

protected:

    void setFont(QPainter& painter, double font_scale = 1.0, int font_weight = QFont::Normal) const;
    QPointF getPoint(QPainter& painter, double column, double line) const;

    void drawTextLeft(QPainter& painter, double column, double line, 
                      const QString& text, const QColor& color = Qt::white) const;

    void drawTextRight(QPainter& painter, double column, double line, 
                       const QString& text, const QColor& color = Qt::white) const;

    void drawTextCenter(QPainter& painter, double line, 
                        const QString& text, const QColor& color = Qt::white) const;

    const FMCControl& fmcControl() const;
    FMCControl& fmcControl();

    //! parses the given text for a PBD syntax (WPT/BEAR/DIST), returns true on success.
    bool parsePBDSyntax(const QString& text, QString& wpt_name, double& pbd_bearing, double& pbd_distance_nm);

    //! processed up, down, prev, next actions and returns true if the given
    //action was handled, false otherwise.
    virtual bool processBasicActions(const QString& action);

    //! Sets the given LLSK and RLSK parameters according to the given action.
    //! Returns true if either LLSK or RLSK was set, false otherwise.
    virtual bool getLSKIndex(const QString& action, int &llsk_index, int &rlsk_index);

    //! returns true if we are airborne and the route is not empty
    virtual bool airborneAndRouteNotEmpty() const;

    //! draws the waypoints of the given route between the given lines
    virtual void drawRouteWaypoints(QPainter& painter, const Route& route, uint start_line, uint end_line) const;

    void setDrawHorizontalScrollPageCounter(bool yes) { m_draw_horizontal_scroll_page_counter = yes; }

protected:

    QString m_page_name;
    FMCCDUPageManager* m_page_manager;

    uint m_max_lines;
    uint m_max_columns;

    mutable double m_column_width;
    mutable double m_line_height;

    bool m_active;
    mutable int m_vertical_scroll_offset;
    mutable int m_horizontal_scroll_offset;
    bool m_draw_horizontal_scroll_page_counter;
  
    QString m_data_change_page_name_to_switch;

    FMCData& m_fmc_data;
    FlightStatus* m_flightstatus;
};

#endif /* __FMC_CDU_PAGE_BASE_H__ */

// End of file

