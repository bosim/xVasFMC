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
#include "flightstatus.h"
#include "aircraft_data.h"
#include "vas_path.h"
#include "statistics.h"
#include "flight_mode_tracker.h"

#include "gldraw.h"
#include "fmc_control.h"
#include "fmc_pfd.h"

#include "fly_by_wire.h"

#include "fmc_flightstatus_checker_base.h"

#include "fmc_pfd_glwidget_style_a.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

//#define BLUE "#0098B7"
#define BLUE "#19A5A6"
//#define BROWN "#544D17"
#define BROWN "#945B01"
#define DARKGRAY "#4A4A4A"

#define MIN_SPEED 30.0
#define FMA_QUAD_SPACING 4
#define VS_SCALE_MAX 4000.0

#define STATISTICS 0

#define MAX_LOC_NEEDLE_DEV 125
#define MAX_GS_NEEDLE_DEV 115

/////////////////////////////////////////////////////////////////////////////

GLPFDWidgetStyleA::GLPFDWidgetStyleA(ConfigWidgetProvider* config_widget_provider,
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
    m_fbw_was_enabled(false),
    m_horizon_clip_gllist(0),
    m_horizon_gllist(0),
    m_horizon_static_marker_gllist(0),
    m_horizon_bank_marker_gllist(0),
    m_pitch(0), m_bank(0), m_mag_hdg(0), m_vs(0), m_alt_readout(0), m_is_below_mda(false), 
    m_show_altimeter_setting(true), m_show_ils_warning(false), m_show_thr_lvr_clb(false), m_stat(0)
{
    MYASSERT(m_config_widget_provider != 0);
    MYASSERT(m_main_config != 0);
    MYASSERT(m_pfd_config != 0);
    MYASSERT(m_fmc_control != 0);
    MYASSERT(m_flightstatus != 0);

	m_fps_counter_time.start();
    m_radar_alt_blink_timer = QTime::currentTime().addSecs(-60);
    m_ils_warning_blink_timer = QTime::currentTime().addSecs(-60);
    m_altitude_window_blink_timer = QTime::currentTime().addSecs(-60);
    m_thr_lvr_clb_blink_timer = QTime::currentTime().addSecs(-60);

    MYASSERT(connect(&m_altimeter_blink_timer, SIGNAL(timeout()), this, SLOT(slotAltimeterBlinkTimer())));

    MYASSERT(connect(m_fmc_control->bankController(), SIGNAL(signalParametersChanged()),
                     this, SLOT(slotFBWParametersChanged())));
    MYASSERT(connect(m_fmc_control->pitchController(), SIGNAL(signalParametersChanged()),
                     this, SLOT(slotFBWParametersChanged())));

#if STATISTICS    
    m_stat = new Statistics("pfd.csv", " ");
    MYASSERT(m_stat != 0);
    m_stat_timer.start();
#endif
}

/////////////////////////////////////////////////////////////////////////////

GLPFDWidgetStyleA::~GLPFDWidgetStyleA() 
{
    if (m_horizon_clip_gllist != 0) glDeleteLists(m_horizon_clip_gllist, 1);
    if (m_horizon_gllist != 0) glDeleteLists(m_horizon_gllist, 1);
    if (m_horizon_static_marker_gllist != 0) glDeleteLists(m_horizon_static_marker_gllist, 1);
    if (m_horizon_bank_marker_gllist != 0) glDeleteLists(m_horizon_bank_marker_gllist, 1);
    delete m_stat;
}

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleA::initializeGL()
{
    // setup opengl

    qglClearColor(Qt::black);
    glShadeModel(GL_SMOOTH);
    //glShadeModel(GL_FLAT); 
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

    m_font->loadFont(VasPath::prependPath(m_main_config->getValue(CFG_FONT_NAME)), m_fmc_control->glFontSize());
    m_font_height = m_font->getHeight();
    m_font_height_half = m_font_height * 0.5;
    m_font_height_double = m_font_height * 2.0;
}

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleA::resizeGL(int width, int height)
{
    m_recalc_pfd = true;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, width, height, 0.0, 1.0, -1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleA::refreshPFD()
{
    if (!isVisible()) return;
    updateGL();
};

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleA::paintGL()
{
    if (!isVisible()) return;

    if (m_flightstatus->onground != m_onground || m_fbw_was_enabled != m_fmc_control->fbwEnabled())
    {
        m_onground = m_flightstatus->onground;
        m_recalc_pfd = true;
    }

    m_fbw_was_enabled = m_fmc_control->fbwEnabled();

    setupStateBeforeDraw();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_flightstatus->isValid() && !m_flightstatus->battery_on) return;

    if (m_onground)
    { 
        if (!m_flightstatus->isAtLeastOneEngineOn()) m_draw_joystick_input_when_engines_running = true;
    }
    else
    {
        m_draw_joystick_input_when_engines_running = false;
    }

    drawDisplay();
    if (m_fmc_control->showFps()) drawFPS();
    glFlush();

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) Logger::log(QString("GLPFDWidgetStyleA:paintGL: GL Error: %1").arg(error));
}

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleA::setupStateBeforeDraw()
{
    if (m_recalc_pfd)
    {
        m_recalc_pfd = false;

        m_horizon_offset = QPointF(size().width() * (2.9/6.6), size().height() * 0.5);

        m_horizon_size = QSizeF(size().width() * 3.3/6.6, size().height() * 3.8/6.6);

        m_horizon_half_height = m_horizon_size.height()/2.0;
        m_horizon_half_width = m_horizon_half_height * sin(Navcalc::toRad(67.0));

        m_horizon_even_pitch_line_width = (size().width() * 0.8 / 6.6) / 2.0;
        m_horizon_odd_pitch_line_width = m_horizon_even_pitch_line_width  / 2.0;
        m_horizon_limit_y_offset = -getVerticalOffsetByPitch(20.0);

        m_fd_width = (size().width() * 1.7 / 6.6) / 2.0;
        m_fd_height = (size().height() * 1.7 / 6.6) / 2.0;

        m_horizon_middle_marking_width = size().height() * 0.1 / (6.6 * 2.0);
        m_horizon_middle_marking_height = size().height() * 0.1 / (6.6 * 2.0);
        m_horizon_side_marking_width = size().width() * 0.7 / 6.6 ;
        m_horizon_side_marking_height = m_horizon_middle_marking_height;
        m_horizon_marking_border = 1.0;

        m_bank_marking_angle_inc = Navcalc::toRad(10.0);
        m_bank_marking_width = 3;
        m_bank_marking_height = size().height() * 0.2 / 6.6;
        
        m_alt_band_width = size().width() * 0.6 / 6.6;
        m_alt_band_vert_gap = m_font_height* 1.1;
        m_alt_band_horiz_offset = size().width() * 2.2 / 6.6;
        m_alt_last_two_digit_offset = getTextWidth("XX") * 1.1;
        m_alt_value_horiz_offset = m_alt_band_horiz_offset + m_alt_band_width + m_alt_last_two_digit_offset;
        m_alt_range = 1200;
        m_alt_pixel_per_ft = m_horizon_size.height() / (double)m_alt_range;
        m_alt_tick_width = size().width() * 0.1 / 6.6;
        m_alt_marker_offset = size().width() * 1.8 / 6.6;
        m_alt_marker_width = m_alt_band_horiz_offset - m_alt_marker_offset;

        m_rocd_band_width = size().width() * 0.35 / 6.6;
        m_rocd_band_horiz_offset = 
            (size().width() - m_horizon_offset.x()) - m_rocd_band_width - (size().width() * 0.1 / 6.6);
        m_rocd_root_offset = (m_horizon_size.height()/4.0 * m_rocd_band_width) / (m_horizon_size.height()/4.0);
        m_rocd_mark_500ft_y = 500.0 * m_horizon_half_height / VS_SCALE_MAX;

        m_ils_circle_size = size().width() * 0.1 / 6.6 * 0.5;
        m_ils_circle_offset = size().width() * 0.7 / 6.6 * 0.5;
        m_gs_bug_width = size().width() * 0.3 / 6.6 * 0.5;
        m_gs_bug_height = size().height() * 0.4 / 6.6 * 0.5;

        m_speed_band_width = size().width() * 0.7 / 6.6;
        m_speed_band_horiz_offset = - size().width() * 2.8 / 6.6;
        m_speed_range = 80;
        m_spd_pixel_per_knot = m_horizon_size.height() / (double)m_speed_range;
        m_speed_repaint_y_offset = m_horizon_half_height;
        m_speed_repaint_height = m_spd_pixel_per_knot*10 + 2*m_font_height;
        m_speed_marker_width = size().width() * 0.5 / 6.6;
        m_speed_marker_offset = m_speed_band_horiz_offset + m_speed_band_width;
        m_ap_speed_marker_width = size().width() * 0.29 / 6.6;

        m_hdg_band_height = size().height() * 0.5 / 6.6;
        m_hdg_band_vert_offset = size().height() * 2.7 / 6.6;
        m_hdg_range = 50;
        m_hdg_pixel_per_deg = m_horizon_size.width() / (double)m_hdg_range;
        m_hdg_marker_height = size().height() * 0.4 / 6.6;
        m_hdg_marker_offset = m_hdg_band_vert_offset - m_hdg_marker_height*0.75;
        m_hdg_repaint_x_offset = m_horizon_half_width;
        m_hdg_repaint_width = m_hdg_pixel_per_deg*20;
        m_ap_hdg_marker_height = size().height() * 0.3 / 6.6;

        //-----

        createHorizonClipping();
        createHorizon();
        createHorizonStaticMarkings();
        createHorizonBankMarkings();
    }
};

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleA::createHorizonClipping()
{
    if (m_horizon_clip_gllist != 0) glDeleteLists(m_horizon_clip_gllist, 1);
    m_horizon_clip_gllist = glGenLists(1);
    MYASSERT(m_horizon_clip_gllist != 0);
    glNewList(m_horizon_clip_gllist, GL_COMPILE);
    glLoadIdentity();
    glTranslated(m_horizon_offset.x(), m_horizon_offset.y(), +0.5);
    
    m_horizon_radius = sqrt(m_horizon_half_width * m_horizon_half_width + 
                            m_horizon_size.height()/4.0 * m_horizon_size.height()/4.0);
    
    m_horizon_angle = asin((m_horizon_half_width) / m_horizon_radius);
    
    GLDraw::drawFilledCircle(m_horizon_radius, -m_horizon_angle, +m_horizon_angle);
    GLDraw::drawFilledCircle(m_horizon_radius, M_PI-m_horizon_angle, M_PI+m_horizon_angle);
    
    glBegin(GL_QUADS);
    glVertex2d(-m_horizon_half_width, -m_horizon_size.height()/4.0);
    glVertex2d(+m_horizon_half_width, -m_horizon_size.height()/4.0);
    glVertex2d(+m_horizon_half_width, +m_horizon_size.height()/4.0);
    glVertex2d(-m_horizon_half_width, +m_horizon_size.height()/4.0);
    glEnd();
    
    glEndList();        
}

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleA::createHorizon()
{
    if (m_horizon_gllist != 0) glDeleteLists(m_horizon_gllist, 1);
    m_horizon_gllist = glGenLists(1);
    MYASSERT(m_horizon_gllist != 0);
    glNewList(m_horizon_gllist, GL_COMPILE);

    // heaven & earth

    glBegin(GL_QUADS);
    qglColor(BLUE);
    glVertex2d(-size().width(), -size().height());
    glVertex2d(+size().width(), -size().height());
    glVertex2d(+size().width(), 0);
    glVertex2d(-size().width(), 0);
    qglColor(BROWN);
    glVertex2d(-size().width(), 0);
    glVertex2d(+size().width(), 0);
    glVertex2d(+size().width(), +size().height());
    glVertex2d(-size().width(), +size().height());
    glEnd();

    // pitch ladder

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
            if (!m_flightstatus->onground)
            {
                glVertex2d(-width, +offset);
                glVertex2d(+width, +offset);
            }
            glEnd();
        }
        else
        {
            glBegin(GL_LINES);
            glVertex2d(-m_horizon_even_pitch_line_width, -offset);
            glVertex2d(+m_horizon_even_pitch_line_width, -offset);
            if (!m_flightstatus->onground)
            {
                glVertex2d(-m_horizon_even_pitch_line_width, +offset);
                glVertex2d(+m_horizon_even_pitch_line_width, +offset);
            }
            glEnd();

            QString pitch_text = QString::number(2.5 * index);
            drawText(-m_horizon_even_pitch_line_width-10-text_offset, -offset+m_font_height_half, pitch_text);
            drawText(+m_horizon_even_pitch_line_width+7,              -offset+m_font_height_half, pitch_text);
            if (!m_flightstatus->onground)
            {
                drawText(-m_horizon_even_pitch_line_width-10-text_offset, +offset+m_font_height_half, pitch_text);
                drawText(+m_horizon_even_pitch_line_width+7,              +offset+m_font_height_half, pitch_text);
            }
        }

        offset += offset_inc;
    }

    // horizon line
    glLineWidth(1.8);
    glBegin(GL_LINES);
    glVertex2d(-m_horizon_size.width(), 0);
    glVertex2d(+m_horizon_size.width(), 0);
    glEnd();

    glLineWidth(1.5);

    // FBW pitch limits

    double fbw_width = m_horizon_odd_pitch_line_width * 0.5;
    double y = getVerticalOffsetByPitch(-m_fmc_control->pitchController()->maxNegativePitch());
    double y_diff = getVerticalOffsetByPitch(0.5);

    if (m_fmc_control->fbwEnabled())
    {
        qglColor(GREEN);
      
        glBegin(GL_LINES);

        if (!m_flightstatus->onground)
        {
            glVertex2d(-m_horizon_even_pitch_line_width, -y-y_diff);
            glVertex2d(-m_horizon_even_pitch_line_width+fbw_width, -y-y_diff);
            glVertex2d(-m_horizon_even_pitch_line_width, -y+y_diff);
            glVertex2d(-m_horizon_even_pitch_line_width+fbw_width, -y+y_diff);
            glVertex2d(+m_horizon_even_pitch_line_width, -y-y_diff);
            glVertex2d(+m_horizon_even_pitch_line_width-fbw_width, -y-y_diff);
            glVertex2d(+m_horizon_even_pitch_line_width, -y+y_diff);
            glVertex2d(+m_horizon_even_pitch_line_width-fbw_width, -y+y_diff);
        }

        y = getVerticalOffsetByPitch(-m_fmc_control->pitchController()->maxPositivePitch());
        glVertex2d(-m_horizon_even_pitch_line_width, -y-y_diff);
        glVertex2d(-m_horizon_even_pitch_line_width+fbw_width, -y-y_diff);
        glVertex2d(-m_horizon_even_pitch_line_width, -y+y_diff);
        glVertex2d(-m_horizon_even_pitch_line_width+fbw_width, -y+y_diff);
        glVertex2d(+m_horizon_even_pitch_line_width, -y-y_diff);
        glVertex2d(+m_horizon_even_pitch_line_width-fbw_width, -y-y_diff);
        glVertex2d(+m_horizon_even_pitch_line_width, -y+y_diff);
        glVertex2d(+m_horizon_even_pitch_line_width-fbw_width, -y+y_diff);
        glEnd();
    }
    else
    {
        qglColor(MAGENTA);

        y_diff *= 2.0;

        glBegin(GL_LINES);

        if (!m_flightstatus->onground)
        {
            glVertex2d(-m_horizon_even_pitch_line_width-y_diff, -y-y_diff);
            glVertex2d(-m_horizon_even_pitch_line_width+y_diff, -y+y_diff);
            glVertex2d(-m_horizon_even_pitch_line_width+y_diff, -y-y_diff);
            glVertex2d(-m_horizon_even_pitch_line_width-y_diff, -y+y_diff);

            glVertex2d(+m_horizon_even_pitch_line_width-y_diff, -y-y_diff);
            glVertex2d(+m_horizon_even_pitch_line_width+y_diff, -y+y_diff);
            glVertex2d(+m_horizon_even_pitch_line_width+y_diff, -y-y_diff);
            glVertex2d(+m_horizon_even_pitch_line_width-y_diff, -y+y_diff);
        }

        y = getVerticalOffsetByPitch(-m_fmc_control->pitchController()->maxPositivePitch());
        glVertex2d(-m_horizon_even_pitch_line_width-y_diff, -y-y_diff);
        glVertex2d(-m_horizon_even_pitch_line_width+y_diff, -y+y_diff);
        glVertex2d(-m_horizon_even_pitch_line_width+y_diff, -y-y_diff);
        glVertex2d(-m_horizon_even_pitch_line_width-y_diff, -y+y_diff);

        glVertex2d(+m_horizon_even_pitch_line_width-y_diff, -y-y_diff);
        glVertex2d(+m_horizon_even_pitch_line_width+y_diff, -y+y_diff);
        glVertex2d(+m_horizon_even_pitch_line_width+y_diff, -y-y_diff);
        glVertex2d(+m_horizon_even_pitch_line_width-y_diff, -y+y_diff);
        glEnd();
    }

    glEndList();
}

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleA::createHorizonStaticMarkings()
{
    if (m_horizon_static_marker_gllist != 0) glDeleteLists(m_horizon_static_marker_gllist, 1);
    m_horizon_static_marker_gllist = glGenLists(1);
    MYASSERT(m_horizon_static_marker_gllist != 0);
    glNewList(m_horizon_static_marker_gllist, GL_COMPILE);

    glLineWidth(1.5);
    glBegin(GL_QUADS);

    qglColor(Qt::yellow);
    //middle
    glVertex2d(-m_horizon_middle_marking_width, -m_horizon_middle_marking_height);
    glVertex2d(+m_horizon_middle_marking_width, -m_horizon_middle_marking_height);
    glVertex2d(+m_horizon_middle_marking_width, +m_horizon_middle_marking_height);
    glVertex2d(-m_horizon_middle_marking_width, +m_horizon_middle_marking_height);
    //left horizontal
    glVertex2d(-m_horizon_half_width, -m_horizon_side_marking_height);
    glVertex2d(-m_horizon_half_width+m_horizon_side_marking_width, -m_horizon_side_marking_height);
    glVertex2d(-m_horizon_half_width+m_horizon_side_marking_width, +m_horizon_side_marking_height);
    glVertex2d(-m_horizon_half_width, +m_horizon_side_marking_height);
    //left vertical
    glVertex2d(-m_horizon_half_width+m_horizon_side_marking_width, -m_horizon_side_marking_height);
    glVertex2d(-m_horizon_half_width+m_horizon_side_marking_width, +m_horizon_side_marking_height*5);
    glVertex2d(-m_horizon_half_width+m_horizon_side_marking_width-m_horizon_side_marking_height*2, +m_horizon_side_marking_height*5);
    glVertex2d(-m_horizon_half_width+m_horizon_side_marking_width-m_horizon_side_marking_height*2, -m_horizon_side_marking_height);
    //right horizontal
    glVertex2d(+m_horizon_half_width, -m_horizon_side_marking_height);
    glVertex2d(+m_horizon_half_width-m_horizon_side_marking_width, -m_horizon_side_marking_height);
    glVertex2d(+m_horizon_half_width-m_horizon_side_marking_width, +m_horizon_side_marking_height);
    glVertex2d(+m_horizon_half_width, +m_horizon_side_marking_height);
    //right vertical
    glVertex2d(+m_horizon_half_width-m_horizon_side_marking_width, -m_horizon_side_marking_height);
    glVertex2d(+m_horizon_half_width-m_horizon_side_marking_width, +m_horizon_side_marking_height*5);
    glVertex2d(+m_horizon_half_width-m_horizon_side_marking_width+m_horizon_side_marking_height*2, +m_horizon_side_marking_height*5);
    glVertex2d(+m_horizon_half_width-m_horizon_side_marking_width+m_horizon_side_marking_height*2, -m_horizon_side_marking_height);

    qglColor(Qt::black);
    // middle
    glVertex2d(-m_horizon_middle_marking_width+m_horizon_marking_border, -m_horizon_middle_marking_height+m_horizon_marking_border);
    glVertex2d(+m_horizon_middle_marking_width-m_horizon_marking_border, -m_horizon_middle_marking_height+m_horizon_marking_border);
    glVertex2d(+m_horizon_middle_marking_width-m_horizon_marking_border, +m_horizon_middle_marking_height-m_horizon_marking_border);
    glVertex2d(-m_horizon_middle_marking_width+m_horizon_marking_border, +m_horizon_middle_marking_height-m_horizon_marking_border);
    //left horizontal
    glVertex2d(-m_horizon_half_width+m_horizon_marking_border, -m_horizon_side_marking_height+m_horizon_marking_border);
    glVertex2d(-m_horizon_half_width+m_horizon_side_marking_width-m_horizon_marking_border, -m_horizon_side_marking_height+m_horizon_marking_border);
    glVertex2d(-m_horizon_half_width+m_horizon_side_marking_width-m_horizon_marking_border, +m_horizon_side_marking_height-m_horizon_marking_border);
    glVertex2d(-m_horizon_half_width+m_horizon_marking_border, +m_horizon_side_marking_height-m_horizon_marking_border);
    //left vertical
    glVertex2d(-m_horizon_half_width+m_horizon_side_marking_width-m_horizon_marking_border, -m_horizon_side_marking_height+m_horizon_marking_border);
    glVertex2d(-m_horizon_half_width+m_horizon_side_marking_width-m_horizon_marking_border, +m_horizon_side_marking_height*5-m_horizon_marking_border);
    glVertex2d(-m_horizon_half_width+m_horizon_side_marking_width-m_horizon_side_marking_height*2+m_horizon_marking_border, +m_horizon_side_marking_height*5-m_horizon_marking_border);
    glVertex2d(-m_horizon_half_width+m_horizon_side_marking_width-m_horizon_side_marking_height*2+m_horizon_marking_border, -m_horizon_side_marking_height+m_horizon_marking_border);
    //right horizontal
    glVertex2d(+m_horizon_half_width-m_horizon_marking_border, -m_horizon_side_marking_height+m_horizon_marking_border);
    glVertex2d(+m_horizon_half_width-m_horizon_side_marking_width+m_horizon_marking_border, -m_horizon_side_marking_height+m_horizon_marking_border);
    glVertex2d(+m_horizon_half_width-m_horizon_side_marking_width+m_horizon_marking_border, +m_horizon_side_marking_height-m_horizon_marking_border);
    glVertex2d(+m_horizon_half_width-m_horizon_marking_border, +m_horizon_side_marking_height-m_horizon_marking_border);
    //right vertical
    glVertex2d(+m_horizon_half_width-m_horizon_side_marking_width+m_horizon_marking_border, -m_horizon_side_marking_height+m_horizon_marking_border);
    glVertex2d(+m_horizon_half_width-m_horizon_side_marking_width+m_horizon_marking_border, +m_horizon_side_marking_height*5-m_horizon_marking_border);
    glVertex2d(+m_horizon_half_width-m_horizon_side_marking_width+m_horizon_side_marking_height*2-m_horizon_marking_border, +m_horizon_side_marking_height*5-m_horizon_marking_border);
    glVertex2d(+m_horizon_half_width-m_horizon_side_marking_width+m_horizon_side_marking_height*2-m_horizon_marking_border, -m_horizon_side_marking_height+m_horizon_marking_border);
    glEnd();

    glEndList();
}

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleA::createHorizonBankMarkings()
{
    if (m_horizon_bank_marker_gllist != 0) glDeleteLists(m_horizon_bank_marker_gllist, 1);
    m_horizon_bank_marker_gllist = glGenLists(1);
    MYASSERT(m_horizon_bank_marker_gllist != 0);
    glNewList(m_horizon_bank_marker_gllist, GL_COMPILE);

    qglColor(Qt::white);
    glLineWidth(1.8);
    GLDraw::drawCircle(m_horizon_radius, -3*m_bank_marking_angle_inc, +3*m_bank_marking_angle_inc);

    glLineWidth(2.0);

    // 45 deg left

    glPushMatrix();
    glRotated(-45, 0, 0, 1);
    glBegin(GL_LINES);
    glVertex2d(0, -m_horizon_radius);
    glVertex2d(0, -m_horizon_radius-m_bank_marking_height/2.0);
    glEnd();
    glPopMatrix();

    // 45 deg right

    glPushMatrix();
    glRotated(+45, 0, 0, 1);
    glBegin(GL_LINES);
    glVertex2d(0, -m_horizon_radius);
    glVertex2d(0, -m_horizon_radius-m_bank_marking_height/2.0);
    glEnd();
    glPopMatrix();

    glLineWidth(1.5);

    // bank protection

    double fbw_mark_size = m_bank_marking_height / 2.0;

    if (m_fmc_control->fbwEnabled())
    {
        qglColor(GREEN);
    }
    else
    { 
        qglColor(MAGENTA);
        fbw_mark_size *= 1.5;
    }

    double fbw_mark_size_half = fbw_mark_size * 0.5;
              
    glPushMatrix();
    glRotated(-m_fmc_control->bankController()->maxForcedBank(), 0, 0, 1.0);
    glBegin(GL_LINES);

    if (m_fmc_control->fbwEnabled())
    {
        glVertex2d(-m_bank_marking_width, -m_horizon_radius);
        glVertex2d(-m_bank_marking_width, -m_horizon_radius-fbw_mark_size);
        glVertex2d(+m_bank_marking_width, -m_horizon_radius);
        glVertex2d(+m_bank_marking_width, -m_horizon_radius-fbw_mark_size);
    }
    else
    {
        glVertex2d(-fbw_mark_size_half, -m_horizon_radius-fbw_mark_size);
        glVertex2d(+fbw_mark_size_half, -m_horizon_radius);
        glVertex2d(+fbw_mark_size_half, -m_horizon_radius-fbw_mark_size);
        glVertex2d(-fbw_mark_size_half, -m_horizon_radius);
    }

    glEnd();
    glPopMatrix();
    
    glPushMatrix();
    glRotated(+m_fmc_control->bankController()->maxForcedBank(), 0, 0, 1.0);
    glBegin(GL_LINES);

    if (m_fmc_control->fbwEnabled())
    {
        glVertex2d(-m_bank_marking_width, -m_horizon_radius);
        glVertex2d(-m_bank_marking_width, -m_horizon_radius-fbw_mark_size);
        glVertex2d(+m_bank_marking_width, -m_horizon_radius);
        glVertex2d(+m_bank_marking_width, -m_horizon_radius-fbw_mark_size);
    }
    else
    {
        glVertex2d(-fbw_mark_size_half, -m_horizon_radius-fbw_mark_size);
        glVertex2d(+fbw_mark_size_half, -m_horizon_radius);
        glVertex2d(+fbw_mark_size_half, -m_horizon_radius-fbw_mark_size);
        glVertex2d(-fbw_mark_size_half, -m_horizon_radius);
        
    }
     
    glEnd();
    glPopMatrix();
    
    qglColor(Qt::white);

    // 10, 20, 30 deg left/right

    glPushMatrix();
    glRotated(-40, 0, 0, 1);

    for(int index = 0; index < 7; ++index)
    {
        glRotated(+10, 0, 0, 1);

        switch(index)
        {
            case(0):
            case(6):
                qglColor(Qt::white);
                glBegin(GL_LINE_LOOP);
                glVertex2d(-m_bank_marking_width, -m_horizon_radius);
                glVertex2d(-m_bank_marking_width, -m_horizon_radius-m_bank_marking_height);
                glVertex2d(+m_bank_marking_width, -m_horizon_radius-m_bank_marking_height);
                glVertex2d(+m_bank_marking_width, -m_horizon_radius);
                glEnd();
                break;
            case(1):
            case(2):
            case(4):
            case(5):
                qglColor(Qt::white);
                glBegin(GL_LINE_LOOP);
                glVertex2d(-m_bank_marking_width, -m_horizon_radius);
                glVertex2d(-m_bank_marking_width, -m_horizon_radius-m_bank_marking_height/2.0);
                glVertex2d(+m_bank_marking_width, -m_horizon_radius-m_bank_marking_height/2.0);
                glVertex2d(+m_bank_marking_width, -m_horizon_radius);
                glEnd();
                break;    
            case(3):
                qglColor(Qt::yellow);
                glBegin(GL_LINE_LOOP);
                glVertex2d(0, -m_horizon_radius);
                glVertex2d(-m_bank_marking_height*0.5, -m_horizon_radius-m_bank_marking_height*0.8);
                glVertex2d(+m_bank_marking_height*0.5, -m_horizon_radius-m_bank_marking_height*0.8);
                glEnd();
                break;
        }
    }

    glPopMatrix();

    glEndList();
}

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleA::drawDisplay()
{
    if (m_flightstatus->isValid() && !m_flightstatus->avionics_on)
    {
        qglColor(RED);
        drawText(size().width()/2.0, size().height()/2.0, "ATT");
        return;
    }

    //-----

    m_pitch = m_flightstatus->smoothedPitch();
    m_bank = m_flightstatus->smoothedBank();
    m_mag_hdg = m_flightstatus->smoothedMagneticHeading();
    m_vs = m_flightstatus->smoothedVS();
    m_alt_readout = m_flightstatus->smoothedAltimeterReadout();

    m_is_below_mda = m_fmc_data.minDescentAltitudeFt() > 0 && m_alt_readout < m_fmc_data.minDescentAltitudeFt();

    //-----    

    glLoadIdentity();
    glTranslated(m_horizon_offset.x(), m_horizon_offset.y(), 0);

    drawHorizon();
    drawAltitude();
    drawSpeed();
    drawHeading();
    drawInfoTexts();
};

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleA::drawHorizon()
{
    double fd_pitch = m_flightstatus->smoothedFlightDirectorPitch();
    double fd_bank = m_flightstatus->smoothedFlightDirectorBank();

    // gnuplot command
    /*
      plot 'C:\devel\vas\vasfmc\pfd.csv' using 1:2 with lines, 'C:\devel\vas\vasfmc\pfd.csv' using 1:3 with lines
    */
    if (m_stat != 0)
    {
        m_stat->putItemInLine(QString::number(m_stat_timer.elapsed()/1000.0));
        m_stat->putItemInLine(QString::number(fd_pitch));
        m_stat->putItemInLine(QString::number(fd_bank));
        m_stat->nextLine();
        //Logger::log(QString("pitch=%1 bank=%2").arg(fd_pitch).arg(fd_bank));
    }

    glPushMatrix();

    enableClipping(m_horizon_clip_gllist);

    glPushMatrix();

    glRotated(m_bank, 0, 0, 1.0);
    glPushMatrix();

    // draw sky, earth and pitch lines

    double pitch_y_offset = getVerticalOffsetByPitch(m_pitch);

    glTranslated(0, pitch_y_offset, 0);
    glCallList(m_horizon_gllist);

    // erase earth when < 570ft accordingly

    bool draw_lower_limit = true;
    
    if (m_flightstatus->radarAltitude() <= 570)
    {
        qglColor(BROWN);
        draw_lower_limit = false;
    
        double clear_y = (m_flightstatus->radarAltitude() / 570.0) * 
                         (getVerticalOffsetByPitch(-20.0) - pitch_y_offset);
        
        glBegin(GL_QUADS);
        qglColor(BROWN);
        glVertex2d(-m_horizon_half_width, clear_y);
        glVertex2d(+m_horizon_half_width, clear_y);
        glVertex2d(+m_horizon_half_width, +size().height());
        glVertex2d(-m_horizon_half_width, +size().height());
        glEnd();

        qglColor(WHITE);
        glLineWidth(1.8);

        glBegin(GL_LINES);
        glVertex2d(-m_horizon_half_width, clear_y);
        glVertex2d(+m_horizon_half_width, clear_y);
        glEnd();
    }

    // draw hdg ticks on horizon line

    qglColor(WHITE);
    glLineWidth(2.0);

    glBegin(GL_LINES);
    
    double hdg_fraction_x_offset = ( ((int)(m_mag_hdg * 100)) % 1000) / 100.0 * m_hdg_pixel_per_deg;
    double hdg_tape_x_offset = -hdg_fraction_x_offset - m_hdg_pixel_per_deg*10;
    
    int start_hdg = ((int)(m_mag_hdg) / 10 * 10) - (m_hdg_range / 2 / 10 * 10);
    int max_hdg = start_hdg + m_hdg_range + 10;
    for(int hdg = start_hdg; hdg < max_hdg; hdg += 10)
    {
        double x = (hdg - start_hdg - 10) * m_hdg_pixel_per_deg + hdg_tape_x_offset;
        glVertex2d(x, 0);
        glVertex2d(x, +2.0*m_horizon_side_marking_height);
    }
    
    glEnd();        

    glPopMatrix();

    // upper and lower limit + lines
    
    glBegin(GL_QUADS);
    qglColor(BLUE);
    glVertex2d(+size().width(), -m_horizon_limit_y_offset);
    glVertex2d(-size().width(), -m_horizon_limit_y_offset);
    glVertex2d(-size().width(), -size().height());
    glVertex2d(+size().width(), -size().height());
    if (draw_lower_limit)
    {
        qglColor(BROWN);
        glVertex2d(-size().width(), m_horizon_limit_y_offset);
        glVertex2d(+size().width(), m_horizon_limit_y_offset);
        glVertex2d(+size().width(), +size().height());
        glVertex2d(-size().width(), +size().height());
    }
    glEnd();

    qglColor(Qt::white);
    glLineWidth(1.8);
    glBegin(GL_LINES);
    glVertex2d(+size().width(), -m_horizon_limit_y_offset);
    glVertex2d(-size().width(), -m_horizon_limit_y_offset);
    if (draw_lower_limit)
    {
        glVertex2d(-size().width(), m_horizon_limit_y_offset);
        glVertex2d(+size().width(), m_horizon_limit_y_offset);
    }
    glEnd();

    // draw height above ground

    int height = (int)m_flightstatus->radarAltitude();

    
    if (height <= 2500)
    {
        if (height >= 50) height = height / 10 * 10;
        else if (height >= 10) height = height / 5 * 5;

        if (m_fmc_data.decisionHeightFt() > 0)
        {
            (height < (int)m_fmc_data.decisionHeightFt() + 100) ? qglColor(AMBER) : qglColor(Qt::green);

            if (m_vs < 0 && 
                qAbs((int)height - (int)m_fmc_data.decisionHeightFt()) < 10 && 
                m_radar_alt_blink_timer.elapsed() >= 3000) m_radar_alt_blink_timer.start();
        }
        else
        {
            (height <= 400) ? qglColor(AMBER) : qglColor(Qt::green);
        }

        if (m_radar_alt_blink_timer.elapsed() < 3000 && (m_radar_alt_blink_timer.elapsed() / 250) % 2 == 0)
        {
            // let RA blink
        }
        else
        {
            QString height_text = QString::number(height);
            drawText(-getTextWidth(height_text)/2.0, m_horizon_limit_y_offset + m_font_height, height_text);
        }
    }
    
    // draw bank marking triangle and slip indicator

    glLineWidth(1.5);
    qglColor(Qt::yellow);

    double width = m_bank_marking_height*0.5;
    double y = -m_horizon_radius + m_bank_marking_height * 0.8 + 1;

    glBegin(GL_LINE_LOOP);
    glVertex2d(0, -m_horizon_radius+1);
    glVertex2d(-width, y);
    glVertex2d(+width, y);
    glEnd();

    //TODO engine failure during t/o or g/a -> slip indicator turns blue

    double slip_x_offset = (width * 2.5) * -m_flightstatus->slip_percent * 0.01;
    glBegin(GL_LINE_LOOP);
    glVertex2d(-width - slip_x_offset, y);
    glVertex2d(+width - slip_x_offset, y);
    glVertex2d(+width - slip_x_offset, y + m_bank_marking_height * 0.3);
    glVertex2d(-width - slip_x_offset, y + m_bank_marking_height * 0.3);
    glEnd();

//     //TODO
//     // draw fbw fpv target

//     qglColor(Qt::red);

//     double fpv_target_y = 
//         getVerticalOffsetByPitch(m_fmc_control->pitchController()->flightPathVerticalTarget()+m_pitch);
//     glBegin(GL_LINES);
//     glVertex2d(-30, fpv_target_y);
//     glVertex2d(30, fpv_target_y);
//     glEnd();

//     //TODO
//     // draw fbw pitch target

//     qglColor(Qt::red);

//     double pitch_target_y = 
//         getVerticalOffsetByPitch(m_fmc_control->pitchController()->pitchTarget()-m_pitch);
//     glBegin(GL_LINES);
//     glVertex2d(-20, -pitch_target_y);
//     glVertex2d(20, -pitch_target_y);
//     glEnd();

    // draw flight path

    if (m_fmc_control->fmcAutoPilot().isFlightPathModeEnabled())
    {
        double fpv = m_flightstatus->smoothedFPVVertical();

        double fpv_y = getVerticalOffsetByPitch(fpv+m_pitch);
        double fpv_x = -m_flightstatus->wind_correction_angle_deg * m_hdg_pixel_per_deg;
        double fpv_size = qMax(m_horizon_half_width * 0.125, 10.0);
        
        glLineWidth(2.0);
        qglColor(GREEN);

        //----- flight director

        if (m_flightstatus->fd_active && !m_fmc_control->bankController()->isBankExcessive())
        {
            glPushMatrix();

            glTranslated(fpv_x, fpv_y + getVerticalOffsetByPitch(m_pitch - fd_pitch), 0.0);
            glRotated(-fd_bank, 0, 0, 1.0);

            double circle_size = fpv_size*0.25;

            glBegin(GL_LINES);
            glVertex2d(-m_fd_width, 0);
            glVertex2d(-circle_size, 0);
            glVertex2d(+circle_size, 0);
            glVertex2d(+m_fd_width, 0);
            glEnd();

            double triangle_size = m_fd_width / 10.0;

            glBegin(GL_TRIANGLES);
            glVertex2d(-m_fd_width, -triangle_size);
            glVertex2d(-m_fd_width+triangle_size*2.0, 0);
            glVertex2d(-m_fd_width, +triangle_size);
            glVertex2d(+m_fd_width, -triangle_size);
            glVertex2d(+m_fd_width-triangle_size*2.0, 0);
            glVertex2d(+m_fd_width, +triangle_size);
            glEnd();

            GLDraw::drawCircle(circle_size, -M_PI, M_PI);

            glPopMatrix();
        }

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

    // draw joystick input

    if (m_draw_joystick_input_when_engines_running &&
        m_flightstatus->isAtLeastOneEngineOn() &&
        m_flightstatus->radarAltitude() < 10)
    {
        qglColor(Qt::white);
        glLineWidth(2.0);

        double joy_input_max = m_horizon_half_width * 0.75;
        double joy_x_offset = m_flightstatus->aileron_input_percent * 0.01 * joy_input_max;
        double joy_y_offset = -m_flightstatus->elevator_input_percent * 0.01 * joy_input_max;
        double joy_cross_size = qMax(m_horizon_half_width * 0.051, 15.0);

        glBegin(GL_LINES);
        // input corner marker left/top
        glVertex2d(-joy_input_max, -joy_input_max+joy_cross_size);
        glVertex2d(-joy_input_max, -joy_input_max);
        glVertex2d(-joy_input_max, -joy_input_max);
        glVertex2d(-joy_input_max+joy_cross_size, -joy_input_max);
        // input corner marker right/top
        glVertex2d(+joy_input_max, -joy_input_max+joy_cross_size);
        glVertex2d(+joy_input_max, -joy_input_max);
        glVertex2d(+joy_input_max, -joy_input_max);
        glVertex2d(+joy_input_max-joy_cross_size, -joy_input_max);
        // input corner marker left/bottom
        glVertex2d(-joy_input_max, +joy_input_max-joy_cross_size);
        glVertex2d(-joy_input_max, +joy_input_max);
        glVertex2d(-joy_input_max, +joy_input_max);
        glVertex2d(-joy_input_max+joy_cross_size, +joy_input_max);
        // input corner marker right/bottom
        glVertex2d(+joy_input_max, +joy_input_max-joy_cross_size);
        glVertex2d(+joy_input_max, +joy_input_max);
        glVertex2d(+joy_input_max, +joy_input_max);
        glVertex2d(+joy_input_max-joy_cross_size, +joy_input_max);
        // joy input cross
        joy_cross_size *= 0.5;
        glVertex2d(joy_x_offset - joy_cross_size, joy_y_offset);
        glVertex2d(joy_x_offset + joy_cross_size, joy_y_offset);
        glVertex2d(joy_x_offset, joy_y_offset - joy_cross_size);
        glVertex2d(joy_x_offset, joy_y_offset + joy_cross_size);
        glEnd();
    }

    //TODO draw ground roll command bar

//     if (m_flightstatus->radarAltitude() < 30.0 && !m_flightstatus->nav1.id().isEmpty() && m_flightstatus->nav1_has_loc)
//     {
//         double needle_offset = 

//         double y_offset = 2.0*m_horizon_side_marking_height;

//         double bar_width = m_horizon_middle_marking_width;
//         double bar_height = qMax(m_horizon_half_height * 0.3, 15.0);

//         qglColor(GREEN);
//         glLineWidth(2.0);

//         glBegin(GL_LINE_LOOP);
//         glVertex2d(needle_offset, +y_offset);
//         glVertex2d(needle_offset-bar_width, bar_width*2.0+y_offset);
//         glVertex2d(needle_offset-bar_width, bar_height+y_offset);
//         glVertex2d(needle_offset+bar_width, bar_height+y_offset);
//         glVertex2d(needle_offset+bar_width, bar_width*2.0+y_offset);
//         glEnd();
//     }

    // draw flight director

    if (m_flightstatus->fd_active && 
        !m_fmc_control->fmcAutoPilot().isFlightPathModeEnabled() && 
        !m_fmc_control->bankController()->isBankExcessive())
    {
        qglColor(Qt::green);
        glLineWidth(3.0);
        
        // pitch

        double fd_pitch_offset = -getVerticalOffsetByPitch(fd_pitch - m_pitch);
        if (fd_pitch_offset > 0) fd_pitch_offset = qMin(fd_pitch_offset, m_fd_height);
        if (fd_pitch_offset < 0) fd_pitch_offset = qMax(fd_pitch_offset, -m_fd_height);

        glBegin(GL_LINES);
        glVertex2d(-m_fd_width, fd_pitch_offset);
        glVertex2d(+m_fd_width, fd_pitch_offset);
        glEnd();

        // bank

        double fd_bank_offset = getHorizontalOffsetByHeading(fd_bank - m_bank);
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

    // draw bank controller status

//     glPushMatrix();

//     glRotated(m_fmc_control->bankController()->targetBank(), 0, 0, 1.0);

//     glLineWidth(2.0);
//     qglColor(Qt::red);

//     glBegin(GL_LINES);
//     glVertex2d(0, -m_horizon_radius);
//     glVertex2d(0, -m_horizon_radius-10);
//     glEnd();
    
//     glPopMatrix();
}

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleA::drawAltitude()
{
    glPushMatrix();

    double& alt = m_alt_readout;
    double& vs = m_vs;

    double alt_fraction_y_offset = ( ((int)(alt * 100)) % 10000) / 100.0 * m_alt_pixel_per_ft;
    double alt_tape_y_offset = +m_horizon_half_height + alt_fraction_y_offset - m_alt_pixel_per_ft*10;

    //TODO make this non linear
    double limited_vs = LIMIT(vs, VS_SCALE_MAX);
    double rocd_indicator_left_y = -limited_vs * m_horizon_half_height / VS_SCALE_MAX;
    double rocd_indicator_right_y = m_rocd_root_offset * rocd_indicator_left_y / (m_rocd_band_width + m_rocd_root_offset);

    qglColor(DARKGRAY);
    glLineWidth(1.5);

    glBegin(GL_QUADS);
    //upper band
    glVertex2d(m_alt_band_horiz_offset, -m_horizon_half_height);
    glVertex2d(m_alt_band_horiz_offset+m_alt_band_width, -m_horizon_half_height);
    glVertex2d(m_alt_band_horiz_offset+m_alt_band_width, -m_alt_band_vert_gap/2.0);
    glVertex2d(m_alt_band_horiz_offset, -m_alt_band_vert_gap/2.0);
    //lower band
    glVertex2d(m_alt_band_horiz_offset, +m_horizon_half_height);
    glVertex2d(m_alt_band_horiz_offset+m_alt_band_width, +m_horizon_half_height);
    glVertex2d(m_alt_band_horiz_offset+m_alt_band_width, +m_alt_band_vert_gap/2.0);
    glVertex2d(m_alt_band_horiz_offset, +m_alt_band_vert_gap/2.0);
    glEnd();
    // rocd band
    glBegin(GL_POLYGON);
    glVertex2d(m_rocd_band_horiz_offset, -m_horizon_half_height);
    glVertex2d(m_rocd_band_horiz_offset, +m_horizon_half_height);
    glVertex2d(m_rocd_band_horiz_offset+m_rocd_band_width, +m_horizon_size.height()/4.0);
    glVertex2d(m_rocd_band_horiz_offset+m_rocd_band_width, -m_horizon_size.height()/4.0);
    glEnd();
    // thick lines
    qglColor(Qt::white);
    glLineWidth(2.0);
    glBegin(GL_LINES);
    // upper
    glVertex2d(m_alt_band_horiz_offset, -m_horizon_half_height);
    glVertex2d(m_alt_band_horiz_offset+m_alt_band_width*1.3, -m_horizon_half_height);
    glVertex2d(m_alt_band_horiz_offset+m_alt_band_width, -m_horizon_half_height);
    glVertex2d(m_alt_band_horiz_offset+m_alt_band_width, -m_alt_band_vert_gap/2.0);
    // lower
    glVertex2d(m_alt_band_horiz_offset+m_alt_band_width, +m_horizon_half_height);
    glVertex2d(m_alt_band_horiz_offset+m_alt_band_width, +m_alt_band_vert_gap/2.0);
    glVertex2d(m_alt_band_horiz_offset, +m_horizon_half_height);
    glVertex2d(m_alt_band_horiz_offset+m_alt_band_width*1.3, +m_horizon_half_height);
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
        glVertex2d(m_alt_band_horiz_offset+m_alt_band_width, y + alt_tape_y_offset);
        glVertex2d(m_alt_band_horiz_offset+m_alt_band_width-m_alt_tick_width, y + alt_tape_y_offset);
        glEnd();

        if ((alt/100) % 5 == 0)
            drawText(m_alt_band_horiz_offset+2, y+alt_tape_y_offset+m_font_height_half, 
                     QString("%1").arg(qAbs(alt)/100, 3, 10, QChar('0')));
    }

    // draw line for dest airport altitude

    const Airport* airport = m_fmc_control->normalRoute().destinationAirport();
    
    if (!m_flightstatus->onground && m_fmc_control->flightModeTracker().isApproach() &&
        airport != 0 && airport->hasActiveRunway())
    {
        double gnd_y = (alt - airport->activeRunway().thresholdElevationFt()) * m_alt_pixel_per_ft;
        if (gnd_y < m_horizon_half_height)
        {
            gnd_y = qMax(-m_horizon_half_height, gnd_y);
            glLineWidth(3.0);
            qglColor(CYAN);
            glBegin(GL_LINES);
            glVertex2d(m_alt_band_horiz_offset, gnd_y);
            glVertex2d(m_alt_band_horiz_offset+m_alt_band_width, gnd_y);
            glEnd();
        }
    }

    // draw red line for ground level

    double alt_diff = m_flightstatus->radarAltitude();

    if (alt_diff < 570)
    {
        double gnd_y = alt_diff* m_alt_pixel_per_ft;
        if (gnd_y < m_horizon_half_height)
        {
            gnd_y = qMax(-m_horizon_half_height, gnd_y);
            glLineWidth(1.0);
            qglColor(Qt::red);
            glBegin(GL_QUADS);
            glVertex2d(m_alt_band_horiz_offset+m_alt_band_width+1, gnd_y);
            glVertex2d(m_alt_band_horiz_offset+m_alt_band_width-m_alt_tick_width, gnd_y);
            glVertex2d(m_alt_band_horiz_offset+m_alt_band_width-m_alt_tick_width, m_horizon_half_height-1);
            glVertex2d(m_alt_band_horiz_offset+m_alt_band_width+1, m_horizon_half_height-1);
            glEnd();
        }
    }

    // repaint alt tape above and below the drawing area with black

    qglColor(Qt::black);
    double repaint_y_offset = m_horizon_half_height;
    double repaint_height = m_alt_pixel_per_ft*10 + alt_fraction_y_offset + m_font_height;

    glBegin(GL_QUADS);
    // upper
    glVertex2d(m_alt_band_horiz_offset-1, -repaint_y_offset-1);
    glVertex2d(m_alt_band_horiz_offset-1, -repaint_y_offset-repaint_height);
    glVertex2d(m_alt_band_horiz_offset+m_alt_band_width+1, -repaint_y_offset-repaint_height);
    glVertex2d(m_alt_band_horiz_offset+m_alt_band_width+1, -repaint_y_offset-1);
    //lower
    glVertex2d(m_alt_band_horiz_offset-1, repaint_y_offset+1);
    glVertex2d(m_alt_band_horiz_offset-1, repaint_y_offset+repaint_height);
    glVertex2d(m_alt_band_horiz_offset+m_alt_band_width+1, repaint_y_offset+repaint_height);
    glVertex2d(m_alt_band_horiz_offset+m_alt_band_width+1, repaint_y_offset+1);
    glEnd();

    // draw AP selected alt 

    qglColor(Qt::cyan);
    glLineWidth(1.5);

    double ap_alt_marker_y_offset = (alt - m_flightstatus->APAlt()) * m_alt_pixel_per_ft;

    QString sel_alt_string = 
        (m_fmc_control->isAltimeterSetToSTD(m_left_side)) ?
        QString("FL%1").arg(m_flightstatus->APAlt() / 100) :
        QString::number(m_flightstatus->APAlt() / 100 * 100);
    
    if (m_fmc_control->showMetricAlt())
    {
        QString metric_alt = QString("%1 M").arg(m_flightstatus->APAlt() * Navcalc::FEET_TO_METER, 0, 'f', 0);
        drawText(m_alt_band_horiz_offset-getTextWidth(metric_alt)-getTextWidth("XX"), 
                 -m_horizon_half_height, metric_alt);
    }

    if (fabs(ap_alt_marker_y_offset) > m_horizon_half_height)
    {
        if (ap_alt_marker_y_offset < 0)
            drawText(m_alt_band_horiz_offset+5, -m_horizon_half_height, sel_alt_string);
        else
            drawText(m_alt_band_horiz_offset+5, m_horizon_half_height + m_font_height, sel_alt_string);
    }
    else
    {
        double ap_alt_marker_width = m_horizon_size.width() * 0.1;
        double ap_alt_marker_height = 100 * m_alt_pixel_per_ft;
        double ap_alt_nose_size = ap_alt_marker_width/3.0;
        
        glLineWidth(2.0);
        glBegin(GL_LINE_LOOP);
        glVertex2d(m_alt_band_horiz_offset-ap_alt_marker_width/3.0,     ap_alt_marker_y_offset-ap_alt_marker_height);
        glVertex2d(m_alt_band_horiz_offset+ap_alt_marker_width*2.0/3.0, ap_alt_marker_y_offset-ap_alt_marker_height);
        glVertex2d(m_alt_band_horiz_offset+ap_alt_marker_width*2.0/3.0, ap_alt_marker_y_offset+ap_alt_marker_height);
        glVertex2d(m_alt_band_horiz_offset-ap_alt_marker_width/3.0,     ap_alt_marker_y_offset+ap_alt_marker_height);
        glVertex2d(m_alt_band_horiz_offset-ap_alt_marker_width/3.0,     ap_alt_marker_y_offset+ap_alt_nose_size);
        glVertex2d(m_alt_band_horiz_offset-ap_alt_marker_width/3.0+ap_alt_nose_size, ap_alt_marker_y_offset);
        glVertex2d(m_alt_band_horiz_offset-ap_alt_marker_width/3.0, ap_alt_marker_y_offset-ap_alt_nose_size);
        glEnd();
    }

    // repaint alt value background

    qglColor(Qt::black);
    glBegin(GL_QUADS);
    // between gap
    glVertex2d(m_alt_band_horiz_offset,    -m_alt_band_vert_gap/2.0);
    glVertex2d(m_alt_value_horiz_offset+3, -m_alt_band_vert_gap/2.0);
    glVertex2d(m_alt_value_horiz_offset+3, +m_alt_band_vert_gap/2.0);
    glVertex2d(m_alt_band_horiz_offset,    +m_alt_band_vert_gap/2.0);
    // right up/down
    glVertex2d(m_alt_band_horiz_offset+m_alt_band_width, -m_font_height);
    glVertex2d(m_alt_value_horiz_offset+3, -m_font_height);
    glVertex2d(m_alt_value_horiz_offset+3, +m_font_height);
    glVertex2d(m_alt_band_horiz_offset+m_alt_band_width, +m_font_height);
    glEnd();

    // rocd marks

    qglColor(Qt::white);
    double mark_offset = 0;
    for(int index = 0; index < 6; ++index)
    {
        if (index % 2 == 0)
        {
            glLineWidth(2.0);
            glBegin(GL_LINES);
            glVertex2d(m_rocd_band_horiz_offset,   mark_offset);
            glVertex2d(m_rocd_band_horiz_offset+4, mark_offset);
            glVertex2d(m_rocd_band_horiz_offset,   -mark_offset);
            glVertex2d(m_rocd_band_horiz_offset+4, -mark_offset);
            glEnd();
        }
        else
        {
            glLineWidth(1.5);
            glBegin(GL_LINES);
            glVertex2d(m_rocd_band_horiz_offset,   mark_offset);
            glVertex2d(m_rocd_band_horiz_offset+2, mark_offset);
            glVertex2d(m_rocd_band_horiz_offset,   -mark_offset);
            glVertex2d(m_rocd_band_horiz_offset+2, -mark_offset);
            glEnd();
        }

        mark_offset += m_rocd_mark_500ft_y;
    }
    
    // draw alt value

    //TODO always or only in DES/APP modes?
    if ((m_fmc_control->flightModeTracker().isDescent() || m_fmc_control->flightModeTracker().isApproach()) && m_is_below_mda)
    {
        qglColor(AMBER);
    }
    else
    {
        qglColor(Qt::green);
    }

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

    for(int index = +2; index >= -2; --index)
    {
        if (clipped_last_tow_digit_alt_value < 0) 
            clipped_last_tow_digit_alt_value = 100 - clipped_last_tow_digit_alt_value;

        drawText(m_alt_value_horiz_offset - m_alt_last_two_digit_offset + 2, 
                 m_font_height_half + last_two_digit_y_offset + (index*m_font_height),
                 QString("%1").arg(clipped_last_tow_digit_alt_value, 2, 10, QChar('0')));

        clipped_last_tow_digit_alt_value = (clipped_last_tow_digit_alt_value + 20) % 100;
    }

    drawText(m_alt_value_horiz_offset - m_alt_last_two_digit_offset - getTextWidth(alt_text) - 1, m_font_height_half, alt_text);

    qglColor(Qt::black);
    glBegin(GL_QUADS);
    glVertex2d(m_alt_band_horiz_offset+m_alt_band_width+2, -m_font_height*1.5 - 1);
    glVertex2d(m_alt_band_horiz_offset+m_alt_band_width+2, -m_font_height*1.5 - 2*m_font_height);
    glVertex2d(m_rocd_band_horiz_offset-1, -m_font_height*1.5 - 1 - 2*m_font_height);
    glVertex2d(m_rocd_band_horiz_offset-1, -m_font_height*1.5 - 1);

    glVertex2d(m_alt_band_horiz_offset+m_alt_band_width+2, +m_font_height*1.5 + 1);
    glVertex2d(m_alt_band_horiz_offset+m_alt_band_width+2, +m_font_height*1.5 + 2*m_font_height);
    glVertex2d(m_rocd_band_horiz_offset-1, +m_font_height*1.5 + 1 + 2*m_font_height);
    glVertex2d(m_rocd_band_horiz_offset-1, +m_font_height*1.5 + 1);
    glEnd();

    // draw rocd

    double rocd_text_y_corr = 0;

    double vs_abs =qAbs(vs);
    double radar_alt = m_flightstatus->radarAltitude();

    if (vs_abs >= 200)
    {
        if (rocd_indicator_left_y > 0) rocd_text_y_corr = m_font_height;

        qglColor(Qt::black);
        glBegin(GL_QUADS);
        glVertex2d(m_rocd_band_horiz_offset-1, rocd_indicator_left_y + rocd_text_y_corr);
        glVertex2d(m_rocd_band_horiz_offset+m_rocd_band_width+1, rocd_indicator_left_y + rocd_text_y_corr);
        glVertex2d(m_rocd_band_horiz_offset+m_rocd_band_width+1, rocd_indicator_left_y + rocd_text_y_corr - m_font_height);
        glVertex2d(m_rocd_band_horiz_offset-1, rocd_indicator_left_y + rocd_text_y_corr - m_font_height);
        glEnd();
    }

    qglColor(Qt::green);

    if ((vs_abs> 6000.0) || 
        (radar_alt < 2500.0 && radar_alt > 1000.0 && m_vs < -2000.0) ||
        (radar_alt < 1000.0 && m_vs < -1200.0)) qglColor(MAGENTA);

    glLineWidth(2.0);
    glBegin(GL_LINES);
    glVertex2d(m_rocd_band_horiz_offset+1, rocd_indicator_left_y);
    glVertex2d(m_rocd_band_horiz_offset+m_rocd_band_width-1, rocd_indicator_right_y);
    glEnd();

    if (vs_abs >= 200)
    {
        QString rocd_value = QString::number(Navcalc::round( qAbs(vs) / 100.0 ));
        drawText(m_rocd_band_horiz_offset, rocd_indicator_left_y + rocd_text_y_corr, rocd_value);
    }

    //draw alt value border

    double font_height_onehalf = m_font_height*1.5;

    glLineWidth(1.5);

    bool draw_alt_window = true;

    qglColor(Qt::yellow);
    if (m_fmc_control->flightStatusChecker().isAltitudeDeviationAlert())
    {
        qglColor(AMBER);
        if (m_altitude_window_blink_timer.elapsed() > 1000) m_altitude_window_blink_timer.start();
        if (m_altitude_window_blink_timer.elapsed() < 500)  draw_alt_window = false;
    }

    if (draw_alt_window)
    {
        glBegin(GL_LINES);
        glVertex2d(m_alt_band_horiz_offset,                  -m_alt_band_vert_gap/2.0);
        glVertex2d(m_alt_band_horiz_offset+m_alt_band_width, -m_alt_band_vert_gap/2.0);
        
        glVertex2d(m_alt_band_horiz_offset+m_alt_band_width, -font_height_onehalf);
        glVertex2d(m_alt_value_horiz_offset+3,               -font_height_onehalf);
        
        glVertex2d(m_alt_value_horiz_offset+3,               -font_height_onehalf);
        glVertex2d(m_alt_value_horiz_offset+3,               +font_height_onehalf);
        
        glVertex2d(m_alt_value_horiz_offset+3,               +font_height_onehalf);
        glVertex2d(m_alt_band_horiz_offset+m_alt_band_width, +font_height_onehalf);
        
        glVertex2d(m_alt_band_horiz_offset+m_alt_band_width, +m_alt_band_vert_gap/2.0);
        glVertex2d(m_alt_band_horiz_offset,                  +m_alt_band_vert_gap/2.0);
        glEnd();
    }

    // draw alt marker

    qglColor(Qt::yellow);
    glLineWidth(3.0);
    glBegin(GL_LINES);
    glVertex2d(m_alt_marker_offset, 0);
    glVertex2d(m_alt_marker_offset + m_alt_marker_width, 0);
    glEnd();    

    // draw ILS

    if (m_fmc_control->showPFDILS(m_left_side))
    {
        // draw loc & gs

        double loc_y_offset = m_hdg_band_vert_offset - 2*m_font_height;
        double loc_needle = m_left_side ? m_flightstatus->obs1_loc_needle : m_flightstatus->obs2_loc_needle;
        double loc_needle_offset = (4*m_ils_circle_offset / 127.0) * loc_needle;
        if (qAbs(loc_needle) < MAX_LOC_NEEDLE_DEV)
            loc_needle_offset += m_left_side ? m_fmc_control->getIls1Noise() : m_fmc_control->getIls2Noise();

        double gs_x_offset = m_alt_marker_offset+m_alt_marker_width/2.0;
        double gs_needle = m_left_side ? m_flightstatus->obs1_gs_needle : m_flightstatus->obs2_gs_needle;
        double gs_needle_offset = (4*m_ils_circle_offset / 127) * gs_needle;
        if (qAbs(gs_needle) < MAX_GS_NEEDLE_DEV)
            gs_needle_offset += m_left_side ? m_fmc_control->getIls1Noise() : m_fmc_control->getIls2Noise();

        // loc middle line
        qglColor(Qt::yellow);
        glLineWidth(3.0);
        glBegin(GL_LINES);
        glVertex2d(0, -2*m_ils_circle_size+loc_y_offset);
        glVertex2d(0, +2*m_ils_circle_size+loc_y_offset);
        glEnd();    

        qglColor(Qt::white);

        glLineWidth(2.0);

        // loc
        GLDraw::drawCircleOffset(m_ils_circle_size, -M_PI, M_PI, -4*m_ils_circle_offset, loc_y_offset);
        GLDraw::drawCircleOffset(m_ils_circle_size, -M_PI, M_PI, -2*m_ils_circle_offset, loc_y_offset);
        GLDraw::drawCircleOffset(m_ils_circle_size, -M_PI, M_PI, +2*m_ils_circle_offset, loc_y_offset);
        GLDraw::drawCircleOffset(m_ils_circle_size, -M_PI, M_PI, +4*m_ils_circle_offset, loc_y_offset);

        // gs
        GLDraw::drawCircleOffset(m_ils_circle_size, -M_PI, M_PI, gs_x_offset, -4*m_ils_circle_offset);
        GLDraw::drawCircleOffset(m_ils_circle_size, -M_PI, M_PI, gs_x_offset, -2*m_ils_circle_offset);
        GLDraw::drawCircleOffset(m_ils_circle_size, -M_PI, M_PI, gs_x_offset, +2*m_ils_circle_offset);
        GLDraw::drawCircleOffset(m_ils_circle_size, -M_PI, M_PI, gs_x_offset, +4*m_ils_circle_offset);

        qglColor(Qt::magenta);
        glLineWidth(1.5);

        if ((m_left_side && !m_flightstatus->nav1.id().isEmpty() && m_flightstatus->nav1_has_loc) ||
            (!m_left_side && !m_flightstatus->nav2.id().isEmpty() && m_flightstatus->nav2_has_loc))
        {
            // loc needle

            glBegin(GL_LINES);
            if (loc_needle > -MAX_LOC_NEEDLE_DEV)
            {
                glVertex2d(loc_needle_offset               , +m_gs_bug_width+loc_y_offset);
                glVertex2d(loc_needle_offset-m_gs_bug_height, loc_y_offset);
                glVertex2d(loc_needle_offset-m_gs_bug_height, loc_y_offset);
                glVertex2d(loc_needle_offset               , -m_gs_bug_width+loc_y_offset);
            }

            if (loc_needle < MAX_LOC_NEEDLE_DEV)
            {
                glVertex2d(loc_needle_offset               , -m_gs_bug_width+loc_y_offset);
                glVertex2d(loc_needle_offset+m_gs_bug_height, loc_y_offset);
                glVertex2d(loc_needle_offset+m_gs_bug_height, loc_y_offset);
                glVertex2d(loc_needle_offset               , +m_gs_bug_width+loc_y_offset);
            }
            glEnd();

            m_ils_circle_offset = m_horizon_size.height() / 10.0;
            m_gs_bug_width = 1.9*m_ils_circle_size;
            m_gs_bug_height = m_gs_bug_width * 1.5;

            // gs needle
        
            glBegin(GL_LINES);

            if (gs_needle > -MAX_GS_NEEDLE_DEV)
            {
                glVertex2d(gs_x_offset-m_gs_bug_width, gs_needle_offset);
                glVertex2d(gs_x_offset               , -m_gs_bug_height+gs_needle_offset);
                glVertex2d(gs_x_offset               , -m_gs_bug_height+gs_needle_offset);
                glVertex2d(gs_x_offset+m_gs_bug_width, gs_needle_offset);
            }

            if (gs_needle < MAX_GS_NEEDLE_DEV)
            {
                glVertex2d(gs_x_offset+m_gs_bug_width, gs_needle_offset);
                glVertex2d(gs_x_offset               , +m_gs_bug_height+gs_needle_offset);
                glVertex2d(gs_x_offset               , +m_gs_bug_height+gs_needle_offset);
                glVertex2d(gs_x_offset-m_gs_bug_width, gs_needle_offset);
            }
            glEnd();
        }
    }

    glPopMatrix();
}

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleA::drawSpeed()
{
    glPushMatrix();

    // draw speed

    double ias_trend_per_second = 0.0;
    double ias = m_flightstatus->smoothedIAS(&ias_trend_per_second);
    double speed_fraction_y_offset = ( ((int)(ias * 100)) % 1000) / 100.0 * m_spd_pixel_per_knot;

    qglColor(DARKGRAY);
    glLineWidth(1.5);

    glBegin(GL_QUADS);
    glVertex2d(m_speed_band_horiz_offset, -m_horizon_half_height);
    glVertex2d(m_speed_band_horiz_offset+m_speed_band_width, -m_horizon_half_height);
    glVertex2d(m_speed_band_horiz_offset+m_speed_band_width, +m_horizon_half_height);
    glVertex2d(m_speed_band_horiz_offset, +m_horizon_half_height);
    glEnd();
    // thick lines
    qglColor(Qt::white);
    glLineWidth(2.0);
    glBegin(GL_LINES);
    glVertex2d(m_speed_band_horiz_offset, -m_horizon_half_height);
    glVertex2d(m_speed_band_horiz_offset+m_speed_band_width*1.3, -m_horizon_half_height);
    glVertex2d(m_speed_band_horiz_offset+m_speed_band_width, -m_horizon_half_height);
    glVertex2d(m_speed_band_horiz_offset+m_speed_band_width, +m_horizon_half_height);
    glVertex2d(m_speed_band_horiz_offset, +m_horizon_half_height);
    glVertex2d(m_speed_band_horiz_offset+m_speed_band_width*1.3, +m_horizon_half_height);
    glEnd();

    // draw speed tape

    qglColor(Qt::white);
    glLineWidth(2.0);
    double speed_tick_width = 7;

    int start_speed = (((int)ias) / 10 * 10) - m_speed_range/2;
    int max_speed = start_speed + m_speed_range + 20;

    for(int speed = start_speed; speed < max_speed; speed += 10)
    {
        if (speed < MIN_SPEED) continue;
        double y = (start_speed - speed) * m_spd_pixel_per_knot + m_horizon_half_height;
        glBegin(GL_LINES);
        glVertex2d(m_speed_band_horiz_offset+m_speed_band_width, y + speed_fraction_y_offset);
        glVertex2d(m_speed_band_horiz_offset+m_speed_band_width-speed_tick_width, y + speed_fraction_y_offset);
        glEnd();
        if ((speed/10) % 2 == 0) 
            drawText(m_speed_band_horiz_offset+2, y + speed_fraction_y_offset + m_font_height_half, QString::number(speed));
    }

    // repaint speed tape above and below the drawing area with black

    qglColor(Qt::black);

    glBegin(GL_QUADS);
    // upper
    glVertex2d(m_speed_band_horiz_offset-1, -m_speed_repaint_y_offset-1);
    glVertex2d(m_speed_band_horiz_offset-1, -m_speed_repaint_y_offset-m_speed_repaint_height);
    glVertex2d(m_speed_band_horiz_offset+m_speed_band_width+1, -m_speed_repaint_y_offset-m_speed_repaint_height);
    glVertex2d(m_speed_band_horiz_offset+m_speed_band_width+1, -m_speed_repaint_y_offset-1);
    //lower
    glVertex2d(m_speed_band_horiz_offset-1, m_speed_repaint_y_offset+1);
    glVertex2d(m_speed_band_horiz_offset-1, m_speed_repaint_y_offset+m_speed_repaint_height);
    glVertex2d(m_speed_band_horiz_offset+m_speed_band_width+1, m_speed_repaint_y_offset+m_speed_repaint_height);
    glVertex2d(m_speed_band_horiz_offset+m_speed_band_width+1, m_speed_repaint_y_offset+1);
    glEnd();

    // draw overspeed area

    double x_offset = m_speed_band_horiz_offset + m_speed_band_width;

    if (m_fmc_control->aircraftData().getMaxSpeedKts() > 10)
    {
        double overspeed_y_offset = 
            qMin(m_horizon_half_height-1, (ias - m_fmc_control->aircraftData().getMaxSpeedKts()) * m_spd_pixel_per_knot);
                 
        if (overspeed_y_offset > -m_horizon_half_height)
        {
            double thickness = 6.0;
            
            glLineWidth(thickness);
            qglColor(Qt::red);
            
            glLineStipple((int)thickness, 0xAAAA); // 0xAAAA = 1010101010101010
            glEnable(GL_LINE_STIPPLE);

            glBegin(GL_LINES);
            glVertex2d(x_offset + thickness/2.0, overspeed_y_offset+thickness);
            glVertex2d(x_offset + thickness/2.0, -m_horizon_half_height+1);
            glEnd();

            glDisable(GL_LINE_STIPPLE);

            glLineWidth(1.5);
            glBegin(GL_LINES);
            glVertex2d(x_offset + thickness, overspeed_y_offset);
            glVertex2d(x_offset + thickness, -m_horizon_half_height+1);
            glEnd();

//TODO
//             // draw speed protection sign

//             glLineWidth(1.2);
//             qglColor(Qt::green);
//             glBegin(GL_LINES);
//             glVertex2d(x_offset - speed_tick_width*0.5, overspeed_y_offset);
//             glVertex2d(x_offset - speed_tick_width*1.5, overspeed_y_offset);
//             glVertex2d(x_offset - speed_tick_width*0.5, overspeed_y_offset - 1.5*m_spd_pixel_per_knot);
//             glVertex2d(x_offset - speed_tick_width*1.5, overspeed_y_offset - 1.5*m_spd_pixel_per_knot);
//             glEnd();
        }
    }

    // draw stall speed

   
    double y_offset_stall_speed = 
        qMax(-m_horizon_half_height, (ias - m_fmc_control->aircraftData().getStallSpeed()) * m_spd_pixel_per_knot);

    if (m_fmc_control->aircraftData().getStallSpeed() > 0.0 && 
        m_flightstatus->radarAltitude() > 10 &&
        y_offset_stall_speed < m_horizon_half_height)
    {
        glLineWidth(1.5);
        qglColor(Qt::red);
        glBegin(GL_QUADS);
        glVertex2d(x_offset+1, y_offset_stall_speed);
        glVertex2d(x_offset+7, y_offset_stall_speed);
        glVertex2d(x_offset+7, m_horizon_half_height-1);
        glVertex2d(x_offset+1, m_horizon_half_height-1);
        glEnd();
    }

    // draw minimum selectable speed

    glLineWidth(2.0);

    if (m_fmc_control->aircraftData().getMinimumSelectableSpeed() > 0  && m_flightstatus->radarAltitude() > 10)
    {
        double y_offset = 
            qMax(-m_horizon_half_height, (ias - m_fmc_control->aircraftData().getMinimumSelectableSpeed()) * 
                 m_spd_pixel_per_knot);

        if (y_offset < m_horizon_half_height)
        {
            qglColor(AMBER);
            glBegin(GL_LINES);
            glVertex2d(x_offset+1, y_offset);
            glVertex2d(x_offset+4, y_offset);
            glVertex2d(x_offset+4, y_offset);
            glVertex2d(x_offset+4, qMin(m_horizon_half_height, y_offset_stall_speed));
            glEnd();
        }
    }

    // draw green dot speed

    if (m_fmc_control->aircraftData().getGreenDotSpeed() > 0)
    {
        double y_offset = (ias - ((int)m_fmc_control->aircraftData().getGreenDotSpeed())) * m_spd_pixel_per_knot;
        if (y_offset < m_horizon_half_height && y_offset > -m_horizon_half_height)
        {
            qglColor(Qt::green);
            GLDraw::drawCircleOffset(speed_tick_width/2.0, -M_PI, M_PI, x_offset, y_offset, 0.1);
        }
    }

    // draw V1, Vr

    if (m_flightstatus->radarAltitude() < 50)
    {
        if (m_fmc_data.V1() > 0)
        {
            qglColor(Qt::cyan);
            double y_offset = (ias - m_fmc_data.V1()) * m_spd_pixel_per_knot;

            if (y_offset < -m_horizon_half_height)
            {
                drawText(m_speed_marker_offset+3, m_font_height - (int)m_horizon_half_height,
                         QString::number(m_fmc_data.V1()));
            }
            else if (y_offset < m_horizon_half_height)
            {
                glBegin(GL_LINES);
                glVertex2d(m_speed_marker_offset, y_offset);
                glVertex2d(m_speed_marker_offset+m_speed_marker_width/4.0, y_offset);
                glEnd();
                drawText(m_speed_marker_offset+m_speed_marker_width/3.0, y_offset+m_font_height_half, "1");
            }
        }

        if (m_fmc_data.Vr() > 0)
        {
            double y_offset = (ias - m_fmc_data.Vr()) * m_spd_pixel_per_knot;
            if (y_offset < m_horizon_half_height && y_offset > -m_horizon_half_height)
            {
                qglColor(Qt::cyan);
                GLDraw::drawCircleOffset(speed_tick_width/2.0, -M_PI, M_PI, x_offset, y_offset, 0.1);
            }
        }
    }

    // draw Vf, Vs, next flaps max. speed

    qglColor(GREEN);

    if (m_flightstatus->current_flap_lever_notch > 2 &&
        m_flightstatus->current_flap_lever_notch < (m_flightstatus->flaps_lever_notch_count-1) && 
        m_fmc_control->aircraftData().getMinimumAStyleFlapsRetractionSpeed() > 0)
    {
        double y_offset = (ias - m_fmc_control->aircraftData().getMinimumAStyleFlapsRetractionSpeed()) * m_spd_pixel_per_knot;
        if (y_offset < m_horizon_half_height && y_offset > -m_horizon_half_height)
        {
            glBegin(GL_LINES);
            glVertex2d(m_speed_marker_offset, y_offset);
            glVertex2d(m_speed_marker_offset+m_speed_marker_width/4.0, y_offset);
            glEnd();
            drawText(m_speed_marker_offset+m_speed_marker_width/3.0, y_offset+m_font_height_half, "F");
        }
    }

    if ((m_flightstatus->current_flap_lever_notch == 1 || m_flightstatus->current_flap_lever_notch == 2) &&
        m_fmc_control->aircraftData().getMinimumAStyleSlatsRetractionSpeed() > 0)
    {
        double y_offset = (ias - m_fmc_control->aircraftData().getMinimumAStyleSlatsRetractionSpeed()) * m_spd_pixel_per_knot;
        if (y_offset < m_horizon_half_height && y_offset > -m_horizon_half_height)
        {
            glBegin(GL_LINES);
            glVertex2d(m_speed_marker_offset, y_offset);
            glVertex2d(m_speed_marker_offset+m_speed_marker_width/4.0, y_offset);
            glEnd();
            drawText(m_speed_marker_offset+m_speed_marker_width/3.0, y_offset+m_font_height_half, "S");
        }
    }

    qglColor(MAGENTA);

    if (m_alt_readout < 20000.0 &&
        m_flightstatus->current_flap_lever_notch+1 < m_flightstatus->flaps_lever_notch_count &&
        m_fmc_control->aircraftData().getMaxFlapsSpeed(m_flightstatus->current_flap_lever_notch+1) > 0)
    {
        uint next_flaps_notch = m_flightstatus->current_flap_lever_notch+1;
        if (m_flightstatus->current_flap_lever_notch == 1) ++next_flaps_notch;

        double y_offset = 
            (ias - m_fmc_control->aircraftData().getMaxFlapsSpeed(next_flaps_notch)) *
            m_spd_pixel_per_knot;

        if (y_offset < m_horizon_half_height && y_offset > -m_horizon_half_height)
        {
            glBegin(GL_LINES);
            glVertex2d(x_offset-speed_tick_width*1.5, y_offset-2);
            glVertex2d(x_offset-speed_tick_width*0.5, y_offset-2);
            glVertex2d(x_offset-speed_tick_width*1.5, y_offset+2);
            glVertex2d(x_offset-speed_tick_width*0.5, y_offset+2);
            glEnd();
        }
    }

    // draw speed marker
    
    qglColor(Qt::yellow);

    glLineWidth(4.0);
    glBegin(GL_LINES);
    glVertex2d(+m_speed_marker_offset-m_speed_marker_width/2.0, 0);
    glVertex2d(+m_speed_marker_offset+m_speed_marker_width/2.0, 0);
    glVertex2d(m_speed_band_horiz_offset, 0);
    glVertex2d(m_speed_band_horiz_offset+m_speed_marker_width/4.0, 0);
    glEnd();

    glLineWidth(1.6);
    glBegin(GL_LINES);    
    glVertex2d(m_speed_band_horiz_offset, 0);
    glVertex2d(m_speed_band_horiz_offset+m_speed_band_width, 0);
    glEnd();

    glLineWidth(2.0);
    glBegin(GL_TRIANGLES);
    glVertex2d(+m_speed_marker_offset, 0);
    glVertex2d(+m_speed_marker_offset+m_speed_marker_width/2.0, -5);
    glVertex2d(+m_speed_marker_offset+m_speed_marker_width/2.0, +5);
    glEnd();

    // draw speed trend
    
    if (qAbs(ias_trend_per_second*10) > 2)
    {
        double height = m_spd_pixel_per_knot * -ias_trend_per_second * 10;
        height = LIMIT(height, m_horizon_half_height);
        double x = +m_speed_marker_offset-2-m_speed_marker_width/2.0;

        glLineWidth(1.5);
        if (height < 0) GLDraw::drawVerticalArrow(x, height, x, 0, 4, 4, true, true, true);
        else            GLDraw::drawVerticalArrow(x, 0, x, height, 4, 4, false, true, true);
    }

    // draw AP selected speed

    if (m_flightstatus->APSpd() > 0)
    {
        qglColor(Qt::cyan);

        QString sel_ap_speed_string;
        double effective_ap_speed = m_flightstatus->APSpd();

        if (m_fmc_control->fmcAutothrottle().isAPThrottleModeMachSet())
        {
            effective_ap_speed = 
                Navcalc::getIasFromMach(
                    m_flightstatus->APMach(), m_flightstatus->oat, 
                    m_flightstatus->smoothedIAS(), m_flightstatus->tas, m_flightstatus->mach);

            sel_ap_speed_string = QString::number(effective_ap_speed, 'f', 0);
        }
        else
        {
            sel_ap_speed_string = QString::number(m_flightstatus->APSpd());
        }

        double ap_speed_marker_y_offset = (ias - effective_ap_speed) * m_spd_pixel_per_knot;
                
        if (ap_speed_marker_y_offset < -m_horizon_half_height)
        {
            drawText(m_speed_band_horiz_offset+5, -m_horizon_half_height, sel_ap_speed_string);
        }
        else if (ap_speed_marker_y_offset > m_horizon_half_height)
        {
            drawText(m_speed_band_horiz_offset+5, m_horizon_half_height + m_font_height, sel_ap_speed_string);
        }
        else
        {
            glLineWidth(2.0);
            glBegin(GL_LINE_LOOP);
            glVertex2d(+m_speed_marker_offset, 
                       ap_speed_marker_y_offset);
            glVertex2d(+m_speed_marker_offset+m_ap_speed_marker_width, 
                       ap_speed_marker_y_offset-m_ap_speed_marker_width*0.5*0.8);
            glVertex2d(+m_speed_marker_offset+m_ap_speed_marker_width, 
                       ap_speed_marker_y_offset+m_ap_speed_marker_width*0.5*0.8);
            glEnd();
        }
    }

    glPopMatrix();
}

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleA::drawHeading()
{
    glPushMatrix();

    // draw heading  

    double hdg_fraction_x_offset = ( ((int)(m_mag_hdg * 100)) % 1000) / 100.0 * m_hdg_pixel_per_deg;
    
    qglColor(DARKGRAY);
    glLineWidth(1.5);

    glBegin(GL_QUADS);
    glVertex2d(-m_horizon_half_width, m_hdg_band_vert_offset);
    glVertex2d(+m_horizon_half_width, m_hdg_band_vert_offset);
    glVertex2d(+m_horizon_half_width, m_hdg_band_vert_offset+m_hdg_band_height);
    glVertex2d(-m_horizon_half_width, m_hdg_band_vert_offset+m_hdg_band_height);
    glEnd();
    qglColor(Qt::white);
    // thick lines
    glLineWidth(2.0);
    glBegin(GL_LINES);
    glVertex2d(-m_horizon_half_width, m_hdg_band_vert_offset+m_hdg_band_height);
    glVertex2d(-m_horizon_half_width, m_hdg_band_vert_offset);
    glVertex2d(-m_horizon_half_width, m_hdg_band_vert_offset);
    glVertex2d(+m_horizon_half_width, m_hdg_band_vert_offset);
    glVertex2d(+m_horizon_half_width, m_hdg_band_vert_offset);
    glVertex2d(+m_horizon_half_width, m_hdg_band_vert_offset+m_hdg_band_height);
    glEnd();

    // draw hdg tape

    qglColor(Qt::white);
    glLineWidth(2.0);
    double hdg_tape_x_offset = -hdg_fraction_x_offset - m_hdg_pixel_per_deg*10;
    double hdg_tick_height = 7;
    
    int start_hdg = ((int)(m_mag_hdg) / 10 * 10) - (m_hdg_range / 2 / 10 * 10);
    int max_hdg = start_hdg + m_hdg_range + 10;
    for(int hdg = start_hdg; hdg < max_hdg; hdg += 5)
    {
        double x = (hdg - start_hdg - 10) * m_hdg_pixel_per_deg;

        if ((hdg) % 10 == 0)
        {
            glBegin(GL_LINES);
            glVertex2d(x + hdg_tape_x_offset, m_hdg_band_vert_offset);
            glVertex2d(x + hdg_tape_x_offset, m_hdg_band_vert_offset + hdg_tick_height);
            glEnd();        
            QString hdg_text = QString::number(Navcalc::trimHeading(hdg)/10);
            drawText(x+hdg_tape_x_offset-getTextWidth(hdg_text)/2.0, m_hdg_band_vert_offset+m_hdg_band_height, hdg_text);
        }
        else
        {
            glBegin(GL_LINES);
            glVertex2d(x + hdg_tape_x_offset, m_hdg_band_vert_offset);
            glVertex2d(x + hdg_tape_x_offset, m_hdg_band_vert_offset + hdg_tick_height/2.0);
            glEnd();        
        }
    }

    // draw hdg marker

    qglColor(Qt::yellow);
    glLineWidth(3.0);
    glBegin(GL_LINES);
    glVertex2d(0, +m_hdg_marker_offset);
    glVertex2d(0, +m_hdg_marker_offset+m_hdg_marker_height);
    glEnd();    

    // draw track diamond
    
    qglColor(Qt::green);
    glLineWidth(1.5);
    double wca = -m_flightstatus->wind_correction_angle_deg * m_hdg_pixel_per_deg;
    //if (m_navdisplay_config->getIntValue(CFG_WIND_CORRECTION) != 0) wca = -wca;

    double diamond_size = hdg_tick_height/2.0;

    glBegin(GL_LINE_LOOP);
    glVertex2d(wca, m_hdg_band_vert_offset+1);
    glVertex2d(wca+diamond_size, m_hdg_band_vert_offset+1+2*diamond_size);
    glVertex2d(wca, m_hdg_band_vert_offset+1+4*diamond_size);
    glVertex2d(wca-diamond_size, m_hdg_band_vert_offset+1+2*diamond_size);
    glEnd();

    // repaint hdg tape above and below the drawing area with black
    
    qglColor(Qt::black);

    glBegin(GL_QUADS);
    // left
    glVertex2d(-m_hdg_repaint_x_offset-1, m_hdg_band_vert_offset-2);
    glVertex2d(-m_hdg_repaint_x_offset-m_hdg_repaint_width, m_hdg_band_vert_offset-2);
    glVertex2d(-m_hdg_repaint_x_offset-m_hdg_repaint_width, m_hdg_band_vert_offset+m_hdg_band_height);
    glVertex2d(-m_hdg_repaint_x_offset-1, m_hdg_band_vert_offset+m_hdg_band_height);
    //right
    glVertex2d(m_hdg_repaint_x_offset+1, m_hdg_band_vert_offset-2);
    glVertex2d(m_hdg_repaint_x_offset+m_hdg_repaint_width, m_hdg_band_vert_offset-2);
    glVertex2d(m_hdg_repaint_x_offset+m_hdg_repaint_width, m_hdg_band_vert_offset+m_hdg_band_height);
    glVertex2d(m_hdg_repaint_x_offset+1, m_hdg_band_vert_offset+m_hdg_band_height);
    glEnd();

    if (!m_fmc_control->bankController()->isBankExcessive())
    {
        // draw ILS course marker

        if (m_fmc_control->showPFDILS(m_left_side))
        {
            qglColor(Qt::magenta);
            double ils_hdg_marker_x_offset = 
                -Navcalc::getSignedHeadingDiff(
                    (m_left_side ? m_flightstatus->obs1 : m_flightstatus->obs2), m_mag_hdg) * m_hdg_pixel_per_deg;

            if (ils_hdg_marker_x_offset >= -m_horizon_half_width && ils_hdg_marker_x_offset < m_horizon_half_width)
            {
                double marker_width = m_hdg_band_height*0.18;
                glLineWidth(2.0);
                glBegin(GL_LINES);
                glVertex2d(ils_hdg_marker_x_offset, m_hdg_band_vert_offset+m_hdg_band_height*0.1);
                glVertex2d(ils_hdg_marker_x_offset, m_hdg_band_vert_offset+m_hdg_band_height*0.9);
                glVertex2d(ils_hdg_marker_x_offset-marker_width, m_hdg_band_vert_offset + m_hdg_band_height*0.9 - marker_width);
                glVertex2d(ils_hdg_marker_x_offset+marker_width, m_hdg_band_vert_offset + m_hdg_band_height*0.9 - marker_width);
                glEnd();
            }
            else
            {
                QString obs_string = QString::number((m_left_side ? m_flightstatus->obs1 : m_flightstatus->obs2));
                double text_width = getTextWidth(obs_string);

                double x = (ils_hdg_marker_x_offset < -m_horizon_half_width) ?
                           (-m_horizon_half_width - text_width*0.1) :
                           (m_horizon_half_width - text_width*0.9);

                double y = m_hdg_band_vert_offset + m_font_height;

                qglColor(DARKGRAY);
                glBegin(GL_QUADS);
                glVertex2d(x, y);
                glVertex2d(x+text_width, y);
                glVertex2d(x+text_width, y-m_font_height);
                glVertex2d(x, y-m_font_height);
                glEnd();

                qglColor(Qt::white);
                glLineWidth(1.1);
                glBegin(GL_LINE_LOOP);
                glVertex2d(x-1, y);
                glVertex2d(x+1+text_width, y);
                glVertex2d(x+1+text_width, y-m_font_height);
                glVertex2d(x-1, y-m_font_height);
                glEnd();

                qglColor(Qt::magenta);
                drawText(x, y, obs_string);
            }
        }
    }
    
    // draw selected hdg marker

    if (!m_fmc_control->fmcAutoPilot().isNAVCoupled() && !m_fmc_control->fmcAutoPilot().isTakeoffModeActiveLateral())
    {
        qglColor(Qt::cyan);
        glLineWidth(1.5);
    
        double ap_hdg_marker_x_offset = -Navcalc::getSignedHeadingDiff(m_flightstatus->APHdg(), m_mag_hdg) * m_hdg_pixel_per_deg;
    
        if (ap_hdg_marker_x_offset < -m_horizon_half_width)
        {
            QString hdg_string = QString::number(m_flightstatus->APHdg());
            drawText(-m_horizon_half_width, m_hdg_band_vert_offset, hdg_string);
        }
        else if (ap_hdg_marker_x_offset > m_horizon_half_width)
        {
            QString hdg_string = QString::number(m_flightstatus->APHdg());
            drawText(m_horizon_half_width - getTextWidth(hdg_string), m_hdg_band_vert_offset, hdg_string);
        }
        else
        {
            glLineWidth(2.0);
            glBegin(GL_LINE_LOOP);
            glVertex2d(ap_hdg_marker_x_offset,
                       m_hdg_band_vert_offset);
            glVertex2d(ap_hdg_marker_x_offset+m_ap_hdg_marker_height*0.5*0.8,
                       m_hdg_band_vert_offset-m_ap_hdg_marker_height);
            glVertex2d(ap_hdg_marker_x_offset-m_ap_hdg_marker_height*0.5*0.8,
                       m_hdg_band_vert_offset-m_ap_hdg_marker_height);
            glEnd();
        }
    }

    glPopMatrix();
}

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleA::drawInfoTexts()
{
    glLoadIdentity();
    glTranslated(m_horizon_offset.x(), 0, 0);

    // draw metric alt

    if (m_fmc_control->showMetricAlt())
    {
        qglColor(Qt::green);
        QString metric_alt = 
            QString("%1 M").arg(m_alt_readout * Navcalc::FEET_TO_METER, 0, 'f', 0);

        drawText(m_alt_band_horiz_offset, size().height(), metric_alt);
    }

    if (!m_fmc_control->bankController()->isBankExcessive())
    {
        if (m_fmc_control->showPFDILS(m_left_side))
        {
            if ((m_left_side && !m_flightstatus->nav1.id().isEmpty() && m_flightstatus->nav1_has_loc) ||
                (!m_left_side && !m_flightstatus->nav2.id().isEmpty() && m_flightstatus->nav2_has_loc))
            {
                // draw ILS info
                
                double x_text_offset = m_speed_band_horiz_offset;
                double y_text_offset = size().height() - m_font_height_double;
                qglColor(Qt::magenta);
                drawText(x_text_offset, y_text_offset, 
                         (m_left_side ? m_flightstatus->nav1.id() : m_flightstatus->nav2.id()));
                
                y_text_offset += m_font_height;
                QString text;

                if (m_left_side)
                {
                    if (m_flightstatus->nav1_freq == 0) 
                        text = QString("---.--");
                    else                                
                        text = QString("%1").arg(m_flightstatus->nav1_freq / 1000.0, 3, 'f', 2);
                }
                else
                {
                    if (m_flightstatus->nav2_freq == 0) 
                        text = QString("---.--");
                    else                                
                        text = QString("%1").arg(m_flightstatus->nav2_freq / 1000.0, 3, 'f', 2);
                }

                drawText(x_text_offset, y_text_offset, text);
                
                y_text_offset += m_font_height;
                QString dist_text;

                if (m_left_side && !m_flightstatus->nav1_distance_nm.isEmpty())
                {
                    QString dist_string = QString("%1").arg(m_flightstatus->nav1_distance_nm);
                    drawText(x_text_offset, y_text_offset, dist_string);
                    qglColor(Qt::cyan);
                    drawText(x_text_offset+getTextWidth(dist_string), y_text_offset, " NM");
                }

                if (!m_left_side && !m_flightstatus->nav2_distance_nm.isEmpty())
                {
                    QString dist_string = QString("%1").arg(m_flightstatus->nav2_distance_nm);
                    drawText(x_text_offset, y_text_offset, dist_string);
                    qglColor(Qt::cyan);
                    drawText(x_text_offset+getTextWidth(dist_string), y_text_offset, " NM");
                }
            }
        }
        else
        {
            // draw MACH number

            if (m_flightstatus->mach >= 0.5)
            {
                qglColor(Qt::green);
                QString mach_string = QString(".%1").arg((((int)(m_flightstatus->mach * 1000)) % 1000));
                drawText(m_speed_band_horiz_offset, size().height() - m_font_height_double, mach_string);
            }
          
            // draw ILS warning

            if (m_flightstatus->ap_app_lock)
            {
                qglColor(AMBER);

                if (m_ils_warning_blink_timer.elapsed() > 500)
                {
                    m_show_ils_warning = !m_show_ils_warning;
                    m_ils_warning_blink_timer.start();
                }

                if (m_show_ils_warning)
                    drawText(m_horizon_half_width - getTextWidth("ILS"), 
                             m_horizon_offset.y() + m_hdg_band_vert_offset - 2*m_font_height, "ILS");
            }
            else
            {
                m_show_ils_warning = false;
            }
        }

        // draw QNH

        qglColor(Qt::cyan);

        (m_fmc_control->flightStatusChecker().hasAltimeterWrongSetting(m_left_side)) ?
            enableAltimeterBlink() : disableAltimeterBlink();

        double qnh_x = m_alt_band_horiz_offset;

        if (m_show_altimeter_setting)
        {
            double qnh_y = size().height() - m_font_height_double;
        
            if (m_fmc_control->isAltimeterSetToSTD(m_left_side))
            {
                double text_width = getTextWidth("STD");
            
                qglColor(Qt::yellow);
                glLineWidth(1.3);
                glBegin(GL_LINE_LOOP);
                glVertex2d(qnh_x-2, qnh_y);
                glVertex2d(qnh_x+2+text_width, qnh_y);
                glVertex2d(qnh_x+2+text_width, qnh_y-m_font_height);
                glVertex2d(qnh_x-2, qnh_y-m_font_height);
                glEnd();

                qglColor(Qt::cyan);
                drawText(qnh_x, qnh_y, "STD");
            }
            else
            {
                if (m_flightstatus->AltPressureSettingHpa() == 0.0)
                {
                    drawText(qnh_x, qnh_y, "- - - -");
                }
                else
                {
                    if (m_fmc_control->showAltHpa(m_left_side))
                    {
                        drawText(qnh_x, qnh_y, QString("%1").arg(m_flightstatus->AltPressureSettingHpa(), 0, 'f', 0));
                        drawText(qnh_x, qnh_y + m_font_height, " QNH");
                    }
                    else
                    {
                        drawText(qnh_x, qnh_y, QString("%1").arg(
                                     Navcalc::getInchesFromHpa(m_flightstatus->AltPressureSettingHpa()), 0, 'f', 2));
                        drawText(qnh_x, qnh_y + m_font_height, " ALT");
                    }
                }
            }
        }

        // draw marker (outer, middle, inner)

        if (m_flightstatus->radarAltitude() < 2500)
        {
            if (m_flightstatus->outer_marker)
            {
                qglColor(BLUE);
                drawText(m_horizon_half_width - getTextWidth("OM"), 
                         m_horizon_offset.y() + m_hdg_band_vert_offset - 3.0*m_font_height, "OM");
            }
            else if (m_flightstatus->middle_marker)
            {
                qglColor(AMBER);
                drawText(m_horizon_half_width - getTextWidth("MM"), 
                         m_horizon_offset.y() + m_hdg_band_vert_offset - 3.0*m_font_height, "MM");
            }
            else if (m_flightstatus->inner_marker)
            {
                qglColor(WHITE);
                drawText(m_horizon_half_width - getTextWidth("AWY"), 
                         m_horizon_offset.y() + m_hdg_band_vert_offset - 3.0*m_font_height, "AWY");
            }
        }

        // draw AP mode texts

        glLoadIdentity();
        double item_line_y_offset = 5;
        double item_height_2 = m_font_height*2.0;
        double item_height_3 = m_font_height*3.0;

        double line1_x = m_horizon_offset.x() - size().width() * 1.5 / 6.6;
        double line2_x = m_horizon_offset.x() - size().width() * 0.15 / 6.6;
        double line3_x = m_horizon_offset.x() + size().width() * 1.2 / 6.6;
        double line4_x = m_horizon_offset.x() + size().width() * 2.6 / 6.6;
        int fma_text_offset = 5;

        bool common_lateral_vertical_mode = false;
        if ((m_fmc_control->fmcAutoPilot().lateralModeActive() == FMCAutopilot::LATERAL_MODE_LANDING &&
             m_fmc_control->fmcAutoPilot().verticalModeActive() == FMCAutopilot::VERTICAL_MODE_LANDING)
            ||
            (m_fmc_control->fmcAutoPilot().lateralModeActive() == FMCAutopilot::LATERAL_MODE_FLARE &&
             m_fmc_control->fmcAutoPilot().verticalModeActive() == FMCAutopilot::VERTICAL_MODE_FLARE)
            ||
            (m_fmc_control->fmcAutoPilot().lateralModeActive() == FMCAutopilot::LATERAL_MODE_ROLLOUT &&
             m_fmc_control->fmcAutoPilot().verticalModeActive() == FMCAutopilot::VERTICAL_MODE_ROLLOUT))
        {
            common_lateral_vertical_mode = true;
        }

        qglColor(Qt::white);
        glLineWidth(1.5);

        QString fma_extra_text;
        if (m_fmc_control->fmcAutothrottle().isMoreDragNecessary())
            fma_extra_text = "MORE DRAG";

//         //TODO
//         if (m_fmc_control->pitchController()->isStable())
//             fma_extra_text = "PITCH STABLE";

        glBegin(GL_LINES);
        glVertex2d(line1_x, item_line_y_offset);
        glVertex2d(line1_x, item_line_y_offset+item_height_3);

        if (!common_lateral_vertical_mode)
        {
            glVertex2d(line2_x, item_line_y_offset);

            if (fma_extra_text.isEmpty())
                glVertex2d(line2_x, item_line_y_offset+item_height_3);
            else
                glVertex2d(line2_x, item_line_y_offset+item_height_2);
        }

        glVertex2d(line3_x, item_line_y_offset);
        glVertex2d(line3_x, item_line_y_offset+item_height_3);
        glVertex2d(line4_x, item_line_y_offset);
        glVertex2d(line4_x, item_line_y_offset+item_height_3);
        glEnd();

        // draw more drag

        if (!fma_extra_text.isEmpty())
        {
            drawText(line1_x +((line3_x - line1_x)*0.5) - (getTextWidth(fma_extra_text) * 0.5),
                     item_line_y_offset+ (3.0*m_font_height), 
                     fma_extra_text);
        }

        qglColor(Qt::green);

        double x = 0.0;
        QString fma_text;

        // draw speed FMA

        // draw 1st line

        FMCAutothrottle::SPEED_MODE speed_mode = m_fmc_control->fmcAutothrottle().speedModeActive();
        bool fma_athr_box_double_height = false;

        if (speed_mode != FMCAutothrottle::SPEED_MODE_NONE)
        {
            if (speed_mode == FMCAutothrottle::SPEED_MODE_TOGA ||
                speed_mode == FMCAutothrottle::SPEED_MODE_FLEX)
                qglColor(Qt::white);
            else
                qglColor(Qt::green);

            QString text = speedFMAText(m_fmc_control->fmcAutothrottle().speedModeActive());

            if (text.contains("\n"))
            {
                QStringList textlist = text.split("\n");
                drawText(3, item_line_y_offset+m_font_height, textlist[0]);
                drawText(3, item_line_y_offset+2*m_font_height, textlist[1]);
                fma_athr_box_double_height = true;
            }
            else
            {
                x = (line1_x * 0.5) - (getTextWidth(text) * 0.5);
                drawText(x, item_line_y_offset+m_font_height, text);
            }
        }

        //draw 2nd line

        if (m_fmc_control->fmcAutothrottle().speedModeArmed() != FMCAutothrottle::SPEED_MODE_NONE)
        {
            qglColor(Qt::cyan);
            drawText(3, item_line_y_offset+m_font_height_double, 
                     speedFMAText(m_fmc_control->fmcAutothrottle().speedModeArmed()));
        }

        //draw 3rd line

        if (m_fmc_control->flightStatusChecker().thrustLeverClimbDetentRequest())
        {
            if (m_thr_lvr_clb_blink_timer.elapsed() > 500)
            {
                m_show_thr_lvr_clb = !m_show_thr_lvr_clb;
                m_thr_lvr_clb_blink_timer.start();
            }

            if (m_show_thr_lvr_clb)
            {
                qglColor(WHITE);
                drawText(3, item_line_y_offset+m_font_height_double+m_font_height, "LVR CLB");
            }
        }
        else
        {
            m_show_thr_lvr_clb = false;
        }

        glLineWidth(1.0);
        qglColor(Qt::white);        

        if (m_fmc_control->fmcAutothrottle().speedModeActiveChangeTimeMs() < 10000)
        {
            double y_add = m_font_height;
            if (fma_athr_box_double_height) y_add += m_font_height;

            glBegin(GL_LINE_LOOP);
            glVertex2d(1, item_line_y_offset);
            glVertex2d(line1_x-FMA_QUAD_SPACING, item_line_y_offset);
            glVertex2d(line1_x-FMA_QUAD_SPACING, item_line_y_offset+y_add-1);
            glVertex2d(1, item_line_y_offset+y_add-1);
            glEnd();
        }
        
        if (!common_lateral_vertical_mode)
        {
            // draw vertical FMA

            if (m_fmc_control->fmcAutoPilot().verticalModeActive() == FMCAutopilot::VERTICAL_MODE_VS)
            {
                qglColor(Qt::green);
                QString vs_text = verticalFMAText(m_fmc_control->fmcAutoPilot().verticalModeActive());
                drawText(line1_x+fma_text_offset, item_line_y_offset+m_font_height, vs_text);

                qglColor(Qt::cyan);
                if (m_flightstatus->APVs() == 0)
                    drawText(line1_x+fma_text_offset+getTextWidth(vs_text), item_line_y_offset+m_font_height, "=0");
                if (m_flightstatus->APVs() >= 0)
                    drawText(line1_x+fma_text_offset+getTextWidth(vs_text), item_line_y_offset+m_font_height, 
                             QString("+%1").arg(m_flightstatus->APVs()/100, 2, 10, QChar('0')));
                else
                    drawText(line1_x+fma_text_offset+getTextWidth(vs_text), item_line_y_offset+m_font_height, 
                             QString("%1").arg(m_flightstatus->APVs()/100, 2, 10, QChar('0')));
            }
//         if (m_fmc_control->fmcAutoPilot().verticalModeActive() == FMCAutopilot::VERTICAL_MODE_FPV)
//         {
//             qglColor(Qt::green);
//             QString vs_text = verticalFMAText(m_fmc_control->fmcAutoPilot().verticalModeActive());
//             drawText(line1_x+fma_text_offset, item_line_y_offset+m_font_height, vs_text);

//             qglColor(Qt::cyan);
//             if (m_fmc_control->fmcAutoPilot().flightPath() == 0)
//                 drawText(line1_x+fma_text_offset+getTextWidth(vs_text), item_line_y_offset+m_font_height, "=0.0");
//             if (m_fmc_control->fmcAutoPilot().flightPath() >= 0)
//                 drawText(line1_x+fma_text_offset+getTextWidth(vs_text), item_line_y_offset+m_font_height, 
//                          QString("+%1").arg(m_fmc_control->fmcAutoPilot().flightPath(), 0, 'f', 1));
//             else
//                 drawText(line1_x+fma_text_offset+getTextWidth(vs_text), item_line_y_offset+m_font_height, 
//                          QString("%1").arg(m_fmc_control->fmcAutoPilot().flightPath(), 0, 'f', 1));
//         }
            else if (m_fmc_control->fmcAutoPilot().verticalModeActive() != FMCAutopilot::VERTICAL_MODE_NONE)
            {
                qglColor(Qt::green);
                fma_text = verticalFMAText(m_fmc_control->fmcAutoPilot().verticalModeActive());
                x = line1_x + ((line2_x - line1_x) * 0.5) - (getTextWidth(fma_text) * 0.5);
                drawText(x, item_line_y_offset+m_font_height, fma_text);
            }

            if (m_fmc_control->fmcAutoPilot().verticalModeArmed() != FMCAutopilot::VERTICAL_MODE_NONE)
            {
                qglColor(Qt::cyan);
                drawText(line1_x+fma_text_offset, item_line_y_offset+m_font_height_double, 
                         verticalFMAText(m_fmc_control->fmcAutoPilot().verticalModeArmed()));
            }

            glLineWidth(1.0);
            qglColor(Qt::white);        

            if (m_fmc_control->fmcAutoPilot().verticalModeActiveChangeTimeMs() < 10000)
            {
                glBegin(GL_LINE_LOOP);
                glVertex2d(line1_x+FMA_QUAD_SPACING, item_line_y_offset);
                glVertex2d(line2_x-FMA_QUAD_SPACING, item_line_y_offset);
                glVertex2d(line2_x-FMA_QUAD_SPACING, item_line_y_offset+m_font_height-1);
                glVertex2d(line1_x+FMA_QUAD_SPACING, item_line_y_offset+m_font_height-1);
                glEnd();
            }

            // draw lateral FMA

            if (m_fmc_control->fmcAutoPilot().lateralModeActive() != FMCAutopilot::LATERAL_MODE_NONE)
            {
                qglColor(Qt::green);
                fma_text = lateralFMAText(m_fmc_control->fmcAutoPilot().lateralModeActive());
                x = line2_x + ((line3_x - line2_x) * 0.5) - (getTextWidth(fma_text) * 0.5);
                drawText(x, item_line_y_offset+m_font_height, fma_text);
            }

            if (m_fmc_control->fmcAutoPilot().lateralModeArmed() != FMCAutopilot::LATERAL_MODE_NONE)
            {
                qglColor(Qt::cyan);
                drawText(line2_x+fma_text_offset, item_line_y_offset+m_font_height_double, 
                         lateralFMAText(m_fmc_control->fmcAutoPilot().lateralModeArmed()));
            }

            glLineWidth(1.0);
            qglColor(Qt::white);        
            
            if (m_fmc_control->fmcAutoPilot().lateralModeActiveChangeTimeMs() < 10000)
            {
                glBegin(GL_LINE_LOOP);
                glVertex2d(line2_x+FMA_QUAD_SPACING, item_line_y_offset);
                glVertex2d(line3_x-FMA_QUAD_SPACING, item_line_y_offset);
                glVertex2d(line3_x-FMA_QUAD_SPACING, item_line_y_offset+m_font_height-1);
                glVertex2d(line2_x+FMA_QUAD_SPACING, item_line_y_offset+m_font_height-1);
                glEnd();
            }
        }
        else
        {
            qglColor(Qt::green);

            QString text = lateralFMAText(m_fmc_control->fmcAutoPilot().lateralModeActive());
            drawText(line1_x + 0.5*(line3_x - line1_x) - 0.5*getTextWidth(text), item_line_y_offset+m_font_height, text);

            glLineWidth(1.0);
            qglColor(Qt::white);        

            if (m_fmc_control->fmcAutoPilot().lateralModeActiveChangeTimeMs() < 10000)
            {
                glBegin(GL_LINE_LOOP);
                glVertex2d(line1_x+FMA_QUAD_SPACING, item_line_y_offset);
                glVertex2d(line3_x-FMA_QUAD_SPACING, item_line_y_offset);
                glVertex2d(line3_x-FMA_QUAD_SPACING, item_line_y_offset+m_font_height-1);
                glVertex2d(line1_x+FMA_QUAD_SPACING, item_line_y_offset+m_font_height-1);
                glEnd();
            }
        }

        // draw decision height / MDA

        qglColor(Qt::white);

        if (m_fmc_control->flightModeTracker().isDescent() || m_fmc_control->flightModeTracker().isApproach())
        {
            if (m_fmc_data.decisionHeightFt() > 0)
            {
                drawText(line3_x+fma_text_offset, item_line_y_offset+m_font_height*3, "DH");
                qglColor(Qt::cyan);
                drawText(line3_x+fma_text_offset+getTextWidth("DH "), item_line_y_offset+m_font_height*3, 
                         QString::number(m_fmc_data.decisionHeightFt()));
            }
            else if (m_fmc_data.minDescentAltitudeFt() > 0)
            {
                drawText(line3_x+fma_text_offset, item_line_y_offset+m_font_height*3, "MDA");
                qglColor(Qt::cyan);
                drawText(line3_x+fma_text_offset+getTextWidth("MDA "), item_line_y_offset+m_font_height*3, 
                         QString::number(m_fmc_data.minDescentAltitudeFt()));
            }
        }

        qglColor(Qt::white);

        // draw approach capabilities

        if (m_fmc_control->fmcAutoPilot().isAPPHoldActive())
        {
            switch(m_fmc_control->fmcAutoPilot().ilsMode())
            {
                case(FMCAutopilot::ILS_MODE_CAT1):
                    drawText(line3_x+fma_text_offset, item_line_y_offset+m_font_height, "CAT 1");
                    break;

                case(FMCAutopilot::ILS_MODE_CAT2):
                    drawText(line3_x+fma_text_offset, item_line_y_offset+m_font_height, "CAT 2");
                    break;
                    
                default:
                    break;
            }
        }

        // draw AP FMA

        //TODO draw white change rectangles!

        if (m_flightstatus->ap_enabled)
            drawText(line4_x+fma_text_offset, item_line_y_offset+m_font_height, "AP1");

        if (m_flightstatus->fd_active)
            drawText(line4_x+fma_text_offset, item_line_y_offset+m_font_height_double, "1FD2");

        if (m_fmc_control->fmcAutothrottle().isAPThrottleArmed())
        {
            if (m_fmc_control->fmcAutothrottle().isAPThrottleEngaged() &&
                m_fmc_control->fmcAutothrottle().speedModeActive() != FMCAutothrottle::SPEED_MODE_TOGA &&
                m_fmc_control->fmcAutothrottle().speedModeActive() != FMCAutothrottle::SPEED_MODE_FLEX)
                qglColor(Qt::white);
            else
                qglColor(Qt::cyan);

            drawText(line4_x+fma_text_offset, item_line_y_offset+m_font_height*3, "A/THR");
        }
    }
}    

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleA::enableClipping(GLuint gllist_id)
{
    vasglBeginClipRegion(size());

    // draw clip poly for regions that should be displayed
    glCallList(gllist_id);

    vasglEndClipRegion();
}

/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleA::disableClipping()
{
    vasglDisableClipping();
}
/////////////////////////////////////////////////////////////////////////////

void GLPFDWidgetStyleA::drawFPS()
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
    m_font->drawText(2.0, size().height()-m_font->getHeight(), QString("%1fps").arg(m_fps));
}

