///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2008 Alexander Wemmer 
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

/*! \file    statistics.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include <QFile>

#include "assert.h"
#include "logger.h"

#include "statistics.h"

/////////////////////////////////////////////////////////////////////////////

Statistics::Statistics(const QString& filename, const QString& separator) :
    m_stat_file(new QFile(filename)), m_separator(separator)
{
    MYASSERT(m_stat_file != 0);
    MYASSERT(m_stat_file->open(QIODevice::WriteOnly));
    MYASSERT(!m_separator.isEmpty());
}

/////////////////////////////////////////////////////////////////////////////

Statistics::~Statistics()
{
    delete m_stat_file;
}

/////////////////////////////////////////////////////////////////////////////

void Statistics::putItemInLine(const QString& item)
{
    m_stat_file->write((item+m_separator).toLatin1());
}

/////////////////////////////////////////////////////////////////////////////

void Statistics::nextLine()
{
    m_stat_file->write("\n", 1);
}

// End of file
