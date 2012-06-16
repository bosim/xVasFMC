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

/*!
  \file    fmc_navdisplay_style_a.cpp
  \author  Alexander Wemmer, alex@wemmer.at
*/

#include "config.h"
#include "navcalc.h"
#include "holding.h"
#include "fmc_control.h"
#include "flightstatus.h"
#include "projection_mercator.h"

#include "defines.h"
#include "fmc_navdisplay_defines.h"
#include "fmc_navdisplay_style_a.h"
#include "fmc_autopilot.h"

/////////////////////////////////////////////////////////////////////////////

#define CFG_RANGES_COLOR "ranges_color"
#define CFG_RANGES_TEXT_COLOR "ranges_text_color"
#define CFG_RANGES_LINE_WIDTH "ranges_line_width"
#define CFG_ARC_RANGES_RING_COUNT "arc_ranges_ring_count"
#define CFG_ROSE_RANGES_RING_COUNT "rose_ranges_ring_count"

#define CFG_VIRT_PLANE_COLOR "virtual_plane_color"
#define CFG_VIRT_PLANE_LINE_WIDTH "virtual_plane_line_width"
#define CFG_VIRT_PLANE_HEIGHT "virtual_plane_height"
#define CFG_VIRT_PLANE_UPPER_VERT_OFFSET "virtual_plane_upper_vert_offset"
#define CFG_VIRT_PLANE_WIDTH_UPPER "virtual_plane_width_upper"
#define CFG_VIRT_PLANE_WIDTH_LOWER "virtual_plane_width_lower"

#define CFG_COMPASS_LINE_COLOR "compass_line_color"
#define CFG_COMPASS_LINE_WIDTH "compass_line_width"
#define CFG_COMPASS_EVE_LINE_LEN "compass_even_line_length"
#define CFG_COMPASS_ODD_LINE_LEN "compass_odd_line_length"
#define CFG_COMPASS_NOSE_LINE_LEN "compass_nose_line_length"
#define CFG_COMPASS_NOSE_LINE_COLOR "compass_nose_line_color"
#define CFG_COMPASS_NOSE_LINE_WIDTH "compass_nose_line_width"
#define CFG_COMPASS_BOX_ADD_TEXT_COLOR "compass_box_add_text_color"
#define CFG_COMPASS_HDG_BUG_LINE_COLOR "compass_hdg_bug_line_color"
#define CFG_COMPASS_HDG_BUG_LINE_WIDTH "compass_hdg_bug_ling_width"
#define CFG_COMPASS_HDG_TRK_FLAG_COLOR "compass_hdg_track_flag_color"
#define CFG_COMPASS_HDG_TRK_FLAG_LINE_WIDTH "compass_hdg_track_flag_line_width"

#define CFG_WIND_ARROW_COLOR "wind_arror_color"
#define CFG_WIND_ARROW_LINE_WIDTH "wind_arror_line_width"
#define CFG_WIND_ARROW_LEN "wind_arror_length"
#define CFG_WIND_ARROW_SPACE "wind_arror_space"

#define CFG_ALT_REACH_LINE_WIDTH "altitude_reach_line_width"
#define CFG_ALT_REACH_LINE_COLOR "altitude_reach_line_color"

// TODO move to config
#define WCA_BUG_SIZE 4
#define HSI_CIRCLE_SIZE 3.5

/////////////////////////////////////////////////////////////////////////////

FMCNavdisplayStyleA::FMCNavdisplayStyleA(ConfigWidgetProvider* config_widget_provider,
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
                                         bool left_side) :
    FMCNavdisplayStyle(parent, main_config, navdisplay_config, tcas_config,
                       fmc_control, projection, size, display_top_offset,
                       max_drawable_y, position_center, dist_scale_factor, left_side),
    m_wca_bug_gllist(0), m_hdg_bug_gllist(0), m_plane_gllist(0),
    m_range_ring_gllist(0), m_compass_circle_gllist(0), m_wpt_item_gllist(0), m_plan_mod_compass_gllist(0),
    m_full_compass_gllist(0), m_full_compass_triangles_gllist(0), m_hsi_gllist(0), m_hsi_offset(0)
{
    MYASSERT(config_widget_provider != 0);

    // setup config

    m_navdisplay_style_config = new Config(cfg_navdisplay_style_filename);
    MYASSERT(m_navdisplay_style_config != 0);
    setupDefaultConfig();
    m_navdisplay_style_config->loadfromFile();
    m_navdisplay_style_config->saveToFile();
    m_navdisplay_config->setValue(CFG_WIND_CORRECTION, 0);

    // set basic values

    reset(size, display_top_offset, max_drawable_y, position_center, dist_scale_factor);

    // compile wca bug list

    m_wca_bug_gllist = glGenLists(1);
    MYASSERT(m_wca_bug_gllist != 0);
    glNewList(m_wca_bug_gllist, GL_COMPILE);
    glBegin(GL_LINE_LOOP);
    glVertex2i(0, 0);
    glVertex2i(WCA_BUG_SIZE, 2*WCA_BUG_SIZE);
    glVertex2i(0, 4*WCA_BUG_SIZE);
    glVertex2i(-WCA_BUG_SIZE, 2*WCA_BUG_SIZE);
    glEnd();
    glEndList();

    // compile compass bug list

    m_hdg_bug_gllist = glGenLists(1);
    MYASSERT(m_hdg_bug_gllist != 0);
    glNewList(m_hdg_bug_gllist, GL_COMPILE);
    glBegin(GL_LINE_LOOP);
    glVertex2d(0.0, 0.0);
    glVertex2d(6, -m_navdisplay_style_config->getDoubleValue(CFG_COMPASS_EVE_LINE_LEN));
    glVertex2d(-6, -m_navdisplay_style_config->getDoubleValue(CFG_COMPASS_EVE_LINE_LEN));
    glEnd();
    glEndList();

    // compile virt plane list

    m_plane_gllist = glGenLists(1);
    MYASSERT(m_plane_gllist != 0);
    glNewList(m_plane_gllist, GL_COMPILE);

    glPushMatrix();
    glTranslated(0, -m_navdisplay_style_config->getDoubleValue(CFG_VIRT_PLANE_UPPER_VERT_OFFSET), 0);

    glBegin(GL_LINES);
    glVertex2d(0, 0);
    glVertex2d(0, +m_navdisplay_style_config->getDoubleValue(CFG_VIRT_PLANE_HEIGHT));

    glVertex2d(-m_navdisplay_style_config->getDoubleValue(CFG_VIRT_PLANE_WIDTH_UPPER),
               m_navdisplay_style_config->getDoubleValue(CFG_VIRT_PLANE_UPPER_VERT_OFFSET));
    glVertex2d(+m_navdisplay_style_config->getDoubleValue(CFG_VIRT_PLANE_WIDTH_UPPER),
               m_navdisplay_style_config->getDoubleValue(CFG_VIRT_PLANE_UPPER_VERT_OFFSET));

    glVertex2d(-m_navdisplay_style_config->getDoubleValue(CFG_VIRT_PLANE_WIDTH_LOWER),
               +m_navdisplay_style_config->getDoubleValue(CFG_VIRT_PLANE_HEIGHT)-
               m_navdisplay_style_config->getDoubleValue(CFG_VIRT_PLANE_WIDTH_LOWER));
    glVertex2d(+m_navdisplay_style_config->getDoubleValue(CFG_VIRT_PLANE_WIDTH_LOWER),
               +m_navdisplay_style_config->getDoubleValue(CFG_VIRT_PLANE_HEIGHT)-
               m_navdisplay_style_config->getDoubleValue(CFG_VIRT_PLANE_WIDTH_LOWER));
    glEnd();

    glPopMatrix();

    glEndList();

    // compile waypoint item list

    m_wpt_item_gllist = glGenLists(1);
    MYASSERT(m_wpt_item_gllist != 0);
    glNewList(m_wpt_item_gllist, GL_COMPILE);
    glLineWidth(1.6);
    glBegin(GL_LINE_LOOP);
    glVertex2i(0, -4);
	glVertex2i(4, 0);
	glVertex2i(0, +4);
	glVertex2i(-4, 0);
    glEnd();
    glEndList();

    // compile airport symbol list

    m_airport_item_gllist = glGenLists(1);
    MYASSERT(m_airport_item_gllist != 0);
    glNewList(m_airport_item_gllist, GL_COMPILE);
    glLineWidth(1.3);
    uint len = 5;
    double diagonal = len / sqrt(2.0);
    glBegin(GL_LINES);
    glVertex2i(0, -len);
    glVertex2i(0, +len);
    glVertex2i(-len, 0);
    glVertex2i(+len, 0);
    glVertex2d(-diagonal, -diagonal);
    glVertex2d(+diagonal, +diagonal);
    glVertex2d(+diagonal, -diagonal);
    glVertex2d(-diagonal, +diagonal);
    glEnd();
    glEndList();

    // compile VOR/DME symbol list

    m_vor_dme_item_gllist = glGenLists(1);
    MYASSERT(m_vor_dme_item_gllist != 0);
    glNewList(m_vor_dme_item_gllist, GL_COMPILE);
    glLineWidth(1.0);

    double vor_len = 7;
    double vor_len_part = 3;
    GLDraw::drawCircle(vor_len_part, -M_PI, M_PI);
    glBegin(GL_LINES);
    glVertex2d(0, +vor_len_part);
    glVertex2d(0, +vor_len);
    glVertex2d(0, -vor_len_part);
    glVertex2d(0, -vor_len);
    glVertex2d(-vor_len_part, 0);
    glVertex2d(-vor_len, 0);
    glVertex2d(+vor_len_part, 0);
    glVertex2d(+vor_len, 0);
    glEnd();
    glEndList();

    // compile VOR w/o DME symbol list

    m_vor_wo_dme_item_gllist = glGenLists(1);
    MYASSERT(m_vor_wo_dme_item_gllist != 0);
    glNewList(m_vor_wo_dme_item_gllist, GL_COMPILE);
    glLineWidth(1.0);

    glBegin(GL_LINES);
    glVertex2d(0, +vor_len);
    glVertex2d(0, -vor_len);
    glVertex2d(-vor_len, 0);
    glVertex2d(+vor_len, 0);
    glEnd();
    glEndList();

    // compile NDB symbol list

    double ndb_len = 5;
    double rad_30 = Navcalc::toRad(30);

    m_ndb_item_gllist = glGenLists(1);
    MYASSERT(m_ndb_item_gllist != 0);
    glNewList(m_ndb_item_gllist, GL_COMPILE);
    glLineWidth(1.0);

    glBegin(GL_LINE_LOOP);
    glVertex2d(0, -ndb_len);
    glVertex2d( ndb_len*cos(rad_30), ndb_len*sin(rad_30));
    glVertex2d(-ndb_len*cos(rad_30), ndb_len*sin(rad_30));
    glEnd();

    glEndList();

    // compile TOC item

    double arrow_len = 7;

    m_toc_item_gllist = glGenLists(1);
    MYASSERT(m_toc_item_gllist != 0);
    glNewList(m_toc_item_gllist, GL_COMPILE);
    glLineWidth(2.0);

    glBegin(GL_LINES);
    glVertex2d(0, 0);
    glVertex2d(-20, 0);
    glVertex2d(-20, 0);
    glVertex2d(-30, 10);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2d(0, 0);
    glVertex2d(-arrow_len, -arrow_len/2.0);
    glVertex2d(-arrow_len, +arrow_len/2.0);
    glEnd();

    glEndList();

    // compile EOD item

    m_eod_item_gllist = glGenLists(1);
    MYASSERT(m_eod_item_gllist != 0);
    glNewList(m_eod_item_gllist, GL_COMPILE);
    glLineWidth(2.0);

    glBegin(GL_LINES);
    glVertex2d(0, 0);
    glVertex2d(-20, 0);
    glVertex2d(-20, 0);
    glVertex2d(-30, -10);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2d(0, 0);
    glVertex2d(-arrow_len, -arrow_len/2.0);
    glVertex2d(-arrow_len, +arrow_len/2.0);
    glEnd();

    glEndList();

    // compile TOD item

    m_tod_item_gllist = glGenLists(1);
    MYASSERT(m_tod_item_gllist != 0);
    glNewList(m_tod_item_gllist, GL_COMPILE);
    glLineWidth(2.0);

    glBegin(GL_LINES);
    glVertex2d(30, 10);
    glVertex2d(15, 0);
    glVertex2d(15, 0);
    glVertex2d(0, 0);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2d(30, 10);
    glVertex2d(30, -arrow_len+10);
    glVertex2d(30-arrow_len, 10);
    glEnd();

    glEndList();

    //config_widget_provider->registerConfigWidget("ND Style A", m_navdisplay_style_config);
}

