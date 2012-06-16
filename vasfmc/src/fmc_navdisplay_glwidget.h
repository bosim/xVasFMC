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

#ifndef FMCNAVDISPLAY_GLWIDGET_H
#define FMCNAVDISPLAY_GLWIDGET_H

#include <QDateTime>

#include "fmc_navdisplay_defines.h"
#include "vas_widget.h"
#include "vas_gl_widget.h"

class Config;
class ConfigWidgetProvider;
class FMCControl;
class FlightStatus;
class FMCNavdisplayStyle;
class ProjectionBase;
class OpenGLText;

/////////////////////////////////////////////////////////////////////////////

class GLNavdisplayWidget : public VasGLWidget
{
    Q_OBJECT

public:

    GLNavdisplayWidget(ConfigWidgetProvider* config_widget_provider,
                       Config* main_config,
                       Config* navdisplay_config,
                       Config* tcas_config,
                       FMCControl* fmc_control,
                       VasWidget *parent,
                       bool left_side);

    ~GLNavdisplayWidget();

 	void refreshNavDisplay();

    inline void reset() { updateGL(); }

protected:

    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

	void setupStateBeforeDraw();
	void drawDisplay();

    void drawFPS();

    void enableRouteClipping();
    void disableRouteClipping();

protected:
    
    ConfigWidgetProvider* m_config_widget_provider;

    Config* m_main_config;
	Config* m_navdisplay_config;
    Config* m_tcas_config;

    FMCControl* m_fmc_control;
    FlightStatus* m_flightstatus;
    const ProjectionBase* m_projection;

    QSize m_size;
    OpenGLText* m_font;

    FMCNavdisplayStyle* m_style;

	int m_current_style;
    int m_current_mode;
    
    bool m_reset_metrics_flag;

	// display vars

	unsigned int m_virt_plane_width;
	unsigned int m_virt_plane_height;

	double m_display_top_offset_font_factor;
	unsigned int m_display_top_offset;
	unsigned int m_display_bottom_offset;
	QPointF m_position_center;

	double m_dist_scale_factor;
	int m_nav_vor_point_width;
	int m_nav_ndb_point_width;
	int m_nav_fix_point_width;
	int m_max_drawable_y;

	// status vars

	QTime m_fps_counter_time;
	unsigned int m_fps;
	unsigned int m_fps_count;

    GLuint m_route_clip_gllist;

    bool m_left_side;
};

#endif // FMCNAVDISPLAY_GLWIDGET_H
