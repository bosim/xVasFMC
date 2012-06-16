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

/*! \file    ge_route.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include <QFont>
#include <QFontMetrics>

#include "navcalc.h"
#include "chart.xml.defines.h"

#include "ge_route.h"

#define ROUTE_WIDTH 2
#define ROUTE_HIGHLIGHT_AREA 10

/////////////////////////////////////////////////////////////////////////////

GERoute::GERoute(const ProjectionBase* projection, const QString& name, 
                 const QString& type, double mag_variation) :
    GraphicElementProjectionBase(projection), m_route(name, type), 
    m_mag_variation(mag_variation), m_route_changed(true)
{
    MYASSERT(connect(&m_route, SIGNAL(signalChanged()), this, SLOT(slotRouteChanged())));
}

/////////////////////////////////////////////////////////////////////////////

GERoute::~GERoute()
{
}

/////////////////////////////////////////////////////////////////////////////

unsigned int GERoute::getSpacing(const Waypoint* wpt)
{
    MYASSERT(wpt != 0);
    switch(wpt->getType())
    {
        case(Waypoint::VOR): return 16;
        case(Waypoint::NDB): return 8;
        case(Waypoint::AIRPORT): return 8;
        case(Waypoint::INTERSECTION): return 2;
        default: return 2;
    }
    return 2;
}

/////////////////////////////////////////////////////////////////////////////

void GERoute::recalcBoundingAndSelectorElements() 
{
    if (!m_projection_changed && !m_route_changed) return;

    m_projection_changed = false;
    m_route_changed = false;

    m_bounding_rect = QRect();
    m_selector_path = QPainterPath();

    QPointF prev_point_xy;

    int max_count = m_route.count();
    for(int index=0; index<max_count; ++index)
    {
        const Waypoint* wpt = m_route.getFix(index);
        MYASSERT(wpt != 0);
        const RouteFixData& fix_data = m_route.getFixData(index);
        QPointF point_xy;
        MYASSERT(m_projection->convertLatLonToXY(wpt->getLatLon(), point_xy));

        if (index > 0)
        {
            QRect leg_bounding_rect = 
                QRect(QPoint((int)prev_point_xy.x(), (int)prev_point_xy.y()),
                      QPoint((int)point_xy.x(), (int)point_xy.y())).normalized();

            // we add selector rect for all legs with a visible route name
            if (fix_data.m_label_list.contains(CHART_ATTR_LABEL_PARAM_ROUTE))
            {
                m_selector_path.addRect(
                    QRect(leg_bounding_rect.center().x()-ROUTE_HIGHLIGHT_AREA,
                          leg_bounding_rect.center().y()-ROUTE_HIGHLIGHT_AREA,
                          2*ROUTE_HIGHLIGHT_AREA, 2*ROUTE_HIGHLIGHT_AREA));
            }

            m_bounding_rect |= leg_bounding_rect;
        }

        prev_point_xy = point_xy;
    }
}

/////////////////////////////////////////////////////////////////////////////

void GERoute::draw(QPainter& painter)
{
    painter.save();
    painter.setPen(QPen(m_active_color));

    QPen pen = painter.pen();
    pen.setWidthF(ROUTE_WIDTH);
    painter.setPen(pen);

    // loop through waypoints

    const Waypoint* last_wpt = 0;
    QPointF last_wpt_xy;
    RouteFixData last_fix_data;
    int last_leg_mag_course = -1;

    const QFontMetrics fontmetrics = painter.fontMetrics();
    int font_height = fontmetrics.boundingRect("X").height();

    unsigned int max_count = m_route.count();
    for(unsigned int index=0; index<max_count; ++index)
    {
        QRectF max_needed_rect;

        // get the waypoint and calc projection
        
        const Waypoint* wpt = m_route.getFix(index);
        MYASSERT(wpt != 0);
        const RouteFixData& fix_data = m_route.getFixData(index);
        int leg_mag_course = -1;

        QPointF wpt_xy;
        MYASSERT(m_projection->convertLatLonToXY(wpt->getLatLon(), wpt_xy));

        // draw the leg
        
        if (!last_wpt_xy.isNull())
        {
            painter.drawLine(last_wpt_xy, wpt_xy);

            QPointF xy_diff = wpt_xy - last_wpt_xy;
            double leg_angle= 90.0 - Navcalc::toDeg(atan2(xy_diff.x(), xy_diff.y()));
            double leg_dist_xy = sqrt((xy_diff.x()*xy_diff.x()) + (xy_diff.y()*xy_diff.y()));
            unsigned int leg_dist_nm = (int)(Navcalc::getDistBetweenWaypoints(*last_wpt, *wpt)+0.5);
            leg_mag_course = 
                (int)(Navcalc::trimHeading(
                          Navcalc::getTrackBetweenWaypoints(*last_wpt, *wpt) + 
                          m_mag_variation) + 0.5);

            // set leg direction dependend values
            unsigned int left_spacing = getSpacing(last_wpt);
            unsigned int right_spacing = getSpacing(wpt);
            int course_text_align = Qt::AlignLeft|Qt::AlignTop;
            bool direction_right = true;

            painter.save();
            painter.translate(last_wpt_xy);

            if (leg_angle < -90.0 || leg_angle > 90.0)
            {
                if (leg_angle < 0) leg_angle += 180;
                else leg_angle -= 180;

                painter.rotate(leg_angle);
                painter.translate(-leg_dist_xy, 0);

                left_spacing = getSpacing(wpt);
                right_spacing = getSpacing(last_wpt);
                course_text_align = Qt::AlignRight|Qt::AlignTop;
                direction_right = false;
            }
            else
            {
                painter.rotate(leg_angle);
            }

            QRectF above_text_rect = QRectF(
                left_spacing, -3-(font_height*2),
                leg_dist_xy-left_spacing-right_spacing, 2*font_height);

            QRectF below_text_rect = QRectF(
                left_spacing, 3,
                leg_dist_xy-left_spacing-right_spacing, 2*font_height);

            if (leg_dist_xy > left_spacing+right_spacing)
            {
                // draw leg crs and distance

                QString dist_text = QString("%1").arg(leg_dist_nm);
                QString text = dist_text;

                QString crs_text;
                if (qAbs(leg_mag_course -last_leg_mag_course) > 1)
                {
                    crs_text = QString("%1°").arg(leg_mag_course, 3, 10, QChar('0'));
                    text = crs_text+"/"+dist_text;
                }

                QRectF needed_rect = painter.boundingRect(
                    above_text_rect, Qt::AlignHCenter|Qt::AlignBottom, text);
                bool dist_only = false;

                // if crs + dist do not fit, reduce to dist only
                if (!above_text_rect.contains(needed_rect)) 
                {
                    text = dist_text;
                    dist_only = true;
                }

                needed_rect = painter.boundingRect(
                    above_text_rect, Qt::AlignHCenter|Qt::AlignBottom, text);

                if (above_text_rect.contains(needed_rect))
                {
                    painter.drawText(above_text_rect, Qt::AlignHCenter|Qt::AlignBottom, text);
                    max_needed_rect |= needed_rect;
                }

                // draw alt restriction

                QString alt_text;
                int align = Qt::AlignHCenter|Qt::AlignTop;
                needed_rect = QRectF();
                bool underline = false;
                bool overline = false;

                painter.save();
                
                if (fix_data.m_alt_restriction.startsWith(">"))
                {
                    alt_text = fix_data.m_alt_restriction.mid(1);
                    needed_rect = painter.boundingRect(above_text_rect, align, alt_text);
                    underline = true;
                }
                else if (fix_data.m_alt_restriction.startsWith("<"))
                {
                    alt_text = fix_data.m_alt_restriction.mid(1);
                    needed_rect = painter.boundingRect(above_text_rect, align, alt_text);
                    overline = true;
                }
                else if (!fix_data.m_alt_restriction.isEmpty())
                {
                    alt_text = fix_data.m_alt_restriction;
                    needed_rect = painter.boundingRect(above_text_rect, align, alt_text);
                }

                if (!alt_text.isEmpty() && above_text_rect.contains(needed_rect))
                {
                    QFont font = painter.font();
                    font.setUnderline(underline);
                    font.setOverline(overline);
                    painter.setFont(font);
                    painter.drawText(above_text_rect, align, alt_text);
                    max_needed_rect |= needed_rect;
                }
                else
                {
                    alt_text = QString::null;
                }
                
                painter.restore();

                // draw route label

                QString label_text;

                QStringListIterator iter(fix_data.m_label_list);
                while (iter.hasNext())
                {
                    QString entry = iter.next();

                    if (entry == CHART_ATTR_LABEL_PARAM_ROUTE)
                        label_text += getName();
                    else
                        label_text += entry;

                    int valign = Qt::AlignTop;
                    
                    needed_rect = 
                        painter.boundingRect(below_text_rect, Qt::AlignHCenter|valign, label_text);
                    if (below_text_rect.contains(needed_rect))
                    {
                        painter.drawText(below_text_rect, Qt::AlignHCenter|valign, label_text);
                        if (valign == Qt::AlignVCenter) 
                            max_needed_rect |= needed_rect;
                    }
                    else
                    {
                        label_text = QString::null;
                    }
                }
                    
                // try to fit the course somewhere else if the alt or label is missing
                if (dist_only)
                {
                    text = crs_text;
                    if (label_text.isEmpty())
                    {
                        needed_rect = painter.boundingRect(
                            below_text_rect, Qt::AlignHCenter|Qt::AlignTop, text);
                        
                        if (below_text_rect.contains(needed_rect))
                        {
                            painter.drawText(below_text_rect, 
                                             Qt::AlignHCenter|Qt::AlignTop, text);
                            max_needed_rect |= needed_rect;
                        }
                    }
                    else if (alt_text.isEmpty())
                    {
                        needed_rect = painter.boundingRect(
                            above_text_rect, Qt::AlignHCenter|Qt::AlignTop, text);
                        
                        if (above_text_rect.contains(needed_rect))
                        {
                            painter.drawText(above_text_rect, 
                                             Qt::AlignHCenter|Qt::AlignTop, text);
                        }
                    }
                }
            }

            // draw direction arrow

            double arrow_len = 6;
            double arrow_height = 2.5;
            double arrow_spacing = 2;

            if (max_needed_rect.width() + arrow_len + 2*arrow_spacing < 
                leg_dist_xy-left_spacing-right_spacing)
            {
                QPointF arrow_points[3];

                if (direction_right)
                {
                    arrow_points[0] = QPointF(max_needed_rect.right()+arrow_spacing, arrow_height);
                    arrow_points[1] = QPointF(max_needed_rect.right()+arrow_spacing+arrow_len, 0);
                    arrow_points[2] = QPointF(max_needed_rect.right()+arrow_spacing, -arrow_height);
                }
                else
                {
                    arrow_points[0] = QPointF(max_needed_rect.left()-arrow_spacing, arrow_height);
                    arrow_points[1] = QPointF(max_needed_rect.left()-arrow_spacing-arrow_len, 0);
                    arrow_points[2] = QPointF(max_needed_rect.left()-arrow_spacing, -arrow_height);
                }

                painter.drawPolygon(arrow_points, 3);
            }
            
            painter.restore();
        }

        // advance to next fix

        last_wpt = wpt;
        last_wpt_xy = wpt_xy;
        last_fix_data = fix_data;
        last_leg_mag_course = leg_mag_course;
    }

    painter.restore();
}

// End of file
