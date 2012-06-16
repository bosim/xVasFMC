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

/*! \file    holdingeditdlgimpl.cpp
  \author  Alexander Wemmer, alex@wemmer.at
*/

#include <QMessageBox>
#include <QGroupBox>

#include "vor.h"
#include "navcalc.h"
#include "chartmodel.h"
#include "chart.elem.navaids.h"
#include "chart.xml.defines.h"

#include "routedlgimpl.h"

// FMC Waypoint choose dlg
#define FMC_WPT_CHOOSE_COL_HEADER_POS "POS"
#define FMC_WPT_CHOOSE_COL_HEADER_ID "ID"
#define FMC_WPT_CHOOSE_COL_HEADER_TYPE "TYPE"
#define FMC_WPT_CHOOSE_COL_HEADER_LABELS_NAME "NAME"
#define FMC_WPT_CHOOSE_COL_HEADER_ALT "ALT"

#define ROUTE_DLG_CONFIG_POS_X "routedlgposx"
#define ROUTE_DLG_CONFIG_POS_Y "routedlgposy"

/////////////////////////////////////////////////////////////////////////////

RouteDlgImpl::RouteDlgImpl(Config* config, ChartModel* chart_model, 
                           QWidget* parent, Qt::WFlags fl) : 
	QDialog(parent,fl), m_cfg(config), m_chart_model(chart_model), m_chart_elem_route(0)
{
    setupUi(this);

    MYASSERT(m_cfg != 0);
    MYASSERT(m_chart_model != 0);

    m_header_list.append(FMC_WPT_CHOOSE_COL_HEADER_POS);
    m_header_list.append(FMC_WPT_CHOOSE_COL_HEADER_ID);
    m_header_list.append(FMC_WPT_CHOOSE_COL_HEADER_TYPE);
    m_header_list.append(FMC_WPT_CHOOSE_COL_HEADER_LABELS_NAME);
    m_header_list.append(FMC_WPT_CHOOSE_COL_HEADER_ALT);
	
    if (m_cfg->contains(ROUTE_DLG_CONFIG_POS_X) && m_cfg->contains(ROUTE_DLG_CONFIG_POS_Y))
    {
        m_last_pos = QPoint(m_cfg->getIntValue(ROUTE_DLG_CONFIG_POS_X),
                            m_cfg->getIntValue(ROUTE_DLG_CONFIG_POS_Y));

        printf("RouteDlgImpl: Loaded pos: %d/%d\n", m_last_pos.x(), m_last_pos.y());
        fflush(stdout);
        
        if (!m_last_pos.isNull()) move(m_last_pos);
    }

    MYASSERT(connect(remove_last_item_btn, SIGNAL(clicked()),
                     this, SLOT(slotRemoveLastFix())));

    MYASSERT(connect(routename, SIGNAL(editingFinished()), this, SLOT(slotRouteNameChanged())));
    MYASSERT(connect(routetype_sid, SIGNAL(toggled(bool)), 
                     this, SLOT(slotRouteTypeChangedToSID(bool))));
    MYASSERT(connect(routetype_star, SIGNAL(toggled(bool)), 
                     this, SLOT(slotRouteTypeChangedToSTAR(bool))));
    MYASSERT(connect(routetype_airway, SIGNAL(toggled(bool)), 
                     this, SLOT(slotRouteTypeChangedToAirway(bool))));
    MYASSERT(connect(routetype_unknown, SIGNAL(toggled(bool)), 
                     this, SLOT(slotRouteTypeChangedToUnknown(bool))));

    MYASSERT(connect(routelist, SIGNAL(itemChanged(QTableWidgetItem*)),
                     this, SLOT(slotTableItemChanged(QTableWidgetItem*))));
};

/////////////////////////////////////////////////////////////////////////////

RouteDlgImpl::~RouteDlgImpl() 
{
    close();
    m_cfg->setValue(ROUTE_DLG_CONFIG_POS_X, m_last_pos.x());
    m_cfg->setValue(ROUTE_DLG_CONFIG_POS_Y, m_last_pos.y());    
};

/////////////////////////////////////////////////////////////////////////////

void RouteDlgImpl::show()
{
    hide();

    QDialog::show();

    if (m_chart_elem_route == 0) 
    {
        QMessageBox::critical(this, "Error: Route dialog", "No route set");
        return;
    }

    if (!m_last_pos.isNull()) move(m_last_pos);

    routename->setFocus();
    routename->selectAll();
}

/////////////////////////////////////////////////////////////////////////////

