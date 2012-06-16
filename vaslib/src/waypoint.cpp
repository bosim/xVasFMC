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
#include "navcalc.h"

#include "waypoint.h"

/////////////////////////////////////////////////////////////////////////////

QString WaypointRestrictions::speedAndAltitudeRestrictionText() const 
{
    QString text;
    if (m_speed_restriction_kts > 0) text += QString::number(m_speed_restriction_kts) + "/";
    text += altitudeRestrictionText();
    return text;
}

/////////////////////////////////////////////////////////////////////////////

QString WaypointRestrictions::speedRestrictionText() const
{
    return QString::number(m_speed_restriction_kts);
}

/////////////////////////////////////////////////////////////////////////////

QString WaypointRestrictions::altitudeRestrictionText() const 
{
    if (altitudeGreaterRestriction())      return QString("+") + QString::number(m_altitude_restriction_ft);
    else if (altitudeSmallerRestriction()) return QString("-") + QString::number(m_altitude_restriction_ft);
    else                                   return QString::number(m_altitude_restriction_ft);
}

/////////////////////////////////////////////////////////////////////////////

bool WaypointRestrictions::setSpeedAndAltitudeRestrictionFromText(const QString& text)
{
    uint speed = 0;
    QString alt_text;
    bool convok = false;
    
    if (text.contains("/"))
    {
        speed = text.section('/', 0, 0).toUInt(&convok);
        if (!convok || speed > 999 || speed < 50) return false;
        
        if (!text.endsWith("/")) alt_text = text.section('/', 1, 1);
    }
    else
    {
        alt_text = text;
    }
    
    uint alt = 0;
    if (!alt_text.isEmpty()) alt = qAbs(alt_text.toInt(&convok));
    if (!convok || alt > 99999) return false;
    
    if (speed != 0) m_speed_restriction_kts = speed;
    if (alt != 0)
    {
        m_altitude_restriction_ft = alt;
        if (alt_text.startsWith("-"))      m_altitude_restriction_type = WaypointRestrictions::RESTRICTION_ALT_SMALLER;
        else if (alt_text.startsWith("+")) m_altitude_restriction_type = WaypointRestrictions::RESTRICTION_ALT_GREATER;
        else                           m_altitude_restriction_type = WaypointRestrictions::RESTRICTION_ALT_EQUAL;
    }
    
    return true;
}

/////////////////////////////////////////////////////////////////////////////

void WaypointRestrictions::operator>>(QDataStream& out) const
{
    out << m_speed_restriction_kts
        << m_altitude_restriction_ft
        << (qint16)m_altitude_restriction_type
        << m_overfly_restriction
        << (qint16)m_turn_restriction;
}

/////////////////////////////////////////////////////////////////////////////

