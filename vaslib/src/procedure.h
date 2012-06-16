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

#ifndef PROCEDURE_H
#define PROCEDURE_H

#include <QString>
#include <QList>

#include "route.h"

/////////////////////////////////////////////////////////////////////////////

class Procedure : public Route
{
public:
    
    Procedure();
    
    Procedure(const QString& id, 
              const QStringList& runway_list);
    
    virtual ~Procedure() {};

    virtual const Procedure* asProcedure() const { return this; }
    virtual Procedure* asProcedure() { return this; }

    virtual QString toString() const;

    virtual void operator<<(QDataStream& in);
    virtual void operator>>(QDataStream& out) const;

    /////////////////////////////////////////////////////////////////////////////

    const QStringList& runwayList() const {  return m_runway_list; }
    void setRunwayList(const QStringList& runway_list) { m_runway_list = runway_list; }

protected:

    QStringList m_runway_list;
};

/////////////////////////////////////////////////////////////////////////////

typedef PtrList<Procedure> ProcedurePtrList;
typedef QListIterator<Procedure*> ProcedurePtrListIterator;

#endif
