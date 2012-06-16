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

// vasfmc_gauge.nd.c

#include <windows.h>

#include <math.h>
#include <stdio.h>

//	Set up gauge header
char nd_gauge_name[] = GAUGE_NAME;
extern PELEMENT_HEADER nd_list;
extern MOUSERECT       nd_mouse_rect[];

GAUGE_HEADER_FS800(GAUGEHDR_VAR_NAME, GAUGE_W, nd_gauge_name, &nd_list, \
    nd_mouse_rect, NdCallback, 0, 0);

/////////////////////////////////////////////////////////////////////////////

FAILURE_RECORD	nd_fail[] =
{
	{ FAIL_NONE, FAIL_ACTION_NONE }
};

/////////////////////////////////////////////////////////////////////////////

MAKE_STATIC
(
	nd_canvas,
	BMP_ND_CANVAS,
	NULL,
	NULL,
	IMAGE_USE_ERASE | IMAGE_USE_BRIGHT | IMAGE_CREATE_DIBSECTION,
	0,
	0,0
)

PELEMENT_HEADER nd_list = &nd_canvas.header;

/////////////////////////////////////////////////////////////////////////////
MOUSE_BEGIN( nd_mouse_rect, HELP_NONE, 0, 0 )
    MOUSE_CHILD_FUNCT(ND_BORDER, ND_BORDER, ND_WIDTH-2*ND_BORDER,
        ND_HEIGHT-2*ND_BORDER, CURSOR_HAND,
        MOUSE_LEFTALL | MOUSE_RIGHTALL | MOUSE_WHEEL, NdMouseFunction)
MOUSE_END


/////////////////////////////////////////////////////////////////////////////
#undef GAUGE_NAME
#undef GAUGEHDR_VAR_NAME
#undef GAUGE_W
