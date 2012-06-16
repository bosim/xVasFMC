//////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////

#include <QString>
#include <QStringList>
#include "vas_gl_format.h"

#include "defines.h"
#include "logger.h"
#include "config.h"
#include "navcalc.h"
#include "waypoint.h"
#include "flightstatus.h"
#include "vas_path.h"

#include "opengltext.h"
#include "fmc_control.h"
#include "fmc_navdisplay.h"
#include "fmc_navdisplay_defines.h"
#include "fmc_navdisplay_style_a.h"
#include "fmc_navdisplay_style_b.h"
#include "gldraw.h"

#include "fmc_navdisplay_glwidget.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

/////////////////////////////////////////////////////////////////////////////

GLNavdisplayWidget::GLNavdisplayWidget(ConfigWidgetProvider* config_widget_provider,
                                       Config* main_config,
                                       Config* navdisplay_config, 
                                       Config* tcas_config,
                                       FMCControl* fmc_control,
                                       VasWidget *parent,
                                       bool left_side) : 
    VasGLWidget(QGLFormat(QGL::SampleBuffers), parent),
    m_config_widget_provider(config_widget_provider),
    m_main_config(main_config),
    m_navdisplay_config(navdisplay_config),
    m_tcas_config(tcas_config),
    m_fmc_control(fmc_control),
    m_flightstatus(fmc_control->flightStatus()),
    m_projection(fmc_control->projection()),
    m_style(0),
    m_reset_metrics_flag(true),
    m_fps_count(0),
    m_route_clip_gllist(0),
    m_left_side(left_side)
{
    MYASSERT(m_config_widget_provider != 0);
    MYASSERT(m_main_config != 0);
    MYASSERT(m_navdisplay_config != 0);
    MYASSERT(m_fmc_control != 0);
    MYASSERT(m_flightstatus != 0);
    MYASSERT(m_projection != 0);

    m_max_drawable_y = 0;
    m_dist_scale_factor = 1.0;
	m_fps_counter_time.start();

	m_current_style = -1;
    m_current_mode = -1;
}

/////////////////////////////////////////////////////////////////////////////

GLNavdisplayWidget::~GLNavdisplayWidget() 
{
    delete m_style;
}

/////////////////////////////////////////////////////////////////////////////

void GLNavdisplayWidget::initializeGL()
{
    // setup opengl

    qglClearColor(Qt::black);
    //glShadeModel(GL_SMOOTH);
    glShadeModel(GL_FLAT); 
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);

    int smooth_hint = GL_DONT_CARE;
    if (m_main_config->getIntValue(CFG_BEST_ANTI_ALIASING) != 0) smooth_hint = GL_NICEST;
    
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, smooth_hint);
    glHint(GL_LINE_SMOOTH_HINT, smooth_hint);
    glHint(GL_POLYGON_SMOOTH_HINT, smooth_hint);

    // Delete old style, since the display lists it created can no longer be
    // used with the new pbuffer
    delete m_style;
    m_style=0;

    // setup font
    m_font = m_fmc_control->getGLFont();

    m_font->loadFont(
        VasPath::prependPath(m_main_config->getValue(CFG_FONT_NAME)), m_fmc_control->glFontSize());
}

/////////////////////////////////////////////////////////////////////////////

void GLNavdisplayWidget::resizeGL(int width, int height)
{
    m_reset_metrics_flag = true;
    m_size = QSize(width, height);

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, width, height, 0.0, 1.0, -1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/////////////////////////////////////////////////////////////////////////////

void GLNavdisplayWidget::refreshNavDisplay()
{
    if (!isVisible()) return;
    updateGL();
};

/////////////////////////////////////////////////////////////////////////////

void GLNavdisplayWidget::paintGL()
{
    if (!isVisible()) return;

    //TODO disabled PLAN mode for right ND - we have to split the
    //projection (see fmc processor) in order to make this work
    if (!m_left_side && (m_current_mode == CFG_ND_DISPLAY_MODE_NAV_PLAN || 
                         m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_NAV_PLAN))
    {
        m_fmc_control->slotEnableNDModeNavArc(m_left_side);
    }

    if (m_current_mode != m_fmc_control->currentNDMode(m_left_side))
    {
        if (!m_left_side && m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_NAV_PLAN)
        {
            //TODO disabled PLAN mode for right ND - we have to split the
            //projection (see fmc processor) in order to make this work
        }
        else
        {
            m_current_mode = m_fmc_control->currentNDMode(m_left_side);
            m_reset_metrics_flag = true;
        }
    }

    if (m_style==0 || m_main_config->getIntValue(CFG_STYLE) != m_current_style)
    {
        m_reset_metrics_flag = true;

        delete m_style;
        m_style = 0;

        m_current_style = m_main_config->getIntValue(CFG_STYLE);

        switch(m_current_style)
        {
            case(CFG_STYLE_A): {
                m_display_top_offset_font_factor = 3.0;
                m_style = new FMCNavdisplayStyleA(m_config_widget_provider, 
                                                  this,
                                                  m_main_config,
                                                  CFG_NAVDISPLAY_STYLE_A_FILENAME,
                                                  m_navdisplay_config,
                                                  m_tcas_config,
                                                  m_fmc_control,
                                                  m_projection,
                                                  m_size,
                                                  m_display_top_offset,
                                                  m_max_drawable_y,
                                                  m_position_center,
                                                  m_dist_scale_factor,
                                                  m_left_side);
                break;
            }
            case(CFG_STYLE_B): {
                m_display_top_offset_font_factor = 2.0;
                m_style = new FMCNavdisplayStyleB(m_config_widget_provider,
                                                  this,
                                                  m_main_config,
                                                  CFG_NAVDISPLAY_STYLE_B_FILENAME,
                                                  m_navdisplay_config,
                                                  m_tcas_config,
                                                  m_fmc_control,
                                                  m_projection,
                                                  m_size,
                                                  m_display_top_offset,
                                                  m_max_drawable_y,
                                                  m_position_center,
                                                  m_dist_scale_factor,
                                                  m_left_side);
                break;
            }
        }
    }

    setupStateBeforeDraw();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (m_style == 0 || (m_flightstatus->isValid() && !m_flightstatus->battery_on)) return;

    drawDisplay();
   
    glFlush();

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) Logger::log(QString("GLNavdisplayWidget:paintGL: GL Error: %1").arg(error));
}

