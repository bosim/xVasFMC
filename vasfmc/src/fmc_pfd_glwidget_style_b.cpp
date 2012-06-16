/////////////////////////////////////////////////////////////////////////////
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
#include "flightstatus.h"
#include "aircraft_data.h"
#include "vas_path.h"

#include "gldraw.h"
#include "fmc_control.h"
#include "fmc_pfd.h"

#include "fmc_pfd_glwidget_style_b.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

#define BLUE "#0A7DAA"
#define BROWN "#976D21"
#define DARKGRAY "#4B4B4B"

#define NOSE_BOX_X_FACTOR 1.5
#define FMA_QUAD_SPACING 2
#define MIN_SPEED 30.0

/////////////////////////////////////////////////////////////////////////////

GLPFDWidgetStyleB::GLPFDWidgetStyleB(ConfigWidgetProvider* config_widget_provider,
                                     Config* main_config,
                                     Config* pfd_config, 
                                     FMCControl* fmc_control,
                                     VasWidget *parent,
                                     bool left_side) : 
    GLPFDWidgetBase(config_widget_provider, main_config, pfd_config, fmc_control, parent, left_side),
    m_font(0),
    m_recalc_pfd(true),
    m_fps_count(0),
    m_onground(false),
    m_horizon_clip_gllist(0),
    m_horizon_gllist(0),
    m_horizon_static_marker_gllist(0),
    m_horizon_bank_marker_gllist(0),
    m_compass_gllist(0),
    m_alt_value_clip_gllist(0),
    m_pitch(0), m_bank(0)
{
    MYASSERT(m_config_widget_provider != 0);
    MYASSERT(m_main_config != 0);
    MYASSERT(m_pfd_config != 0);
    MYASSERT(m_fmc_control != 0);
    MYASSERT(m_flightstatus != 0);

	m_fps_counter_time.start();
}

/////////////////////////////////////////////////////////////////////////////

