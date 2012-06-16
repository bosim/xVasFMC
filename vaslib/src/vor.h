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

#ifndef VOR_H
#define VOR_H

#include "ndb.h"

/////////////////////////////////////////////////////////////////////////////

class Vor : public Ndb
{
public:
    
    Vor();

    Vor(const QString& id, const QString& name,
        const double& lat, const double& lon,
        int freq, bool has_dme, int range_nm, int elevation_ft,
        const QString& country_code);

    virtual ~Vor() {};

    virtual Waypoint* deepCopy() const { return new Vor(*this); }

    inline virtual const Vor* asVor() const { return this; }
    inline virtual Vor* asVor() { return this; }

    QString toString() const;

    const bool& hasDME() const { return m_has_dme; }

    virtual void operator<<(QDataStream& in);
    virtual void operator>>(QDataStream& out) const;

protected:

    bool m_has_dme;
};

#endif
