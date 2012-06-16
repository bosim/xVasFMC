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

/*! \file    procedure_serialization.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "logger.h"

#ifndef __PROCEDURE_SERIALIZATION_H__
#define __PROCEDURE_SERIALIZATION_H__

#include "procedure.h"
#include "sid.h"
#include "star.h"
#include "approach.h"
#include "transition.h"

/////////////////////////////////////////////////////////////////////////////

//! the caller is responsible to delete the returned pointer (which may be NULL)
Procedure* getProcedureByType(const QString& proc_type);

/////////////////////////////////////////////////////////////////////////////

inline QDataStream &operator<<(QDataStream &stream, const ProcedurePtrList& list)
{
    stream << (int)list.count();
    for (int index=0; index < list.count(); ++index) 
    {
        stream << list.at(index)->type();
        *list.at(index) >> stream;
    }
    return stream;
}

/////////////////////////////////////////////////////////////////////////////

inline QDataStream &operator>>(QDataStream &stream,  ProcedurePtrList& list)
{
    int count;
    stream >> count;

    QString proc_type;

    list.clear();
    for (int index=0; index < count; ++index)
    {
        stream >> proc_type;

        Procedure* proc = getProcedureByType(proc_type);

        if (proc == 0)
        {
            Logger::log(QString("Procedure:operator>>: got unknown procedure type (%s) - aborting").
                        arg(proc_type));
            list.clear();
            return stream;
        }
                        
        list.append(proc);
        *proc << stream;
    }

    return stream;
}

#endif /* __PROCEDURE_SERIALIZATION_H__ */

// End of file

