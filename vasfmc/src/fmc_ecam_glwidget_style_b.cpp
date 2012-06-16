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
#include "fmc_autothrottle.h"

#include "gldraw.h"
#include "fmc_control.h"
#include "fmc_ecam.h"

#include "fmc_ecam_glwidget_style_b.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

#define SCALE_MAX_N1 110.0
#define POINTER_OVERLAP_LEN 4

/////////////////////////////////////////////////////////////////////////////

GLECAMWidgetStyleB::GLECAMWidgetStyleB(bool upper_ecam, 
                                       ConfigWidgetProvider* config_widget_provider,
                                       Config* main_config,
                                       Config* ecam_config, 
                                       FMCControl* fmc_control,
                                       VasWidget *parent) : 
    GLECAMWidgetBase(upper_ecam, config_widget_provider, main_config, ecam_config, fmc_control, parent),
    m_font(0),
    m_recalc_ecam(true),
    m_fps_count(0),
    m_onground(false)
{
    MYASSERT(m_config_widget_provider != 0);
    MYASSERT(m_main_config != 0);
    MYASSERT(m_ecam_config != 0);
    MYASSERT(m_fmc_control != 0);
    MYASSERT(m_flightstatus != 0);

	m_fps_counter_time.start();
    m_gear_was_down_at_last_update = m_flightstatus->isGearDown();
    m_gear_up_timer = QTime::currentTime().addSecs(-20);
    m_flaps_were_down_at_last_update = !m_flightstatus->flapsAreUp();
    m_flaps_up_timer = QTime::currentTime().addSecs(-20);
}

/////////////////////////////////////////////////////////////////////////////

GLECAMWidgetStyleB::~GLECAMWidgetStyleB() 
{
}

/////////////////////////////////////////////////////////////////////////////

void GLECAMWidgetStyleB::initializeGL()
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

void GLECAMWidgetStyleB::resizeGL(int width, int height)
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

void GLECAMWidgetStyleB::refreshECAM()
{
    if (!isVisible()) return;
    updateGL();
};

/////////////////////////////////////////////////////////////////////////////

void GLECAMWidgetStyleB::paintGL()
{
    if (!isVisible()) return;

    setupStateBeforeDraw();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_flightstatus->isValid() && !m_flightstatus->battery_on) return;

    drawDisplay();
    if (m_fmc_control->showFps()) drawFPS();
    glFlush();

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) Logger::log(QString("GLECAMWidgetStyleB:paintGL: GL Error: %1").arg(error));
}

/////////////////////////////////////////////////////////////////////////////

