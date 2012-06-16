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

/*! \file    fmc_fcu_defines.h
    \author  Alexander Wemmer, alex@wemmer.at
*/


#ifndef __FMC_FCU_DEFINES_H__
#define __FMC_FCU_DEFINES_H__

#define CFG_FCU_POS_X "fcux"
#define CFG_FCU_POS_Y "fcuy"
#define CFG_FCU_WIDTH "fcuwidth"
#define CFG_FCU_HEIGHT "fcuheight"
#define CFG_FCU_WINDOW_STATUS "fcuwindowstatus"
#define CFG_FCU_INPUT_AREA_FILE "fcuinputareafile"

#define CFG_FCU_REFRESH_PERIOD_MS "fcu_refresh_period"
#define CFG_FCU_BACKGROUND_IMAGE "background_image"
#define CFG_FCU_BACKGROUND_IMAGE_GAUGE "background_image_gauge"
#define CFG_FCU_LCD_FONTNAME "lcd_font_name"
#define CFG_FCU_LCD_FONTSIZE "lcd_font_size"
#define CFG_FCU_NORMAL_FONTNAME "norm_font_name"
#define CFG_FCU_NORMAL_FONTSIZE "norm_font_size"

#define CFG_FCU_RANGE10_IMAGE "range_10_image"
#define CFG_FCU_RANGE20_IMAGE "range_20_image"
#define CFG_FCU_RANGE40_IMAGE "range_40_image"
#define CFG_FCU_RANGE80_IMAGE "range_80_image"
#define CFG_FCU_RANGE160_IMAGE "range_160_image"
#define CFG_FCU_RANGE320_IMAGE "range_320_image"

#define CFG_FCU_MODE_ILS_IMAGE "mode_ils_image"
#define CFG_FCU_MODE_VOR_IMAGE "mode_vor_image"
#define CFG_FCU_MODE_ROSE_IMAGE "mode_rose_image"
#define CFG_FCU_MODE_ARC_IMAGE "mode_arc_image"
#define CFG_FCU_MODE_PLAN_IMAGE "mode_plan_image"

#define FMC_FCU_INPUT_ALT_SET_HPA_LEFT  "alt_set_hpa_left"
#define FMC_FCU_INPUT_ALT_SET_INHG_LEFT "alt_set_inhg_left"
#define FMC_FCU_INPUT_ALT_SET_HPA_RIGHT  "alt_set_hpa_right"
#define FMC_FCU_INPUT_ALT_SET_INHG_RIGHT "alt_set_inhg_right"

#define FMC_FCU_INPUT_ALT_SET_METRIC "alt_set_metric"
#define FMC_FCU_INPUT_FLIGHTPATH_MODE "flightpath_mode"

#define FMC_FCU_INPUT_ND_LEFT_AIRPORT "nd_left_surr_airports"
#define FMC_FCU_INPUT_ND_LEFT_NDB "nd_left_surr_ndb"
#define FMC_FCU_INPUT_ND_LEFT_VOR "nd_left_surr_vor"
#define FMC_FCU_INPUT_ND_LEFT_TERR "nd_left_surr_terr"
#define FMC_FCU_INPUT_ND_LEFT_CSTR "nd_left_rte_cstr"

#define FMC_FCU_INPUT_ND_RIGHT_AIRPORT "nd_right_surr_airports"
#define FMC_FCU_INPUT_ND_RIGHT_NDB "nd_right_surr_ndb"
#define FMC_FCU_INPUT_ND_RIGHT_VOR "nd_right_surr_vor"
#define FMC_FCU_INPUT_ND_RIGHT_TERR "nd_right_surr_terr"
#define FMC_FCU_INPUT_ND_RIGHT_CSTR "nd_right_rte_cstr"

#define FMC_FCU_INPUT_ND_LEFT_POINTER_LEFT_OFF "nd_left_ptr_l_off"
#define FMC_FCU_INPUT_ND_LEFT_POINTER_LEFT_NDB "nd_left_ptr_l_ndb"
#define FMC_FCU_INPUT_ND_LEFT_POINTER_LEFT_VOR "nd_left_ptr_l_vor"
#define FMC_FCU_INPUT_ND_LEFT_POINTER_RIGHT_OFF "nd_left_ptr_r_off"
#define FMC_FCU_INPUT_ND_LEFT_POINTER_RIGHT_NDB "nd_left_ptr_r_ndb"
#define FMC_FCU_INPUT_ND_LEFT_POINTER_RIGHT_VOR "nd_left_ptr_r_vor"

