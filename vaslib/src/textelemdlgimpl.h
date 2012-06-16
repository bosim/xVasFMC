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

/*! \file    textelemdlgimpl.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __TEXTELEMDLGIMPL_H__
#define __TEXTELEMDLGIMPL_H__

#include <QDialog>


class ChartElemText;
class ChartElemTexts;
class QPointF;
class QRect;
class ProjectionBase;

#include "ui_textelemdlg.h"

//! Hold edit dialog implementation
class TextElemDlgImpl : public QDialog, private Ui::TextElemDlg
{
    Q_OBJECT

public:

    //! Standard Constructor
    TextElemDlgImpl(QWidget* parent, Qt::WFlags fl);

    //! Destructor
    virtual ~TextElemDlgImpl();

    int exec(ChartElemText& element);
    int exec(ChartElemTexts& element, const QPointF& lefttop, ProjectionBase* projection);

protected:

private:

    //! Hidden copy-constructor
    TextElemDlgImpl(const TextElemDlgImpl&);
    //! Hidden assignment operator
    const TextElemDlgImpl& operator = (const TextElemDlgImpl&);
};

#endif /* __TEXTELEMDLGIMPL_H__ */

// End of file

