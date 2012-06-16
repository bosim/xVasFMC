///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2007 Alexander Wemmer 
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

/*! \file    gldraw.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "gldraw.h"

/////////////////////////////////////////////////////////////////////////////

void GLDraw::drawVerticalArrow(double x_upper, double y_upper, 
                               double x_lower, double y_lower,
                               double arrow_width, double arrow_height,
                               bool shows_up, 
                               bool filled,
                               bool arrow_closed,
                               bool no_arrow_line,
                               double close_gap_half_width)
{
    if (filled) arrow_closed = true;

    if (!no_arrow_line)
    {
        glBegin(GL_LINES);
        glVertex2d(x_lower, y_lower);
        glVertex2d(x_upper, y_upper);
        glEnd();
    }
    
    double xref = x_upper;
    double yref = y_upper;
    double heightref = arrow_height;
    
    if (!shows_up)
    {
        xref = x_lower;
        yref = y_lower;
        heightref = -heightref;
    }
    
    if (filled) glBegin(GL_POLYGON);
    else glBegin(GL_LINES);
    glVertex2d(xref, yref);
    glVertex2d(xref - arrow_width, yref + heightref);
    if (!filled) glVertex2d(xref, yref);
    glVertex2d(xref + arrow_width, yref + heightref);

    if (arrow_closed)
    {
        if (close_gap_half_width == 0)
        {
            glVertex2d(xref - arrow_width, yref + heightref);
            glVertex2d(xref + arrow_width, yref + heightref);        
        }
        else
        {
            glVertex2d(xref - arrow_width, yref + heightref);
            glVertex2d(xref - arrow_width + close_gap_half_width, yref + heightref);

            glVertex2d(xref + arrow_width - close_gap_half_width, yref + heightref);
            glVertex2d(xref + arrow_width, yref + heightref);
        }
    }

    glEnd();
}

/////////////////////////////////////////////////////////////////////////////

void GLDraw::drawCircle(const double& radius,
                        const double& start_angle_rad,
                        const double& stop_angle_rad,
                        const double& angle_inc)
{
    vasglCircle(0, 0, radius, start_angle_rad, stop_angle_rad, angle_inc);
}

/////////////////////////////////////////////////////////////////////////////

void GLDraw::drawCircleOffset(const double& radius,
                              const double& start_angle_rad,
                              const double& stop_angle_rad,
                              const double& offset_x,
                              const double& offset_y,
                              const double& angle_inc)
{
    vasglCircle(offset_x, offset_y, radius, start_angle_rad,
        stop_angle_rad, angle_inc);
}

/////////////////////////////////////////////////////////////////////////////

void GLDraw::drawFilledCircle(const double& radius,
                              const double& start_angle_rad,
                              const double& stop_angle_rad,
                              const double& angle_inc)
{
    vasglFilledCircle(0, 0, radius, start_angle_rad, stop_angle_rad, angle_inc);
}

/////////////////////////////////////////////////////////////////////////////

void GLDraw::drawFilledCircleOffset(const double& radius,
                                    const double& start_angle_rad,
                                    const double& stop_angle_rad,
                                    const double& offset_x,
                                    const double& offset_y,
                                    const double& angle_inc)
{
    vasglFilledCircle(offset_x, offset_y, radius, start_angle_rad,
        stop_angle_rad, angle_inc);
}

// End of file
