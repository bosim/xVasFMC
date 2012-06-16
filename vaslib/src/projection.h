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

/*! \file    projection.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef PROJECTION_H
#define PROJECTION_H

#include <QObject>
#include <QPointF>

#include "waypoint.h"

/////////////////////////////////////////////////////////////////////////////

//! Base class for projections
class ProjectionBase : public QObject
{
    Q_OBJECT

signals:

    void signalChanged();

public:
    //! Standard Constructor
    ProjectionBase(unsigned int range_nm = 100) : m_range_nm(range_nm) {};

    //! Destructor
    virtual ~ProjectionBase() {};

    //-----

    //! Sets the center and the scale of the projection, where the center
    //! lat/lon will result in (0/0) x/y coordinates, and the lat/lon points
    //! at range (e.g.20nm) will have a (max.) distance of "drawing_dist_at_max_range"
    //! from the (0/0) x/y coordinate.
    virtual void setScaleAndCenter(const Waypoint& center_wpt,
                           unsigned int range_nm,
                           unsigned int drawing_dist_at_max_range) = 0;

    virtual void setCenter(const Waypoint& center_wpt)
    {
        setScaleAndCenter(center_wpt, m_range_nm, m_drawing_dist_at_max_range);
    };

    virtual void setScale(unsigned int range_nm,
                          unsigned int drawing_dist_at_max_range)
    {
        setScaleAndCenter(m_center_latlon, range_nm, drawing_dist_at_max_range);
    };
    
    //! returns the lat/lon center of the projection
    virtual const Waypoint& getCenter() const { return m_center_latlon; }

    //! returns the range of the projection
    virtual unsigned int getRangeNm() const { return m_range_nm; }

    //! returns the drawing dist at max range
    virtual unsigned int getDrawingDistAtMaxRange() const { return m_drawing_dist_at_max_range; }

    //! Converts the given point's lat/lon values to x/y.
    //! (QPointF: x = lat , y=lon)
    inline bool convertLatLonToXY(Waypoint& waypoint) const
    {
        QPointF wpt_xy;
        bool ret = convertLatLonToXY(waypoint.pointLatLon(), wpt_xy);
        waypoint.setPointXY(wpt_xy);
        return ret;
    }

    //! Converts the given point's lat/lon values to x/y.
    //! (QPointF: x = lat , y=lon)
    virtual bool convertLatLonToXY(const QPointF& latlon_point, QPointF& xy_point) const = 0;


	//! Converts the given x/y to lat/lon values.
    //! (QPointF: x = lat , y=lon)
	virtual bool convertXYToLatLon(const QPointF& xy_point, QPointF& latlon_point) const = 0;

protected:

    //! lat/lon center of the projection
    Waypoint m_center_latlon;

    //! the range of the projection
    unsigned int m_range_nm;
    unsigned int m_drawing_dist_at_max_range;

private:
    //! Hidden copy-constructor
    ProjectionBase(const ProjectionBase&);
    //! Hidden assignment operator
    const ProjectionBase& operator = (const ProjectionBase&);
};

#endif /* PROJECTION_H */

// End of file

