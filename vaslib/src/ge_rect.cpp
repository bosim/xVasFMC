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

/*! \file    ge_rect.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "ge_rect.h"

/////////////////////////////////////////////////////////////////////////////

GERect::GERect(const ProjectionBase* projection, const QRectF& rect, bool is_moveable) :
    GraphicElementProjectionBase(projection, is_moveable)
{
    m_rect = rect;
}

/////////////////////////////////////////////////////////////////////////////

GERect::~GERect()
{
}

/////////////////////////////////////////////////////////////////////////////

void GERect::recalcBoundingAndSelectorElements()
{
    m_bounding_rect = QRect((int)(m_rect.left()), (int)(m_rect.top()),
                            (int)(m_rect.width()), (int)(m_rect.height()));

    m_selector_path = QPainterPath();
    m_selector_path.addRect(m_bounding_rect);
}

/////////////////////////////////////////////////////////////////////////////

void GERect::draw(QPainter& painter)
{
    painter.save();
    painter.setPen(QPen(m_active_color));
    painter.drawRect(m_rect);
    painter.restore();
}

// End of file
