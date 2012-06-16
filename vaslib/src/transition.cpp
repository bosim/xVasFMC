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

#include "logger.h"

#include "transition.h"
#include "procedure_serialization.h"

/////////////////////////////////////////////////////////////////////////////
    
Transition::Transition() : Procedure() 
{
    m_type = TYPE_TRANSITION;
};
 
/////////////////////////////////////////////////////////////////////////////
   
Transition::Transition(const QString& id, const QStringList& runway_list) :
    Procedure(id, runway_list), m_parent_procedure(0)
{
    m_type = TYPE_TRANSITION;
}

/////////////////////////////////////////////////////////////////////////////

QString Transition::toString() const
{
    return QString("Transition: ID %1, RWYS %2, LEGS %3").
        arg(m_id).arg(m_runway_list.join(",")).arg(count());
}

/////////////////////////////////////////////////////////////////////////////

void Transition::operator<<(QDataStream& in)
{
    m_parent_procedure = 0;
    Procedure::operator<<(in);
}

/////////////////////////////////////////////////////////////////////////////

void Transition::operator>>(QDataStream& out) const
{
    Procedure::operator>>(out);
}

/////////////////////////////////////////////////////////////////////////////