/////////////////////////////////////////////////////////////////////////////

FMCNavdisplayStyleA::~FMCNavdisplayStyleA()
{
    glDeleteLists(m_wca_bug_gllist, 1);
    glDeleteLists(m_hdg_bug_gllist, 1);
    glDeleteLists(m_plane_gllist, 1);
    glDeleteLists(m_range_ring_gllist, 1);
    glDeleteLists(m_compass_circle_gllist, 1);
    glDeleteLists(m_wpt_item_gllist, 1);
    glDeleteLists(m_plan_mod_compass_gllist, 1);
    glDeleteLists(m_full_compass_gllist, 1);
    glDeleteLists(m_full_compass_triangles_gllist, 1);
    glDeleteLists(m_hsi_gllist, 1);

    m_navdisplay_style_config->saveToFile();
    delete m_navdisplay_style_config;
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleA::setupDefaultConfig()
{
    m_navdisplay_style_config->setValue(CFG_RANGES_COLOR, "white");
    m_navdisplay_style_config->setValue(CFG_RANGES_TEXT_COLOR, "cyan");
    m_navdisplay_style_config->setValue(CFG_RANGES_LINE_WIDTH, "1.1");
    m_navdisplay_style_config->setValue(CFG_ARC_RANGES_RING_COUNT, 4);
    m_navdisplay_style_config->setValue(CFG_ROSE_RANGES_RING_COUNT, 2);

    m_navdisplay_style_config->setValue(CFG_VIRT_PLANE_COLOR, "yellow");
    m_navdisplay_style_config->setValue(CFG_VIRT_PLANE_LINE_WIDTH, "3.0");
    m_navdisplay_style_config->setValue(CFG_VIRT_PLANE_HEIGHT, "26");
    m_navdisplay_style_config->setValue(CFG_VIRT_PLANE_UPPER_VERT_OFFSET, 8);
    m_navdisplay_style_config->setValue(CFG_VIRT_PLANE_WIDTH_UPPER, "12");
    m_navdisplay_style_config->setValue(CFG_VIRT_PLANE_WIDTH_LOWER, "5");

    m_navdisplay_style_config->setValue(CFG_COMPASS_LINE_COLOR, "white");
    m_navdisplay_style_config->setValue(CFG_COMPASS_LINE_WIDTH, "2.1");
    m_navdisplay_style_config->setValue(CFG_COMPASS_EVE_LINE_LEN, "8");
    m_navdisplay_style_config->setValue(CFG_COMPASS_ODD_LINE_LEN, "4");
    m_navdisplay_style_config->setValue(CFG_COMPASS_NOSE_LINE_LEN, "12");
    m_navdisplay_style_config->setValue(CFG_COMPASS_NOSE_LINE_COLOR, "yellow");
    m_navdisplay_style_config->setValue(CFG_COMPASS_NOSE_LINE_WIDTH, "3.0");
    m_navdisplay_style_config->setValue(CFG_COMPASS_BOX_ADD_TEXT_COLOR, "cyan");
    m_navdisplay_style_config->setValue(CFG_COMPASS_HDG_BUG_LINE_COLOR, "cyan");
    m_navdisplay_style_config->setValue(CFG_COMPASS_HDG_BUG_LINE_WIDTH, "1.1");
    m_navdisplay_style_config->setValue(CFG_COMPASS_HDG_TRK_FLAG_COLOR, "#00ff00");
    m_navdisplay_style_config->setValue(CFG_COMPASS_HDG_TRK_FLAG_LINE_WIDTH, "1.3");

    m_navdisplay_style_config->setValue(CFG_WIND_ARROW_COLOR, "#00ff00");
    m_navdisplay_style_config->setValue(CFG_WIND_ARROW_LINE_WIDTH, "1.1");
    m_navdisplay_style_config->setValue(CFG_WIND_ARROW_LEN, "10");
    m_navdisplay_style_config->setValue(CFG_WIND_ARROW_SPACE, "15");

    m_navdisplay_style_config->setValue(CFG_ALT_REACH_LINE_WIDTH, "1.5");
    m_navdisplay_style_config->setValue(CFG_ALT_REACH_LINE_COLOR, "cyan");
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleA::reset(const QSize& size,
                                const double& display_top_offset,
                                const int& max_drawable_y,
                                const QPointF& position_center,
                                const double& dist_scale_factor)
{
    FMCNavdisplayStyle::reset(size, display_top_offset, max_drawable_y, position_center, dist_scale_factor);

    // compile range ring list

    if (m_range_ring_gllist != 0) glDeleteLists(m_range_ring_gllist, 1);
    m_range_ring_gllist = glGenLists(1);
    MYASSERT(m_range_ring_gllist != 0);
    glNewList(m_range_ring_gllist, GL_COMPILE);

    int ring_count = rangeRingCount();
    double radius_inc = max_drawable_y / (double)ring_count;
    double radius = 0;

    for(int count=1; count < ring_count; ++count)
    {
        radius += radius_inc;

        if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_NAV_ARC)
            GLDraw::drawCircle(radius);
        else
            GLDraw::drawCircle(radius, -M_PI, M_PI);
    }

    glEndList();

    // compile compass circle list

    if (m_compass_circle_gllist) glDeleteLists(m_compass_circle_gllist, 1);
    m_compass_circle_gllist = glGenLists(1);
    MYASSERT(m_compass_circle_gllist != 0);
    glNewList(m_compass_circle_gllist, GL_COMPILE);

    if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_NAV_ARC)
        GLDraw::drawCircle(max_drawable_y);
    else
        GLDraw::drawCircle(max_drawable_y, -M_PI, M_PI);

    glEndList();

    // compile plan mode compass list

    if (m_plan_mod_compass_gllist) glDeleteLists(m_plan_mod_compass_gllist, 1);
    m_plan_mod_compass_gllist = glGenLists(1);
    MYASSERT(m_plan_mod_compass_gllist != 0);
    glNewList(m_plan_mod_compass_gllist, GL_COMPILE);

    double triangle_len = 9;

    drawText(-(getTextWidth("N")/2.0), -m_max_drawable_y + m_font_height + triangle_len, "N");
    drawText(-(getTextWidth("S")/2.0), +m_max_drawable_y - triangle_len, "S");
    drawText(-m_max_drawable_y + triangle_len, m_font_height/2.0, " W");
    drawText(m_max_drawable_y - getTextWidth("E ") - triangle_len, m_font_height/2.0, "E");

    glBegin(GL_TRIANGLES);
    glVertex2d(0, -m_max_drawable_y);
    glVertex2d(-triangle_len/2.0, -m_max_drawable_y+triangle_len);
    glVertex2d(+triangle_len/2.0, -m_max_drawable_y+triangle_len);
    glVertex2d(0, +m_max_drawable_y);
    glVertex2d(-triangle_len/2.0, +m_max_drawable_y-triangle_len);
    glVertex2d(+triangle_len/2.0, +m_max_drawable_y-triangle_len);
    glVertex2d(-m_max_drawable_y, 0);
    glVertex2d(-m_max_drawable_y+triangle_len, -triangle_len/2.0);
    glVertex2d(-m_max_drawable_y+triangle_len, +triangle_len/2.0);
    glVertex2d(+m_max_drawable_y, 0);
    glVertex2d(+m_max_drawable_y-triangle_len, -triangle_len/2.0);
    glVertex2d(+m_max_drawable_y-triangle_len, +triangle_len/2.0);
    glEnd();

    glEndList();

    // compile full compass list

    if (m_full_compass_gllist) glDeleteLists(m_full_compass_gllist, 1);
    m_full_compass_gllist = glGenLists(1);
    MYASSERT(m_full_compass_gllist != 0);
    glNewList(m_full_compass_gllist, GL_COMPILE);

    double hdg_text_ytrans = m_navdisplay_style_config->getDoubleValue(CFG_COMPASS_EVE_LINE_LEN) + 3;

    int alpha_inc = 5;
    int alpha = 0;
    for (; alpha <360; alpha += alpha_inc)
    {
        glLineWidth(m_navdisplay_style_config->getDoubleValue(CFG_COMPASS_LINE_WIDTH));

        if (alpha % 10 == 0)
        {
            // draw scale line
            glBegin(GL_LINES);
            glVertex2d(0, -m_max_drawable_y);
            glVertex2d(0, -m_max_drawable_y - m_navdisplay_style_config->getDoubleValue(CFG_COMPASS_EVE_LINE_LEN));
            glEnd();

            // draw scale text
            int hdg = alpha/10;
            QString hdg_text = QString::number(hdg);
            glLineWidth(1.1);
            if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_NAV_ARC || hdg % 3 == 0)
                drawText(-getTextWidth(hdg_text)/2.0, -m_max_drawable_y - hdg_text_ytrans, hdg_text);
        }
        else
        {
            glBegin(GL_LINES);
            glVertex2d(0, -m_max_drawable_y);
            glVertex2d(0, -m_max_drawable_y - m_navdisplay_style_config->getDoubleValue(CFG_COMPASS_ODD_LINE_LEN));
            glEnd();
        }

        glRotated(alpha_inc, 0, 0, 1);
    }

    glEndList();

    // compile full compass triangles list

    if (m_full_compass_triangles_gllist) glDeleteLists(m_full_compass_triangles_gllist, 1);
    m_full_compass_triangles_gllist = glGenLists(1);
    MYASSERT(m_full_compass_triangles_gllist != 0);
    glNewList(m_full_compass_triangles_gllist, GL_COMPILE);

    triangle_len = m_navdisplay_style_config->getDoubleValue(CFG_COMPASS_EVE_LINE_LEN)+1;
    alpha_inc = 45;
    alpha=0;
    for (; alpha <360; alpha += alpha_inc)
    {
        glRotated(alpha_inc, 0, 0, 1);
        glBegin(GL_TRIANGLES);
        glVertex2d(0, -m_max_drawable_y);
        glVertex2d(-triangle_len/2.0, -m_max_drawable_y-triangle_len);
        glVertex2d(+triangle_len/2.0, -m_max_drawable_y-triangle_len);
        glEnd();
    }

    glEndList();

    // compile HSI mode list

    if (m_hsi_gllist != 0) glDeleteLists(m_hsi_gllist, 1);
    m_hsi_gllist = glGenLists(1);
    MYASSERT(m_hsi_gllist != 0);
    glNewList(m_hsi_gllist, GL_COMPILE);

    glPushMatrix();
    glLineWidth(3.0);

    m_hsi_offset = m_max_drawable_y * 2.0/16.0;

    glBegin(GL_LINES);
    glVertex2d(0, -m_max_drawable_y);
    glVertex2d(0, -m_max_drawable_y/2.0 + m_hsi_offset);
    glVertex2d(0, +m_max_drawable_y/2.0 - m_hsi_offset);
    glVertex2d(0, + m_max_drawable_y);
    glVertex2d(-m_hsi_offset, -m_max_drawable_y/2.0);
    glVertex2d(+m_hsi_offset, -m_max_drawable_y/2.0);
    glEnd();

    glLineWidth(2.0);
    m_parent->qglColor(Qt::white);

    glTranslated(-2*m_hsi_offset, 0, 0);
    GLDraw::drawCircle(HSI_CIRCLE_SIZE, -M_PI, M_PI);
    glTranslated(-2*m_hsi_offset, 0, 0);
    GLDraw::drawCircle(HSI_CIRCLE_SIZE, -M_PI, M_PI);
    glTranslated(6*m_hsi_offset, 0, 0);
    GLDraw::drawCircle(HSI_CIRCLE_SIZE, -M_PI, M_PI);
    glTranslated(2*m_hsi_offset, 0, 0);
    GLDraw::drawCircle(HSI_CIRCLE_SIZE, -M_PI, M_PI);

    glPopMatrix();

    glEndList();
}