/////////////////////////////////////////////////////////////////////////////

void GLNavdisplayWidget::setupStateBeforeDraw()
{
    if (m_reset_metrics_flag)
    {
        m_reset_metrics_flag = false;

        // recalc dimensions dependent on display mode

        if (m_current_mode == CFG_ND_DISPLAY_MODE_NAV_ARC)
        {
            m_display_top_offset = Navcalc::round(m_display_top_offset_font_factor * m_font->getHeight());
            m_display_bottom_offset = Navcalc::round(m_display_top_offset_font_factor * m_font->getHeight());
            m_fmc_control->slotEnableNDModeNavArc(m_left_side);
            m_max_drawable_y = m_size.height() - m_display_top_offset - m_display_bottom_offset;
            m_position_center = QPointF(m_size.width()/2.0, m_size.height() - m_display_bottom_offset);
            m_dist_scale_factor = m_max_drawable_y / (double)m_fmc_control->getNDRangeNM(m_left_side);
        }
        else
        {
            if (m_main_config->getIntValue(CFG_STYLE) == CFG_STYLE_A)
            {
                m_display_top_offset = Navcalc::round(2.5 * m_font->getHeight());
                m_display_bottom_offset = Navcalc::round(2.5 * m_font->getHeight());
                m_max_drawable_y = (m_size.height() - m_display_top_offset - m_display_bottom_offset) / 2;
                m_position_center = QPointF(m_size.width()/2.0, ((m_size.height()/2.0)) + (0.5*m_font->getHeight()));
            }
            else
            {
                m_display_top_offset = Navcalc::round(2.0 * m_font->getHeight());
                m_display_bottom_offset = Navcalc::round(2.0 * m_font->getHeight());
                m_max_drawable_y = (m_size.height() - m_display_top_offset - m_display_bottom_offset) / 2;
                m_position_center = QPointF(m_size.width()/2.0, m_size.height()/2.0);
            }
            
            m_dist_scale_factor = m_max_drawable_y / (double)m_fmc_control->getNDRangeNM(m_left_side);
        }

        // setup clipping list

        if (m_route_clip_gllist != 0) glDeleteLists(m_route_clip_gllist, 1);
        m_route_clip_gllist = glGenLists(1);
        MYASSERT(m_route_clip_gllist != 0);
        glNewList(m_route_clip_gllist, GL_COMPILE);
        glLoadIdentity();

        if (m_current_mode == CFG_ND_DISPLAY_MODE_NAV_ROSE ||
            m_current_mode == CFG_ND_DISPLAY_MODE_NAV_PLAN ||
            m_current_mode == CFG_ND_DISPLAY_MODE_VOR_ROSE ||
            m_current_mode == CFG_ND_DISPLAY_MODE_ILS_ROSE)
        {
            glTranslated(m_position_center.x(), m_position_center.y(), +0.5);
            
            GLDraw::drawFilledCircle(m_max_drawable_y, -M_PI, M_PI, 0.01f);
        }
        else
        {
            glTranslated(m_position_center.x(), m_position_center.y(), +0.5);

            GLDraw::drawFilledCircle(m_max_drawable_y, -M_PI/2, M_PI/2, 0.01f);

            // below the virt airplane

            double width = m_size.width() / 2.0;
            double height = m_size.height() - m_position_center.y();

            glBegin(GL_TRIANGLES);
            glVertex2d(0, height);
            glVertex2d(-width, -5);
            glVertex2d(+width, -5);
            glEnd();
        }

        glEndList();        

        if (m_style != 0)
            m_style->reset(m_size, m_display_top_offset, m_max_drawable_y, m_position_center, m_dist_scale_factor);
    }                                               
    else
    {
        double dist_scale_factor = 
            (m_size.height() - m_display_top_offset - m_display_bottom_offset) / 
            (double)m_fmc_control->getNDRangeNM(m_left_side);

        if (dist_scale_factor != m_dist_scale_factor)
        {
            m_dist_scale_factor = dist_scale_factor;
            if (m_style != 0)
                m_style->reset(m_size, m_display_top_offset, m_max_drawable_y, m_position_center, m_dist_scale_factor);
        }
    }
};