bool RouteDlgImpl::appendFix(const QString& id, 
                             const QString& type, 
                             const QStringList& labels,
                             QString& err_msg)
{
    err_msg = QString::null;

    MYASSERT(!id.isEmpty());
    MYASSERT(!type.isEmpty());
    MYASSERT(m_chart_elem_route != 0);
    MYASSERT(m_chart_elem_route->getGERoute());
    Route& route = m_chart_elem_route->getGERoute()->getRoute();
    unsigned int rows = routelist->rowCount();
    
    bool new_fix = route.count() == rows;

    // add the fix to the route
    if (new_fix)
    {
        ChartElemNavaids* elemnavaids = m_chart_model->getElemNavaids();
        if (elemnavaids == 0) 
        {
            err_msg = "Route dialog: Could not get the chart model's navaids element";
            return false;
        }

        const Waypoint* wpt = elemnavaids->getNavaid(id, type);
        if (wpt == 0) 
        {
            err_msg = "Route dialog: Could not find the fix in the chart models' navaid list";
            return false;
        }

        // check if such an entry already exists!!
        if (route.containsFix(*wpt)) 
        {
            err_msg = "Route dialog: This fix already exists in this route";
            return false;
        }

        route.addFix(*wpt, RouteFixData());
    }

    routelist->setRowCount(rows+1);
    QTableWidgetItem* item = 0;

    const RouteFixData& fixdata = route.getFixData(rows);

    // waypoint number
    item = new QTableWidgetItem(QString::number(rows));
    MYASSERT(item);
    routelist->setItem(rows, 0, item);
    item->setFlags(Qt::ItemIsEnabled);
	
    // waypoint id
    item = new QTableWidgetItem(id);
    MYASSERT(item);
    routelist->setItem(rows, 1, item);
    item->setFlags(Qt::ItemIsEnabled);
    
    // waypoint type
    item = new QTableWidgetItem(type);
    MYASSERT(item);
    routelist->setItem(rows, 2, item);
    item->setFlags(Qt::ItemIsEnabled);

    if (rows > 0)
    {
        // route name checkbox
        item = new QTableWidgetItem("");
        MYASSERT(item);
        item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        if (labels.contains(CHART_ATTR_LABEL_PARAM_ROUTE)) item->setCheckState(Qt::Checked);
        else item->setCheckState(Qt::Unchecked);
        if (new_fix && rows == 1) item->setCheckState(Qt::Checked);
        routelist->setItem(rows, 3, item);
    }

    // altitude restriction field
    item = new QTableWidgetItem(fixdata.m_alt_restriction);
    MYASSERT(item);
    routelist->setItem(rows, 4, item);
    item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled);
    
    routelist->resizeRowsToContents();
    routelist->resizeColumnsToContents();

    m_chart_model->setDirty();
    return true;
}

/////////////////////////////////////////////////////////////////////////////

void RouteDlgImpl::slotRemoveLastFix()
{
    if (m_chart_elem_route == 0) return;
    MYASSERT(m_chart_elem_route->getGERoute());
    Route& route = m_chart_elem_route->getGERoute()->getRoute();

    int count = route.count();
    MYASSERT(count == routelist->rowCount());
    if (count <= 0) return;

    route.deleteFix(count-1);
    routelist->removeRow(count-1);
    m_chart_model->setDirty();
    updateParent();
}

/////////////////////////////////////////////////////////////////////////////

void RouteDlgImpl::slotRouteNameChanged()
{
    if (m_chart_elem_route == 0) return; 
    
    if (routename->text().isEmpty() || 
        routename->text() == m_chart_elem_route->getName()) return;
    
    QString err_msg;
    if (!m_chart_elem_route->setName(routename->text(), err_msg))
    {
        QMessageBox::critical(this, "Error: Route dialog", 
                              QString("Could not rename route %1 to %2: %3").
                              arg(m_chart_elem_route->getName()).arg(routename->text()).
                              arg(err_msg));
   }

    routename->setText(m_chart_elem_route->getName());
    m_chart_model->setDirty();
    updateParent();
}

/////////////////////////////////////////////////////////////////////////////

void RouteDlgImpl::slotRouteTypeChangedToSID(bool on)
{
    if (!on) return;
    if (m_chart_elem_route == 0) return;
    MYASSERT(m_chart_elem_route->getGERoute());
    m_chart_elem_route->getGERoute()->getRoute().setType("SID");
    m_chart_model->setDirty();
    updateParent();
}

/////////////////////////////////////////////////////////////////////////////

void RouteDlgImpl::slotRouteTypeChangedToSTAR(bool on)
{
    if (!on) return;
    if (m_chart_elem_route == 0) return;
    MYASSERT(m_chart_elem_route->getGERoute());
    m_chart_elem_route->getGERoute()->getRoute().setType("STAR");
    m_chart_model->setDirty();
    updateParent();
}

/////////////////////////////////////////////////////////////////////////////

