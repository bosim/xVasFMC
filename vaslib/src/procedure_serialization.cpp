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

/*! \file    procedure_serialization.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "procedure_serialization.h"

/////////////////////////////////////////////////////////////////////////////

Procedure* getProcedureByType(const QString& proc_type)
{
    Procedure* proc = 0;
    
    if (proc_type == Route::TYPE_PROCEDURE)
        proc = new Procedure;
    else if (proc_type == Route::TYPE_SID)
        proc = new Sid;
    else if (proc_type == Route::TYPE_STAR)
        proc = new Star;
    else if (proc_type == Route::TYPE_APPROACH)
        proc = new Approach;
    else if (proc_type == Route::TYPE_TRANSITION)
        proc = new Transition;
    
    return proc;
}

/////////////////////////////////////////////////////////////////////////////

// End of file
