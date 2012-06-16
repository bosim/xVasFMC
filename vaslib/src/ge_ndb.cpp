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

/*! \file    ge_ndb.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "ge_ndb.h"

#define NDB_RADIUS_1 8
#define NDB_RADIUS_2 6
#define NDB_RADIUS_3 4

/////////////////////////////////////////////////////////////////////////////

GENdb::GENdb(const ProjectionBase* projection, 
             const Ndb* ndb) : 
    GraphicElementProjectionBase(projection)
{
    MYASSERT(ndb != 0);
    m_ndb = static_cast<Ndb*>(ndb->copy());
    MYASSERT(m_ndb != 0);
    m_normal_color = m_active_color = Qt::darkGreen;

    QString text = QString("%1 %2").arg(m_ndb->getFreq()/1000.0, 0, 'f', 2).arg(m_ndb->getId());
    m_label = new GELabel(m_projection, this, QPointF(0,0), text);
    MYASSERT(m_label != 0);
    m_label->setNormalColor(m_normal_color);
}

/////////////////////////////////////////////////////////////////////////////

GENdb::~GENdb()
{
  delete m_label;
  delete m_ndb;
}

/////////////////////////////////////////////////////////////////////////////

void GENdb::recalcBoundingAndSelectorElements() 
{
    if (!m_projection_changed) return;

    m_projection_changed = false;
    QPointF center_xy;
    MYASSERT(m_projection->convertLatLonToXY(m_ndb->getLatLon(), center_xy));
    m_bounding_rect = QRect((int)(center_xy.x()-NDB_RADIUS_1),
                            (int)(center_xy.y()-NDB_RADIUS_1),
                            2*NDB_RADIUS_1, 2*NDB_RADIUS_1);

    // FIXXME add circle ??
    m_selector_path = QPainterPath();
    m_selector_path.addRect(m_bounding_rect);
}

/////////////////////////////////////////////////////////////////////////////

void GENdb::draw(QPainter& painter)
{
    painter.save();

    QPointF center_xy;
    MYASSERT(m_projection->convertLatLonToXY(m_ndb->getLatLon(), center_xy));
    painter.translate(center_xy.x(), center_xy.y());

    QPen pen(m_active_color);
    pen.setStyle(Qt::DotLine);
    painter.setPen(pen);
    
    // circle 1
    QRectF rect(-NDB_RADIUS_1, -NDB_RADIUS_1, 2*NDB_RADIUS_1, 2*NDB_RADIUS_1);
    painter.drawArc(rect, 0, FULL_ARC);

    // circle 2
    rect = QRectF(-NDB_RADIUS_2, -NDB_RADIUS_2, 2*NDB_RADIUS_2, 2*NDB_RADIUS_2);
    painter.drawArc(rect, 0, FULL_ARC);

    // circle 2
    rect = QRectF(-NDB_RADIUS_3, -NDB_RADIUS_3, 2*NDB_RADIUS_3, 2*NDB_RADIUS_3);
    painter.drawArc(rect, 0, FULL_ARC);

    painter.restore();
}

// End of file
