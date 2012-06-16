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
#include "vas_path.h"
#include "aircraft_data.h"

#include "gldraw.h"
#include "fmc_control.h"
#include "fmc_data.h"
#include "fmc_ecam.h"
#include "fmc_autothrottle.h"
#include "fmc_flightstatus_checker_base.h"

#include "fmc_ecam_glwidget_style_a.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

#define POINTER_OVERLAP_LEN 4

/////////////////////////////////////////////////////////////////////////////

GLECAMWidgetStyleA::GLECAMWidgetStyleA(bool upper_ecam, 
                                       ConfigWidgetProvider* config_widget_provider,
                                       Config* main_config,
                                       Config* ecam_config, 
                                       FMCControl* fmc_control,
                                       VasWidget *parent) : 
    GLECAMWidgetBase(upper_ecam, config_widget_provider, main_config, ecam_config, fmc_control, parent),
    m_font(0), m_recalc_ecam(true), m_fps_count(0), m_onground(false), m_at_was_connected(false),
    m_takeoff_inhibit(false), m_landing_inhibit(false)
{
    MYASSERT(m_config_widget_provider != 0);
    MYASSERT(m_main_config != 0);
    MYASSERT(m_ecam_config != 0);
    MYASSERT(m_fmc_control != 0);
    MYASSERT(m_flightstatus != 0);

	m_fps_counter_time.start();
}

/////////////////////////////////////////////////////////////////////////////

GLECAMWidgetStyleA::~GLECAMWidgetStyleA() 
{
}

/////////////////////////////////////////////////////////////////////////////

void GLECAMWidgetStyleA::initializeGL()
{
    m_recalc_ecam = true;

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

    m_font->loadFont(
        VasPath::prependPath(m_main_config->getValue(CFG_FONT_NAME)), m_fmc_control->glFontSize());

    m_font_height = m_font->getHeight();
}

/////////////////////////////////////////////////////////////////////////////

void GLECAMWidgetStyleA::resizeGL(int width, int height)
{
    m_recalc_ecam = true;
    m_size = QSize(width, height);

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, width, height, 0.0, 1.0, -1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/////////////////////////////////////////////////////////////////////////////

void GLECAMWidgetStyleA::refreshECAM()
{
    if (!isVisible()) return;
    updateGL();
};

/////////////////////////////////////////////////////////////////////////////

void GLECAMWidgetStyleA::paintGL()
{
    if (!isVisible()) return;

    setupStateBeforeDraw();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_flightstatus->isValid() && !m_flightstatus->battery_on) return;

    drawDisplay();
    if (m_fmc_control->showFps()) drawFPS();
    glFlush();

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) Logger::log(QString("GLECAMWidgetStyleA:paintGL: GL Error: %1").arg(error));
}

/////////////////////////////////////////////////////////////////////////////

void GLECAMWidgetStyleA::setupStateBeforeDraw()
{
    if (m_recalc_ecam)
    {
        m_recalc_ecam = false;

        //----- separators

        double spacer = m_size.width() * 0.03448;

        m_separator_vertical_x = m_size.width() * 0.603448;
        m_separator_horizontal_y = m_size.height() * 0.60345;

        m_separator_horizontal_left_x = m_size.width() * 0.06034;
        m_separator_horizontal_right_x = m_separator_vertical_x + spacer;
       
        m_separator_vertical_upper_y = m_size.height() * 0.28448;
        m_separator_vertical_lower_y = m_separator_horizontal_y + spacer;

        m_separator_left_len = m_separator_vertical_x - spacer - m_separator_horizontal_left_x; 
        m_separator_right_len = m_size.width() * 0.25862;
        m_separator_upper_len = m_separator_horizontal_y - spacer - m_separator_vertical_upper_y;
        m_separator_lower_len = m_size.height() * 0.22413;

        //----- engines

        double eng_x1 = m_size.width() * 0.18965;
        double eng_x2 = m_size.width() * 0.49137;
        double eng_y1 = m_size.width() * 0.15;
        double eng_y2 = m_size.width() * 0.36207 - 0.5 * m_font_height;

        m_engine1_center_n1 = QPointF(eng_x1, eng_y1);
        m_engine2_center_n1 = QPointF(eng_x2, eng_y1);
        m_engine_n1_radius = m_size.width() * 0.09483;

        m_engine1_center_egt = QPointF(eng_x1, eng_y2);
        m_engine2_center_egt = QPointF(eng_x2, eng_y2);
        m_engine_egt_radius = m_size.width() * 0.08621;

        m_engine_n1_angle_left = Navcalc::toRad(-120.0);
        m_engine_n1_angle_right = Navcalc::toRad(70.0);
        m_n1_angle_range = qAbs(m_engine_n1_angle_right - m_engine_n1_angle_left);

        m_engine_egt_angle_left = Navcalc::toRad(-120.0);
        m_engine_egt_angle_right = Navcalc::toRad(60.0);
        m_egt_angle_range = qAbs(m_engine_egt_angle_right - m_engine_egt_angle_left);

        m_engine1_center_n2 = QPointF(eng_x1, m_size.height() * 0.45689 + 0.5*m_font_height);
        m_engine2_center_n2 = QPointF(eng_x2, m_size.height() * 0.45689 + 0.5*m_font_height);

        m_engine1_center_ff = QPointF(eng_x1, m_size.height() * 0.56034 + 0.5*m_font_height);
        m_engine2_center_ff = QPointF(eng_x2, m_size.height() * 0.56034 + 0.5*m_font_height);

        //----- texts

        m_left_text_ref = QPointF(m_separator_horizontal_left_x * 1.5, 
                                  m_separator_horizontal_y + 0.3 * spacer + m_font_height);
        m_right_text_ref = QPointF(m_separator_horizontal_right_x - spacer*0.3, 
                                   m_separator_horizontal_y + 0.3 * spacer + m_font_height);

        //----- flaps

        m_flap_text_spacer = 2* (m_right_text_ref.x() - m_separator_vertical_x);
        m_flap_text_x_left = m_right_text_ref.x();
        m_flap_text_x_right = m_size.width() - 2*spacer;
        m_flap_text_middle = m_flap_text_x_left + ((m_flap_text_x_right - m_flap_text_x_left) * 0.5);
        m_flap_text_y = m_engine2_center_egt.y() + 2*m_font_height;

        m_slat_x = m_flap_text_x_left + getTextWidth("X") * 3;
        m_slat_y = m_flap_text_y + m_font_height;
        m_slat_x_inc = -getTextWidth("XXX") / 3.0;
        m_slat_y_inc = (m_separator_horizontal_y - m_flap_text_y) / 4.5;
        
        m_flap_x = m_flap_text_x_right - getTextWidth("XX");
        m_flap_y = m_flap_text_y + m_font_height;
        m_flap_x_inc = getTextWidth("XXXX") / 4.0;
        m_flap_y_inc = m_slat_y_inc * 0.8;

        m_slat_points.clear();
        m_slat_points.append(QPointF(m_slat_x, m_slat_y+0.2*m_slat_y_inc));
        m_slat_points.append(QPointF(m_slat_x+m_slat_x_inc, m_slat_y+m_slat_y_inc));
        m_slat_points.append(QPointF(m_slat_x+1.5*m_slat_x_inc, m_slat_y+2*m_slat_y_inc));

        m_flap_points.clear();
        m_flap_points.append(QPointF(m_flap_x, m_flap_y+0.2*m_flap_y_inc));
        m_flap_points.append(QPointF(m_flap_x+1.5*m_flap_x_inc, m_flap_y+m_flap_y_inc));
        m_flap_points.append(QPointF(m_flap_x+2*m_flap_x_inc, m_flap_y+2*m_flap_y_inc));
        m_flap_points.append(QPointF(m_flap_x+2.4*m_flap_x_inc, m_flap_y+3*m_flap_y_inc));
    }
};

