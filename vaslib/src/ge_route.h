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

/*! \file    ge_route.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __GE_ROUTE_H__
#define __GE_ROUTE_H__

#include "ge_projection.h"
#include "waypoint.h"
#include "route.h"

/////////////////////////////////////////////////////////////////////////////

//! graphic ROUTE representation
class GERoute : public GraphicElementProjectionBase
{
    Q_OBJECT

public:
    //! Standard Constructor
    GERoute(const ProjectionBase* projection, const QString& name, 
            const QString& type, double mag_variation);

    //! Destructor
    virtual ~GERoute();

    void draw(QPainter& painter);

    const QString& getName() const { return m_route.getName(); }
    void setName(const QString& name) { m_route.setName(name); }

    const QString& getType() const { return m_route.getType(); }
    Route& getRoute() { return m_route; }

public slots:

    void slotNewMagneticVariation(const double& variation) { m_mag_variation = variation; }

protected slots:

    void slotRouteChanged() { m_route_changed = true; }

protected:

    void recalcBoundingAndSelectorElements();

    unsigned int getSpacing(const Waypoint* wpt);

protected:
    
    Route m_route;
    double m_mag_variation;
    bool m_route_changed;

private:
    //! Hidden copy-constructor
    GERoute(const GERoute&);
    //! Hidden assignment operator
    const GERoute& operator = (const GERoute&);
};

#endif /* __GE_ROUTE_H__ */

// End of file

