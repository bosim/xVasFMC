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

/*! \file    projection_mercator.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/
#include <math.h>
#include <QPointF>

#include "projection_mercator.h"
#include "navcalc.h"

/////////////////////////////////////////////////////////////////////////////

ProjectionMercator::ProjectionMercator() : m_xy_scale_factor(1.0) {};

/////////////////////////////////////////////////////////////////////////////

void ProjectionMercator::setScaleAndCenter(const Waypoint& center_wpt,
                                           unsigned int range_nm,
                                           unsigned int drawing_dist_at_max_range)
{
    // reset translation & scale
    m_center_xy = QPointF();
    m_xy_scale_factor = 1.0;

    // remember the values
    m_center_latlon.setPointLatLon(center_wpt.pointLatLon());
    m_range_nm = range_nm;
    m_drawing_dist_at_max_range = drawing_dist_at_max_range;
    
    // set new translation
    MYASSERT(convertLatLonToXY(center_wpt.pointLatLon(), m_center_xy));

    // test the new center
    QPointF center_xy_test_wpt;
    MYASSERT(convertLatLonToXY(center_wpt.pointLatLon(), center_xy_test_wpt));
    MYASSERT(center_xy_test_wpt.x() < 0.001);
    MYASSERT(center_xy_test_wpt.y() < 0.001);

    // set new scale
    Waypoint pbd_wpt = Navcalc::getPBDWaypoint(center_wpt, 0, range_nm, 0);
    QPointF max_range_wpt_xy;
    MYASSERT(convertLatLonToXY(pbd_wpt.pointLatLon(), max_range_wpt_xy));

    m_xy_scale_factor = (double)drawing_dist_at_max_range /
                        qMax(fabs(max_range_wpt_xy.x()), fabs(max_range_wpt_xy.y()));
    
    // test the new scale

    QPointF scale_xy_test_wpt;
    MYASSERT(convertLatLonToXY(pbd_wpt.pointLatLon(), scale_xy_test_wpt));
    MYASSERT(qMax(scale_xy_test_wpt.x(), scale_xy_test_wpt.y()) <= 
             drawing_dist_at_max_range + 0.001);

    emit signalChanged();
}


/////////////////////////////////////////////////////////////////////////////

bool ProjectionMercator::convertLatLonToXY(const QPointF& latlon_point, QPointF& xy_point) const
{
    MYASSERT(latlon_point.x() > -90.0 && latlon_point.x() < 90.0);
    MYASSERT(latlon_point.y() >= -180.0 && latlon_point.y() <= 180.0);

    // calc projection
    double sin_lat = sin(Navcalc::toRad(latlon_point.x()));
    double x = latlon_point.y();
    double y = Navcalc::toDeg(-0.5 * log((1.0 + sin_lat) / (1.0 - sin_lat)));

    // translate and scale
    xy_point.setX((x - m_center_xy.x()) * m_xy_scale_factor);
    xy_point.setY((y - m_center_xy.y()) * m_xy_scale_factor);

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool ProjectionMercator::convertXYToLatLon(const QPointF& xy_point, QPointF& latlon_point) const
{
    // translate and scale
    double x = (xy_point.x() / m_xy_scale_factor) + m_center_xy.x();
    double y = (xy_point.y() / m_xy_scale_factor) + m_center_xy.y();

    // calc projection
    latlon_point.setX(Navcalc::toDeg(asin(tanh(Navcalc::toRad(-y)))));
    latlon_point.setY(x);

    return true;
}

// End of file
