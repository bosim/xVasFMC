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

#ifndef FMCNAVDISPLAY_H
#define FMCNAVDISPLAY_H

#include "math.h"

#include <QApplication>
#include <QString>
#include <QColor>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTimer>

#include "vas_gl.h"

#include "defines.h"
#include "config.h"

#include "fmc_navdisplay_defines.h"
#include "ui_fmc_navdisplay.h"
#include "vas_widget.h"
#include "fmc_autopilot.h"

class FlightStatus;
class FMCNavdisplayStyle;
class GLNavdisplayWidget;
class OpenGLText;
class FMCControl;

/////////////////////////////////////////////////////////////////////////////

class FMCNavdisplay : 
    public VasWidget
#if !VASFMC_GAUGE
    , private Ui::FMCNavDisplay
#endif
{
    Q_OBJECT

public:
	
    FMCNavdisplay(ConfigWidgetProvider* config_widget_provider,
                  Config* main_config,
                  const QString& navdisplay_config_filename,
                  const QString& tcas_config_filename,
                  FMCControl* fmc_control,
                  QWidget* parent,
                  Qt::WFlags fl,
                  bool left_side);
       
    virtual ~FMCNavdisplay();

#if VASFMC_GAUGE
    virtual void paintBitmapToGauge(PELEMENT_STATIC_IMAGE pelement);
#endif

    void processFSControls();

    bool doWindCorrection() const { return m_navdisplay_config->getIntValue(CFG_WIND_CORRECTION) != 0; }
    void toggleWindCorrection() { doWindCorrection() ? slotDisableWindCorrection() : slotEnableWindCorrection(); }

    bool normalScrollMode() const { return m_navdisplay_config->getIntValue(CFG_SCROLL_MODE) == SCROLL_MODE_REGULAR; }
    bool inverseScrollMode() const { return m_navdisplay_config->getIntValue(CFG_SCROLL_MODE) != SCROLL_MODE_REGULAR; }
    void toggleScrollMode() { inverseScrollMode() ? slotNormalScrolling() : slotInverseScrolling(); }

signals:

    void signalRestart();

    void signalTimeUsed(const QString& name, uint millisecs);

public slots:

    void slotRefresh();

    void slotEnableWindCorrection() { m_navdisplay_config->setValue(CFG_WIND_CORRECTION, 1); }
    void slotDisableWindCorrection() { m_navdisplay_config->setValue(CFG_WIND_CORRECTION, 0); }

    void slotNormalScrolling() { m_navdisplay_config->setValue(CFG_SCROLL_MODE, SCROLL_MODE_REGULAR); }
    void slotInverseScrolling() { m_navdisplay_config->setValue(CFG_SCROLL_MODE, SCROLL_MODE_INVERSE); }

protected:

    void setupDefaultConfig();
	void loadWindowGeometry();
	void saveWindowGeometry();

    void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event) {     m_last_mouse_position = event->globalPos(); }
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent* event);
 	void resizeEvent(QResizeEvent* event);

protected:

    ConfigWidgetProvider* m_config_widget_provider;
    Config* m_main_config;
    Config* m_navdisplay_config;
    Config* m_tcas_config;
    FMCControl* m_fmc_control;
    GLNavdisplayWidget* m_gl_navdisp;
    QPoint m_last_mouse_position;
    bool m_left_side;
};

/////////////////////////////////////////////////////////////////////////////

//! FMC navdisplay handler (restarter)
class FMCNavdisplayHandler : public QObject
{
    Q_OBJECT

public:
    //! Standard Constructor
    FMCNavdisplayHandler(ConfigWidgetProvider* config_widget_provider, 
                         Config* main_config,
                         const QString& navdisplay_config_filename,
                         const QString& tcas_config_filename,
                         FMCControl* fmc_control,
                         bool left_side);    

    //! Destructor
    virtual ~FMCNavdisplayHandler()
    {
        delete m_navdisplay;
    }

    inline void show() { m_navdisplay->show(); }
    inline void hide() { m_navdisplay->hide(); }
    inline bool isVisible() { return m_navdisplay->isVisible(); }
    inline void close() { m_navdisplay->close(); }

    FMCNavdisplay *fmcNavdisplay()
    {
#if VASFMC_GAUGE
        // Protect m_navdisplay from being accessed during a restart (when it might be invalid)
        QMutexLocker locker(&m_navdisplay_mutex);
#endif
        return m_navdisplay;
    }

public slots:

    void slotRestartNavdisplay()
    {
#if VASFMC_GAUGE
        QMutexLocker locker(&m_navdisplay_mutex);
#endif
        delete m_navdisplay;
        m_navdisplay = createNavdisplay();
        MYASSERT(m_navdisplay != 0);
        MYASSERT(connect(m_navdisplay, SIGNAL(signalRestart()), this, SLOT(slotTriggerNavdisplayRestart())));
    }

protected slots:

    void slotTriggerNavdisplayRestart() { QTimer::singleShot(1, this, SLOT(slotRestartNavdisplay())); }

protected:

    FMCNavdisplay* createNavdisplay();

protected:

    ConfigWidgetProvider* m_config_widget_provider;
    Config* m_main_config;
    QString m_navdisplay_config_filename;
    QString m_tcas_config_filename;
    FMCControl* m_fmc_control;
    FMCNavdisplay* m_navdisplay;
#if VASFMC_GAUGE
    QMutex m_navdisplay_mutex;
#endif

    bool m_left_side;

private:
    //! Hidden copy-constructor
    FMCNavdisplayHandler(const FMCNavdisplayHandler&);
    //! Hidden assignment operator
    const FMCNavdisplayHandler& operator = (const FMCNavdisplayHandler&);
};

#endif // FMCNAVDISPLAY_H
