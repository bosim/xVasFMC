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

#include "vor.h"

/////////////////////////////////////////////////////////////////////////////
    
Vor::Vor() : Ndb(), m_has_dme(false)
{
    m_type = TYPE_VOR;
}

/////////////////////////////////////////////////////////////////////////////

Vor::Vor(const QString& id, const QString& name,
         const double& lat, const double& lon,
         int freq, bool has_dme, int range_nm, int elevation_ft,
         const QString& country_code) :
    Ndb(id, name, lat, lon, freq, range_nm, elevation_ft, country_code), m_has_dme(has_dme)
{
    m_type = TYPE_VOR;
}

/////////////////////////////////////////////////////////////////////////////

QString Vor::toString() const
{
    return QString("VOR %1, %2, %3, %4, %5nm, %6ft, %7, hasDME:%8").
        arg(m_id).arg(m_name).arg(latLonString()).
        arg(m_freq / 1000.0).arg(m_range_nm).arg(m_elevation_ft).arg(m_country_code).arg(m_has_dme);
}

/////////////////////////////////////////////////////////////////////////////

void Vor::operator<<(QDataStream& in)
{
    in >> m_has_dme;
    Ndb::operator<<(in);
}

/////////////////////////////////////////////////////////////////////////////

void Vor::operator>>(QDataStream& out) const
{
    out << m_has_dme;
    Ndb::operator>>(out);
}

/////////////////////////////////////////////////////////////////////////////
