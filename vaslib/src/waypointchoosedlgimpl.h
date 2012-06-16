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

/*! \file    waypointchoosedlgimpl.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __WAYPOINTCHOOSEDLGIMPL_H__
#define __WAYPOINTCHOOSEDLGIMPL_H__

#include <QStringList>

#include "waypoint.h"
#include "ui_waypointchoosedlg.h"

//! Hold edit dialog implementation
class WaypointChooseDlgImpl : public QDialog, private Ui::WaypointChooseDlg
{
    Q_OBJECT

public:

    //! Standard Constructor
    WaypointChooseDlgImpl(QWidget* parent, Qt::WFlags fl);

    //! Destructor
    virtual ~WaypointChooseDlgImpl();

    void setValuesFromWaypointList(const Waypoint& refrence_wpt,
								   const WaypointList& waypoint_list);
    
    int getSelectedRowIndex() const;

protected:

	bool eventFilter(QObject* object, QEvent* event);

    QStringList m_header_list;

 	QMap<int,int> m_insert_index_map;

private:

    //! Hidden copy-constructor
    WaypointChooseDlgImpl(const WaypointChooseDlgImpl&);
    //! Hidden assignment operator
    const WaypointChooseDlgImpl& operator = (const WaypointChooseDlgImpl&);
};

#endif /* __WAYPOINTCHOOSEDLGIMPL_H__ */

// End of file