/////////////////////////////////////////////////////////////////////////////

void GLECAMWidgetStyleA::drawDisplay()
{
    glLoadIdentity();

    drawSeparators();
    drawEnginges();
    drawTexts();
};

/////////////////////////////////////////////////////////////////////////////

void GLECAMWidgetStyleA::drawSeparators()
{
    qglColor(Qt::white);
    glLineWidth(3.0);

    glBegin(GL_LINES);
    // left sep
    glVertex2d(m_separator_horizontal_left_x, m_separator_horizontal_y);
    glVertex2d(m_separator_horizontal_left_x + m_separator_left_len, m_separator_horizontal_y);
    // right sep
    glVertex2d(m_separator_horizontal_right_x, m_separator_horizontal_y);
    glVertex2d(m_separator_horizontal_right_x + m_separator_right_len, m_separator_horizontal_y);
    // upper sep
    glVertex2d(m_separator_vertical_x, m_separator_vertical_upper_y);
    glVertex2d(m_separator_vertical_x, m_separator_vertical_upper_y + m_separator_upper_len);
    // lower sep
    glVertex2d(m_separator_vertical_x, m_separator_vertical_lower_y);
    glVertex2d(m_separator_vertical_x, m_separator_vertical_lower_y + m_separator_lower_len);

    glEnd();
}

/////////////////////////////////////////////////////////////////////////////

