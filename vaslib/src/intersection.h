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

#ifndef INTERSECTION_H
#define INTERSECTION_H

#include "waypoint.h"

/////////////////////////////////////////////////////////////////////////////

class Intersection : public Waypoint
{
public:
    
    Intersection();

    Intersection(const QString& id,
                 const QString& name,
                 const double& lat, const double& lon,
                 const QString& country_code);
    
    virtual ~Intersection() {};

    virtual Waypoint* deepCopy() const { return new Intersection(*this); }

    inline virtual const Intersection* asIntersection() const { return this; }
    inline virtual Intersection* asIntersection() { return this; }

    QString toString() const;

    const QString& countryCode() const { return m_country_code; }

    virtual void operator<<(QDataStream& in);
    virtual void operator>>(QDataStream& out) const;

protected:

    QString m_country_code;
};

#endif
