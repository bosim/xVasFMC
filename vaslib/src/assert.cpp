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

#include "stdio.h"

#include <QString>
#include <QMessageBox>
#include <QApplication>

#include "assert.h"
#include "logger.h"

/////////////////////////////////////////////////////////////////////////////

void assertFailed(const char *file, const char* function, int line)
{
    Logger::log(QString("********** Assert failed in file %1, line %2, function %3\n").
                arg(file).arg(line).arg(function));

#if! VASFMC_GAUGE

    printf("********** Assert failed in file %s, line %d, function %s\n", file, line, function);
    fflush(stdout);

    QMessageBox::critical(0, "ASSERT FAILED", 
			  QString("Assert failed in file %1, line %2, function %3").
			  arg(QString(file)).arg(line).arg(QString(function)));

    qFatal(QString("********** Assert failed in file %1, line %2, function %3").
           arg(QString(file)).arg(line).arg(QString(function)).toLatin1().data());
    
	qApp->quit();
    exit(1);
#endif
}