void WaypointRestrictions::operator<<(QDataStream& in)
{
    qint16 altitude_restriction_type, turn_restriction;

    in >> m_speed_restriction_kts
       >> m_altitude_restriction_ft
       >> altitude_restriction_type
       >> m_overfly_restriction
       >> turn_restriction;
    
    m_altitude_restriction_type= (AltitudeResstrictionType)altitude_restriction_type;
    m_turn_restriction = (TurnRestrictionType)turn_restriction;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void WaypointMetaData::operator>>(QDataStream& out) const
{
    out << m_speed_kts
        << m_altitude_ft;
}

/////////////////////////////////////////////////////////////////////////////

void WaypointMetaData::operator<<(QDataStream& in)
{
    in >> m_speed_kts
       >> m_altitude_ft;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

const double Waypoint::LAT_LON_COMPARE_EPSILON = 0.00001;

QString Waypoint::TYPE_WAYPOINT = "WAYPOINT";
QString Waypoint::TYPE_AIRPORT = "AIRPORT";
QString Waypoint::TYPE_RUNWAY = "RUNWAY";
QString Waypoint::TYPE_INTERSECTION = "INTERSECTION";
QString Waypoint::TYPE_NDB = "NDB";
QString Waypoint::TYPE_VOR = "VOR";
QString Waypoint::TYPE_ILS = "ILS";
QString Waypoint::TYPE_ALL = "ALL";
QString Waypoint::TYPE_HDG_TO_ALT = "HDG2ALT";
QString Waypoint::TYPE_HDG_TO_INTERCEPT = "HDG2INTERCEPT";

QString Waypoint::FLAG_ADEP = "ADEP";
QString Waypoint::FLAG_SID = "SID";
QString Waypoint::FLAG_SID_TRANS = "SID_TRANS";
QString Waypoint::FLAG_TOP_OF_CLIMB = "T/C";
QString Waypoint::FLAG_END_OF_DESCENT = "E/D";
QString Waypoint::FLAG_TOP_OF_DESCENT = "T/D";
QString Waypoint::FLAG_STAR = "STAR";
QString Waypoint::FLAG_APP_TRANS = "APP_TRANS";
QString Waypoint::FLAG_APPROACH = "APP";
QString Waypoint::FLAG_ADES = "ADES";
QString Waypoint::FLAG_MISSED_APPROACH = "MISSED";
QString Waypoint::FLAG_DCT = "DCT";
QString Waypoint::FLAG_DISCONTINUITY = "DISC";

/////////////////////////////////////////////////////////////////////////////

Waypoint::Waypoint() :
    m_is_valid(false), m_type(TYPE_WAYPOINT)
{
}
  
/////////////////////////////////////////////////////////////////////////////
    
Waypoint::Waypoint(const QString& id, const QString& name, const double &lat, const double& lon) :
    m_is_valid(true), m_type(TYPE_WAYPOINT), m_id(id.trimmed()), m_name(name.trimmed()), 
    m_polar_coordinates(QPointF(lat, lon))
{
}

/////////////////////////////////////////////////////////////////////////////

QString Waypoint::toString() const
{
    return QString("Waypoint: %1, %2, %3, %4").arg(m_type).arg(m_id).arg(m_name).arg(latLonString());
}

/////////////////////////////////////////////////////////////////////////////

QString Waypoint::latString() const
{
    if (m_polar_coordinates.x() < 0) return QString("S") + QString::number(-m_polar_coordinates.x(), 'f', 7);
    else                             return QString("N") + QString::number(m_polar_coordinates.x(), 'f', 7);
}

/////////////////////////////////////////////////////////////////////////////

QString Waypoint::latStringDegMinSec() const
{
    double minutes = 60.0 * (lat() - ((int)lat()));
    int seconds = (int)((minutes - ((int)minutes)) * 100);
    QString lat_string =  QString("%1°%2.%3").arg(qAbs((int)lat()), 2, 10, QChar('0')).
                          arg(qAbs((int)minutes), 2, 10, QChar('0')).arg(qAbs(seconds), 2, 10, QChar('0'));
    (lat() < 0) ? lat_string += "S" : lat_string += "N";
    return lat_string;
}

/////////////////////////////////////////////////////////////////////////////

QString Waypoint::lonString() const
{
    if (m_polar_coordinates.y() < 0) return QString("W") + QString::number(-m_polar_coordinates.y(), 'f', 7);
    else                             return QString("E") + QString::number(m_polar_coordinates.y(), 'f', 7);
}

/////////////////////////////////////////////////////////////////////////////

QString Waypoint::lonStringDegMinSec() const
{
    double minutes = 60.0 * (lon() - ((int)lon()));
    int seconds = (int)((minutes - ((int)minutes)) * 100);
    QString lon_string =  QString("%1°%2.%3").arg(qAbs((int)lon()), 3, 10, QChar('0')).
                          arg(qAbs((int)minutes), 2, 10, QChar('0')).arg(qAbs(seconds), 2, 10, QChar('0'));
    (lon() < 0) ? lon_string += "W" : lon_string += "E";
    return lon_string;
}

/////////////////////////////////////////////////////////////////////////////

bool Waypoint::operator==(const Waypoint& other) const
{
    if (m_id != other.m_id) return false;
    if (qAbs(m_polar_coordinates.x() - other.m_polar_coordinates.x()) > LAT_LON_COMPARE_EPSILON) return false;
    if (qAbs(m_polar_coordinates.y() - other.m_polar_coordinates.y()) > LAT_LON_COMPARE_EPSILON) return false;
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool Waypoint::operator!=(const Waypoint& other) const
{
    return ! operator==(other);

}

/////////////////////////////////////////////////////////////////////////////

const Waypoint& Waypoint::operator=(const Waypoint& other)
{
    m_is_valid = other.m_is_valid;
    m_id = other.m_id;
    m_name = other.m_name;
    m_parent = other.m_parent;
    m_flag = other.m_flag;
    
    m_polar_coordinates = other.m_polar_coordinates;
    m_cartesian_coordinates = other.m_cartesian_coordinates;
    
    m_restrictions = other.m_restrictions;
    m_estimated_data = other.m_estimated_data;
    m_overflown_data = other.m_overflown_data;
    m_holding = other.m_holding;
    return *this;
}

/////////////////////////////////////////////////////////////////////////////

void Waypoint::operator>>(QDataStream& out) const
{
    out << m_is_valid
        << m_id
        << m_name
        << m_parent
        << m_flag
        << m_polar_coordinates
        << m_cartesian_coordinates;

    m_restrictions >> out;
    m_estimated_data >> out;
    m_overflown_data >> out;
    m_holding >> out;
}

/////////////////////////////////////////////////////////////////////////////

void Waypoint::operator<<(QDataStream& in)
{
    in >> m_is_valid
       >> m_id
       >> m_name
       >> m_parent
       >> m_flag
       >> m_polar_coordinates
       >> m_cartesian_coordinates;

    m_restrictions << in;
    m_estimated_data << in;
    m_overflown_data << in;
    m_holding << in;
}

/////////////////////////////////////////////////////////////////////////////

bool Waypoint::isDependendWaypoint() const
{
    return 
        asWaypointHdgToAlt() != 0 ||
        asWaypointHdgToIntercept() != 0;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void WaypointPtrList::sortByDistance(const Waypoint& reference_wpt)
{
    for (int loop_index=0; loop_index < count() ; ++loop_index)
    {
        for (int compare_index=0; compare_index < count() - 1 - loop_index; ++compare_index)
        {
            if (Navcalc::getDistBetweenWaypoints(reference_wpt, *at(compare_index)) >
                Navcalc::getDistBetweenWaypoints(reference_wpt, *at(compare_index+1)))
            {
                Waypoint* wpt = takeAt(compare_index);
                insert(compare_index+1, wpt);
            }        
        }
    }
}

