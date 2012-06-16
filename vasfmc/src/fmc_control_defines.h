///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2008 Alexander Wemmer 
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

/*! \file    fmc_control_defines.h
    \author  Alexander Wemmer, alex@wemmer.at
*/


#ifndef __FMC_CONTROL_DEFINES_H__
#define __FMC_CONTROL_DEFINES_H__

#define CFG_PFDND_REFRESH_PERIOD_MS "pfdnd_refresh_perios_ms"
#define CFG_ECAM_REFRESH_PERIOD_MS "ecam_refresh_perios_ms"
#define CFG_CDUFCU_REFRESH_PERIOD_MS "cdufcu_refresh_period_ms"

#define CFG_SHOW_FPS "show_fps"
#define CFG_KEEP_ON_TOP "keep_on_top"

#define CFG_SQUAWKBOX_HANDLING_MODE "sbox_handling_mode"
#define ALTIMETER_HPA 0
#define ALTIMETER_INCHES 1
#define CFG_ALTIMETER_LEFT "altimeter_left"
#define CFG_ALTIMETER_RIGHT "altimeter_right"
#define CFG_SHOW_METRIC_ALT "pfd_show_metric_alt"
#define CFG_PFD_LEFT_DISPLAY_ILS "pfd_left_display_ils"
#define CFG_PFD_RIGHT_DISPLAY_ILS "pfd_right_display_ils"
#define CFG_ND_LEFT_DISPLAY_RANGE_NM "nd_left_display_range_nm"
#define CFG_ND_RIGHT_DISPLAY_RANGE_NM "nd_right_display_range_nm"
#define CFG_ND_RANGES_NM "nd_ranges_nm"
#define CFG_ND_LEFT_DISPLAY_MODE "nd_left_display_mode"
#define CFG_ND_RIGHT_DISPLAY_MODE "nd_right_display_mode"
#define CFG_ND_LEFT_LEFT_NAVAID_POINTER_TYPE "nd_left_left_navaid_pointer_type"
#define CFG_ND_LEFT_RIGHT_NAVAID_POINTER_TYPE "nd_left_right_navaid_pointer_type"
#define CFG_ND_RIGHT_LEFT_NAVAID_POINTER_TYPE "nd_right_left_navaid_pointer_type"
#define CFG_ND_RIGHT_RIGHT_NAVAID_POINTER_TYPE "nd_right_navaid_pointer_type"
#define CFG_DRAW_LEFT_SURROUNDING_AIRPORTS "nd_left_draw_surrounding_airports"
#define CFG_DRAW_LEFT_SURROUNDING_VORS "nd_left_draw_surrounding_vors"
#define CFG_DRAW_LEFT_SURROUNDING_NDBS "nd_left_draw_surrounding_ndbs"
#define CFG_DRAW_RIGHT_SURROUNDING_AIRPORTS "nd_right_draw_surrounding_airports"
#define CFG_DRAW_RIGHT_SURROUNDING_VORS "nd_right_draw_surrounding_vors"
#define CFG_DRAW_RIGHT_SURROUNDING_NDBS "nd_right_draw_surrounding_ndbs"
#define CFG_DRAW_GEODATA "nd_draw_geodata"
#define CFG_SOUNDS_ENABLED "sounds_enabled"
#define CFG_SOUND_CHANNELS_ENABLED "sound_channels_enabled"
#define CFG_SYNC_CLOCK_TIME "sync_clock_time"
#define CFG_SYNC_CLOCK_DATE "sync_clock_date"
#define CFG_FMC_AUTOTHRUST_ENABLED "fmc_autothrust_enabled"
#define CFG_SHOW_INPUTAREAS "show_inputareas"
#define CFG_SHOW_GEO_DATA_FILLED "show_geo_data_filled"
#define CFG_ALTIMETER_LEFT_IS_SET_TO_STD "altimeter_left_is_set_to_std"
#define CFG_ALTIMETER_RIGHT_IS_SET_TO_STD "altimeter_right_is_set_to_std"
#define CFG_SHOW_LEFT_CONSTRAINS "show_left_constrains"
#define CFG_SHOW_RIGHT_CONSTRAINS "show_right_constrains"
#define CFG_FCU_LEFT_ONLY_MODE "fcu_left_only_mode"
#define CFG_CDU_DISPLAY_ONLY_MODE "cdu_display_only_mode"
#define CFG_TCAS "tcas"

#define CFG_NOISE_GENERATION_INTERVAL_MS "noise_generation_intervall_ms"
#define CFG_ADF_NOISE_LIMIT_DEG "adf_noise_limit_degrees"
#define CFG_ADF_NOISE_INC_LIMIT_DEG "adf_noise_increase_limit_degrees"
#define CFG_VOR_NOISE_LIMIT_DEG "vor_noise_limit_degrees"
#define CFG_VOR_NOISE_INC_LIMIT_DEG "vor_noise_increase_limit_degrees"
#define CFG_ILS_NOISE_LIMIT "ils_noise_limit"
#define CFG_ILS_NOISE_INC_LIMIT "ils_noise_increase_limit"

