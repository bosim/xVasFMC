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

/*! \file    chartwidget.cpp
  \author  Alexander Wemmer, alex@wemmer.at
*/

//#include <QCloseEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QShortcut>
#include <QLineEdit>
#include <QTextEdit>

#include "assert.h"
#include "config.h"
#include "projection.h"
#include "navcalc.h"
#include "airport.h"
#include "route.h"
#include "waypointchoosedlgimpl.h"
#include "routedlgimpl.h"
#include "chartmodel.h"
#include "ge.h"
#include "chart.elem.info.h"
#include "chart.elem.navaid.h"
#include "chart.elem.navaids.h"
#include "chart.elem.route.h"
#include "chart.elem.routes.h"
#include "chart.elem.texts.h"
#include "chart.elem.text.h"
#include "ge_textrect.h"
#include "ge_vor.h"
#include "ge_ndb.h"
#include "ge_airport.h"
#include "ge_intersection.h"
#include "ge_route.h"
#include "infodlgimpl.h"
#include "chartinfodlgimpl.h"
#include "textelemdlgimpl.h"
#include "ui_latlonpointdlg.h"
#include "ui_pbdpointdlg.h"
#include "ui_textelemdlg.h"

#include "chartwidget.h"

/////////////////////////////////////////////////////////////////////////////

ChartWidget::ChartWidget(Config* config, QWidget* parent, ChartModel* chart_model) :
    QWidget(parent), m_cfg(config), m_chart_model(chart_model), m_route_dlg(0),
    m_normal_menu(0), m_ge_menu(0), m_last_selected_ge_element(0)
{
    MYASSERT(m_cfg != 0);
    MYASSERT(m_chart_model != 0);

    // connect to chart model

    MYASSERT(connect(m_chart_model, SIGNAL(signalClosing()), this, SLOT(slotModelClosing())));
    MYASSERT(connect(m_chart_model, SIGNAL(signalClosed()), this, SLOT(slotModelClosed())));
    MYASSERT(connect(m_chart_model, SIGNAL(signalChanged()), this, SLOT(slotModelChanged())));
    MYASSERT(connect(m_chart_model, SIGNAL(signalWaypointChoose(const WaypointList&, Waypoint**)),
                     this, SLOT(slotWaypointChoose(const WaypointList&, Waypoint**))));
    
    // setup dialogs

    m_dialog_flags = Qt::Dialog |
					 Qt::WindowTitleHint |
					 Qt::WindowSystemMenuHint;

    m_wpt_choose_dlg = new WaypointChooseDlgImpl(this, m_dialog_flags);
    MYASSERT(m_wpt_choose_dlg != 0);

    // setup projection

    ProjectionBase* projection = m_chart_model->getProjection();
    MYASSERT(projection != 0);
    unsigned int min_side = qMin(width(), height());
    projection->setScale(projection->getRangeNm(), min_side);

    // setup actions and menus

    createActions();
    createMenus();

    // misc

    setFocusPolicy(Qt::WheelFocus);
    setMouseTracking(true);
}

/////////////////////////////////////////////////////////////////////////////

ChartWidget::~ChartWidget()
{
    delete m_route_dlg;
    delete m_wpt_choose_dlg;
    delete m_normal_menu;
    delete m_ge_menu;
}

/////////////////////////////////////////////////////////////////////////////

