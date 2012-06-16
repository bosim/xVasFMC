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

#ifndef AIRPORT_H
#define AIRPORT_H

#include <QMap>

#include "waypoint.h"
#include "runway.h"

// dict of runway, indexed by the runway id
typedef QMap<QString, Runway> RunwayMap;
typedef QMapIterator<QString, Runway> RunwayMapIterator;

/////////////////////////////////////////////////////////////////////////////

class Airport : public Waypoint
{
public:
    
    Airport();
    
    Airport(const QString& id, const QString& name, 
            const double& lat, const double& lon, int elevation_ft);
    
    virtual ~Airport() {};

    virtual Waypoint* deepCopy() const { return new Airport(*this); }

    inline virtual const Airport* asAirport() const { return this; }
    inline virtual Airport* asAirport() { return this; }
    
    virtual QString toString() const;
    
    inline const int& elevationFt() const { return m_elevation_ft; }

    inline void addRunway(const Runway& runway_to_add)
    { m_runway_map.insert(runway_to_add.id(), runway_to_add); };
    
    inline Runway runway(const QString& id) const
    { 
		if (!m_runway_map.contains(id)) return Runway();
		return m_runway_map[id]; 
    }
    
    inline const RunwayMap& runwayMap() const { return m_runway_map; }
    inline const RunwayMapIterator runwayMapIterator() const { return RunwayMapIterator(m_runway_map); }
    inline RunwayMap& runwayMap() { return m_runway_map; }
    inline uint runwayCount() const { return m_runway_map.count(); }
    inline void clearRunways() { m_runway_map.clear(); }

    inline bool hasActiveRunway() const { return !m_active_runway_id.isEmpty(); }
    inline const QString& activeRunwayId() const { return m_active_runway_id; }
    inline Runway activeRunway() const { return runway(m_active_runway_id); }
    void setActiveRunwayId(const QString& id);

    virtual void operator<<(QDataStream& in);
    virtual void operator>>(QDataStream& out) const;

protected:

    int m_elevation_ft;
    RunwayMap m_runway_map;
    QString m_active_runway_id;
    Waypoint m_default_airport_location;
};

#endif
