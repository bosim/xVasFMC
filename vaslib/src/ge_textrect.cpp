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

/*! \file    ge_textrect.cpp
  \author  Alexander Wemmer, alex@wemmer.at
*/

#include <QPainter>

#include "ge_textrect.h"

#define SPACING 2

/////////////////////////////////////////////////////////////////////////////

GETextRect::GETextRect(const ProjectionBase* projection,
                       const QString& text,
                       const QPointF& reference_lefttop_point,
                       bool reference_point_is_latlon,
                       AbsoluteRefCorner abs_ref_corner,
                       bool draw_border) : 
    GERect(projection, QRectF(0, 0, 100, 10), true), m_text(text), 
    m_reference_point_is_latlon(reference_point_is_latlon), 
    m_abs_ref_corner(abs_ref_corner), m_draw_border(draw_border)
{
    if (reference_point_is_latlon) 
        setLeftTopFromLatLon(reference_lefttop_point);
    else
        setLeftTopFromXY(reference_lefttop_point);
}

/////////////////////////////////////////////////////////////////////////////

GETextRect::~GETextRect()
{
}

/////////////////////////////////////////////////////////////////////////////

QPointF GETextRect::getLeftTopXY() const
{
    QPointF lefttop_xy = m_reference_lefttop_point;
 
    if (m_reference_point_is_latlon)
    {
        MYASSERT(m_projection->convertLatLonToXY(m_reference_lefttop_point, lefttop_xy));
    }

    return lefttop_xy;
}

/////////////////////////////////////////////////////////////////////////////

void GETextRect::setLeftTopFromXY(const QPointF& lefttop_xy)
{
    if (m_reference_point_is_latlon)
    {
        MYASSERT(m_projection->convertXYToLatLon(lefttop_xy, m_reference_lefttop_point));
    }
    else
    {
        m_reference_lefttop_point = lefttop_xy;
    }
}

/////////////////////////////////////////////////////////////////////////////

void GETextRect::setLeftTopFromLatLon(const QPointF& lefttop_latlon)
{
    if (m_reference_point_is_latlon)
    {
        m_reference_lefttop_point = lefttop_latlon;
    }
    else
    {
        MYASSERT(m_projection->convertLatLonToXY(lefttop_latlon, m_reference_lefttop_point));
    }   
}

/////////////////////////////////////////////////////////////////////////////

GraphicElementBase* GETextRect::mouseClick(QPoint point, bool& already_used)
{
    GraphicElementBase::mouseClick(point, already_used);
    
    if (m_is_selected)
    {
        QPointF lefttop_xy = getLeftTopXY();
        m_selection_mouse_pos_diff = lefttop_xy - point;
        return this;
    }
    
    return 0;
}

/////////////////////////////////////////////////////////////////////////////

bool GETextRect::mouseMove(QPoint point_xy, bool already_used, bool do_highlight)
{
    already_used |= GraphicElementBase::mouseMove(point_xy, already_used, do_highlight);

    if (m_is_moveable && m_is_selected)
    {
        QPointF lefttop_xy = getLeftTopXY();
        setLeftTopFromXY(point_xy + m_selection_mouse_pos_diff);
        already_used = true;
    }
    
    return already_used;
}

/////////////////////////////////////////////////////////////////////////////

void GETextRect::draw(QPainter& painter)
{
    painter.save();
    painter.setPen(QPen(m_active_color));

    QPointF lefttop_xy = getLeftTopXY();

    QRectF text_rect = QRectF(lefttop_xy.x(), lefttop_xy.y(), 100, 10);
    QString text_rect_contstraint = m_text;
    if (text_rect_contstraint.isEmpty()) text_rect_contstraint = "text";

    m_rect = painter.boundingRect(
        text_rect, Qt::AlignHCenter|Qt::AlignVCenter, text_rect_contstraint);
    m_rect = QRectF(m_rect.left()-SPACING, m_rect.top()-SPACING, 
                    m_rect.width()+2*SPACING, m_rect.height()+2*SPACING);
    
    if (m_draw_border) painter.drawRect(m_rect);
    painter.drawText(m_rect, Qt::AlignHCenter|Qt::AlignVCenter, m_text);
    painter.restore();
}

// End of file