void ChartWidget::createActions()
{
    m_action_edit_chartinfo = new QAction("Edit Chartinfo", this);
    MYASSERT(m_action_edit_chartinfo != 0);
    m_action_edit_chartinfo->setShortcut(tr("CTRL+C"));
    MYASSERT(connect(m_action_edit_chartinfo, SIGNAL(triggered()), 
                     this, SLOT(slotEditChartInfo())));
    m_shortcut_edit_chartinfo = new QShortcut(tr("CTRL+C"), this);
    MYASSERT(m_shortcut_edit_chartinfo != 0);
    MYASSERT(connect(m_shortcut_edit_chartinfo, SIGNAL(activated()), 
                     this, SLOT(slotEditChartInfo())));
    
    m_action_add_intersection = new QAction("Add Intersection", this);
    MYASSERT(m_action_add_intersection != 0);
    m_action_add_intersection->setShortcut(tr("CTRL+I"));
    MYASSERT(connect(m_action_add_intersection, SIGNAL(triggered()), 
                     this, SLOT(slotAddIntersection())));
    m_shortcut_add_intersection = new QShortcut(tr("CTRL+I"), this);
    MYASSERT(m_shortcut_add_intersection != 0);
    MYASSERT(connect(m_shortcut_add_intersection, SIGNAL(activated()), 
                     this, SLOT(slotAddIntersection())));
    
    m_action_add_latlonpoint = new QAction("Add LAT/LON point", this);
    MYASSERT(m_action_add_latlonpoint != 0);
    m_action_add_latlonpoint->setShortcut(tr("CTRL+P"));
    MYASSERT(connect(m_action_add_latlonpoint, SIGNAL(triggered()), 
                     this, SLOT(slotAddLatLonPoint())));
    m_shortcut_add_latlonpoint = new QShortcut(tr("CTRL+P"), this);
    MYASSERT(m_shortcut_add_latlonpoint != 0);
    MYASSERT(connect(m_shortcut_add_latlonpoint, SIGNAL(activated()), 
                     this, SLOT(slotAddLatLonPoint())));

    m_action_add_pbdpoint = new QAction("Add PBD point", this);
    MYASSERT(m_action_add_pbdpoint != 0);
    MYASSERT(connect(m_action_add_pbdpoint, SIGNAL(triggered()), 
                     this, SLOT(slotAddPBDPoint())));

    m_action_add_vor = new QAction("Add VOR", this);
    MYASSERT(m_action_add_vor != 0);
    m_action_add_vor->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_V));
    MYASSERT(connect(m_action_add_vor, SIGNAL(triggered()), this, SLOT(slotAddVOR())));
    m_shortcut_add_vor = new QShortcut(tr("CTRL+V"), this);
    MYASSERT(m_shortcut_add_vor != 0);
    MYASSERT(connect(m_shortcut_add_vor, SIGNAL(activated()), this, SLOT(slotAddVOR())));

    m_action_add_ndb = new QAction("Add NDB", this);
    MYASSERT(m_action_add_ndb != 0);
    m_action_add_ndb->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_N));
    MYASSERT(connect(m_action_add_ndb, SIGNAL(triggered()), this, SLOT(slotAddNDB())));
    m_shortcut_add_ndb = new QShortcut(tr("CTRL+N"), this);
    MYASSERT(m_shortcut_add_ndb != 0);
    MYASSERT(connect(m_shortcut_add_ndb, SIGNAL(activated()), this, SLOT(slotAddNDB())));

    m_action_add_airport = new QAction("Add Airport", this);
    MYASSERT(m_action_add_airport != 0);
    m_action_add_airport->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_A));
    MYASSERT(connect(m_action_add_airport, SIGNAL(triggered()), this, SLOT(slotAddAirport())));
    m_shortcut_add_airport = new QShortcut(tr("CTRL+A"), this);
    MYASSERT(m_shortcut_add_airport != 0);
    MYASSERT(connect(m_shortcut_add_airport, SIGNAL(activated()), this, SLOT(slotAddAirport())));

    m_action_add_route = new QAction("Add Route", this);
    MYASSERT(m_action_add_route != 0);
    m_action_add_route->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_R));
    MYASSERT(connect(m_action_add_route, SIGNAL(triggered()), this, SLOT(slotAddRoute())));
    m_shortcut_add_route = new QShortcut(tr("CTRL+R"), this);
    MYASSERT(m_shortcut_add_route != 0);
    MYASSERT(connect(m_shortcut_add_route, SIGNAL(activated()), this, SLOT(slotAddRoute())));

    m_action_add_textrect = new QAction("Add Text", this);
    MYASSERT(m_action_add_textrect != 0);
    m_action_add_textrect->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_T));
    MYASSERT(connect(m_action_add_textrect, SIGNAL(triggered()), this, SLOT(slotAddTextRect())));
    m_shortcut_add_textrect = new QShortcut(tr("CTRL+T"), this);
    MYASSERT(m_shortcut_add_textrect != 0);
    MYASSERT(connect(m_shortcut_add_textrect, SIGNAL(activated()), this, SLOT(slotAddTextRect())));

    m_action_edit_item = new QAction("Edit", this);
    MYASSERT(m_action_edit_item != 0);
    MYASSERT(connect(m_action_edit_item, SIGNAL(triggered()), this, SLOT(slotEditItem())));

    m_action_remove_item = new QAction("Remove", this);
    MYASSERT(m_action_remove_item != 0);
    MYASSERT(connect(m_action_remove_item, SIGNAL(triggered()), this, SLOT(slotRemoveItem())));
}

