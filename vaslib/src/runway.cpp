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

#include "runway.h"

/////////////////////////////////////////////////////////////////////////////

Runway::Runway() : Waypoint(), m_hdg(0), m_length_m(0), m_has_ils(false),
                   m_ils_freq(0), m_ils_hdg(0), m_gs_angle(0), 
                   m_threshold_elevation_ft(0), m_threshold_overflying_height_ft(0)
{
    m_type = TYPE_RUNWAY;
}

/////////////////////////////////////////////////////////////////////////////

Runway::Runway(const QString& id, double lat, double lon,
               int hdg, int length_m, bool has_ils, int ils_freq, 
               int ils_hdg, int threshold_elevation_ft, 
               int gs_angle, int threshold_overflying_height_ft) :
    Waypoint(id, QString::null, lat, lon), m_hdg(hdg), m_length_m(length_m), m_has_ils(has_ils),
    m_ils_freq(ils_freq), m_ils_hdg(ils_hdg), m_gs_angle(gs_angle), 
    m_threshold_elevation_ft(threshold_elevation_ft), m_threshold_overflying_height_ft(threshold_overflying_height_ft)
{           
    m_type = TYPE_RUNWAY;
}

/////////////////////////////////////////////////////////////////////////////

QString Runway::toString() const
{
    return QString("Runway %1, %2°, %3m, "
                   "ILS: %4/%5/%6°, "
                   "GS:%7°, THR elev: %8ft, "
                   "THR o.height: %9ft").
        arg(m_id).arg(m_hdg).arg(m_length_m).
        arg(m_has_ils).arg(m_ils_freq/1000.0).arg(m_ils_hdg).
        arg(m_gs_angle / 100.0).arg(m_threshold_elevation_ft).
        arg(m_threshold_overflying_height_ft);
}
    
/////////////////////////////////////////////////////////////////////////////

void Runway::operator<<(QDataStream& in)
{
    in >> m_hdg
       >> m_length_m
       >> m_has_ils
       >> m_ils_freq
       >> m_ils_hdg
       >> m_gs_angle
       >> m_threshold_elevation_ft
       >> m_threshold_overflying_height_ft;

    Waypoint::operator<<(in);
}

/////////////////////////////////////////////////////////////////////////////

void Runway::operator>>(QDataStream& out) const
{
    out << m_hdg
        << m_length_m
        << m_has_ils
        << m_ils_freq
        << m_ils_hdg
        << m_gs_angle
        << m_threshold_elevation_ft
        << m_threshold_overflying_height_ft;

    Waypoint::operator>>(out);
}

/////////////////////////////////////////////////////////////////////////////
