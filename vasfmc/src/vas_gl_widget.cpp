///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2006 Martin B�hme
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

#include "vas_gl_widget.h"

#if VASFMC_GAUGE || VAS_GL_EMUL

#ifdef Q_OS_WIN32
#include "code_timer.h"
#endif

#if !VASFMC_GAUGE
#include <QPainter>
#endif

#define CODETIMER 0

VasGLWidget::VasGLWidget(const QGLFormat &format, VasWidget *parent /* =0 */)
#if VASFMC_GAUGE
    : m_pBuffer(NULL), m_size(0, 0)
#else
    : VasWidget(parent), m_pBuffer(NULL), m_size(0, 0)
#endif
{
}

/* virtual */ VasGLWidget::~VasGLWidget()
{
    if(m_pBuffer!=NULL)
        m_pBuffer->doneCurrent();

    delete m_pBuffer;
}

void VasGLWidget::makeCurrent()
{
    int dxDesired, dyDesired;

    getDesiredSize(&dxDesired, &dyDesired);
    createPixelBuffer(QSize(dxDesired, dyDesired));
    m_pBuffer->makeCurrent(pimgCur());
}

void VasGLWidget::doneCurrent()
{
    if(m_pBuffer!=NULL)
        m_pBuffer->doneCurrent();
}

void VasGLWidget::qglColor(const QColor &c) const
{
    glColor4f(c.redF(), c.greenF(), c.blueF(), c.alphaF());
}

void VasGLWidget::qglClearColor(const QColor &c) const
{
    glClearColor(c.redF(), c.greenF(), c.blueF(), c.alphaF());
}

void VasGLWidget::updateGL()
{
    int dxDesired, dyDesired;
#if CODETIMER
    CodeTimer timer;
#endif
    QString strPaintGL, strConvert;

    // Get the desired size for the bitmap (i.e. the size of the window).
    getDesiredSize(&dxDesired, &dyDesired);
    if(dxDesired==0 || dyDesired==0)
    {
        // Make sure there is no GL context current when we flip
        doneCurrent();

        // Flip buffers so we get a chance to resize the buffer to the desired
        // size
        flip();
        return;
    }

    // Create an OpenGL pixel buffer and make the GL context current
    createPixelBuffer(QSize(dxDesired, dyDesired));
    m_pBuffer->makeCurrent(pimgCur());

    // Paint the gauge using OpenGL
#if CODETIMER
    timer.Tick();
#endif

    paintGL();

#if CODETIMER
    strPaintGL=timer.Tock();
    timer.Tick();
#endif

    // Convert the pixel buffer to a QImage
    m_pBuffer->doneCurrent();

#if CODETIMER
    strConvert=timer.Tock();
#endif

    // Add time taken to image
    QPainter painter(pimgCur());
    painter.setPen(QPen(Qt::white));
    painter.drawText(5, 25, strPaintGL + " " + strConvert);
    painter.end();

    // Flip buffers
    flip();
}

// protected:

#if !VASFMC_GAUGE
/* virtual */ void VasGLWidget::paintEvent(QPaintEvent *event)
{
    if(m_pixmap.size()==size())
    {
        QPainter painter(this);

        painter.drawPixmap(0, 0, m_pixmap);
    }
}
#endif

// private:

void VasGLWidget::createPixelBuffer(QSize size)
{
    QImage imgDummy;

    // If we haven't got a pixel buffer yet or if it's the wrong size...
    if(m_pBuffer==NULL || m_size!=size)
    {
        // Create a new VasGLPixelBuffer and remember its size
        delete m_pBuffer;
        m_pBuffer=new VasGLPixelBuffer(size);
        m_size=size;

        // Make sure the dummy image has the right size
        imgDummy=QImage(m_size.width(), m_size.height(),
            QImage::Format_ARGB32);
        imgDummy.bits();

        // Make the GL context current
        m_pBuffer->makeCurrent(&imgDummy);

        // Initialize the pixel buffer
        initializeGL();
        resizeGL(size.width(), size.height());

        m_pBuffer->doneCurrent();
    }
}

#if !VASFMC_GAUGE
void VasGLWidget::getDesiredSize(int *pdxDesired, int *pdyDesired)
{
    *pdxDesired=size().width();
    *pdyDesired=size().height();
}

void VasGLWidget::flip()
{
    // Store the image in m_pixmap
    m_pixmap=QPixmap::fromImage(m_img);

    // Update the window contents
    update();
}

QImage *VasGLWidget::pimgCur()
{
    if(m_img.width()!=size().width() || m_img.height()!=size().height())
    {
        m_img=QImage(size().width(), size().height(), QImage::Format_ARGB32);

        // Make sure m_img is deep-copied, i.e. it has is own buffer
        m_img.bits();
    }

    return &m_img;
}
#endif // !VASFMC_GAUGE

#endif // VASFMC_GAUGE || VAS_GL_EMUL
