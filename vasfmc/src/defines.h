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

#ifndef DEFINES_H
#define DEFINES_H

/////////////////////////////////////////////////////////////////////////////


// MAIN

#if VASFMC_GAUGE
#define PROCNAME "vasFMC Gauge"
#else
#define PROCNAME "VFMC"
#endif

#define COPYRIGHT "VAS Project Team (www.vas-project.org)"
#define COPYRIGHT_SHORT "VAS Project Team"
#define HOMEPAGE "www.vas-project.org"
#define VERSION "2.0a9"

#define CFG_DIR "cfg"

// COLORS

#define ORANGE QColor("#E6933F")
#define GREEN Qt::green
#define CYAN Qt::cyan
#define MAGENTA Qt::magenta
#define YELLOW Qt::yellow
#define WHITE Qt::white
#define BLACK Qt::black
#define RED Qt::red
#define AMBER "#FFBF00"

// CONFIG FILENAMES

#define CFG_MAIN_FILENAME CFG_DIR"/vasfmc.cfg"
#define CFG_MSFS_FILENAME CFG_DIR"/fsaccess_msfs.cfg"
#define CFG_FGFS_FILENAME CFG_DIR"/fsaccess_fgfs.cfg"
#define CFG_XPLANE_FILENAME CFG_DIR"/fsaccess_xplane.cfg"
#define CFG_NAVDATA_FILENAME CFG_DIR"/navdata.cfg"
#define CFG_NAVDATA_INDEX_FILENAME CFG_DIR"/navdata_index.cfg"
#define CFG_AUTOPILOT_FILENAME CFG_DIR"/autopilot.cfg"
#define CFG_AUTOTHROTTLE_FILENAME CFG_DIR"/autothrottle.cfg"
#define CFG_PROCESSOR_FILENAME CFG_DIR"/processor.cfg"
#define CFG_CONTROL_FILENAME CFG_DIR"/control.cfg"
#define CFG_DISPLAY_CONTROL_FILENAME CFG_DIR"/display_control.cfg"
#define CFG_NAVDISPLAY_LEFT_FILENAME CFG_DIR"/navdisplay_left.cfg"
#define CFG_NAVDISPLAY_RIGHT_FILENAME CFG_DIR"/navdisplay_right.cfg"
#define CFG_NAVDISPLAY_STYLE_A_FILENAME CFG_DIR"/navdisplay_style_a.cfg"
#define CFG_NAVDISPLAY_STYLE_B_FILENAME CFG_DIR"/navdisplay_style_b.cfg"
#define CFG_UPPER_ECAM_FILENAME CFG_DIR"/upper_ecam.cfg"
#define CFG_LOWER_ECAM_FILENAME CFG_DIR"/lower_ecam.cfg"
#define CFG_PFD_LEFT_FILENAME CFG_DIR"/pfd_left.cfg"
#define CFG_PFD_RIGHT_FILENAME CFG_DIR"/pfd_right.cfg"
#define CFG_TCAS_FILENAME CFG_DIR"/tcas.cfg"
#define CFG_CDU_LEFT_FILENAME CFG_DIR"/cdu_left.cfg"
#define CFG_CDU_RIGHT_FILENAME CFG_DIR"/cdu_right.cfg"
#define CFG_FCU_FILENAME CFG_DIR"/fcu.cfg"
#define CFG_GPS_FILENAME CFG_DIR"/gps.cfg"
#define CFG_CONSOLE_FILENAME CFG_DIR"/console.cfg"
#define CFG_WEATHER_FILENAME CFG_DIR"/weather.cfg"
#define CFG_SOUNDS_STYLE_A_FILENAME CFG_DIR"/sounds_style_a.cfg"
#define CFG_SOUNDS_STYLE_B_FILENAME CFG_DIR"/sounds_style_b.cfg"
#define CFG_CPFLIGHT_FILENAME CFG_DIR"/cpflight.cfg"
#define CFG_IOCP_FILENAME CFG_DIR"/iocp.cfg"

#define CFG_STYLE "style"
#define CFG_STYLE_A 0
#define CFG_STYLE_B 1
#define CFG_STYLE_G 2

#define CFG_VASFMC_DIR "vasfmcdir"
#define CFG_STARTUP_COUNTER "startup_counter"
#define CFG_FS_ACCESS_TYPE "fs_access_type"
#define CFG_LOGFILE_NAME "vasfmc.log"
#define CFG_PERSISTANCE_FILE "persistence_file"
#define SPLASHSCREEN_FILE "graphics/vasfmc-splash.png"
#define SPLASH_SHOW_TIME_MS 3000
#define CFG_FLIGHTPLAN_SUBDIR "flightplan_subdir"
#define CFG_AIRCRAFT_DATA_SUBDIR "aircraft_data_subdir"
#define CFG_AIRCRAFT_DATA_EXTENSION ".cfg"
#define CFG_FLIGHTPLAN_EXTENSION ".fmc"
#define CFG_GEODATA_FILE "geodata_file"
#define CFG_GEODATA_FILTER_LEVEL "geodata_filter_level"
#define CFG_DECLINATION_DATAFILE "declination_datafile"
#define CFG_FLIGHTSTATUS_SMOOTHING_DELAY_MS "flightstatus_smoothing_delay_ms"
#define CFG_BEST_ANTI_ALIASING "best_anti_aliasing"
#define CFG_CHECKLIST_SUBDIR "checklist_subdir"

#define CFG_FONT_NAME "font_name"
#define CFG_ACTIVE_FONT_SIZE "active_font_size"
#define CFG_ACTIVE_FONT_INDEX "active_font_index"
#define CFG_FONT_SIZE1 "font_size1"
#define CFG_FONT_SIZE2 "font_size2"
#define CFG_FONT_SIZE3 "font_size3"
#define CFG_FONT_SIZE4 "font_size4"
#define CFG_FONT_SIZE5 "font_size5"
#define CFG_FONT_SIZE6 "font_size6"
#define CFG_FONT_SIZE7 "font_size7"
#define CFG_FONT_SIZE8 "font_size8"
#define CFG_FONT_SIZE9 "font_size9"

#define FS_ACCESS_TYPE_MSFS "msfs"
#define FS_ACCESS_TYPE_XPLANE "xplane"
#define FS_ACCESS_TYPE_FGFS "fgfs"

#define FS_TIME_SYNC_MAX_DIFF_SEC 60

#endif

