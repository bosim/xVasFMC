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

/*! \file    infodlgimpl.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include <QFile>

#include "assert.h"

#include "infodlgimpl.h"

/////////////////////////////////////////////////////////////////////////////

InfoDlgImpl::InfoDlgImpl(QWidget* parent, Qt::WFlags fl) : QDialog(parent, fl) 
{
    setupUi(this);
    infotext->setFontFamily("Courier");
    infotext->setLineWrapMode(QTextEdit::NoWrap);
};

/////////////////////////////////////////////////////////////////////////////

InfoDlgImpl::~InfoDlgImpl() {};

/////////////////////////////////////////////////////////////////////////////    

void InfoDlgImpl::setHTML(const QString& info) { infotext->setHtml(info); }

/////////////////////////////////////////////////////////////////////////////

void InfoDlgImpl::setText(const QString& info) { infotext->setText(info); }

/////////////////////////////////////////////////////////////////////////////

bool InfoDlgImpl::loadHTMLFromFile(const QString& filename)
{
    MYASSERT(!filename.isEmpty());
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) return false;
    infotext->setHtml(QString(file.readAll()));
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool InfoDlgImpl::loadTextFromFile(const QString& filename)
{
    MYASSERT(!filename.isEmpty());
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) return false;
    infotext->setText(QString(file.readAll()));
    return true;
}

// End of file
