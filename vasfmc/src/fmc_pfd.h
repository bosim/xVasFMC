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

#ifndef FMCPFD_H
#define FMCPFD_H

#include "math.h"

#include <QApplication>
#include <QString>
#include <QColor>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QMenu>
#include <QTimer>

#include "vas_gl.h"

#include "defines.h"
#include "config.h"

#include "fmc_pfd_defines.h"
#include "ui_fmc_pfd.h"
#include "vas_widget.h"

class FlightStatus;
class GLPFDWidgetBase;
class OpenGLText;
class FMCControl;

/////////////////////////////////////////////////////////////////////////////

class FMCPFD : public VasWidget
#if !VASFMC_GAUGE
    , private Ui::FMCPFDDisplay
#endif
{
    Q_OBJECT

public:
	
    FMCPFD(ConfigWidgetProvider* config_widget_provider,
           Config* main_config,
           const QString& pfd_config_filename,
           FMCControl* fmc_control,
           QWidget* parent,
           Qt::WFlags fl,
           bool left_side);
       
    virtual ~FMCPFD();

#if VASFMC_GAUGE
    virtual void paintBitmapToGauge(PELEMENT_STATIC_IMAGE pelement);
#endif

    void processFSControls();

signals:

    void signalRestart();

    void signalTimeUsed(const QString& name, uint millisecs);

public slots:

    void slotRefresh();

protected:

    void setupDefaultConfig();
	void loadWindowGeometry();
	void saveWindowGeometry();
    void setupRangesList();

    void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event) {     m_last_mouse_position = event->globalPos(); }
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent* event);
 	void resizeEvent(QResizeEvent* event);

protected:

    ConfigWidgetProvider* m_config_widget_provider;
    Config* m_main_config;
    Config* m_pfd_config;
    FMCControl* m_fmc_control;
    GLPFDWidgetBase* m_gl_pfd;
    QPoint m_last_mouse_position;
    bool m_left_side;
};

/////////////////////////////////////////////////////////////////////////////

//! FMC pfd handler (restarter)
class FMCPFDHandler : public QObject
{
    Q_OBJECT

public:

    //! Standard Constructor
    FMCPFDHandler(ConfigWidgetProvider* config_widget_provider, 
                  Config* main_config,
                  const QString& pfd_config_filename,
                  FMCControl* fmc_control,
                  bool left_side);
    
    //! Destructor
    virtual ~FMCPFDHandler()
    {
        delete m_pfd;
    }

    inline void show() { if (m_pfd != 0) m_pfd->show(); }
    inline void hide() { if (m_pfd != 0) m_pfd->hide(); }

    inline bool isVisible() 
    {
        if (m_pfd == 0) return false;
        return m_pfd->isVisible(); 
    }

    inline void close() { if (m_pfd != 0) m_pfd->close(); }

    FMCPFD *fmcPFD()
    {
#if VASFMC_GAUGE
        // Protect m_pfd from being accessed during a restart (when it might be invalid)
        QMutexLocker locker(&m_pfd_mutex);
#endif
        return m_pfd;
    }

public slots:

    void slotRestartPFD()
    {
#if VASFMC_GAUGE
        QMutexLocker locker(&m_pfd_mutex);
#endif
        delete m_pfd;
        m_pfd = createPFD();
        if (m_pfd == 0) return;
        MYASSERT(connect(m_pfd, SIGNAL(signalRestart()), this, SLOT(slotTriggerPFDRestart())));
    }

protected slots:

    void slotTriggerPFDRestart() { QTimer::singleShot(1, this, SLOT(slotRestartPFD())); }

protected:

    FMCPFD* createPFD();

protected:

    ConfigWidgetProvider* m_config_widget_provider;
    Config* m_main_config;
    QString m_pfd_config_filename;
    FMCControl* m_fmc_control;
    FMCPFD* m_pfd;
#if VASFMC_GAUGE
    QMutex m_pfd_mutex;
#endif

    bool m_left_side;

private:
    //! Hidden copy-constructor
    FMCPFDHandler(const FMCPFDHandler&);
    //! Hidden assignment operator
    const FMCPFDHandler& operator = (const FMCPFDHandler&);
};

#endif // FMCPFD_H