/////////////////////////////////////////////////////////////////////////////

void ChartWidget::createMenus()
{
    // setup normal menu
    m_normal_menu = new QMenu(this);
    MYASSERT(m_normal_menu != 0);
    m_normal_menu->addAction(m_action_add_intersection);
    m_normal_menu->addAction(m_action_add_vor);
    m_normal_menu->addAction(m_action_add_ndb);
    m_normal_menu->addAction(m_action_add_airport);
    m_normal_menu->addAction(m_action_add_latlonpoint);
    m_normal_menu->addAction(m_action_add_textrect);
    m_normal_menu->addSeparator();
    m_normal_menu->addAction(m_action_edit_chartinfo);
    m_normal_menu->addAction(m_action_add_route);

    // setup GE context menu
    m_ge_menu = new QMenu(this);
    MYASSERT(m_ge_menu != 0);
    m_ge_menu->addAction(m_action_add_pbdpoint);
    m_ge_menu->addAction(m_action_edit_item);
    m_ge_menu->addAction(m_action_remove_item);
}

/////////////////////////////////////////////////////////////////////////////

void ChartWidget::slotModelClosing()
{
    delete m_route_dlg;
    m_route_dlg = 0;
//     m_route_dlg->setRoute(0);
//     m_route_dlg->close();
}

/////////////////////////////////////////////////////////////////////////////

void ChartWidget::slotModelClosed()
{
}

/////////////////////////////////////////////////////////////////////////////

void ChartWidget::slotModelChanged()
{
    ProjectionBase* projection = m_chart_model->getProjection();
    MYASSERT(projection != 0);
    unsigned int min_side = qMin(width(), height());
    projection->setScale(projection->getRangeNm(), min_side);
    update();
}

/////////////////////////////////////////////////////////////////////////////a

void ChartWidget::slotEditChartInfo()
{
    ChartInfoDlgImpl* dlg = new ChartInfoDlgImpl(this, m_dialog_flags);
    MYASSERT(dlg != 0);
    if (dlg->exec(m_chart_model) == QDialog::Accepted)
        m_chart_model->setDirty();

    delete dlg;
    update();
}

/////////////////////////////////////////////////////////////////////////////a

void ChartWidget::slotAddVOR()
{
    bool ok = false;
    QString vor_id_string = 
        QInputDialog::getText(this, "Add VOR", "Enter the ICAO code of the VOR or a "
                              "comma separated list of codes:", QLineEdit::Normal, "", &ok);
    if (!ok || vor_id_string.trimmed().isEmpty()) return;

    QStringList vor_id_list = vor_id_string.split(",");
    QStringList::iterator iter = vor_id_list.begin();
    for (; iter != vor_id_list.end(); ++iter)
    {
        QString err_msg;

        QString id = (*iter).trimmed().toUpper();
        if (!m_chart_model->getElemNavaids()->addNewVor(id, err_msg))
            QMessageBox::warning(this, "Error: Add VOR", err_msg, QMessageBox::Ok, 0);
    }

    m_chart_model->setDirty();
}

/////////////////////////////////////////////////////////////////////////////a

void ChartWidget::slotAddNDB()
{
    bool ok = false;
    QString ndb_id_string = 
        QInputDialog::getText(this, "Add NDB", "Enter the ICAO code of the NDB or a "
                              "comma separated list of codes:", QLineEdit::Normal, "", &ok);
    if (!ok || ndb_id_string.trimmed().isEmpty()) return;

    QStringList ndb_id_list = ndb_id_string.split(",");
    QStringList::iterator iter = ndb_id_list.begin();
    for (; iter != ndb_id_list.end(); ++iter)
    {
        QString err_msg;
        QString id = (*iter).trimmed().toUpper();
        if (!m_chart_model->getElemNavaids()->addNewNdb(id, err_msg))
            QMessageBox::warning(this, "Error: Add NDB", err_msg, QMessageBox::Ok, 0);
    }

    m_chart_model->setDirty();
}

/////////////////////////////////////////////////////////////////////////////a

