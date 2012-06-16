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

#include "airport.h"

/////////////////////////////////////////////////////////////////////////////

Airport::Airport() : Waypoint(), m_elevation_ft(0) 
{
    m_type = TYPE_AIRPORT;
}

/////////////////////////////////////////////////////////////////////////////

Airport::Airport(const QString& id, const QString& name,
                 const double& lat, const double& lon, int elevation_ft) :
    Waypoint(id, name, lat, lon), m_elevation_ft(elevation_ft)
{
    m_type = TYPE_AIRPORT;
    m_default_airport_location = *this;
}

/////////////////////////////////////////////////////////////////////////////

QString Airport::toString() const
{
    return QString("Airport %1, %2, %3, %4ft").
        arg(m_id).arg(m_name).arg(latLonString()).arg(m_elevation_ft);
}
/////////////////////////////////////////////////////////////////////////////

void Airport::setActiveRunwayId(const QString& id) 
{
    m_active_runway_id = id; 

    if (activeRunway().isValid()) setPointLatLon(activeRunway().pointLatLon());
    else                          setPointLatLon(m_default_airport_location.pointLatLon());
}

/////////////////////////////////////////////////////////////////////////////

void Airport::operator<<(QDataStream& in)
{
    in >> m_elevation_ft
       >> m_runway_map
       >> m_active_runway_id;

    m_default_airport_location << in;

    Waypoint::operator<<(in);
}

/////////////////////////////////////////////////////////////////////////////

void Airport::operator>>(QDataStream& out) const
{
    out << m_elevation_ft
        << m_runway_map
        << m_active_runway_id;

    m_default_airport_location >> out;

    Waypoint::operator>>(out);
}

/////////////////////////////////////////////////////////////////////////////
