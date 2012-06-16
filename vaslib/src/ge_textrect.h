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

/*! \file    ge_textrect.h
  \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __GE_TEXTRECT_H__
#define __GE_TEXTRECT_H__

#include <QPointF>
#include <QString>

#include "ge_rect.h"

/////////////////////////////////////////////////////////////////////////////

//! graphic VOR representation
class GETextRect : public GERect
{
    Q_OBJECT

public:

    enum AbsoluteRefCorner { ABSREF_TOPLEFT = 0,
                             ABSREF_TOPRIGHT,
                             ABSREF_BOTTOMLEFT,
                             ABSREF_BOTTOMRIGHT
    };

    QString absRefCornerText() { return absRefCornerText(m_abs_ref_corner); }
    static QString absRefCornerText(AbsoluteRefCorner abs_ref_corner)
    {
        switch(abs_ref_corner)
        {
            case ABSREF_TOPLEFT: return "topleft";
            case ABSREF_TOPRIGHT: return "topright";
            case ABSREF_BOTTOMLEFT: return "bottomleft";
            case ABSREF_BOTTOMRIGHT: return "bottomright";
        }
        MYASSERT(0);
        return QString::null;
    }

    static AbsoluteRefCorner getAbsRefCornerFromText(const QString& text)
    {
        QString mytext = text.toLower().trimmed();
        if (mytext == "topleft") return ABSREF_TOPLEFT;
        if (mytext == "topright") return ABSREF_TOPRIGHT;
        if (mytext == "bottomleft") return ABSREF_BOTTOMLEFT;
        if (mytext == "bottomright") return ABSREF_BOTTOMRIGHT;
        return ABSREF_TOPLEFT;
    }

    //! When reference_point_is_latlon = true, the reference_lefttop_point
    //! must be a LOT/LON value. When false, the reference_lefttop_point must
    //! be a X/Y value in reference to abs_ref_corner.
    GETextRect(const ProjectionBase* projection,
               const QString& text,
               const QPointF& reference_lefttop_point,
               bool reference_point_is_latlon = true,
               AbsoluteRefCorner abs_ref_corner = ABSREF_TOPLEFT,
               bool draw_border = true);
    
    virtual ~GETextRect();
    
    void draw(QPainter& painter);
    
    // Returns the selected element
    GraphicElementBase* mouseClick(QPoint point, bool& already_used);
    bool mouseMove(QPoint point, bool already_used, bool do_highlight);

    void setText(const QString text) { m_text = text; }
    const QString getText() const { return m_text; }
    
    void setRefPoint(const QPointF reference_lefttop_point)
    { m_reference_lefttop_point = reference_lefttop_point;}
    const QPointF getRefPoint() const { return m_reference_lefttop_point; }

    void setRefPointLatLon(const bool reference_point_is_latlon)
    { m_reference_point_is_latlon = reference_point_is_latlon; }
    const bool isRefPointLatLon() const { return m_reference_point_is_latlon; }

    void setDrawBorder(const bool draw_border) { m_draw_border = draw_border; }
    const bool doDrawBorder() const { return m_draw_border; }
    
    void setAbsRefCorner(const AbsoluteRefCorner abs_ref_corner)
    { m_abs_ref_corner = abs_ref_corner; }
    const AbsoluteRefCorner getAbsRefCorner() const { return m_abs_ref_corner; }
    
protected:
    
    QPointF getLeftTopXY() const;
    void setLeftTopFromXY(const QPointF& lefttop_xy);
    void setLeftTopFromLatLon(const QPointF& lefttop_latlon);

protected:

    QString m_text;
    QPointF m_selection_mouse_pos_diff;
    QPointF m_reference_lefttop_point;
    bool m_reference_point_is_latlon;
    AbsoluteRefCorner m_abs_ref_corner;
    bool m_draw_border;
private:
    //! Hidden copy-constructor
    GETextRect(const GETextRect&);
    //! Hidden assignment operator
    const GETextRect& operator = (const GETextRect&);
};

#endif /* __GE_TEXTRECT_H__ */

// End of file

