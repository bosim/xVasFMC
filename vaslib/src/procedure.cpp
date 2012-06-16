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

#include "assert.h"
#include "logger.h"

#include "procedure.h"

/////////////////////////////////////////////////////////////////////////////
    
Procedure::Procedure() : Route(QString::null)
{
    m_type = TYPE_PROCEDURE;
}
 
/////////////////////////////////////////////////////////////////////////////
   
Procedure::Procedure(const QString& id, 
                     const QStringList& runway_list) :
    Route(id), 
    m_runway_list(runway_list)
{
    m_type = TYPE_PROCEDURE;
}

/////////////////////////////////////////////////////////////////////////////

QString Procedure::toString() const
{
    return QString("Procedure: ID %1, RWYS %2").arg(m_id).arg(m_runway_list.join(","));
}

/////////////////////////////////////////////////////////////////////////////

void Procedure::operator<<(QDataStream& in)
{
    in >> m_runway_list;
    Route::operator<<(in);
}

/////////////////////////////////////////////////////////////////////////////

void Procedure::operator>>(QDataStream& out) const
{
    out << m_runway_list;
    Route::operator>>(out);
}

/////////////////////////////////////////////////////////////////////////////
