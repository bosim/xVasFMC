///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2006 Alexander Wemmer 
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

/*! \file    chartwidget.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __CHARTWIDGET_H__
#define __CHARTWIDGET_H__

#include <QWidget>
#include <QObject>

#include "ge_layers.h"
#include "waypoint.h"
#include "config.h"

class QPaintEvent;
class QResizeEvent;
class QWheelEvent;
class QMouseEvent;
class QKeyEvent;
class ChartModel;
class WaypointChooseDlgImpl;
class RouteDlgImpl;
class QMenu;
class QShortcut;

/////////////////////////////////////////////////////////////////////////////

//! chart widget
class ChartWidget : public QWidget
{
    Q_OBJECT

public:
    //! Standard Constructor
    ChartWidget(Config* config, QWidget* parent, ChartModel* chart_model);

    //! Destructor
    virtual ~ChartWidget();

protected:

    void resizeEvent(QResizeEvent * event);
    void paintEvent(QPaintEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);

signals:

    void signalMouseClick(QPoint point);

protected slots:

    void slotModelClosing();
    void slotModelClosed();
    void slotModelChanged();

    void slotWaypointChoose(const WaypointList& wpt_list, Waypoint** chosen_waypoint);

    void slotEditChartInfo();

    void slotAddVOR();
    void slotAddNDB();
    void slotAddAirport();
    void slotAddIntersection();
    void slotAddLatLonPoint();
    void slotAddPBDPoint();
    void slotAddRoute();
    void slotEditItem();
    void slotAddTextRect();
    void slotRemoveItem();

protected:

    void changeRange(bool increase);
    void changeCenter(QPoint center_xy);

    void createActions();
    void createMenus();

    //void closeEvent(QCloseEvent * event);

protected:

    Config* m_cfg;

    //! model of the chart
    ChartModel* m_chart_model;

    Qt::WindowFlags m_dialog_flags;
    WaypointChooseDlgImpl* m_wpt_choose_dlg; 
    RouteDlgImpl* m_route_dlg;
    
    QPoint m_last_mouse_mmb_click_global_position;
    QPoint m_last_mouse_rmb_click_window_position;

    QMenu* m_normal_menu;
    QMenu* m_ge_menu;

    QAction* m_action_edit_chartinfo;

    QAction* m_action_add_vor;
    QAction* m_action_add_ndb;
    QAction* m_action_add_airport;
    QAction* m_action_add_intersection;
    QAction* m_action_add_latlonpoint;
    QAction* m_action_add_pbdpoint;
    QAction* m_action_add_route;
    QAction* m_action_add_textrect;
    QAction* m_action_edit_item;
    QAction* m_action_remove_item;

    QShortcut* m_shortcut_edit_chartinfo;
    QShortcut* m_shortcut_add_vor;
    QShortcut* m_shortcut_add_ndb;
    QShortcut* m_shortcut_add_airport;
    QShortcut* m_shortcut_add_intersection;
    QShortcut* m_shortcut_add_latlonpoint;
    QShortcut* m_shortcut_add_route;
    QShortcut* m_shortcut_add_textrect;

    GraphicElementBase* m_last_selected_ge_element;

private:
    //! Hidden copy-constructor
    ChartWidget(const ChartWidget&);
    //! Hidden assignment operator
    const ChartWidget& operator = (const ChartWidget&);
};

#endif /* __CHARTWIDGET_H__ */

// End of file
