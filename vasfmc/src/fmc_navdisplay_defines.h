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

/*! \file    fmc_navdisplay_defines.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __FMC_NAVDISPLAY_DEFINES_H__
#define __FMC_NAVDISPLAY_DEFINES_H__

/////////////////////////////////////////////////////////////////////////////

//#define DO_PERF

#define NAVDISPLAY_TRACK_OFFSET_LIMIT 0.2

#define VIEW_DIR_DETECTION_ANGLE_DIFF 2

#define SCROLL_MODE_REGULAR 0
#define SCROLL_MODE_INVERSE 1

/////////////////////////////////////////////////////////////////////////////

#define CFG_NAVWIN_POS_X "navwinx"
#define CFG_NAVWIN_POS_Y "navwiny"
#define CFG_NAVWIN_WIDTH "navwinwidth"
#define CFG_NAVWIN_HEIGHT "navwinheight"
#define CFG_NAVWIN_WINDOW_STATUS "navwinwindowstatus"

#define CFG_SKIP_OVERFLOWN_POINTS "skip_overflown_points"
#define CFG_DRAW_RANGE_RINGS "range_rings"
#define CFG_WIND_CORRECTION "wind_correction"
#define CFG_ROUTE_DISPLAY_MAX_DISTANCE_NM "route_display_max_distance_nm"
#define CFG_SCROLL_MODE "scroll_mode"
#define CFG_GEO_DATA_COLOR "geo_data_color"

#define CFG_TCAS_MAX_FL_DIFF "tcas_max_fl_diff"
#define CFG_TCAS_MIN_OTHER_SPEED "tcas_min_other_speed"
#define CFG_TCAS_MIN_OWN_SPEED "tcas_min_own_speed"
#define CFG_TCAS_ALERT_DIST "tcas_alert_dist"
#define CFG_TCAS_ALERT_FL_DIFF "tcas_alert_fl_diff"
#define CFG_TCAS_HINT_DIST "tcas_hint_dist"
#define CFG_TCAS_HINT_FL_DIFF "tcas_hint_fl_diff"
#define CFG_TCAS_FULL_DIST "tcas_full_dist"
#define CFG_TCAS_FULL_FL_DIFF "tcas_full_fl_diff"
#define CFG_TCAS_MIN_VS_CLIMB_DESCENT_DETECTION "tcas_min_vs_climb_descent_detection"
#define CFG_TCAS_NORMAL_COL "tcas_normal_color"
#define CFG_TCAS_HINT_COL "tcas_hint_color"
#define CFG_TCAS_ALERT_COL "tcas_alert_color"
#define CFG_TCAS_FULL_COL "tcas_full_color"
#define CFG_TCAS_DATA_COL "tcas_data_color"

#endif /* __FMC_NAVDISPLAY_DEFINES_H__ */

// End of file
