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

/*! \file    ge.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "ge_projection.h"

GraphicElementProjectionBase::GraphicElementProjectionBase(const ProjectionBase* projection,
                                                           bool is_moveable) :
    GraphicElementBase(is_moveable), m_projection(projection), m_projection_changed(true)
{
    MYASSERT(m_projection != 0);
    MYASSERT(connect(projection, SIGNAL(signalChanged()), this, SLOT(slotProjectionChanged())));
}

// End of file
