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

/*! \file    chartinfodlgimpl.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __CHARTINFODLGIMPL_H__
#define __CHARTINFODLGIMPL_H__

#include <QDialog>
#include <QString>

#include "ui_chart.info.dlg.h"

class ChartModel;

/////////////////////////////////////////////////////////////////////////////

//! Hold edit dialog implementation
class ChartInfoDlgImpl : public QDialog, private Ui::ChartInfoDlg
{
    Q_OBJECT
    
public:

    //! Standard Constructor
    ChartInfoDlgImpl(QWidget* parent, Qt::WFlags fl = 0);

    //! Destructor
    virtual ~ChartInfoDlgImpl();

    virtual int exec(ChartModel* model);
    
protected:

    virtual int exec() { return 0; };

private:

    //! Hidden copy-constructor
    ChartInfoDlgImpl(const ChartInfoDlgImpl&);
    //! Hidden assignment operator
    const ChartInfoDlgImpl& operator = (const ChartInfoDlgImpl&);
};

#endif /* __CHARTINFODLGIMPL_H__ */

// End of file

