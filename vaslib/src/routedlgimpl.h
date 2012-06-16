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

/*! \file    routedlgimpl.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __ROUTEDLGIMPL_H__
#define __ROUTEDLGIMPL_H__

#include <QDialog>
#include <QStringList>
#include <QCloseEvent>
#include <QMoveEvent>

#include "config.h"
#include "waypoint.h"
#include "chart.elem.route.h"
#include "ge_route.h"
#include "route.h"
#include "ui_routedlg.h"

class ChartModel;

/////////////////////////////////////////////////////////////////////////////

//! Hold edit dialog implementation
class RouteDlgImpl : public QDialog, private Ui::RouteDlg
{
    Q_OBJECT
    
public:

    //! Standard Constructor
    RouteDlgImpl(Config* config, ChartModel* chart_model, QWidget* parent, Qt::WFlags fl);

    //! Destructor
    virtual ~RouteDlgImpl();

    void setRoute(ChartElemRoute* chart_elem_route);

    virtual void show();

    //! returns true on success, false otherwise.
    //! ATTENTION: The given fix must exists in the chart model's navaid element.
    bool appendFix(const QString& id, 
                   const QString& type, 
                   const QStringList& labels,
                   QString& err_msg);

protected slots:

    void slotRemoveLastFix();
    void slotRouteNameChanged();
    void slotRouteTypeChangedToSID(bool on);
    void slotRouteTypeChangedToSTAR(bool on);
    void slotRouteTypeChangedToAirway(bool on);
    void slotRouteTypeChangedToUnknown(bool on);
    void slotTableItemChanged(QTableWidgetItem* item);

protected:

    void closeEvent(QCloseEvent * event);
    void moveEvent(QMoveEvent* event);

    void updateParent();
    void close();

protected:

    Config* m_cfg;
    ChartModel* m_chart_model;

    QPoint m_last_pos;
    ChartElemRoute* m_chart_elem_route;
    QStringList m_header_list;
 	QMap<int,int> m_insert_index_map;

private:

    //! Hidden copy-constructor
    RouteDlgImpl(const RouteDlgImpl&);
    //! Hidden assignment operator
    const RouteDlgImpl& operator = (const RouteDlgImpl&);
};

#endif /* __ROUTEDLGIMPL_H__ */

// End of file