#define FMC_FCU_INPUT_ND_RIGHT_POINTER_LEFT_OFF "nd_right_ptr_l_off"
#define FMC_FCU_INPUT_ND_RIGHT_POINTER_LEFT_NDB "nd_right_ptr_l_ndb"
#define FMC_FCU_INPUT_ND_RIGHT_POINTER_LEFT_VOR "nd_right_ptr_l_vor"
#define FMC_FCU_INPUT_ND_RIGHT_POINTER_RIGHT_OFF "nd_right_ptr_r_off"
#define FMC_FCU_INPUT_ND_RIGHT_POINTER_RIGHT_NDB "nd_right_ptr_r_ndb"
#define FMC_FCU_INPUT_ND_RIGHT_POINTER_RIGHT_VOR "nd_right_ptr_r_vor"

#define FMC_FCU_INPUT_LEFT_ILS_ENABLE "ils_left_enable"
#define FMC_FCU_INPUT_LEFT_FD_ENABLE "fd_left_enable"
#define FMC_FCU_INPUT_RIGHT_ILS_ENABLE "ils_right_enable"
#define FMC_FCU_INPUT_RIGHT_FD_ENABLE "fd_right_enable"

#define FMC_FCU_INPUT_ATHROTTLE_ARM "athrottle_arm"
#define FMC_FCU_INPUT_ATHROTTLE_SPDMACH "athrottle_spdmach"
#define FMC_FCU_INPUT_ATHROTTLE_N1 "athrottle_n1"

#define FMC_FCU_INPUT_AP1_ARM "ap1_arm"
#define FMC_FCU_INPUT_AP2_ARM "ap2_arm"

#define FMC_FCU_INPUT_LOC_ARM "ap_loc_arm"
#define FMC_FCU_INPUT_APP_ARM "ap_app_arm"

#define FMC_FCU_INPUT_KNOB_LEFT_ALT_SET "knob_left_alt_set"
#define FMC_FCU_INPUT_KNOB_LEFT_ALT_SET_LEFT "knob_left_alt_set_left"
#define FMC_FCU_INPUT_KNOB_LEFT_ALT_SET_RIGHT "knob_left_alt_set_right"

#define FMC_FCU_INPUT_KNOB_RIGHT_ALT_SET "knob_right_alt_set"
#define FMC_FCU_INPUT_KNOB_RIGHT_ALT_SET_LEFT "knob_right_alt_set_left"
#define FMC_FCU_INPUT_KNOB_RIGHT_ALT_SET_RIGHT "knob_right_alt_set_right"

#define FMC_FCU_INPUT_KNOB_ALT_SET_100FT "knob_alt_set_100ft"
#define FMC_FCU_INPUT_KNOB_ALT_SET_1000FT "knob_alt_set_1000ft"

#define FMC_FCU_INPUT_KNOB_ND_LEFT_RANGE_LEFT "knob_nd_left_range_left"
#define FMC_FCU_INPUT_KNOB_ND_LEFT_RANGE_RIGHT "knob_nd_left_range_right"
#define FMC_FCU_INPUT_KNOB_ND_LEFT_MODE_LEFT "knob_nd_left_mode_left"
#define FMC_FCU_INPUT_KNOB_ND_LEFT_MODE_RIGHT "knob_nd_left_mode_right"

#define FMC_FCU_INPUT_KNOB_ND_RIGHT_RANGE_LEFT "knob_nd_right_range_left"
#define FMC_FCU_INPUT_KNOB_ND_RIGHT_RANGE_RIGHT "knob_nd_right_range_right"
#define FMC_FCU_INPUT_KNOB_ND_RIGHT_MODE_LEFT "knob_nd_right_mode_left"
#define FMC_FCU_INPUT_KNOB_ND_RIGHT_MODE_RIGHT "knob_nd_right_mode_right"

#define FMC_FCU_INPUT_KNOB_SPD          "knob_spd"
#define FMC_FCU_INPUT_KNOB_SPD_LEFT     "knob_spd_left"
#define FMC_FCU_INPUT_KNOB_SPD_RIGHT    "knob_spd_right"

#define FMC_FCU_INPUT_KNOB_HDG          "knob_hdg"
#define FMC_FCU_INPUT_KNOB_HDG_LEFT     "knob_hdg_left"
#define FMC_FCU_INPUT_KNOB_HDG_RIGHT    "knob_hdg_right"

#define FMC_FCU_INPUT_KNOB_ALT          "knob_alt"
#define FMC_FCU_INPUT_KNOB_ALT_LEFT     "knob_alt_left"
#define FMC_FCU_INPUT_KNOB_ALT_RIGHT    "knob_alt_right"

#define FMC_FCU_INPUT_KNOB_VS           "knob_vs"
#define FMC_FCU_INPUT_KNOB_VS_LEFT      "knob_vs_left"
#define FMC_FCU_INPUT_KNOB_VS_RIGHT     "knob_vs_right"

#define FMC_FCU_INPUT_CDU_DISPLAY_ONLY  "cdu_display_only"
#define FMC_FCU_INPUT_NEXT_CHECKLIST_ITEM "next_checklist_item"

#endif /* __FMC_FCU_DEFINES_H__ */

// End of file

