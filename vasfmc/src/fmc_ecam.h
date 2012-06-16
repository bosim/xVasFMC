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

#ifndef FMCECAM_H
#define FMCECAM_H

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

#include "fmc_ecam_defines.h"
#include "ui_fmc_ecam.h"
#include "vas_widget.h"

class FlightStatus;
class GLECAMWidgetBase;
class OpenGLText;
class FMCControl;

/////////////////////////////////////////////////////////////////////////////

class FMCECAM : public VasWidget
#if !VASFMC_GAUGE
    , private Ui::FMCECAMDisplay
#endif
{
    Q_OBJECT

public:
	
    FMCECAM(bool upper_ecam,
            ConfigWidgetProvider* config_widget_provider,
            Config* main_config,
            const QString& ecam_config_filename,
            FMCControl* fmc_control,
            QWidget* parent,
            Qt::WFlags fl);
       
    virtual ~FMCECAM();

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
    
    bool m_upper_ecam;
    ConfigWidgetProvider* m_config_widget_provider;
    Config* m_main_config;
    Config* m_ecam_config;
    FMCControl* m_fmc_control;
    GLECAMWidgetBase* m_gl_ecam;
    QPoint m_last_mouse_position;
};

/////////////////////////////////////////////////////////////////////////////

//! FMC ecam handler (restarter)
class FMCECAMHandler : public QObject
{
    Q_OBJECT

public:
    //! Standard Constructor
    FMCECAMHandler(bool upper_ecam,
                   ConfigWidgetProvider* config_widget_provider, 
                   Config* main_config,
                   const QString& ecam_config_filename,
                   FMCControl* fmc_control);
    
    //! Destructor
    virtual ~FMCECAMHandler()
    {
        delete m_ecam;
    }

    inline bool isVisible() 
    {
        if (m_ecam == 0) return false;
        return m_ecam->isVisible(); 
    }

    inline void show()  { if (m_ecam != 0) m_ecam->show(); }
    inline void hide()  { if (m_ecam != 0) m_ecam->hide(); }
    inline void close() { if (m_ecam != 0) m_ecam->close(); }

    FMCECAM *fmcECAM()
    {
#if VASFMC_GAUGE
        // Protect m_ecam from being accessed during a restart (when it might be invalid)
        QMutexLocker locker(&m_ecam_mutex);
#endif
        return m_ecam;
    }

public slots:

    void slotRestartECAM()
    {
#if VASFMC_GAUGE
        QMutexLocker locker(&m_ecam_mutex);
#endif
        delete m_ecam;
        m_ecam = createECAM();
        if (m_ecam == 0) return;
        MYASSERT(connect(m_ecam, SIGNAL(signalRestart()), this, SLOT(slotTriggerECAMRestart())));
    }

protected slots:

    void slotTriggerECAMRestart() { QTimer::singleShot(1, this, SLOT(slotRestartECAM())); }

protected:

    FMCECAM* createECAM();

protected:

    bool m_upper_ecam;
    ConfigWidgetProvider* m_config_widget_provider;
    Config* m_main_config;
    QString m_ecam_config_filename;
    FMCControl* m_fmc_control;
    FMCECAM* m_ecam;
#if VASFMC_GAUGE
    QMutex m_ecam_mutex;
#endif

private:
    //! Hidden copy-constructor
    FMCECAMHandler(const FMCECAMHandler&);
    //! Hidden assignment operator
    const FMCECAMHandler& operator = (const FMCECAMHandler&);
};

#endif // FMCECAM_H