void RouteDlgImpl::slotRouteTypeChangedToAirway(bool on)
{
    if (!on) return;
    if (m_chart_elem_route == 0) return;
    MYASSERT(m_chart_elem_route->getGERoute());
    m_chart_elem_route->getGERoute()->getRoute().setType("AIRWAY");
    m_chart_model->setDirty();
    updateParent();
}

/////////////////////////////////////////////////////////////////////////////
    
void RouteDlgImpl::slotRouteTypeChangedToUnknown(bool on)
{
    if (!on) return;
    if (m_chart_elem_route == 0) return;
    MYASSERT(m_chart_elem_route->getGERoute());
    m_chart_elem_route->getGERoute()->getRoute().setType("UNKNOWN");
    m_chart_model->setDirty();
    updateParent();
}

/////////////////////////////////////////////////////////////////////////////

void RouteDlgImpl::slotTableItemChanged(QTableWidgetItem* item)
{
    if (m_chart_elem_route == 0) return;

    if (item == 0) return;
    int row = routelist->row(item);
    int column = routelist->column(item);
    
    bool changed = false;

    if (column == 3)
    {
        MYASSERT(row >= 0 && row < routelist->rowCount());
        MYASSERT(m_chart_elem_route->getGERoute());
        
        if (item->checkState() == Qt::Unchecked)
        {
            m_chart_elem_route->getGERoute()->getRoute().removeLabelEntryFromFixData(
                row, CHART_ATTR_LABEL_PARAM_ROUTE);
        }
        else if (item->checkState() == Qt::Checked)
        {
            m_chart_elem_route->getGERoute()->getRoute().addLabelEntryToFixData(
                row, CHART_ATTR_LABEL_PARAM_ROUTE);
        }

        changed = true;
    }
    else if (column == 4)
    {
        MYASSERT(row >= 0 && row < routelist->rowCount());
        MYASSERT(m_chart_elem_route->getGERoute());
        m_chart_elem_route->getGERoute()->getRoute().setFixDataAltRestriction(row, item->text());
        changed = true;
    }

    if (changed)
    {
        m_chart_model->setDirty();
        updateParent();
    }
}

/////////////////////////////////////////////////////////////////////////////

void RouteDlgImpl::setRoute(ChartElemRoute* chart_elem_route)
{
    if (m_chart_elem_route != 0)
    {
        slotRouteNameChanged();
        MYASSERT(m_chart_elem_route->getGERoute());
        m_chart_elem_route->getGERoute()->setUsed(false);
        m_chart_elem_route = 0;
    }

    // reset the item table
    routelist->clear();
    routelist->setColumnCount(m_header_list.count());
    routelist->setHorizontalHeaderLabels(m_header_list);
    routelist->setRowCount(0);

    if (chart_elem_route == 0) return;
    m_chart_elem_route = chart_elem_route;

    // extract the route

    MYASSERT(m_chart_elem_route->getGERoute());
    m_chart_elem_route->getGERoute()->setUsed(true);
    Route& route = m_chart_elem_route->getGERoute()->getRoute();

    // set the route name
   routename->setText(route.getName());

    // set the route type
    if (route.getType() == "STAR") routetype_star->setChecked(true);
    else if (route.getType() == "SID") routetype_sid->setChecked(true);
    else if (route.getType() == "AIRWAY") routetype_airway->setChecked(true);
    else routetype_unknown->setChecked(true);


	// insert waypoints into table in a sorted way

    for(unsigned int index=0; index < route.count(); ++index)
    {
        const Waypoint* wpt = route.getFix(index);
        const RouteFixData& fixdata = route.getFixData(index);
        MYASSERT(wpt != 0);
        QString err_msg;
        appendFix(wpt->getId(), wpt->getTypeString(), fixdata.m_label_list, err_msg);
	}
    
    MYASSERT((int)route.count() == routelist->rowCount());
        
    routelist->resizeRowsToContents();
    routelist->resizeColumnsToContents();
}

/////////////////////////////////////////////////////////////////////////////

void RouteDlgImpl::closeEvent(QCloseEvent *event)
{
    close();
    event->accept();
}

/////////////////////////////////////////////////////////////////////////////

void RouteDlgImpl::close()
{
    m_last_pos = pos();
    slotRouteNameChanged();

    if (m_chart_elem_route != 0)
    {
        MYASSERT(m_chart_elem_route->getGERoute());
        m_chart_elem_route->getGERoute()->setUsed(false);
        m_chart_elem_route = 0;
    }
}

/////////////////////////////////////////////////////////////////////////////

void RouteDlgImpl::moveEvent(QMoveEvent* event)
{
    m_last_pos = pos();
}

/////////////////////////////////////////////////////////////////////////////

void RouteDlgImpl::updateParent()
{
    MYASSERT(parent()->inherits("QWidget"));
    ((QWidget*)parent())->update();
}


// End of file
