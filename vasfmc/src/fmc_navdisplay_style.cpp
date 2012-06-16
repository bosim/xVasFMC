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

#include "defines.h"
#include "config.h"
#include "waypoint.h"
#include "airport.h"
#include "runway.h"
#include "navcalc.h"
#include "fmc_control.h"
#include "projection_mercator.h"
#include "geodata.h"

#include "fmc_data.h"
#include "fmc_navdisplay_defines.h"
#include "fmc_navdisplay_style.h"

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

FMCNavdisplayStyle::FMCNavdisplayStyle(VasGLWidget* parent,
                                       Config* main_config,
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
    m_parent(parent), m_main_config(main_config), m_navdisplay_style_config(0), m_navdisplay_config(navdisplay_config), 
    m_tcas_config(tcas_config), m_fmc_control(fmc_control), m_fmc_data(fmc_control->fmcData()),
    m_flightstatus(fmc_control->flightStatus()), 
    m_projection(projection), m_font(0), m_tcas_empty_item_gllist(0), m_tcas_full_item_gllist(0), m_airport_item_gllist(0), 
    m_vor_dme_item_gllist(0), m_vor_wo_dme_item_gllist(0), m_ndb_item_gllist(0), 
    m_toc_item_gllist(0), m_eod_item_gllist(0), m_tod_item_gllist(0),
    m_left_holding_item_gllist(0), m_right_holding_item_gllist(0), m_last_holding_gllist_calculation_speed_kts(0),
    m_left_side(left_side)
{
    MYASSERT(m_parent != 0);
    MYASSERT(m_main_config != 0);
    MYASSERT(m_navdisplay_config != 0);
    MYASSERT(m_tcas_config != 0);
    MYASSERT(m_fmc_control != 0);
    MYASSERT(m_flightstatus != 0);
    MYASSERT(m_projection != 0);

    m_font = m_fmc_control->getGLFont();
    m_font_height = m_font->getHeight();

    // reset overall params
    
    reset(size, display_top_offset, max_drawable_y, position_center, dist_scale_factor);

    // compile lists

    compileHoldingGLLists();
    compileTcasGLLists();
};

/////////////////////////////////////////////////////////////////////////////

FMCNavdisplayStyle::~FMCNavdisplayStyle() 
{
    glDeleteLists(m_tcas_empty_item_gllist, 1);
    glDeleteLists(m_tcas_full_item_gllist, 1);
    glDeleteLists(m_airport_item_gllist, 1);
    glDeleteLists(m_vor_dme_item_gllist, 1);
    glDeleteLists(m_vor_wo_dme_item_gllist, 1);
    glDeleteLists(m_ndb_item_gllist, 1);
    glDeleteLists(m_toc_item_gllist, 1);
    glDeleteLists(m_eod_item_gllist, 1);
    glDeleteLists(m_tod_item_gllist, 1);
    glDeleteLists(m_left_holding_item_gllist, 1);
    glDeleteLists(m_right_holding_item_gllist, 1);
};

/////////////////////////////////////////////////////////////////////////////

bool FMCNavdisplayStyle::doWindCorrection() const 
{
    return m_navdisplay_config->getIntValue(CFG_WIND_CORRECTION) != 0; 
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyle::reset(const QSize& size,
                               const double& display_top_offset,
                               const int& max_drawable_y,
                               const QPointF& position_center,
                               const double& dist_scale_factor)
{
    m_size = size;
    m_display_top_offset = display_top_offset;
    m_max_drawable_y = max_drawable_y;
    m_position_center = position_center;
    m_dist_scale_factor = dist_scale_factor;

    compileHoldingGLLists();
    compileTcasGLLists();
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyle::drawAltitudeReachDisplay(const double& dist_scale_factor)
{
    double current_vs = m_flightstatus->smoothedVS();
    double diff_alt = m_flightstatus->smoothed_altimeter_readout.lastValue() - m_flightstatus->APAlt();

    // we will not draw the circle if we would have to climb 
    // while we are actually descending and vice versa.
    if ((current_vs < 0.0 && diff_alt < 0.0 )|| (current_vs > 0.0 && diff_alt > 0.0)) return;

    current_vs = fabs(current_vs);
    if (current_vs < 100) return;

    diff_alt = fabs(diff_alt);
    if (diff_alt < 100.0) return;

    current_vs = (current_vs / 10) * 10;
    diff_alt = (diff_alt / 10) * 10;

    double diff_dist = (diff_alt * m_flightstatus->ground_speed_kts * dist_scale_factor) / (current_vs * 60.0);
    if ((m_position_center.y() - (int)diff_dist) < (int)m_display_top_offset) return;
   
    double alpha = fabs(atan((m_size.width()/5.0)/diff_dist));
    GLDraw::drawCircle(diff_dist, -alpha, +alpha);

    //shift circle rather than change its radius?
    //glTranslated(0, m_max_drawable_y-diff_dist, 0);
    //double alpha = M_PI/16.0;    
    //GLDraw::drawCircle(m_max_drawable_y, -alpha, +alpha);
};

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyle::drawSurroundingItems(const double& north_track_rotation)
{
    #ifdef DO_PERF
    startPerfTimer("drawSurroundingItems");
    #endif

    QColor item_color = Qt::magenta;
    if (m_main_config->getIntValue(CFG_STYLE) == CFG_STYLE_B) item_color = Qt::cyan;

    glLoadIdentity();
    glTranslated(m_position_center.x(), m_position_center.y(), 0.0);
    
    // correct the display position for projection offset
    moveToCorrectedProjectionPosition(north_track_rotation);

    // rotate for heading
    glRotated(-north_track_rotation, 0, 0, 1.0);
    
    if (m_fmc_control->showSurroundingAirports(m_left_side))
    {
        WaypointPtrListIterator airport_iter(m_fmc_data.surroundingAirportList());
        while(airport_iter.hasNext()) 
            drawAirport(*((*airport_iter.next()).asAirport()), north_track_rotation, item_color, true, false);
    }

    if (m_fmc_control->showSurroundingVORs(m_left_side))
    {
        WaypointPtrListIterator vor_iter(m_fmc_data.surroundingVorList());
        while(vor_iter.hasNext()) 
        {
            const Waypoint* vor = vor_iter.next();
            if (vor->id() == m_flightstatus->nav1.id() || vor->id() == m_flightstatus->nav2.id()) continue;
            drawVor(*vor, vor->asVor()->hasDME(), north_track_rotation, item_color);
        }
    }

    if (m_fmc_control->showSurroundingNDBs(m_left_side))
    {
        WaypointPtrListIterator ndb_iter(m_fmc_data.surroundingNdbList());
        while(ndb_iter.hasNext()) 
            drawNdb(*((*ndb_iter.next()).asNdb()), north_track_rotation, item_color);
    }

    #ifdef DO_PERF
    stopPerfTimer();
    #endif
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyle::drawAirport(const Airport& airport, 
                                     const double& north_track_rotation,
                                     const QColor& airport_symbol_color,
                                     bool draw_airport, 
                                     bool draw_runways)
{
    glPushMatrix();

    // draw airport circle

    if (draw_airport)
    {
        glPushMatrix();
        glTranslated(scaleXY(airport.x()), scaleXY(airport.y()), 0.0);
        glRotated(+north_track_rotation, 0, 0, 1.0);

        m_parent->qglColor(airport_symbol_color);

        if (m_main_config->getIntValue(CFG_STYLE) == CFG_STYLE_A)
            drawText(7.0, -1.0, airport.id());
        else 
            drawText(7.0, +m_font_height, airport.id());

        glCallList(m_airport_item_gllist);

        glPopMatrix();
    }

    // draw runways

    if (draw_runways)
    {
        m_parent->qglColor(Qt::lightGray);
        glLineStipple(4, 0xAAAA);
        glLineWidth(1);
        glPolygonMode(GL_FRONT, GL_FILL);

        QMapIterator<QString, Runway> iter(airport.runwayMap());
        while(iter.hasNext())
        {
            iter.next();
            const Runway& runway = iter.value();
            double length_xy = runway.lengthM() / 1850.0 * m_dist_scale_factor;

            double rwy_true_hdg = ((double)runway.hdg()) + m_flightstatus->magvar;

            glPushMatrix();
            glTranslated(scaleXY(runway.x()), scaleXY(runway.y()), 0.0);
            glRotated(rwy_true_hdg, 0, 0, 1.0);

            // draw runway
            glRectd(-2, 0, +2, -length_xy);

            // draw final approach line
            glEnable(GL_LINE_STIPPLE);
            glBegin(GL_LINES);
            glVertex2d(0.0, 0.0);
            glVertex2d(0.0, 10.0 * m_dist_scale_factor);
            glEnd();
            glDisable(GL_LINE_STIPPLE);

            glPopMatrix();
        }    
    }

    glPopMatrix();
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyle::drawVor(const Waypoint& vor, 
                                 bool has_dme,
                                 const double& north_track_rotation,
                                 const QColor& vor_symbol_color)
{
    glPushMatrix();

    glTranslated(scaleXY(vor.x()), scaleXY(vor.y()), 0.0);
    glRotated(+north_track_rotation, 0, 0, 1.0);

    m_parent->qglColor(vor_symbol_color);
    if (m_main_config->getIntValue(CFG_STYLE) == CFG_STYLE_A)
        drawText(7.0, -1.0, vor.id());
    else
        drawText(7.0, m_font_height, vor.id());

    if (has_dme) glCallList(m_vor_dme_item_gllist);
    else         glCallList(m_vor_wo_dme_item_gllist);
    
    glPopMatrix();
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyle::drawNdb(const Ndb& ndb, 
                                 const double& north_track_rotation,
                                 const QColor& ndb_symbol_color)
{
    glPushMatrix();

    glTranslated(scaleXY(ndb.x()), scaleXY(ndb.y()), 0.0);
    glRotated(+north_track_rotation, 0, 0, 1.0);

    m_parent->qglColor(ndb_symbol_color);
    if (m_main_config->getIntValue(CFG_STYLE) == CFG_STYLE_A)
        drawText(7.0, -1.0, ndb.id());
    else
        drawText(7.0, m_font_height, ndb.id());

    glCallList(m_ndb_item_gllist);
    
    glPopMatrix();
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyle::drawTcas(const double& north_track_rotation)
{
    if (m_flightstatus->tcasEntryList().count() <= 0 || m_fmc_control->isTCASOff()) return;

    glLoadIdentity();
    glTranslated(m_position_center.x(), m_position_center.y(), 0.0);

    // correct the display position for projection offset
    moveToCorrectedProjectionPosition(north_track_rotation);

    // loop through tcas targets
    
    TcasEntryValueListIterator iter(m_flightstatus->tcasEntryList());
    for(; iter.hasNext(); )
    {
        const TcasEntry& tcas_entry= iter.next();
        MYASSERT(tcas_entry.m_valid);
        
        // check for minimum speed of target to avoid targets on ground
        if (tcas_entry.m_groundspeed_kts < m_tcas_config->getIntValue(CFG_TCAS_MIN_OTHER_SPEED)) continue;
        
        // check flightlevel diff
        double fl_diff = (tcas_entry.m_altitude_ft - m_flightstatus->alt_ft) / 100;
        if (fabs(fl_diff) > m_tcas_config->getIntValue(CFG_TCAS_MAX_FL_DIFF)) continue;

        // check distance
        double dist = Navcalc::getDistBetweenWaypoints(m_flightstatus->current_position_raw, tcas_entry.m_position);
        if (dist > m_fmc_control->getNDRangeNM(m_left_side)) continue;

        // calc target X/Y

        QPointF tcas_entry_xy;
        m_projection->convertLatLonToXY(tcas_entry.m_position.pointLatLon(), tcas_entry_xy);

        // rotate for heading

        glPushMatrix();
        glRotated(-north_track_rotation, 0, 0, 1.0);
        glTranslated(scaleXY(tcas_entry_xy.x()), scaleXY(tcas_entry_xy.y()), 0.0);
        glRotated(+north_track_rotation, 0, 0, 1.0);
        
        // draw target

        glLineWidth(1.0);

        if (!m_fmc_control->isTCASOn())
        {
            m_parent->qglColor(Qt::white);
            glCallList(m_tcas_empty_item_gllist);
        }
        else if (m_flightstatus->ground_speed_kts < m_tcas_config->getIntValue(CFG_TCAS_MIN_OWN_SPEED))
        {
            m_parent->qglColor(Qt::white);
            glCallList(m_tcas_empty_item_gllist);
        }
        else if (dist <= m_tcas_config->getIntValue(CFG_TCAS_ALERT_DIST) && 
                 fabs(fl_diff) <= m_tcas_config->getIntValue(CFG_TCAS_ALERT_FL_DIFF))
        {
            m_parent->qglColor(m_tcas_config->getColorValue(CFG_TCAS_ALERT_COL));
            glCallList(m_tcas_full_item_gllist);
        }
        else if (dist <= m_tcas_config->getIntValue(CFG_TCAS_HINT_DIST) && 
                 fabs(fl_diff) <= m_tcas_config->getIntValue(CFG_TCAS_HINT_FL_DIFF))
        {
            m_parent->qglColor(m_tcas_config->getColorValue(CFG_TCAS_HINT_COL));
            glCallList(m_tcas_full_item_gllist);
        }
        else if (dist <= m_tcas_config->getIntValue(CFG_TCAS_FULL_DIST) && 
                 fabs(fl_diff) <= m_tcas_config->getIntValue(CFG_TCAS_FULL_FL_DIFF))
        {
            m_parent->qglColor(m_tcas_config->getColorValue(CFG_TCAS_FULL_COL));
            glCallList(m_tcas_full_item_gllist);
        }
        else
        {
            m_parent->qglColor(m_tcas_config->getColorValue(CFG_TCAS_NORMAL_COL));
            glCallList(m_tcas_empty_item_gllist);
        }

        m_parent->qglColor(m_tcas_config->getColorValue(CFG_TCAS_DATA_COL));

        // draw climb/descent arrow
	
        if (tcas_entry.m_vs_fpm > m_tcas_config->getIntValue(CFG_TCAS_MIN_VS_CLIMB_DESCENT_DETECTION))
        {
            GLDraw::drawVerticalArrow(-8, -7, -8, +7, 4, 4, true, true);
        }
        else if (tcas_entry.m_vs_fpm < -m_tcas_config->getIntValue(CFG_TCAS_MIN_VS_CLIMB_DESCENT_DETECTION))
        {
            GLDraw::drawVerticalArrow(-8, -7, -8, +7, 4, 4, false, true);
        }

        // draw altitude

        QString alt_diff_text = QString::number(Navcalc::round(fl_diff));

        if (fl_diff > 0.0) drawText(-getTextWidth(alt_diff_text)/2.0, -10, alt_diff_text);
        else               drawText(-getTextWidth(alt_diff_text)/2.0, +8 + m_font_height, alt_diff_text);

        // revert rotation

        glPopMatrix();
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyle::drawLeftNavaid()
{
    if (m_fmc_control->currentNDNavaidPointerLeft(m_left_side) == CFG_ND_NAVAID_POINTER_TYPE_NDB)      drawAdfPointer(1);
    else if (m_fmc_control->currentNDNavaidPointerLeft(m_left_side) == CFG_ND_NAVAID_POINTER_TYPE_VOR) drawVorPointer(1);
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyle::drawRightNavaid()
{
    if (m_fmc_control->currentNDNavaidPointerRight(m_left_side) == CFG_ND_NAVAID_POINTER_TYPE_NDB)      drawAdfPointer(2);
    else if (m_fmc_control->currentNDNavaidPointerRight(m_left_side) == CFG_ND_NAVAID_POINTER_TYPE_VOR) drawVorPointer(2);
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyle::compileTcasGLLists()
{
    // compile empty tcas item list

    if (m_tcas_empty_item_gllist != 0) glDeleteLists(m_tcas_empty_item_gllist, 1);
    m_tcas_empty_item_gllist = glGenLists(1);
    MYASSERT(m_tcas_empty_item_gllist != 0);
    glNewList(m_tcas_empty_item_gllist, GL_COMPILE);
    glBegin(GL_LINE_LOOP);
    glVertex2i(0, -7);
    glVertex2i(4, 0);
    glVertex2i(0, 7);
    glVertex2i(-4, 0);
    glEnd();
    glEndList();

    // compile full tcas item list

    if (m_tcas_full_item_gllist != 0) glDeleteLists(m_tcas_full_item_gllist, 1);
    m_tcas_full_item_gllist = glGenLists(1);
    MYASSERT(m_tcas_full_item_gllist != 0);
    glNewList(m_tcas_full_item_gllist, GL_COMPILE);
    glBegin(GL_TRIANGLES);
    glVertex2i(0, -7);
    glVertex2i(4, 0);
    glVertex2i(0, 7);
    glVertex2i(0, 7);
    glVertex2i(-4, 0);
    glVertex2i(0, -7);
    glEnd();
    glEndList();

    MYASSERT(m_left_holding_item_gllist != m_right_holding_item_gllist);
    MYASSERT(m_left_holding_item_gllist != m_tcas_full_item_gllist);
    MYASSERT(m_left_holding_item_gllist != m_tcas_empty_item_gllist);
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyle::compileHoldingGLLists()
{
    m_last_holding_gllist_calculation_speed_kts = m_flightstatus->ground_speed_kts;
    double turn_radius = m_last_holding_gllist_calculation_speed_kts * 0.01 * m_dist_scale_factor;
    double hold_length = (m_last_holding_gllist_calculation_speed_kts / 60.0) * m_dist_scale_factor;

    // compile left holding

    if (m_left_holding_item_gllist != 0) glDeleteLists(m_left_holding_item_gllist, 1);
    m_left_holding_item_gllist = glGenLists(1);
    MYASSERT(m_left_holding_item_gllist != 0);
    glNewList(m_left_holding_item_gllist, GL_COMPILE);
    
    glBegin(GL_LINES);
    glVertex2d(0, 0);
    glVertex2d(0, +hold_length);
    glVertex2d(-turn_radius*2.0, 0);
    glVertex2d(-turn_radius*2.0, +hold_length);
    glEnd();

    glTranslated(-turn_radius, 0, 0);
    GLDraw::drawCircle(turn_radius);
    glTranslated(0, hold_length, 0);
    GLDraw::drawCircle(turn_radius, M_PI/2.0, M_PI*1.5);
    glTranslated(+turn_radius, -hold_length, 0);

    glEndList();

    // compile right holding

    if (m_right_holding_item_gllist != 0) glDeleteLists(m_right_holding_item_gllist, 1);
    m_right_holding_item_gllist = glGenLists(1);
    MYASSERT(m_right_holding_item_gllist != 0);
    glNewList(m_right_holding_item_gllist, GL_COMPILE);
    
    glBegin(GL_LINES);
    glVertex2d(0, 0);
    glVertex2d(0, +hold_length);
    glVertex2d(turn_radius*2.0, 0);
    glVertex2d(turn_radius*2.0, +hold_length);
    glEnd();

    glTranslated(turn_radius, 0, 0);
    GLDraw::drawCircle(turn_radius);
    glTranslated(0, hold_length, 0);
    GLDraw::drawCircle(turn_radius, M_PI/2.0, M_PI*1.5);
    glTranslated(-turn_radius, -hold_length, 0);

    glEndList();

    MYASSERT(m_left_holding_item_gllist != m_right_holding_item_gllist);
    MYASSERT(m_left_holding_item_gllist != m_tcas_full_item_gllist);
    MYASSERT(m_left_holding_item_gllist != m_tcas_empty_item_gllist);
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyle::drawGeoData(const double& north_track_rotation, const QColor& color, bool filled)
{
    if (!m_fmc_control->showGeoData()) return;

    #ifdef DO_PERF
    startPerfTimer("drawGeoData");
    #endif

    glLoadIdentity();
    glTranslated(m_position_center.x(), m_position_center.y(), 0.0);
    
    // correct the display position for projection offset
    moveToCorrectedProjectionPosition(north_track_rotation);

    m_parent->qglColor(color);
    glLineWidth(1.0);
    glRotated(-north_track_rotation, 0, 0, 1.0);

    RoutePtrListIterator route_iter(m_fmc_control->geoData().activeRouteList());
    while(route_iter.hasNext())
    {
        filled ? glBegin(GL_POLYGON) : glBegin(GL_LINE_STRIP);

        WaypointPtrListIterator wpt_iter(route_iter.next()->waypointList());
        while(wpt_iter.hasNext())
        {
            const Waypoint* wpt = wpt_iter.next();
            glVertex2d(scaleXY(wpt->x()), scaleXY(wpt->y()));
        }

        glEnd();
    }

    #ifdef DO_PERF
    stopPerfTimer();
    #endif
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplayStyle::drawEnergyCircle(const QColor& color, bool draw_dashed)
{
    double bank_angle = m_flightstatus->smoothedBank();
    if (qAbs(bank_angle) < 2.0 || qAbs(bank_angle) > 50.0) return;

    m_parent->qglColor(color);
    glLineWidth(2.0);
    glLoadIdentity();
    glTranslated(m_position_center.x(), m_position_center.y(), 0.0);

    // the length is the position in 90s
    double max_circle_length_nm = m_flightstatus->ground_speed_kts * 0.025;

    double radius = Navcalc::getTurnRadiusNm(m_flightstatus->ground_speed_kts, bank_angle);
    double angle = Navcalc::toRad(qMin(180.0, qAbs( (max_circle_length_nm * 360.0) / (2 * M_PI * radius))));
    radius *= m_dist_scale_factor;

    if (draw_dashed)
    {
        glLineStipple(4, 0xAAAA); // 0xAAAA = 1010101010101010
        glEnable(GL_LINE_STIPPLE);
    }

    if (radius > 0.0)
    {
        // left turn
        GLDraw::drawCircleOffset(radius, M_PI/2.0 - angle, M_PI/2.0, -radius, 0);
    }
    else
    {
        // right turn
        GLDraw::drawCircleOffset(-radius, -M_PI/2.0, -M_PI/2.0 + angle, -radius, 0);
    }

    if (draw_dashed) glDisable(GL_LINE_STIPPLE);
}

// End of file

