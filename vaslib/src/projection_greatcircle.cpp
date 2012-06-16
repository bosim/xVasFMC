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

/*! \file    projection_greatcircle.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/
#include <math.h>
#include <QPointF>

#include "logger.h"
#include "navcalc.h"

#include "projection_greatcircle.h"

#ifndef isnan
#define isnan _isnan
#endif

/////////////////////////////////////////////////////////////////////////////

ProjectionGreatCircle::ProjectionGreatCircle() : m_xy_scale_factor(1.0) {};

/////////////////////////////////////////////////////////////////////////////

void ProjectionGreatCircle::setScaleAndCenter(const Waypoint& center_wpt,
                                              unsigned int,
                                              unsigned int)
{
    // reset translation & scale
    m_center_latlon = center_wpt;
    m_xy_scale_factor = 1.0;

    emit signalChanged();
}


/////////////////////////////////////////////////////////////////////////////

bool ProjectionGreatCircle::convertLatLonToXY(const QPointF& latlon_point, QPointF& xy_point) const
{
    QPointF mypoint = latlon_point;
    while (mypoint.x() > 180) mypoint.setX(mypoint.x() - 180);
    while (mypoint.x() < -180) mypoint.setX(mypoint.x() + 180);
    while (mypoint.x() > 90) mypoint.setX(90 - (mypoint.x() - 90));
    while (mypoint.x() < -90) mypoint.setX(180 + mypoint.x());
    while (mypoint.y() > 180) mypoint.setY(mypoint.y() - 360);
    while (mypoint.y() < -180) mypoint.setY(mypoint.y() + 360);

    if (mypoint.x() < -90 || mypoint.x() > 90 || isnan(mypoint.x()) ||
        mypoint.y() < -180.0 || mypoint.y() > 180.0 || isnan(mypoint.y()))
    {
        Logger::log(QString("invalid LAT/LON: %1/%2").arg(mypoint.x()).arg(mypoint.y()));
        return false;
    }
    
    MYASSERT(mypoint.x() >= -90.0);
    MYASSERT(mypoint.x() <= 90.0);
    MYASSERT(mypoint.y() >= -180.0);
    MYASSERT(mypoint.y() <= 180.0);

    // calc projection

    double distance = 0.0;
    double track = 0.0;
    
    Waypoint wpt;
    wpt.setPointLatLon(mypoint);

    MYASSERT(Navcalc::getDistAndTrackBetweenWaypoints(m_center_latlon, wpt, distance, track));

    double track_rad = Navcalc::toRad(track);
    xy_point.setX(distance * sin(track_rad) * m_xy_scale_factor);
    xy_point.setY(-distance * cos(track_rad) * m_xy_scale_factor);
    
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool ProjectionGreatCircle::convertXYToLatLon(const QPointF&, QPointF&) const
{
    MYASSERT(0);
    return false;
}

// End of file
