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

/*! \file    ge_label.cpp
  \author  Alexander Wemmer, alex@wemmer.at
*/

#include <QPainter>

#include "ge_label.h"

/////////////////////////////////////////////////////////////////////////////

GELabel::GELabel(const ProjectionBase* projection,
                 const AnchorLatLon* anchor,
                 const QPointF diff_xy,
                 const QString& text,
                 bool draw_line_to_anchor,
                 bool draw_border) : 
    GERect(projection, QRectF(0, 0, 100, 100), true), 
    m_anchor(anchor), m_diff_xy(diff_xy), 
    m_text(text), m_draw_line_to_anchor(draw_line_to_anchor), m_draw_border(draw_border)
{
    MYASSERT(m_anchor != 0);
}

/////////////////////////////////////////////////////////////////////////////

GELabel::~GELabel()
{
}

/////////////////////////////////////////////////////////////////////////////

GraphicElementBase* GELabel::mouseClick(QPoint point, bool& already_used)
{
    GraphicElementBase::mouseClick(point, already_used);
    
    if (m_is_selected)
    {
        QPointF anchor_latlon = m_anchor->getAnchor();
        QPointF anchor_xy;
        MYASSERT(m_projection->convertLatLonToXY(anchor_latlon, anchor_xy));
        m_selection_mouse_pos_diff = m_diff_xy + anchor_xy - point;
        return this;
    }
    
    return 0;
}

/////////////////////////////////////////////////////////////////////////////

bool GELabel::mouseMove(QPoint point_xy, bool already_used, bool do_highlight)
{
    already_used |= GraphicElementBase::mouseMove(point_xy, already_used, do_highlight);

    if (m_is_moveable && m_is_selected)
    {
        //FIXXME mark model dirty!        
        QPointF anchor_latlon = m_anchor->getAnchor();
        QPointF anchor_xy;
        MYASSERT(m_projection->convertLatLonToXY(anchor_latlon, anchor_xy));
        m_diff_xy = point_xy - anchor_xy + m_selection_mouse_pos_diff;
        already_used = true;
    }
    
    return already_used;
}

/////////////////////////////////////////////////////////////////////////////

void GELabel::draw(QPainter& painter)
{
    painter.save();
    painter.setPen(QPen(m_active_color));

    QPointF anchor_latlon = m_anchor->getAnchor();
    QPointF anchor_xy;
    MYASSERT(m_projection->convertLatLonToXY(anchor_latlon, anchor_xy));
    QPointF lefttop_xy = anchor_xy + m_diff_xy;
    
    QRectF text_rect = QRectF(lefttop_xy.x(), lefttop_xy.y(), 500, 100);
    m_rect = painter.boundingRect(text_rect, Qt::AlignLeft|Qt::AlignTop, m_text);

    if (m_draw_line_to_anchor)
    {
        painter.drawLine(anchor_xy, m_rect.center());
        painter.fillRect(m_rect, QBrush(Qt::white));
    }

    if (m_draw_border) painter.drawRect(m_rect);
    painter.drawText(m_rect, m_text);
    painter.restore();
}

// End of file
