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

#ifndef WAYPOINT_HDG_TO_INTERCEPT
#define WAYPOINT_HDG_TO_INTERCEPT

#include "logger.h"
#include "navcalc.h"

#include "waypoint.h"

/////////////////////////////////////////////////////////////////////////////

class WaypointHdgToIntercept : public Waypoint
{
public:
    
    WaypointHdgToIntercept(const QString& id, const double& lat, const double& lon, 
                           const Waypoint& fix_to_intercept, 
                           uint radial_to_intercept,
                           uint hdg_until_intercept,
                           Navcalc::TURN_DIRECTION turn_direction) :
        Waypoint(id, QString::null, lat, lon), 
        m_fix_to_intercept(fix_to_intercept),
        m_radial_to_intercept(radial_to_intercept),
        m_hdg_until_intercept(hdg_until_intercept),
        m_turn_direction(turn_direction)
    {
        m_type = TYPE_HDG_TO_INTERCEPT;
    }

    WaypointHdgToIntercept(const WaypointHdgToIntercept& other_wpt) : 
        Waypoint(other_wpt),
        m_fix_to_intercept(other_wpt.m_fix_to_intercept),
        m_radial_to_intercept(other_wpt.m_radial_to_intercept),
        m_hdg_until_intercept(other_wpt.m_hdg_until_intercept),
        m_turn_direction(other_wpt.m_turn_direction)
    {}

    virtual ~WaypointHdgToIntercept() {};

    virtual Waypoint* deepCopy() const { return new WaypointHdgToIntercept(*this); }

    inline virtual const WaypointHdgToIntercept* asWaypointHdgToIntercept() const { return this; }
    inline virtual WaypointHdgToIntercept* asWaypointHdgToIntercept() { return this; }

    QString toString() const
    {
        return QString("WPT_HDG2INTERCEPT (%1, %2, %3/%4, fix2icept=%5, radial2icept=%6, hdg2icept=%7)").
            arg(m_id).arg(m_name).arg(lat()).arg(lon()).
            arg(m_fix_to_intercept.toString()).arg(m_radial_to_intercept).arg(m_hdg_until_intercept);
    }

    void setFixToIntercept(const Waypoint& fix_to_intercept) { m_fix_to_intercept = fix_to_intercept; }
    const Waypoint& fixToIntercept() const { return m_fix_to_intercept; }

    void setRadialToIntercept(int radial_to_intercept) { m_radial_to_intercept = radial_to_intercept; }
    int radialToIntercept() const { return m_radial_to_intercept; }

    void setHdgUntilIntercept(int hdg_until_intercept) { m_hdg_until_intercept = hdg_until_intercept; }
    int hdgUntilIntercept() const { return m_hdg_until_intercept; }

    void setTurnDirection(Navcalc::TURN_DIRECTION turn_direction) { m_turn_direction = turn_direction; }
    Navcalc::TURN_DIRECTION turnDirection() const { return m_turn_direction; }

    virtual void operator<<(QDataStream& in)
    {
        qint8 turndir;
        m_fix_to_intercept << in;
        in >> m_radial_to_intercept >> m_hdg_until_intercept >> turndir;
        Waypoint::operator<<(in);

        m_turn_direction = (Navcalc::TURN_DIRECTION)turndir;
    }

    virtual void operator>>(QDataStream& out) const
    {
        m_fix_to_intercept >> out;
        out << m_radial_to_intercept << m_hdg_until_intercept << (qint8)m_turn_direction;
        Waypoint::operator>>(out);    
    }
    
protected:

    Waypoint m_fix_to_intercept;
    int m_radial_to_intercept;
    int m_hdg_until_intercept;
    Navcalc::TURN_DIRECTION m_turn_direction;
};

#endif