void GLECAMWidgetStyleB::setupStateBeforeDraw()
{
    if (m_recalc_ecam)
    {
        m_recalc_ecam = false;

        //----- engines

        double eng_x1 = m_size.width() * 2.8 / 17.5;
        double eng_x2 = m_size.width() * 7.0 / 17.5;
        double eng_y1 = m_size.width() * 3.0 / 17.5;
        double eng_y2 = m_size.width() * 6.5 / 17.5;

        m_engine_middle = eng_x1 + (eng_x2 - eng_x1)*0.5;

        m_engine1_center_n1 = QPointF(eng_x1, eng_y1);
        m_engine2_center_n1 = QPointF(eng_x2, eng_y1);
        m_engine_n1_radius = m_size.width() * 1.5 / 17.5;

        m_engine1_center_egt = QPointF(eng_x1, eng_y2);
        m_engine2_center_egt = QPointF(eng_x2, eng_y2);
        m_engine_egt_radius = m_size.width() * 1.5 / 17.5;

        m_engine_n1_angle_left = Navcalc::toRad(90);
        m_engine_n1_angle_right = Navcalc::toRad(300.0);

        m_engine_egt_angle_left = Navcalc::toRad(90.0);
        m_engine_egt_angle_right = Navcalc::toRad(300.0);

        m_engine1_center_n2 = QPointF(eng_x1, m_size.height()*0.5 + m_font_height);
        m_engine2_center_n2 = QPointF(eng_x2, m_size.height()*0.5 + m_font_height);

        m_engine1_center_ff = QPointF(eng_x1, m_engine1_center_n2.y() + m_font_height*1.5);
        m_engine2_center_ff = QPointF(eng_x2, m_engine2_center_n2.y() + m_font_height*1.5);
        
        //----- texts

        m_right_text_ref = QPointF(m_size.width() * 10.0 / 17.5, m_font_height*2);
        
        //----- gear

        m_gear_center = QPointF(m_right_text_ref.x() + (m_size.width() - m_right_text_ref.x())*0.5, 
                                m_engine1_center_egt.y() + m_engine_egt_radius + m_font_height);

        //----- flaps

        m_flaps_upper_center = QPointF(m_gear_center.x(), m_gear_center.y() + m_font_height*2.5);
        m_flaps_lower_center = QPointF(m_gear_center.x(), m_size.height()-(m_font_height*2.5));
        
//TODO
//         m_flap_text_spacer = 2* (m_right_text_ref.x() - m_separator_vertical_x);
//         m_flap_text_x_left = m_right_text_ref.x();
//         m_flap_text_x_right = m_size.width() - 2*spacer;
//         m_flap_text_middle = m_flap_text_x_left + ((m_flap_text_x_right - m_flap_text_x_left) * 0.5);
//         m_flap_text_y = m_engine2_center_egt.y() + 2*m_font_height;

//         m_slat_x = m_flap_text_x_left + getTextWidth("X") * 3;
//         m_slat_y = m_flap_text_y + m_font_height;
//         m_slat_x_inc = -getTextWidth("XXX") / 3.0;
//         m_slat_y_inc = (m_separator_horizontal_y - m_flap_text_y) / 4.5;
        
//         m_flap_x = m_flap_text_x_right - getTextWidth("XX");
//         m_flap_y = m_flap_text_y + m_font_height;
//         m_flap_x_inc = getTextWidth("XXXX") / 4.0;
//         m_flap_y_inc = m_slat_y_inc * 0.8;

//         m_slat_points.clear();
//         m_slat_points.append(QPointF(m_slat_x, m_slat_y+0.2*m_slat_y_inc));
//         m_slat_points.append(QPointF(m_slat_x+m_slat_x_inc, m_slat_y+m_slat_y_inc));
//         m_slat_points.append(QPointF(m_slat_x+1.5*m_slat_x_inc, m_slat_y+2*m_slat_y_inc));

//         m_flap_points.clear();
//         m_flap_points.append(QPointF(m_flap_x, m_flap_y+0.2*m_flap_y_inc));
//         m_flap_points.append(QPointF(m_flap_x+1.5*m_flap_x_inc, m_flap_y+m_flap_y_inc));
//         m_flap_points.append(QPointF(m_flap_x+2*m_flap_x_inc, m_flap_y+2*m_flap_y_inc));
//         m_flap_points.append(QPointF(m_flap_x+2.4*m_flap_x_inc,
//         m_flap_y+3*m_flap_y_inc));

    }
};

/////////////////////////////////////////////////////////////////////////////

void GLECAMWidgetStyleB::drawDisplay()
{
    glLoadIdentity();

    drawEnginges();
    drawTexts();
};

/////////////////////////////////////////////////////////////////////////////

