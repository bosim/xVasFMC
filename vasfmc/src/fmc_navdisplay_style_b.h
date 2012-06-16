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

/*! \file    fmc_navdisplay_style.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __FMC_NAVDISPLAY_STYLE_B_H__
#define __FMC_NAVDISPLAY_STYLE_B_H__

#include <QColor>

#include "fmc_navdisplay_style.h"

class Waypoint;

/////////////////////////////////////////////////////////////////////////////

class FMCNavdisplayStyleB : public FMCNavdisplayStyle
{
public:

    FMCNavdisplayStyleB(ConfigWidgetProvider* config_widget_provider,
                        VasGLWidget* parent,
                        Config* main_config,
                        const QString& cfg_navdisplay_style_filename,
                        Config* navdisplay_config,
                        Config* tcas_config,
                        FMCControl* fmc_control,
                        const ProjectionBase* projection,
                        const QSize& size,
                        const double& display_top_offset,
                        const int& max_drawable_y,
                        const QPointF& position_center,
                        const double& dist_scale_factor,
                        bool left_side);

    virtual ~FMCNavdisplayStyleB();

    void reset(const QSize& size,
               const double& display_top_offset,
               const int& max_drawable_y,
               const QPointF& position_center,
               const double& dist_scale_factor);

    void drawAvionicsOff();
    void drawRanges();
	void drawPlanePoly();
    void drawCompass(double north_track_rotation);
    void drawWindArrow();
    void drawAltitudeReachDisplay(const double& dist_scale_factor);
    void drawInfoTexts(const FlightRoute& route);
    void drawAdfPointer(uint adf_index);
    void drawVorPointer(uint vor_index);

    virtual void drawAirportRunways(const FlightRoute& route, const double& north_track_rotation);
    
    virtual void drawRouteNormalMode(const FlightRoute& route, const double& north_track_rotation);
    
    //! TOD
    void drawSpecialWaypoint(const Waypoint* wpt, 
                             const double& north_track_rotation,
                             const QColor& color);

    void drawWaypoint(const Waypoint* waypoint, 
                      const double& north_track_rotation, 
                      const QColor& wpt_color,
                      const QColor& holding_color,
                      bool do_stiple, 
                      bool draw_restrictions);

    void drawLeg(const Waypoint* from_wpt,
                 const Waypoint* to_wpt,
                 const double& north_track_rotation,
                 const QColor& leg_color,
                 bool do_stiple);

    void drawRouteMapMode(const FlightRoute& route);


    virtual void drawHSI(const double& north_track_rotation);

protected:

    void setupDefaultConfig();
    int rangeRingCount() const;

protected:

    GLuint m_hdg_bug_gllist;
    GLuint m_plane_gllist;
    GLuint m_range_ring_gllist;
    GLuint m_compass_circle_gllist;
    GLuint m_full_compass_gllist;
    GLuint m_plan_mod_compass_gllist;
    GLuint m_wpt_item_gllist;
    GLuint m_compass_box_gllist;
    GLuint m_hsi_gllist;
    double m_hsi_offset;
    double m_hsi_pointer_width;

	double m_compass_box_width;
    double m_compass_box_height;
    double m_hdg_bug_height;
    double m_hdg_triangle_width;
};

#endif /* __FMC_NAVDISPLAYSTYLE_H__ */

// End of file