/////////////////////////////////////////////////////////////////////////////

void GLNavdisplayWidget::drawDisplay()
{
    if (m_flightstatus->isValid() && !m_flightstatus->avionics_on)
    {
        m_style->drawAvionicsOff();
        return;
    }

    //TODO make gllists whereever possible and look for potential performance boosts

    double north_track_rotation = m_flightstatus->smoothedTrueHeading();
    if (m_navdisplay_config->getIntValue(CFG_WIND_CORRECTION) != 0)
        north_track_rotation -= m_flightstatus->wind_correction_angle_deg;

    //TODO for left side only for now
    if (m_left_side) 
    {
        if (m_current_mode == CFG_ND_DISPLAY_MODE_NAV_PLAN)
        {
            enableRouteClipping();
            m_style->drawRouteMapMode(m_fmc_control->normalRoute());
            disableRouteClipping();
    
            m_style->drawRanges();
            m_style->drawCompass(north_track_rotation);
            m_style->drawWindArrow();
            m_style->drawInfoTexts(m_fmc_control->normalRoute());
            return;
        }
    }

    enableRouteClipping();

    if (m_current_mode == CFG_ND_DISPLAY_MODE_NAV_ROSE || m_current_mode == CFG_ND_DISPLAY_MODE_NAV_ARC)
    {
        m_style->drawGeoData(north_track_rotation, 
                             m_navdisplay_config->getColorValue(CFG_GEO_DATA_COLOR),
                             m_fmc_control->showGeoDataFilled());

        if (m_fmc_control->getNDRangeNM(m_left_side) < 90.0)
        {
            if (m_current_style == CFG_STYLE_A)
            {
                //TODO do this later only in descent and approach phase
                // draw estimated distance to loose alt for landing - energy circle
            }
            else
            {
                m_style->drawEnergyCircle(Qt::white, true);
            }
        }
    }
    
    m_style->drawLeftNavaid();
    m_style->drawRightNavaid();

    if (m_current_mode == CFG_ND_DISPLAY_MODE_NAV_ROSE || m_current_mode == CFG_ND_DISPLAY_MODE_NAV_ARC)
    {
        m_style->drawSurroundingItems(north_track_rotation);
        m_style->drawAirportRunways(m_fmc_control->normalRoute(), north_track_rotation);
        m_style->drawRouteNormalMode(m_fmc_control->normalRoute(), north_track_rotation);

        if (m_fmc_control->temporaryRoute().count() > 0)
            m_style->drawRouteNormalMode(m_fmc_control->temporaryRoute(), north_track_rotation);

        m_style->drawAltitudeReachDisplay(m_dist_scale_factor);
    }
        
    m_style->drawTcas(north_track_rotation);

    disableRouteClipping();
    
    m_style->drawRanges();
    m_style->drawCompass(north_track_rotation);
    m_style->drawWindArrow();

    if (m_current_mode == CFG_ND_DISPLAY_MODE_VOR_ROSE ||
        m_current_mode == CFG_ND_DISPLAY_MODE_ILS_ROSE)
        m_style->drawHSI(north_track_rotation);

    m_style->drawPlanePoly();

    m_style->drawInfoTexts(m_fmc_control->normalRoute());

    if (m_fmc_control->showFps()) drawFPS();
};

/////////////////////////////////////////////////////////////////////////////

void GLNavdisplayWidget::enableRouteClipping()
{
    vasglBeginClipRegion(m_size);

    // draw route clip poly for regions the route should be displayed
    glCallList(m_route_clip_gllist);

    vasglEndClipRegion();
}

/////////////////////////////////////////////////////////////////////////////

void GLNavdisplayWidget::disableRouteClipping()
{
    vasglDisableClipping();
}

/////////////////////////////////////////////////////////////////////////////

void GLNavdisplayWidget::drawFPS()
{
    ++m_fps_count;
    if (m_fps_counter_time.elapsed() >= 1000)
    {
        m_fps_counter_time.start();
        m_fps = m_fps_count;
        m_fps_count = 0;
    }

    glLoadIdentity();
    qglColor(Qt::white);
    m_font->drawText(2.0, m_font->getHeight() * 5.0, QString("%1fps").arg(m_fps));
}