/////////////////////////////////////////////////////////////////////////////

int FMCNavdisplayStyleA::rangeRingCount() const
{
    if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_NAV_ARC)
        return m_navdisplay_style_config->getIntValue(CFG_ARC_RANGES_RING_COUNT);

    return m_navdisplay_style_config->getIntValue(CFG_ROSE_RANGES_RING_COUNT);
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleA::drawAvionicsOff()
{
    m_parent->qglColor(RED);
    drawText(m_size.width()/2.0, m_size.height()/2.0, "ATT");
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleA::drawAirportRunways(const FlightRoute& route, const double& north_track_rotation)
{
    glLoadIdentity();
    glTranslated(m_position_center.x(), m_position_center.y(), 0.0);

    // correct the display position for projection offset
    moveToCorrectedProjectionPosition(north_track_rotation);

    // rotate for heading
    glRotated(-north_track_rotation, 0, 0, 1.0);

    // draw departure airport runways
    if (route.departureAirport() != 0)
        drawAirport(*route.departureAirport(), north_track_rotation, Qt::magenta, false, true);

    // draw destination airport runways
    if (route.destinationAirport() != 0)
        drawAirport(*route.destinationAirport(), north_track_rotation, Qt::magenta, false, true);

    // draw alternate airport runways
    if (m_fmc_control->alternateRoute().destinationAirport() != 0)
        drawAirport(*m_fmc_control->alternateRoute().destinationAirport(), north_track_rotation, Qt::magenta, false, true);
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleA::drawRouteNormalMode(const FlightRoute& route, const double& north_track_rotation)
{
    m_navdisplay_config->setValue(CFG_WIND_CORRECTION, 0);

    if (fabs(m_flightstatus->tas - m_last_holding_gllist_calculation_speed_kts) >= 10.0) compileHoldingGLLists();

    glLoadIdentity();
    glTranslated(m_position_center.x(), m_position_center.y(), 0.0);

    // correct the display position for projection offset
    moveToCorrectedProjectionPosition(north_track_rotation);

    // rotate for heading
    glRotated(-north_track_rotation, 0, 0, 1.0);

    //setup points
    const Waypoint* active_wpt = route.activeWaypoint();
    int ades_wpt_index = route.destinationAirportIndex();
    int active_wpt_index = route.activeWaypointIndex();
    int wpt_index = active_wpt_index;

    // loop through the route waypoints

    bool do_stiple_legs = false;

    if (m_fmc_control->fmcAutoPilot().isNAVCoupled() ||
        (m_fmc_control->fmcAutoPilot().isTakeoffModeActiveLateral() &&
         m_fmc_control->fmcAutoPilot().lateralModeArmed() == FMCAutopilot::LATERAL_MODE_LNAV))
    {
        do_stiple_legs = false;
    }
    else
    {
        do_stiple_legs = true;
    }

    bool reached_active_wpt = false;

    double max_display_range_quad = m_navdisplay_config->getDoubleValue(CFG_ROUTE_DISPLAY_MAX_DISTANCE_NM) *
                                    m_navdisplay_config->getDoubleValue(CFG_ROUTE_DISPLAY_MAX_DISTANCE_NM);
    bool stop_drawing = false;

    bool temp_route = route.flag() == Route::FLAG_TEMPORARY;
    if (temp_route) do_stiple_legs = true;

    WaypointPtrListIterator iter(route.waypointList());
    while(iter.hasNext() && !stop_drawing)
    {
        const Waypoint* wpt = iter.next();

        QColor leg_color = Qt::green;

        // missed approach / go around route drawing
        if (wpt_index > ades_wpt_index && active_wpt_index <= ades_wpt_index)
        {
            if (!m_fmc_control->isMissedAppWaypointVisibleOnCDU(m_left_side) &&
                !m_fmc_control->isMissedAppWaypointVisibleOnCDU(!m_left_side)) //TODO work around as the view wpt is shared
            {
                break;
            }
            
            leg_color = Qt::cyan;
        }

        if (temp_route) leg_color = Qt::yellow;

        QColor wpt_color = Qt::green;
        if (wpt == route.activeWaypoint() || temp_route) wpt_color = Qt::white;

        // test every third waypoint for distance
        if (reached_active_wpt && wpt_index % 3 == 0)
            stop_drawing = (wpt->x() * wpt->x()) + (wpt->y() * wpt->y()) > max_display_range_quad;

        if(!stop_drawing && iter.hasNext()) drawLeg(wpt, iter.peekNext(), north_track_rotation, leg_color, do_stiple_legs);

        reached_active_wpt |= (wpt == active_wpt || active_wpt == 0);

        drawWaypoint(wpt, north_track_rotation, wpt_color, leg_color, do_stiple_legs, reached_active_wpt);

        ++wpt_index;
    }

    // draw tuned VORs

    if (!m_flightstatus->nav1.id().isEmpty() && !m_flightstatus->nav1_has_loc)
        drawVor(m_flightstatus->nav1, !m_flightstatus->nav1_distance_nm.isEmpty(), north_track_rotation, Qt::cyan);
    if (!m_flightstatus->nav2.id().isEmpty() && !m_flightstatus->nav2_has_loc)
        drawVor(m_flightstatus->nav2, !m_flightstatus->nav2_distance_nm.isEmpty(), north_track_rotation, Qt::cyan);

    if (active_wpt != 0 && !active_wpt->holding().isActive() && m_fmc_control->fmcAutoPilot().isNAVCoupled())
    {
        // draw TOC/EOD
        if (m_fmc_control->normalRoute().altReachWpt().isValid())
        {
            drawSpecialWaypoint(&m_fmc_control->normalRoute().altReachWpt(), north_track_rotation,
                                m_navdisplay_style_config->getColorValue(CFG_ALT_REACH_LINE_COLOR));
        }

        // draw TOD
        if (m_fmc_control->normalRoute().todWpt().isValid() && m_fmc_data.distanceToTODNm() > 0.0)
        {
            drawSpecialWaypoint(&m_fmc_control->normalRoute().todWpt(), north_track_rotation,
                                m_navdisplay_style_config->getColorValue(CFG_ALT_REACH_LINE_COLOR));
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleA::drawRouteMapMode(const FlightRoute& route)
{
    if (fabs(m_flightstatus->tas - m_last_holding_gllist_calculation_speed_kts) >= 10.0) compileHoldingGLLists();

    glLoadIdentity();
    glTranslated(m_position_center.x(), m_position_center.y(), 0.0);

    // correct the display position for projection offset
    //moveToCorrectedProjectionPosition(0);

    //setup points

    const Waypoint* view_wpt = m_fmc_control->normalRoute().viewWpt();
    if (view_wpt == 0) return;

    glTranslated(-scaleXY(view_wpt->x()), -scaleXY(view_wpt->y()), 0);

    // loop through the route waypoints

    bool temp_route = route.flag() == Route::FLAG_TEMPORARY;

    int active_wpt_index = route.activeWaypointIndex();
    int wpt_index = active_wpt_index;
    int ades_wpt_index = route.destinationAirportIndex();

    WaypointPtrListIterator iter(route.waypointList());
    while(iter.hasNext())
    {
        const Waypoint* wpt = iter.next();

        QColor leg_color = Qt::green;

        // missed approach / go around route drawing
        if (wpt_index > ades_wpt_index && 
            active_wpt_index <= ades_wpt_index)
        {
            if (!m_fmc_control->isMissedAppWaypointVisibleOnCDU(m_left_side) &&
                !m_fmc_control->isMissedAppWaypointVisibleOnCDU(!m_left_side)) //TODO work around as the view wpt is shared
            {
                break;
            }

            leg_color = Qt::cyan;
        }

        if (temp_route) leg_color = Qt::yellow;

        QColor wpt_color = Qt::cyan;
        if (wpt == route.activeWaypoint() || temp_route) wpt_color = Qt::white;

        if(iter.hasNext()) drawLeg(wpt, iter.peekNext(), 0, leg_color, false);

        drawWaypoint(wpt, 0, wpt_color, leg_color, false, true);

        ++wpt_index;
    }

    // temp route

    if (!temp_route && m_fmc_control->temporaryRoute().count() > 0)
    {
        WaypointPtrListIterator iter(m_fmc_control->temporaryRoute().waypointList());
        while(iter.hasNext())
        {
            const Waypoint* wpt = iter.next();
            if(iter.hasNext()) drawLeg(wpt, iter.peekNext(), 0, Qt::yellow, true);
            drawWaypoint(wpt, 0, Qt::white, Qt::yellow, true, false);
        }
    }

    // draw virt plane

    glTranslated(+scaleXY(m_flightstatus->current_position_smoothed.x()),
                 +scaleXY(m_flightstatus->current_position_smoothed.y()), 0.0);

    glRotated(m_flightstatus->smoothedTrueHeading(), 0, 0, 1);

    m_parent->qglColor(m_navdisplay_style_config->getColorValue(CFG_VIRT_PLANE_COLOR));
    glLineWidth(m_navdisplay_style_config->getDoubleValue(CFG_VIRT_PLANE_LINE_WIDTH));
    glCallList(m_plane_gllist);
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleA::drawSpecialWaypoint(const Waypoint* wpt,
                                              const double& north_track_rotation,
                                              const QColor& color)
{
    MYASSERT(wpt != 0);

    glPushMatrix();

    // rotate for heading

    glTranslated(scaleXY(wpt->x()), scaleXY(wpt->y()), 0.0);
    glRotated(north_track_rotation, 0, 0, 1.0);
    m_parent->qglColor(color);

    if (wpt->flag() == Waypoint::FLAG_TOP_OF_CLIMB)
        glCallList(m_toc_item_gllist);
    else if (wpt->flag() == Waypoint::FLAG_END_OF_DESCENT)
        glCallList(m_eod_item_gllist);
    else if (wpt->flag() == Waypoint::FLAG_TOP_OF_DESCENT)
        glCallList(m_tod_item_gllist);

    glPopMatrix();
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleA::drawWaypoint(const Waypoint* wpt,
                                       const double& north_track_rotation,
                                       const QColor& wpt_color,
                                       const QColor& holding_color,
                                       bool do_stiple,
                                       bool draw_restrictions)
{
    MYASSERT(wpt != 0);

    glPushMatrix();

    // rotate for heading

    glTranslated(scaleXY(wpt->x()), scaleXY(wpt->y()), 0.0);
    glRotated(north_track_rotation, 0, 0, 1.0);

    // draw waypoint

    m_parent->qglColor(wpt_color);
    glLineWidth(1.1);
    glCallList(m_wpt_item_gllist);
    drawText(5.0, 0, wpt->id());

    // draw restrictions

    bool has_restriction = 
        wpt->restrictions().altitudeRestrictionFt() != 0 ||
        wpt->restrictions().speedRestrictionKts() != 0;                           

    if (has_restriction)
    {
        m_parent->qglColor(Qt::magenta);
        GLDraw::drawCircle(5, -M_PI, +M_PI, 0.1f);

        if (draw_restrictions &&
            m_fmc_control->showConstrains(m_left_side) &&
            wpt->asAirport() == 0)
        {
            double y = m_font_height;
            
            if (wpt->restrictions().altitudeRestrictionFt() != 0)
            {
                drawText(5.0, y, wpt->restrictions().altitudeRestrictionText()+"FT");
                y += m_font_height;
            }
            
            if (wpt->restrictions().speedRestrictionKts() != 0)
            {
                drawText(5.0, y, wpt->restrictions().speedRestrictionText()+"KT");
            }
        }
    }

    // draw holding

    if (wpt->holding().isValid())
    {
        m_parent->qglColor(holding_color);
        glLineWidth(1.5);

        if (do_stiple)
        {
            glLineStipple(4, 0xAAAA); // 0xAAAA = 1010101010101010
            glEnable(GL_LINE_STIPPLE);
        }

        double rotate = wpt->holding().holdingTrack() - north_track_rotation;
        glRotated(rotate, 0, 0, 1);

        if (wpt->holding().isLeftHolding()) glCallList(m_left_holding_item_gllist);
        else                                glCallList(m_right_holding_item_gllist);

        if (do_stiple) glDisable(GL_LINE_STIPPLE);
    }

    glPopMatrix();
};

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleA::drawLeg(const Waypoint* from_wpt,
                                  const Waypoint* to_wpt,
                                  const double& north_track_rotation,
                                  const QColor& leg_color,
                                  bool do_stiple)
{
    Q_UNUSED(north_track_rotation);
    MYASSERT(from_wpt != 0);
    MYASSERT(to_wpt != 0);

    glPushMatrix();

    // rotate for heading

    glTranslated(scaleXY(from_wpt->x()), scaleXY(from_wpt->y()), 0.0);

    // draw leg

    m_parent->qglColor(leg_color);
    glLineWidth(1.6);

    if (do_stiple)
    {
        glLineStipple(4, 0xAAAA); // 0xAAAA = 1010101010101010
        glEnable(GL_LINE_STIPPLE);
    }

    glBegin(GL_LINES);
    glVertex2d(0, 0);
    glVertex2d(scaleXY(to_wpt->x()) - scaleXY(from_wpt->x()),
               scaleXY(to_wpt->y()) - scaleXY(from_wpt->y()));
    glEnd();

    if (do_stiple) glDisable(GL_LINE_STIPPLE);

    glPopMatrix();
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleA::drawRanges()
{
    if (m_navdisplay_config->getIntValue(CFG_DRAW_RANGE_RINGS) == 0) return;

    glLoadIdentity();
    glTranslated(m_position_center.x(), m_position_center.y(), 0);

    m_parent->qglColor(m_navdisplay_style_config->getColorValue(CFG_RANGES_COLOR));
    glLineWidth(m_navdisplay_style_config->getDoubleValue(CFG_RANGES_LINE_WIDTH));

    glLineStipple(4, 0xAAAA); // 0xAAAA = 1010101010101010
    glEnable(GL_LINE_STIPPLE);
    glCallList(m_range_ring_gllist);
    glDisable(GL_LINE_STIPPLE);

    // separator lines + range texts

    glLineWidth(1.0);
    m_parent->qglColor(m_navdisplay_style_config->getColorValue(CFG_RANGES_TEXT_COLOR));

    int ring_count = rangeRingCount();

    double draw_range_inc = m_max_drawable_y / (double)ring_count;
    double real_range_inc = m_fmc_control->getNDRangeNM(m_left_side) / (double)ring_count;

    double draw_range = draw_range_inc;
    double real_range = real_range_inc;

    int start_count = 0;
    int max_count = 0;
    double rotate = 0;

    if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_NAV_ARC)
    {
        start_count = 1;
        max_count = ring_count-2;
        rotate = 75;
        draw_range += draw_range_inc;
        real_range += real_range_inc;
    }
    else
    {
        start_count = 0;
        max_count = ring_count-1;
        rotate = 135;
        real_range_inc /= 2;
        real_range = real_range_inc;
    }

    for (int counter=start_count; counter <= max_count; ++counter)
    {
        // draw range texts

        double dummy = 0.0;
        QString range_text = QString("%1").arg(real_range, 0, 'f', (modf(real_range, &dummy)*10 == 0 ? 0 : 1));
        double y_offset = draw_range - (m_font_height / 2.0);

        glRotated(-rotate, 0.0, 0.0, 1.0);
        glTranslated(0, -y_offset, 0);
        glRotated(+rotate, 0.0, 0.0, 1.0);
        drawText(0, 0, range_text);
        glRotated(-rotate, 0.0, 0.0, 1.0);
        glTranslated(0, +y_offset, 0);

        if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_NAV_ARC)
        {
            glRotated(+150.0, 0.0, 0.0, 1.0);
            glTranslated(0, -y_offset, 0);
            glRotated(-rotate, 0.0, 0.0, 1.0);
            drawText(-getTextWidth(range_text), 0, range_text);
            glRotated(+rotate, 0.0, 0.0, 1.0);
            glTranslated(0, +y_offset, 0);
            glRotated(-rotate, 0.0, 0.0, 1.0);
        }
        else
        {
            glRotated(+rotate, 0.0, 0.0, 1.0);
        }

        draw_range += draw_range_inc;
        real_range += real_range_inc;
    }
};

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleA::drawPlanePoly()
{
    glLoadIdentity();
    glTranslated(m_position_center.x(), m_position_center.y(), 0);
    m_parent->qglColor(m_navdisplay_style_config->getColorValue(CFG_VIRT_PLANE_COLOR));
    glLineWidth(m_navdisplay_style_config->getDoubleValue(CFG_VIRT_PLANE_LINE_WIDTH));
    glCallList(m_plane_gllist);

    // lateral offset

    if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_NAV_PLAN ||
        m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_VOR_ROSE ||
        m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_ILS_ROSE) return;

    const Waypoint* active_wpt = m_fmc_control->normalRoute().activeWaypoint();

    if (active_wpt != 0)
    {
        if ((active_wpt->holding().isValid() && active_wpt->holding().status() != Holding::STATUS_INACTIVE) ||
            !m_fmc_control->fmcAutoPilot().isNAVCoupled())
        {
            // suppress cross track distance display when in a holding or when not coupled
        }
        else
        {
            double cross_track_dist = m_fmc_data.crossTrackDistanceNm();
            const Waypoint* prev_wpt = m_fmc_control->normalRoute().previousWaypoint();

            if (fabs(cross_track_dist) > NAVDISPLAY_TRACK_OFFSET_LIMIT &&
                m_fmc_control->fmcAutoPilot().isNAVHoldActive() &&
                prev_wpt != 0 && !prev_wpt->restrictions().hasOverflyRestriction())
            {
                glTranslated(0, m_navdisplay_style_config->getDoubleValue(CFG_VIRT_PLANE_HEIGHT) - (m_font_height/2.0), 0);
                m_parent->qglColor(Qt::white);

                if (cross_track_dist < 0.0)
                {
                    QString offset_text = QString("%1L").arg(-cross_track_dist, 0, 'f', 2);
                    if (cross_track_dist > -1.0) offset_text = offset_text.mid(1);

                    drawText(-m_navdisplay_style_config->getDoubleValue(CFG_VIRT_PLANE_WIDTH_LOWER)*1.5 -
                             getTextWidth(offset_text), 0, offset_text);
                }
                else
                {
                    QString offset_text = QString("%1R").arg(cross_track_dist, 0, 'f', 2);
                    if (cross_track_dist < 1.0) offset_text = offset_text.mid(1);

                    drawText(m_navdisplay_style_config->getDoubleValue(CFG_VIRT_PLANE_WIDTH_LOWER)*1.5, 0, offset_text);
                }
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleA::drawCompass(double north_track_rotation)
{
    glLoadIdentity();

    north_track_rotation -= m_flightstatus->magvar;
    glTranslated(m_position_center.x(), m_position_center.y(), 0);

    // TODO draw approach type texts (VOR/ILS/...)
    // char approach_type_text[4];
    // drawText(m_position_center.x() - getTextWidth(approach_type_text)/2.0, m_font_height, approach_type_text);

    // draw nose direction flag

    m_parent->qglColor(m_navdisplay_style_config->getColorValue(CFG_COMPASS_NOSE_LINE_COLOR));
    glLineWidth(m_navdisplay_style_config->getDoubleValue(CFG_COMPASS_NOSE_LINE_WIDTH));

    glBegin(GL_LINES);
    glVertex2d(0, - m_max_drawable_y + m_navdisplay_style_config->getDoubleValue(CFG_COMPASS_LINE_WIDTH));
    glVertex2d(0, - m_max_drawable_y - m_navdisplay_style_config->getDoubleValue(CFG_COMPASS_NOSE_LINE_LEN));
    glEnd();

    // draw circle

    m_parent->qglColor(m_navdisplay_style_config->getColorValue(CFG_COMPASS_LINE_COLOR));
    glLineWidth(m_navdisplay_style_config->getDoubleValue(CFG_COMPASS_LINE_WIDTH));
    glCallList(m_compass_circle_gllist);

    // draw compass scale

    if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_NAV_PLAN)
    {
        glCallList(m_plan_mod_compass_gllist);
        return;
    }

    if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_VOR_ROSE ||
        m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_ILS_ROSE ||
        m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_NAV_ROSE)
    {
        glCallList(m_full_compass_triangles_gllist);
    }

    glPushMatrix();
    glRotated(-north_track_rotation, 0, 0, 1);
    glCallList(m_full_compass_gllist);
    glPopMatrix();

    // hdg/track flag

    double wca = -m_flightstatus->wind_correction_angle_deg;
    if (doWindCorrection()) wca = -wca;

    m_parent->qglColor(m_navdisplay_style_config->getColorValue(CFG_COMPASS_HDG_TRK_FLAG_COLOR));
    glLineWidth(m_navdisplay_style_config->getDoubleValue(CFG_COMPASS_HDG_TRK_FLAG_LINE_WIDTH));

    glPushMatrix();
    glRotated(wca, 0, 0, 1);
    glTranslated(0, -m_max_drawable_y+1, 0);
    glCallList(m_wca_bug_gllist);

    // draw hdg/track line

    if (!m_fmc_control->fmcAutoPilot().isNAVCoupled() &&
        !m_fmc_control->fmcAutoPilot().isTakeoffModeActiveLateral() &&
        m_fmc_control->currentNDMode(m_left_side) != CFG_ND_DISPLAY_MODE_VOR_ROSE &&
        m_fmc_control->currentNDMode(m_left_side) != CFG_ND_DISPLAY_MODE_ILS_ROSE)
    {
        m_parent->qglColor(Qt::green);
        glBegin(GL_LINES);
        glVertex2d(0, 4*WCA_BUG_SIZE);
        glVertex2d(0, m_max_drawable_y);
        glEnd();

        // draw TOC/EOD
        
        double vs = m_flightstatus->smoothedVS();
        double diff_alt = m_flightstatus->APAlt() - m_flightstatus->smoothed_altimeter_readout.lastValue();
        double abs_alt_reach_distance_nm = Navcalc::getDistanceToAltitude(diff_alt, m_flightstatus->ground_speed_kts, vs);

        if (qAbs(diff_alt) > 500 && qAbs(vs) > 250 && abs_alt_reach_distance_nm > 0.0)
        {
            double distance_y = m_max_drawable_y * (abs_alt_reach_distance_nm / m_fmc_control->getNDRangeNM(m_left_side));

            if (distance_y < m_max_drawable_y)
            {
                m_parent->qglColor(m_navdisplay_style_config->getColorValue(CFG_ALT_REACH_LINE_COLOR));
                glTranslated(0.0, m_max_drawable_y - distance_y, 0.0);

                if (diff_alt > 0)
                    glCallList(m_toc_item_gllist);
                else
                    glCallList(m_eod_item_gllist);
            }
        }
    }

    glPopMatrix();

    // draw autopilot heading

    if (!m_fmc_control->fmcAutoPilot().isNAVCoupled() &&
        !m_fmc_control->fmcAutoPilot().isTakeoffModeActiveLateral())
    {
        double ap_hdg_diff_angle = m_flightstatus->APHdg() - m_flightstatus->smoothedMagneticHeading();
        while(ap_hdg_diff_angle < -180.0) ap_hdg_diff_angle += 360.0;

        m_parent->qglColor(m_navdisplay_style_config->getColorValue(CFG_COMPASS_HDG_BUG_LINE_COLOR));
        glLineWidth(m_navdisplay_style_config->getDoubleValue(CFG_COMPASS_HDG_BUG_LINE_WIDTH));

        glRotated(ap_hdg_diff_angle, 0, 0, 1);
        glTranslated(0, -m_max_drawable_y-1, 0);
        glCallList(m_hdg_bug_gllist);
    }
};

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleA::drawWindArrow()
{
    if (m_flightstatus->radarAltitude() < 10 || m_flightstatus->wind_speed_kts <= 0.0) return;

    m_parent->qglColor(Qt::green);

    double diff_angle_deg = m_flightstatus->wind_dir_deg_true - m_flightstatus->smoothedTrueHeading();

    glLoadIdentity();
    m_parent->qglColor(m_navdisplay_style_config->getColorValue(CFG_WIND_ARROW_COLOR));
    glLineWidth(m_navdisplay_style_config->getDoubleValue(CFG_WIND_ARROW_LINE_WIDTH));
    glTranslated(m_navdisplay_style_config->getDoubleValue(CFG_WIND_ARROW_SPACE),
                 m_navdisplay_style_config->getDoubleValue(CFG_WIND_ARROW_SPACE) + m_font_height * 2.0,
                 0.0);
    glRotated(diff_angle_deg, 0, 0, 1);

    GLDraw::drawVerticalArrow(0.0, -m_navdisplay_style_config->getDoubleValue(CFG_WIND_ARROW_LEN),
                              0.0, +m_navdisplay_style_config->getDoubleValue(CFG_WIND_ARROW_LEN),
                              m_navdisplay_style_config->getDoubleValue(CFG_WIND_ARROW_LEN)/2.0,
                              m_navdisplay_style_config->getDoubleValue(CFG_WIND_ARROW_LEN)/2.0,
                              false, true);
};

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleA::drawInfoTexts(const FlightRoute& route)
{
    const Waypoint* active_wpt = route.activeWaypoint();

    glLoadIdentity();

    //  top left corner: TAS, GS

    QString gs_text = QString("%1").arg(m_flightstatus->ground_speed_kts, 0, 'f', 0);
    QString tas_text = QString("%1").arg(m_flightstatus->tas, 0, 'f', 0);

    double gs_offset = getTextWidth("GS ");
    double gs_value_offset = getTextWidth(gs_text+" ");
    double tas_offset = getTextWidth("TAS ");

    double yoff = m_font_height;

    m_parent->qglColor(Qt::white);
    drawText(2.0, yoff, "GS");
    drawText(2.0 + gs_offset + gs_value_offset, yoff, "TAS");

    m_parent->qglColor(Qt::green);
    drawText(2.0 + gs_offset, yoff, gs_text);
    drawText(2.0 + gs_offset + gs_value_offset + tas_offset,
             yoff, tas_text);

    // top left corner: Wind

    if (!m_flightstatus->onground && m_flightstatus->wind_speed_kts > 0.0)
    {
        QString wind_string = QString("%1/%2").
                              arg(Navcalc::trimHeading(m_flightstatus->wind_dir_deg_true -
                                                       m_flightstatus->magvar), 0, 'f', 0).
                              arg(m_flightstatus->wind_speed_kts, 3, 'f', 0);

        drawText(2.0,
                 m_font_height * 2.0,
                 wind_string);
    }

    // top right corner: waypoint info

    double y_text_offset = m_font_height;

    if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_VOR_ROSE)
    {
        glLineWidth(1.0);

        QString text;
        double offset = 0;

        const Waypoint& navaid = (m_left_side) ? m_flightstatus->nav1 : m_flightstatus->nav2;
        const int& nav_freq = (m_left_side) ? m_flightstatus->nav1_freq : m_flightstatus->nav2_freq;
        const int& obs = (m_left_side) ? m_flightstatus->obs1 : m_flightstatus->obs2;
        bool has_loc = (m_left_side) ? m_flightstatus->nav1_has_loc : m_flightstatus->nav2_has_loc;
        int to_from = (m_left_side) ? m_flightstatus->obs1_to_from : m_flightstatus->obs2_to_from;

        // first line

        if (nav_freq == 0) text = QString(" ---.--");
        else               text = QString(" %1").arg(nav_freq / 1000.0, 3, 'f', 2);
        offset = getTextWidth(text);

        m_parent->qglColor(Qt::white);
        if (m_left_side)
        {
            QString text = "VOR1";
            if (m_flightstatus->obs1_to_from <= 0) text = "DME1";
            drawText(m_size.width() - 2.0 - offset - getTextWidth(text), y_text_offset, text);
        }
        else
        {
            QString text = "VOR2";
            if (m_flightstatus->obs2_to_from <= 0) text = "DME2";
            drawText(m_size.width() - 2.0 - offset - getTextWidth(text), y_text_offset, text);
        }

        drawText(m_size.width() - 2.0 - offset, y_text_offset, text);

        // second line

        y_text_offset += m_font_height;

        if (to_from > 0)
        {
            text = QString(" %1").arg(obs);
            offset = getTextWidth(text);
            
            m_parent->qglColor(Qt::white);
            drawText(m_size.width() - 2.0 - offset - getTextWidth("CRS"), y_text_offset, "CRS");
            m_parent->qglColor(Qt::cyan);
            drawText(m_size.width() - 2.0 - offset, y_text_offset, text);
        }

        // third line

        y_text_offset += m_font_height;

        if (!navaid.id().isEmpty() && !has_loc)
        {
            m_parent->qglColor(Qt::white);
            drawText(m_size.width() - 2.0 - getTextWidth(navaid.id()), y_text_offset, navaid.id());
        }
    }
    else if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_ILS_ROSE)
    {
        glLineWidth(1.0);

        QString text;
        double offset = 0;

        const Waypoint& navaid = (m_left_side) ? m_flightstatus->nav1 : m_flightstatus->nav2;
        const int& nav_freq = (m_left_side) ? m_flightstatus->nav1_freq : m_flightstatus->nav2_freq;
        const int& obs = (m_left_side) ? m_flightstatus->obs1 : m_flightstatus->obs2;
        bool has_loc = (m_left_side) ? m_flightstatus->nav1_has_loc : m_flightstatus->nav2_has_loc;
        int to_from = (m_left_side) ? m_flightstatus->obs1_to_from : m_flightstatus->obs2_to_from;

        // first line

        if (nav_freq == 0) text = QString(" ---.--");
        else               text = QString(" %1").arg(nav_freq / 1000.0, 3, 'f', 2);
        offset = getTextWidth(text);

        m_parent->qglColor(Qt::white);
        if (m_left_side)
            drawText(m_size.width() - 2.0 - offset - getTextWidth("ILS1"), y_text_offset, "ILS1");
        else
            drawText(m_size.width() - 2.0 - offset - getTextWidth("ILS2"), y_text_offset, "ILS2");
        m_parent->qglColor(Qt::magenta);
        drawText(m_size.width() - 2.0 - offset, y_text_offset, text);

        // second line

        y_text_offset += m_font_height;

        if (to_from > 0)
        {
            text = QString(" %1").arg(obs);
            offset = getTextWidth(text);
            
            m_parent->qglColor(Qt::white);
            drawText(m_size.width() - 2.0 - offset - getTextWidth("CRS"), y_text_offset, "CRS");
            m_parent->qglColor(Qt::cyan);
            drawText(m_size.width() - 2.0 - offset, y_text_offset, text);
        }

        // third line

        y_text_offset += m_font_height;

        if (!navaid.id().isEmpty() && has_loc)
        {
            m_parent->qglColor(Qt::magenta);
            drawText(m_size.width() - 2.0 - getTextWidth(navaid.id()), y_text_offset, navaid.id());
        }
    }
    else
    {
        //----- draw next waypoint text (name/distance/time-to-go) into right top corner

        if (active_wpt != 0)
        {
            const QString& wpt_text_id = active_wpt->id();
            QString wpt_text_track =
                QString("%1\260").arg(
                    Navcalc::round(
                        Navcalc::trimHeading(m_fmc_data.trueTrackToActiveWpt() - m_flightstatus->magvar)));

            //double wpt_text_x_off = getTextWidth(wpt_text_track);
            double wpt_text_x_off = getTextWidth("X")*wpt_text_track.length();

            glLineWidth(1.0);
            m_parent->qglColor(Qt::white);
            drawText(m_size.width() - 2.0 - wpt_text_x_off - getTextWidth(wpt_text_id) - getTextWidth("W"),
                     y_text_offset, wpt_text_id);

            m_parent->qglColor(Qt::green);
            drawText(m_size.width() - 2.0 - wpt_text_x_off, y_text_offset, wpt_text_track);

            y_text_offset += m_font_height;
            QString wpt_text_dist = QString("%1").arg(m_fmc_data.distanceToActiveWptNm(), 0, 'f', 1);
            QString wpt_text_dist_unit = "NM";

            double wpt_text_dist_xoff = getTextWidth(wpt_text_dist_unit);

            drawText(m_size.width() - 4.0 - wpt_text_dist_xoff - getTextWidth(wpt_text_dist) - getTextWidth("W"),
                     y_text_offset, wpt_text_dist);

            m_parent->qglColor(Qt::cyan);
            drawText(m_size.width() - 4.0 - wpt_text_dist_xoff,
                     y_text_offset,
                     wpt_text_dist_unit);

            // calc and draw time to next waypoint

            y_text_offset += m_font_height;
            const QTime tow = m_fmc_control->normalRoute().routeData(
                m_fmc_control->normalRoute().activeWaypointIndex()).m_time_over_waypoint;

            char time_string[10];
            sprintf(time_string, "%02d:%02d", tow.hour(), tow.minute());

            QString wpt_text_time(time_string);
            m_parent->qglColor(Qt::green);
            drawText(m_size.width() - 4.0 - getTextWidth(wpt_text_time), y_text_offset, wpt_text_time);

            y_text_offset += m_font_height;
        }
    }

    // bottom center: display TCAS

    if (!m_fmc_control->isTCASOn())
    {
        m_parent->qglColor(Qt::red);
        drawText(m_size.width()/4.0, m_size.height(), "TCAS");
    }

    // bottom lines - draw avionics info texts

    glLineWidth(1.5);

    if (m_fmc_control->currentNDMode(m_left_side) != CFG_ND_DISPLAY_MODE_NAV_PLAN)
    {
        double unit = getTextWidth("W");
        register double unit_half = unit / 2.0;

        if (m_fmc_control->currentNDNavaidPointerLeft(m_left_side) == CFG_ND_NAVAID_POINTER_TYPE_NDB)
        {
            glLoadIdentity();
            glTranslated(0, m_size.height(), 0);

            // draw info text

            m_parent->qglColor(Qt::white);
            drawText(unit*2, -m_font_height*2.0, "ADF1");

            QString id_text = "- - -";
            if (!m_flightstatus->adf1.id().isEmpty()) id_text = m_flightstatus->adf1.id();

            drawText(unit*2, -m_font_height, id_text);

            if (!m_flightstatus->adf1.id().isEmpty())
            {
                // draw info arrow

                m_parent->qglColor(Qt::green);
                glTranslated(unit*1, -m_font_height*3.0, 0);
                GLDraw::drawVerticalArrow(0, unit_half, 0, (unit_half)+(m_font_height*2.0), unit_half, unit_half, true, false, false);
            }
        }

        if (m_fmc_control->currentNDNavaidPointerRight(m_left_side) == CFG_ND_NAVAID_POINTER_TYPE_NDB)
        {
            glLoadIdentity();
            glTranslated(m_size.width() - 8*unit, m_size.height(), 0);

            // draw info text
            m_parent->qglColor(Qt::white);
            drawText(unit*2, -m_font_height*2.0, "ADF2");

            QString id_text = "- - -";
            if (!m_flightstatus->adf2.id().isEmpty()) id_text = m_flightstatus->adf2.id();

            drawText(unit*2, -m_font_height, id_text);

            if (!m_flightstatus->adf2.id().isEmpty())
            {
                // draw info arrow

                m_parent->qglColor(Qt::green);

                double double_arrow_width = unit / 4.0;

                glTranslated(unit*7, -m_font_height*3.0, 0);

                glBegin(GL_LINE_LOOP);
                glVertex2d(-double_arrow_width, (unit_half)+(m_font_height*2.0));
                glVertex2d(-double_arrow_width, 2*unit_half);
                glVertex2d(0, unit_half);
                glVertex2d(+double_arrow_width, 2*unit_half);
                glVertex2d(+double_arrow_width, (unit_half)+(m_font_height*2.0));
                glEnd();
            }
        }

        if (m_fmc_control->currentNDNavaidPointerLeft(m_left_side) == CFG_ND_NAVAID_POINTER_TYPE_VOR)
        {
            glLoadIdentity();
            glTranslated(0, m_size.height(), 0);

            // draw info text

            m_parent->qglColor(Qt::white);

            QString id_text = "- - -";
            if (!m_flightstatus->nav1.id().isEmpty()) id_text = m_flightstatus->nav1.id();

            QString dist_text = "---.- ";
            if (!m_flightstatus->nav1_distance_nm.isEmpty())
                dist_text = QString("%1 ").arg(m_flightstatus->nav1_distance_nm);

            if (m_flightstatus->nav1_has_loc)          drawText(unit*2, -m_font_height*2.0, "ILS1");
            else if (m_flightstatus->obs1_to_from > 0) drawText(unit*2, -m_font_height*2.0, "VOR1");
            else                                       drawText(unit*2, -m_font_height*2.0, "DME1");

            drawText(unit*2, -m_font_height, id_text);
            m_parent->qglColor(Qt::green);
            drawText(unit*2, 0, dist_text);
            m_parent->qglColor(Qt::cyan);
            drawText((unit*2)+getTextWidth(dist_text), 0, "NM");

            if (!m_flightstatus->nav1.id().isEmpty() && !m_flightstatus->nav1_has_loc && m_flightstatus->obs1_to_from > 0)
            {
                // draw info arrow

                m_parent->qglColor(Qt::white);
                glTranslated(unit*1, -m_font_height*3.0, 0);
                GLDraw::drawVerticalArrow(0, unit_half, 0, (unit_half)+(m_font_height*2.0), unit_half, unit_half, true, false, true, true);
                glBegin(GL_LINES);
                glVertex2d(0, 2*unit_half);
                glVertex2d(0, (unit_half)+(m_font_height*2.0));
                glEnd();
            }
        }

        if (m_fmc_control->currentNDNavaidPointerRight(m_left_side) == CFG_ND_NAVAID_POINTER_TYPE_VOR)
        {
            glLoadIdentity();
            glTranslated(m_size.width() - 8*unit, m_size.height(), 0);

            // draw info text

            m_parent->qglColor(Qt::white);


            QString id_text = "- - -";
            if (!m_flightstatus->nav2.id().isEmpty()) id_text = m_flightstatus->nav2.id();

            QString dist_text = "---.- ";
            if (!m_flightstatus->nav2_distance_nm.isEmpty())
                dist_text = QString("%1 ").arg(m_flightstatus->nav2_distance_nm);

            if (m_flightstatus->nav2_has_loc) drawText(unit*2, -m_font_height*2.0, "ILS2");
            else if (m_flightstatus->obs2_to_from > 0) drawText(unit*2, -m_font_height*2.0, "VOR2");
            else                                       drawText(unit*2, -m_font_height*2.0, "DME2");

            drawText(unit*2, -m_font_height, id_text);
            m_parent->qglColor(Qt::green);
            drawText(unit*2, 0, dist_text);
            m_parent->qglColor(Qt::cyan);
            drawText((unit*2)+getTextWidth(dist_text), 0, "NM");

            if (!m_flightstatus->nav2.id().isEmpty() && !m_flightstatus->nav2_has_loc && m_flightstatus->obs2_to_from > 0)
            {
                // draw info arrow

                double double_arrow_width = unit / 4.0;

                m_parent->qglColor(Qt::white);
                glTranslated(unit*7, -m_font_height*3.0, 0);
                GLDraw::drawVerticalArrow(0, unit_half, 0, (unit_half)+(m_font_height*2.0),
                                  unit_half, unit_half, true, false, true, true, double_arrow_width);
                glBegin(GL_LINES);
                glVertex2d(-double_arrow_width, 2*unit_half);
                glVertex2d(-double_arrow_width, (unit_half)+(m_font_height*2.0));
                glVertex2d(+double_arrow_width, 2*unit_half);
                glVertex2d(+double_arrow_width, (unit_half)+(m_font_height*2.0));
                glEnd();
            }
        }
    }
};

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleA::drawAdfPointer(uint adf_index)
{
    MYASSERT(adf_index > 0 && adf_index < 3);
    if (adf_index == 1 && m_flightstatus->adf1.id().isEmpty()) return;
    if (adf_index == 2 && m_flightstatus->adf2.id().isEmpty()) return;

    double length = m_max_drawable_y / (double)rangeRingCount();
    double arrow_offset = length / 3.0;
    double arrow_width = arrow_offset / 2.0;

    m_parent->qglColor(Qt::green);
    glLineWidth(2.0);

    glLoadIdentity();
    glTranslated(m_position_center.x(), m_position_center.y(), 0);

    if (doWindCorrection())
        glRotated(m_flightstatus->wind_correction_angle_deg, 0, 0, 1.0);

    if (adf_index == 1)
    {
        glRotated(m_flightstatus->adf1_bearing.value() + m_fmc_control->getAdf1Noise(), 0, 0, 1.0);

        // upper line
        glBegin(GL_LINES);
        glVertex2d(0, -m_max_drawable_y);
        glVertex2d(0, -m_max_drawable_y+length);
        glEnd();

        // upper arrow
        GLDraw::drawVerticalArrow(0, -m_max_drawable_y+arrow_offset,
                          0, -m_max_drawable_y+length,
                          arrow_width/2.0, arrow_width,
                          true, false, false);

        // lower line
        glBegin(GL_LINES);
        glVertex2d(0, +m_max_drawable_y);
        glVertex2d(0, +m_max_drawable_y-length);
        glEnd();

        // lower arrow
        arrow_offset *= 2.0;
        GLDraw::drawVerticalArrow(0, +m_max_drawable_y-arrow_offset,
                          0, +m_max_drawable_y-length,
                          arrow_width/2.0, arrow_width,
                          true, false, false);
    }
    else
    {
        glRotated(m_flightstatus->adf2_bearing.value() + m_fmc_control->getAdf2Noise(), 0, 0, 1.0);

        arrow_width /= 2.0;
        double arrow_height = arrow_width*1.5;

        // upper line and arrow

        glBegin(GL_LINES);
        glVertex2d(0, -m_max_drawable_y);
        glVertex2d(0, -m_max_drawable_y+arrow_offset);
        glEnd();

        glBegin(GL_LINE_LOOP);
        glVertex2d(arrow_width, -m_max_drawable_y+length);
        glVertex2d(arrow_width, -m_max_drawable_y+arrow_offset+arrow_height);
        glVertex2d(0, -m_max_drawable_y+arrow_offset);
        glVertex2d(-arrow_width, -m_max_drawable_y+arrow_offset+arrow_height);
        glVertex2d(-arrow_width, -m_max_drawable_y+length);
        glEnd();

        // lower line and arrow

        double base_offset = +m_max_drawable_y-arrow_offset-arrow_height;

        glBegin(GL_LINES);
        glVertex2d(0, +m_max_drawable_y);
        glVertex2d(0, +base_offset-arrow_width*2.0);
        glEnd();

        glBegin(GL_LINE_LOOP);
        glVertex2d(+arrow_width, +m_max_drawable_y-length);
        glVertex2d(+arrow_width, base_offset);
        glVertex2d(0, base_offset-arrow_width*2.0);
        glVertex2d(-arrow_width, base_offset);
        glVertex2d(-arrow_width, +m_max_drawable_y-length);
        glEnd();

    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleA::drawVorPointer(uint vor_index)
{
    MYASSERT(vor_index > 0 && vor_index < 3);
    if (vor_index == 1 && (m_flightstatus->nav1.id().isEmpty() || m_flightstatus->nav1_has_loc)) return;
    if (vor_index == 2 && (m_flightstatus->nav2.id().isEmpty() || m_flightstatus->nav2_has_loc)) return;

    double length = m_max_drawable_y / (double)rangeRingCount();
    double arrow_offset = length / 5.0;
    double arrow_width = length / 7.0;
    double arrow_height = arrow_width*1.5;

    m_parent->qglColor(Qt::white);
    glLineWidth(2.0);

    glLoadIdentity();
    glTranslated(m_position_center.x(), m_position_center.y(), 0);

    if (doWindCorrection())
        glRotated(m_flightstatus->wind_correction_angle_deg, 0, 0, 1.0);

//     Logger::log(QString("%1/%2 windcorr=%3  %4/%5").arg(m_flightstatus->nav1.id()).arg(m_flightstatus->nav2.id()).
//                 arg(m_flightstatus->wind_correction_angle_deg).
//                 arg(m_flightstatus->nav1_bearing.value()).arg(m_flightstatus->nav2_bearing.value()));

    if (vor_index == 1)
    {
        if (m_flightstatus->obs1_to_from > 0)
        {
            glRotated(m_flightstatus->nav1_bearing.value() + m_fmc_control->getVor1Noise(), 0, 0, 1.0);

            // upper line
            glBegin(GL_LINES);
            glVertex2d(0, -m_max_drawable_y);
            glVertex2d(0, -m_max_drawable_y+arrow_offset);
            glVertex2d(0, -m_max_drawable_y+length);
            glVertex2d(0, -m_max_drawable_y+arrow_offset+arrow_height);
            glEnd();

            // upper arrow
            GLDraw::drawVerticalArrow(0, -m_max_drawable_y+arrow_offset,
                                      0, -m_max_drawable_y+length,
                                      arrow_width, arrow_height,
                                      true, false, true, true);

            // lower line
            glBegin(GL_LINES);
            glVertex2d(0, +m_max_drawable_y);
            glVertex2d(0, +m_max_drawable_y-arrow_offset);
            glVertex2d(0, +m_max_drawable_y-length);
            glVertex2d(0, +m_max_drawable_y-arrow_offset-arrow_height+1);
            glEnd();

            // lower arrow
            arrow_offset *= 2.0;
            GLDraw::drawVerticalArrow(0, +m_max_drawable_y-arrow_offset-2,
                                      0, +m_max_drawable_y-length,
                                      arrow_width, arrow_height,
                                      true, false, true, true);
        }
    }
    else
    {
        if (m_flightstatus->obs2_to_from > 0)
        {
            glRotated(m_flightstatus->nav2_bearing.value() + m_fmc_control->getVor2Noise(), 0, 0, 1.0);

            register double double_arrow_width = arrow_width / 2.0;

            // upper line
            glBegin(GL_LINES);
            glVertex2d(0, -m_max_drawable_y);
            glVertex2d(0, -m_max_drawable_y+arrow_offset+arrow_height);

            glVertex2d(double_arrow_width, -m_max_drawable_y+length);
            glVertex2d(double_arrow_width, -m_max_drawable_y+arrow_offset+arrow_height+arrow_height);
            glVertex2d(-double_arrow_width, -m_max_drawable_y+length);
            glVertex2d(-double_arrow_width, -m_max_drawable_y+arrow_offset+arrow_height+arrow_height);
            glEnd();

            // upper arrow
            GLDraw::drawVerticalArrow(0, -m_max_drawable_y+arrow_offset+arrow_height,
                                      0, -m_max_drawable_y+length,
                                      arrow_width, arrow_height,
                                      true, false, true, true, double_arrow_width);

            // lower line

            double base_offset = +m_max_drawable_y-arrow_offset-arrow_height;

            glBegin(GL_LINES);
            glVertex2d(0, +m_max_drawable_y);
            glVertex2d(0, +base_offset);

            glVertex2d(+double_arrow_width, base_offset);
            glVertex2d(-double_arrow_width, base_offset);

            glVertex2d(+double_arrow_width, base_offset);
            glVertex2d(+double_arrow_width, +m_max_drawable_y-length);

            glVertex2d(-double_arrow_width, base_offset);
            glVertex2d(-double_arrow_width, +m_max_drawable_y-length);
            glEnd();
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleA::drawHSI(const double& north_track_rotation)
{
    glLoadIdentity();
    glTranslated(m_position_center.x(), m_position_center.y(), 0.0);

    if (m_left_side)
        glRotated(m_flightstatus->obs1 - (north_track_rotation - m_flightstatus->magvar), 0, 0, 1);
    else
        glRotated(m_flightstatus->obs2 - (north_track_rotation - m_flightstatus->magvar), 0, 0, 1);

    if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_VOR_ROSE)
    {
        m_parent->qglColor(Qt::cyan);
        glCallList(m_hsi_gllist);
        m_parent->qglColor(Qt::cyan);
        
        if (m_left_side)
        {
            if (m_flightstatus->nav1.id().isEmpty() || m_flightstatus->nav1_has_loc) return;
        }
        else
        {
            if (m_flightstatus->nav2.id().isEmpty() || m_flightstatus->nav2_has_loc) return;
        }
    }
    else if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_ILS_ROSE)
    {
        m_parent->qglColor(Qt::magenta);
        glCallList(m_hsi_gllist);
        m_parent->qglColor(Qt::magenta);

        if (m_left_side)
        {
            if (m_flightstatus->nav1.id().isEmpty() || !m_flightstatus->nav1_has_loc) return;
        }
        else
        {
            if (m_flightstatus->nav2.id().isEmpty() || !m_flightstatus->nav2_has_loc) return;
        }
    }

    // draw localizer

    double obs_loc_needle = (m_left_side) ? m_flightstatus->obs1_loc_needle : m_flightstatus->obs2_loc_needle;
    const int& obs_to_from = (m_left_side) ? m_flightstatus->obs1_to_from : m_flightstatus->obs2_to_from;

    if (obs_loc_needle < 125) obs_loc_needle += (m_left_side ? m_fmc_control->getIls1Noise() : m_fmc_control->getIls2Noise());

    glLineWidth(3.0);
    double offset = (4*m_hsi_offset / 127.0) * obs_loc_needle;

    glBegin(GL_LINES);
    glVertex2d(offset, -m_max_drawable_y/2.0 + m_hsi_offset);
    glVertex2d(offset, +m_max_drawable_y/2.0 - m_hsi_offset);
    glEnd();

    if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_VOR_ROSE)
    {
        if (obs_to_from != 0)
        {
            glLineWidth(2.0);
            if (obs_to_from == FlightStatus::FROM)
            {
                glRotated(180, 0, 0, 1);
                offset *= -1.0;
            }

            glTranslated(offset, -m_max_drawable_y/2.0 + m_hsi_offset, 0);

            glBegin(GL_LINES);
            glVertex2d(0, 0);
            glVertex2d(-8, 8);
            glVertex2d(0, 0);
            glVertex2d(8, 8);
            glEnd();
        }
        return;
    }

    // draw glideslope

    glLoadIdentity();
    glTranslated(m_size.width() - 10, m_position_center.y(), 0.0);

    m_parent->qglColor(Qt::yellow);
    glLineWidth(3.0);

    double gs_bug_width = 2*HSI_CIRCLE_SIZE;

    glBegin(GL_LINES);
    glVertex2d(-gs_bug_width, 0);
    glVertex2d(+gs_bug_width, 0);
    glEnd();

    m_parent->qglColor(Qt::white);
    glLineWidth(2.0);

    glTranslated(0, (-2*m_hsi_offset), 0);
    GLDraw::drawCircle(HSI_CIRCLE_SIZE, -M_PI, M_PI);
    glTranslated(0, (-2*m_hsi_offset), 0);
    GLDraw::drawCircle(HSI_CIRCLE_SIZE, -M_PI, M_PI);
    glTranslated(0, (6*m_hsi_offset), 0);
    GLDraw::drawCircle(HSI_CIRCLE_SIZE, -M_PI, M_PI);
    glTranslated(0, (2*m_hsi_offset), 0);
    GLDraw::drawCircle(HSI_CIRCLE_SIZE, -M_PI, M_PI);

    double obs_gs_needle = (m_left_side) ? m_flightstatus->obs1_gs_needle : m_flightstatus->obs2_gs_needle;
    if (obs_gs_needle < 125) obs_gs_needle += (m_left_side ? m_fmc_control->getIls1Noise() : m_fmc_control->getIls2Noise());

    if (obs_gs_needle >= -127)
    {
        m_parent->qglColor(Qt::magenta);
        glLineWidth(1.5);

        double offset = (4*m_hsi_offset / 127) * obs_gs_needle;
        glTranslated(0, (-4*m_hsi_offset) + offset, 0);

        double gs_bug_height = gs_bug_width * 1.5;

        glBegin(GL_LINES);

        if (obs_gs_needle > -125)
        {
            glVertex2d(-gs_bug_width, 0);
            glVertex2d(0              , -gs_bug_height);
            glVertex2d(0              , -gs_bug_height);
            glVertex2d(+gs_bug_width, 0);
        }

        if (obs_gs_needle < 125)
        {
            glVertex2d(+gs_bug_width, 0);
            glVertex2d(0              , +gs_bug_height);
            glVertex2d(0              , +gs_bug_height);
            glVertex2d(-gs_bug_width, 0);
        }
        glEnd();

//         glBegin(GL_LINE_LOOP);
//         glVertex2d(-gs_bug_width, 0);
//         glVertex2d(0, -gs_bug_height);
//         glVertex2d(+gs_bug_width, 0);
//         glVertex2d(0, +gs_bug_height);
//         glEnd();
    }
}

// End of file