void GLECAMWidgetStyleA::drawEnginges()
{
    glPushMatrix();

    double max_red_n1 = 100.0;

    double n1_1 = m_flightstatus->engine_data[1].smoothedN1();
    double n1_2 = m_flightstatus->engine_data[2].smoothedN1();

    bool left_red = n1_1 > max_red_n1;
    bool left_out = m_flightstatus->engine_data[1].n2_percent < 2;
    bool right_red = n1_2 > max_red_n1;
    bool right_out = m_flightstatus->engine_data[2].n2_percent < 2;

    //----- circles

    glLineWidth(1.5);

    // left n1 + egt

    if (left_out) qglColor(ORANGE);
    else qglColor(WHITE);

    GLDraw::drawCircleOffset(m_engine_n1_radius, m_engine_n1_angle_left, m_engine_n1_angle_right, 
                             m_engine1_center_n1.x(), m_engine1_center_n1.y());
    GLDraw::drawCircleOffset(m_engine_egt_radius, m_engine_egt_angle_left, m_engine_egt_angle_right, 
                             m_engine1_center_egt.x(), m_engine1_center_egt.y());

    // right n1 + egt

    if (right_out) qglColor(ORANGE);
    else qglColor(WHITE);

    GLDraw::drawCircleOffset(m_engine_n1_radius, m_engine_n1_angle_left, m_engine_n1_angle_right, 
                             m_engine2_center_n1.x(), m_engine2_center_n1.y());
    GLDraw::drawCircleOffset(m_engine_egt_radius, m_engine_egt_angle_left, m_engine_egt_angle_right, 
                             m_engine2_center_egt.x(), m_engine2_center_egt.y());

    // n1 red area

    qglColor(RED);
    
    glLineWidth(3.0);
    double radius = m_engine_n1_radius - 2.0;

    GLDraw::drawCircleOffset(radius, getN1Angle(max_red_n1),
                             m_engine_n1_angle_right, m_engine1_center_n1.x(), m_engine1_center_n1.y());
    GLDraw::drawCircleOffset(radius, getN1Angle(max_red_n1),
                             m_engine_n1_angle_right, m_engine2_center_n1.x(), m_engine2_center_n1.y());
        
    // egt red area

    radius = m_engine_egt_radius - 2.0;
    GLDraw::drawCircleOffset(radius, 0.95 * m_egt_angle_range + m_engine_egt_angle_left,
                             m_engine_egt_angle_right, m_engine1_center_egt.x(), m_engine1_center_egt.y());
    GLDraw::drawCircleOffset(radius, 0.95 * m_egt_angle_range + m_engine_egt_angle_left,
                             m_engine_egt_angle_right, m_engine2_center_egt.x(), m_engine2_center_egt.y());

    //----- n1 markers

    qglColor(WHITE);

    glLineWidth(1.5);

    double inc = m_n1_angle_range / m_fmc_control->aircraftData().maxN1() * 10.0;
    double n1_angle = m_engine_n1_angle_left;

    double n1_font_radius = m_engine_n1_radius-POINTER_OVERLAP_LEN-m_font_height;

    for (int n1_marker = 0; n1_marker <= (int)(m_fmc_control->aircraftData().maxN1() / 10);)
    {
        if (!left_out)
        {
            glBegin(GL_LINES);
            glVertex2d(m_engine1_center_n1.x() + m_engine_n1_radius * sin(n1_angle), 
                       m_engine1_center_n1.y() + m_engine_n1_radius * -cos(n1_angle));
            glVertex2d(m_engine1_center_n1.x() + (m_engine_n1_radius-POINTER_OVERLAP_LEN) * sin(n1_angle), 
                       m_engine1_center_n1.y() + (m_engine_n1_radius-POINTER_OVERLAP_LEN) * -cos(n1_angle));
            glEnd();

            if (n1_marker == 5)
            {
                drawText(m_engine1_center_n1.x() + n1_font_radius * 
                         sin(n1_angle) - getTextWidth("1"), 
                         m_engine1_center_n1.y() + n1_font_radius * -cos(n1_angle), 
                         "5");
            }
            else if (n1_marker == 10)
            {
                drawText(m_engine1_center_n1.x() + n1_font_radius * sin(n1_angle), 
                         m_engine1_center_n1.y() + (n1_font_radius * -cos(n1_angle)) +
                         (m_font_height/2.0), "10");
            }
        }

        if (!right_out)
        {
            glBegin(GL_LINES);
            glVertex2d(m_engine2_center_n1.x() + m_engine_n1_radius * sin(n1_angle), 
                       m_engine2_center_n1.y() + m_engine_n1_radius * -cos(n1_angle));
            glVertex2d(m_engine2_center_n1.x() + (m_engine_n1_radius-POINTER_OVERLAP_LEN) * sin(n1_angle), 
                       m_engine2_center_n1.y() + (m_engine_n1_radius-POINTER_OVERLAP_LEN) * -cos(n1_angle));
            glEnd();

            if (n1_marker == 5)
            {
                drawText(m_engine2_center_n1.x() + n1_font_radius * sin(n1_angle) - getTextWidth("1"), 
                         m_engine2_center_n1.y() + n1_font_radius * -cos(n1_angle), 
                         "5");
            }
            else if (n1_marker == 10) 
            {
                drawText(m_engine2_center_n1.x() + n1_font_radius * sin(n1_angle), 
                         m_engine2_center_n1.y() + (n1_font_radius * -cos(n1_angle)) + 
                         (m_font_height/2.0), "10");
            }
        }
        
        if (n1_marker == 0)
        {
            n1_marker = 5;
            n1_angle += 5 * inc;
            continue;
        }

        ++n1_marker;
        n1_angle += inc;
    }

    //----- pointer + text

    glLineWidth(2.0);

    // left n1 + egt + n2 + ff

    if (left_out) qglColor(ORANGE);
    else if (left_red) qglColor(RED);
    else qglColor(GREEN);

    double n1_1_angle = getN1Angle(n1_1);

    if (!left_out)
    {
        double egt_1_angle = m_flightstatus->engine_data[1].egt_degrees / 1000.0 * m_egt_angle_range + m_engine_egt_angle_left;
        
        glBegin(GL_LINES);
        glVertex2d(m_engine1_center_n1.x(), m_engine1_center_n1.y());
        glVertex2d(m_engine1_center_n1.x() + (m_engine_n1_radius+POINTER_OVERLAP_LEN) * sin(n1_1_angle), 
                   m_engine1_center_n1.y() + (m_engine_n1_radius+POINTER_OVERLAP_LEN) * -cos(n1_1_angle));
        glEnd();

        qglColor(GREEN);
        
        glBegin(GL_LINES);
        glVertex2d(m_engine1_center_egt.x(), m_engine1_center_egt.y());
        glVertex2d(m_engine1_center_egt.x() + (m_engine_egt_radius+POINTER_OVERLAP_LEN) * sin(egt_1_angle), 
                   m_engine1_center_egt.y() + (m_engine_egt_radius+POINTER_OVERLAP_LEN) * -cos(egt_1_angle));
        glEnd();
    }

    double n1_rect_width = getTextWidth("WW.X") * 0.5;

    qglColor(BLACK);
    glBegin(GL_QUADS);
    glVertex2d(m_engine1_center_n1.x() - n1_rect_width, m_engine1_center_n1.y());
    glVertex2d(m_engine1_center_n1.x() + n1_rect_width, m_engine1_center_n1.y());
    glVertex2d(m_engine1_center_n1.x() + n1_rect_width, m_engine1_center_n1.y()+m_font_height);
    glVertex2d(m_engine1_center_n1.x() - n1_rect_width, m_engine1_center_n1.y()+m_font_height);
    glEnd();

    if (left_out) qglColor(ORANGE);
    else qglColor(GREEN);

    glBegin(GL_LINE_LOOP);
    glVertex2d(m_engine1_center_n1.x() - n1_rect_width, m_engine1_center_n1.y());
    glVertex2d(m_engine1_center_n1.x() + n1_rect_width, m_engine1_center_n1.y());
    glVertex2d(m_engine1_center_n1.x() + n1_rect_width, m_engine1_center_n1.y()+m_font_height);
    glVertex2d(m_engine1_center_n1.x() - n1_rect_width, m_engine1_center_n1.y()+m_font_height);
    glEnd();

    if (m_flightstatus->engine_data[1].reverser_percent >= 0.001)
    {
        double rev_width = getTextWidth("REV") + 2;
        double x_ref = m_engine1_center_n1.x() - n1_rect_width;

        qglColor(BLACK);
        glBegin(GL_QUADS);
        glVertex2d(x_ref, m_engine1_center_n1.y() - m_font_height);
        glVertex2d(x_ref + rev_width, m_engine1_center_n1.y() - m_font_height);
        glVertex2d(x_ref + rev_width, m_engine1_center_n1.y()-1);
        glVertex2d(x_ref, m_engine1_center_n1.y()-1);
        glEnd();

        if (left_out) qglColor(ORANGE);
        else qglColor(GREEN);
        
        glBegin(GL_LINE_LOOP);
        glVertex2d(x_ref, m_engine1_center_n1.y() - m_font_height);
        glVertex2d(x_ref + rev_width, m_engine1_center_n1.y() - m_font_height);
        glVertex2d(x_ref + rev_width, m_engine1_center_n1.y()-1);
        glVertex2d(x_ref, m_engine1_center_n1.y()-1);
        glEnd();
        
        drawText(x_ref+1, m_engine1_center_n1.y(), "REV");
    }

    QString value = QString::number(n1_1, 'f', 1);
    if (left_out) value = "XX";
    drawText(m_engine1_center_n1.x() - getTextWidth(value)/2.0, 
             m_engine1_center_n1.y() + m_font_height, value);

    value = QString::number(m_flightstatus->engine_data[1].egt_degrees, 'f', 0);
    if (left_out) value = "XX";
    drawText(m_engine1_center_egt.x() - getTextWidth(value)/2.0, 
             m_engine1_center_egt.y() + m_font_height, value);

    value = QString::number(m_flightstatus->engine_data[1].n2_percent, 'f', 1);
    if (left_out) value = "XX";
    drawText(m_engine1_center_n2.x() - getTextWidth(value)/2.0, 
             m_engine1_center_n2.y(), value);

    value = QString::number(((int)m_flightstatus->engine_data[1].ff_kg_per_hour)/10*10);
    if (left_out) value = "XX";
    drawText(m_engine1_center_ff.x() - getTextWidth(value)/2.0, 
             m_engine1_center_ff.y(), value);

    // right n1 + egt + n2 + ff

    if (right_out) qglColor(ORANGE);
    else if (right_red) qglColor(RED);
    else qglColor(GREEN);

    double n1_2_angle = getN1Angle(n1_2);
    
    if (!right_out)
    {
        double egt_2_angle = m_flightstatus->engine_data[2].egt_degrees / 1000.0 * m_egt_angle_range + m_engine_egt_angle_left;
        
        glBegin(GL_LINES);
        glVertex2d(m_engine2_center_n1.x(), m_engine2_center_n1.y());
        glVertex2d(m_engine2_center_n1.x() + (m_engine_n1_radius+POINTER_OVERLAP_LEN) * sin(n1_2_angle), 
                   m_engine2_center_n1.y() + (m_engine_n1_radius+POINTER_OVERLAP_LEN) * -cos(n1_2_angle));
        glEnd();

        qglColor(GREEN);

        glBegin(GL_LINES);
        glVertex2d(m_engine2_center_egt.x(), m_engine2_center_egt.y());
        glVertex2d(m_engine2_center_egt.x() + (m_engine_egt_radius+POINTER_OVERLAP_LEN) * sin(egt_2_angle), 
                   m_engine2_center_egt.y() + (m_engine_egt_radius+POINTER_OVERLAP_LEN) * -cos(egt_2_angle));
        glEnd();
    }

    qglColor(BLACK);
    glBegin(GL_QUADS);
    glVertex2d(m_engine2_center_n1.x() - n1_rect_width, m_engine2_center_n1.y());
    glVertex2d(m_engine2_center_n1.x() + n1_rect_width, m_engine2_center_n1.y());
    glVertex2d(m_engine2_center_n1.x() + n1_rect_width, m_engine2_center_n1.y()+m_font_height);
    glVertex2d(m_engine2_center_n1.x() - n1_rect_width, m_engine2_center_n1.y()+m_font_height);
    glEnd();

    if (right_out) qglColor(ORANGE);
    else qglColor(GREEN);

    glBegin(GL_LINE_LOOP);
    glVertex2d(m_engine2_center_n1.x() - n1_rect_width, m_engine2_center_n1.y());
    glVertex2d(m_engine2_center_n1.x() + n1_rect_width, m_engine2_center_n1.y());
    glVertex2d(m_engine2_center_n1.x() + n1_rect_width, m_engine2_center_n1.y()+m_font_height);
    glVertex2d(m_engine2_center_n1.x() - n1_rect_width, m_engine2_center_n1.y()+m_font_height);
    glEnd();

    if (m_flightstatus->engine_data[2].reverser_percent >= 0.001)
    {
        double rev_width = getTextWidth("REV") + 2;

        double x_ref = m_engine2_center_n1.x() - n1_rect_width;

        qglColor(BLACK);
        glBegin(GL_QUADS);
        glVertex2d(x_ref, m_engine2_center_n1.y() - m_font_height);
        glVertex2d(x_ref + rev_width, m_engine2_center_n1.y() - m_font_height);
        glVertex2d(x_ref + rev_width, m_engine2_center_n1.y()-1);
        glVertex2d(x_ref, m_engine2_center_n1.y()-1);
        glEnd();

        if (right_out) qglColor(ORANGE);
        else qglColor(GREEN);
        
        glBegin(GL_LINE_LOOP);
        glVertex2d(x_ref, m_engine2_center_n1.y() - m_font_height);
        glVertex2d(x_ref + rev_width, m_engine2_center_n1.y() - m_font_height);
        glVertex2d(x_ref + rev_width, m_engine2_center_n1.y()-1);
        glVertex2d(x_ref, m_engine2_center_n1.y()-1);
        glEnd();
        
        drawText(x_ref+1, m_engine2_center_n1.y(), "REV");
    }

    value = QString::number(n1_2, 'f', 1);
    if (right_out) value = "XX";
    drawText(m_engine2_center_n1.x() - getTextWidth(value)/2.0, 
             m_engine2_center_n1.y() + m_font_height, value);
   
    value = QString::number(m_flightstatus->engine_data[2].egt_degrees, 'f', 0);
    if (right_out) value = "XX";
    drawText(m_engine2_center_egt.x() - getTextWidth(value)/2.0, 
             m_engine2_center_egt.y() + m_font_height, value);

    value = QString::number(m_flightstatus->engine_data[2].n2_percent, 'f', 1);
    if (right_out) value = "XX";
    drawText(m_engine2_center_n2.x() - getTextWidth(value)/2.0, 
             m_engine2_center_n2.y(), value);

    value = QString::number(((int)m_flightstatus->engine_data[2].ff_kg_per_hour)/10*10);
    if (right_out) value = "XX";
    drawText(m_engine2_center_ff.x() - getTextWidth(value)/2.0, 
             m_engine2_center_ff.y(), value);

    //----- labels

    double middle = m_engine1_center_n1.x() + ((m_engine2_center_n1.x() - m_engine1_center_n1.x()) / 2.0);

    if (!m_flightstatus->onground && 
        m_flightstatus->engine_data[1].throttle_lever_percent <= 0 && 
        m_flightstatus->engine_data[2].throttle_lever_percent <= 0)
    {
        qglColor(GREEN);
        drawText(middle-getTextWidth("IDLE")/2.0, 1.5*m_font_height, "IDLE");
    }

    qglColor(Qt::white);
    drawText(middle-getTextWidth("N1")/2.0, m_engine1_center_n1.y()+0.5*m_font_height, "N1");
    drawText(middle-getTextWidth("EGT")/2.0, m_engine1_center_egt.y()+0.5*m_font_height, "EGT");
    drawText(middle-getTextWidth("N2")/2.0, m_engine1_center_n2.y()-0.5*m_font_height, "N2");
    drawText(middle-getTextWidth("FF")/2.0, m_engine1_center_ff.y()-0.5*m_font_height, "FF");

    //----- lines

    double xlen = (m_engine2_center_n2.x() - getTextWidth("XXX") - middle);

    glLineWidth(1.5);
    glBegin(GL_LINES);
    // left n2
    glVertex2d(middle-getTextWidth("N2"), m_engine1_center_n2.y()-0.8*m_font_height);
    glVertex2d(middle - xlen, m_engine1_center_n2.y()-0.5*m_font_height);
    // right n2
    glVertex2d(middle+getTextWidth("N2"), m_engine2_center_n2.y()-0.8*m_font_height);
    glVertex2d(middle + xlen, m_engine2_center_n2.y()-0.5*m_font_height);
    // left ff
    glVertex2d(middle-getTextWidth("FF"), m_engine1_center_ff.y()-0.8*m_font_height);
    glVertex2d(middle - xlen, m_engine1_center_ff.y()-0.5*m_font_height);
    // right ff
    glVertex2d(middle+getTextWidth("FF"), m_engine2_center_ff.y()-0.8*m_font_height);
    glVertex2d(middle + xlen, m_engine2_center_ff.y()-0.5*m_font_height);
    glEnd();

    qglColor(Qt::cyan);
    drawText(middle-getTextWidth("%")/2.0, m_engine1_center_n1.y()+1.5*m_font_height, "%");
    drawText(middle-getTextWidth("CC")/2.0, m_engine1_center_egt.y()+1.5*m_font_height, "°C");
    drawText(middle-getTextWidth("%")/2.0, m_engine1_center_n2.y()+0.4*m_font_height, "%");
    drawText(middle-getTextWidth("KG/H")/2.0, m_engine1_center_ff.y()+0.3*m_font_height, "KG/H");

    //----- egt markers

    double egt_angle_5 = m_engine_egt_angle_left + 0.5 * m_egt_angle_range;
    double egt_angle_10 = m_engine_egt_angle_left + m_egt_angle_range;
    
    qglColor(WHITE);

    if (!left_out)
    {
        glBegin(GL_LINES);
        glVertex2d(m_engine1_center_egt.x() + m_engine_egt_radius * sin(egt_angle_5), 
                   m_engine1_center_egt.y() + m_engine_egt_radius * -cos(egt_angle_5));
        glVertex2d(m_engine1_center_egt.x() + (m_engine_egt_radius-POINTER_OVERLAP_LEN) * sin(egt_angle_5), 
                   m_engine1_center_egt.y() + (m_engine_egt_radius-POINTER_OVERLAP_LEN) * -cos(egt_angle_5));

        glVertex2d(m_engine1_center_egt.x() + m_engine_egt_radius * sin(egt_angle_10), 
                   m_engine1_center_egt.y() + m_engine_egt_radius * -cos(egt_angle_10));
        glVertex2d(m_engine1_center_egt.x() + (m_engine_egt_radius-POINTER_OVERLAP_LEN) * sin(egt_angle_10), 
                   m_engine1_center_egt.y() + (m_engine_egt_radius-POINTER_OVERLAP_LEN) * -cos(egt_angle_10));

        glEnd();

        drawText(m_engine1_center_egt.x() + (m_engine_egt_radius-POINTER_OVERLAP_LEN-m_font_height) * 
                 sin(egt_angle_5) - getTextWidth("1"), 
                 m_engine1_center_egt.y() + (m_engine_egt_radius-POINTER_OVERLAP_LEN-m_font_height) * 
                 -cos(egt_angle_5), 
                 "5");

        drawText(m_engine1_center_egt.x() + (m_engine_egt_radius-POINTER_OVERLAP_LEN-0.8*m_font_height) * 
                 sin(egt_angle_10), 
                 m_engine1_center_egt.y() + ((m_engine_egt_radius-POINTER_OVERLAP_LEN-0.8*m_font_height) * 
                                             -cos(egt_angle_10)) +
                 (m_font_height/2.0), "10");
    }

    if (!right_out)
    {
        glBegin(GL_LINES);
        glVertex2d(m_engine2_center_egt.x() + m_engine_egt_radius * sin(egt_angle_5), 
                   m_engine2_center_egt.y() + m_engine_egt_radius * -cos(egt_angle_5));
        glVertex2d(m_engine2_center_egt.x() + (m_engine_egt_radius-POINTER_OVERLAP_LEN) * sin(egt_angle_5), 
                   m_engine2_center_egt.y() + (m_engine_egt_radius-POINTER_OVERLAP_LEN) * -cos(egt_angle_5));

        glVertex2d(m_engine2_center_egt.x() + m_engine_egt_radius * sin(egt_angle_10), 
                   m_engine2_center_egt.y() + m_engine_egt_radius * -cos(egt_angle_10));
        glVertex2d(m_engine2_center_egt.x() + (m_engine_egt_radius-POINTER_OVERLAP_LEN) * sin(egt_angle_10), 
                   m_engine2_center_egt.y() + (m_engine_egt_radius-POINTER_OVERLAP_LEN) * -cos(egt_angle_10));
        glEnd();

        drawText(m_engine2_center_egt.x() + (m_engine_egt_radius-POINTER_OVERLAP_LEN-m_font_height) * 
                 sin(egt_angle_5) - getTextWidth("1"), 
                 m_engine2_center_egt.y() + (m_engine_egt_radius-POINTER_OVERLAP_LEN-m_font_height) * 
                 -cos(egt_angle_5), 
                 "5");

        drawText(m_engine2_center_egt.x() + (m_engine_egt_radius-POINTER_OVERLAP_LEN-0.8*m_font_height) * 
                 sin(egt_angle_10), 
                 m_engine2_center_egt.y() + ((m_engine_egt_radius-POINTER_OVERLAP_LEN-0.8*m_font_height) * 
                                             -cos(egt_angle_10)) + 
                 (m_font_height/2.0), "10");
    }

    //----- N1 target bug

    if (!m_flightstatus->isReverserOn() &&
        m_flightstatus->isAtLeastOneEngineOn() &&
        (m_fmc_control->fmcAutothrottle().isAPThrottleModeN1Engaged() ||
         (m_fmc_control->fmcAutothrottle().useAirbusThrottleModes() &&
          !m_fmc_control->fmcAutothrottle().isAPThrottleEngaged())))
    {
        double target_n1 = m_fmc_control->fmcAutothrottle().getAPThrottleN1Target();

        bool draw_1 = qAbs(target_n1 - n1_1) >= 1.0;
        bool draw_2 = qAbs(target_n1 - n1_2) >= 1.0;

        if (draw_1 || draw_2)
        {
            double n1_target_angle = getN1Angle(target_n1);
            double eng1_target_angle = n1_target_angle;
            double eng2_target_angle = n1_target_angle;

            if (target_n1 < 1.0)
            {
                eng1_target_angle =
                    getN1Angle(LIMITMINMAX(n1_1 - m_flightstatus->engine_data[1].throttle_lever_percent, 0.0, 100.0)); 
                
                eng2_target_angle =
                    getN1Angle(LIMITMINMAX(n1_2 - m_flightstatus->engine_data[2].throttle_lever_percent, 0.0, 100.0));
            }

            qglColor(CYAN);
            glLineWidth(1.5);
        
            glBegin(GL_LINES);
            if (draw_1)
            {
                glVertex2d(m_engine1_center_n1.x() + (m_engine_n1_radius-POINTER_OVERLAP_LEN) * sin(eng1_target_angle), 
                           m_engine1_center_n1.y() + (m_engine_n1_radius-POINTER_OVERLAP_LEN) * -cos(eng1_target_angle));
                glVertex2d(m_engine1_center_n1.x() + (m_engine_n1_radius+POINTER_OVERLAP_LEN) * sin(eng1_target_angle), 
                           m_engine1_center_n1.y() + (m_engine_n1_radius+POINTER_OVERLAP_LEN) * -cos(eng1_target_angle));
            }
            if (draw_2)
            {
                glVertex2d(m_engine2_center_n1.x() + (m_engine_n1_radius-POINTER_OVERLAP_LEN) * sin(eng2_target_angle), 
                           m_engine2_center_n1.y() + (m_engine_n1_radius-POINTER_OVERLAP_LEN) * -cos(eng2_target_angle));
                glVertex2d(m_engine2_center_n1.x() + (m_engine_n1_radius+POINTER_OVERLAP_LEN) * sin(eng2_target_angle), 
                           m_engine2_center_n1.y() + (m_engine_n1_radius+POINTER_OVERLAP_LEN) * -cos(eng2_target_angle));
            }
            glEnd();

            // draw connector circles between n1 target bugs and n1 pointers

            if (draw_1)
                GLDraw::drawCircleOffset(m_engine_n1_radius+POINTER_OVERLAP_LEN, 
                                         qMin(n1_1_angle, eng1_target_angle),
                                         qMax(n1_1_angle, eng1_target_angle),
                                         m_engine1_center_n1.x(), m_engine1_center_n1.y());

            if (draw_2)
                GLDraw::drawCircleOffset(m_engine_n1_radius+POINTER_OVERLAP_LEN, 
                                         qMin(n1_2_angle, eng2_target_angle), 
                                         qMax(n1_2_angle, eng2_target_angle), 
                                         m_engine2_center_n1.x(), m_engine2_center_n1.y());
        }
    }

    //----- throttle position bug

    if (m_flightstatus->isAtLeastOneEngineOn())
    {
        double throttle_input_percent_1 = 
            (m_fmc_control->fmcAutothrottle().useAirbusThrottleModes()) ?
            m_fmc_control->fmcAutothrottle().getAirbusModeN1LimitByLever() : 
            m_flightstatus->engine_data[1].throttle_input_percent;

        double throttle_input_percent_2 = 
            (m_fmc_control->fmcAutothrottle().useAirbusThrottleModes()) ?
            m_fmc_control->fmcAutothrottle().getAirbusModeN1LimitByLever() : 
            m_flightstatus->engine_data[2].throttle_input_percent;

        double throttle_angle_1 = getN1Angle(throttle_input_percent_1);
        double throttle_angle_2 = getN1Angle(throttle_input_percent_2);

        if (m_fmc_control->fmcAutothrottle().isAPThrottleEngaged() &&
            !m_fmc_control->fmcAutothrottle().useAirbusThrottleModes())
        {
            throttle_angle_1 = 
                getN1Angle(LIMITMINMAX(n1_1 + throttle_input_percent_1 - 
                                       m_flightstatus->engine_data[1].throttle_lever_percent, 0.0, 100.0));
            throttle_angle_2 = 
                getN1Angle(LIMITMINMAX(n1_2 + throttle_input_percent_2 - 
                                       m_flightstatus->engine_data[2].throttle_lever_percent, 0.0, 100.0));
        }

        qglColor(Qt::white);
        glLineWidth(1.5);
    
        double sin_angle1 = sin(throttle_angle_1);
        double cos_angle1 = cos(throttle_angle_1);
        double sin_angle2 = sin(throttle_angle_2);
        double cos_angle2 = cos(throttle_angle_2);
    
        double length = POINTER_OVERLAP_LEN*1.1;

        glBegin(GL_LINES);
        glVertex2d(m_engine1_center_n1.x() + (m_engine_n1_radius) * sin_angle1, 
                   m_engine1_center_n1.y() + (m_engine_n1_radius) * -cos_angle1);
        glVertex2d(m_engine1_center_n1.x() + (m_engine_n1_radius+length) * sin_angle1,
                   m_engine1_center_n1.y() + (m_engine_n1_radius+length) * -cos_angle1);
        
        glVertex2d(m_engine2_center_n1.x() + (m_engine_n1_radius) * sin_angle2, 
                   m_engine2_center_n1.y() + (m_engine_n1_radius) * -cos_angle2);
        glVertex2d(m_engine2_center_n1.x() + (m_engine_n1_radius+length) * sin_angle2,
                   m_engine2_center_n1.y() + (m_engine_n1_radius+length) * -cos_angle2);
        glEnd();

        radius = 2.0;

        QPointF left_end = 
            QPointF(m_engine1_center_n1.x() + (m_engine_n1_radius+length+radius) * sin_angle1,
                    m_engine1_center_n1.y() + (m_engine_n1_radius+length+radius) * -cos_angle1);

        GLDraw::drawCircleOffset(radius, -M_PI, M_PI, left_end.x(), left_end.y(), 0.05);

        QPointF right_end = 
            QPointF(m_engine2_center_n1.x() + (m_engine_n1_radius+length+radius) * sin_angle2, 
                    m_engine2_center_n1.y() + (m_engine_n1_radius+length+radius) * -cos_angle2);

        GLDraw::drawCircleOffset(radius, -M_PI, M_PI, right_end.x(), right_end.y(), 0.05);
    }
    
    glPopMatrix();
}

