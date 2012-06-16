///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2006 Martin Böhme
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

// vasfmc_gauge.mcdu.c

#include <windows.h>

#include <math.h>
#include <stdio.h>

//	Set up gauge header
char mcdu_gauge_name[] = GAUGE_NAME;
extern PELEMENT_HEADER mcdu_list;
extern MOUSERECT       mcdu_mouse_rect[];

GAUGE_HEADER_FS800(GAUGEHDR_VAR_NAME, GAUGE_W, mcdu_gauge_name, &mcdu_list, \
    mcdu_mouse_rect, McduCallback, 0, 0);

/////////////////////////////////////////////////////////////////////////////

FAILURE_RECORD	mcdu_fail[] =
{
	{ FAIL_NONE, FAIL_ACTION_NONE }
};

/////////////////////////////////////////////////////////////////////////////

MAKE_STATIC
(
	mcdu_canvas,
	BMP_MCDU_CANVAS,
	NULL,
	NULL,
	IMAGE_USE_ERASE | IMAGE_USE_BRIGHT | IMAGE_CREATE_DIBSECTION,
	0,
	0,0
)

PELEMENT_HEADER mcdu_list = &mcdu_canvas.header;

/////////////////////////////////////////////////////////////////////////////
MOUSE_BEGIN( mcdu_mouse_rect, HELP_NONE, 0, 0 )
    MOUSE_CHILD_FUNCT(MCDU_BORDER, MCDU_BORDER_TOP, MCDU_WIDTH-2*MCDU_BORDER,
        MCDU_HEIGHT-MCDU_BORDER-MCDU_BORDER_TOP, CURSOR_HAND,  // CROSSHAIR
        MOUSE_LEFTALL | MOUSE_RIGHTALL | MOUSE_WHEEL, McduMouseFunction)
MOUSE_END


/////////////////////////////////////////////////////////////////////////////
#undef GAUGE_NAME
#undef GAUGEHDR_VAR_NAME
#undef GAUGE_W
