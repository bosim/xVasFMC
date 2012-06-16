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

/*! \file    infodlgimpl.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __INFODLGIMPL_H__
#define __INFODLGIMPL_H__

#include <QDialog>
#include <QString>

#include "ui_infodlg.h"

/////////////////////////////////////////////////////////////////////////////

//! Hold edit dialog implementation
class InfoDlgImpl : public QDialog, private Ui::InfoDlg
{
    Q_OBJECT
    
public:

    InfoDlgImpl(QWidget* parent, Qt::WFlags fl = 0);
    virtual ~InfoDlgImpl();
    
    void setHTML(const QString& info);
    void setText(const QString& info);

    //! loads the info html text from the given file, returns true on success.
    bool loadHTMLFromFile(const QString& filename);
    //! loads the info text from the given file, returns true on success.
    bool loadTextFromFile(const QString& filename);

private:

    //! Hidden copy-constructor
    InfoDlgImpl(const InfoDlgImpl&);
    //! Hidden assignment operator
    const InfoDlgImpl& operator = (const InfoDlgImpl&);
};

#endif /* __INFODLGIMPL_H__ */

// End of file

