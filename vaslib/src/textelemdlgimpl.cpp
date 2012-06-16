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

#include <QPointF>
#include <QMessageBox>
#include <QRect>

#include "projection.h"
#include "chart.elem.text.h"
#include "chart.elem.texts.h"
#include "ge_textrect.h"

#include "textelemdlgimpl.h"

/////////////////////////////////////////////////////////////////////////////

TextElemDlgImpl::TextElemDlgImpl(QWidget* parent, Qt::WFlags fl) : 
	QDialog(parent,fl)
{
    setupUi(this);


};

/////////////////////////////////////////////////////////////////////////////

TextElemDlgImpl::~TextElemDlgImpl() {};

/////////////////////////////////////////////////////////////////////////////

int TextElemDlgImpl::exec(ChartElemText& element)
{
    GETextRect* textrect = element.getGETextRect();
    MYASSERT(textrect != 0);

    // set dialog fields from element

    text_edit->setPlainText(textrect->getText());
    draw_border_checkbox->setChecked(textrect->doDrawBorder());
    absolute_placing_checkbox->setChecked(!textrect->isRefPointLatLon());
    
    int ret = QDialog::exec();
    if (ret == QDialog::Accepted)
    {
        // set element fields from dialog

        textrect->setText(text_edit->toPlainText());
        textrect->setDrawBorder(draw_border_checkbox->isChecked());
        textrect->setRefPointLatLon(!absolute_placing_checkbox->isChecked());
    }
    
    return ret;
}

/////////////////////////////////////////////////////////////////////////////

int TextElemDlgImpl::exec(ChartElemTexts& element,
                          const QPointF& lefttop,
                          ProjectionBase* projection)
{
    MYASSERT(projection != 0);

    // set dialog fields from element

    int ret = QDialog::exec();
    if (ret == QDialog::Accepted)
    {
        // set element fields from dialog

        GETextRect::AbsoluteRefCorner abs_ref_corner = GETextRect::ABSREF_TOPLEFT;
        if (righttop_radiobtn->isChecked()) abs_ref_corner = GETextRect::ABSREF_TOPRIGHT;
        if (leftbottom_radiobtn->isChecked()) abs_ref_corner = GETextRect::ABSREF_BOTTOMLEFT;
        if (rightbottom_radiobtn->isChecked()) abs_ref_corner = GETextRect::ABSREF_BOTTOMRIGHT;
        
        QPointF my_lefttop;
        if (absolute_placing_checkbox->isChecked())
            projection->convertXYToLatLon(lefttop, my_lefttop);
//         else
//             my_lefttop

        QString err_msg;
        if (!element.addNewText(
                text_edit->toPlainText(),
                my_lefttop,
                absolute_placing_checkbox->isChecked(),
                abs_ref_corner,
                draw_border_checkbox->isChecked(),
                err_msg))
        {
            QMessageBox::warning(this, "Error: Add Text", err_msg, QMessageBox::Ok, 0);
            return QDialog::Rejected;
        }
    }
    
    return ret;
}

// End of file
