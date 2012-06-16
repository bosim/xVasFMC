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

#ifndef TRANSITION_H
#define TRANSITION_H

#include "procedure.h"

/////////////////////////////////////////////////////////////////////////////

class Transition : public Procedure
{
public:
    
    Transition();
    
    Transition(const QString& id, const QStringList& runway_list);
    
    virtual ~Transition() {};
    
    virtual const Transition* asTransition() const { return this; }
    virtual Transition* asTransition() { return this; }
    
    virtual QString toString() const;

    void setParentProcedure(Procedure* parent_procedure) { m_parent_procedure = parent_procedure; }
    const Procedure* parentProcedure() const { return m_parent_procedure; }
    Procedure* parentProcedure() { return m_parent_procedure; }

    virtual void operator<<(QDataStream& in);
    virtual void operator>>(QDataStream& out) const;

protected:

    Procedure* m_parent_procedure;
};

#endif