/////////////////////////////////////////////////////////////////////////////

double GLECAMWidgetStyleA::getN1Angle(const double& n1) const
{ 
    return n1 / m_fmc_control->aircraftData().maxN1() * m_n1_angle_range + m_engine_n1_angle_left; 
}

/////////////////////////////////////////////////////////////////////////////

void GLECAMWidgetStyleA::drawTexts()
{
    //----- upper texts

    double thrust_y = m_engine2_center_n1.y()+2*m_font_height;
    double fuel_y = m_engine2_center_egt.y()+m_font_height;

    bool climb_power = false;
    bool mct_power = false;
    bool flex_power = false;
    bool toga_power = false;
    QString n1_string;

    if (m_fmc_control->fmcAutothrottle().useAirbusThrottleModes())
    {
        if (m_fmc_control->fmcAutothrottle().isAPThrottleModeN1Engaged() ||
            !m_fmc_control->fmcAutothrottle().isAPThrottleEngaged())
        {
            switch(m_fmc_control->fmcAutothrottle().currentAirbusThrottleMode())
            {
                case(FMCAutothrottle::AIRBUS_THROTTLE_CLIMB):
                    climb_power = m_fmc_control->fmcAutothrottle().getAPThrottleN1Target() > 1.0;
                    n1_string = QString::number(m_fmc_control->fmcAutothrottle().currentClimbThrust(), 'f', 1);
                    break;
                case(FMCAutothrottle::AIRBUS_THROTTLE_MCT):
                    mct_power = true;
                    n1_string = QString::number(m_fmc_control->fmcAutothrottle().currentMaxContinousThrust(), 'f', 1);
                    break;
                case(FMCAutothrottle::AIRBUS_THROTTLE_FLEX):
                    flex_power = true;
                    n1_string = QString::number(m_fmc_control->fmcAutothrottle().currentFlexThrust(), 'f', 1);
                    break;
                case(FMCAutothrottle::AIRBUS_THROTTLE_TOGA):
                    toga_power = true;
                    n1_string = QString::number(m_fmc_control->fmcAutothrottle().currentTakeoffThrust(), 'f', 1);
                    break;
                default:
                    break;
            }
        }
    }
    else
    {
        flex_power = m_flightstatus->onground || 
                     (m_fmc_control->fmcAutothrottle().isAPThrottleModeN1Engaged() &&
                      !m_fmc_control->fmcAutothrottle().isAPThrottleN1ClimbModeActive());
        
        climb_power = (m_fmc_control->fmcAutothrottle().isAPThrottleModeN1Engaged() &&
                       m_fmc_control->fmcAutothrottle().isAPThrottleN1ClimbModeActive() &&
                       m_fmc_control->fmcAutothrottle().getAPThrottleN1Target() > 1.0);
        
        if (climb_power) n1_string = QString::number(m_fmc_control->fmcAutothrottle().currentClimbThrust(), 'f', 1);
        else if (flex_power) n1_string = QString::number(m_fmc_control->fmcAutothrottle().currentFlexThrust(), 'f', 1);
    }

    // white
    
    qglColor(Qt::white);
    drawText(m_right_text_ref.x(), fuel_y, "FOB:");
    
    // green
    
    qglColor(Qt::green);
    QString fob_string = QString::number(m_flightstatus->fuelOnBoard());
    drawText(m_right_text_ref.x() + getTextWidth("FOB:  "), fuel_y, fob_string);
    
    if (toga_power || flex_power || climb_power || mct_power)
    {
        drawText(m_right_text_ref.x() + getTextWidth("OGA  "), thrust_y, n1_string);
    }

    // cyan

    qglColor(Qt::cyan);

    drawText(m_right_text_ref.x() + getTextWidth("FOB:    ") + getTextWidth(fob_string), fuel_y, "KG");

    if (climb_power)
    {
        drawText(m_right_text_ref.x() - getTextWidth("T"), thrust_y, "CL");
        drawText(m_right_text_ref.x() + getTextWidth("OGA   ") + getTextWidth(n1_string), thrust_y, "%");
    }
    else if (mct_power)
    {
        drawText(m_right_text_ref.x() - getTextWidth("T"), thrust_y, "MCT");
        drawText(m_right_text_ref.x() + getTextWidth("OGA   ") + getTextWidth(n1_string), thrust_y, "%");
    }
    else if (flex_power)
    {
        drawText(m_right_text_ref.x() - getTextWidth("T"), thrust_y, "FLEX");
        drawText(m_right_text_ref.x() + getTextWidth("OGA   ") + getTextWidth(n1_string), thrust_y, "%");
    }
    else if (toga_power)
    {
        drawText(m_right_text_ref.x() - getTextWidth("T"), thrust_y, "TOGA");
        drawText(m_right_text_ref.x() + getTextWidth("OGA   ") + getTextWidth(n1_string), thrust_y, "%");
    }

    //----- flaps

    qglColor(WHITE);

    double quad_width = getTextWidth("LA") / 2.0;
    double quad_height = m_flap_y_inc * 0.5;

    glLineWidth(1.5);
    glBegin(GL_QUADS);
    glVertex2d(m_flap_text_middle-quad_width*1.5, m_slat_y-quad_height);
    glVertex2d(m_flap_text_middle+quad_width*1.5, m_slat_y-quad_height);
    glVertex2d(m_flap_text_middle+quad_width, m_slat_y);
    glVertex2d(m_flap_text_middle-quad_width, m_slat_y);
    glEnd();

    if (m_flightstatus->current_flap_lever_notch > 0 || m_flightstatus->areFlapsInTransit())
    {
        drawText(m_flap_text_x_left, m_flap_text_y, "S");
        drawText(m_flap_text_x_right, m_flap_text_y, "F");
        if (m_flightstatus->areFlapsInTransit()) qglColor(CYAN);
        drawText(m_flap_text_middle - 0.5 * getTextWidth("FLAP"), m_flap_text_y, "FLAP");
        qglColor(WHITE);

        // draw slat/flap dots

        double len = 3.0;
        glLineWidth(len);
        glBegin(GL_LINES);
        
        for(int slat_index = 0; slat_index < m_slat_points.count(); ++slat_index)
        {
            glVertex2d(m_slat_points[slat_index].x(), m_slat_points[slat_index].y());
            glVertex2d(m_slat_points[slat_index].x()+len, m_slat_points[slat_index].y());
        }

        for(int flap_index = 0; flap_index < m_flap_points.count(); ++flap_index)
        {
            glVertex2d(m_flap_points[flap_index].x(), m_flap_points[flap_index].y());
            glVertex2d(m_flap_points[flap_index].x()+len, m_flap_points[flap_index].y());
        }

        glEnd();

        // draw slat/flap lines

        QPointF slat_ref_point = QPointF(m_flap_text_middle-quad_width, m_slat_y);
        QPointF flap_ref_point = QPointF(m_flap_text_middle+quad_width, m_slat_y);

        glLineWidth(1.1);
        qglColor(GREEN);
        glBegin(GL_LINES);

        if (m_flightstatus->current_flap_lever_notch >= 1)
        {
            glVertex2d(slat_ref_point.x(), slat_ref_point.y());
            glVertex2d(m_slat_points[0].x(), m_slat_points[0].y());
        }
        if (m_flightstatus->current_flap_lever_notch >= 2)
        {
            glVertex2d(flap_ref_point.x(), flap_ref_point.y());
            glVertex2d(m_flap_points[0].x(), m_flap_points[0].y());
        }
        if (m_flightstatus->current_flap_lever_notch >= 3)
        {
            glVertex2d(m_slat_points[0].x(), m_slat_points[0].y());
            glVertex2d(m_slat_points[1].x(), m_slat_points[1].y());

            glVertex2d(m_flap_points[0].x(), m_flap_points[0].y());
            glVertex2d(m_flap_points[1].x(), m_flap_points[1].y());

        }
        if (m_flightstatus->current_flap_lever_notch >= 4)
        {
            glVertex2d(m_slat_points[0].x(), m_slat_points[0].y());
            glVertex2d(m_slat_points[1].x(), m_slat_points[1].y());
                
            glVertex2d(m_flap_points[1].x(), m_flap_points[1].y());
            glVertex2d(m_flap_points[2].x(), m_flap_points[2].y());
        }
        if (m_flightstatus->current_flap_lever_notch >= 5)
        {
            glVertex2d(m_slat_points[1].x(), m_slat_points[1].y());
            glVertex2d(m_slat_points[2].x(), m_slat_points[2].y());
                
            glVertex2d(m_flap_points[2].x(), m_flap_points[2].y());            
            glVertex2d(m_flap_points[3].x(), m_flap_points[3].y());
        }

        //TODO make flap movements
        glEnd();

        // draw flap notch text

        qglColor(GREEN);
        
        QString notch_string;

        if (m_flightstatus->current_flap_lever_notch < 2) 
            notch_string = QString::number(m_flightstatus->current_flap_lever_notch);
        if (m_flightstatus->current_flap_lever_notch == 2) 
            notch_string = "1+F";
        else if (m_flightstatus->current_flap_lever_notch == (m_flightstatus->flaps_lever_notch_count-1)) 
            notch_string = "FULL";
        else if (m_flightstatus->current_flap_lever_notch > 2) 
            notch_string = QString::number(m_flightstatus->current_flap_lever_notch-1);
        
        if (m_flightstatus->areFlapsInTransit()) qglColor(CYAN);
        drawText(m_flap_text_middle - 0.5 * getTextWidth(notch_string), m_flap_text_y + m_font_height * 2.5, notch_string);
    }

    if (!m_flightstatus->isAtLeastOneEngineOn())
    {
        m_takeoff_inhibit = false;
    }
    else if (m_fmc_control->normalRoute().accelerationAltitudeFt() > 0)
    {
        if (m_flightstatus->smoothed_altimeter_readout.lastValue() >= m_fmc_control->normalRoute().accelerationAltitudeFt())
            m_takeoff_inhibit = false;
    }
    else
    {
        if (m_flightstatus->smoothed_altimeter_readout.lastValue() >= 5000)
            m_takeoff_inhibit = false;
    }

    if (m_flightstatus->onground && m_flightstatus->smoothed_ias.lastValue() < 80)
        m_landing_inhibit = false;

    //----- left texts

    double vs = m_flightstatus->smoothedVS();

    double text_x = m_left_text_ref.x();
    double text_y = m_left_text_ref.y();

    double max_y = m_size.height();

    if (m_flightstatus->onground &&
        m_flightstatus->isAtLeastOneEngineOn() &&
        !m_fmc_control->flightStatusChecker().wasAirborneSinceLastEngineOff() &&
        m_fmc_control->flightStatusChecker().secondsSinceLastEngingeStart() >= 120 &&
        m_flightstatus->engine_data[1].throttle_lever_percent < 40.0)
    {
        m_takeoff_inhibit = true;
        m_landing_inhibit = false;
        bool all_ok = true;

        qglColor(GREEN);

        drawText(text_x, text_y, "T.O");
        double x = text_x + getTextWidth("T.O  ");
        
//TODO this does not work for now because the spoilers will extend when armed on the ground !?
//         // check spoilers

//         if (!m_flightstatus->spoilers_armed)
//         {
//             all_ok = false;
//             drawText(x, text_y, "SPLRS");
//             qglColor(CYAN);
//             drawText(x + getTextWidth("SPLRS "), text_y, ".........ARM");
//         }
//         else
//         {
//             drawText(x, text_y, "SPLRS ARM");
//         }

//         text_y += m_font_height;

        // check flaps

        qglColor(GREEN);

        if (m_fmc_control->isAirbusFlapsHandlingModeEnabled() &&
            m_fmc_control->fmcData().takeoffFlapsNotch()+1 != m_flightstatus->current_flap_lever_notch)
        {
            all_ok = false;
            drawText(x, text_y, "FLAPS");
            qglColor(CYAN);
            drawText(x + getTextWidth("FLAPS "), text_y, "..........T.O");
        }
        else
        {
            drawText(x, text_y, "FLAPS T.O");
        }

        text_y += m_font_height;
        
        // summary

        qglColor(GREEN);

        if (!all_ok)
        {
            drawText(x, text_y, "T.O CONFIG");
            qglColor(CYAN);
            drawText(x + getTextWidth("T.O CONFIG"), text_y, "...TEST");
        }
        else
        {
            drawText(x, text_y, "T.O CONFIG NORMAL");
        }

        text_y += m_font_height;
    }
    else if (m_flightstatus->radarAltitude() < 2000 &&
             ((!m_flightstatus->isGearDown() && m_flightstatus->radarAltitude() < 800) || m_flightstatus->isGearDown()) &&
             m_fmc_control->flightStatusChecker().wasAbove1000FtSinceLastOnGroundTaxi() &&
             !(m_flightstatus->onground && m_flightstatus->smoothed_ias.lastValue() < 80.0))
    {
        m_landing_inhibit = true;
        m_takeoff_inhibit = false;

        if (!m_flightstatus->onground)
        {
            qglColor(GREEN);

            drawText(text_x, text_y, "LDG");
            double x = text_x + getTextWidth("LDG  ");
        
            // check gear

            if (!m_flightstatus->isGearDown()) 
            {
                drawText(x, text_y, "LDG GEAR");
                qglColor(CYAN);
                drawText(x + getTextWidth("LDG GEAR "), text_y, ".....DN");
            }
            else
            {
                drawText(x, text_y, "LDG GEAR DN");
            }

            text_y += m_font_height;

            // check spoilers

            qglColor(GREEN);

            if (!m_flightstatus->spoilers_armed) 
            {
                drawText(x, text_y, "SPLRS");
                qglColor(CYAN);
                drawText(x + getTextWidth("SPLRS "), text_y, "...........ARM");
            }
            else
            {
                drawText(x, text_y, "SPLRS ARM");
            }

            text_y += m_font_height;

            // check flaps

            qglColor(GREEN);

            if (m_fmc_control->isAirbusFlapsHandlingModeEnabled() && 
                m_flightstatus->current_flap_lever_notch != m_fmc_control->fmcData().landingFlapsNotch()) 
            {
                drawText(x, text_y, "FLAPS");
                qglColor(CYAN);
                drawText(x + getTextWidth("FLAPS "), text_y, "...........LDG");
            }
            else
            {
                drawText(x, text_y, "FLAPS LDG");
            }

            text_y += m_font_height;
        }
    }

    qglColor(RED);

    if (!m_fmc_control->navdata().isValid())
    {
        drawText(text_x, text_y, "NAVDATA NOT VALID");
        text_y += m_font_height;
    }

    if (!m_flightstatus->isValid())
    {
        drawText(text_x, text_y, "NO FLTSIM CONNECT");
        text_y += m_font_height;
    }

    if (!m_flightstatus->onground &&
        text_y < max_y && 
        m_flightstatus->doors_open)
    {
        drawText(text_x, text_y, "L FWD CABIN");
        text_y += m_font_height;
    }

    if (!m_flightstatus->onground &&
        text_y < max_y && 
        m_flightstatus->radarAltitude() < 500 && 
        m_flightstatus->current_flap_lever_notch <= 0 &&
        vs <= 0)
    {
        drawText(text_x, text_y, "TOO LOW - FLAPS");
        text_y += m_font_height;
    }

    if (!m_flightstatus->onground &&
        text_y < max_y && 
        m_flightstatus->radarAltitude() < 500 && 
        !m_flightstatus->isGearDown() &&
        vs <= 0)
    {
        drawText(text_x, text_y, "TOO LOW - GEAR");
        text_y += m_font_height;
    }

    if (text_y < max_y && 
        m_flightstatus->fuelOnBoardPercentage() < 10.0 && 
        m_flightstatus->fuelOnBoard() < 4000)
    {
        drawText(text_x, text_y, "LOW FUEL");
        text_y += m_font_height;
    }

    if (text_y < max_y && 
        !m_flightstatus->onground &&
        m_flightstatus->fuelOnBoardPercentage() < 5.0 &&
        m_flightstatus->fuelOnBoard() < 2000)
    {
        drawText(text_x, text_y, "LAND ASAP");
        text_y += m_font_height;
    }

    if (text_y < max_y && !m_flightstatus->onground && m_flightstatus->parking_brake_set && !m_takeoff_inhibit)
    {
        drawText(text_x, text_y, "PARK BRK");
        text_y += m_font_height;
    }

    qglColor(AMBER);

    if (text_y < max_y && m_flightstatus->isGearDown() && m_flightstatus->radarAltitude() > 10000)
    {
        drawText(text_x, text_y, "GEAR");
        text_y += m_font_height;
    }

    if (text_y < max_y && !m_flightstatus->onground && !m_flightstatus->pitot_heat_on && !m_takeoff_inhibit)
    {
        drawText(text_x, text_y, "PITOT HEAT OFF");
        text_y += m_font_height;
    }

    if (text_y < max_y && !m_flightstatus->onground && !m_flightstatus->lights_strobe && !m_takeoff_inhibit)
    {
        drawText(text_x, text_y, "STROBE LT OFF");
        text_y += m_font_height;
    }

    if (text_y < max_y && !m_flightstatus->isAtLeastOneEngineOn() && m_flightstatus->lights_strobe)
    {
        drawText(text_x, text_y, "STROBE LT ON");
        text_y += m_font_height;
    }
    
    if (text_y < max_y && m_flightstatus->isAtLeastOneEngineOn())
    {
        if (!m_flightstatus->lights_beacon)
        {
            drawText(text_x, text_y, "BEACON LT OFF");
            text_y += m_font_height;
        }

        if (text_y < max_y && 
            m_flightstatus->onground &&
            m_flightstatus->doors_open)
        {
            drawText(text_x, text_y, "L FWD CABIN");
            text_y += m_font_height;
        }
    }

    if (text_y < max_y && !m_flightstatus->onground)
    {
        if (!m_flightstatus->lights_landing && !m_takeoff_inhibit)
        {
            if (m_flightstatus->smoothed_altimeter_readout.lastValue() < 9000)
            {
                drawText(text_x, text_y, "LDG LT OFF");
                text_y += m_font_height;
            }
        }
        else 
        {
            if (m_flightstatus->smoothed_altimeter_readout.lastValue() > 19000)
            {
                drawText(text_x, text_y, "LDG LT ON");
                    text_y += m_font_height;
            }
        }
    }
    
    if (text_y < max_y && !m_flightstatus->onground && !m_flightstatus->lights_navigation && !m_takeoff_inhibit)
    {
        drawText(text_x, text_y, "NAV LT OFF");
        text_y += m_font_height;
    }
    
    qglColor(GREEN);

    if (text_y < max_y && 
        m_flightstatus->engine_ignition_on)
    {
        drawText(text_x, text_y, "IGNITION");
        text_y += m_font_height;
    }

    if (text_y < max_y && !m_takeoff_inhibit && !m_landing_inhibit)
    {
        if (m_flightstatus->spoiler_left_percent > 0 || m_flightstatus->spoiler_right_percent > 0)
        {
            if (!m_flightstatus->onground)
            {
                if (m_flightstatus->spoiler_lever_percent > 1.0)
                {
                    drawText(text_x, text_y, "SPEED BRK");
                    text_y += m_font_height;
                }
                else if (m_flightstatus->spoilers_armed)
                {
                    drawText(text_x, text_y, "GND SPLRS ARMED");
                    text_y += m_font_height;
                }
            }
            else
            {
                drawText(text_x, text_y, "GND SPLRS");
                text_y += m_font_height;
            }
        }
        else if (m_flightstatus->spoilers_armed)
        {
            drawText(text_x, text_y, "GND SPLRS ARMED");
            text_y += m_font_height;
        }
    }

    if (text_y < max_y && 
        m_flightstatus->onground &&
        m_flightstatus->doors_open &&
        !m_flightstatus->isAtLeastOneEngineOn())
    {
        drawText(text_x, text_y, "L FWD CABIN");
        text_y += m_font_height;
    }

    if (text_y < max_y &&
        m_flightstatus->seat_belt_sign)
    {
        drawText(text_x, text_y, "SEAT BELT");
        text_y += m_font_height;
    }

    if (text_y < max_y &&
        m_flightstatus->no_smoking_sign)
    {
        drawText(text_x, text_y, "NO SMOKING");
        text_y += m_font_height;
    }

    //----- right texts

    text_x = m_right_text_ref.x();
    text_y = m_right_text_ref.y();

    qglColor(MAGENTA);

    if (m_takeoff_inhibit)
    {
        drawText(text_x, text_y, "T.O. INHIBIT");
        text_y += m_font_height;
    }
    else if (m_landing_inhibit)
    {
        drawText(text_x, text_y, "LDG INHIBIT");
        text_y += m_font_height;
    }

    qglColor(GREEN);

    if (text_y < max_y)
    {
        if (m_fmc_control->fmcAutothrottle().isAPThrottleArmed())
        {
            m_at_was_connected = true;
            m_at_disco_timestamp = QTime();
        }
        else
        {
            if (m_at_was_connected)
            {
                m_at_was_connected = false;
                m_at_disco_timestamp.start();
            }
        }

        if (m_at_disco_timestamp.isValid())
        {
            drawText(text_x, text_y, "A/THR OFF");
            text_y += m_font_height;
            if (m_at_disco_timestamp.elapsed() > 7000) m_at_disco_timestamp = QTime();
        }
    }

    if (text_y < max_y && !m_fmc_control->isTCASOn())
    {
        m_fmc_control->isTCASStandby() ? 
            drawText(text_x, text_y, "TCAS STBY") :
            drawText(text_x, text_y, "TCAS OFF");
            
        text_y += m_font_height;
    }

    if (text_y < max_y && m_flightstatus->isEngineAntiIceOn())
    {
        drawText(text_x, text_y, "ENG A.ICE");
        text_y += m_font_height;
    }

    if (text_y < max_y && m_flightstatus->onground && m_flightstatus->parking_brake_set) 
    {
        drawText(text_x, text_y, "PARK BRK");
        text_y += m_font_height;
    }

    if (text_y < max_y && m_flightstatus->lights_landing) 
    {
        (m_flightstatus->lights_taxi) ? drawText(text_x, text_y, "LDG/TAXI LT") : drawText(text_x, text_y, "LDG LT");
        text_y += m_font_height;
    }

    if (text_y < max_y && m_flightstatus->lights_taxi && !m_flightstatus->lights_landing) 
    {
        drawText(text_x, text_y, "TAXI LT");
        text_y += m_font_height;
    }
}

/////////////////////////////////////////////////////////////////////////////

// void GLECAMWidgetStyleA::enableClipping(GLuint gllist_id)
// {
//     glPushMatrix();

//     glEnable(GL_DEPTH_TEST);
//     glDepthFunc(GL_ALWAYS);
//     glDepthMask(0xFFFF);
//     glColor4f(1.0, 0.0, 0.0, 0.0);

//     // draw overal clipping rect to init depth buffer
//     glLoadIdentity();
//     glTranslated(0, 0, -0.5);
//     glBegin(GL_QUADS);
//     glVertex2d(0, 0);
//     glVertex2d(m_size.width(), 0);
//     glVertex2d(m_size.width(), m_size.height());
//     glVertex2d(0, m_size.height());
//     glEnd();

//     // draw clip poly for regions that should be displayed
//     glCallList(gllist_id);

//     glDepthMask(0x0);
//     glDepthFunc(GL_LESS);

//     glPopMatrix();
// }

// /////////////////////////////////////////////////////////////////////////////

// void GLECAMWidgetStyleA::disableClipping()
// {
//     glDisable(GL_DEPTH_TEST);
//}

/////////////////////////////////////////////////////////////////////////////

void GLECAMWidgetStyleA::drawFPS()
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