void ChartWidget::slotAddAirport()
{
    bool ok = false;
    QString airport_id_string = 
        QInputDialog::getText(this, "Add Airport", "Enter the ICAO code of the Airport or a "
                              "comma separated list of codes:", QLineEdit::Normal, "", &ok);
    if (!ok || airport_id_string.trimmed().isEmpty()) return;

    QStringList airport_id_list = airport_id_string.split(",");
    QStringList::iterator iter = airport_id_list.begin();
    for (; iter != airport_id_list.end(); ++iter)
    {
        QString err_msg;
        QString id = (*iter).trimmed().toUpper();
        if (!m_chart_model->getElemNavaids()->addNewAirport(id, err_msg))
            QMessageBox::warning(this, "Error: Add Airport", err_msg, QMessageBox::Ok, 0);
    }

    m_chart_model->setDirty();
}

/////////////////////////////////////////////////////////////////////////////a

void ChartWidget::slotAddIntersection()
{
    bool ok = false;
    QString intersection_id_string = 
        QInputDialog::getText(this, "Add Intersection", 
                              "Enter the ICAO code of the Intersection or a "
                              "comma separated list of codes:", QLineEdit::Normal, "", &ok);
    if (!ok || intersection_id_string.trimmed().isEmpty()) return;

    QStringList intersection_id_list = intersection_id_string.split(",");
    QStringList::iterator iter = intersection_id_list.begin();
    for (; iter != intersection_id_list.end(); ++iter)
    {
        QString err_msg;
        QString id = (*iter).trimmed().toUpper();
        if (!m_chart_model->getElemNavaids()->addNewIntersection(id, err_msg))
            QMessageBox::warning(this, "Error: Add Intersection", err_msg, QMessageBox::Ok, 0);
    }

    m_chart_model->setDirty();
}

/////////////////////////////////////////////////////////////////////////////a

void ChartWidget::slotAddLatLonPoint()
{
    QDialog *latlonpoint_dlg = new QDialog(this, m_dialog_flags);
    MYASSERT(latlonpoint_dlg != 0);
    Ui::LatLonPointDlg latlonpoint_dlg_ui;
    latlonpoint_dlg_ui.setupUi(latlonpoint_dlg);

    if (latlonpoint_dlg->exec() != QDialog::Accepted)
    {
        delete latlonpoint_dlg;        
        return;
    }

    QString err_msg;
    QString id = latlonpoint_dlg_ui.wpt_id_edit->text().toUpper().trimmed();
    QString latlon_string = latlonpoint_dlg_ui.lat_edit->text()+ "/"+
                            latlonpoint_dlg_ui.lon_edit->text();

    delete latlonpoint_dlg;

    if (id.isEmpty())
    {
        QMessageBox::warning(this, "Error: Add LatLonPoint", 
                             "Waypoint ID was empty", QMessageBox::Ok, 0);
        return;
    }

    Waypoint* wpt = Waypoint::getIntersectionFromLatLonString(id, latlon_string, err_msg);
    if (wpt == 0) 
    {
        QMessageBox::warning(this, "Error: Add LatLonPoint", err_msg, QMessageBox::Ok, 0);
        return;
    }

    if (!m_chart_model->getElemNavaids()->addNewLatLonPoint(
            wpt->getId(), wpt->getLat(), wpt->getLon(), err_msg))
    {
        QMessageBox::warning(this, "Error: Add LatLonPoint", err_msg, QMessageBox::Ok, 0);
    }

    m_chart_model->setDirty();
    delete wpt;
}

/////////////////////////////////////////////////////////////////////////////a