#define CFG_FBW_ENABLED "fbw_enabled"

#define CFG_FBW_BANK_P_GAIN "fbw_bank_p_gain"
#define CFG_FBW_BANK_I_GAIN "fbw_bank_i_gain"
#define CFG_FBW_BANK_D_GAIN "fbw_bank_d_gain"
#define CFG_FBW_BANK_MAX_RATE "fbw_bank_max_rate"
#define CFG_FBW_BANK_IDLE_LIMIT "fbw_bank_idle_limit"
#define CFG_FBW_BANK_FORCED_LIMIT "fbw_bank_forced_limit"
#define CFG_FBW_BANK_DO_STATISTICS "fbw_bank_do_statistics"
#define CFG_FBW_BANK_I_TO_P_RESPONSE_FACTOR "fbw_bank_i_to_p_response_factor"

#define CFG_FBW_PITCH_P_GAIN "fbw_pitch_p_gain"
#define CFG_FBW_PITCH_I_GAIN "fbw_pitch_i_gain"
#define CFG_FBW_PITCH_D_GAIN "fbw_pitch_d_gain"
#define CFG_FBW_PITCH_MAX_RATE "fbw_pitch_max_rate"
#define CFG_FBW_PITCH_NEG_LIMIT "fbw_pitch_neg_limit"
#define CFG_FBW_PITCH_POS_LIMIT "fbw_pitch_pos_limit"
#define CFG_FBW_PITCH_GOOD_TREND_DAMPING_FACTOR "fbw_pitch_good_trend_damping_factor"
#define CFG_FBW_PITCH_I_TO_P_RESPONSE_FACTOR "fbw_pitch_i_to_p_response_factor"
#define CFG_FBW_PITCH_BANK_RATE_BOOST_FACTOR "fbw_pitch_bank_rate_boost_factor"
#define CFG_FBW_PITCH_STABLE_FPV_DAMP_FACTOR "fbw_pitch_stable_fvp_damp_factor"
#define CFG_FBW_PITCH_TRANSITION_BOOST_FACTOR "fbw_pitch_transition_boost_factor"
#define CFG_FBW_PITCH_DO_STATISTICS "fbw_pitch_do_statistics"

#define CFG_ACFT_DATA_LAST_FILE "acft_data_last_file"

#define CFG_USE_CPFLIGHT_SERIAL "use_cpflight_serial"
#define CFG_USE_IOCP_SERVER "use_iocp_server"

#define CFG_FMC_CONNECT_MODE "fmc_connect_mode"
#define CFG_FMC_CONNECT_MODE_MASTER_SERVER_PORT "fmc_connect_mode_master_server_port"
#define CFG_FMC_CONNECT_MODE_SLAVE_MASTER_IP "fmc_connect_mode_slave_master_ip"
#define CFG_FMC_CONNECT_MODE_SLAVE_MASTER_PORT "fmc_connect_mode_slave_master_port"

#define CFG_ENABLE_AIRBUS_FLAP_HANDLING_MODE "enable_airbus_flap_handling_mode"
#define CFG_ENABLE_SEPARATE_THROTTLE_LEVER_INPUTS "enable_seperate_throttle_lever_inputs"

#define CFG_VROUTE_AIRAC_RESTRICTION "vroute_airac_restriction"

//-----

#define CFG_SQUAWKBOX_HANDLING_MODE_DISABLED 0
#define CFG_SQUAWKBOX_HANDLING_MODE_OFF 1
#define CFG_SQUAWKBOX_HANDLING_MODE_ON 2
#define CFG_SQUAWKBOX_HANDLING_MODE_AUTOMATIC 3

#define CFG_ND_DISPLAY_MODE_NAV_ARC  0
#define CFG_ND_DISPLAY_MODE_NAV_ROSE 1
#define CFG_ND_DISPLAY_MODE_NAV_PLAN 2
#define CFG_ND_DISPLAY_MODE_VOR_ROSE 3
#define CFG_ND_DISPLAY_MODE_ILS_ROSE 4

#define CFG_ND_NAVAID_POINTER_TYPE_OFF 0
#define CFG_ND_NAVAID_POINTER_TYPE_NDB 1
#define CFG_ND_NAVAID_POINTER_TYPE_VOR 2

#define CFG_FMC_CONNECT_MODE_SINGLE "single"
#define CFG_FMC_CONNECT_MODE_MASTER "master"
#define CFG_FMC_CONNECT_MODE_SLAVE "slave"

#define CFG_TCAS_OFF 0
#define CFG_TCAS_STDBY 1
#define CFG_TCAS_ON 2

#endif /* __FMC_CONTROL_DEFINES_H__ */

// End of file

