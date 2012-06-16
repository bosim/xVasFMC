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

/*! \file    fmc_gps_defines.h
    \author  Alexander Wemmer, alex@wemmer.at
*/


#ifndef __FMC_GPS_DEFINES_H__
#define __FMC_GPS_DEFINES_H__

#define CFG_GPS_POS_X "gpsx"
#define CFG_GPS_POS_Y "gpsy"
#define CFG_GPS_WIDTH "gpswidth"
#define CFG_GPS_HEIGHT "gpsheight"
#define CFG_GPS_WINDOW_STATUS "gpswindowstatus"
#define CFG_GPS_INPUT_AREA_FILE "gpsinputareafile"

#define CFG_GPS_REFRESH_PERIOD_MS "gps_refresh_period"
#define CFG_GPS_BACKGROUND_IMAGE "background_image"
#define CFG_GPS_NORMAL_FONTNAME "norm_font_name"
#define CFG_GPS_NORMAL_FONTSIZE "norm_font_size"

//TODO
// #define CFG_GPS_RANGE10_IMAGE "range_10_image"
// #define CFG_GPS_RANGE20_IMAGE "range_20_image"
// #define CFG_GPS_RANGE40_IMAGE "range_40_image"
// #define CFG_GPS_RANGE80_IMAGE "range_80_image"
// #define CFG_GPS_RANGE160_IMAGE "range_160_image"
// #define CFG_GPS_RANGE320_IMAGE "range_320_image"

// #define CFG_GPS_MODE_ILS_IMAGE "mode_ils_image"
// #define CFG_GPS_MODE_VOR_IMAGE "mode_vor_image"
// #define CFG_GPS_MODE_ROSE_IMAGE "mode_rose_image"
// #define CFG_GPS_MODE_ARC_IMAGE "mode_arc_image"
// #define CFG_GPS_MODE_PLAN_IMAGE "mode_plan_image"

// #define FMC_GPS_INPUT_ALT_SET_HPA_LEFT  "alt_set_hpa_left"
// #define FMC_GPS_INPUT_ALT_SET_INHG_LEFT "alt_set_inhg_left"
// #define FMC_GPS_INPUT_ALT_SET_HPA_RIGHT  "alt_set_hpa_right"
// #define FMC_GPS_INPUT_ALT_SET_INHG_RIGHT "alt_set_inhg_right"

// #define FMC_GPS_INPUT_ALT_SET_METRIC "alt_set_metric"
// #define FMC_GPS_INPUT_FLIGHTPATH_MODE "flightpath_mode"

// #define FMC_GPS_INPUT_ND_LEFT_AIRPORT "nd_left_surr_airports"
// #define FMC_GPS_INPUT_ND_LEFT_NDB "nd_left_surr_ndb"
// #define FMC_GPS_INPUT_ND_LEFT_VOR "nd_left_surr_vor"
// #define FMC_GPS_INPUT_ND_LEFT_TERR "nd_left_surr_terr"
// #define FMC_GPS_INPUT_ND_LEFT_CSTR "nd_left_rte_cstr"

// #define FMC_GPS_INPUT_ND_RIGHT_AIRPORT "nd_right_surr_airports"
// #define FMC_GPS_INPUT_ND_RIGHT_NDB "nd_right_surr_ndb"
// #define FMC_GPS_INPUT_ND_RIGHT_VOR "nd_right_surr_vor"
// #define FMC_GPS_INPUT_ND_RIGHT_TERR "nd_right_surr_terr"
// #define FMC_GPS_INPUT_ND_RIGHT_CSTR "nd_right_rte_cstr"

// #define FMC_GPS_INPUT_ND_LEFT_POINTER_LEFT_OFF "nd_left_ptr_l_off"
// #define FMC_GPS_INPUT_ND_LEFT_POINTER_LEFT_NDB "nd_left_ptr_l_ndb"
// #define FMC_GPS_INPUT_ND_LEFT_POINTER_LEFT_VOR "nd_left_ptr_l_vor"
// #define FMC_GPS_INPUT_ND_LEFT_POINTER_RIGHT_OFF "nd_left_ptr_r_off"
// #define FMC_GPS_INPUT_ND_LEFT_POINTER_RIGHT_NDB "nd_left_ptr_r_ndb"
// #define FMC_GPS_INPUT_ND_LEFT_POINTER_RIGHT_VOR "nd_left_ptr_r_vor"

// #define FMC_GPS_INPUT_ND_RIGHT_POINTER_LEFT_OFF "nd_right_ptr_l_off"
// #define FMC_GPS_INPUT_ND_RIGHT_POINTER_LEFT_NDB "nd_right_ptr_l_ndb"
// #define FMC_GPS_INPUT_ND_RIGHT_POINTER_LEFT_VOR "nd_right_ptr_l_vor"
// #define FMC_GPS_INPUT_ND_RIGHT_POINTER_RIGHT_OFF "nd_right_ptr_r_off"
// #define FMC_GPS_INPUT_ND_RIGHT_POINTER_RIGHT_NDB "nd_right_ptr_r_ndb"
// #define FMC_GPS_INPUT_ND_RIGHT_POINTER_RIGHT_VOR "nd_right_ptr_r_vor"