void ChartWidget::slotAddPBDPoint()
{
    QDialog *pbdpoint_dlg = new QDialog(this, m_dialog_flags);
    MYASSERT(pbdpoint_dlg != 0);
    Ui::PbdPointDlg pbdpoint_dlg_ui;
    pbdpoint_dlg_ui.setupUi(pbdpoint_dlg);

    if (pbdpoint_dlg->exec() != QDialog::Accepted)
    {
        delete pbdpoint_dlg;        
        return;
    }

    QString id = pbdpoint_dlg_ui.wpt_id_edit->text().toUpper().trimmed();
    int bearing = (int)((double)pbdpoint_dlg_ui.bearing_spinbox->value() -
                        m_chart_model->getElemInfo()->getMagneticVariation());
    double distance = pbdpoint_dlg_ui.distance_spinbox->value();

    delete pbdpoint_dlg;

    if (id.isEmpty()) 
    {
        QMessageBox::warning(this, "Error: Add PbdPoint", 
                             "Waypoint ID was empty", QMessageBox::Ok, 0);
        return;
    }
    
    const Waypoint* pbd_reference_waypoint = 0;

    if (m_last_selected_ge_element->inherits("GEVor"))
        pbd_reference_waypoint = ((GEVor*)m_last_selected_ge_element)->getVor();
    else if (m_last_selected_ge_element->inherits("GENdb"))
        pbd_reference_waypoint = ((GENdb*)m_last_selected_ge_element)->getNdb();
    else if (m_last_selected_ge_element->inherits("GEIntersection"))
        pbd_reference_waypoint = ((GEIntersection*)m_last_selected_ge_element)->getIntersection();
    else if (m_last_selected_ge_element->inherits("GEAirport"))
        pbd_reference_waypoint = ((GEAirport*)m_last_selected_ge_element)->getAirport();
    
    if (pbd_reference_waypoint == 0)
    {
        QMessageBox::warning(this, "Error: Add PbdPoint", 
                             "Could not find PBD reference waypoint", QMessageBox::Ok, 0);
        return;
    }

    Waypoint wpt;
    if (!Navcalc::getPBDWaypoint(*pbd_reference_waypoint, bearing, distance, wpt))
    {
        QMessageBox::warning(this, "Error: Add PbdPoint", 
                             "Could not create PBD waypoint", QMessageBox::Ok, 0);
        return;
    };

    QString err_msg;
    if (!m_chart_model->getElemNavaids()->addNewLatLonPoint(
            id, wpt.getLat(), wpt.getLon(), err_msg))
    {
        QMessageBox::warning(this, "Error: Add PbdPoint", err_msg, QMessageBox::Ok, 0);
    }

    m_chart_model->setDirty();
}

/////////////////////////////////////////////////////////////////////////////

void ChartWidget::slotAddRoute()
{
    ChartElemRoutes* elem_routes = m_chart_model->getElemRoutes();
    if (elem_routes == 0)
    {
        QMessageBox::critical(this, "Error: add route", "Could not get chart routes element");
        return;
    }

    QString err_msg;
    ChartElemRoute* elem_route = elem_routes->addRoute("New", "unknown", err_msg);
    if (!elem_route)
    {
        QMessageBox::critical(this, "Error: add route", err_msg);
        return;
    }

    //FIXXME
    printf("slotAddRoute: got route (%s) with %d elements\n", 
           elem_route->getName().toLatin1().data(),
           elem_route->getGERoute()->getRoute().count());
    fflush(stdout);

    m_chart_model->setDirty();
    delete m_route_dlg;
    m_route_dlg = new RouteDlgImpl(m_cfg, m_chart_model, this, m_dialog_flags);
    MYASSERT(m_route_dlg);
    m_route_dlg->setRoute(elem_route);
    m_route_dlg->show();
}

/////////////////////////////////////////////////////////////////////////////

void ChartWidget::slotAddTextRect()
{
    ChartElemTexts *texts_elem = m_chart_model->getElemTexts();
    if (!texts_elem) 
    {
        QMessageBox::warning(
            this, "Error: Add Text", "No text elem found in map", QMessageBox::Ok, 0);
        return;
    }

    QMessageBox::warning(
        this, "Error: Add Text", "Not implemented", QMessageBox::Ok, 0);
    return;

//     QPointF lefttop = m_last_mouse_rmb_click_window_position - QPoint(width()/2, height()/2);

//     TextElemDlgImpl dlg(this, m_dialog_flags);
//     if (dlg.exec(*texts_elem, lefttop, geometry(), m_chart_model->getProjection()) == 
//         QDialog::Accepted) m_chart_model->setDirty();
}

/////////////////////////////////////////////////////////////////////////////

void ChartWidget::slotEditItem()
{
    if (m_last_selected_ge_element == 0) return;
    TreeBase* element = m_last_selected_ge_element->getDataElement();
    if (element == 0) return;

    if (element->inherits("ChartElemRoute"))
    {
        ChartElemRoute* route_element = (ChartElemRoute*)element;
        delete m_route_dlg;
        m_route_dlg = new RouteDlgImpl(m_cfg, m_chart_model, this, m_dialog_flags);
        MYASSERT(m_route_dlg);
        m_route_dlg->setRoute(route_element);
        m_route_dlg->show();
    }
    else if (element->inherits("ChartElemText"))
    {
        ChartElemText* text = (ChartElemText*)element;
        TextElemDlgImpl dlg(this, m_dialog_flags);
        dlg.exec(*text);        
    }

    m_chart_model->setDirty();
}

/////////////////////////////////////////////////////////////////////////////

