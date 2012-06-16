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

/*! \file    fmc_navdisplay_style_b.cpp
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
#include "fmc_navdisplay_style_b.h"
#include "fmc_autopilot.h"

/////////////////////////////////////////////////////////////////////////////

#define CFG_RANGES_COLOR "ranges_color"
#define CFG_RANGES_TEXT_COLOR "ranges_text_color"
#define CFG_RANGES_LINE_WIDTH "ranges_line_width"
#define CFG_ARC_RANGES_RING_COUNT "arc_ranges_ring_count"
#define CFG_ROSE_RANGES_RING_COUNT "rose_ranges_ring_count"
#define CFG_RANGES_RING_COUNT "ranges_ring_count" //TODO

#define CFG_VIRT_PLANE_COLOR "virtual_plane_color"
#define CFG_VIRT_PLANE_LINE_WIDTH "virtual_plane_line_width"
#define CFG_VIRT_PLANE_HEIGHT "virtual_plane_height"
#define CFG_VIRT_PLANE_WIDTH "virtual_plane_width"

#define CFG_COMPASS_LINE_COLOR "compass_line_color"
#define CFG_COMPASS_LINE_WIDTH "compass_line_width"
#define CFG_COMPASS_EVE_LINE_LEN "compass_even_line_length"
#define CFG_COMPASS_ODD_LINE_LEN "compass_odd_line_length"
#define CFG_COMPASS_BOX_LINE_WIDTH "compass_box_line_width"
#define CFG_COMPASS_BOX_COLOR "compass_box_color"
#define CFG_COMPASS_BOX_ADD_TEXT_COLOR "compass_box_add_text_color"
#define CFG_COMPASS_HDG_BUG_LINE_COLOR "compass_hdg_bug_line_color"
#define CFG_COMPASS_HDG_BUG_LINE_WIDTH "compass_hdg_bug_ling_width"
#define CFG_COMPASS_HDG_BUG_MAX_HEIGHT "compass_hdg_bug_max_height"

#define CFG_WIND_ARROW_COLOR "wind_arror_color"
#define CFG_WIND_ARROW_LINE_WIDTH "wind_arror_line_width"
#define CFG_WIND_ARROW_LEN "wind_arror_length"
#define CFG_WIND_ARROW_SPACE "wind_arror_space"

#define CFG_ALT_REACH_LINE_WIDTH "altitude_reach_line_width"
#define CFG_ALT_REACH_LINE_COLOR "altitude_reach_line_color"

//TODO move to config
#define HSI_CIRCLE_SIZE 3.5
#define GS_BUG_WIDTH 7
#define GS_BUG_HEIGHT 7
#define GS_BUG_VERT_OFFSET 0

/////////////////////////////////////////////////////////////////////////////

FMCNavdisplayStyleB::FMCNavdisplayStyleB(ConfigWidgetProvider* config_widget_provider,
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
    m_hdg_bug_gllist(0), m_plane_gllist(0),
    m_range_ring_gllist(0), m_compass_circle_gllist(0), m_full_compass_gllist(0), m_plan_mod_compass_gllist(0),
    m_wpt_item_gllist(0),m_compass_box_gllist(0), m_hsi_gllist(0), m_hsi_offset(0), m_hsi_pointer_width(0),
    m_compass_box_width(0), m_compass_box_height(0), m_hdg_bug_height(0), m_hdg_triangle_width(0)
{
    MYASSERT(config_widget_provider != 0);

    // setup config

    m_navdisplay_style_config = new Config(cfg_navdisplay_style_filename);
    MYASSERT(m_navdisplay_style_config != 0);
    setupDefaultConfig();
    m_navdisplay_style_config->loadfromFile();
    m_navdisplay_style_config->saveToFile();

    // set basic values

    reset(size, display_top_offset, max_drawable_y, position_center, dist_scale_factor);

    // compile virt plane list

    m_plane_gllist = glGenLists(1);
    MYASSERT(m_plane_gllist != 0);
    glNewList(m_plane_gllist, GL_COMPILE);
    glBegin(GL_LINE_LOOP);
    glVertex2d(0, 0);
    glVertex2d(m_navdisplay_style_config->getDoubleValue(CFG_VIRT_PLANE_WIDTH),
               m_navdisplay_style_config->getDoubleValue(CFG_VIRT_PLANE_HEIGHT));
    glVertex2d(-m_navdisplay_style_config->getDoubleValue(CFG_VIRT_PLANE_WIDTH),
               m_navdisplay_style_config->getDoubleValue(CFG_VIRT_PLANE_HEIGHT));
    glEnd();
    glEndList();

    // compile waypoint item list

    m_wpt_item_gllist = glGenLists(1);
    MYASSERT(m_wpt_item_gllist != 0);
    glNewList(m_wpt_item_gllist, GL_COMPILE);
    glBegin(GL_LINE_LOOP);
    glVertex2i(0, -9);
	glVertex2i(2, -2);
	glVertex2i(9, 0);
	glVertex2i(2, 2);
    glVertex2i(0, 9);
	glVertex2i(-2, 2);
	glVertex2i(-9, 0);
	glVertex2i(-2, -2);
    glEnd();
    glEndList();

    // compile airport symbol list

    m_airport_item_gllist = glGenLists(1);
    MYASSERT(m_airport_item_gllist != 0);
    glNewList(m_airport_item_gllist, GL_COMPILE);
    glLineWidth(1.5);
    GLDraw::drawCircle(6, -M_PI, M_PI);
    glEndList();

    // TODO adapt symbol lists to B style!!

    // compile VOR/DME symbol list

    m_vor_dme_item_gllist = glGenLists(1);
    MYASSERT(m_vor_dme_item_gllist != 0);
    glNewList(m_vor_dme_item_gllist, GL_COMPILE);
    glLineWidth(1.0);

    double unit = 3;

    glBegin(GL_LINE_LOOP);
    glVertex2d(-unit, -unit);
    glVertex2d(-2*unit, 0);
    glVertex2d(-unit, +unit);
    glVertex2d(+unit, +unit);
    glVertex2d(+2*unit, 0);
    glVertex2d(+unit, -unit);
    glEnd();
    glBegin(GL_LINE_LOOP);
    glVertex2d(-unit, -unit);
    glVertex2d(-1.8*unit, -1.6*unit);
    glVertex2d(-2.7*unit, -0.5*unit);
    glVertex2d(-2*unit, 0);
    glEnd();
    glBegin(GL_LINE_LOOP);
    glVertex2d(+unit, -unit);
    glVertex2d(+1.8*unit, -1.6*unit);
    glVertex2d(+2.7*unit, -0.5*unit);
    glVertex2d(+2*unit, 0);
    glEnd();
    glBegin(GL_LINE_LOOP);
    glVertex2d(-unit, +unit);
    glVertex2d(-unit, +2*unit);
    glVertex2d(+unit, +2*unit);
    glVertex2d(+unit, +unit);
    glEnd();
    glEndList();

    // compile VOR w/o DME symbol list

    m_vor_wo_dme_item_gllist = glGenLists(1);
    MYASSERT(m_vor_wo_dme_item_gllist != 0);
    glNewList(m_vor_wo_dme_item_gllist, GL_COMPILE);
    glCallList(m_vor_dme_item_gllist);
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

    // compile TOD item

    m_tod_item_gllist = glGenLists(1);
    MYASSERT(m_tod_item_gllist != 0);
    glNewList(m_tod_item_gllist, GL_COMPILE);

    glLineWidth(1.5);
    GLDraw::drawCircle(3, -M_PI, M_PI);
    drawText(5, m_font_height, "T/D");

    glEndList();

    // compile EOD item

    m_eod_item_gllist = glGenLists(1);
    MYASSERT(m_eod_item_gllist != 0);
    glNewList(m_eod_item_gllist, GL_COMPILE);
    glEndList();

    // compile TOC item

    m_toc_item_gllist = glGenLists(1);
    MYASSERT(m_toc_item_gllist != 0);
    glNewList(m_toc_item_gllist, GL_COMPILE);
    glEndList();
}

/////////////////////////////////////////////////////////////////////////////

FMCNavdisplayStyleB::~FMCNavdisplayStyleB()
{
    glDeleteLists(m_hdg_bug_gllist, 1);
    glDeleteLists(m_plane_gllist, 1);
    glDeleteLists(m_range_ring_gllist, 1);
    glDeleteLists(m_compass_circle_gllist, 1);
    glDeleteLists(m_full_compass_gllist, 1);
    glDeleteLists(m_plan_mod_compass_gllist, 1);
    glDeleteLists(m_wpt_item_gllist, 1);
    glDeleteLists(m_compass_box_gllist, 1);
    glDeleteLists(m_hsi_gllist, 1);

    m_navdisplay_style_config->saveToFile();
    delete m_navdisplay_style_config;
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleB::setupDefaultConfig()
{
    m_navdisplay_style_config->setValue(CFG_RANGES_COLOR, "white");
    m_navdisplay_style_config->setValue(CFG_RANGES_TEXT_COLOR, "white");
    m_navdisplay_style_config->setValue(CFG_RANGES_LINE_WIDTH, "1.3");
    m_navdisplay_style_config->setValue(CFG_ARC_RANGES_RING_COUNT, 4);
    m_navdisplay_style_config->setValue(CFG_ROSE_RANGES_RING_COUNT, 2);
    m_navdisplay_style_config->setValue(CFG_RANGES_RING_COUNT, 4); //TODO

    m_navdisplay_style_config->setValue(CFG_VIRT_PLANE_COLOR, "white");
    m_navdisplay_style_config->setValue(CFG_VIRT_PLANE_LINE_WIDTH, "1.5");
    m_navdisplay_style_config->setValue(CFG_VIRT_PLANE_HEIGHT, "24");
    m_navdisplay_style_config->setValue(CFG_VIRT_PLANE_WIDTH, 8);

    m_navdisplay_style_config->setValue(CFG_COMPASS_LINE_COLOR, "white");
    m_navdisplay_style_config->setValue(CFG_COMPASS_LINE_WIDTH, "2.1");
    m_navdisplay_style_config->setValue(CFG_COMPASS_EVE_LINE_LEN, "11");
    m_navdisplay_style_config->setValue(CFG_COMPASS_ODD_LINE_LEN, "5");

    m_navdisplay_style_config->setValue(CFG_COMPASS_BOX_LINE_WIDTH, 2.1);
    m_navdisplay_style_config->setValue(CFG_COMPASS_BOX_COLOR, "white");
    m_navdisplay_style_config->setValue(CFG_COMPASS_BOX_ADD_TEXT_COLOR, "lightgreen");

    m_navdisplay_style_config->setValue(CFG_COMPASS_HDG_BUG_LINE_COLOR, "magenta");
    m_navdisplay_style_config->setValue(CFG_COMPASS_HDG_BUG_LINE_WIDTH, "1.5");
    m_navdisplay_style_config->setValue(CFG_COMPASS_HDG_BUG_MAX_HEIGHT, 10);

    m_navdisplay_style_config->setValue(CFG_WIND_ARROW_COLOR, "white");
    m_navdisplay_style_config->setValue(CFG_WIND_ARROW_LINE_WIDTH, "1.1");
    m_navdisplay_style_config->setValue(CFG_WIND_ARROW_LEN, "10");
    m_navdisplay_style_config->setValue(CFG_WIND_ARROW_SPACE, "15");

    m_navdisplay_style_config->setValue(CFG_ALT_REACH_LINE_WIDTH, "1.5");
    m_navdisplay_style_config->setValue(CFG_ALT_REACH_LINE_COLOR, "#63f76b");
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleB::reset(const QSize& size,
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

//     // compile range ring list
//     double radius_inc = max_drawable_y / m_navdisplay_style_config->getDoubleValue(CFG_RANGES_RING_COUNT);

//     if (m_range_ring_gllist != 0) glDeleteLists(m_range_ring_gllist, 1);
//     m_range_ring_gllist = glGenLists(1);
//     MYASSERT(m_range_ring_gllist != 0);
//     glNewList(m_range_ring_gllist, GL_COMPILE);

//     double radius = 0;
//     for(uint count=1; count < m_navdisplay_style_config->getDoubleValue(CFG_RANGES_RING_COUNT); ++count)
//     {
//         radius += radius_inc;
//         glBegin(GL_LINE_STRIP);
//         for(double angle = -pi_half; angle<=pi_half; angle+=0.01f)
//             glVertex2d(radius*sin(angle), -radius*cos(angle));
//         glEnd();
//     }

//     glEndList();

    // setup compass box & bug

    double unit = getTextWidth("W");
    m_compass_box_width = 1.5*unit;
    m_compass_box_height = m_font_height + 3;

    m_hdg_bug_height = qMin(m_position_center.y() - m_max_drawable_y - m_compass_box_height,
                            m_navdisplay_style_config->getDoubleValue(CFG_COMPASS_HDG_BUG_MAX_HEIGHT));

    m_hdg_triangle_width = 0.5 * unit;

    // compile compass bug list

    if (m_hdg_bug_gllist != 0) glDeleteLists(m_hdg_bug_gllist, 1);
    m_hdg_bug_gllist = glGenLists(1);
    MYASSERT(m_hdg_bug_gllist != 0);
    glNewList(m_hdg_bug_gllist, GL_COMPILE);
    glBegin(GL_LINE_LOOP);
    glVertex2d(-1*unit,0);
    glVertex2d(-1*unit, -m_hdg_bug_height);
    glVertex2d(-0.5*unit, -m_hdg_bug_height);
    glVertex2d(0, 0);
    glVertex2d(0.5*unit, -m_hdg_bug_height);
    glVertex2d(1*unit, -m_hdg_bug_height);
    glVertex2d(1*unit, 0);
    glEnd();
    glEndList();

    // compile compass box list

    if (m_compass_box_gllist != 0) glDeleteLists(m_compass_box_gllist, 1);
    m_compass_box_gllist = glGenLists(1);
    MYASSERT(m_compass_box_gllist != 0);
    glNewList(m_compass_box_gllist, GL_COMPILE);
    glBegin(GL_LINE_LOOP);
    glVertex2d(-m_compass_box_width, 0);
    glVertex2d(+m_compass_box_width,  0);
    glVertex2d(+m_compass_box_width,  m_compass_box_height);
    glVertex2d(-m_compass_box_width, m_compass_box_height);
    glEnd();
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
    glEndList();

    // compile full compass list

    if (m_full_compass_gllist) glDeleteLists(m_full_compass_gllist, 1);
    m_full_compass_gllist = glGenLists(1);
    MYASSERT(m_full_compass_gllist != 0);
    glNewList(m_full_compass_gllist, GL_COMPILE);

    double hdg_text_ytrans = m_navdisplay_style_config->getDoubleValue(CFG_COMPASS_EVE_LINE_LEN) + m_font_height;
    glLineWidth(m_navdisplay_style_config->getDoubleValue(CFG_COMPASS_LINE_WIDTH));

    int alpha_inc = 5;
    int alpha = 0;
    for (; alpha <360; alpha += alpha_inc)
    {
        if (alpha % 10 == 0)
        {
            // draw scale line
            glBegin(GL_LINES);
            glVertex2d(0, m_max_drawable_y);
            glVertex2d(0, m_max_drawable_y - m_navdisplay_style_config->getDoubleValue(CFG_COMPASS_EVE_LINE_LEN));
            glEnd();

            // draw scale text
            int hdg = alpha/10;
            QString hdg_text = QString::number(hdg);
            if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_NAV_ARC || hdg % 3 == 0)
                drawText(-getTextWidth(hdg_text)/2.0, -m_max_drawable_y + hdg_text_ytrans, hdg_text);
        }
        else
        {
            glBegin(GL_LINES);
            glVertex2d(0, m_max_drawable_y);
            glVertex2d(0, m_max_drawable_y - m_navdisplay_style_config->getDoubleValue(CFG_COMPASS_ODD_LINE_LEN));
            glEnd();
        }

        glRotated(alpha_inc, 0, 0, 1);
    }

    glEndList();

    // compile HSI mode list

    if (m_hsi_gllist != 0) glDeleteLists(m_hsi_gllist, 1);
    m_hsi_gllist = glGenLists(1);
    MYASSERT(m_hsi_gllist != 0);
    glNewList(m_hsi_gllist, GL_COMPILE);

    glPushMatrix();

    m_parent->qglColor(Qt::white);
    glLineWidth(2.0);

    m_hsi_pointer_width = m_hdg_triangle_width;

    // upper lines
    glBegin(GL_LINE_LOOP);
    glVertex2d(0, -m_max_drawable_y);
    glVertex2d(-m_hsi_pointer_width, -m_max_drawable_y+m_hsi_pointer_width*2);
    glVertex2d(-m_hsi_pointer_width, -(m_max_drawable_y/2.0) - m_hsi_pointer_width*4);
    glVertex2d(-2*m_hsi_pointer_width, -(m_max_drawable_y/2.0) - m_hsi_pointer_width*4);
    glVertex2d(-2*m_hsi_pointer_width, -(m_max_drawable_y/2.0) - m_hsi_pointer_width*2);
    glVertex2d(-m_hsi_pointer_width, -(m_max_drawable_y/2.0) - m_hsi_pointer_width*2);
    glVertex2d(-m_hsi_pointer_width, -(m_max_drawable_y/2.0));
    glVertex2d(m_hsi_pointer_width, -(m_max_drawable_y/2.0));
    glVertex2d(+m_hsi_pointer_width, -(m_max_drawable_y/2.0) - m_hsi_pointer_width*2);
    glVertex2d(+2*m_hsi_pointer_width, -(m_max_drawable_y/2.0) - m_hsi_pointer_width*2);
    glVertex2d(+2*m_hsi_pointer_width, -(m_max_drawable_y/2.0) - m_hsi_pointer_width*4);
    glVertex2d(m_hsi_pointer_width, -(m_max_drawable_y/2.0) - m_hsi_pointer_width*4);
    glVertex2d(m_hsi_pointer_width, -m_max_drawable_y+m_hsi_pointer_width*2);
    glEnd();

    // lower lines
    glBegin(GL_LINE_LOOP);
    glVertex2d(0, +m_max_drawable_y);
    glVertex2d(m_hsi_pointer_width, +m_max_drawable_y-m_hsi_pointer_width*2);
    glVertex2d(m_hsi_pointer_width, +m_max_drawable_y/2.0);
    glVertex2d(-m_hsi_pointer_width, +m_max_drawable_y/2.0);
    glVertex2d(-m_hsi_pointer_width, +m_max_drawable_y-m_hsi_pointer_width*2);
    glEnd();

    m_hsi_offset = m_max_drawable_y * 2.0/16.0;
    glLineWidth(2.0);

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

int FMCNavdisplayStyleB::rangeRingCount() const
{
    if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_NAV_ARC)
        return m_navdisplay_style_config->getIntValue(CFG_ARC_RANGES_RING_COUNT);

    return m_navdisplay_style_config->getIntValue(CFG_ROSE_RANGES_RING_COUNT);
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleB::drawAvionicsOff()
{
    m_parent->qglColor(RED);
    drawText(m_size.width()/2.0, m_size.height()/2.0, "ATT");
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleB::drawAirportRunways(const FlightRoute& route, const double& north_track_rotation)
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

void FMCNavdisplayStyleB::drawRouteNormalMode(const FlightRoute& route, const double& north_track_rotation)
{
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
    if (!m_fmc_control->fmcAutoPilot().isNAVCoupled()) do_stiple_legs = true; //TODO correct for B style ?

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

        QColor leg_color = Qt::magenta;
        if (wpt_index > ades_wpt_index && active_wpt_index <= ades_wpt_index) leg_color = Qt::cyan;
        if (temp_route) leg_color = Qt::yellow;

        QColor wpt_color = Qt::white;
        if (wpt == route.activeWaypoint()) wpt_color = Qt::magenta;
        if (temp_route) wpt_color = Qt::white;

        // test every third waypoint for distance
        if (reached_active_wpt && wpt_index % 3 == 0)
            stop_drawing = (wpt->x() * wpt->x()) + (wpt->y() * wpt->y()) > max_display_range_quad;

        if(!stop_drawing && iter.hasNext()) drawLeg(wpt, iter.peekNext(), north_track_rotation, leg_color, do_stiple_legs);

        reached_active_wpt |= (wpt == active_wpt || active_wpt == 0);

        drawWaypoint(wpt, north_track_rotation, wpt_color, leg_color, do_stiple_legs, reached_active_wpt);

        ++wpt_index;
    }

    // draw tuned VORs //TODO separate this from the route
    if (!m_flightstatus->nav1.id().isEmpty() && !m_flightstatus->nav1_has_loc)
        drawVor(m_flightstatus->nav1, !m_flightstatus->nav1_distance_nm.isEmpty(), north_track_rotation, Qt::green);
    if (!m_flightstatus->nav2.id().isEmpty() && !m_flightstatus->nav2_has_loc)
        drawVor(m_flightstatus->nav2, !m_flightstatus->nav2_distance_nm.isEmpty(), north_track_rotation, Qt::green);

    if (active_wpt != 0 && !active_wpt->holding().isActive())
    {
        // draw TOD
        if (m_fmc_control->normalRoute().todWpt().isValid() && m_fmc_data.distanceToTODNm() > 0.0)
        {
            drawSpecialWaypoint(&m_fmc_control->normalRoute().todWpt(), north_track_rotation,
                                m_navdisplay_style_config->getColorValue(CFG_ALT_REACH_LINE_COLOR));
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleB::drawRanges()
{
    if (m_navdisplay_config->getIntValue(CFG_DRAW_RANGE_RINGS) == 0) return;

    glLoadIdentity();
    glTranslated(m_position_center.x(), m_position_center.y(), 0);

    m_parent->qglColor(m_navdisplay_style_config->getColorValue(CFG_RANGES_COLOR));
    glLineWidth(m_navdisplay_style_config->getDoubleValue(CFG_RANGES_LINE_WIDTH));

    double wca = 0;
    if (!doWindCorrection()) wca = -m_flightstatus->wind_correction_angle_deg;

    //TODO range rings?

    if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_NAV_PLAN)
    {
//         glLineStipple(4, 0xAAAA); // 0xAAAA = 1010101010101010
//         glEnable(GL_LINE_STIPPLE);
        glCallList(m_range_ring_gllist);
//         glDisable(GL_LINE_STIPPLE);
    }
    else
    {
        // draw hdg/trk line

        if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_NAV_ARC ||
            m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_NAV_ROSE)
        {
            glPushMatrix();
            glRotated(wca, 0, 0, 1);

            m_parent->qglColor(Qt::white);
            glBegin(GL_LINES);
            glVertex2d(0, 0);
            glVertex2d(0, -m_max_drawable_y);
            glEnd();

            glPopMatrix();
        }
    }

    // separator lines + range texts

    glLineWidth(1.5);
    m_parent->qglColor(m_navdisplay_style_config->getColorValue(CFG_RANGES_TEXT_COLOR));

    int ring_count = rangeRingCount();

    double draw_range_inc = m_max_drawable_y / (double)ring_count;
    double real_range_inc = m_fmc_control->getNDRangeNM(m_left_side) / (double)ring_count;

    double draw_range = draw_range_inc;
    double real_range = real_range_inc;

    int start_count = 0;
    int max_count = 0;

    if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_NAV_ARC)
    {
        start_count = 0;
        max_count = ring_count-2;
    }
    else if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_NAV_PLAN)
    {
        start_count = 0;
        max_count = ring_count-1;
        real_range_inc /= 2;
        real_range = real_range_inc;
    }
    else
    {
        start_count = 0;
        max_count = ring_count-2;
        real_range_inc /= 2;
        real_range = real_range_inc;
    }

    for (int counter=start_count; counter <= max_count; ++counter)
    {
        // draw range texts

        double dummy = 0.0;
        QString range_text = QString("%1").arg(real_range, 0, 'f', (modf(real_range, &dummy)*10 == 0 ? 0 : 1));

        if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_NAV_ARC ||
            m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_NAV_ROSE)
        {
            glPushMatrix();
            glRotated(wca, 0, 0, 1);

            if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_NAV_ARC)
            {
                if (counter > start_count && counter < max_count)
                    drawText(-getTextWidth(range_text) - 15, -draw_range+(m_font_height/2.0), range_text);
            }
            else
            {
                drawText(-getTextWidth(range_text) - 15, -draw_range+(m_font_height/2.0), range_text);
            }

            glBegin(GL_LINES);
            glVertex2d(-10, -draw_range);
            glVertex2d(+10, -draw_range);
            glEnd();

            glPopMatrix();
        }
        else if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_NAV_PLAN)
        {
            double shiftx = -getTextWidth(range_text)/2.0;
            drawText(shiftx, -draw_range, range_text);
            drawText(shiftx, +draw_range, range_text);
        }

        draw_range += draw_range_inc;
        real_range += real_range_inc;
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleB::drawPlanePoly()
{
    if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_VOR_ROSE ||
        m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_ILS_ROSE) return;

    glLoadIdentity();
    glTranslated(m_position_center.x(), m_position_center.y(), 0);
    m_parent->qglColor(m_navdisplay_style_config->getColorValue(CFG_VIRT_PLANE_COLOR));
    glLineWidth(m_navdisplay_style_config->getDoubleValue(CFG_VIRT_PLANE_LINE_WIDTH));
    glCallList(m_plane_gllist);
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleB::drawCompass(double north_track_rotation)
{
    // draw heading box and text

    if (m_fmc_control->currentNDMode(m_left_side) != CFG_ND_DISPLAY_MODE_NAV_PLAN)
    {
        glLoadIdentity();
        glTranslated(m_position_center.x(), 0, 0);

        glCallList(m_compass_box_gllist);

        QString hdg_text = QString("%1").arg(m_flightstatus->smoothedMagneticHeading(), 3, 'f', 0, QChar('0'));
        if (doWindCorrection())
            hdg_text = QString("%1").arg(m_flightstatus->smoothedMagneticHeading()
                                         -m_flightstatus->wind_correction_angle_deg, 3, 'f', 0, QChar('0'));

        drawText(-getTextWidth(hdg_text)/2.0, m_font_height + 2, hdg_text);

        // draw heading text + status

        char hdg_type_text[4];
        (doWindCorrection()) ?
            sprintf(hdg_type_text, "TRK ") : sprintf(hdg_type_text, "HDG ");

        m_parent->qglColor(m_navdisplay_style_config->getColorValue(CFG_COMPASS_BOX_ADD_TEXT_COLOR));
        drawText(-m_compass_box_width - getTextWidth(hdg_type_text), m_font_height + 2, hdg_type_text);
        drawText(m_compass_box_width, m_font_height + 2, " MAG");
    }

    // draw circle

    glLoadIdentity();
    north_track_rotation -= m_flightstatus->magvar;
    glTranslated(m_position_center.x(), m_position_center.y(), 0);

    m_parent->qglColor(m_navdisplay_style_config->getColorValue(CFG_COMPASS_LINE_COLOR));
    glLineWidth(m_navdisplay_style_config->getDoubleValue(CFG_COMPASS_LINE_WIDTH));

    if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_NAV_ARC ||
        m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_NAV_PLAN)
        glCallList(m_compass_circle_gllist);

    // draw compass scale

    if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_NAV_PLAN)
    {
        glCallList(m_plan_mod_compass_gllist);
        drawText(-m_size.width()/2.0 + 5, 0, "W");
        drawText(+m_size.width()/2.0 - getTextWidth("E") - 5, 0, "E");
        glLoadIdentity();
        glTranslated(m_size.width()/2.0, m_size.height()/2.0, 0);
        drawText(-getTextWidth("N")/2.0, -m_size.height()/2.0 + m_font_height, "N");
        drawText(-getTextWidth("S")/2.0, +m_size.height()/2.0, "S");
        return;
    }

    glPushMatrix();
    glRotated(-north_track_rotation, 0, 0, 1);
    glCallList(m_full_compass_gllist);
    glPopMatrix();

    // draw autopilot heading

//TODO correct for B style?
//     if (!m_fmc_control->fmcAutoPilot().isNAVCoupled())
//     {
        double ap_hdg_diff_angle = m_flightstatus->APHdg() - m_flightstatus->smoothedMagneticHeading();
        if (doWindCorrection())
            ap_hdg_diff_angle += m_flightstatus->wind_correction_angle_deg;
        
        while(ap_hdg_diff_angle < -180.0) ap_hdg_diff_angle += 360.0;
        
        m_parent->qglColor(m_navdisplay_style_config->getColorValue(CFG_COMPASS_HDG_BUG_LINE_COLOR));
        glLineWidth(m_navdisplay_style_config->getDoubleValue(CFG_COMPASS_HDG_BUG_LINE_WIDTH));
        
        glPushMatrix();
        glRotated(ap_hdg_diff_angle, 0, 0, 1);
        glTranslated(0, -m_max_drawable_y-1, 0);
        glCallList(m_hdg_bug_gllist);
        glPopMatrix();
//TODO    }

    // draw hdg triangle

    m_parent->qglColor(Qt::white);
    if (doWindCorrection())
        glRotated(m_flightstatus->wind_correction_angle_deg, 0, 0, 1);

    glBegin(GL_LINE_LOOP);
    glVertex2d(0, -m_max_drawable_y);
    glVertex2d(-m_hdg_triangle_width, -m_max_drawable_y-m_hdg_bug_height);
    glVertex2d(+m_hdg_triangle_width, -m_max_drawable_y-m_hdg_bug_height);
    glEnd();
};

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleB::drawWindArrow()
{
    if (m_flightstatus->radarAltitude() < 10 || m_flightstatus->wind_speed_kts <= 0.0) return;

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

void FMCNavdisplayStyleB::drawAltitudeReachDisplay(const double& dist_scale_factor)
{
    glLoadIdentity();
    glTranslated(m_position_center.x(), m_position_center.y(), 0);
    m_parent->qglColor(m_navdisplay_style_config->getColorValue(CFG_ALT_REACH_LINE_COLOR));
    glLineWidth(m_navdisplay_style_config->getDoubleValue(CFG_ALT_REACH_LINE_WIDTH));

    FMCNavdisplayStyle::drawAltitudeReachDisplay(dist_scale_factor);
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleB::drawInfoTexts(const FlightRoute& route)
{
    const Waypoint* active_wpt = route.activeWaypoint();

    glLoadIdentity();
    m_parent->qglColor(Qt::white);
    glLineWidth(1.0);

    //  top left corner: TAS, GS

    drawText(2, m_font_height, QString("GS %1  TAS %2").
             arg(m_flightstatus->ground_speed_kts, 0, 'f', 0).
             arg(m_flightstatus->tas, 0, 'f', 0));

    // top left corner: Wind

    if (!m_flightstatus->onground && m_flightstatus->wind_speed_kts > 0.0)
    {
        QString wind_string = QString("%1°/%2").
                              arg(Navcalc::trimHeading(m_flightstatus->wind_dir_deg_true -
                                                       m_flightstatus->magvar), 0, 'f', 0).
                              arg(m_flightstatus->wind_speed_kts, 3, 'f', 0);

        drawText(2.0, m_font_height * 2.0, wind_string);
    }

    // top right corner: waypoint info

    double y_text_offset = m_font_height;

    if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_VOR_ROSE)
    {
        //TODO
    }
    else if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_ILS_ROSE)
    {
        //TODO
    }
    else
    {
        if (active_wpt != 0)
        {
            // draw next wpt ID

            const QString& wpt_text_id = active_wpt->id();
            m_parent->qglColor(Qt::magenta);
            drawText(m_size.width() - 2.0 - getTextWidth(wpt_text_id), y_text_offset, wpt_text_id);

            // calc and draw time to next waypoint

            y_text_offset += m_font_height;
            const QTime tow = m_fmc_control->normalRoute().routeData(
                m_fmc_control->normalRoute().activeWaypointIndex()).m_time_over_waypoint;

            char time_string[10];
            sprintf(time_string, "%02d%02d.%dZ", tow.hour(), tow.minute(), Navcalc::round(tow.second()*(100.0/600.0)));

            QString wpt_text_time(time_string);
            m_parent->qglColor(Qt::white);
            drawText(m_size.width() - 4.0 - getTextWidth(wpt_text_time), y_text_offset, wpt_text_time);

            // draw distance to next waypoint

            y_text_offset += m_font_height;
            QString wpt_text_dist = QString("%1NM").arg(m_fmc_data.distanceToActiveWptNm(), 0, 'f', 1);
            drawText(m_size.width() - 4.0 - getTextWidth(wpt_text_dist), y_text_offset, wpt_text_dist);
        }
    }

    // bottom center: display TCAS, AP and INFO status

    if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_NAV_ARC ||
        m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_NAV_ROSE)
    {
        m_parent->qglColor(Qt::cyan);

        if (m_fmc_control->showSurroundingAirports(m_left_side))
            drawText(4.0, m_size.height() - 9*m_font_height, "APT");

        if (m_fmc_control->showSurroundingVORs(m_left_side))
            drawText(4.0, m_size.height() - 8*m_font_height, "VOR");

        if (m_fmc_control->showSurroundingNDBs(m_left_side))
            drawText(4.0, m_size.height() - 7*m_font_height, "NDB");

        if (m_fmc_control->showGeoData())
            drawText(4.0, m_size.height() - 6*m_font_height, "TERR");

        if (!m_fmc_control->isTCASOn())
        {
            m_parent->qglColor(Qt::red);
            drawText(4.0, m_size.height() - 5*m_font_height, "TCAS");
        }
        else
        {
            m_parent->qglColor(Qt::cyan);
            drawText(4.0, m_size.height() - 5*m_font_height, "TFC");
        }
    }

//TODO
//     if (!m_flightstatus->isValid() || !m_fmc_control->fmcAutoPilot().isNAVCoupled())
//     {
//         m_parent->qglColor(Qt::red);
//         drawText(4.0, m_size.height() - 4*m_font_height, "COUPLE");
//     }

    // bottom lines - draw avionics info texts

    if (m_fmc_control->currentNDMode(m_left_side) != CFG_ND_DISPLAY_MODE_NAV_PLAN)
    {
        double unit = getTextWidth("W");

        if (m_fmc_control->currentNDNavaidPointerLeft(m_left_side) == CFG_ND_NAVAID_POINTER_TYPE_NDB)
        {
            glLoadIdentity();
            glTranslated(0, m_size.height(), 0);
            m_parent->qglColor(Qt::cyan);

            // draw info text

            drawText(unit, -m_font_height*2.0, "ADF L");

            QString id_text = "- - -";
            if (!m_flightstatus->adf1.id().isEmpty()) id_text = m_flightstatus->adf1.id();
            drawText(unit, -m_font_height, id_text);
        }

        if (m_fmc_control->currentNDNavaidPointerRight(m_left_side) == CFG_ND_NAVAID_POINTER_TYPE_NDB)
        {
            glLoadIdentity();
            glTranslated(m_size.width() - 8*unit, m_size.height(), 0);
            m_parent->qglColor(Qt::cyan);

            //TODO shift to right corner

            // draw info text
            drawText(unit*2, -m_font_height*2.0, "ADF R");

            QString id_text = "- - -";
            if (!m_flightstatus->adf2.id().isEmpty()) id_text = m_flightstatus->adf2.id();

            drawText(unit*2, -m_font_height, id_text);
        }

        if (m_fmc_control->currentNDNavaidPointerLeft(m_left_side) == CFG_ND_NAVAID_POINTER_TYPE_VOR)
        {
            glLoadIdentity();
            glTranslated(0, m_size.height(), 0);
            m_parent->qglColor(Qt::green);

            // draw info text

            QString id_text = "- - -";
            if (!m_flightstatus->nav1.id().isEmpty()) id_text = m_flightstatus->nav1.id();

            QString dist_text = "DME - - -";
            if (!m_flightstatus->nav1_distance_nm.isEmpty())
                dist_text = QString("DME %1").arg(m_flightstatus->nav1_distance_nm);

            if (m_flightstatus->nav1_has_loc)          drawText(unit, -m_font_height*2.0, "ILS L");
            else if (m_flightstatus->obs1_to_from > 0) drawText(unit, -m_font_height*2.0, "VOR L");
            else                                       drawText(unit, -m_font_height*2.0, "DME L");

            drawText(unit, -m_font_height, id_text);
            drawText(unit, 0, dist_text);
        }

        if (m_fmc_control->currentNDNavaidPointerRight(m_left_side) == CFG_ND_NAVAID_POINTER_TYPE_VOR)
        {
            glLoadIdentity();
            glTranslated(m_size.width() - 8*unit, m_size.height(), 0);
            m_parent->qglColor(Qt::green);

            // draw info text

            QString id_text = "- - -";
            if (!m_flightstatus->nav2.id().isEmpty()) id_text = m_flightstatus->nav2.id();

            QString dist_text = "DME - - -";
            if (!m_flightstatus->nav2_distance_nm.isEmpty())
                dist_text = QString("DME %1").arg(m_flightstatus->nav2_distance_nm);

            if (m_flightstatus->nav2_has_loc)          drawText(unit*2, -m_font_height*2.0, "ILS R");
            else if (m_flightstatus->obs2_to_from > 0) drawText(unit*2, -m_font_height*2.0, "VOR R");
            else                                       drawText(unit*2, -m_font_height*2.0, "DME R");

            drawText(unit*2, -m_font_height, id_text);
            drawText(unit*2, 0, dist_text);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleB::drawSpecialWaypoint(const Waypoint* wpt,
                                              const double& north_track_rotation,
                                              const QColor& color)
{
    MYASSERT(wpt != 0);

    glPushMatrix();

    // rotate for heading

    glTranslated(scaleXY(wpt->x()), scaleXY(wpt->y()), 0.0);
    glRotated(north_track_rotation, 0, 0, 1.0);
    m_parent->qglColor(color);

    if (wpt->flag() == Waypoint::FLAG_TOP_OF_DESCENT)
        glCallList(m_tod_item_gllist);

    glPopMatrix();
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleB::drawWaypoint(const Waypoint* wpt,
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
    drawText(5.0, m_font_height, wpt->id());

    // draw restrictions

    if (draw_restrictions &&
        m_fmc_control->showConstrains(m_left_side) &&
        wpt->asAirport() == 0)
    {
        m_parent->qglColor(Qt::magenta);
        double y = 2*m_font_height;

        if (wpt->restrictions().altitudeRestrictionFt() != 0)
        {
            drawText(5.0, y, wpt->restrictions().altitudeRestrictionText()+"FT");
            y += m_font_height;
        }

        if (wpt->restrictions().speedRestrictionKts() != 0)
            drawText(5.0, y, wpt->restrictions().speedRestrictionText()+"KT");
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
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleB::drawLeg(const Waypoint* from_wpt,
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

void FMCNavdisplayStyleB::drawRouteMapMode(const FlightRoute& route)
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

    WaypointPtrListIterator iter(route.waypointList());
    while(iter.hasNext())
    {
        const Waypoint* wpt = iter.next();

        QColor leg_color = Qt::magenta;
        if (temp_route) leg_color = Qt::yellow;

        QColor wpt_color = Qt::white;
        if (wpt == route.activeWaypoint()) wpt_color = Qt::magenta;
        if (temp_route) wpt_color = Qt::white;

        if(iter.hasNext()) drawLeg(wpt, iter.peekNext(), 0, leg_color, false);

        drawWaypoint(wpt, 0, wpt_color, leg_color, false, true);
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

//     // draw virt plane

//     glTranslated(+scaleXY(m_flightstatus->current_position_smoothed.x()),
//                  +scaleXY(m_flightstatus->current_position_smoothed.y()), 0.0);

//     glRotated(m_flightstatus->smoothedTrueHeading(), 0, 0, 1);

//     m_parent->qglColor(m_navdisplay_style_config->getColorValue(CFG_VIRT_PLANE_COLOR));
//     glLineWidth(m_navdisplay_style_config->getDoubleValue(CFG_VIRT_PLANE_LINE_WIDTH));
//     glCallList(m_plane_gllist);
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleB::drawAdfPointer(uint adf_index)
{
    MYASSERT(adf_index > 0 && adf_index < 3);
    if (adf_index == 1 && m_flightstatus->adf1.id().isEmpty()) return;
    if (adf_index == 2 && m_flightstatus->adf2.id().isEmpty()) return;

    double length = m_max_drawable_y / (2.0*(double)rangeRingCount());
    double arrow_height = m_navdisplay_style_config->getIntValue(CFG_COMPASS_EVE_LINE_LEN);
    double arrow_width = arrow_height/2.0;

    m_parent->qglColor(Qt::cyan);
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
        glVertex2d(-arrow_width, -m_max_drawable_y+length);
        glVertex2d(+arrow_width, -m_max_drawable_y+length);
        glEnd();

        // upper arrow
        GLDraw::drawVerticalArrow(0, -m_max_drawable_y,
                          0, -m_max_drawable_y+arrow_height,
                          arrow_width, arrow_height,
                          true, true, true, true);

        // lower line
        glBegin(GL_LINES);
        glVertex2d(0, +m_max_drawable_y);
        glVertex2d(0, +m_max_drawable_y-length);
        glEnd();

        // lower arrow
        GLDraw::drawVerticalArrow(0, +m_max_drawable_y-arrow_height,
                          0, +m_max_drawable_y,
                          arrow_width, arrow_height,
                          true, true, true, true);
    }
    else
    {
        glRotated(m_flightstatus->adf2_bearing.value() + m_fmc_control->getAdf2Noise(), 0, 0, 1.0);

        // upper line
        glBegin(GL_LINE_LOOP);
        glVertex2d(0, -m_max_drawable_y);
        glVertex2d(-arrow_width, -m_max_drawable_y+arrow_height);
        glVertex2d(-arrow_width, -m_max_drawable_y+length);
        glVertex2d(-2.0*arrow_width, -m_max_drawable_y+length);
        glVertex2d(+2.0*arrow_width, -m_max_drawable_y+length);
        glVertex2d(+arrow_width, -m_max_drawable_y+length);
        glVertex2d(+arrow_width, -m_max_drawable_y+arrow_height);
        glVertex2d(0, -m_max_drawable_y);
        glVertex2d(0, -m_max_drawable_y+arrow_height);
        glEnd();

        // lower line
        glBegin(GL_LINE_LOOP);
        glVertex2d(0, m_max_drawable_y-length);
        glVertex2d(-arrow_width, m_max_drawable_y-length+arrow_height);
        glVertex2d(-arrow_width, m_max_drawable_y-arrow_height);
        glVertex2d(-2.0*arrow_width, m_max_drawable_y-arrow_height/2.0);
        glVertex2d(-2.0*arrow_width, m_max_drawable_y);
        glVertex2d(0, m_max_drawable_y-arrow_height/2.0);
        glVertex2d(0, m_max_drawable_y-arrow_height);
        glVertex2d(0, m_max_drawable_y-arrow_height/2.0);
        glVertex2d(+2.0*arrow_width, m_max_drawable_y);
        glVertex2d(+2.0*arrow_width, m_max_drawable_y-arrow_height/2.0);
        glVertex2d(+arrow_width, m_max_drawable_y-arrow_height);
        glVertex2d(+arrow_width, m_max_drawable_y-length+arrow_height);
        glVertex2d(0, m_max_drawable_y-length);
        glEnd();
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleB::drawVorPointer(uint vor_index)
{
    MYASSERT(vor_index > 0 && vor_index < 3);
    if (vor_index == 1 && (m_flightstatus->nav1.id().isEmpty() || m_flightstatus->nav1_has_loc)) return;
    if (vor_index == 2 && (m_flightstatus->nav2.id().isEmpty() || m_flightstatus->nav2_has_loc)) return;

    double length = m_max_drawable_y / (1.5 * (double)rangeRingCount());
    double arrow_height = m_navdisplay_style_config->getIntValue(CFG_COMPASS_EVE_LINE_LEN);
    double arrow_width = arrow_height/2.0;

    m_parent->qglColor(Qt::green);
    glLineWidth(2.0);

    glLoadIdentity();
    glTranslated(m_position_center.x(), m_position_center.y(), 0);

    if (doWindCorrection())
        glRotated(m_flightstatus->wind_correction_angle_deg, 0, 0, 1.0);

//     //TODO
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
            glVertex2d(0, -m_max_drawable_y+length);
            glVertex2d(-arrow_width, -m_max_drawable_y+length);
            glVertex2d(+arrow_width, -m_max_drawable_y+length);
            glEnd();

            // upper arrow
            GLDraw::drawVerticalArrow(0, -m_max_drawable_y,
                                      0, -m_max_drawable_y+arrow_height,
                                      arrow_width, arrow_height,
                                      true, true, true, true);

            // lower line
            glBegin(GL_LINES);
            glVertex2d(0, +m_max_drawable_y);
            glVertex2d(0, +m_max_drawable_y-length);
            glEnd();

            // lower arrow
            GLDraw::drawVerticalArrow(0, +m_max_drawable_y-arrow_height,
                                      0, +m_max_drawable_y,
                                      arrow_width, arrow_height,
                                      true, true, true, true);
        }
    }
    else
    {
        if (m_flightstatus->obs2_to_from > 0)
        {
            glRotated(m_flightstatus->nav2_bearing.value() + m_fmc_control->getVor2Noise(), 0, 0, 1.0);

            // upper line
            glBegin(GL_LINE_LOOP);
            glVertex2d(0, -m_max_drawable_y);
            glVertex2d(-arrow_width, -m_max_drawable_y+arrow_height);
            glVertex2d(-arrow_width, -m_max_drawable_y+length);
            glVertex2d(-2.0*arrow_width, -m_max_drawable_y+length);
            glVertex2d(+2.0*arrow_width, -m_max_drawable_y+length);
            glVertex2d(+arrow_width, -m_max_drawable_y+length);
            glVertex2d(+arrow_width, -m_max_drawable_y+arrow_height);
            glVertex2d(0, -m_max_drawable_y);
            glVertex2d(0, -m_max_drawable_y+arrow_height);
            glEnd();

            // lower line
            glBegin(GL_LINE_LOOP);
            glVertex2d(0, m_max_drawable_y-length);
            glVertex2d(-arrow_width, m_max_drawable_y-length+arrow_height);
            glVertex2d(-arrow_width, m_max_drawable_y-arrow_height);
            glVertex2d(-2.0*arrow_width, m_max_drawable_y-arrow_height/2.0);
            glVertex2d(-2.0*arrow_width, m_max_drawable_y);
            glVertex2d(0, m_max_drawable_y-arrow_height/2.0);
            glVertex2d(0, m_max_drawable_y-arrow_height);
            glVertex2d(0, m_max_drawable_y-arrow_height/2.0);
            glVertex2d(+2.0*arrow_width, m_max_drawable_y);
            glVertex2d(+2.0*arrow_width, m_max_drawable_y-arrow_height/2.0);
            glVertex2d(+arrow_width, m_max_drawable_y-arrow_height);
            glVertex2d(+arrow_width, m_max_drawable_y-length+arrow_height);
            glVertex2d(0, m_max_drawable_y-length);
            glEnd();
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyleB::drawHSI(const double& north_track_rotation)
{
    glLoadIdentity();
    glTranslated(m_position_center.x(), m_position_center.y(), 0.0);

    m_parent->qglColor(Qt::white);

    // middle lines

    glLineWidth(2.0);

    glBegin(GL_LINES);
    glVertex2d(-m_hsi_pointer_width, -m_hsi_pointer_width*3);
    glVertex2d(-m_hsi_pointer_width, +m_hsi_pointer_width*3);
    glVertex2d(-m_hsi_pointer_width, +m_hsi_pointer_width*3);
    glVertex2d(-2*m_hsi_pointer_width, +m_hsi_pointer_width*3);

    glVertex2d(-m_hsi_pointer_width, 0);
    glVertex2d(-3*m_hsi_pointer_width, 0);

    glVertex2d(+m_hsi_pointer_width, -m_hsi_pointer_width*3);
    glVertex2d(+m_hsi_pointer_width, +m_hsi_pointer_width*3);
    glVertex2d(+m_hsi_pointer_width, +m_hsi_pointer_width*3);
    glVertex2d(+2*m_hsi_pointer_width, +m_hsi_pointer_width*3);

    glVertex2d(m_hsi_pointer_width, 0);
    glVertex2d(3*m_hsi_pointer_width, 0);

    glEnd();

    //-----

    if (m_left_side)
        glRotated(m_flightstatus->obs1 - (north_track_rotation - m_flightstatus->magvar), 0, 0, 1);
    else
        glRotated(m_flightstatus->obs2 - (north_track_rotation - m_flightstatus->magvar), 0, 0, 1);
    glCallList(m_hsi_gllist);

    if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_VOR_ROSE)
    {
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
        if (m_left_side)
        {
            if (m_flightstatus->nav1.id().isEmpty() || !m_flightstatus->nav1_has_loc) return;
        }
        else
        {
            if (m_flightstatus->nav2.id().isEmpty() || !m_flightstatus->nav2_has_loc) return;
        }
    }

    // draw to flag

    if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_VOR_ROSE)
    {
        const int& obs_to_from = (m_left_side) ? m_flightstatus->obs1_to_from : m_flightstatus->obs2_to_from;

        if (obs_to_from != 0)
        {
            glPushMatrix();
            if (obs_to_from == FlightStatus::FROM) glRotated(180, 0, 0, 1);
            glBegin(GL_LINE_LOOP);
            glVertex2d(0, -m_max_drawable_y/3.0);
            glVertex2d(-2*m_hsi_pointer_width, -m_max_drawable_y/3.0+2*m_hsi_pointer_width);
            glVertex2d(2*m_hsi_pointer_width, -m_max_drawable_y/3.0+2*m_hsi_pointer_width);
            glEnd();
            glPopMatrix();
        }
    }

    m_parent->qglColor(Qt::magenta);

    // draw localizer

    double loc_needle = (m_left_side) ? m_flightstatus->obs1_loc_needle : m_flightstatus->obs2_loc_needle;
    double gs_needle = (m_left_side) ? m_flightstatus->obs1_gs_needle : m_flightstatus->obs2_gs_needle;

    if (loc_needle < 125) loc_needle += (m_left_side ? m_fmc_control->getIls1Noise() : m_fmc_control->getIls2Noise());
    if (gs_needle < 125) gs_needle += (m_left_side ? m_fmc_control->getIls1Noise() : m_fmc_control->getIls2Noise());

    double offset = (4*m_hsi_offset / 127.0) * loc_needle;

    if (qAbs(loc_needle) >= 126)
    {
        glBegin(GL_LINE_LOOP);
        glVertex2d(offset-m_hsi_pointer_width, -m_max_drawable_y/2.0);
        glVertex2d(offset+m_hsi_pointer_width, -m_max_drawable_y/2.0);
        glVertex2d(offset+m_hsi_pointer_width, +m_max_drawable_y/2.0);
        glVertex2d(offset-m_hsi_pointer_width, +m_max_drawable_y/2.0);
        glEnd();
    }
    else
    {
        glBegin(GL_QUADS);
        glVertex2d(offset-m_hsi_pointer_width, -m_max_drawable_y/2.0);
        glVertex2d(offset+m_hsi_pointer_width, -m_max_drawable_y/2.0);
        glVertex2d(offset+m_hsi_pointer_width, +m_max_drawable_y/2.0);
        glVertex2d(offset-m_hsi_pointer_width, +m_max_drawable_y/2.0);
        glEnd();
    }

    if (m_fmc_control->currentNDMode(m_left_side) == CFG_ND_DISPLAY_MODE_VOR_ROSE) return;

    // draw glideslope

    glLoadIdentity();
    glTranslated(m_size.width() - 10, m_position_center.y(), 0.0);

    m_parent->qglColor(Qt::white);

    glBegin(GL_LINE_LOOP);
    glVertex2d(-GS_BUG_WIDTH, +2);
    glVertex2d(+GS_BUG_WIDTH, +2);
    glVertex2d(+GS_BUG_WIDTH, -2);
    glVertex2d(-GS_BUG_WIDTH, -2);
    glEnd();

    m_parent->qglColor(Qt::white);

    glTranslated(0, (-2*m_hsi_offset), 0);
    GLDraw::drawCircle(HSI_CIRCLE_SIZE, -M_PI, M_PI);
    glTranslated(0, (-2*m_hsi_offset), 0);
    GLDraw::drawCircle(HSI_CIRCLE_SIZE, -M_PI, M_PI);
    glTranslated(0, (6*m_hsi_offset), 0);
    GLDraw::drawCircle(HSI_CIRCLE_SIZE, -M_PI, M_PI);
    glTranslated(0, (2*m_hsi_offset), 0);
    GLDraw::drawCircle(HSI_CIRCLE_SIZE, -M_PI, M_PI);

    if (gs_needle >= -127)
    {
        m_parent->qglColor(Qt::magenta);
        double offset = (4*m_hsi_offset / 127) * gs_needle;

        glTranslated(0, (-4*m_hsi_offset) + offset, 0);

        if (qAbs(gs_needle) >= 118)
        {
            glBegin(GL_LINE_LOOP);
            glVertex2d(-GS_BUG_WIDTH, -GS_BUG_VERT_OFFSET);
            glVertex2d(0, -GS_BUG_HEIGHT-GS_BUG_VERT_OFFSET);
            glVertex2d(+GS_BUG_WIDTH, -GS_BUG_VERT_OFFSET);
            glVertex2d(0, +GS_BUG_HEIGHT+GS_BUG_VERT_OFFSET);
            glEnd();
        }
        else
        {
            glBegin(GL_POLYGON);
            glVertex2d(-GS_BUG_WIDTH, -GS_BUG_VERT_OFFSET);
            glVertex2d(0, -GS_BUG_HEIGHT-GS_BUG_VERT_OFFSET);
            glVertex2d(+GS_BUG_WIDTH, -GS_BUG_VERT_OFFSET);
            glVertex2d(0, +GS_BUG_HEIGHT+GS_BUG_VERT_OFFSET);
            glEnd();
        }
    }
}

// End of file
