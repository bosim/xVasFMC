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

#include "ndb.h"

/////////////////////////////////////////////////////////////////////////////
    
Ndb::Ndb() : Intersection(), m_freq(0), m_range_nm(0), m_elevation_ft(0)
{
    m_type = TYPE_NDB;
}

/////////////////////////////////////////////////////////////////////////////

Ndb::Ndb(const QString& id, const QString& name,
         const double& lat, const double& lon,
         int freq, int range_nm, int elevation_ft,
         const QString& country_code) :
    Intersection(id, name, lat, lon, country_code), m_freq(freq), m_range_nm(range_nm), m_elevation_ft(elevation_ft)
{
    m_type = TYPE_NDB;
}

/////////////////////////////////////////////////////////////////////////////

QString Ndb::toString() const
{
    return QString("NDB %1, %2, %3, %4, %5nm, %6ft, %7").
        arg(m_id).arg(m_name).arg(latLonString()).
        arg(m_freq / 1000.0).arg(m_range_nm).arg(m_elevation_ft).arg(m_country_code);
}

/////////////////////////////////////////////////////////////////////////////

void Ndb::operator<<(QDataStream& in)
{
    Intersection::operator<<(in);

    in >> m_freq
       >> m_range_nm
       >> m_elevation_ft;
}

/////////////////////////////////////////////////////////////////////////////

void Ndb::operator>>(QDataStream& out) const
{
    Intersection::operator>>(out);

    out << m_freq
        << m_range_nm
        << m_elevation_ft;
}

/////////////////////////////////////////////////////////////////////////////