// #define FMC_GPS_INPUT_LEFT_ILS_ENABLE "ils_left_enable"
// #define FMC_GPS_INPUT_LEFT_FD_ENABLE "fd_left_enable"
// #define FMC_GPS_INPUT_RIGHT_ILS_ENABLE "ils_right_enable"
// #define FMC_GPS_INPUT_RIGHT_FD_ENABLE "fd_right_enable"

// #define FMC_GPS_INPUT_ATHROTTLE_ARM "athrottle_arm"
// #define FMC_GPS_INPUT_ATHROTTLE_SPDMACH "athrottle_spdmach"
// #define FMC_GPS_INPUT_ATHROTTLE_N1 "athrottle_n1"

// #define FMC_GPS_INPUT_AP1_ARM "ap1_arm"
// #define FMC_GPS_INPUT_AP2_ARM "ap2_arm"

// #define FMC_GPS_INPUT_LOC_ARM "ap_loc_arm"
// #define FMC_GPS_INPUT_APP_ARM "ap_app_arm"

// #define FMC_GPS_INPUT_KNOB_LEFT_ALT_SET "knob_left_alt_set"
// #define FMC_GPS_INPUT_KNOB_LEFT_ALT_SET_LEFT "knob_left_alt_set_left"
// #define FMC_GPS_INPUT_KNOB_LEFT_ALT_SET_RIGHT "knob_left_alt_set_right"

// #define FMC_GPS_INPUT_KNOB_RIGHT_ALT_SET "knob_right_alt_set"
// #define FMC_GPS_INPUT_KNOB_RIGHT_ALT_SET_LEFT "knob_right_alt_set_left"
// #define FMC_GPS_INPUT_KNOB_RIGHT_ALT_SET_RIGHT "knob_right_alt_set_right"

// #define FMC_GPS_INPUT_KNOB_ND_LEFT_RANGE_LEFT "knob_nd_left_range_left"
// #define FMC_GPS_INPUT_KNOB_ND_LEFT_RANGE_RIGHT "knob_nd_left_range_right"
// #define FMC_GPS_INPUT_KNOB_ND_LEFT_MODE_LEFT "knob_nd_left_mode_left"
// #define FMC_GPS_INPUT_KNOB_ND_LEFT_MODE_RIGHT "knob_nd_left_mode_right"

// #define FMC_GPS_INPUT_KNOB_ND_RIGHT_RANGE_LEFT "knob_nd_right_range_left"
// #define FMC_GPS_INPUT_KNOB_ND_RIGHT_RANGE_RIGHT "knob_nd_right_range_right"
// #define FMC_GPS_INPUT_KNOB_ND_RIGHT_MODE_LEFT "knob_nd_right_mode_left"
// #define FMC_GPS_INPUT_KNOB_ND_RIGHT_MODE_RIGHT "knob_nd_right_mode_right"

// #define FMC_GPS_INPUT_KNOB_SPD_PUSH "knob_spd_push"
// #define FMC_GPS_INPUT_KNOB_SPD_PULL "knob_spd_pull"
// #define FMC_GPS_INPUT_KNOB_SPD_LEFT "knob_spd_left"
// #define FMC_GPS_INPUT_KNOB_SPD_RIGHT "knob_spd_right"

// #define FMC_GPS_INPUT_KNOB_HDG_PULL "knob_hdg_pull"
// #define FMC_GPS_INPUT_KNOB_HDG_PUSH "knob_hdg_push"
// #define FMC_GPS_INPUT_KNOB_HDG_LEFT "knob_hdg_left"
// #define FMC_GPS_INPUT_KNOB_HDG_RIGHT "knob_hdg_right"

// #define FMC_GPS_INPUT_KNOB_ALT_PUSH "knob_alt_push"
// #define FMC_GPS_INPUT_KNOB_ALT_PULL "knob_alt_pull"
// #define FMC_GPS_INPUT_KNOB_ALT_LEFT "knob_alt_left"
// #define FMC_GPS_INPUT_KNOB_ALT_RIGHT "knob_alt_right"

// #define FMC_GPS_INPUT_KNOB_VS_PUSH "knob_vs_push"
// #define FMC_GPS_INPUT_KNOB_VS_PULL "knob_vs_pull"
// #define FMC_GPS_INPUT_KNOB_VS_LEFT "knob_vs_left"
// #define FMC_GPS_INPUT_KNOB_VS_RIGHT "knob_vs_right"

#endif /* __FMC_GPS_DEFINES_H__ */

// End of file

