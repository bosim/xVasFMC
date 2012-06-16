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

/*! \file    gldraw.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __GLDRAW_H__
#define __GLDRAW_H__

#include <math.h>

#include "vas_gl.h"

/////////////////////////////////////////////////////////////////////////////

//! GL drawing routines
class GLDraw 
{
public:

    static void drawVerticalArrow(double x_upper, double y_upper, 
                                  double x_lower, double y_lower,
                                  double arrow_width, double arrow_height,
                                  bool shows_up, 
                                  bool filled = true,
                                  bool arrow_closed = true,
                                  bool no_arrow_line = false,
                                  double close_gap_half_width = 0);

    // draws a circle, angle(-PI/2) = west (=270°), angle(0) = north (=0°)
    static void drawCircle(const double& radius,
                           const double& start_angle_rad = -M_PI/2.0,
                           const double& stop_angle_rad = M_PI/2.0,
                           const double& angle_inc = 0.025f);

    // draws a circle, angle(-PI/2) = west (=270°), angle(0) = north (=0°)
    static void drawCircleOffset(const double& radius,
                                 const double& start_angle_rad,
                                 const double& stop_angle_rad,
                                 const double& offset_x,
                                 const double& offset_y,
                                 const double& angle_inc = 0.025f);

    // draws a circle, angle(-PI/2) = west (=270°), angle(0) = north (=0°)
    static void drawFilledCircle(const double& radius,
                                 const double& start_angle_rad = -M_PI/2.0,
                                 const double& stop_angle_rad = M_PI/2.0,
                                 const double& angle_inc = 0.025f);

    // draws a circle, angle(-PI/2) = west (=270°), angle(0) = north (=0°)
    static void drawFilledCircleOffset(const double& radius,
                                       const double& start_angle_rad,
                                       const double& stop_angle_rad,
                                       const double& offset_x,
                                       const double& offset_y,
                                       const double& angle_inc = 0.025f);

    
};

#endif /* __GLDRAW_H__ */

// End of file

