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

/*! \file    fmc_cdu.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __FMC_CDU_H__
#define __FMC_CDU_H__

#include <QWidget>
#include <QPoint>
#include <QTime>
#include <QTimer>

#include "defines.h"
#include "config.h"

#include "fmc_cdu_defines.h"
#include "vas_widget.h"

class ConfigWidgetProvider;
class FMCControl;

/////////////////////////////////////////////////////////////////////////////

class FMCCDUStyleBase : public VasWidget
{
    Q_OBJECT

public:
	
    FMCCDUStyleBase(const QString& style,
                    ConfigWidgetProvider* config_widget_provider,
                    Config* main_config,
                    const QString& cdu_config_filename,
                    FMCControl* fmc_control,
                    QWidget* parent,
                    Qt::WFlags fl,
                    bool left_side);
       
    virtual ~FMCCDUStyleBase();

    inline Config* config() { return m_cdu_config; }

    void incFontSize();
    void decFontSize();

    void slotProcessInput();

    bool normalScrollMode() const { return m_cdu_config->getIntValue(CFG_SCROLL_MODE) == SCROLL_MODE_REGULAR; }
    bool inverseScrollMode() const { return m_cdu_config->getIntValue(CFG_SCROLL_MODE) != SCROLL_MODE_REGULAR; }
    void toggleScrollMode() { inverseScrollMode() ? slotNormalScrolling() : slotInverseScrolling(); }

signals:

    void signalRestart();

    void signalTimeUsed(const QString& name, uint millisecs);

public slots:

    virtual void slotRefresh() = 0;

protected slots:

    void slotNormalScrolling() { m_cdu_config->setValue(CFG_SCROLL_MODE, SCROLL_MODE_REGULAR); }
    void slotInverseScrolling() { m_cdu_config->setValue(CFG_SCROLL_MODE, SCROLL_MODE_INVERSE); }

    void slotShowKbdActiveIndicator(bool show);

protected:

    void setupDefaultConfig(const QString& style);
    void loadWindowGeometry();
    void saveWindowGeometry();
    virtual void keyPressEvent(QKeyEvent *event) = 0;

protected:

    ConfigWidgetProvider* m_config_widget_provider;
    Config* m_main_config;
    Config* m_cdu_config;
    FMCControl* m_fmc_control;
    bool m_showKbdActiveIndicator;

    bool m_left_side;

private:

    //! Hidden copy-constructor
    FMCCDUStyleBase(const FMCCDUStyleBase&);
    //! Hidden assignment operator
    const FMCCDUStyleBase& operator = (const FMCCDUStyleBase&);
};

/////////////////////////////////////////////////////////////////////////////

//! FMC cdu handler (restarter)
class FMCCDUHandler : public QObject
{
    Q_OBJECT

public:
    //! Standard Constructor
    FMCCDUHandler(ConfigWidgetProvider* config_widget_provider,
                  Config* main_config,
                  const QString& cdu_config_filename,
                  FMCControl* fmc_control,
                  bool left_side);
    
    //! Destructor
    virtual ~FMCCDUHandler()
    {
        delete m_cdu;
    }
    
    inline void show() { if (m_cdu != 0) m_cdu->show(); }
    inline void hide() { if (m_cdu != 0) m_cdu->hide(); }

    inline bool isVisible()
    {
        if (m_cdu == 0) return false;
        return m_cdu->isVisible(); 
    }
    
    inline void close() { if (m_cdu != 0) m_cdu->close(); }

    FMCCDUStyleBase *fmcCduBase()
    {
#if VASFMC_GAUGE
        // Protect m_cdu from being accessed during a restart (when it might be invalid)
        QMutexLocker locker(&m_cdu_mutex);
#endif
        return m_cdu;
    }

public slots:

    void slotRestartCdu()
    {
#if VASFMC_GAUGE
        QMutexLocker locker(&m_cdu_mutex);
#endif

        delete m_cdu;
        m_cdu = createCdu();
        if (m_cdu == 0) return;
        MYASSERT(connect(m_cdu, SIGNAL(signalRestart()), this, SLOT(slotTriggerCduRestart())));
    }

protected slots:

    void slotTriggerCduRestart() { QTimer::singleShot(1, this, SLOT(slotRestartCdu())); }

protected:

    FMCCDUStyleBase* createCdu();

protected:

    ConfigWidgetProvider* m_config_widget_provider;
    Config* m_main_config;
    QString m_cdu_config_filename;
    FMCControl* m_fmc_control;
    FMCCDUStyleBase* m_cdu;
#if VASFMC_GAUGE
    QMutex m_cdu_mutex;
#endif

    bool  m_left_side;

private:
    //! Hidden copy-constructor
    FMCCDUHandler(const FMCCDUHandler&);
    //! Hidden assignment operator
    const FMCCDUHandler& operator = (const FMCCDUHandler&);
};

#endif /* __FMC_CDU_H__ */

// End of file

