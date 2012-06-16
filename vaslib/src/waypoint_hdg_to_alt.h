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

#ifndef WAYPOINT_HDG_TO_ALT
#define WAYPOINT_HDG_TO_ALT

#include "logger.h"

#include "waypoint.h"

/////////////////////////////////////////////////////////////////////////////

class WaypointHdgToAlt : public Waypoint
{
public:
    
    WaypointHdgToAlt(const QString& id, uint hdg_to_hold) :
        Waypoint(id, QString::null, 0.0, 0.0), m_hdg_to_hold(hdg_to_hold)
    {
        m_type = TYPE_HDG_TO_ALT;
    }

    WaypointHdgToAlt(const QString& id, const QString& name, int hdg_to_hold);

    virtual ~WaypointHdgToAlt() {};

    virtual Waypoint* deepCopy() const { return new WaypointHdgToAlt(*this); }

    inline virtual const WaypointHdgToAlt* asWaypointHdgToAlt() const { return this; }
    inline virtual WaypointHdgToAlt* asWaypointHdgToAlt() { return this; }

    QString toString() const
    {
        return QString("WPT_HDG2ALT (%1, %2, %3/%4, %5)").
            arg(m_id).arg(m_name).arg(lat()).arg(lon()).arg(m_hdg_to_hold);
    }

    void setHdgToHold(int hdg_to_hold) { m_hdg_to_hold = hdg_to_hold; }
    int hdgToHold() const { return m_hdg_to_hold; }

    virtual void operator<<(QDataStream& in)
    {
        in >> m_hdg_to_hold;
        Waypoint::operator<<(in);
    }

    virtual void operator>>(QDataStream& out) const
    {
        out << m_hdg_to_hold;
        Waypoint::operator>>(out);    
    }
    
protected:

    int m_hdg_to_hold;
};

#endif
