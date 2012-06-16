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

#ifndef NDB_H
#define NDB_H

#include "intersection.h"

/////////////////////////////////////////////////////////////////////////////

class Ndb : public Intersection
{
public:
    
    Ndb();

    Ndb(const QString& id, const QString& name,
        const double& lat, const double& lon,
        int freq, int range_nm, int elevation_ft,
        const QString& country_code);

    virtual ~Ndb() {};

    virtual Waypoint* deepCopy() const { return new Ndb(*this); }

    inline virtual const Ndb* asNdb() const { return this; }
    inline virtual Ndb* asNdb() { return this; }

    QString toString() const;

#ifdef HAVE_PLIB    
    inline void setFreq(const int& freq) { m_freq = freq; }
#endif
    const int& freq() const { return m_freq; }
    QString freqString() const { return QString("%1").arg(m_freq/1000.0, 3, 'f', 3); }
    const int& rangeNm() const { return m_range_nm; }
    const int& elevationFt() const { return m_elevation_ft; }
    
    virtual void operator<<(QDataStream& in);
    virtual void operator>>(QDataStream& out) const;

protected:

    int m_freq; // frequency * 1,000
    int m_range_nm;
    int m_elevation_ft;
};

#endif