void GLECAMWidgetStyleB::drawEnginges()
{
    glPushMatrix();

    double n1_angle_range = qAbs(m_engine_n1_angle_right - m_engine_n1_angle_left);
    double egt_angle_range = qAbs(m_engine_egt_angle_right - m_engine_egt_angle_left);

    double max_n1 = 95;

    double n1_1 = m_flightstatus->engine_data[1].smoothedN1();
    double n1_2 = m_flightstatus->engine_data[2].smoothedN1();
    
    bool left_red = n1_1 > max_n1;
    bool left_out = m_flightstatus->engine_data[1].n2_percent < 2;
    bool right_red = n1_2 > max_n1;
    bool right_out = m_flightstatus->engine_data[2].n2_percent < 2;

    double n1_1_angle = n1_1 / SCALE_MAX_N1 * n1_angle_range + m_engine_n1_angle_left;
    double n1_2_angle = n1_2 / SCALE_MAX_N1 * n1_angle_range + m_engine_n1_angle_left;

    double egt_1_angle = m_flightstatus->engine_data[1].egt_degrees / 1000.0 * egt_angle_range + m_engine_egt_angle_left;
    double egt_2_angle = m_flightstatus->engine_data[2].egt_degrees / 1000.0 * egt_angle_range + m_engine_egt_angle_left;

    //----- circles

    glLineWidth(2.0);

    qglColor(Qt::darkGray);

    // left n1 + egt

    GLDraw::drawFilledCircleOffset(m_engine_n1_radius, m_engine_n1_angle_left, n1_1_angle, 
                             m_engine1_center_n1.x(), m_engine1_center_n1.y());
    GLDraw::drawFilledCircleOffset(m_engine_egt_radius, m_engine_egt_angle_left, egt_1_angle, 
                                   m_engine1_center_egt.x(), m_engine1_center_egt.y());

    // right n1 + egt
    
    GLDraw::drawFilledCircleOffset(m_engine_n1_radius, m_engine_n1_angle_left, n1_2_angle, 
                                   m_engine2_center_n1.x(), m_engine2_center_n1.y());
    GLDraw::drawFilledCircleOffset(m_engine_egt_radius, m_engine_egt_angle_left, egt_2_angle, 
                                   m_engine2_center_egt.x(), m_engine2_center_egt.y());

    qglColor(WHITE);

    // left n1 + egt

    GLDraw::drawCircleOffset(m_engine_n1_radius, m_engine_n1_angle_left, m_engine_n1_angle_right, 
                             m_engine1_center_n1.x(), m_engine1_center_n1.y());
    GLDraw::drawCircleOffset(m_engine_egt_radius, m_engine_egt_angle_left, m_engine_egt_angle_right, 
                             m_engine1_center_egt.x(), m_engine1_center_egt.y());

    // right n1 + egt

    GLDraw::drawCircleOffset(m_engine_n1_radius, m_engine_n1_angle_left, m_engine_n1_angle_right, 
                             m_engine2_center_n1.x(), m_engine2_center_n1.y());
    GLDraw::drawCircleOffset(m_engine_egt_radius, m_engine_egt_angle_left, m_engine_egt_angle_right, 
                             m_engine2_center_egt.x(), m_engine2_center_egt.y());

    // n1 red area

    glLineWidth(2.5);

    qglColor(RED);
    
    double max_n1_angle = max_n1 / SCALE_MAX_N1 * n1_angle_range + m_engine_n1_angle_left;

    glBegin(GL_LINES);
    glVertex2d(m_engine1_center_n1.x() + (m_engine_n1_radius) * sin(max_n1_angle), 
               m_engine1_center_n1.y() + (m_engine_n1_radius) * -cos(max_n1_angle));
    glVertex2d(m_engine1_center_n1.x() + (m_engine_n1_radius+POINTER_OVERLAP_LEN*2) * sin(max_n1_angle),
               m_engine1_center_n1.y() + (m_engine_n1_radius+POINTER_OVERLAP_LEN*2) * -cos(max_n1_angle));
    glVertex2d(m_engine2_center_n1.x() + (m_engine_n1_radius) * sin(max_n1_angle), 
               m_engine2_center_n1.y() + (m_engine_n1_radius) * -cos(max_n1_angle));
    glVertex2d(m_engine2_center_n1.x() + (m_engine_n1_radius+POINTER_OVERLAP_LEN*2) * sin(max_n1_angle),
               m_engine2_center_n1.y() + (m_engine_n1_radius+POINTER_OVERLAP_LEN*2) * -cos(max_n1_angle));
    glEnd();
        
    // egt red area

    double max_egt_angle = 0.95 * egt_angle_range + m_engine_egt_angle_left;

    glBegin(GL_LINES);
    glVertex2d(m_engine1_center_egt.x() + (m_engine_egt_radius) * sin(max_egt_angle), 
               m_engine1_center_egt.y() + (m_engine_egt_radius) * -cos(max_egt_angle));
    glVertex2d(m_engine1_center_egt.x() + (m_engine_egt_radius+POINTER_OVERLAP_LEN*2) * sin(max_egt_angle),
               m_engine1_center_egt.y() + (m_engine_egt_radius+POINTER_OVERLAP_LEN*2) * -cos(max_egt_angle));
    glVertex2d(m_engine2_center_egt.x() + (m_engine_egt_radius) * sin(max_egt_angle), 
               m_engine2_center_egt.y() + (m_engine_egt_radius) * -cos(max_egt_angle));
    glVertex2d(m_engine2_center_egt.x() + (m_engine_egt_radius+POINTER_OVERLAP_LEN*2) * sin(max_egt_angle),
               m_engine2_center_egt.y() + (m_engine_egt_radius+POINTER_OVERLAP_LEN*2) * -cos(max_egt_angle));
    glEnd();

    //----- pointer + text

    //----- left n1 + egt + n2 + ff

    glLineWidth(2.5);

    if (left_red) qglColor(RED);
    else qglColor(WHITE);

    if (!left_out)
    {
        glBegin(GL_LINES);
        glVertex2d(m_engine1_center_n1.x(), m_engine1_center_n1.y());
        glVertex2d(m_engine1_center_n1.x() + (m_engine_n1_radius+POINTER_OVERLAP_LEN) * sin(n1_1_angle), 
                   m_engine1_center_n1.y() + (m_engine_n1_radius+POINTER_OVERLAP_LEN) * -cos(n1_1_angle));
        glEnd();
    }

    glBegin(GL_LINES);
    glVertex2d(m_engine1_center_egt.x(), m_engine1_center_egt.y());
    glVertex2d(m_engine1_center_egt.x() + (m_engine_egt_radius) * sin(egt_1_angle), 
               m_engine1_center_egt.y() + (m_engine_egt_radius) * -cos(egt_1_angle));
    glEnd();

    glLineWidth(1.5);
    qglColor(WHITE);

    double n1_box_y = m_engine1_center_n1.y() - 4;
    double egt_box_y = m_engine1_center_egt.y() - 4;
    double n1_box_width = getTextWidth("112.0")*1.07;

    glBegin(GL_LINE_LOOP);
    glVertex2d(m_engine1_center_n1.x(), n1_box_y);
    glVertex2d(m_engine1_center_n1.x() + n1_box_width, n1_box_y);
    glVertex2d(m_engine1_center_n1.x() + n1_box_width, n1_box_y-m_font_height);
    glVertex2d(m_engine1_center_n1.x(), n1_box_y-m_font_height);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex2d(m_engine1_center_egt.x(), egt_box_y);
    glVertex2d(m_engine1_center_egt.x() + n1_box_width, egt_box_y);
    glVertex2d(m_engine1_center_egt.x() + n1_box_width, egt_box_y-m_font_height);
    glVertex2d(m_engine1_center_egt.x(), egt_box_y-m_font_height);
    glEnd();

    if (m_flightstatus->engine_data[1].reverser_percent >= 0.001)
        drawText(m_engine1_center_n1.x() + n1_box_width - getTextWidth("REV"), n1_box_y-m_font_height, "REV");

    QString value;

    if (!left_out)
    {
        value = QString::number(n1_1, 'f', 1);
        drawText(m_engine1_center_n1.x() + n1_box_width - getTextWidth(value) - 3,
                 n1_box_y, value);
    }

    value = (left_out) ? "0.0" : QString::number(m_flightstatus->engine_data[1].egt_degrees, 'f', 0);
    drawText(m_engine1_center_egt.x() + n1_box_width - getTextWidth(value) - 3, 
             egt_box_y, value);

    //----- right n1 + egt + n2 + ff

    glLineWidth(2.5);

    if (right_red) qglColor(RED);
    else qglColor(WHITE);

    if (!right_out)
    {
        glBegin(GL_LINES);
        glVertex2d(m_engine2_center_n1.x(), m_engine2_center_n1.y());
        glVertex2d(m_engine2_center_n1.x() + (m_engine_n1_radius+POINTER_OVERLAP_LEN) * sin(n1_2_angle), 
                   m_engine2_center_n1.y() + (m_engine_n1_radius+POINTER_OVERLAP_LEN) * -cos(n1_2_angle));
        glEnd();
    }

    glBegin(GL_LINES);
    glVertex2d(m_engine2_center_egt.x(), m_engine2_center_egt.y());
    glVertex2d(m_engine2_center_egt.x() + (m_engine_egt_radius) * sin(egt_2_angle), 
               m_engine2_center_egt.y() + (m_engine_egt_radius) * -cos(egt_2_angle));
    glEnd();

    glLineWidth(1.5);
    qglColor(WHITE);
    
    glBegin(GL_LINE_LOOP);
    glVertex2d(m_engine2_center_n1.x(), n1_box_y);
    glVertex2d(m_engine2_center_n1.x() + n1_box_width, n1_box_y);
    glVertex2d(m_engine2_center_n1.x() + n1_box_width, n1_box_y-m_font_height);
    glVertex2d(m_engine2_center_n1.x(), n1_box_y-m_font_height);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex2d(m_engine2_center_egt.x(), egt_box_y);
    glVertex2d(m_engine2_center_egt.x() + n1_box_width, egt_box_y);
    glVertex2d(m_engine2_center_egt.x() + n1_box_width, egt_box_y-m_font_height);
    glVertex2d(m_engine2_center_egt.x(), egt_box_y-m_font_height);
    glEnd();

    if (m_flightstatus->engine_data[2].reverser_percent >= 0.001)
        drawText(m_engine2_center_n1.x()+ n1_box_width - getTextWidth("REV"), n1_box_y-m_font_height, "REV");

    if (!right_out)
    {
        value = QString::number(n1_2, 'f', 1);
        drawText(m_engine2_center_n1.x() + n1_box_width - getTextWidth(value) - 3,
                 n1_box_y, value);
    }

    value = (right_out) ? "0.0" : QString::number(m_flightstatus->engine_data[2].egt_degrees, 'f', 0);
    drawText(m_engine2_center_egt.x() + n1_box_width - getTextWidth(value) - 3, 
             egt_box_y, value);

    //----- n2 + ff

    QString text = QString::number(m_flightstatus->engine_data[1].n2_percent, 'f', 1);
    drawText(m_engine1_center_n2.x() - getTextWidth(text)*0.5, m_engine1_center_n2.y(), text);
    text = QString::number(m_flightstatus->engine_data[2].n2_percent, 'f', 1);
    drawText(m_engine2_center_n2.x() - getTextWidth(text)*0.5, m_engine2_center_n2.y(), text);

    text = QString::number(((int)m_flightstatus->engine_data[1].ff_kg_per_hour)/10*10);
    drawText(m_engine1_center_ff.x() - getTextWidth(text)*0.5, m_engine1_center_ff.y(), text); 
    text = QString::number(((int)m_flightstatus->engine_data[2].ff_kg_per_hour)/10*10);
    drawText(m_engine2_center_ff.x() - getTextWidth(text)*0.5, m_engine2_center_ff.y(), text); 

    //----- labels

    qglColor(CYAN);
    drawText(m_engine_middle-getTextWidth("N1")/2.0, m_engine1_center_n1.y()+2+m_engine_n1_radius, "N1");
    drawText(m_engine_middle-getTextWidth("EGT")/2.0, m_engine1_center_egt.y()+2+m_engine_egt_radius, "EGT");
    drawText(m_engine_middle-getTextWidth("N2")/2.0, m_engine1_center_n2.y(), "N2");
    drawText(m_engine_middle-getTextWidth("FF")/2.0, m_engine1_center_ff.y(), "FF");

    //----- N1 target bug

    glLineWidth(2.5);
    qglColor(GREEN);

    bool flex_power = m_flightstatus->onground || 
                      (m_fmc_control->fmcAutothrottle().isAPThrottleModeN1Engaged() &&
                       !m_fmc_control->fmcAutothrottle().isAPThrottleN1ClimbModeActive());
    
    bool climb_power = (m_fmc_control->fmcAutothrottle().isAPThrottleModeN1Engaged() &&
                        m_fmc_control->fmcAutothrottle().isAPThrottleN1ClimbModeActive() &&
                        m_fmc_control->fmcAutothrottle().getAPThrottleN1Target() > 1.0);

    if (!m_flightstatus->isReverserOn())
    {
        if (flex_power || climb_power)
        {
            double n1_target = m_fmc_control->fmcAutothrottle().currentClimbThrust();
            if (flex_power) n1_target = m_fmc_control->fmcAutothrottle().currentFlexThrust();
            QString n1_string = QString::number(n1_target, 'f', 1);

            drawText(m_engine1_center_n1.x() + n1_box_width - getTextWidth(n1_string) - 3,
                     n1_box_y-m_font_height, n1_string);
            drawText(m_engine2_center_n1.x() + n1_box_width - getTextWidth(n1_string) - 3,
                     n1_box_y-m_font_height, n1_string);

            //TODO toga correct?
            text =  (climb_power) ? "CLB" : "TOGA";
            drawText(m_engine_middle-getTextWidth(text)/2.0, m_font_height, text);

            double n1_target_angle = n1_target / SCALE_MAX_N1 * n1_angle_range + m_engine_n1_angle_left;

            //TODO make arrows instead of lines
            glBegin(GL_LINES);
            glVertex2d(m_engine1_center_n1.x() + (m_engine_n1_radius) * sin(n1_target_angle), 
                       m_engine1_center_n1.y() + (m_engine_n1_radius) * -cos(n1_target_angle));
            glVertex2d(m_engine1_center_n1.x() + (m_engine_n1_radius+POINTER_OVERLAP_LEN*1.2) * sin(n1_target_angle),
                       m_engine1_center_n1.y() + (m_engine_n1_radius+POINTER_OVERLAP_LEN*1.2) * -cos(n1_target_angle));
            glVertex2d(m_engine2_center_n1.x() + (m_engine_n1_radius) * sin(n1_target_angle), 
                       m_engine2_center_n1.y() + (m_engine_n1_radius) * -cos(n1_target_angle));
            glVertex2d(m_engine2_center_n1.x() + (m_engine_n1_radius+POINTER_OVERLAP_LEN*1.2) * sin(n1_target_angle),
                       m_engine2_center_n1.y() + (m_engine_n1_radius+POINTER_OVERLAP_LEN*1.2) * -cos(n1_target_angle));
            glEnd();
        }
        else if (!m_flightstatus->onground && 
                 m_flightstatus->engine_data[1].throttle_lever_percent <= 0 && 
                 m_flightstatus->engine_data[2].throttle_lever_percent <= 0)
        {
            drawText(m_engine_middle-getTextWidth("IDLE")/2.0, m_font_height, "IDLE");
        }
    }

    //----- relative throttle position bug

    glLineWidth(1.5);
    qglColor(WHITE);

    if (m_fmc_control->fmcAutothrottle().isAPThrottleEngaged())
    {
        double throttle_angle_1 = 
            LIMITMINMAX(n1_1 + m_flightstatus->engine_data[1].throttle_input_percent - 
                        m_flightstatus->engine_data[1].throttle_lever_percent, 0.0, 100.0) / 
            SCALE_MAX_N1 * n1_angle_range + m_engine_n1_angle_left;
        double throttle_angle_2 = 
            LIMITMINMAX(n1_2 + m_flightstatus->engine_data[2].throttle_input_percent - 
                        m_flightstatus->engine_data[2].throttle_lever_percent, 0.0, 100.0) /
            SCALE_MAX_N1 * n1_angle_range + m_engine_n1_angle_left;
        
        glBegin(GL_LINES);
        glVertex2d(m_engine1_center_n1.x() + (m_engine_n1_radius) * sin(throttle_angle_1), 
                   m_engine1_center_n1.y() + (m_engine_n1_radius) * -cos(throttle_angle_1));
        glVertex2d(m_engine1_center_n1.x() + (m_engine_n1_radius+POINTER_OVERLAP_LEN) * sin(throttle_angle_1),
                   m_engine1_center_n1.y() + (m_engine_n1_radius+POINTER_OVERLAP_LEN) * -cos(throttle_angle_1));
        
        glVertex2d(m_engine2_center_n1.x() + (m_engine_n1_radius) * sin(throttle_angle_2), 
                   m_engine2_center_n1.y() + (m_engine_n1_radius) * -cos(throttle_angle_2));
        glVertex2d(m_engine2_center_n1.x() + (m_engine_n1_radius+POINTER_OVERLAP_LEN) * sin(throttle_angle_2),
                   m_engine2_center_n1.y() + (m_engine_n1_radius+POINTER_OVERLAP_LEN) * -cos(throttle_angle_2));
        glEnd();

        double radius = 2;

        QPointF left_end = 
            QPointF(m_engine1_center_n1.x() + (m_engine_n1_radius+POINTER_OVERLAP_LEN+radius) * sin(throttle_angle_1),
                    m_engine1_center_n1.y() + (m_engine_n1_radius+POINTER_OVERLAP_LEN+radius) * -cos(throttle_angle_1));

        GLDraw::drawCircleOffset(radius, -M_PI, M_PI, left_end.x(), left_end.y(), 0.05);

        QPointF right_end = 
            QPointF(m_engine2_center_n1.x() + (m_engine_n1_radius+POINTER_OVERLAP_LEN+radius) * sin(throttle_angle_2), 
                    m_engine2_center_n1.y() + (m_engine_n1_radius+POINTER_OVERLAP_LEN+radius) * -cos(throttle_angle_2));

        GLDraw::drawCircleOffset(radius, -M_PI, M_PI, right_end.x(), right_end.y(), 0.05);
    }
    
    glPopMatrix();
}

