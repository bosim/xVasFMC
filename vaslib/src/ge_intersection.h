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

/*! \file    ge_intersection.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __GE_INTERSECTION_H__
#define __GE_INTERSECTION_H__

#include "intersection.h"
#include "ge_projection.h"
#include "ge_label.h"

/////////////////////////////////////////////////////////////////////////////

//! graphic INTERSECTION representation
class GEIntersection : public GraphicElementProjectionBase, public AnchorLatLon
{
    Q_OBJECT

public:
    //! Standard Constructor
    GEIntersection(const ProjectionBase* projection, const Intersection* intersection);

    //! Destructor
    virtual ~GEIntersection();

    void draw(QPainter& painter);

    const Intersection* getIntersection() const { return m_intersection; }

    GELabel* getLabel() const { return m_label; }

    QPointF getAnchor() const { return m_intersection->getLatLon(); }

protected:

    void recalcBoundingAndSelectorElements();
    
    Intersection* m_intersection;
    GELabel* m_label;

    QPointF m_triangle_points[3];
    QRectF m_circle_rect;

private:
    //! Hidden copy-constructor
    GEIntersection(const GEIntersection&);
    //! Hidden assignment operator
    const GEIntersection& operator = (const GEIntersection&);
};

#endif /* __GE_INTERSECTION_H__ */

// End of file

