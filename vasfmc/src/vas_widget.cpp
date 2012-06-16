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

/*! \file    vas_widget.h
    \author  Martin Böhme
*/

#include "vas_widget.h"

#if VASFMC_GAUGE
#include "code_timer.h"
#include "mmx.h"

#include <windows.h>

#include <QMouseEvent>

static void convertScanlineARGBto1555(uint32_t *from, uint16_t *to, int len);

bool VasWidget::event(QEvent *e)
{
    switch(e->type())
    {
        case QEvent::MouseButtonPress:
            mousePressEvent((QMouseEvent *)e);
            break;
        case QEvent::MouseButtonRelease:
            mouseReleaseEvent((QMouseEvent *)e);
            break;
        case QEvent::Wheel:
            wheelEvent((QWheelEvent *)e);
            break;
        case QEvent::KeyPress:
            keyPressEvent((QKeyEvent *)e);
            break;
        case QEvent::KeyRelease:
            keyReleaseEvent((QKeyEvent *)e);
            break;
        default:
            return QObject::event(e);
    }

    return true;
}

void VasWidget::paintGauge(QPainter *pPainter)
{
}

void VasWidget::paintToBitmap()
{
    int dxDesired, dyDesired;

    // Get the desired size for the bitmap (i.e. the size of the gauge). If we don't know the size of the gauge yet, return.
    getDesiredSize(&dxDesired, &dyDesired);
    if(dxDesired==0 || dyDesired==0)
    {
        // Flip buffers so we get a chance to resize the buffer to the desired size
        flip();
        return;
    }

    QPainter painter;
    QPixmap  pixmap(dxDesired, dyDesired);

    // Paint to a pixmap
    painter.begin(&pixmap);
    paintGauge(&painter);
    painter.end();

    // Store the pixmap to the image and flip buffers
    *pimgCur()=pixmap.toImage();
    flip();
}

void VasWidget::paintBitmapToGauge(PELEMENT_STATIC_IMAGE pelement)
{
    PIMAGE pimg;
    HDC hdc;
    MMXState mmxState;

    // Get HDC for painting to gauge
    hdc=pelement->hdc;

    if(!hdc) return;

    // Get pointer to gauge image
    pimg=pelement->image_data.final;

    // Lock the mutex to make sure the bitmap doesn't get changed while we're
    // painting
    m_mutex.lock();

    QImage *pimgToOutput=&m_images[(m_iImgCur+1)%2];

    // Remember the desired size for the bitmap
    m_dxDesired=pimg->dim.x;
    m_dyDesired=pimg->dim.y;

    // If we have a bitmap with the correct size, copy it to the HDC
    if(!m_gaugeCurrent && pimgToOutput->width()==pimg->dim.x && pimgToOutput->height()==pimg->dim.y)
    {
        m_gaugeCurrent=true;

        mmxState.save();

        for(int y=0; y<pimg->dim.y; y++)
            convertScanlineARGBto1555((uint32_t *)pimgToOutput->scanLine(y),
                                      (uint16_t *)(pimg->image+y*pimg->pitch), pimg->dim.x);

        mmxState.restore();

        // Tell FS we updated the gauge
        SET_OFF_SCREEN(pelement);
    }

    // Unlock the mutex
    m_mutex.unlock();
}

// protected:

void VasWidget::getDesiredSize(int *pdxDesired, int *pdyDesired)
{
    // Desired size is determined according to the current bitmap
    *pdxDesired = pimgCur()->width();
    *pdyDesired = pimgCur()->height();
}

void VasWidget::flip()
{
    m_mutex.lock();
    
    m_iImgCur=(m_iImgCur+1)%2;
    m_gaugeCurrent=false;

    if(m_images[m_iImgCur].width()!=m_dxDesired | m_images[m_iImgCur].height()!=m_dyDesired)
    {
        // Resize image to desired size
        m_images[m_iImgCur]=QImage(m_dxDesired, m_dyDesired, QImage::Format_ARGB32);

        // Make sure the image is deep-copied, i.e. it has its own buffer
        m_images[m_iImgCur].bits();
    }

    m_mutex.unlock();
}

static void convertScanlineARGBto1555(uint32_t *from, uint16_t *to, int len)
{
    // TODO: Use C code for pixels left at the end of scanlines with odd length

#ifndef _X86_
    uint32_t r, g, b;

    while(len--)
    {
        r=(*from & 0xff) >> 3;
        g=(*from & 0xff00) >> (8+3);
        b=(*from & 0xff0000) >> (16+3);

        *to=r | (g<<5) | (b<<10);

        from++;
        to++;
    }
#else
    uint32_t masks[6]={ 0x1f, 0x1f, 0x3e0, 0x3e0, 0x7c00, 0x7c00 };

    __asm__ __volatile__
    (
        // Load zero into mm0
        "pxor       %%mm0, %%mm0\n\t"

        // Load mm4 to mm6 with masks
        "movq       (%%eax), %%mm4\n\t"
        "add        $8, %%eax\n\t"
        "movq       (%%eax), %%mm5\n\t"
        "add        $8, %%eax\n\t"
        "movq       (%%eax), %%mm6\n\t"

        // loop begin
        // TODO: Handle odd pixels
        "0:         \n\t"
        "sub        $2, %%ecx\n\t"
        "jl         0f\n\t"

        // Get input into mm1 and increment pointer
        "movq       (%%ebx), %%mm1\n\t"
        "add        $8, %%ebx\n\t"

        "psrld      $3, %%mm1\n\t"
        "movq       %%mm1, %%mm2\n\t"
        "pand       %%mm4, %%mm2\n\t"
        "psrld      $3, %%mm1\n\t"
        "movq       %%mm1, %%mm3\n\t"
        "pand       %%mm5, %%mm3\n\t"
        "psrld      $3, %%mm1\n\t"
        "pand       %%mm6, %%mm1\n\t"
        "por        %%mm3, %%mm2\n\t"
        "por        %%mm2, %%mm1\n\t"

        "packssdw   %%mm0, %%mm1\n\t"

        // Save result and increment pointer
        "movd       %%mm1, (%%edx)\n\t"
        "add        $4, %%edx\n\t"

        // loop end
        "jmp        0b\n\t"
        "0:"

    // output registers
    : // none
    
    // input registers
    : "a" (masks), "b" (from), "c" (len), "d" (to)

    // clobbers
    :
    );
#endif
}

#else // if we are not a gauge

#include "logger.h"

/////////////////////////////////////////////////////////////////////////////

void VasWidget::close()
{
    m_got_close = true;
    QWidget::close();
}

/////////////////////////////////////////////////////////////////////////////

void VasWidget::hideEvent(QHideEvent* event)
{
    if (!m_got_close) m_is_visible = false;
    QWidget::hideEvent(event);
}

/////////////////////////////////////////////////////////////////////////////

void VasWidget::showEvent(QShowEvent* event)
{
    m_is_visible = true;
    QWidget::showEvent(event);
}

/////////////////////////////////////////////////////////////////////////////

void VasWidget::closeEvent(QCloseEvent* event)
{
    QWidget::closeEvent(event);
}

#endif // VASFMC_GAUGE
