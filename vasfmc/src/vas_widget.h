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

#ifndef __VAS_WIDGET_H__
#define __VAS_WIDGET_H__

#include <QWidget>

#if VASFMC_GAUGE
#include <QMutex>
#include <QObject>
#include <QPainter>

#include <fs9gauges.h>

#include <windows.h>

/////////////////////////////////////////////////////////////////////////////

class VasWidget : public QObject
// Base class for widgets that are displayed as a gauge in MSFS. VasWidget
// implements some of the interface of QWidget, minimizing the amount of
// changes needed between the gauge version and the standalone version of
// vasFMC. In the standalone version, VasWidget is simply a typedef for
// QWidget.
{
public:
    VasWidget(QWidget *parent=0, Qt::WindowFlags f=0)
        // 'parent' and 'f' are simply for compatibility and are otherwise ignored
        : m_gauge_background_width(1), m_gauge_background_height(1), m_dxDesired(0), m_dyDesired(0)
    {
        Q_UNUSED(f);
        Q_UNUSED(parent);

        m_iImgCur=0;
        m_images[0]=QImage(0, 0, QImage::Format_ARGB32);
        m_images[1]=QImage(0, 0, QImage::Format_ARGB32);

        // Make sure the images are deep-copied, i.e. they have their own buffers
        m_images[0].bits();
        m_images[1].bits();

        m_gaugeCurrent=false;
    }

    ~VasWidget()
    {
    }

    //! Dispatches the event 'e' to the relevant event handler (overridden from QObject).
    virtual bool event(QEvent *e);

    //! Override this in the derived class to paint the widget to the QPainter.
    virtual void paintGauge(QPainter *pPainter);

    //! Calls paintGauge() to paint the widget to an internal bitmap
    //! (should be called at regular intervals from a timer or when the
    //! gauge needs to be updated). The bitmap can then be copied to the
    //! screen by paintBitmapToGauge();
    virtual void paintToBitmap();

    //! Copies the internal bitmap (created by paintToBitmap()) to the
    //! gauge 'pelement'. Calls SET_OFF_SCREEN() if necessary.
    virtual void paintBitmapToGauge(PELEMENT_STATIC_IMAGE pelement);
    
    //! Does the same as paintToBitmap(); provided for compatibility with QWidget.
    void update() { paintToBitmap(); }
    
    // Dummy functions
    // These functions are provided for compatibility with QWidget but do not
    // do anything meaningful
    void show() {}
    void hide() {}
    bool isVisible() { return true; }
    void close() {}
    inline QSize size() const { return QSize(pimgCur()->width(), pimgCur()->height()); }
    inline int width() const { return pimgCur()->width(); }
    inline int height() const { return pimgCur()->height(); }
    void resize(int , int ) {}
    QPoint pos() { return QPoint(0, 0); }
    void move(int , int ) {}

    inline int gaugeBackgroundWidth() const { return m_gauge_background_width; }
    inline int gaugeBackgroundHeight() const { return m_gauge_background_height; }

protected:
    virtual void mousePressEvent(QMouseEvent *) {}
    virtual void mouseReleaseEvent(QMouseEvent *) {}
    virtual void wheelEvent(QWheelEvent *) {}
    virtual void keyPressEvent(QKeyEvent *) {}
    virtual void keyReleaseEvent(QKeyEvent *) {}

    //! Returns the current size of the gauge in pixels (i.e. the desired size for the bitmap)
    void getDesiredSize(int *pdxDesired, int *pdyDesired);
      
    void flip();

    //! The "current" image is the one that is currently being drawn to
    inline const QImage *pimgCur() const { return &m_images[m_iImgCur]; }
    //! The "current" image is the one that is currently being drawn to
    inline QImage *pimgCur() { return &m_images[m_iImgCur]; }

protected:

    int m_gauge_background_width;
    int m_gauge_background_height;

private:
    // Mutex for protecting member variables
    QMutex m_mutex;

    // Desired size for bitmap
    int m_dxDesired, m_dyDesired;

    QImage m_images[2];
    int m_iImgCur;
    bool m_gaugeCurrent;
};

#else // if we are not a gauge

class VasWidget : public QWidget
{
public:
    VasWidget(QWidget *parent=0, Qt::WindowFlags f=0) : QWidget(parent, f), m_is_visible(false), m_got_close(false)
    {};

    ~VasWidget()
    {}

    void close();

protected:

    void hideEvent(QHideEvent* event);
    void showEvent(QShowEvent* event);
    void closeEvent(QCloseEvent* event);

protected:

    bool m_is_visible;
    bool m_got_close;
};

#endif

#endif // __VAS_WIDGET_H__
