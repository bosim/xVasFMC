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

/*! \file    ge_label.h
  \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __GE_LABEL_H__
#define __GE_LABEL_H__

#include <QPointF>
#include <QString>

#include "ge_rect.h"
#include "anchor_latlon.h"

/////////////////////////////////////////////////////////////////////////////

//! graphic VOR representation
class GELabel : public GERect
{
public:

    GELabel(const ProjectionBase* projection,
            const AnchorLatLon* anchor, 
            const QPointF diff_xy, 
            const QString& text,
            bool draw_line_to_anchor = true,
            bool draw_border = true);

    virtual ~GELabel();
      
    void draw(QPainter& painter);

    // Returns the selected element
    GraphicElementBase* mouseClick(QPoint point, bool& already_used);
    bool mouseMove(QPoint point, bool already_used, bool do_highlight);

    void setDiffXY(const QPointF& diff_xy) { m_diff_xy = diff_xy; }
    const QPointF& getDiffXY() const { return m_diff_xy; }

protected:

    const AnchorLatLon* m_anchor;
    QPointF m_diff_xy;
    QString m_text;
    bool m_draw_line_to_anchor;
    bool m_draw_border;
    QPointF m_selection_mouse_pos_diff;

private:
    //! Hidden copy-constructor
    GELabel(const GELabel&);
    //! Hidden assignment operator
    const GELabel& operator = (const GELabel&);
};

#endif /* __GE_LABEL_H__ */

// End of file

