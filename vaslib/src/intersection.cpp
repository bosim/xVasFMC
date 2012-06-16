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

#include "intersection.h"

/////////////////////////////////////////////////////////////////////////////
    
Intersection::Intersection() : Waypoint()
{
    m_type = TYPE_INTERSECTION;
}

/////////////////////////////////////////////////////////////////////////////

Intersection::Intersection(const QString& id,
                           const QString& name,
                           const double& lat, const double& lon,
                           const QString& country_code) :
    Waypoint(id, name, lat, lon), m_country_code(country_code.trimmed())
{
    m_type = TYPE_INTERSECTION;
}

/////////////////////////////////////////////////////////////////////////////

QString Intersection::toString() const
{
    return QString("INT %1, %2, %3, %4 ").
        arg(m_id).arg(m_name).arg(latLonString()).arg(m_country_code);
}

/////////////////////////////////////////////////////////////////////////////

void Intersection::operator<<(QDataStream& in)
{
    Waypoint::operator<<(in);
    in >> m_country_code;
}

/////////////////////////////////////////////////////////////////////////////

void Intersection::operator>>(QDataStream& out) const
{
    Waypoint::operator>>(out);
    out << m_country_code;
}
