///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2007 Alexander Wemmer 
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

/*! \file    fmc_gps.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __FMC_GPS_H__
#define __FMC_GPS_H__

#include <QWidget>
#include <QPoint>
#include <QTimer>
#include <QTime>

#include "defines.h"
#include "config.h"

#include "fmc_gps_defines.h"
#include "vas_widget.h"

class ConfigWidgetProvider;
class FMCControl;

/////////////////////////////////////////////////////////////////////////////

class FMCGPSStyleBase : public VasWidget
{
    Q_OBJECT

public:
	
    FMCGPSStyleBase(const QString& style,
                    ConfigWidgetProvider* config_widget_provider,
                    Config* main_config,
                    const QString& gps_config_filename,
                    FMCControl* fmc_control,
                    QWidget* parent,
                    Qt::WFlags fl);
       
    virtual ~FMCGPSStyleBase();

    inline Config* config() { return m_gps_config; }

    void incFontSize();
    void decFontSize();

signals:

    void signalRestart();


public slots:

    void slotControlTimer();

protected:

    void setupDefaultConfig(const QString& style);
    void loadWindowGeometry();
    void saveWindowGeometry();
    virtual void keyPressEvent(QKeyEvent *event) = 0;

protected:

    ConfigWidgetProvider* m_config_widget_provider;
    Config* m_main_config;
    Config* m_gps_config;
    FMCControl* m_fmc_control;

private:

    //! Hidden copy-constructor
    FMCGPSStyleBase(const FMCGPSStyleBase&);
    //! Hidden assignment operator
    const FMCGPSStyleBase& operator = (const FMCGPSStyleBase&);
};

/////////////////////////////////////////////////////////////////////////////

//! FMC gps handler (restarter)
class FMCGPSHandler : public QObject
{
    Q_OBJECT

public:
    //! Standard Constructor
    FMCGPSHandler(ConfigWidgetProvider* config_widget_provider,
                  Config* main_config,
                  const QString& gps_config_filename,
                  FMCControl* fmc_control);
    
    //! Destructor
    virtual ~FMCGPSHandler()
    {
        delete m_gps;
    }
    
    inline void show() { if (m_gps != 0) m_gps->show(); }
    inline void hide() { if (m_gps != 0) m_gps->hide(); }
    inline bool isVisible() 
    {
        if (m_gps == 0) return false;
        return m_gps->isVisible(); 
    }

    inline void close() { if (m_gps != 0) m_gps->close(); }

    FMCGPSStyleBase *fmcGpsBase()
    {
#if VASFMC_GAUGE
        // Protect m_gps from being accessed during a restart (when it might be invalid)
        QMutexLocker locker(&m_gps_mutex);
#endif
        return m_gps;
    }

    void restartGPS()
    {
#if VASFMC_GAUGE
        QMutexLocker locker(&m_gps_mutex);
#endif

        delete m_gps;
        m_gps = createGps();
        if (m_gps == 0) return;
        MYASSERT(connect(m_gps, SIGNAL(signalRestart()), this, SLOT(slotTriggerGpsRestart())));
    }

protected slots:

    void slotTriggerGpsRestart()
    {
        QTimer::singleShot(1, this, SLOT(slotRestartGps()));
    }

    void slotRestartGps()
    {
#if VASFMC_GAUGE
        QMutexLocker locker(&m_gps_mutex);
#endif

        delete m_gps;
        m_gps = createGps();
        if (m_gps == 0) return;
        MYASSERT(connect(m_gps, SIGNAL(signalRestart()), this, SLOT(slotTriggerGpsRestart())));
    }

protected:

    FMCGPSStyleBase* createGps();

protected:

    ConfigWidgetProvider* m_config_widget_provider;
    Config* m_main_config;
    QString m_gps_config_filename;
    FMCControl* m_fmc_control;
    FMCGPSStyleBase* m_gps;
#if VASFMC_GAUGE
    QMutex m_gps_mutex;
#endif

private:
    //! Hidden copy-constructor
    FMCGPSHandler(const FMCGPSHandler&);
    //! Hidden assignment operator
    const FMCGPSHandler& operator = (const FMCGPSHandler&);
};

#endif /* __FMC_GPS_H__ */

// End of file

