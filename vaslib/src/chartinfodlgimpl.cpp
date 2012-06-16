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

/*! \file    chartinfodlgimpl.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include <QLineEdit>

#include "assert.h"

#include "chartmodel.h"
#include "chart.elem.info.h"

#include "chartinfodlgimpl.h"

/////////////////////////////////////////////////////////////////////////////

ChartInfoDlgImpl::ChartInfoDlgImpl(QWidget* parent, Qt::WFlags fl) : QDialog(parent, fl) 
{
    setupUi(this);
};

/////////////////////////////////////////////////////////////////////////////

ChartInfoDlgImpl::~ChartInfoDlgImpl() {};

/////////////////////////////////////////////////////////////////////////////    

int ChartInfoDlgImpl::exec(ChartModel* model)
{
    MYASSERT(model != 0);

    ChartElemInfo* info_elem = model->getElemInfo();
    MYASSERT(info_elem != 0);

    airport_icao_edit->setText(info_elem->getAirportId());
    ctry_edit->setText(info_elem->getCountry());
    city_edit->setText(info_elem->getCity());
    
    chartname_edit->setText(info_elem->getChartName());
    effdate_edit->setDate(info_elem->getEffDate());
    magvar_spinbox->setValue(info_elem->getMagneticVariation());

    // setup dialog from the given info element

    int ret = QDialog::exec();

    if (ret == QDialog::Accepted)
    {
        // save values back to the info element
        
        info_elem->setAirportId(airport_icao_edit->text().toUpper().trimmed());
        info_elem->setCountry(ctry_edit->text().trimmed());
        info_elem->setCity(city_edit->text().trimmed());

        info_elem->setChartName(chartname_edit->text().trimmed());
        info_elem->setEffDate(effdate_edit->date());
        info_elem->setMagneticVariation(magvar_spinbox->value());
    }

    return ret;
}

/////////////////////////////////////////////////////////////////////////////


// End of file
