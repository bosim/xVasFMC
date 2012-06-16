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

/*! \file    ge_vor.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "ge_vor.h"

#define VOR_OUTER_RADIUS 16
#define VOR_MIDDLE_RADIUS 8
#define VOR_INNER_RADIUS 2

/////////////////////////////////////////////////////////////////////////////

GEVor::GEVor(const ProjectionBase* projection, const Vor* vor) : 
    GraphicElementProjectionBase(projection)
{
    MYASSERT(vor != 0);
    m_vor = static_cast<Vor*>(vor->copy());
    MYASSERT(m_vor != 0);
    
    QString text = 
        QString("%1 %2").arg(m_vor->getFreq()/1000.0, 0, 'f', 2).arg(m_vor->getId());
    m_label = new GELabel(m_projection, this, QPointF(0,0), text);
    MYASSERT(m_label != 0);
    m_label->setNormalColor(m_normal_color);
}

/////////////////////////////////////////////////////////////////////////////

GEVor::~GEVor()
{
    delete m_label;
    delete m_vor;
}

/////////////////////////////////////////////////////////////////////////////

void GEVor::recalcBoundingAndSelectorElements() 
{
    if (!m_projection_changed) return;

    m_projection_changed = false;
    QPointF center_xy;
    MYASSERT(m_projection->convertLatLonToXY(m_vor->getLatLon(), center_xy));
    m_bounding_rect = QRect((int)(center_xy.x()-VOR_OUTER_RADIUS),
                            (int)(center_xy.y()-VOR_OUTER_RADIUS),
                            2*VOR_OUTER_RADIUS, 2*VOR_OUTER_RADIUS);

    // FIXXME add circle ??
    m_selector_path = QPainterPath();
    m_selector_path.addRect(m_bounding_rect);
}

/////////////////////////////////////////////////////////////////////////////

void GEVor::draw(QPainter& painter)
{
    painter.save();

    painter.setPen(QPen(m_active_color));
    
    QPointF center_xy;
    MYASSERT(m_projection->convertLatLonToXY(m_vor->getLatLon(), center_xy));
    painter.translate(center_xy.x(), center_xy.y());

    // outer circle
    QRectF rect(-VOR_OUTER_RADIUS, -VOR_OUTER_RADIUS, 2*VOR_OUTER_RADIUS, 2*VOR_OUTER_RADIUS);
    painter.drawArc(rect, 0, FULL_ARC);

    // middle circle
    rect = QRectF(-VOR_MIDDLE_RADIUS, -VOR_MIDDLE_RADIUS, 2*VOR_MIDDLE_RADIUS, 2*VOR_MIDDLE_RADIUS);
    painter.drawArc(rect, 0, FULL_ARC);

    // inner circle
    rect = QRectF(-VOR_INNER_RADIUS, -VOR_INNER_RADIUS, 2*VOR_INNER_RADIUS, 2*VOR_INNER_RADIUS);
    painter.drawArc(rect, 0, FULL_ARC);

    // crown lines
    painter.save();
    QLineF line(0, VOR_MIDDLE_RADIUS, 0, VOR_OUTER_RADIUS);
    int max_rotation = 12;
    for(int rotation = 0; rotation<max_rotation; ++rotation)
    {
        painter.rotate(360/(double)max_rotation);
        painter.drawLine(line);
    }
    painter.restore();

    // text


//     double text_dist = 30;
//     double text_angle = -135.0;

//     painter.rotate(text_angle);
//     painter.translate(0, -text_dist);
//     line = QLineF(0,0, 0, text_dist-VOR_OUTER_RADIUS);
//     painter.drawLine(line);
//     painter.rotate(-text_angle);

//     QRect text_bounding_rect = 
//         painter.boundingRect(QRect(0, 0, 500, 500), Qt::AlignLeft | Qt::AlignTop, text);

//     QRectF text_rect = QRectF(-text_bounding_rect.width(), 0,
//                               text_bounding_rect.width(), text_bounding_rect.height());

//     painter.drawRect(text_rect);
//     painter.drawText(text_rect, text);

    painter.restore();
}

// End of file
