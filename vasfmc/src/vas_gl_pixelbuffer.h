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

/*! \file    vas_gl_pixelbuffer.h
    \author  Martin Böhme
*/

#ifndef VAS_GL_PIXELBUFFER_H
#define VAS_GL_PIXELBUFFER_H

#if VAS_GL_EMUL
#include "vas_gl.h"

class VasGLPixelBuffer
{
public:
    VasGLPixelBuffer(const QSize &size)
    {
        m_ctx=vasglCreateContext(size.width(), size.height());
        m_pimg=NULL;
    }

    ~VasGLPixelBuffer()
    {
        vasglFreeContext(m_ctx);
    }

    void makeCurrent(QImage *pimg)
    {
        vasglMakeCurrent(m_ctx, pimg);
        m_pimg=pimg;
    }

    bool doneCurrent()
    {
        m_pimg=NULL;

        vasglMakeCurrent(NULL, NULL);

        return true;
    }

private:
    VasGLRenderContext m_ctx;
    QImage             *m_pimg;
};
#else
#include <QGLPixelBuffer>

class VasGLPixelBuffer : public QGLPixelBuffer
{
public:
    VasGLPixelBuffer(const QSize &size)
        : QGLPixelBuffer(size), m_pimg(NULL)
    {
    }

    void makeCurrent(QImage *pimg)
    {
        m_pimg=pimg;
    }

    bool doneCurrent()
    {
        *m_pimg=toImage();
        m_pimg=NULL;

        return QGLPixelBuffer::doneCurrent();
    }

private:
    QImage *m_pimg;
};
#endif

#endif // VAS_GL_PIXELBUFFER_H