/////////////////////////////////////////////////////////////////////////////

QString GLPFDWidgetStyleA::speedFMAText(FMCAutothrottle::SPEED_MODE mode)
{
    switch(mode)
    {
        case(FMCAutothrottle::SPEED_MODE_NONE):   return QString::null;
        case(FMCAutothrottle::SPEED_MODE_SPEED):  return "SPEED";
        case(FMCAutothrottle::SPEED_MODE_MACH):   return "MACH";
        case(FMCAutothrottle::SPEED_MODE_TOGA):   return "MAN\nTOGA";
        case(FMCAutothrottle::SPEED_MODE_FLEX):   return "MAN\nFLX";
        case(FMCAutothrottle::SPEED_MODE_MCT):   return "THR MCT";
        case(FMCAutothrottle::SPEED_MODE_THRCLB): return "THR CLB";
        case(FMCAutothrottle::SPEED_MODE_THRLVR): return "THR LVR";
        case(FMCAutothrottle::SPEED_MODE_THRIDLE):return "THR IDLE";
    }

    return QString::null;
}

/////////////////////////////////////////////////////////////////////////////

QString GLPFDWidgetStyleA::verticalFMAText(FMCAutopilot::VERTICAL_MODE mode)
{
    switch(mode)
    {
        case(FMCAutopilot::VERTICAL_MODE_NONE): return QString::null;
        case(FMCAutopilot::VERTICAL_MODE_ALT):  return "ALT";
        case(FMCAutopilot::VERTICAL_MODE_ALT_CAPTURE): return "ALT*";
        case(FMCAutopilot::VERTICAL_MODE_ALTCRZ): return "ALT CRZ";
        case(FMCAutopilot::VERTICAL_MODE_GS): return "G/S";
        case(FMCAutopilot::VERTICAL_MODE_GS_CAPTURE): return "G/S*";
        case(FMCAutopilot::VERTICAL_MODE_VS): return "V/S=";
        case(FMCAutopilot::VERTICAL_MODE_FPV): return "FPA";
        //case(FMCAutopilot::VERTICAL_MODE_FPV): return "FPA=";
        case(FMCAutopilot::VERTICAL_MODE_FLCH): 
            return (m_flightstatus->APAlt() - m_flightstatus->smoothed_altimeter_readout.lastValue() > 0) ?
            "OP CLB" : "OP DES";
        case(FMCAutopilot::VERTICAL_MODE_VNAV): 
            return (m_flightstatus->APAlt() - m_flightstatus->smoothed_altimeter_readout.lastValue() > 0) ?
            "CLB" : "DES";
        case(FMCAutopilot::VERTICAL_MODE_TAKEOFF): return "SRS";
        case(FMCAutopilot::VERTICAL_MODE_LANDING): return "LAND";
        case(FMCAutopilot::VERTICAL_MODE_FLARE): return "FLARE";
        case(FMCAutopilot::VERTICAL_MODE_ROLLOUT): return "ROLLOUT";
    }

    return QString::null;
}

/////////////////////////////////////////////////////////////////////////////

QString GLPFDWidgetStyleA::lateralFMAText(FMCAutopilot::LATERAL_MODE mode)
{
    switch(mode)
    {
        case(FMCAutopilot::LATERAL_MODE_NONE): return QString::null;
        case(FMCAutopilot::LATERAL_MODE_HDG):  return "HDG";
        case(FMCAutopilot::LATERAL_MODE_LNAV):  return "NAV";
        case(FMCAutopilot::LATERAL_MODE_LOC):  return "LOC";
        case(FMCAutopilot::LATERAL_MODE_LOC_CAPTURE):  return "LOC*";
        case(FMCAutopilot::LATERAL_MODE_VOR):  return "VOR";
        case(FMCAutopilot::LATERAL_MODE_TAKEOFF): return (m_flightstatus->radarAltitude() < 10) ? "RWY" : "RWYTRK";
        case(FMCAutopilot::LATERAL_MODE_LANDING): return "LAND";
        case(FMCAutopilot::LATERAL_MODE_FLARE): return "FLARE";
        case(FMCAutopilot::LATERAL_MODE_ROLLOUT): return "ROLLOUT";
    }

    return QString::null;
}