void ChartWidget::slotRemoveItem()
{
    if (m_last_selected_ge_element == 0) return;

    if (m_last_selected_ge_element->isUsed())
    {
        QMessageBox::critical(
            this, "Error: Remove item", 
            "Can not remove selected item, because it is still in use (edit dialog open?)");
        return;
    }
    
    TreeBase* element = m_last_selected_ge_element->getDataElement();
    if (element == 0) return;
    TreeBase* parent = element->getParent();
    if (!parent)
    {
        QMessageBox::critical(this, "Error: Remove item", "Item has no paren!?");
        return;
    }

    parent->removeAndDeleteLeaf(element);
    m_chart_model->setDirty();
}

/////////////////////////////////////////////////////////////////////////////

void ChartWidget::slotWaypointChoose(const WaypointList& wpt_list, Waypoint** chosen_waypoint)
{
    ProjectionBase* projection = m_chart_model->getProjection();
    MYASSERT(projection != 0);
    *chosen_waypoint = 0;
    m_wpt_choose_dlg->setValuesFromWaypointList(projection->getCenter(), wpt_list);
    if (m_wpt_choose_dlg->exec() != QDialog::Accepted) return;

    int selected_row = m_wpt_choose_dlg->getSelectedRowIndex();
    if (selected_row < 0) return;
    MYASSERT(selected_row >= 0);
    MYASSERT(selected_row < wpt_list.count());
    *chosen_waypoint = wpt_list.at(selected_row);
}

/////////////////////////////////////////////////////////////////////////////

void ChartWidget::resizeEvent(QResizeEvent * event)
{
    ProjectionBase* projection = m_chart_model->getProjection();
    MYASSERT(projection != 0);
    unsigned int min_side = qMin(width(), height());
    projection->setScale(projection->getRangeNm(), min_side);
    event->ignore();
}

/////////////////////////////////////////////////////////////////////////////

void ChartWidget::changeRange(bool increase)
{
    ProjectionBase* projection = m_chart_model->getProjection();
    MYASSERT(projection != 0);
    int range_nm = projection->getRangeNm();
    
    int diff = 0;
    if (range_nm > 500) diff = 150;
    if (range_nm > 200) diff = 75;
    if (range_nm > 100) diff = 25;
    else if (range_nm > 50) diff = 10;
    else if (range_nm > 10) diff = 5;
    else if (range_nm > 5) diff = 2;
    else diff = 1;
    
    if (!increase) diff = -diff;
    
    range_nm += diff;
    if (range_nm > 1000) range_nm = 1000;
    else if (range_nm < 5) range_nm = 5;
    
    unsigned int min_side = qMin(width(), height());
    projection->setScale(range_nm, min_side);
    update();
}

/////////////////////////////////////////////////////////////////////////////

void ChartWidget::changeCenter(QPoint center_xy)
{
    ProjectionBase* projection = m_chart_model->getProjection();
    MYASSERT(projection != 0);
    QPointF center_latlon;
    MYASSERT(projection->convertXYToLatLon(center_xy, center_latlon));
    Waypoint center_latlon_wpt;
    center_latlon_wpt.setLatLon(center_latlon.x(), center_latlon.y());
    projection->setCenter(center_latlon_wpt);
    update();
}

/////////////////////////////////////////////////////////////////////////////

