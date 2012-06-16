///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2007 Alexander Wemmer 
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

/*! \file    geodata.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef GEODATA_H
#define GEODATA_H

#include <QObject>
#include <QStringList>

#include "route.h"

class ProjectionBase;

/////////////////////////////////////////////////////////////////////////////

//! geo data provider
class GeoData : public QObject
{
    Q_OBJECT

public:
    //! Standard Constructor
    GeoData();

    //! Destructor
    virtual ~GeoData();

    //! Filenames shall be specified as relative paths (relative to vasFMC directory)
    void setFilenames(const QStringList& filename_list) { m_filename_list = filename_list; }

    //! filter_level = 1 land, 2 lake, 3 island_in_lake, 4 pond_in_island_in_lake
    bool readData(uint filter_level);

    inline const RoutePtrList& routeList() const { return m_route_list; }
    void calcProjection(const ProjectionBase& projection);

    void updateActiveRouteList(const Waypoint& center, int max_dist_nm);
    inline const RoutePtrList& activeRouteList() const { return m_active_route_list; }
    void calcProjectionActiveRoute(const ProjectionBase& projection);

signals:

    void signalActiveRouteChanged();

protected:

    QStringList m_filename_list;
    RoutePtrList m_route_list;
    RoutePtrList m_active_route_list;
    
private:
    //! Hidden copy-constructor
    GeoData(const GeoData&);
    //! Hidden assignment operator
    const GeoData& operator = (const GeoData&);
};

#endif /* GEODATA_H */

// End of file