GLPFDWidgetStyleB::~GLPFDWidgetStyleB() 
{
    if (m_horizon_clip_gllist != 0) glDeleteLists(m_horizon_clip_gllist, 1);
    if (m_horizon_gllist != 0) glDeleteLists(m_horizon_gllist, 1);
    if (m_horizon_static_marker_gllist != 0) glDeleteLists(m_horizon_static_marker_gllist, 1);
    if (m_horizon_bank_marker_gllist != 0) glDeleteLists(m_horizon_bank_marker_gllist, 1);
    if (m_compass_gllist != 0) glDeleteLists(m_compass_gllist, 1);
    if (m_alt_value_clip_gllist != 0) glDeleteLists(m_alt_value_clip_gllist, 1);
}

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleB::initializeGL()
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

   // setup font

    m_font = m_fmc_control->getGLFont();

    m_font->loadFont(
        VasPath::prependPath(m_main_config->getValue(CFG_FONT_NAME)), m_fmc_control->glFontSize());
    m_font_height = m_font->getHeight();
    m_font_height_half = m_font_height/2.0;
}

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleB::resizeGL(int width, int height)
{
    m_recalc_pfd = true;
    m_size = QSize(width, height);

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, width, height, 0.0, 1.0, -1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleB::refreshPFD()
{
    if (!isVisible()) return;
    updateGL();
};

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleB::paintGL()
{
    if (!isVisible()) return;

    if (m_flightstatus->onground != m_onground)
    {
        m_onground = m_flightstatus->onground;
        m_recalc_pfd = true;
    }

    setupStateBeforeDraw();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_flightstatus->isValid() && !m_flightstatus->battery_on) return;

    drawDisplay();
    if (m_fmc_control->showFps()) drawFPS();
    glFlush();

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) Logger::log(QString("GLPFDWidgetStyleB:paintGL: GL Error: %1").arg(error));
}

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleB::setupStateBeforeDraw()
{
    if (m_recalc_pfd)
    {
        m_recalc_pfd = false;
        m_position_center = QPointF(m_size.width()/2.0, m_size.height()/2.0);
        m_horizon_offset = QPointF(m_size.width() * 0.44303, m_size.height() * 0.5);
        m_horizon_size = QSizeF(m_size.width() * 0.46202, m_size.height() * 0.46202);
        m_horizon_half_width = m_horizon_size.width()/2.0;
        m_horizon_half_height = m_horizon_size.height()/2.0;

        m_horizon_odd_pitch_line_width = m_horizon_size.width() * 0.20548 * 0.5;
        m_horizon_even_pitch_line_width = m_horizon_size.width() * 0.41096 * 0.5;
        m_horizon_limit_y_offset = -getVerticalOffsetByPitch(18.0);

        m_fd_width = m_horizon_size.width() * 0.57534 * 0.5;
        m_fd_height = m_horizon_size.height() * 0.57534 * 0.5;

        m_horizon_middle_marking_width = qMax(2.0, m_horizon_size.width() * 0.0274 * 0.5);
        m_horizon_middle_marking_height = qMax(2.0, m_horizon_size.height() * 0.0274 * 0.5);
        m_horizon_side_marking_offset = m_horizon_size.width() * 0.19178 ;
        m_horizon_side_marking_width = m_horizon_size.width() * 0.20548 ;
        m_horizon_side_marking_height = m_horizon_middle_marking_height;
        m_horizon_marking_border = 1.0;

        m_bank_marking_angle_inc = Navcalc::toRad(10.0);
        m_bank_marking_width = 2;
        m_bank_marking_height = m_horizon_size.width() * 0.04109;
        
        m_alt_band_half_height = m_size.height() * 0.67089 * 0.5;
        m_alt_band_width = m_horizon_size.width() * 0.27;
        m_alt_band_horiz_offset = m_horizon_size.width() * 0.64384;
        m_alt_tick_width = m_alt_band_width * 0.1;
        m_alt_value_horiz_offset = m_alt_band_horiz_offset+3*m_alt_tick_width;
        m_alt_value_box_height = m_font_height * 1.5;
        m_alt_range = 800;
        m_alt_pixel_per_ft = m_alt_band_half_height*2.0 / (double)m_alt_range;
        m_alt_last_two_digit_offset = getTextWidth("XX") * 1.1;

        m_rocd_band_half_height = m_size.height() * 0.46835 * 0.5;
        m_rocd_band_width = m_horizon_size.width() * 0.16438;
        m_rocd_band_horiz_offset = m_alt_band_horiz_offset + m_alt_band_width + getTextWidth("W")/2.0;
        m_rocd_root_offset = (m_rocd_band_half_height * 0.5 * m_rocd_band_width) / (m_rocd_band_half_height * 0.5);
        m_rocd_mark_500ft_y = 500.0 * m_horizon_half_height / 3200.0;

        m_ils_circle_size = 3.5;
        m_ils_bug_size = 1.7*m_ils_circle_size;

        m_speed_band_half_height = m_alt_band_half_height;
        m_speed_band_width = m_alt_band_width;
        m_speed_band_horiz_offset = -m_horizon_size.width() * 0.60274 - m_speed_band_width;
        m_speed_range = 120;
        m_speed_pixel_per_knot = m_speed_band_half_height*2.0 / (double)m_speed_range;
        m_speed_tick_width = m_speed_band_width * 0.1;
    
        m_speed_repaint_y_offset = m_speed_band_half_height;
        m_speed_repaint_height = m_speed_pixel_per_knot*10 + 2*m_font_height;

        m_hdg_half_angle = 35.0;
        m_hdg_band_radius = m_horizon_size.width() * 0.8;
        m_hdg_band_bottom_y_offset = m_size.height() - m_horizon_offset.y();
        m_hdg_band_vert_offset = m_hdg_band_bottom_y_offset - 
                                 (m_hdg_band_radius - (m_hdg_band_radius * cos(Navcalc::toRad(m_hdg_half_angle*1.5))));
        m_hdg_range = 35;
        m_hdg_pixel_per_deg = m_horizon_size.width() / (double)m_hdg_range;
        m_hdg_tick_width = m_alt_tick_width * 1.5;

        m_fma_box_half_width = m_size.width() * 0.53797 * 0.5;
        m_fma_box_height = qMax(m_size.height() * 0.06329, m_font_height*2);
        m_fma_box_item_half_width = m_fma_box_half_width / 3.0;
        m_fma_y_offset = 5;

        //-----

        createHorizonClipping();
        createHorizon();
        createHorizonStaticMarkings();
        createHorizonBankMarkings();
        createCompass();
        createAltitudeClipping();
    }
};

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleB::createHorizonClipping()
{
    if (m_horizon_clip_gllist != 0) glDeleteLists(m_horizon_clip_gllist, 1);
    m_horizon_clip_gllist = glGenLists(1);
    MYASSERT(m_horizon_clip_gllist != 0);
    glNewList(m_horizon_clip_gllist, GL_COMPILE);
    glLoadIdentity();
    glTranslated(m_horizon_offset.x(), m_horizon_offset.y(), +0.5);
    
    double corner_radius = m_horizon_size.width() * 0.06849;
    
    glBegin(GL_QUADS);
    glVertex2d(-m_horizon_half_width, -m_horizon_half_height+corner_radius);
    glVertex2d(+m_horizon_half_width, -m_horizon_half_height+corner_radius);
    glVertex2d(+m_horizon_half_width, +m_horizon_half_height-corner_radius);
    glVertex2d(-m_horizon_half_width, +m_horizon_half_height-corner_radius);
    glVertex2d(-m_horizon_half_width+corner_radius, -m_horizon_half_height);
    glVertex2d(+m_horizon_half_width-corner_radius, -m_horizon_half_height);
    glVertex2d(+m_horizon_half_width-corner_radius, +m_horizon_half_height);
    glVertex2d(-m_horizon_half_width+corner_radius, +m_horizon_half_height);
    glEnd();

    GLDraw::drawFilledCircleOffset(
        corner_radius, -M_PI/2, 0, -m_horizon_half_width+corner_radius, -m_horizon_half_height+corner_radius);

    GLDraw::drawFilledCircleOffset(
        corner_radius, 0, M_PI/2, +m_horizon_half_width-corner_radius, -m_horizon_half_height+corner_radius);

    GLDraw::drawFilledCircleOffset(
        corner_radius, -M_PI, -M_PI/2, -m_horizon_half_width+corner_radius, +m_horizon_half_height-corner_radius);

    GLDraw::drawFilledCircleOffset(
        corner_radius, M_PI/2, M_PI, +m_horizon_half_width-corner_radius, +m_horizon_half_height-corner_radius);
    
    glEndList();        
}

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleB::createAltitudeClipping()
{
    if (m_alt_value_clip_gllist != 0) glDeleteLists(m_alt_value_clip_gllist, 1);
    m_alt_value_clip_gllist = glGenLists(1);
    MYASSERT(m_alt_value_clip_gllist != 0);
    glNewList(m_alt_value_clip_gllist, GL_COMPILE);
    glLoadIdentity();
    glTranslated(m_horizon_offset.x(), m_horizon_offset.y(), +0.5);

    glBegin(GL_QUADS);
    glVertex2d(m_alt_value_horiz_offset, -m_alt_value_box_height);
    glVertex2d(m_alt_value_horiz_offset+m_alt_band_width, -m_alt_value_box_height);
    glVertex2d(m_alt_value_horiz_offset+m_alt_band_width, +m_alt_value_box_height);
    glVertex2d(m_alt_value_horiz_offset, +m_alt_value_box_height);
    glEnd();
    
    glEndList();        
}

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleB::createHorizon()
{
    if (m_horizon_gllist != 0) glDeleteLists(m_horizon_gllist, 1);
    m_horizon_gllist = glGenLists(1);
    MYASSERT(m_horizon_gllist != 0);
    glNewList(m_horizon_gllist, GL_COMPILE);

    glBegin(GL_QUADS);
    qglColor(BLUE);
    glVertex2d(-m_size.width(), -m_size.height());
    glVertex2d(+m_size.width(), -m_size.height());
    glVertex2d(+m_size.width(), 0);
    glVertex2d(-m_size.width(), 0);
    qglColor(BROWN);
    glVertex2d(-m_size.width(), 0);
    glVertex2d(+m_size.width(), 0);
    glVertex2d(+m_size.width(), +m_size.height());
    glVertex2d(-m_size.width(), +m_size.height());
    glEnd();

    qglColor(Qt::white);
    glLineWidth(1.5);

    double offset_inc = -getVerticalOffsetByPitch(2.5);
    double offset = offset_inc;
    double text_offset = getTextWidth("XX");

    for(int index=1; index <= 16; ++index)
    {
        if (index % 4 != 0)
        {
            double width = m_horizon_odd_pitch_line_width;
            if (index % 2 != 0) width *= 0.5;

            glBegin(GL_LINES);
            glVertex2d(-width, -offset);
            glVertex2d(+width, -offset);
            glVertex2d(-width, +offset);
            glVertex2d(+width, +offset);
            glEnd();
        }
        else
        {
            glBegin(GL_LINES);
            glVertex2d(-m_horizon_even_pitch_line_width, -offset);
            glVertex2d(+m_horizon_even_pitch_line_width, -offset);
            glVertex2d(-m_horizon_even_pitch_line_width, +offset);
            glVertex2d(+m_horizon_even_pitch_line_width, +offset);
            glEnd();

            QString pitch_text = QString::number(2.5 * index);
            drawText(-m_horizon_even_pitch_line_width-10-text_offset, -offset+m_font_height/2.0, pitch_text);
            drawText(+m_horizon_even_pitch_line_width+7,              -offset+m_font_height/2.0, pitch_text);
            drawText(-m_horizon_even_pitch_line_width-10-text_offset, +offset+m_font_height/2.0, pitch_text);
            drawText(+m_horizon_even_pitch_line_width+7,              +offset+m_font_height/2.0, pitch_text);
        }

        offset += offset_inc;
    }

    // horizon line
    glLineWidth(1.8);
    glBegin(GL_LINES);
    glVertex2d(-m_horizon_size.width(), 0);
    glVertex2d(+m_horizon_size.width(), 0);
    glEnd();

    glEndList();
}

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleB::createHorizonStaticMarkings()
{
    if (m_horizon_static_marker_gllist != 0) glDeleteLists(m_horizon_static_marker_gllist, 1);
    m_horizon_static_marker_gllist = glGenLists(1);
    MYASSERT(m_horizon_static_marker_gllist != 0);
    glNewList(m_horizon_static_marker_gllist, GL_COMPILE);

    glLineWidth(1.5);

    glBegin(GL_QUADS);
    qglColor(Qt::white);
    //middle
    glVertex2d(-m_horizon_middle_marking_width, -m_horizon_middle_marking_height);
    glVertex2d(+m_horizon_middle_marking_width, -m_horizon_middle_marking_height);
    glVertex2d(+m_horizon_middle_marking_width, +m_horizon_middle_marking_height);
    glVertex2d(-m_horizon_middle_marking_width, +m_horizon_middle_marking_height);
    //left
    glVertex2d(-m_horizon_side_marking_offset-m_horizon_side_marking_width, -m_horizon_side_marking_height);
    glVertex2d(-m_horizon_side_marking_offset, -m_horizon_side_marking_height);
    glVertex2d(-m_horizon_side_marking_offset, +m_horizon_side_marking_height);
    glVertex2d(-m_horizon_side_marking_offset-m_horizon_side_marking_width, +m_horizon_side_marking_height);
    //left down
    glVertex2d(-m_horizon_side_marking_offset, -m_horizon_side_marking_height);
    glVertex2d(-m_horizon_side_marking_offset, +m_horizon_side_marking_height*5);
    glVertex2d(-m_horizon_side_marking_offset-m_horizon_side_marking_height*2, +m_horizon_side_marking_height*5);
    glVertex2d(-m_horizon_side_marking_offset-m_horizon_side_marking_height*2, -m_horizon_side_marking_height);
    //right
    glVertex2d(+m_horizon_side_marking_offset+m_horizon_side_marking_width, -m_horizon_side_marking_height);
    glVertex2d(+m_horizon_side_marking_offset, -m_horizon_side_marking_height);
    glVertex2d(+m_horizon_side_marking_offset, +m_horizon_side_marking_height);
    glVertex2d(+m_horizon_side_marking_offset+m_horizon_side_marking_width, +m_horizon_side_marking_height);
    //right down
    glVertex2d(+m_horizon_side_marking_offset, -m_horizon_side_marking_height);
    glVertex2d(+m_horizon_side_marking_offset, +m_horizon_side_marking_height*5);
    glVertex2d(+m_horizon_side_marking_offset+m_horizon_side_marking_height*2, +m_horizon_side_marking_height*5);
    glVertex2d(+m_horizon_side_marking_offset+m_horizon_side_marking_height*2, -m_horizon_side_marking_height);

    qglColor(Qt::black);
    // middle
    glVertex2d(-m_horizon_middle_marking_width+m_horizon_marking_border, -m_horizon_middle_marking_height+m_horizon_marking_border);
    glVertex2d(+m_horizon_middle_marking_width-m_horizon_marking_border, -m_horizon_middle_marking_height+m_horizon_marking_border);
    glVertex2d(+m_horizon_middle_marking_width-m_horizon_marking_border, +m_horizon_middle_marking_height-m_horizon_marking_border);
    glVertex2d(-m_horizon_middle_marking_width+m_horizon_marking_border, +m_horizon_middle_marking_height-m_horizon_marking_border);
    //left
    glVertex2d(-m_horizon_side_marking_offset-m_horizon_side_marking_width+m_horizon_marking_border, -m_horizon_side_marking_height+m_horizon_marking_border);
    glVertex2d(-m_horizon_side_marking_offset-m_horizon_marking_border, -m_horizon_side_marking_height+m_horizon_marking_border);
    glVertex2d(-m_horizon_side_marking_offset-m_horizon_marking_border, +m_horizon_side_marking_height-m_horizon_marking_border);
    glVertex2d(-m_horizon_side_marking_offset-m_horizon_side_marking_width+m_horizon_marking_border, +m_horizon_side_marking_height-m_horizon_marking_border);
    //left down
    glVertex2d(-m_horizon_side_marking_offset-m_horizon_marking_border, -m_horizon_side_marking_height+m_horizon_marking_border);
    glVertex2d(-m_horizon_side_marking_offset-m_horizon_marking_border, +m_horizon_side_marking_height*5-m_horizon_marking_border);
    glVertex2d(-m_horizon_side_marking_offset-m_horizon_side_marking_height*2+m_horizon_marking_border, +m_horizon_side_marking_height*5-m_horizon_marking_border);
    glVertex2d(-m_horizon_side_marking_offset-m_horizon_side_marking_height*2+m_horizon_marking_border, -m_horizon_side_marking_height+m_horizon_marking_border);
    //right
    glVertex2d(+m_horizon_side_marking_offset+m_horizon_side_marking_width-m_horizon_marking_border, -m_horizon_side_marking_height+m_horizon_marking_border);
    glVertex2d(+m_horizon_side_marking_offset+m_horizon_marking_border, -m_horizon_side_marking_height+m_horizon_marking_border);
    glVertex2d(+m_horizon_side_marking_offset+m_horizon_marking_border, +m_horizon_side_marking_height-m_horizon_marking_border);
    glVertex2d(+m_horizon_side_marking_offset+m_horizon_side_marking_width-m_horizon_marking_border, +m_horizon_side_marking_height-m_horizon_marking_border);
    //right down
    glVertex2d(+m_horizon_side_marking_offset+m_horizon_marking_border, -m_horizon_side_marking_height+m_horizon_marking_border);
    glVertex2d(+m_horizon_side_marking_offset+m_horizon_marking_border, +m_horizon_side_marking_height*5-m_horizon_marking_border);
    glVertex2d(+m_horizon_side_marking_offset+m_horizon_side_marking_height*2-m_horizon_marking_border, +m_horizon_side_marking_height*5-m_horizon_marking_border);
    glVertex2d(+m_horizon_side_marking_offset+m_horizon_side_marking_height*2-m_horizon_marking_border, -m_horizon_side_marking_height+m_horizon_marking_border);
    glEnd();

    glEndList();
}

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleB::createHorizonBankMarkings()
{
    if (m_horizon_bank_marker_gllist != 0) glDeleteLists(m_horizon_bank_marker_gllist, 1);
    m_horizon_bank_marker_gllist = glGenLists(1);
    MYASSERT(m_horizon_bank_marker_gllist != 0);
    glNewList(m_horizon_bank_marker_gllist, GL_COMPILE);

    qglColor(Qt::white);
    glLineWidth(1.5);
    glRotated(-60, 0, 0, 1);

    for(int index = 0; index < 11; ++index)
    {
        switch(index)
        {
            case(0):
            case(10):
                glBegin(GL_LINES);
                glVertex2d(0, -m_horizon_half_height-m_bank_marking_height*0.5);
                glVertex2d(0, -m_horizon_half_height+m_bank_marking_height);
                glEnd();
                glRotated(+15, 0, 0, 1);
                break;
            case(1):
            case(9):
                glBegin(GL_LINES);
                glVertex2d(0, -m_horizon_half_height);
                glVertex2d(0, -m_horizon_half_height+m_bank_marking_height);
                glEnd();
                glRotated(+15, 0, 0, 1);
                break;
            case(2):
            case(8):
                glBegin(GL_LINES);
                glVertex2d(0, -m_horizon_half_height-m_bank_marking_height);
                glVertex2d(0, -m_horizon_half_height+m_bank_marking_height);
                glEnd();
                glRotated(+10, 0, 0, 1);
                break;
            case(3):
            case(4):
            case(6):
            case(7):
                glBegin(GL_LINES);
                glVertex2d(0, -m_horizon_half_height);
                glVertex2d(0, -m_horizon_half_height+m_bank_marking_height);
                glEnd();
                glRotated(+10, 0, 0, 1);
                break;    
            case(5):
                glBegin(GL_TRIANGLES);
                glVertex2d(0, -m_horizon_half_height+m_bank_marking_height);
                glVertex2d(-m_bank_marking_height*0.5, -m_horizon_half_height);
                glVertex2d(+m_bank_marking_height*0.5, -m_horizon_half_height);
                glEnd();
                glRotated(+10, 0, 0, 1);
                break;
        }
    }

    glEndList();
}

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleB::createCompass()
{
    glPushMatrix();

    if (m_compass_gllist) glDeleteLists(m_compass_gllist, 1);
    m_compass_gllist = glGenLists(1);
    MYASSERT(m_compass_gllist != 0);
    glNewList(m_compass_gllist, GL_COMPILE);

    double hdg_text_ytrans = m_hdg_tick_width + m_font_height;
    
    int alpha_inc = 5;
    int alpha = 0;
    for (; alpha <360; alpha += alpha_inc)
    {
        if (alpha % 10 == 0)
        {
            // draw scale line
            glBegin(GL_LINES);
            glVertex2d(0, -m_hdg_band_radius);
            glVertex2d(0, -m_hdg_band_radius + m_hdg_tick_width);
            glEnd();
            
            if (alpha % 20 == 0)
            {
                // draw scale text
                int hdg = alpha/10;
                QString hdg_text = QString::number(hdg);
                drawText(-getTextWidth(hdg_text)/2.0, -m_hdg_band_radius + hdg_text_ytrans, hdg_text);
            }
        }
        else
        {
            glBegin(GL_LINES);
            glVertex2d(0, -m_hdg_band_radius);
            glVertex2d(0, -m_hdg_band_radius + m_hdg_tick_width*0.5);
            glEnd();
        }
        
        glRotated(alpha_inc, 0, 0, 1);
    }

    glEndList();

    glPopMatrix();
}

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleB::drawDisplay()
{
    if (m_flightstatus->isValid() && !m_flightstatus->avionics_on)
    {
        qglColor(RED);
        drawText(m_size.width()/2.0, m_size.height()/2.0, "ATT");
        return;
    }

    m_pitch = m_flightstatus->smoothedPitch();
    m_bank = m_flightstatus->smoothedBank();

    glLoadIdentity();
    glTranslated(m_horizon_offset.x(), m_horizon_offset.y(), 0);

    drawHorizon();
    drawAltitude();
    drawSpeed();
    drawHeading();
    drawInfoTexts();
};

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleB::drawHorizon()
{
    glPushMatrix();

    enableClipping(m_horizon_clip_gllist);

    glPushMatrix();

    glRotated(m_bank, 0, 0, 1.0);
    glPushMatrix();

    // draw sky, earth and pitch lines
    double vertical_pitch_offset = getVerticalOffsetByPitch(m_pitch);
    glTranslated(0, vertical_pitch_offset , 0);
    glCallList(m_horizon_gllist);

    glPopMatrix();

    // upper limit

    double redraw_horizon_offset = 0.0;
    
    glBegin(GL_QUADS);
    double limit = -m_horizon_limit_y_offset;

    if (limit > vertical_pitch_offset)
    {
        qglColor(BROWN);
        glVertex2d(+m_horizon_size.width(), limit);
        glVertex2d(-m_horizon_size.width(), limit);
        glVertex2d(+m_horizon_size.width(), vertical_pitch_offset);
        glVertex2d(-m_horizon_size.width(), vertical_pitch_offset);
        limit = redraw_horizon_offset = vertical_pitch_offset;
    }

    qglColor(BLUE);
    glVertex2d(+m_horizon_size.width(), limit);
    glVertex2d(-m_horizon_size.width(), limit);
    glVertex2d(-m_horizon_size.width(), -m_size.height());
    glVertex2d(+m_horizon_size.width(), -m_size.height());

    // lower limit
    
    limit = +m_horizon_half_height - 3.0*m_ils_bug_size - m_font_height;

    if (limit < vertical_pitch_offset)
    {
        qglColor(BLUE);
        glVertex2d(-m_horizon_size.width(), limit);
        glVertex2d(+m_horizon_size.width(), limit);
        glVertex2d(+m_horizon_size.width(), vertical_pitch_offset);
        glVertex2d(-m_horizon_size.width(), vertical_pitch_offset);
        limit = redraw_horizon_offset = vertical_pitch_offset;
    }

    qglColor(BROWN);
    glVertex2d(-m_horizon_size.width(), limit);
    glVertex2d(+m_horizon_size.width(), limit);
    glVertex2d(+m_horizon_size.width(), +m_size.height());
    glVertex2d(-m_horizon_size.width(), +m_size.height());
    glEnd();

    // redraw horizon line when needed

    glLineWidth(1.8);
    qglColor(Qt::white);

    if (redraw_horizon_offset != 0.0)
    {
        glBegin(GL_LINES);
        glVertex2d(-m_horizon_size.width(), redraw_horizon_offset);
        glVertex2d(+m_horizon_size.width(), redraw_horizon_offset);
        glEnd();
    }
   
    // draw bank marking triangle

    double y_bottom = -m_horizon_half_height+m_bank_marking_height*2.5;

    glBegin(GL_LINE_LOOP);
    glVertex2d(0, -m_horizon_half_height+m_bank_marking_height);
    glVertex2d(-m_bank_marking_height, -m_horizon_half_height+m_bank_marking_height*2.5);
    glVertex2d(+m_bank_marking_height, -m_horizon_half_height+m_bank_marking_height*2.5);
    glEnd();

    double slip_x_offset = (m_bank_marking_height * 2.5) * -m_flightstatus->slip_percent * 0.01;
    glBegin(GL_LINE_LOOP);
    glVertex2d(-m_bank_marking_height - slip_x_offset, y_bottom);
    glVertex2d(+m_bank_marking_height - slip_x_offset, y_bottom);
    glVertex2d(+m_bank_marking_height - slip_x_offset, y_bottom + m_bank_marking_height * 0.6);
    glVertex2d(-m_bank_marking_height - slip_x_offset, y_bottom + m_bank_marking_height * 0.6);
    glEnd();

    // draw flight path

    if (m_fmc_control->fmcAutoPilot().isFlightPathModeEnabled())
    {
        double fpv = m_flightstatus->smoothedFPVVertical();

        double fpv_y = getVerticalOffsetByPitch(fpv+m_pitch);
        double fpv_x = -m_flightstatus->wind_correction_angle_deg * m_hdg_pixel_per_deg;
        double fpv_size = qMax(m_horizon_half_width * 0.125, 10.0);
        
        glLineWidth(2.0);
        qglColor(MAGENTA);

//TODO
//         //----- flight director

//         if (m_flightstatus->fd_active && !m_fmc_control->bankController()->isBankExcessive())
//         {
//             glPushMatrix();

//             glTranslated(fpv_x, fpv_y + getVerticalOffsetByPitch(m_pitch-m_flightstatus->smoothedFlightDirectorPitch()), 0.0);
//             glRotated(-m_flightstatus->smoothedFlightDirectorBank(), 0, 0, 1.0);

//             double circle_size = fpv_size*0.25;

//             glBegin(GL_LINES);
//             glVertex2d(-m_fd_width, 0);
//             glVertex2d(-circle_size, 0);
//             glVertex2d(+circle_size, 0);
//             glVertex2d(+m_fd_width, 0);
//             glEnd();

//             double triangle_size = m_fd_width / 10.0;

//             glBegin(GL_TRIANGLES);
//             glVertex2d(-m_fd_width, -triangle_size);
//             glVertex2d(-m_fd_width+triangle_size*2.0, 0);
//             glVertex2d(-m_fd_width, +triangle_size);
//             glVertex2d(+m_fd_width, -triangle_size);
//             glVertex2d(+m_fd_width-triangle_size*2.0, 0);
//             glVertex2d(+m_fd_width, +triangle_size);
//             glEnd();

//             GLDraw::drawCircle(circle_size, -M_PI, M_PI);

//             glPopMatrix();
//         }

        //----- flightpath

        glTranslated(fpv_x, fpv_y, 0.0);
        glRotated(-m_bank, 0, 0, 1.0);
        
        GLDraw::drawCircle(fpv_size*0.5, -M_PI, M_PI);
        
        glBegin(GL_LINES);
        glVertex2d(-fpv_size*0.5, 0);
        glVertex2d(-fpv_size*1.5, 0);
        glVertex2d(+fpv_size*0.5, 0);
        glVertex2d(+fpv_size*1.5, 0);
        glVertex2d(0, -fpv_size*0.5);
        glVertex2d(0, -fpv_size*1.3);
        glEnd();

    }

    glPopMatrix();

    // draw height above ground

    int height = (int)m_flightstatus->radarAltitude();

    if (height <= 2500)
    {
        if (height >= 50) height = height / 10 * 10;
        else if (height >= 10) height = height / 5 * 5;

        double y_off = m_horizon_half_height-2-m_ils_bug_size*2.0;
        double box_width = 5*getTextWidth("W") * 0.5;
        double box_height = m_font_height;

        qglColor(Qt::black);
        glBegin(GL_QUADS);
        glVertex2d(-box_width, +y_off-box_height);
        glVertex2d(+box_width, +y_off-box_height);
        glVertex2d(+box_width, +y_off);
        glVertex2d(-box_width, +y_off);
        glEnd();

        (m_fmc_data.decisionHeightFt() > 0 && height < (int)m_fmc_data.decisionHeightFt()) ?
            qglColor(AMBER) : qglColor(WHITE);

        QString height_text = QString::number(height);
        drawText(-getTextWidth(height_text)/2.0, y_off, height_text);
    }

    // draw flight director

    if (m_flightstatus->fd_active)
    {
        qglColor(Qt::magenta);
        glLineWidth(3.0);
        
        // pitch

        double fd_pitch_offset = -getVerticalOffsetByPitch(m_flightstatus->smoothedFlightDirectorPitch() - m_pitch);
        if (fd_pitch_offset > 0) fd_pitch_offset = qMin(fd_pitch_offset, m_fd_height);
        if (fd_pitch_offset < 0) fd_pitch_offset = qMax(fd_pitch_offset, -m_fd_height);

        glBegin(GL_LINES);
        glVertex2d(-m_fd_width, fd_pitch_offset);
        glVertex2d(+m_fd_width, fd_pitch_offset);
        glEnd();

        // bank

        double fd_bank_offset = getHorizontalOffsetByHeading(m_flightstatus->smoothedFlightDirectorBank() - m_bank);
        if (fd_bank_offset > 0) fd_bank_offset = qMin(fd_bank_offset, m_fd_width);
        if (fd_bank_offset < 0) fd_bank_offset = qMax(fd_bank_offset, -m_fd_width);

        glBegin(GL_LINES);
        glVertex2d(fd_bank_offset, -m_fd_height);
        glVertex2d(fd_bank_offset, +m_fd_height);
        glEnd();
    }

    // draw fixed markings

    glCallList(m_horizon_static_marker_gllist);

    disableClipping();

    // draw bank markings

    glCallList(m_horizon_bank_marker_gllist);

    glPopMatrix();
}

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleB::drawAltitude()
{
    glPushMatrix();

    double alt = m_flightstatus->smoothedAltimeterReadout();
    double vs = m_flightstatus->smoothedVS();

    double alt_fraction_y_offset = ( ((int)(alt * 100)) % 10000) / 100.0 * m_alt_pixel_per_ft;
    double alt_tape_y_offset = +m_alt_band_half_height + alt_fraction_y_offset - m_alt_pixel_per_ft*10;
    double alt_box_nose_height = m_alt_value_box_height * 0.25;

    //TODO make this non linear
    double limited_vs = LIMIT(vs, 3200.0);
    double rocd_indicator_left_y = -limited_vs * m_rocd_band_half_height / 3200.0;
    double rocd_indicator_right_y = m_rocd_root_offset * rocd_indicator_left_y / (m_rocd_band_width + m_rocd_root_offset);
    double rocd_mark_x_offset = m_alt_value_horiz_offset+m_alt_band_width + 4 - m_rocd_band_horiz_offset;

    qglColor(DARKGRAY);
    glLineWidth(1.0);

    //alt band
    glBegin(GL_QUADS);
    glVertex2d(m_alt_band_horiz_offset, -m_alt_band_half_height);
    glVertex2d(m_alt_band_horiz_offset+m_alt_band_width, -m_alt_band_half_height);
    glVertex2d(m_alt_band_horiz_offset+m_alt_band_width, +m_alt_band_half_height);
    glVertex2d(m_alt_band_horiz_offset, +m_alt_band_half_height);
    glEnd();

    // rocd band
    glBegin(GL_POLYGON);
    glVertex2d(m_rocd_band_horiz_offset, -m_rocd_band_half_height);
    glVertex2d(m_rocd_band_horiz_offset+m_rocd_band_width*0.5, -m_rocd_band_half_height);
    glVertex2d(m_rocd_band_horiz_offset+m_rocd_band_width, -m_rocd_band_half_height*0.6);
    glVertex2d(m_rocd_band_horiz_offset+m_rocd_band_width, +m_rocd_band_half_height*0.6);
    glVertex2d(m_rocd_band_horiz_offset+m_rocd_band_width*0.5, +m_rocd_band_half_height);
    glVertex2d(m_rocd_band_horiz_offset, +m_rocd_band_half_height);
    glEnd();

    // draw alt tape

    qglColor(Qt::white);
    glLineWidth(1.5);
    
    int start_alt = ((int)(alt) / 100 * 100) - m_alt_range/2;
    int max_alt = start_alt + m_alt_range + 20;
    for(int alt = start_alt; alt < max_alt; alt += 100)
    {
        double y = -(alt - start_alt - 10) * m_alt_pixel_per_ft;

        glBegin(GL_LINES);
        glVertex2d(m_alt_band_horiz_offset, y + alt_tape_y_offset);
        glVertex2d(m_alt_band_horiz_offset+m_alt_tick_width, y + alt_tape_y_offset);
        glEnd();

        if (alt % 200 == 0)
        {
            QString alt_text = QString::number(alt);
            drawText(m_alt_band_horiz_offset+m_alt_band_width-getTextWidth(alt_text)-2,
                     y+alt_tape_y_offset+m_font_height/2.0, alt_text);
        }
    }

    // draw yellow box for ground level

    double gnd_y = (alt - m_flightstatus->ground_alt_ft) * m_alt_pixel_per_ft;
    if (gnd_y < m_alt_band_half_height)
    {
        gnd_y = qMax(-m_horizon_half_height, gnd_y);
        glLineWidth(1.0);
        qglColor(Qt::yellow);
        //TODO make authentic
        glBegin(GL_QUADS);
        glVertex2d(m_alt_band_horiz_offset, gnd_y);
        glVertex2d(m_alt_band_horiz_offset+m_alt_tick_width, gnd_y);
        glVertex2d(m_alt_band_horiz_offset+m_alt_tick_width, gnd_y+m_alt_tick_width);
        glVertex2d(m_alt_band_horiz_offset, gnd_y+m_alt_tick_width);
        glEnd();
    }

    // draw AP selected alt marker

    qglColor(Qt::magenta);
    glLineWidth(1.8);
    
    double ap_alt_marker_y_offset = LIMIT((alt - m_flightstatus->APAlt()) * m_alt_pixel_per_ft, m_alt_band_half_height);

    QString sel_alt_string = 
        (m_fmc_control->isAltimeterSetToSTD(m_left_side)) ?
        QString("FL%1").arg(m_flightstatus->APAlt() / 100) :
        QString::number(m_flightstatus->APAlt() / 100 * 100);

    double ap_alt_marker_width = m_alt_tick_width * 1.5;
    
    glLineWidth(1.5);
    glBegin(GL_LINE_LOOP);
    glVertex2d(m_alt_band_horiz_offset-ap_alt_marker_width,     ap_alt_marker_y_offset-m_alt_value_box_height);
    glVertex2d(m_alt_value_horiz_offset,                        ap_alt_marker_y_offset-m_alt_value_box_height);
    glVertex2d(m_alt_value_horiz_offset,                        ap_alt_marker_y_offset+m_alt_value_box_height);
    glVertex2d(m_alt_band_horiz_offset-ap_alt_marker_width,     ap_alt_marker_y_offset+m_alt_value_box_height);
    glVertex2d(m_alt_band_horiz_offset-ap_alt_marker_width,     ap_alt_marker_y_offset+alt_box_nose_height);
    glVertex2d(m_alt_band_horiz_offset,                         ap_alt_marker_y_offset);
    glVertex2d(m_alt_band_horiz_offset-ap_alt_marker_width,     ap_alt_marker_y_offset-alt_box_nose_height);
    glEnd();
    
    // repaint alt tape above and below the drawing area with black

    qglColor(Qt::black);
    double repaint_y_offset = m_alt_band_half_height;
    double repaint_height = m_alt_pixel_per_ft*10 + alt_fraction_y_offset + 2*m_font_height;

    glBegin(GL_QUADS);
    // upper
    glVertex2d(m_alt_band_horiz_offset-m_alt_band_width, -repaint_y_offset);
    glVertex2d(m_alt_band_horiz_offset-m_alt_band_width, -repaint_y_offset-repaint_height);
    glVertex2d(m_alt_band_horiz_offset+m_alt_band_width, -repaint_y_offset-repaint_height);
    glVertex2d(m_alt_band_horiz_offset+m_alt_band_width, -repaint_y_offset);
    //lower
    glVertex2d(m_alt_band_horiz_offset-m_alt_band_width, repaint_y_offset);
    glVertex2d(m_alt_band_horiz_offset-m_alt_band_width, repaint_y_offset+repaint_height);
    glVertex2d(m_alt_band_horiz_offset+m_alt_band_width, repaint_y_offset+repaint_height);
    glVertex2d(m_alt_band_horiz_offset+m_alt_band_width, repaint_y_offset);
    glEnd();

    // draw AP selected alt text

    qglColor(Qt::magenta);
    drawText(m_alt_band_horiz_offset+5, -m_alt_band_half_height-m_font_height*0.5, sel_alt_string);

    if (m_fmc_control->showMetricAlt())
    {
        QString metric_alt = QString("%1M").arg(m_flightstatus->APAlt() * Navcalc::FEET_TO_METER, 0, 'f', 0);
        drawText(m_alt_band_horiz_offset+5, -m_alt_band_half_height-m_font_height*1.5, metric_alt);
    }

    // rocd marks

    qglColor(Qt::white);
    double rocd_mark_y_offset = 0;

    for(int index = 0; index <= 6; ++index)
    {
        if (index % 2 == 0)
        {
            glLineWidth(2.0);
            glBegin(GL_LINES);
            glVertex2d(m_rocd_band_horiz_offset+rocd_mark_x_offset,   rocd_mark_y_offset);
            glVertex2d(m_rocd_band_horiz_offset+rocd_mark_x_offset+5, rocd_mark_y_offset);
            glVertex2d(m_rocd_band_horiz_offset+rocd_mark_x_offset,   -rocd_mark_y_offset);
            glVertex2d(m_rocd_band_horiz_offset+rocd_mark_x_offset+5, -rocd_mark_y_offset);
            glEnd();

            if (index > 0)
            {
                QString rocd_value = QString::number(index/2);
                drawText(m_rocd_band_horiz_offset+2, -rocd_mark_y_offset+m_font_height*0.5, rocd_value);
                drawText(m_rocd_band_horiz_offset+2, +rocd_mark_y_offset+m_font_height*0.5, rocd_value);
            }
        }
        else
        {
            glLineWidth(1.5);
            glBegin(GL_LINES);
            glVertex2d(m_rocd_band_horiz_offset+rocd_mark_x_offset,   rocd_mark_y_offset);
            glVertex2d(m_rocd_band_horiz_offset+rocd_mark_x_offset+2, rocd_mark_y_offset);
            glVertex2d(m_rocd_band_horiz_offset+rocd_mark_x_offset,   -rocd_mark_y_offset);
            glVertex2d(m_rocd_band_horiz_offset+rocd_mark_x_offset+2, -rocd_mark_y_offset);
            glEnd();
        }

        rocd_mark_y_offset += m_rocd_mark_500ft_y;
    }

    // rocd alt value notch

    qglColor(Qt::black);
    glBegin(GL_QUADS);
    glVertex2d(m_rocd_band_horiz_offset, -m_alt_value_box_height*1.7);
    glVertex2d(m_rocd_band_horiz_offset+rocd_mark_x_offset, -m_alt_value_box_height*1.4);
    glVertex2d(m_rocd_band_horiz_offset+rocd_mark_x_offset, +m_alt_value_box_height*1.4);
    glVertex2d(m_rocd_band_horiz_offset, +m_alt_value_box_height*1.7);
    glEnd();

    // repaint alt value background

    qglColor(BLACK);
    glLineWidth(1.5);

    // black box
    glBegin(GL_QUADS);
    glVertex2d(m_alt_value_horiz_offset, -m_alt_value_box_height);
    glVertex2d(m_alt_value_horiz_offset+m_alt_band_width, -m_alt_value_box_height);
    glVertex2d(m_alt_value_horiz_offset+m_alt_band_width, +m_alt_value_box_height);
    glVertex2d(m_alt_value_horiz_offset, +m_alt_value_box_height);
    glEnd();

    // white box border
    qglColor(Qt::white);
    glBegin(GL_LINE_LOOP);
    glVertex2d(m_alt_value_horiz_offset, -m_alt_value_box_height);
    glVertex2d(m_alt_value_horiz_offset+m_alt_band_width, -m_alt_value_box_height);
    glVertex2d(m_alt_value_horiz_offset+m_alt_band_width, +m_alt_value_box_height);
    glVertex2d(m_alt_value_horiz_offset, +m_alt_value_box_height);
    glEnd();

    // black nose
    qglColor(Qt::black);
    glBegin(GL_TRIANGLES);
    glVertex2d(m_alt_value_horiz_offset+2, -alt_box_nose_height);
    glVertex2d(m_alt_value_horiz_offset-m_alt_tick_width*NOSE_BOX_X_FACTOR, 0);
    glVertex2d(m_alt_value_horiz_offset+2, +alt_box_nose_height);
    glEnd();

    // white nose border 
    qglColor(Qt::white);
    glBegin(GL_LINES);
    glVertex2d(m_alt_value_horiz_offset-m_alt_tick_width*NOSE_BOX_X_FACTOR, 0);
    glVertex2d(m_alt_value_horiz_offset, -alt_box_nose_height);
    glVertex2d(m_alt_value_horiz_offset, +alt_box_nose_height);
    glVertex2d(m_alt_value_horiz_offset-m_alt_tick_width*NOSE_BOX_X_FACTOR, 0);
    glEnd();
    
    // draw alt value

    enableClipping(m_alt_value_clip_gllist);
    
    qglColor(Qt::white);
    
    int rounded_alt = Navcalc::round(alt); 
    int last_tow_digit_alt_value = rounded_alt % 100;
    double alt_remainder_20 = fmod(alt, 20.0);
    double last_two_digit_y_offset = m_font_height / 20.0 * (rounded_alt % 20);
    int clipped_last_tow_digit_alt_value = last_tow_digit_alt_value / 20 * 20;

    QString alt_text = QString::number(rounded_alt / 100);
    if (alt_remainder_20 > 10 && clipped_last_tow_digit_alt_value == 80) 
        alt_text = QString::number((rounded_alt + 100) / 100);

    clipped_last_tow_digit_alt_value -= 40;
    if (clipped_last_tow_digit_alt_value < 0) clipped_last_tow_digit_alt_value = 100 + clipped_last_tow_digit_alt_value;

    double last_two_digit_x = m_alt_value_horiz_offset + m_alt_band_width - m_alt_last_two_digit_offset - 2;

    for(int index = +2; index >= -2; --index)
    {
        if (clipped_last_tow_digit_alt_value < 0) 
            clipped_last_tow_digit_alt_value = 100 - clipped_last_tow_digit_alt_value;

        drawText(last_two_digit_x, m_font_height/2.0 + last_two_digit_y_offset + (index*m_font_height),
                 QString("%1").arg(clipped_last_tow_digit_alt_value, 2, 10, QChar('0')));

        clipped_last_tow_digit_alt_value = (clipped_last_tow_digit_alt_value + 20) % 100;
    }

    disableClipping();

    drawText(m_alt_value_horiz_offset + m_alt_band_width - m_alt_last_two_digit_offset - getTextWidth(alt_text) - 4, 
             m_font_height/2.0, alt_text);

    // draw metric alt

    if (m_fmc_control->showMetricAlt())
    {
        qglColor(BLACK);
        glLineWidth(1.5);
        
        double x = m_alt_value_horiz_offset - getTextWidth("W");
        double width = m_alt_band_width + getTextWidth("W");

        // black box
        glBegin(GL_QUADS);
        glVertex2d(x, -m_alt_value_box_height);
        glVertex2d(x, -m_alt_value_box_height-m_font_height);
        glVertex2d(x+width, -m_alt_value_box_height-m_font_height);
        glVertex2d(x+width, -m_alt_value_box_height);
        glEnd();
        
        // white box border
        qglColor(Qt::white);
        glBegin(GL_LINE_LOOP);
        glVertex2d(x, -m_alt_value_box_height);
        glVertex2d(x, -m_alt_value_box_height-m_font_height);
        glVertex2d(x+width, -m_alt_value_box_height-m_font_height);
        glVertex2d(x+width, -m_alt_value_box_height);
        glEnd();

        QString metric_alt = QString("%1M").arg(
            m_flightstatus->smoothedAltimeterReadout() * Navcalc::FEET_TO_METER, 0, 'f', 0);

        drawText(x + 0.5* (width - getTextWidth(metric_alt)), -m_font_height*1.5, metric_alt);
    }

    // draw rocd

    qglColor(WHITE);
    glLineWidth(2.0);
    glBegin(GL_LINES);
    glVertex2d(m_rocd_band_horiz_offset+rocd_mark_x_offset+2, rocd_indicator_left_y);
    glVertex2d(m_rocd_band_horiz_offset+m_rocd_band_width-1, rocd_indicator_right_y);
    glEnd();

    if (qAbs(vs) >= 200)
    {
        QString rocd_value = QString::number(Navcalc::round( qAbs(vs) / 100.0 ) * 100);
        if (vs > 0) drawText(m_rocd_band_horiz_offset+3, -m_rocd_band_half_height, rocd_value);
        else        drawText(m_rocd_band_horiz_offset+3, +m_rocd_band_half_height+m_font_height, rocd_value);
    }

    // draw ILS

    if ((m_left_side && m_fmc_control->showPFDILS(true) && !m_flightstatus->nav1.id().isEmpty() && m_flightstatus->nav1_has_loc) ||
        (!m_left_side && m_fmc_control->showPFDILS(false) && !m_flightstatus->nav2.id().isEmpty() && m_flightstatus->nav2_has_loc))
    {
        double ils_circle_offset = m_horizon_size.width() / 16.0;

        // draw loc & gs

        double loc_y_offset = m_horizon_half_height - m_ils_bug_size * 1.2;
        double loc_needle_offset = (4*ils_circle_offset / 127.0) * 
                                   (m_left_side ? m_flightstatus->obs1_loc_needle : m_flightstatus->obs2_loc_needle);

        double gs_x_offset = 
            m_horizon_half_width -
            (m_horizon_half_width - m_horizon_side_marking_offset - m_horizon_side_marking_width) / 2.0;
        double gs_needle_offset = (4*ils_circle_offset / 127) * 
                                  (m_left_side ? m_flightstatus->obs1_gs_needle : m_flightstatus->obs2_gs_needle);

        qglColor(Qt::white);
        glLineWidth(2.0);

        // loc middle line
        glBegin(GL_LINES);
        glVertex2d(0, loc_y_offset-m_ils_bug_size);
        glVertex2d(0, loc_y_offset+m_ils_bug_size);
        glEnd();    

        // gs middle line
        glBegin(GL_LINES);
        glVertex2d(gs_x_offset-m_ils_bug_size, 0);
        glVertex2d(gs_x_offset+m_ils_bug_size, 0);
        glEnd();    

        // loc
        GLDraw::drawCircleOffset(m_ils_circle_size, -M_PI, M_PI, -4*ils_circle_offset, loc_y_offset);
        GLDraw::drawCircleOffset(m_ils_circle_size, -M_PI, M_PI, -2*ils_circle_offset, loc_y_offset);
        GLDraw::drawCircleOffset(m_ils_circle_size, -M_PI, M_PI, +2*ils_circle_offset, loc_y_offset);
        GLDraw::drawCircleOffset(m_ils_circle_size, -M_PI, M_PI, +4*ils_circle_offset, loc_y_offset);

        // gs
        GLDraw::drawCircleOffset(m_ils_circle_size, -M_PI, M_PI, gs_x_offset, -4*ils_circle_offset);
        GLDraw::drawCircleOffset(m_ils_circle_size, -M_PI, M_PI, gs_x_offset, -2*ils_circle_offset);
        GLDraw::drawCircleOffset(m_ils_circle_size, -M_PI, M_PI, gs_x_offset, +2*ils_circle_offset);
        GLDraw::drawCircleOffset(m_ils_circle_size, -M_PI, M_PI, gs_x_offset, +4*ils_circle_offset);

        // loc & gs needles
        glBegin(GL_QUADS);
        qglColor(Qt::white);
        // loc
        glVertex2d(loc_needle_offset-m_ils_bug_size, loc_y_offset);
        glVertex2d(loc_needle_offset, -m_ils_bug_size+loc_y_offset);
        glVertex2d(loc_needle_offset+m_ils_bug_size, loc_y_offset);
        glVertex2d(loc_needle_offset, +m_ils_bug_size+loc_y_offset);
        //gs
        glVertex2d(gs_x_offset-m_ils_bug_size, gs_needle_offset);
        glVertex2d(gs_x_offset, -m_ils_bug_size+gs_needle_offset);
        glVertex2d(gs_x_offset+m_ils_bug_size, gs_needle_offset);
        glVertex2d(gs_x_offset, +m_ils_bug_size+gs_needle_offset);
        qglColor(Qt::black);
        // loc
        glVertex2d(loc_needle_offset-m_ils_bug_size+1, loc_y_offset);
        glVertex2d(loc_needle_offset, -m_ils_bug_size+loc_y_offset+1);
        glVertex2d(loc_needle_offset+m_ils_bug_size-1, loc_y_offset);
        glVertex2d(loc_needle_offset, +m_ils_bug_size+loc_y_offset-1);
        // gs
        glVertex2d(gs_x_offset-m_ils_bug_size+1, gs_needle_offset);
        glVertex2d(gs_x_offset, -m_ils_bug_size+gs_needle_offset+1);
        glVertex2d(gs_x_offset+m_ils_bug_size-1, gs_needle_offset);
        glVertex2d(gs_x_offset, +m_ils_bug_size+gs_needle_offset-1);
        glEnd();
    }

    glPopMatrix();
}

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleB::drawSpeed()
{
    glPushMatrix();

    // draw speed

    double ias_trend_per_second = 0.0;
    double ias = m_flightstatus->smoothedIAS(&ias_trend_per_second);
    double speed_fraction_y_offset = ( ((int)(ias * 100)) % 1000) / 100.0 * m_speed_pixel_per_knot;

    qglColor(DARKGRAY);
    glLineWidth(1.5);

    glBegin(GL_QUADS);
    glVertex2d(m_speed_band_horiz_offset, -m_speed_band_half_height);
    glVertex2d(m_speed_band_horiz_offset+m_speed_band_width, -m_speed_band_half_height);
    glVertex2d(m_speed_band_horiz_offset+m_speed_band_width, +m_speed_band_half_height);
    glVertex2d(m_speed_band_horiz_offset, +m_speed_band_half_height);
    glEnd();

    // draw speed tape

    qglColor(Qt::white);
    glLineWidth(2.0);
    int start_speed = ((int)(ias) / 10 * 10) - m_speed_range/2;

    int max_speed = start_speed + m_speed_range + 20;
    for(int speed = start_speed; speed < max_speed; speed += 10)
    {
        if (speed < MIN_SPEED) continue;
        double y = (start_speed - speed) * m_speed_pixel_per_knot + m_speed_band_half_height;
        glBegin(GL_LINES);
        glVertex2d(m_speed_band_horiz_offset+m_speed_band_width, y + speed_fraction_y_offset);
        glVertex2d(m_speed_band_horiz_offset+m_speed_band_width-m_speed_tick_width, y + speed_fraction_y_offset);
        glEnd();
        if ((speed/10) % 2 == 0) 
        {
            QString speed_text = QString::number(speed);
            drawText(m_speed_band_horiz_offset + m_speed_band_width*0.5 - getTextWidth(speed_text)*0.5 - m_speed_tick_width,
                     y + speed_fraction_y_offset + m_font_height/2.0, speed_text);
        }
    }

    // draw overspeed area

    double overspeed_y_offset = 
        qMin(m_horizon_half_height-1, (ias - m_fmc_control->aircraftData().getMaxSpeedKts()) * m_speed_pixel_per_knot);

    if (m_fmc_control->aircraftData().getMaxSpeedKts() > 10 && overspeed_y_offset > -m_speed_band_half_height)
    {
        double overspeed_x_offset = m_speed_band_horiz_offset + m_speed_band_width;

        glLineWidth(1.5);
        qglColor(Qt::red);
        glBegin(GL_QUADS);
        glVertex2d(overspeed_x_offset+1, overspeed_y_offset);
        glVertex2d(overspeed_x_offset+1, -m_speed_band_half_height+1);
        glVertex2d(overspeed_x_offset+6, -m_speed_band_half_height+1);
        glVertex2d(overspeed_x_offset+6, overspeed_y_offset);
        glEnd();
    }

//TODO
//     // draw green dot speed

//     if (m_fmc_control->aircraftData().getGreenDotSpeed() > 0)
//     {
//         double y_offset = (ias - m_fmc_control->aircraftData().getGreenDotSpeed()) * m_spd_pixel_per_knot;
//         if (y_offset < m_horizon_half_height && y_offset > -m_horizon_half_height)
//         {
//             qglColor(Qt::green);
//             GLDraw::drawCircleOffset(speed_tick_width/2.0, -M_PI, M_PI, x_offset, y_offset, 0.1);
//         }
//     }

    // draw speed trend
    
    if (qAbs(ias_trend_per_second*10) > 1)
    {
        double height = m_speed_pixel_per_knot * -ias_trend_per_second * 10;
        height = LIMIT(height, m_speed_band_half_height);
        double x = m_speed_band_horiz_offset-2+m_speed_band_width-m_speed_tick_width*NOSE_BOX_X_FACTOR;

        glLineWidth(1.5);
        qglColor(Qt::white);
        if (height < 0) GLDraw::drawVerticalArrow(x, height, x, 0, 4, 4, true, true, true);
        else            GLDraw::drawVerticalArrow(x, 0, x, height, 4, 4, false, true, true);
    }

    // draw AP selected speed pointer

    if (m_flightstatus->APSpd() > 0)
    {
        qglColor(Qt::magenta);
        glLineWidth(1.8);

        double effective_ap_speed = m_flightstatus->APSpd();
        if (m_fmc_control->fmcAutothrottle().isAPThrottleModeMachSet())
        {
            effective_ap_speed = 
                Navcalc::getIasFromMach(
                    m_flightstatus->APMach(), m_flightstatus->oat, 
                    m_flightstatus->smoothedIAS(), m_flightstatus->tas, m_flightstatus->mach);
        }

        double ap_speed_marker_x_offset = m_speed_band_horiz_offset + m_speed_band_width;
        double ap_speed_marker_y_offset = 
            LIMIT((ias - effective_ap_speed) * m_speed_pixel_per_knot, m_speed_band_half_height);
        double ap_speed_marker_width = m_speed_tick_width*NOSE_BOX_X_FACTOR;
            
        glLineWidth(1.5);
        glBegin(GL_LINE_LOOP);
        glVertex2d(+ap_speed_marker_x_offset - m_speed_tick_width*NOSE_BOX_X_FACTOR, ap_speed_marker_y_offset);
        glVertex2d(+ap_speed_marker_x_offset, ap_speed_marker_y_offset-m_speed_tick_width);
        glVertex2d(+ap_speed_marker_x_offset+ap_speed_marker_width*2, ap_speed_marker_y_offset-m_speed_tick_width);
        glVertex2d(+ap_speed_marker_x_offset+ap_speed_marker_width*2, ap_speed_marker_y_offset+m_speed_tick_width);
        glVertex2d(+ap_speed_marker_x_offset, ap_speed_marker_y_offset+m_speed_tick_width);
        glEnd();
    }

    // repaint speed tape above and below the drawing area with black

    qglColor(Qt::black);

    glBegin(GL_QUADS);
    // upper
    glVertex2d(m_speed_band_horiz_offset, -m_speed_repaint_y_offset);
    glVertex2d(m_speed_band_horiz_offset, -m_speed_repaint_y_offset-m_speed_repaint_height);
    glVertex2d(m_speed_band_horiz_offset+m_speed_band_width*2, -m_speed_repaint_y_offset-m_speed_repaint_height);
    glVertex2d(m_speed_band_horiz_offset+m_speed_band_width*2, -m_speed_repaint_y_offset);
    //lower
    glVertex2d(m_speed_band_horiz_offset, m_speed_repaint_y_offset);
    glVertex2d(m_speed_band_horiz_offset, m_speed_repaint_y_offset+m_speed_repaint_height);
    glVertex2d(m_speed_band_horiz_offset+m_speed_band_width*2, m_speed_repaint_y_offset+m_speed_repaint_height);
    glVertex2d(m_speed_band_horiz_offset+m_speed_band_width*2, m_speed_repaint_y_offset);
    glEnd();

    // repaint speed value background

    double speed_value_box_width = m_speed_band_width - 2*NOSE_BOX_X_FACTOR*m_speed_tick_width;
    double speed_value_box_height = m_font_height * 1.5;

    glLineWidth(1.5);

    // black box
    qglColor(Qt::black);
    glBegin(GL_QUADS);
    glVertex2d(m_speed_band_horiz_offset, -speed_value_box_height);
    glVertex2d(m_speed_band_horiz_offset+speed_value_box_width, -speed_value_box_height);
    glVertex2d(m_speed_band_horiz_offset+speed_value_box_width, +speed_value_box_height);
    glVertex2d(m_speed_band_horiz_offset, +speed_value_box_height);
    glEnd();

    // white box border
    qglColor(Qt::white);
    glBegin(GL_LINE_LOOP);
    glVertex2d(m_speed_band_horiz_offset, -speed_value_box_height);
    glVertex2d(m_speed_band_horiz_offset+speed_value_box_width, -speed_value_box_height);
    glVertex2d(m_speed_band_horiz_offset+speed_value_box_width, +speed_value_box_height);
    glVertex2d(m_speed_band_horiz_offset, +speed_value_box_height);
    glEnd();

    // black nose
    qglColor(Qt::black);
    glBegin(GL_TRIANGLES);
    glVertex2d(m_speed_band_horiz_offset+speed_value_box_width-2, -m_speed_tick_width*NOSE_BOX_X_FACTOR*0.7);
    glVertex2d(m_speed_band_horiz_offset+speed_value_box_width+m_speed_tick_width*NOSE_BOX_X_FACTOR, 0);
    glVertex2d(m_speed_band_horiz_offset+speed_value_box_width-2, +m_speed_tick_width*NOSE_BOX_X_FACTOR*0.7);
    glEnd();

    // white nose border 
    qglColor(Qt::white);
    glBegin(GL_LINES);
    glVertex2d(m_speed_band_horiz_offset+speed_value_box_width, -m_speed_tick_width*NOSE_BOX_X_FACTOR*0.7);
    glVertex2d(m_speed_band_horiz_offset+speed_value_box_width+m_speed_tick_width*NOSE_BOX_X_FACTOR, 0);
    glVertex2d(m_speed_band_horiz_offset+speed_value_box_width+m_speed_tick_width*NOSE_BOX_X_FACTOR, 0);
    glVertex2d(m_speed_band_horiz_offset+speed_value_box_width, +m_speed_tick_width*NOSE_BOX_X_FACTOR*0.7);
    glEnd();
    
    // draw speed value
    
    QString speed_text = QString("%1").arg(Navcalc::round(ias), 2, 10, QChar('0'));
    drawText(m_speed_band_horiz_offset+speed_value_box_width-getTextWidth(speed_text)-2, m_font_height/2.0, speed_text);

    // draw V1, Vr

    if (m_flightstatus->radarAltitude() < 50)
    {
        qglColor(Qt::green);
        glLineWidth(2.0);

        bool no_v_speeds = true;
        double x_offset = m_speed_band_horiz_offset+m_speed_band_width;

        if (m_fmc_data.V1() > 0)
        {
            no_v_speeds = false;

            double y_offset = (ias - m_fmc_data.V1()) * m_speed_pixel_per_knot;

            if (y_offset < -m_horizon_half_height)
            {
                drawText(x_offset+3, m_font_height - (int)m_speed_band_half_height, "V1");
                drawText(x_offset+3, 2*m_font_height - (int)m_speed_band_half_height,
                         QString::number(m_fmc_data.V1()));
            }
            else if (y_offset < m_horizon_half_height)
            {
                glBegin(GL_LINES);
                glVertex2d(x_offset, y_offset);
                glVertex2d(x_offset+m_speed_tick_width*0.75, y_offset);
                glEnd();
                drawText(x_offset+m_speed_tick_width, y_offset+m_font_height_half, "V1");
            }
        }

        if (m_fmc_data.Vr() > 0)
        {
            no_v_speeds = false;
            double y_offset = (ias - m_fmc_data.Vr()) * m_speed_pixel_per_knot;
            if (y_offset < m_horizon_half_height && y_offset > -m_horizon_half_height)
            {
                glBegin(GL_LINES);
                glVertex2d(x_offset, y_offset);
                glVertex2d(x_offset+m_speed_tick_width*0.75, y_offset);
                glEnd();
                drawText(x_offset+m_speed_tick_width, y_offset+m_font_height_half, "VR");
            }
        }

        if (m_fmc_data.V2() > 0)
        {
            no_v_speeds = false;
            double y_offset = (ias - m_fmc_data.V2()) * m_speed_pixel_per_knot;
            if (y_offset < m_horizon_half_height && y_offset > -m_horizon_half_height)
            {
                glBegin(GL_LINES);
                glVertex2d(x_offset, y_offset);
                glVertex2d(x_offset+m_speed_tick_width*0.75, y_offset);
                glEnd();
                drawText(x_offset+m_speed_tick_width, y_offset+m_font_height_half, "V2");
            }
        }

        if (no_v_speeds)
        {
            //TODO
        }
    }

    // draw AP selected speed text

    qglColor(Qt::magenta);
    QString sel_ap_speed_string;

    if (m_fmc_control->fmcAutothrottle().isAPThrottleModeMachSet())
        sel_ap_speed_string = QString("%1").arg(m_flightstatus->APMach(), 0, 'f', 2);
    else                              
        sel_ap_speed_string = QString::number(m_flightstatus->APSpd());

    drawText(m_speed_band_horiz_offset+5, -m_speed_band_half_height-m_font_height*0.5, sel_ap_speed_string);

    glPopMatrix();
}

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleB::drawHeading()
{
    glPushMatrix();

    double mag_hdg = m_flightstatus->smoothedMagneticHeading();
    //double hdg_fraction_x_offset = ( ((int)(mag_hdg * 100)) % 1000) / 100.0 * m_hdg_pixel_per_deg;

    // draw compass background
    
    qglColor(DARKGRAY);
    glLineWidth(1.5);

    GLDraw::drawFilledCircleOffset(m_hdg_band_radius, -M_PI/3.0, M_PI/3.0, 
                                   0, m_hdg_band_vert_offset+m_hdg_band_radius);

    qglColor(Qt::white);
    glLineWidth(2.0);

//     //TODO
//     glBegin(GL_LINE_LOOP);
//     glVertex2d(-m_hdg_band_radius, m_hdg_band_bottom_y_offset);
//     glVertex2d(+m_hdg_band_radius, m_hdg_band_bottom_y_offset);
//     glVertex2d(+m_hdg_band_radius, m_hdg_band_bottom_y_offset - m_hdg_band_radius);
//     glVertex2d(-m_hdg_band_radius, m_hdg_band_bottom_y_offset - m_hdg_band_radius);
//     glVertex2d(+m_hdg_band_radius, m_hdg_band_bottom_y_offset);
//     glEnd();

    // draw hdg marker

    glBegin(GL_LINE_LOOP);
    glVertex2d(0, +m_hdg_band_vert_offset);
    glVertex2d(-m_hdg_tick_width, +m_hdg_band_vert_offset-m_hdg_tick_width*2);
    glVertex2d(+m_hdg_tick_width, +m_hdg_band_vert_offset-m_hdg_tick_width*2);
    glEnd();    

    // draw compass rose

    glPushMatrix();
    glTranslated(0, m_hdg_band_vert_offset+m_hdg_band_radius, 0);

    glPushMatrix();
    glRotated(-mag_hdg, 0, 0, 1);
    glCallList(m_compass_gllist);
    glPopMatrix();

    // draw track line
    
    glLineWidth(1.5);
    double wca = -m_flightstatus->wind_correction_angle_deg;

    glPushMatrix();

    glRotated(wca, 0, 0, 1);
    glBegin(GL_LINES);
    glVertex2d(0, -m_hdg_band_radius);
    glVertex2d(0, 0);
    glVertex2d(-m_hdg_tick_width*0.5, -m_hdg_band_radius * 0.75);
    glVertex2d(+m_hdg_tick_width*0.5, -m_hdg_band_radius * 0.75);
    glEnd();

    glPopMatrix();

    // draw AP selected hdg marker

    double ap_hdg_marker_angle = -Navcalc::getSignedHeadingDiff(m_flightstatus->APHdg(), mag_hdg);
    double unit = m_hdg_tick_width * 1.2;
    
    glRotated(ap_hdg_marker_angle, 0 , 0, 1);
    glLineWidth(1.5);
    qglColor(Qt::magenta);
    glBegin(GL_LINE_LOOP);
    glVertex2d(-1*unit, -m_hdg_band_radius);
    glVertex2d(-1*unit, -m_hdg_band_radius-unit);
    glVertex2d(-0.5*unit, -m_hdg_band_radius-unit);
    glVertex2d(0, -m_hdg_band_radius);
    glVertex2d(0.5*unit, -m_hdg_band_radius-unit);
    glVertex2d(1*unit,   -m_hdg_band_radius-unit);
    glVertex2d(1*unit, -m_hdg_band_radius);
    glEnd();

    glPopMatrix();


    // draw selected hdg text

    qglColor(Qt::green);
    drawText(getTextWidth("1"), m_hdg_band_bottom_y_offset, "MAG");
    qglColor(Qt::magenta);
    QString hdg_string = QString("%1M").arg((int)m_flightstatus->APHdg(), 3, 10, QChar('0'));
    drawText(-getTextWidth("1")-getTextWidth(hdg_string), m_hdg_band_bottom_y_offset, hdg_string);

    glPopMatrix();
}

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleB::drawInfoTexts()
{
    glLoadIdentity();
    glTranslated(m_horizon_offset.x(), 0, 0);

    qglColor(Qt::white);

    if ((m_left_side && m_fmc_control->showPFDILS(true) && !m_flightstatus->nav1.id().isEmpty() && m_flightstatus->nav1_has_loc) ||
        (!m_left_side && m_fmc_control->showPFDILS(false) && !m_flightstatus->nav2.id().isEmpty() && m_flightstatus->nav2_has_loc))
    {
        // draw ILS info

        double x_text_offset = -m_horizon_half_width;
        double y_text_offset = m_fma_y_offset+m_fma_box_height+m_font_height;

        drawText(x_text_offset, y_text_offset, (m_left_side ? m_flightstatus->nav1.id() : m_flightstatus->nav2.id()));

        y_text_offset += m_font_height;
        QString dist_text;

        if (m_left_side)
        {
            if (!m_flightstatus->nav1_distance_nm.isEmpty())
            {
                QString dist_string = QString("DME %1").arg(m_flightstatus->nav1_distance_nm);
                drawText(x_text_offset, y_text_offset, dist_string);
            }
            else
            {
                drawText(x_text_offset, y_text_offset, "DME - - -");
            }
        }
        else
        {
            if (!m_flightstatus->nav2_distance_nm.isEmpty())
            {
                QString dist_string = QString("DME %1").arg(m_flightstatus->nav2_distance_nm);
                drawText(x_text_offset, y_text_offset, dist_string);
            }
            else
            {
                drawText(x_text_offset, y_text_offset, "DME - - -");
            }
        }
    }

    // draw MACH number

    if (m_flightstatus->mach >= 0.5)
    {
        QString mach_string = QString(".%1").arg((int)(m_flightstatus->mach * 100));
        drawText(m_speed_band_horiz_offset + getTextWidth("X"),
                 m_horizon_offset.y()+m_speed_band_half_height + m_font_height*1.5, mach_string);
    }

    // draw QNH

    qglColor(Qt::green);
    double qnh_x = m_alt_band_horiz_offset + getTextWidth("X");
    double qnh_y = m_horizon_offset.y()+m_alt_band_half_height+1.5*m_font_height;

    if (m_fmc_control->isAltimeterSetToSTD(m_left_side))
    {
        drawText(qnh_x, qnh_y, "STD");
    }
    else
    {
        if (m_fmc_control->showAltHpa(m_left_side))
            drawText(qnh_x, qnh_y, QString(" %1 HPA").arg(m_flightstatus->AltPressureSettingHpa(), 0, 'f', 0));
        else
            drawText(qnh_x, qnh_y, QString(" %1 ALT").arg(
                         Navcalc::getInchesFromHpa(m_flightstatus->AltPressureSettingHpa()), 0, 'f', 2));
    }

    // draw marker (outer, middle)

    if (m_flightstatus->radarAltitude() < 2500)
    {
//TODO
//         if (m_flightstatus->outer_marker)
//         {
//             qglColor(BLUE);
//             drawText(m_horizon_half_width - getTextWidth("OM"), 
//                      m_horizon_offset.y() + m_hdg_band_vert_offset - 3.0*m_font_height, "OM");
//         }
//         else if (m_flightstatus->middle_marker)
//         {
//             qglColor(AMBER);
//                 drawText(m_horizon_half_width - getTextWidth("MM"), 
//                          m_horizon_offset.y() + m_hdg_band_vert_offset - 3.0*m_font_height, "MM");
//         }
//         else if (m_flightstatus->inner_marker)
//         {
//             qglColor(WHITE);
//             drawText(m_horizon_half_width - getTextWidth("AWY"), 
//                      m_horizon_offset.y() + m_hdg_band_vert_offset - 3.0*m_font_height, "AWY");
//         }
    }

    // draw AP FMA

    QString fma_text;

    if (m_flightstatus->ap_enabled) fma_text = "A/P";
    else if (m_flightstatus->fd_active) fma_text = "FLT DIR";

    if (!fma_text.isEmpty())
    {
        qglColor(Qt::green);
        drawText(-getTextWidth(fma_text)/2.0,
                 m_horizon_offset.y() - m_horizon_half_height - m_font_height *0.2, fma_text);
    }

    // draw AP mode texts

    qglColor(DARKGRAY);
    glBegin(GL_QUADS);
    glVertex2d(-m_fma_box_half_width, m_fma_y_offset);
    glVertex2d(+m_fma_box_half_width, m_fma_y_offset);
    glVertex2d(+m_fma_box_half_width, m_fma_y_offset+m_fma_box_height);
    glVertex2d(-m_fma_box_half_width, m_fma_y_offset+m_fma_box_height);
    glEnd();

    qglColor(Qt::white);
    glLineWidth(1.5);

    double line1_x = -m_fma_box_half_width;
    double line2_x = -m_fma_box_item_half_width;
    double line3_x = +m_fma_box_item_half_width;
    double line4_x = +m_fma_box_half_width;

    glBegin(GL_LINES);
    glVertex2d(line2_x, m_fma_y_offset);
    glVertex2d(line2_x, m_fma_y_offset+m_fma_box_height);
    glVertex2d(line3_x, m_fma_y_offset);
    glVertex2d(line3_x, m_fma_y_offset+m_fma_box_height);
    glEnd();

    double item_line_y_offset = 5;
    double fma_box_upper_y = item_line_y_offset+1;
    double fma_box_lower_y = item_line_y_offset+m_font_height-1;

    // draw speed FMA

    if (m_fmc_control->fmcAutothrottle().speedModeActive() != FMCAutothrottle::SPEED_MODE_NONE)
    {
        qglColor(Qt::green);
        QString text = speedFMAText(m_fmc_control->fmcAutothrottle().speedModeActive());
        drawText(-m_fma_box_item_half_width*2.0-getTextWidth(text)*0.5, m_fma_y_offset+m_font_height, text);
    }
    
    glLineWidth(1.2);
    qglColor(Qt::green);        
    
    if (m_fmc_control->fmcAutothrottle().speedModeActiveChangeTimeMs() < 10000)
    {
        glBegin(GL_LINE_LOOP);
        glVertex2d(line1_x+FMA_QUAD_SPACING, fma_box_upper_y);
        glVertex2d(line2_x-FMA_QUAD_SPACING, fma_box_upper_y);
        glVertex2d(line2_x-FMA_QUAD_SPACING, fma_box_lower_y);
        glVertex2d(line1_x+FMA_QUAD_SPACING, fma_box_lower_y);
        glEnd();
    }
        
    // draw horizontal FMA

    if (m_fmc_control->fmcAutoPilot().lateralModeActive() != FMCAutopilot::LATERAL_MODE_NONE)
    {
        qglColor(Qt::green);
        QString upper_text = lateralFMAText(m_fmc_control->fmcAutoPilot().lateralModeActive());
        drawText(-getTextWidth(upper_text)*0.5, m_fma_y_offset+m_font_height, upper_text);
    }
    
    if (m_fmc_control->fmcAutoPilot().lateralModeArmed() != FMCAutopilot::LATERAL_MODE_NONE)
    {
        qglColor(Qt::white);
        QString lower_text = lateralFMAText(m_fmc_control->fmcAutoPilot().lateralModeArmed());
        drawText(-getTextWidth(lower_text)*0.5, m_fma_y_offset+2*m_font_height, lower_text);
    }
    
    glLineWidth(1.2);
    qglColor(Qt::green);        
    
    if (m_fmc_control->fmcAutoPilot().lateralModeActiveChangeTimeMs() < 10000)
    {
        glBegin(GL_LINE_LOOP);
        glVertex2d(line2_x+FMA_QUAD_SPACING, fma_box_upper_y);
        glVertex2d(line3_x-FMA_QUAD_SPACING, fma_box_upper_y);
        glVertex2d(line3_x-FMA_QUAD_SPACING, fma_box_lower_y);
        glVertex2d(line2_x+FMA_QUAD_SPACING, fma_box_lower_y);
        glEnd();
    }
    
    // draw vertival FMA

    if (m_fmc_control->fmcAutoPilot().verticalModeActive() != FMCAutopilot::VERTICAL_MODE_NONE)
    {
        qglColor(Qt::green);
        QString upper_text = verticalFMAText(m_fmc_control->fmcAutoPilot().verticalModeActive());
        drawText(2*m_fma_box_item_half_width-getTextWidth(upper_text)*0.5, m_fma_y_offset+m_font_height, upper_text);
    }
    
    if (m_fmc_control->fmcAutoPilot().verticalModeArmed() != FMCAutopilot::VERTICAL_MODE_NONE)
    {
        qglColor(Qt::white);
        QString lower_text = verticalFMAText(m_fmc_control->fmcAutoPilot().verticalModeArmed());
        drawText(2*m_fma_box_item_half_width-getTextWidth(lower_text)*0.5, m_fma_y_offset+2*m_font_height, lower_text);
    }
    
    glLineWidth(1.2);
    qglColor(Qt::green);        
    
    if (m_fmc_control->fmcAutoPilot().verticalModeActiveChangeTimeMs() < 10000)
    {
        glBegin(GL_LINE_LOOP);
        glVertex2d(line3_x+FMA_QUAD_SPACING, fma_box_upper_y);
        glVertex2d(line4_x-FMA_QUAD_SPACING, fma_box_upper_y);
        glVertex2d(line4_x-FMA_QUAD_SPACING, fma_box_lower_y);
        glVertex2d(line3_x+FMA_QUAD_SPACING, fma_box_lower_y);
        glEnd();
    }
}    

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleB::enableClipping(GLuint gllist_id)
{
    vasglBeginClipRegion(m_size);

    // draw clip poly for regions that should be displayed
    glCallList(gllist_id);

    vasglEndClipRegion();
}

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleB::disableClipping()
{
    vasglDisableClipping();
}

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleB::drawFPS()
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
    m_font->drawText(2.0, m_size.height()-m_font->getHeight(), QString("%1fps").arg(m_fps));
}

