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

#ifndef RUNWAY_H
#define RUNWAY_H

#include "waypoint.h"

/////////////////////////////////////////////////////////////////////////////

class Runway : public Waypoint
{
public:
    
    Runway();
    
    Runway(const QString& id, double lat, double lon,
           int hdg, int length_m, bool has_ils, int ils_freq, 
           int ils_hdg, int threshold_elevation_ft, 
           int gs_angle, int threshold_overflying_height_ft);
    
    virtual ~Runway() {};

    virtual Waypoint* deepCopy() const { return new Runway(*this); }

    inline virtual const Runway* asRunway() const { return this; }
    inline virtual Runway* asRunway() { return this; }

    virtual QString toString() const;

    inline int hdg() const { return m_hdg; }
    inline int lengthM() const { return m_length_m; }
    inline int hasILS() const { return m_has_ils; }
    inline int ILSFreq() const { return m_ils_freq; }
    inline int ILSHdg() const { return m_ils_hdg; }
    inline int GSAngle() const { return m_gs_angle; }
    inline int thresholdElevationFt() const { return m_threshold_elevation_ft; }
    inline int thresholdOverflyingHeightFt() const { return m_threshold_overflying_height_ft; }

    virtual void operator<<(QDataStream& in);
    virtual void operator>>(QDataStream& out) const;

protected:

    int m_hdg;
    int m_length_m;
    bool m_has_ils;
    int m_ils_freq; // ILS frequency integer frequency * 1,000
    int m_ils_hdg;
    int m_gs_angle; // Glideslope angle integer degrees * 100
    int m_threshold_elevation_ft;
    int m_threshold_overflying_height_ft;
};

/////////////////////////////////////////////////////////////////////////////

inline QDataStream &operator<<(QDataStream &stream, const Runway& rwy)
{
    rwy >> stream;
    return stream;
}

/////////////////////////////////////////////////////////////////////////////

inline QDataStream &operator>>(QDataStream &stream,  Runway& rwy)
{
    rwy << stream;
    return stream;
}

#endif
