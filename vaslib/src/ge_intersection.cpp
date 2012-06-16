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

/*! \file    ge_intersection.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "math.h"
#include "ge_intersection.h"

#define INT_TRIANGLE_SIZE 10
#define INT_TRIANGLE_CIRCLE_SIZE 1.5

/////////////////////////////////////////////////////////////////////////////

GEIntersection::GEIntersection(const ProjectionBase* projection, 
             const Intersection* intersection) : 
    GraphicElementProjectionBase(projection)
{
    MYASSERT(intersection != 0);
    m_intersection = static_cast<Intersection*>(intersection->copy());
    MYASSERT(m_intersection != 0);

    double triangle_height = INT_TRIANGLE_SIZE * sqrt(3.0) / 2.0;
    m_triangle_points[0] = QPointF(0, -triangle_height * 0.66);
    m_triangle_points[1] = QPointF(+INT_TRIANGLE_SIZE/2.0, triangle_height * 0.33);
    m_triangle_points[2] = QPointF(-INT_TRIANGLE_SIZE/2.0, triangle_height * 0.33);

    m_circle_rect = QRectF(-INT_TRIANGLE_CIRCLE_SIZE, -INT_TRIANGLE_CIRCLE_SIZE,
                           2*INT_TRIANGLE_CIRCLE_SIZE, 2*INT_TRIANGLE_CIRCLE_SIZE);

    m_label = new GELabel(m_projection, this, QPointF(4,6), m_intersection->getId(), false, false);
    MYASSERT(m_label != 0);
    m_label->setNormalColor(m_normal_color);
}

/////////////////////////////////////////////////////////////////////////////

GEIntersection::~GEIntersection()
{
    delete m_label;
    delete m_intersection;
}

/////////////////////////////////////////////////////////////////////////////

void GEIntersection::recalcBoundingAndSelectorElements()
{
    if (!m_projection_changed) return;

    m_projection_changed = false;
    QPointF center_xy;
    MYASSERT(m_projection->convertLatLonToXY(m_intersection->getLatLon(), center_xy));
    m_bounding_rect = QRect((int)(center_xy.x()-INT_TRIANGLE_SIZE/2),
                            (int)(center_xy.y()-INT_TRIANGLE_SIZE/2),
                            INT_TRIANGLE_SIZE, INT_TRIANGLE_SIZE);

    m_selector_path = QPainterPath();
    m_selector_path.addRect(m_bounding_rect);
}

/////////////////////////////////////////////////////////////////////////////

void GEIntersection::draw(QPainter& painter)
{
    painter.save();

    QPointF center_xy;
    MYASSERT(m_projection->convertLatLonToXY(m_intersection->getLatLon(), center_xy));
    painter.translate(center_xy.x(), center_xy.y());

    // triangle
    painter.setBrush(QBrush(m_active_color));
    painter.setPen(QPen(m_active_color));
    painter.drawPolygon(m_triangle_points, 3);

    // circle
    painter.setBrush(QBrush(Qt::white));
    painter.setPen(QPen(Qt::white));
    painter.drawPie(m_circle_rect, 0, FULL_ARC);

    painter.restore();
}

// End of file