void ChartWidget::keyPressEvent(QKeyEvent *event)
{
    unsigned int translation = 20;

    event->accept();
    int key = event->key();
    switch(key)
    {
        case(Qt::Key_PageUp): {
            changeRange(false);
            break;
        }
        case(Qt::Key_PageDown): {
            changeRange(true);
            break;
        }
        case(Qt::Key_Left): {
            changeCenter(QPoint(-translation,0));
            break;
        }
        case(Qt::Key_Right): {
            changeCenter(QPoint(+translation,0));
            break;
        }
        case(Qt::Key_Down): {
            changeCenter(QPoint(0,translation));
            break;
        }
        case(Qt::Key_Up): {
            changeCenter(QPoint(0,-translation));
            break;
        }
        default:{
            event->ignore();
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void ChartWidget::wheelEvent(QWheelEvent * event)
{
    changeRange(event->delta() < 0); 
    event->accept();
}

/////////////////////////////////////////////////////////////////////////////

void ChartWidget::mousePressEvent(QMouseEvent *event)
{
    event->accept();
    QPoint mouse_pos = event->pos() - QPoint(width()/2, height()/2);
    
    switch(event->button())
    {
        case(Qt::LeftButton): {

            QString err_msg;
            
            m_last_selected_ge_element = m_chart_model->getGELayerList()->mouseClick(mouse_pos);
            //GraphicElementBase* selected =
            //m_chart_model->getGELayerList()->mouseClick(mouse_pos);
            
            if (m_last_selected_ge_element != 0 && m_route_dlg != 0 && m_route_dlg->isVisible())
            {
                if (m_last_selected_ge_element->inherits("GEIntersection"))
                {
                    const Intersection* intersection =
                        ((GEIntersection*)m_last_selected_ge_element)->getIntersection();
                    MYASSERT(intersection != 0);
                    if (!m_route_dlg->appendFix(intersection->getId(), 
                                                intersection->getTypeString(), 
                                                QStringList(),
                                                err_msg))
                        QMessageBox::critical(this, "Error: Adding Intersection", err_msg);
                }
                else if (m_last_selected_ge_element->inherits("GEVor"))
                {
                    const Vor* vor = ((GEVor*)m_last_selected_ge_element)->getVor();
                    MYASSERT(vor != 0);
                    if (!m_route_dlg->appendFix(vor->getId(), 
                                                vor->getTypeString(), 
                                                QStringList(),
                                                err_msg))
                        QMessageBox::critical(this, "Error: Adding VOR", err_msg);
                }
                else if (m_last_selected_ge_element->inherits("GENdb"))
                {
                    const Ndb* ndb = ((GENdb*)m_last_selected_ge_element)->getNdb();
                    MYASSERT(ndb != 0);
                    if (!m_route_dlg->appendFix(ndb->getId(), 
                                                ndb->getTypeString(), 
                                                QStringList(),
                                                err_msg))
                        QMessageBox::critical(this, "Error: Adding NDB", err_msg);
                }
                else if (m_last_selected_ge_element->inherits("GEAirport"))
                {
                    const Airport* airport = ((GEAirport*)m_last_selected_ge_element)->getAirport();
                    MYASSERT(airport != 0);
                    if (!m_route_dlg->appendFix(airport->getId(), 
                                                airport->getTypeString(), 
                                                QStringList(),
                                                err_msg))
                        QMessageBox::critical(this, "Error: Adding Airport", err_msg);
                }
                //FIXXME label, etc.
            }

            update();
            break;
        }
        case(Qt::MidButton): {
         
            QApplication::setOverrideCursor(Qt::SizeAllCursor);
            m_last_mouse_mmb_click_global_position = event->globalPos();
            //changeCenter(mouse_pos);
            break;
        }
        case(Qt::RightButton): {
            
            m_last_selected_ge_element = m_chart_model->getGELayerList()->mouseClick(mouse_pos);
            m_last_mouse_rmb_click_window_position = event->pos();

            if (m_last_selected_ge_element != 0)
            {
                if (m_last_selected_ge_element->inherits("GERoute") ||
                    m_last_selected_ge_element->inherits("GETextRect"))
                    m_action_edit_item->setEnabled(true);
                else
                    m_action_edit_item->setEnabled(false);

                if (m_last_selected_ge_element->inherits("GEVor") ||
                    m_last_selected_ge_element->inherits("GENdb") ||
                    m_last_selected_ge_element->inherits("GEIntersection") ||
                    m_last_selected_ge_element->inherits("GEAirport"))
                    m_action_add_pbdpoint->setEnabled(true);
                else
                    m_action_add_pbdpoint->setEnabled(false);

                m_ge_menu->move(event->globalPos() - QPoint(+2,+2));
                m_ge_menu->exec();
                m_chart_model->getGELayerList()->unselectAll();
            }
            else
            {
                m_normal_menu->move(event->globalPos() - QPoint(+2,+2));
                m_normal_menu->exec();
            }
            break;
        }
        default: {
            event->ignore();
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void ChartWidget::mouseMoveEvent(QMouseEvent *event)
{
    event->accept();
    QPoint mouse_pos = event->pos() - QPoint(width()/2, height()/2);

    switch(event->buttons())
    {
        case(Qt::LeftButton): {
            if (m_last_selected_ge_element != 0) m_chart_model->setDirty();
            m_chart_model->getGELayerList()->mouseMove(mouse_pos, false);
            update();
            break;
        }
        case(Qt::MidButton): {
            QPoint mouse_move_diff = event->globalPos() - m_last_mouse_mmb_click_global_position;
            changeCenter(-mouse_move_diff);
            m_last_mouse_mmb_click_global_position = event->globalPos();
            break;
        }
        case(Qt::RightButton): {
            break;
        }
        case(Qt::NoButton): {
            m_chart_model->getGELayerList()->mouseMove(mouse_pos, true);
            update(); //FIXXME only update when anything changed
        }
        default: {
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void ChartWidget::mouseReleaseEvent(QMouseEvent * event)
{
    event->accept();
    QPoint mouse_pos = event->pos() - QPoint(width()/2, height()/2);
    m_chart_model->getGELayerList()->mouseRelease(mouse_pos);

    QApplication::restoreOverrideCursor();
}

/////////////////////////////////////////////////////////////////////////////

void ChartWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
//     painter.setRenderHint(QPainter::TextAntialiasing);
    painter.fillRect(rect(), QBrush(Qt::white));

    // set the screen

    QSize screen_size = size();
    int screen_width_half = screen_size.width()/2;
    int screen_height_half = screen_size.height()/2;
    
    painter.setWindow(-screen_width_half, -screen_height_half, 
                      screen_size.width(), screen_size.height());

    // setup

    painter.save();
    QFont font = painter.font();
    font.setBold(1);
    painter.setFont(font);

    unsigned int spacing = 3;
    unsigned int cell_line_width = 2;

    QPen pen = painter.pen();
    pen.setWidth(cell_line_width);
    painter.setPen(pen);

    QString text = "ABC";
    QRect text_bounding_rect = 
        painter.boundingRect(QRect(0, 0, 500, 500), Qt::AlignLeft | Qt::AlignTop, text);

    unsigned int header_height = (int)(2.5 * text_bounding_rect.height());

    // draw border

    QRect border_rect(-screen_width_half+spacing, -screen_height_half+spacing+header_height,
                      screen_size.width()-2*spacing, screen_size.height()-2*spacing-header_height);

    painter.drawRect(border_rect);

    // draw header

    ChartElemInfo* info_elem = m_chart_model->getElemInfo();
    if (info_elem)
    {
        QRect header_rect = QRect(-screen_width_half+spacing, -screen_height_half+spacing,
                                  screen_size.width()-2*spacing, header_height-2*spacing);

        QString second_left_line;

        ChartElemNavaids* navaids_elem = m_chart_model->getElemNavaids();
        if (navaids_elem != 0)
        {
            const Airport* airport = (Airport*)navaids_elem->getNavaid(
                info_elem->getAirportId(), Waypoint::AIRPORT);

            if (airport != 0)
            {
                MYASSERT(airport->getType() == Waypoint::AIRPORT);
                QString text = airport->getId()+"/"+airport->getName();
                painter.drawText(header_rect, Qt::AlignLeft|Qt::AlignTop, text);
                second_left_line = QString("Elev %1ft").arg(airport->getElevationFt());
            }
        }

        double variation = info_elem->getMagneticVariation();
        if (variation != 0.0)
        {
            if (!second_left_line.isEmpty()) second_left_line +=", ";
            second_left_line += QString("Var %1°%2").arg(qAbs(variation)).
                                arg((variation < 0)? "E" : "W");
        }
        painter.drawText(header_rect, Qt::AlignLeft|Qt::AlignBottom, second_left_line);
        
        QString text = info_elem->getCity()+", "+info_elem->getCountry();
        painter.drawText(header_rect, Qt::AlignRight|Qt::AlignTop, text);
        painter.drawText(header_rect, Qt::AlignRight|Qt::AlignBottom, info_elem->getChartName());

        pen = painter.pen();
        painter.setPen(QPen(Qt::red));
        painter.drawText(header_rect, Qt::AlignHCenter|Qt::AlignBottom, 
                         "! Not for real naviation - for simulation use only !");
        painter.setPen(pen);

        QDate eff_date = info_elem->getEffDate();
        if (eff_date.isValid())
            painter.drawText(header_rect, Qt::AlignHCenter|Qt::AlignTop, 
                             eff_date.toString("d MMM yy"));
    }

    // draw the chart

    painter.restore();
    painter.setClipRect(border_rect, Qt::ReplaceClip);
    painter.setClipping(true);
    font.setBold(0);
    font.setPointSize(font.pointSize() - 1);
    painter.setFont(font);
    m_chart_model->getGELayerList()->draw(painter);
}

/////////////////////////////////////////////////////////////////////////////

// void ChartWidget::closeEvent(QCloseEvent * event)
// {
//     m_route_dlg->close();
//     event->accept();
// }

// End of file