/////////////////////////////////////////////////////////////////////////////

QString GLPFDWidgetStyleB::speedFMAText(FMCAutothrottle::SPEED_MODE mode)
{
    switch(mode)
    {
        case(FMCAutothrottle::SPEED_MODE_NONE):   return QString::null;
        case(FMCAutothrottle::SPEED_MODE_SPEED):  return "SPD";
        case(FMCAutothrottle::SPEED_MODE_MACH):   return "SPD";
        case(FMCAutothrottle::SPEED_MODE_TOGA):   return "N1";
        case(FMCAutothrottle::SPEED_MODE_MCT):    return "N1";
        case(FMCAutothrottle::SPEED_MODE_FLEX):   return "N1";
        case(FMCAutothrottle::SPEED_MODE_THRCLB): return "N1";
        case(FMCAutothrottle::SPEED_MODE_THRLVR): return "N1"; //TODO check!?
        case(FMCAutothrottle::SPEED_MODE_THRIDLE):return "IDLE";
    }

    return QString::null;
}

/////////////////////////////////////////////////////////////////////////////

QString GLPFDWidgetStyleB::verticalFMAText(FMCAutopilot::VERTICAL_MODE mode)
{
    switch(mode)
    {
        case(FMCAutopilot::VERTICAL_MODE_NONE): return QString::null;
        case(FMCAutopilot::VERTICAL_MODE_ALT):  return "ALT";
        case(FMCAutopilot::VERTICAL_MODE_ALT_CAPTURE): return "ALT";
        case(FMCAutopilot::VERTICAL_MODE_ALTCRZ): return "ALT";
        case(FMCAutopilot::VERTICAL_MODE_GS): return "G/S";
        case(FMCAutopilot::VERTICAL_MODE_GS_CAPTURE): return "G/S";
        case(FMCAutopilot::VERTICAL_MODE_VS): return "V/S";
        case(FMCAutopilot::VERTICAL_MODE_FPV): return "FPA";
        case(FMCAutopilot::VERTICAL_MODE_FLCH): return "FLCH SPD";
        case(FMCAutopilot::VERTICAL_MODE_VNAV): return "VNAV SPD";
        case(FMCAutopilot::VERTICAL_MODE_TAKEOFF): return "TO/GA";
        case(FMCAutopilot::VERTICAL_MODE_LANDING): return "LAND";
        case(FMCAutopilot::VERTICAL_MODE_FLARE): return "LAND";
        case(FMCAutopilot::VERTICAL_MODE_ROLLOUT): return "LAND";
    }

    return QString::null;
}

/////////////////////////////////////////////////////////////////////////////

QString GLPFDWidgetStyleB::lateralFMAText(FMCAutopilot::LATERAL_MODE mode)
{
    switch(mode)
    {
        case(FMCAutopilot::LATERAL_MODE_NONE): return QString::null;
        case(FMCAutopilot::LATERAL_MODE_HDG):  return "HDG SEL";
        case(FMCAutopilot::LATERAL_MODE_LNAV):  return "LNAV";
        case(FMCAutopilot::LATERAL_MODE_LOC):  return "LOC";
        case(FMCAutopilot::LATERAL_MODE_LOC_CAPTURE):  return "LOC";
        case(FMCAutopilot::LATERAL_MODE_VOR):  return "VOR";
        case(FMCAutopilot::LATERAL_MODE_TAKEOFF): return "TO/GA";
        case(FMCAutopilot::LATERAL_MODE_LANDING): return "LAND";
        case(FMCAutopilot::LATERAL_MODE_FLARE): return "LAND";
        case(FMCAutopilot::LATERAL_MODE_ROLLOUT): return "LAND";
    }

    return QString::null;
}

