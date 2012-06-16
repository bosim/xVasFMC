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

#include <QKeyEvent>

#include "vor.h"
#include "navcalc.h"

#include "waypointchoosedlgimpl.h"

// FMC Waypoint choose dlg
#define FMC_WPT_CHOOSE_COL_HEADER_ID "ID"
#define FMC_WPT_CHOOSE_COL_HEADER_TYPE "TYPE"
#define FMC_WPT_CHOOSE_COL_HEADER_FREQ "FREQ"
#define FMC_WPT_CHOOSE_COL_HEADER_NAME "NAME"
#define FMC_WPT_CHOOSE_COL_HEADER_CTRY "CTRY"
#define FMC_WPT_CHOOSE_COL_HEADER_LAT "LAT"
#define FMC_WPT_CHOOSE_COL_HEADER_LON "LON"
#define FMC_WPT_CHOOSE_COL_HEADER_DIST "DIST"

/////////////////////////////////////////////////////////////////////////////

WaypointChooseDlgImpl::WaypointChooseDlgImpl(QWidget* parent, Qt::WFlags fl) : 
	QDialog(parent,fl)
{
    setupUi(this);

    m_header_list.append(FMC_WPT_CHOOSE_COL_HEADER_ID);
    m_header_list.append(FMC_WPT_CHOOSE_COL_HEADER_TYPE);
    m_header_list.append(FMC_WPT_CHOOSE_COL_HEADER_FREQ);
    m_header_list.append(FMC_WPT_CHOOSE_COL_HEADER_NAME);
    m_header_list.append(FMC_WPT_CHOOSE_COL_HEADER_CTRY);
    m_header_list.append(FMC_WPT_CHOOSE_COL_HEADER_LAT);
    m_header_list.append(FMC_WPT_CHOOSE_COL_HEADER_LON);
    m_header_list.append(FMC_WPT_CHOOSE_COL_HEADER_DIST);
	
	fixlist->installEventFilter(this);
};

/////////////////////////////////////////////////////////////////////////////

WaypointChooseDlgImpl::~WaypointChooseDlgImpl() {};

/////////////////////////////////////////////////////////////////////////////

void WaypointChooseDlgImpl::setValuesFromWaypointList(const Waypoint& refrence_wpt,
													  const WaypointList& waypoint_list)
{
    fixlist->clear();
    fixlist->setColumnCount(m_header_list.count());
    fixlist->setRowCount(waypoint_list.count());
    fixlist->setHorizontalHeaderLabels(m_header_list);

	// insert waypoints into table in a sorted way

    long insert_counter = 0;
    QTableWidgetItem* item = 0;

	QSet<int> processed_index_set;
	m_insert_index_map.clear();

	while(processed_index_set.count() != waypoint_list.count())
	{
		double nearest_wpt_dist = 99999999;
		Waypoint* nearest_wpt = 0;
		int nearest_wpt_index = -1;
		
		for(int index = 0; index < waypoint_list.count(); ++index)
		{
			if (processed_index_set.contains(index)) continue;

			const Waypoint* wpt = waypoint_list[index];
			MYASSERT(wpt);

			double dist  = Navcalc::getDistBetweenWaypoints(refrence_wpt, *wpt);

			if (dist < nearest_wpt_dist)
			{
				nearest_wpt = wpt->copy();
				nearest_wpt_index = index;
				nearest_wpt_dist = dist;
			}
		}

		if (nearest_wpt != 0)
		{
			MYASSERT(nearest_wpt_index >= 0 && nearest_wpt_index < waypoint_list.count());

			m_insert_index_map.insert(insert_counter, nearest_wpt_index);
			processed_index_set.insert(nearest_wpt_index);

			item = new QTableWidgetItem(nearest_wpt->getId());
			MYASSERT(item);
			fixlist->setItem(insert_counter, 0, item);
			
			item = new QTableWidgetItem(nearest_wpt->getTypeString());
			MYASSERT(item);
			fixlist->setItem(insert_counter, 1, item);
			
			item = new QTableWidgetItem(nearest_wpt->getName());
			MYASSERT(item);
			fixlist->setItem(insert_counter, 3, item);
			
			item = new QTableWidgetItem(nearest_wpt->getLatString());
			MYASSERT(item);
			fixlist->setItem(insert_counter, 5, item);

			item = new QTableWidgetItem(nearest_wpt->getLonString());
			MYASSERT(item);
			fixlist->setItem(insert_counter, 6, item);
			
			switch(nearest_wpt->getType())
			{
				case(Waypoint::INTERSECTION): {
					
					item = new QTableWidgetItem(
						((Intersection*)nearest_wpt)->getCountryCode());
					MYASSERT(item);
					fixlist->setItem(insert_counter, 4, item);
					break;
				}
					
				case(Waypoint::VOR):
				case(Waypoint::NDB): {
					
					item = new QTableWidgetItem(
						((Ndb*)nearest_wpt)->getCountryCode());
					MYASSERT(item);
					fixlist->setItem(insert_counter, 4, item);
					
					item = new QTableWidgetItem(
						QString::number(((Ndb*)nearest_wpt)->getFreq() / 1000.0, 'f', 2));
					MYASSERT(item);
					fixlist->setItem(insert_counter, 2, item);
					break;
				}

				default: break;
			}

			item = new QTableWidgetItem(QString("%1nm").arg(nearest_wpt_dist, 0, 'f', 1));
			MYASSERT(item);
			fixlist->setItem(insert_counter, 7, item);

			++insert_counter;

			delete nearest_wpt;
			nearest_wpt = 0;
		}
	}

	fixlist->setRangeSelected(QTableWidgetSelectionRange(0, 0, 0, 7), true);
    fixlist->resizeRowsToContents();
    fixlist->resizeColumnsToContents();
}

/////////////////////////////////////////////////////////////////////////////

int WaypointChooseDlgImpl::getSelectedRowIndex() const
{
    QList<QTableWidgetSelectionRange> item_selection_range = fixlist->selectedRanges();
    if (item_selection_range.count() == 0) return -1;
    MYASSERT(item_selection_range.count() == 1);
    MYASSERT(item_selection_range.at(0).bottomRow() == item_selection_range.at(0).topRow());
	MYASSERT(m_insert_index_map.contains(item_selection_range.at(0).bottomRow()));
    return m_insert_index_map[item_selection_range.at(0).bottomRow()];
}

/////////////////////////////////////////////////////////////////////////////

bool WaypointChooseDlgImpl::eventFilter(QObject* object, QEvent* event)
{
	MYASSERT(object);
	MYASSERT(event);

	QWidget *w = (QWidget*)object;

	if (w == fixlist && event->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyevent = (QKeyEvent*)event;
		if (keyevent->key() == Qt::Key_Return) accept();
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////

// End of file
