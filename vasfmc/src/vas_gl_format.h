///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Martin Böhme
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

/*! \file    vas_gl_format.h
    \author  Martin Böhme
*/

#ifndef VAS_GL_FORMAT_H
#define VAS_GL_FORMAT_H

#include "vas_gl.h"

#if VAS_GL_EMUL
class QGLFormat
{
public:
    QGLFormat(QGL::FormatOption)
    {
    }
};
#else
#include <QGLFormat>
#endif

#endif // VAS_GL_FORMAT_H
