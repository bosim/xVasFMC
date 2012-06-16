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

/*! \file    ge_airport.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "ge_airport.h"

#define APT_CIRCLE_RADIUS 8
#define APT_CIRCLE_WIDTH 2

#define APT_CIRCLE_CROWN_HEIGHT 2
#define APT_CIRCLE_CROWN_WIDTH 2

/////////////////////////////////////////////////////////////////////////////

GEAirport::GEAirport(const ProjectionBase* projection, 
                     const Airport* airport) : 
    GraphicElementProjectionBase(projection)
{
    MYASSERT(airport != 0);
    m_airport = static_cast<Airport*>(airport->copy());
    MYASSERT(m_airport != 0);
    m_normal_color = m_active_color = Qt::blue;
    
    m_label = new GELabel(m_projection, this, QPointF(0,6), m_airport->getId(), false, false);
    MYASSERT(m_label != 0);
    m_label->setNormalColor(m_normal_color);
}

/////////////////////////////////////////////////////////////////////////////

GEAirport::~GEAirport()
{
    delete m_label;
    delete m_airport;
}

/////////////////////////////////////////////////////////////////////////////

void GEAirport::recalcBoundingAndSelectorElements()
{
    if (!m_projection_changed) return;

    m_projection_changed = false;
    QPointF center_xy;
    MYASSERT(m_projection->convertLatLonToXY(m_airport->getLatLon(), center_xy));
    m_bounding_rect = QRect((int)(center_xy.x()-APT_CIRCLE_RADIUS),
                            (int)(center_xy.y()-APT_CIRCLE_RADIUS),
                            2*APT_CIRCLE_RADIUS, 2*APT_CIRCLE_RADIUS);

    // FIXXME add circle ??
    m_selector_path = QPainterPath();
    m_selector_path.addRect(m_bounding_rect);
}

/////////////////////////////////////////////////////////////////////////////

void GEAirport::draw(QPainter& painter)
{
    painter.save();

    QPointF center_xy;
    MYASSERT(m_projection->convertLatLonToXY(m_airport->getLatLon(), center_xy));
    painter.translate(center_xy.x(), center_xy.y());

    QPen pen(m_active_color);
    pen.setWidthF(APT_CIRCLE_WIDTH);
    painter.setPen(pen);

    // circle
    QRectF rect(-APT_CIRCLE_RADIUS, -APT_CIRCLE_RADIUS, 2*APT_CIRCLE_RADIUS, 2*APT_CIRCLE_RADIUS);
    painter.drawArc(rect, 0, FULL_ARC);

    // crown

    pen.setWidthF(APT_CIRCLE_CROWN_WIDTH);
    painter.setPen(pen);
    
    QLineF line(0, APT_CIRCLE_RADIUS, 0, APT_CIRCLE_RADIUS + APT_CIRCLE_CROWN_HEIGHT);
    int max_rotation = 6;
    for(int rotation=1; rotation<=max_rotation; ++rotation)
    {
        painter.rotate(360/(double)max_rotation);
        painter.drawLine(line);
    }

    painter.restore();
}

// End of file
