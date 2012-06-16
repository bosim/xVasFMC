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

/*! \file    ge_rect.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __GE_RECT_H__
#define __GE_RECT_H__

#include <QRectF>

#include "ge_projection.h"

/////////////////////////////////////////////////////////////////////////////

//! graphic VOR representation
class GERect : public GraphicElementProjectionBase
{
public:
    //! Standard Constructor
    GERect(const ProjectionBase* projection, const QRectF& rect, bool is_moveable = false);
    
    //! Destructor
    virtual ~GERect();
    
    void draw(QPainter& painter);

protected:

    void recalcBoundingAndSelectorElements();

    QRectF m_rect;

private:
    //! Hidden copy-constructor
    GERect(const GERect&);
    //! Hidden assignment operator
    const GERect& operator = (const GERect&);
};

#endif /* __GE_RECT_H__ */

// End of file