/////////////////////////////////////////////////////////////////////////////

void GLECAMWidgetStyleB::drawTexts()
{
    glLineWidth(1.5);

    //----- tat, fuel

    qglColor(CYAN);

    double tat_x = getTextWidth(" ");
    drawText(tat_x, m_font_height, "TAT");

    QString fob_string = QString::number(m_flightstatus->fuelOnBoard()/1000.0, 'f', 1); 
    QString fuel_text_left = QString("TOTAL FUEL "); 
    QString fuel_text_right = QString(" KGS X "); 
    QString fuel_text = QString("%1%2%3").arg(fuel_text_left).arg(fob_string).arg(fuel_text_right);

    double y = m_size.height()-m_font_height;

    drawText(m_size.width()-getTextWidth(fuel_text), y, fuel_text_left);
    drawText(m_size.width()-getTextWidth(fuel_text_right), y, fuel_text_right);
    drawText(m_size.width()-getTextWidth("1000 "), m_size.height(), "1000");
    
    qglColor(WHITE);

    drawText(tat_x + getTextWidth("TAT  "), m_font_height, 
             (m_flightstatus->tat >= 0) ? 
             QString("+%1c").arg(m_flightstatus->tat, 0, 'f', 0) :
             QString("%1c").arg(m_flightstatus->tat, 0, 'f', 0));
    
    drawText(m_size.width()-getTextWidth(fuel_text_right)-getTextWidth(fob_string), y, fob_string);

    //----- gear

    bool draw_gear = false;
    double gear_width = getTextWidth("DOWN")*0.5;

    if (m_flightstatus->isGearUp())
    {
        if (m_gear_was_down_at_last_update)
        {
            m_gear_was_down_at_last_update = false;
            m_gear_up_timer.start();
        }

        if (m_gear_up_timer.elapsed() <= 10000)
        {
            draw_gear = true;
            qglColor(WHITE);
            drawText(m_gear_center.x() - getTextWidth("UP")*0.5, m_gear_center.y(),"UP");
            glBegin(GL_LINE_LOOP);
        }
    }
    else 
    {
        draw_gear = true;
        m_gear_was_down_at_last_update = true;

        if (m_flightstatus->isGearDown())
        {
            qglColor(GREEN);
            drawText(m_gear_center.x() - gear_width, m_gear_center.y(),"DOWN");
            gear_width *= 1.1;
            glBegin(GL_LINE_LOOP);
        }
        else
        {
            qglColor(WHITE);
            gear_width *= 1.1;
            glBegin(GL_QUADS);
        }

    }

    if (draw_gear)
    {
        glVertex2d(m_gear_center.x()-gear_width, m_gear_center.y());
        glVertex2d(m_gear_center.x()+gear_width+1, m_gear_center.y());
        glVertex2d(m_gear_center.x()+gear_width+1, m_gear_center.y()-m_font_height*1.05);
        glVertex2d(m_gear_center.x()-gear_width, m_gear_center.y()-m_font_height*1.05);
        glEnd();

        qglColor(CYAN);
        drawText(m_gear_center.x() - getTextWidth("GEAR")*0.5, 
                 m_gear_center.y() + m_font_height,
                 "GEAR");
    }

    //----- flaps

    if (m_flightstatus->flapsAreUp())
    {
        if (m_flaps_were_down_at_last_update)
        {
            m_flaps_were_down_at_last_update = false;
            m_flaps_up_timer.start();
        }
    }
    else
    {
        m_flaps_were_down_at_last_update = true;
    }

    if (!m_flightstatus->flapsAreUp() || m_flaps_up_timer.elapsed() <= 10000)
    {
        glLineWidth(1.5);
        qglColor(WHITE);

        double width = getTextWidth("X");
        
        double y = m_flaps_upper_center.y() +
                   ((m_flaps_lower_center.y() - m_flaps_upper_center.y()) * (m_flightstatus->flapsPercent()/100.0));

        glBegin(GL_LINE_LOOP);
        glVertex2d(m_flaps_upper_center.x()-width, m_flaps_upper_center.y());
        glVertex2d(m_flaps_upper_center.x()+width, m_flaps_upper_center.y());
        glVertex2d(m_flaps_lower_center.x()+width, m_flaps_lower_center.y());
        glVertex2d(m_flaps_lower_center.x()-width, m_flaps_lower_center.y());
        glEnd();

        glBegin(GL_QUADS);
        glVertex2d(m_flaps_upper_center.x()-width, m_flaps_upper_center.y());
        glVertex2d(m_flaps_upper_center.x()+width, m_flaps_upper_center.y());
        glVertex2d(m_flaps_lower_center.x()+width, y);
        glVertex2d(m_flaps_lower_center.x()-width, y);
        glEnd();

        (m_flightstatus->areFlapsInTransit()) ? qglColor(MAGENTA) : qglColor(GREEN);

        glLineWidth(2.5);
        glBegin(GL_LINES);
        glVertex2d(m_flaps_upper_center.x()-width*1.5, y);
        glVertex2d(m_flaps_upper_center.x()+width*1.5, y);
        glEnd();

        if (m_flightstatus->flaps_degrees > 0.0)
            drawText(m_flaps_upper_center.x()+width*2, y + m_font_height*0.5, 
                     QString::number((int)m_flightstatus->flaps_degrees));

        qglColor(CYAN);

        double x = m_flaps_upper_center.x()-width*2-getTextWidth("F");

        y = m_flaps_upper_center.y() + m_font_height +
            (((m_flaps_lower_center.y() - m_flaps_upper_center.y()) - (m_font_height*5.0)) / 2.0);
        
        drawText(x, y, "F");
        y += m_font_height;
        drawText(x, y, "L");
        y += m_font_height;
        drawText(x, y, "A");
        y += m_font_height;
        drawText(x, y, "P");
        y += m_font_height;
        drawText(x, y, "S");
    }

    //----- texts

    double vs = m_flightstatus->smoothedVS();

    double text_x = m_right_text_ref.x();
    double text_y = m_right_text_ref.y();

    double max_y = m_engine1_center_egt.y() + m_engine_egt_radius;
    
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
        drawText(text_x, text_y, "DOORS");
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
        m_flightstatus->fuelOnBoardPercentage() < 5.0 &&
        m_flightstatus->fuelOnBoard() < 2000)
    {
        drawText(text_x, text_y, "LAND ASAP");
        text_y += m_font_height;
    }

    if (text_y < max_y && !m_flightstatus->onground && m_flightstatus->parking_brake_set) 
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

    if (text_y < max_y && !m_flightstatus->onground && !m_flightstatus->pitot_heat_on)
    {
        drawText(text_x, text_y, "PITOT HEAT OFF");
        text_y += m_font_height;
    }

    if (text_y < max_y && !m_flightstatus->onground && !m_flightstatus->lights_strobe)
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
            drawText(text_x, text_y, "DOORS");
            text_y += m_font_height;
        }
    }

    if (text_y < max_y && 
        !m_flightstatus->onground)
    {
        if (!m_flightstatus->lights_landing)
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
    
    if (text_y < max_y && !m_flightstatus->onground && !m_flightstatus->lights_navigation)
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

    if (text_y < max_y)
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
        drawText(text_x, text_y, "DOORS");
        text_y += m_font_height;
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
}

/////////////////////////////////////////////////////////////////////////////

// void GLECAMWidgetStyleB::enableClipping(GLuint gllist_id)
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

// void GLECAMWidgetStyleB::disableClipping()
// {
//     glDisable(GL_DEPTH_TEST);
//}

/////////////////////////////////////////////////////////////////////////////

void GLECAMWidgetStyleB::drawFPS()
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

