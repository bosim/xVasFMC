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

/*! \file    fmc_console.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __FMC_CONSOLE_H__
#define __FMC_CONSOLE_H__

#include <QMainWindow>
#include <QTimer>

#include "assert.h"
#include "logger.h"
#include "config.h"

#include "fmc_console_defines.h"

#include "ui_fmc_console.h"

class FMCGPSHandler;
class FMCFCUHandler;
class FMCCDUHandler;
class FMCNavdisplayHandler;
class FMCPFDHandler;
class FMCECAMHandler;
class FMCControl;
class FMCSoundsHandler;
class QCloseEvent;
class QSplashScreen;
class InfoDlgImpl;
class OpenGLText;
class QAction;

/////////////////////////////////////////////////////////////////////////////

//! console
class FMCConsole : 
#if VASFMC_GAUGE
    public QObject,
#else
    public QMainWindow, 
#endif
    public ConfigWidgetProvider
#if !VASFMC_GAUGE
    , private Ui::FMCConsole
#endif
{
    Q_OBJECT

public:
    //! Standard Constructor
    FMCConsole(QWidget* parent, Qt::WFlags fl, const QString& style = QString::null);

    //! Destructor
    virtual ~FMCConsole();

    virtual void registerConfigWidget(const QString& title, Config* cfg);
    virtual void unregisterConfigWidget(const QString& title);

public slots:

    void slotStyleA();
    void slotStyleB();
    void slotStyleG();
    void slotTriggerRestartFMC() { QTimer::singleShot(1, this, SLOT(slotRestartFMC())); }
    void slotTriggerRestartCDU() { QTimer::singleShot(1, this, SLOT(slotRestartCDU())); }

protected slots:

    void slotSetGLFontSize(uint size);

    void slotNDLeftButton();
    void slotPFDLeftButton();
    void slotUpperECAMButton();
    //TODOvoid slotLowerECAMButton();
    void slotCDULeftButton();
    void slotFCUButton();
    void slotGPSButton();
#if !VASFMC_GAUGE
    void slotNDRightButton();
    void slotPFDRightButton();
    void slotCDURightButton();
#endif

    void slotLogging(const QString& text);
    void slotDeleteSplashscreen();

    void slotHelpKeys() { showInfoDialog(HELP_KEYS_FILE); }
    void slotHelpFSUIPCOffsets() { showInfoDialog(HELP_FSUIPC_OFFSETS); }
    void slotHelpChangeLog() { showInfoDialog(HELP_CHANGELOG); }
    void slotHelpGPLLicense() { showInfoDialog(HELP_GPL); }
    void slotHelpCredits() { showInfoDialog(HELP_CREDITS); }
    void slotHelpNavdata() { showInfoDialog(HELP_NAVDATA); }

    void slotToggleAskForQuit();

    void slotXPlane();

    void slotFcuLeftOnlyModeChanged();

    void slotTimeUsed(const QString& name, uint millisecs);

    void slotRestartFMC();
    void slotRestartCDU();

protected:

    void createMenus();
    void showInfoDialog(const QString& text_file, bool html = false);
    void closeEvent(QCloseEvent* event);
    void setupDefaultConfig();
    void loadWindowGeometry();
    void saveWindowGeometry();
    void checkAndSetInputMethod();
    void restartGLDisplays();

protected:

//    bool m_is_minimized;

    Config* m_main_config;
    FMCControl* m_fmc_control;
    QSplashScreen* m_splash;
    InfoDlgImpl* m_info_dlg;

    FMCGPSHandler* m_gps_handler;
    FMCFCUHandler* m_fcu_handler;
    FMCCDUHandler* m_cdu_left_handler;
    FMCCDUHandler* m_cdu_right_handler;
    FMCNavdisplayHandler* m_navdisplay_left_handler;
    FMCPFDHandler* m_pfd_left_handler;
    FMCNavdisplayHandler* m_navdisplay_right_handler;
    FMCPFDHandler* m_pfd_right_handler;
    FMCECAMHandler* m_upper_ecam_handler;
    //TODOFMCECAMHandler* m_lower_ecam_handler;

    FMCSoundsHandler* m_fmc_sounds_handler;

    long m_logline_count;

    QAction* m_quit_action;

    QAction* m_fsaccess_msfs_action;
    QAction* m_fsaccess_xplane_action;
    QAction* m_fsaccess_fgfs_action;

    QAction* m_style_a_action;
    QAction* m_style_b_action;
    QAction* m_style_g_action;

    QStringList m_time_used_list;

private:
    //! Hidden copy-constructor
    FMCConsole(const FMCConsole&);
    //! Hidden assignment operator
    const FMCConsole& operator = (const FMCConsole&);
};

#endif /* __FMC_CONSOLE_H__ */

// End of file

