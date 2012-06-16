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

/*! \file    fmc_cdu_defines.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include <QRegExp>

#ifndef __FMC_CDU_DEFINES_H__
#define __FMC_CDU_DEFINES_H__

#define CFG_CDU_POS_X "cdux"
#define CFG_CDU_POS_Y "cduy"
#define CFG_CDU_WIDTH "cduwidth"
#define CFG_CDU_HEIGHT "cduheight"
#define CFG_CDU_WINDOW_STATUS "cduwindowstatus"
#define CFG_CDU_INPUT_AREA_FILE "cduinputareafile"

#define CFG_CDU_REFRESH_PERIOD_MS "cdu_refresh_period"
#define CFG_CDU_BACKGROUND_IMAGE "background_image"
#define CFG_CDU_FONTNAME "font_name"
#define CFG_CDU_FONTSIZE "font_size"
#define CFG_CDU_DISPLAY_ONLY_MODE_FONTNAME "display_only_mode_font_name"
#define CFG_CDU_DISPLAY_ONLY_MODE_FONTSIZE "display_only_mode_font_size"
#define CFG_SCROLL_MODE "scroll_mode"

#define CFG_CDU_DISPLAY_X "cdu_display_x"
#define CFG_CDU_DISPLAY_Y "cdu_display_y"
#define CFG_CDU_DISPLAY_WIDTH "cdu_display_width"
#define CFG_CDU_DISPLAY_HEIGHT "cdu_display_height"

#define SMALL_FONT 0.75
#define NORM_FONT 1.0

#define SPACER QChar(2)

#define SCROLL_MODE_REGULAR 0
#define SCROLL_MODE_INVERSE 1

const static QRegExp LATLON_WAYPOINT_REGEXP = 
     QRegExp("^(N|S)(\\d{2,2})\\.(\\d{1,2}\\.\\d{1,2})/(E|W)(\\d{2,3})\\.(\\d{1,2}\\.\\d{1,2})$");

#endif /* __FMC_CDU_DEFINES_H__ */

// End of file

