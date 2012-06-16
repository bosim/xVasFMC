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

/*! \file    vas_gl_widget.h
    \author  Martin Böhme
*/

#ifndef __VAS_GL_WIDGET_H__
#define __VAS_GL_WIDGET_H__

#include "vas_gl_format.h"

#if VASFMC_GAUGE || VAS_GL_EMUL
#include "vas_widget.h"
#include "vas_gl_pixelbuffer.h"

#include <QColor>
#include <QImage>
#include <QObject>
#include <QPixmap>

class VasGLWidget : public VasWidget
// Base class for widgets that are drawn using OpenGL. VasGLWidget can be used
// as a replacement for QGLWidget since it uses the same interface. In the
// standalone version of vasFMC, VasGLWidget is simply a typedef for
// QGLWidget.
{
public:
    VasGLWidget(const QGLFormat &format, VasWidget *parent=0);
        // 'format' and 'parent' are only for compatibility and are ignored

    virtual ~VasGLWidget();

#if VASFMC_GAUGE
    virtual void paintToBitmap()
    {
        // Call updateGL() to paint the widget to an internal bitmap
        // (VasWidget override)
        updateGL();
    }
#endif

    void makeCurrent();

    void doneCurrent();

    void qglColor(const QColor &c) const;
        // Sets the current drawing color.
    
    void qglClearColor(const QColor &c) const;
        // Sets the current clearing color.

    void updateGL();

protected:
    // Override these methods to paint the widget (see QGLWidget documentation
    // for details)
    virtual void paintGL() = 0;
    virtual void resizeGL(int width, int height) = 0;
    virtual void initializeGL() = 0;

#if !VASFMC_GAUGE
    virtual void paintEvent(QPaintEvent *event);
#endif

private:
    void createPixelBuffer(QSize size);

#if !VASFMC_GAUGE
    void getDesiredSize(int *pdxDesired, int *pdyDesired);
    void flip();
    QImage *pimgCur();

    QImage         m_img;
    QPixmap        m_pixmap;
#endif

    VasGLPixelBuffer *m_pBuffer;
    QSize            m_size;
};
#else
#include <QGLWidget>

typedef QGLWidget VasGLWidget;
#endif // VASFMC_GAUGE || VAS_GL_EMUL

#endif // __VAS_GL_WIDGET_H__
