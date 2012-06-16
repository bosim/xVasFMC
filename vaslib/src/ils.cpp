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

#include "ils.h"

/////////////////////////////////////////////////////////////////////////////
    
Ils::Ils() : Vor(), m_course(0)
{
    m_type = TYPE_ILS;
}

/////////////////////////////////////////////////////////////////////////////

Ils::Ils(const QString& id, const QString& name,
         const double& lat, const double& lon,
         int freq, bool has_dme, int range_nm, int elevation_ft,
         const QString& country_code, int course) :
    Vor(id, name, lat, lon, freq, has_dme, range_nm, elevation_ft, country_code), m_course(course)
{
    m_type = TYPE_ILS;
}

/////////////////////////////////////////////////////////////////////////////

QString Ils::toString() const
{
    return QString("ILS %1, %2, %3, %4, %5nm, %6ft, %7, hasDME:%8, crs:%9").
        arg(m_id).arg(m_name).arg(latLonString()).
        arg(m_freq / 1000.0).arg(m_range_nm).arg(m_elevation_ft).arg(m_country_code).arg(m_has_dme).arg(m_course);
}

/////////////////////////////////////////////////////////////////////////////

void Ils::operator<<(QDataStream& in)
{
    in >> m_course;
    Vor::operator<<(in);
}

/////////////////////////////////////////////////////////////////////////////

void Ils::operator>>(QDataStream& out) const
{
    out << m_course;
    Vor::operator>>(out);
}

/////////////////////////////////////////////////////////////////////////////
