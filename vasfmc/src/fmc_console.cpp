//////////////////////////////////////////////////////////////////////////////
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
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  11-1307  USA
//
///////////////////////////////////////////////////////////////////////////////

/*! \file    fmc_console.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include <QCloseEvent>
#include <QScrollBar>
#include <QSplashScreen>
#include <QTabWidget>
#include <QDir>

#include "configwidget.h"
#include "infodlgimpl.h"

#include "defines.h"
#include "fmc_cdu.h"
#include "fmc_sounds_handler.h"
#include "fmc_control.h"
#include "opengltext.h"
#include "vas_path.h"
#ifdef USE_OPENAL
// To destroy ALContext Singleton
#include "fmc_sounds.h"
#endif

#include "fmc_console.h"

#define DO_PROFILING 0

/////////////////////////////////////////////////////////////////////////////

#define CFG_CONSOLE_POS_X "consolex"
#define CFG_CONSOLE_POS_Y "consoley"
#define CFG_CONSOLE_WIDTH "consolewidth"
#define CFG_CONSOLE_HEIGHT "consoleheight"
#define CFG_CONSOLE_WINDOW_STATUS "consolewindowstatus"
#define CFG_CONSOLE_MAX_LOGLINES "maxloglines"
#define CFG_ENABLE_CONFIG_ACCESS "enable_config_access"

/////////////////////////////////////////////////////////////////////////////

FMCConsole::FMCConsole(QWidget* parent, Qt::WFlags fl, const QString& style) :
#if !VASFMC_GAUGE
    QMainWindow(parent, fl),
#endif
    //m_is_minimized(true), 
    m_main_config(0), m_fmc_control(0), m_splash(0), m_info_dlg(0),
    m_gps_handler(0), m_fcu_handler(0), m_cdu_left_handler(0), m_cdu_right_handler(0), m_navdisplay_left_handler(0), 
    m_pfd_left_handler(0), m_navdisplay_right_handler(0), m_pfd_right_handler(0), m_logline_count(0),
    m_quit_action(0), m_fsaccess_msfs_action(0), m_fsaccess_xplane_action(0), m_fsaccess_fgfs_action(0), 
    m_style_a_action(0), m_style_b_action(0), m_style_g_action(0)
{
    Logger::log(QString("FMCConsole: current_dir=%1").arg(QDir::currentPath()));

#if !VASFMC_GAUGE
    setupUi(this);

    // setup splash

    if (QFile::exists(SPLASHSCREEN_FILE))
    {
        QPixmap pixmap(SPLASHSCREEN_FILE);
        m_splash = new QSplashScreen(pixmap);
        m_splash->showMessage("Starting up ...", Qt::AlignTop | Qt::AlignRight);
        m_splash->show();
    }
#else
    Q_UNUSED(parent);
    Q_UNUSED(fl);
#endif

    // setup main config
    Logger::log("Setup main config");
    m_main_config = new Config(CFG_MAIN_FILENAME);
    MYASSERT(m_main_config != 0);
    setupDefaultConfig();
    QDir vasfmc_dir(m_main_config->getValue(CFG_VASFMC_DIR));
    MYASSERT(vasfmc_dir.exists());
    vasfmc_dir.mkdir("cfg");
    vasfmc_dir.mkdir(m_main_config->getValue(CFG_FLIGHTPLAN_SUBDIR));
    vasfmc_dir.mkdir(m_main_config->getValue(CFG_AIRCRAFT_DATA_SUBDIR));
    vasfmc_dir.mkdir(m_main_config->getValue(CFG_CHECKLIST_SUBDIR));
    m_main_config->loadfromFile();
    loadWindowGeometry();
    if (!m_main_config->contains(CFG_STARTUP_COUNTER)) m_main_config->setValue(CFG_STARTUP_COUNTER, 1);
    else m_main_config->setValue(CFG_STARTUP_COUNTER, m_main_config->getIntValue(CFG_STARTUP_COUNTER)+1);
    m_main_config->saveToFile();
    Logger::log("Setup main config: fin");

    if (!style.isEmpty())
    {
        if (style == "G") slotStyleG();
        if (style == "B") slotStyleB();
        else              slotStyleA();
    }

    // setup console
    setWindowTitle(QString("vasFMC ")+VERSION+" - (c) "+COPYRIGHT);
    setStatusBar(0);
    show();

    MYASSERT(connect(Logger::getLogger(), SIGNAL(signalLogging(const QString&)), this, SLOT(slotLogging(const QString&))));

    if (m_main_config->getIntValue(CFG_CONSOLE_WINDOW_STATUS) == 0) showMinimized();

    MYASSERT(connect(pfd_left_btn, SIGNAL(clicked()), this, SLOT(slotPFDLeftButton())));
    MYASSERT(connect(nd_left_btn, SIGNAL(clicked()), this, SLOT(slotNDLeftButton())));
    MYASSERT(connect(cdu_left_btn, SIGNAL(clicked()), this, SLOT(slotCDULeftButton())));
    MYASSERT(connect(cdu_right_btn, SIGNAL(clicked()), this, SLOT(slotCDURightButton())));
    MYASSERT(connect(upper_ecam_btn, SIGNAL(clicked()), this, SLOT(slotUpperECAMButton())));
    MYASSERT(connect(fcu_btn, SIGNAL(clicked()), this, SLOT(slotFCUButton())));
    MYASSERT(connect(gps_btn, SIGNAL(clicked()), this, SLOT(slotGPSButton())));
    //TODO
    gps_btn->setEnabled(false);
    //TODOMYASSERT(connect(lower_ecam_btn, SIGNAL(clicked()), this, SLOT(slotLowerECAMButton())));
    MYASSERT(connect(pfd_right_btn, SIGNAL(clicked()), this, SLOT(slotPFDRightButton())));
    MYASSERT(connect(nd_right_btn, SIGNAL(clicked()), this, SLOT(slotNDRightButton())));

    // setup config widget
    config_tab->clear();
    if (m_main_config->getIntValue(CFG_ENABLE_CONFIG_ACCESS) == 0) 
    {
        config_tab->setEnabled(false);
        config_tab->hide();
        overall_tab->removeTab(1);
    }

    registerConfigWidget("Main", m_main_config);

    // setup FMC control
    m_fmc_control = new FMCControl(this, m_main_config, CFG_CONTROL_FILENAME);
    MYASSERT(m_fmc_control != 0);
    MYASSERT(connect(m_fmc_control, SIGNAL(signalSetGLFontSize(uint)), this, SLOT(slotSetGLFontSize(uint))));
    MYASSERT(connect(m_fmc_control, SIGNAL(signalStyleA()), this, SLOT(slotStyleA())));
    MYASSERT(connect(m_fmc_control, SIGNAL(signalStyleB()), this, SLOT(slotStyleB())));
    MYASSERT(connect(m_fmc_control, SIGNAL(signalStyleG()), this, SLOT(slotStyleG())));
    MYASSERT(connect(m_fmc_control, SIGNAL(signalFcuLeftOnlyModeChanged()), this, SLOT(slotFcuLeftOnlyModeChanged())));
    MYASSERT(connect(m_fmc_control, SIGNAL(signalRestartFMC()), this, SLOT(slotTriggerRestartFMC())));
    MYASSERT(connect(m_fmc_control, SIGNAL(signalRestartCDU()), this, SLOT(slotTriggerRestartCDU())));

    // setup FMC sounds
    m_fmc_sounds_handler = new FMCSoundsHandler(m_main_config, m_fmc_control);
    MYASSERT(m_fmc_sounds_handler != 0);

    // setup left FMC CDU
    m_cdu_left_handler = new FMCCDUHandler(this, m_main_config, CFG_CDU_LEFT_FILENAME, m_fmc_control, true);
    MYASSERT(m_cdu_left_handler != 0);

    // setup right FMC CDU
    m_cdu_right_handler = new FMCCDUHandler(this, m_main_config, CFG_CDU_RIGHT_FILENAME, m_fmc_control, false);
    MYASSERT(m_cdu_right_handler != 0);

    // create menus
    createMenus();

    // connect profiling stuff

#if DO_PROFILING

    MYASSERT(connect(m_pfd_left_handler->fmcPFD(), SIGNAL(signalTimeUsed(const QString&, uint)),
                     this, SLOT(slotTimeUsed(const QString&, uint))));
    MYASSERT(connect(m_navdisplay_left_handler->fmcNavdisplay(), SIGNAL(signalTimeUsed(const QString&, uint)),
                     this, SLOT(slotTimeUsed(const QString&, uint))));
    MYASSERT(connect(m_upper_ecam_handler->fmcECAM(), SIGNAL(signalTimeUsed(const QString&, uint)),
                     this, SLOT(slotTimeUsed(const QString&, uint))));
    MYASSERT(connect(m_cdu_left_handler->fmcCduBase(), SIGNAL(signalTimeUsed(const QString&, uint)),
                     this, SLOT(slotTimeUsed(const QString&, uint))));
    MYASSERT(connect(m_fcu_handler->fmcFcuBase(), SIGNAL(signalTimeUsed(const QString&, uint)),
                     this, SLOT(slotTimeUsed(const QString&, uint))));
    MYASSERT(connect(m_fmc_control->fmcProcessor(), SIGNAL(signalTimeUsed(const QString&, uint)),
                     this, SLOT(slotTimeUsed(const QString&, uint))));
    MYASSERT(connect(m_fmc_control, SIGNAL(signalTimeUsed(const QString&, uint)),
                     this, SLOT(slotTimeUsed(const QString&, uint))));

#endif
}

/////////////////////////////////////////////////////////////////////////////

FMCConsole::~FMCConsole()
{
    for(int index=0; index < m_time_used_list.count(); ++index) Logger::log(m_time_used_list[index]);

    delete m_navdisplay_left_handler;
    delete m_upper_ecam_handler;
    delete m_pfd_left_handler;
    delete m_cdu_left_handler;
    delete m_cdu_right_handler;
    delete m_fmc_control;
    delete m_fcu_handler;
    delete m_gps_handler;
    delete m_navdisplay_right_handler;
    delete m_pfd_right_handler;
    delete m_fmc_sounds_handler;

#ifdef USE_OPENAL
    AlContext::destroyContext();
#endif

    saveWindowGeometry();
    m_main_config->saveToFile();
    delete m_main_config;
}

/////////////////////////////////////////////////////////////////////////////

void FMCConsole::restartGLDisplays()
{
    Logger::log("FMCConsole:restartGLDisplays");
    //TODOm_lower_ecam_handler->restartECAM();
}

/////////////////////////////////////////////////////////////////////////////

void FMCConsole::slotRestartCDU()
{
    if (m_cdu_left_handler != 0) m_cdu_left_handler->slotRestartCdu();    
#if !VASFMC_GAUGE    
    if (m_cdu_right_handler != 0) m_cdu_right_handler->slotRestartCdu();
#endif
}

/////////////////////////////////////////////////////////////////////////////

void FMCConsole::slotRestartFMC()
{
#if VASFMC_GAUGE    
    Logger::log("FMCConsole:restartFMC: inhibited in gauge");
    return;
#endif

    Logger::log("FMCConsole:restartFMC");
    m_fmc_sounds_handler->restartSounds();
    m_cdu_left_handler->slotRestartCdu();
#if !VASFMC_GAUGE    
    m_cdu_right_handler->slotRestartCdu();
#endif
    m_fmc_control->setupFlightStatusChecker();
}

/////////////////////////////////////////////////////////////////////////////

void FMCConsole::slotSetGLFontSize(uint index)
{
    Logger::log("FMCConsole:slotSetGLFontSize");

    index = LIMITMINMAX((int)index, 1, 9);

    int old_size = m_main_config->getIntValue(CFG_ACTIVE_FONT_SIZE);
    int new_size = 0;
    switch(index)
    {
        case(1): new_size = m_main_config->getIntValue(CFG_FONT_SIZE1); break;
        case(2): new_size = m_main_config->getIntValue(CFG_FONT_SIZE2); break;
        case(3): new_size = m_main_config->getIntValue(CFG_FONT_SIZE3); break;
        case(4): new_size = m_main_config->getIntValue(CFG_FONT_SIZE4); break;
        case(5): new_size = m_main_config->getIntValue(CFG_FONT_SIZE5); break;
        case(6): new_size = m_main_config->getIntValue(CFG_FONT_SIZE6); break;
        case(7): new_size = m_main_config->getIntValue(CFG_FONT_SIZE7); break;
        case(8): new_size = m_main_config->getIntValue(CFG_FONT_SIZE8); break;
        case(9): new_size = m_main_config->getIntValue(CFG_FONT_SIZE9); break;
    }
    
    new_size = LIMITMINMAX(new_size, 
                           m_main_config->getIntValue(CFG_FONT_SIZE1), 
                           m_main_config->getIntValue(CFG_FONT_SIZE9));

    if (old_size == new_size) return;
    m_main_config->setValue(CFG_ACTIVE_FONT_SIZE, new_size);
    m_main_config->setValue(CFG_ACTIVE_FONT_INDEX, index);
    restartGLDisplays();
}

/////////////////////////////////////////////////////////////////////////////

void FMCConsole::createMenus()
{
#if !VASFMC_GAUGE
    // Make menuBar() as in trolltech's official examples
    // QMenuBar *menubar = menuBar();
    // MYASSERT(menubar != 0);

    //-----

    QMenu* options_menu = menuBar()->addMenu("&Options");

    m_quit_action = options_menu->addAction("&Ask for quit", this, SLOT(slotToggleAskForQuit()));
    m_quit_action->setCheckable(true);
    m_quit_action->setChecked(m_main_config->getIntValue(CFG_ASK_FOR_QUIT) != 0);

    //-----

    QMenu* fs_menu = menuBar()->addMenu("&Flightsim");

    m_fsaccess_xplane_action = fs_menu->addAction("&X-Plane", this, SLOT(slotXPlane()));
    m_fsaccess_xplane_action->setCheckable(true);
    m_fsaccess_xplane_action->setChecked(m_main_config->getValue(CFG_FS_ACCESS_TYPE) == FS_ACCESS_TYPE_XPLANE);

    //-----

    QMenu* style_menu = menuBar()->addMenu("&Style");
    m_style_a_action = style_menu->addAction("&A", this, SLOT(slotStyleA()));
    m_style_a_action->setCheckable(true);
    m_style_a_action->setChecked(m_main_config->getIntValue(CFG_STYLE) == CFG_STYLE_A);
    m_style_b_action = style_menu->addAction("&B", this, SLOT(slotStyleB()));
    m_style_b_action->setCheckable(true);
    m_style_b_action->setChecked(m_main_config->getIntValue(CFG_STYLE) == CFG_STYLE_B);
//TODO
//     m_style_g_action = style_menu->addAction("&G", this, SLOT(slotStyleG()));
//     m_style_g_action->setCheckable(true);
//     m_style_g_action->setChecked(m_main_config->getIntValue(CFG_STYLE) == CFG_STYLE_G);

    //-----

    QMenu *helpmenu = menuBar()->addMenu("&Help");
    helpmenu->addAction("&Keys", this, SLOT(slotHelpKeys()));
    helpmenu->addAction("&Navdata", this, SLOT(slotHelpNavdata()));
    helpmenu->addAction("&FSUIPC Offsets", this, SLOT(slotHelpFSUIPCOffsets()));
    helpmenu->addSeparator();
    helpmenu->addAction("&Changelog", this, SLOT(slotHelpChangeLog()));
    helpmenu->addAction("&GPL License", this, SLOT(slotHelpGPLLicense()));
    helpmenu->addAction("&Credits", this, SLOT(slotHelpCredits()));
#endif
}

/////////////////////////////////////////////////////////////////////////////

void FMCConsole::slotToggleAskForQuit()
{
    m_main_config->setValue(CFG_ASK_FOR_QUIT, m_main_config->getIntValue(CFG_ASK_FOR_QUIT) == 0);
    m_quit_action->setChecked(m_main_config->getIntValue(CFG_ASK_FOR_QUIT) != 0);
}

/////////////////////////////////////////////////////////////////////////////

void FMCConsole::checkAndSetInputMethod()
{
    if (m_fsaccess_xplane_action != 0)
        m_fsaccess_xplane_action->setChecked(m_main_config->getValue(CFG_FS_ACCESS_TYPE) == FS_ACCESS_TYPE_XPLANE);
}

/////////////////////////////////////////////////////////////////////////////


void FMCConsole::slotXPlane()
{
    m_fmc_control->switchToXPlane();
    checkAndSetInputMethod();
}

/////////////////////////////////////////////////////////////////////////////

void FMCConsole::showInfoDialog(const QString& text_file, bool html)
{
#if !VASFMC_GAUGE
    MYASSERT(!text_file.isEmpty());

    delete m_info_dlg;
    m_info_dlg = new InfoDlgImpl(this, Qt::Dialog|Qt::WindowTitleHint|Qt::WindowSystemMenuHint);
    MYASSERT(m_info_dlg != 0);

    bool success = false;

    if (html)
    {
        if (m_info_dlg->loadHTMLFromFile(text_file))
        {
            m_info_dlg->setWindowTitle("vasFMC Info");
            m_info_dlg->show();
            success = true;
        }
    }
    else
    {
        if (m_info_dlg->loadTextFromFile(text_file))
        {
            m_info_dlg->setWindowTitle("vasFMC Info");
            m_info_dlg->show();
            success = true;
        }
    }

    if (!success)
    {
        delete m_info_dlg;
        m_info_dlg = 0;
        QMessageBox::warning(this, "Warning", QString("Could not load text from file (%1)").arg(text_file));
    }
#else
    Q_UNUSED(text_file);
    Q_UNUSED(html);
#endif
}

/////////////////////////////////////////////////////////////////////////////

void FMCConsole::registerConfigWidget(const QString& title, Config* cfg)
{
#if !VASFMC_GAUGE
    if (m_main_config->getIntValue(CFG_ENABLE_CONFIG_ACCESS) == 0) return;
    MYASSERT(!title.isEmpty());
    MYASSERT(cfg != 0);
    config_tab->addTab(new ConfigWidget(cfg), title);
#else
    Q_UNUSED(title);
    Q_UNUSED(cfg);
#endif
}

/////////////////////////////////////////////////////////////////////////////

void FMCConsole::unregisterConfigWidget(const QString& title)
{
#if VASFMC_GAUGE
    Q_UNUSED(title);
#else

    if (m_main_config->getIntValue(CFG_ENABLE_CONFIG_ACCESS) == 0) return;
    MYASSERT(!title.isEmpty());
    for(int index=0; index < config_tab->count(); ++index)
    {
        if (config_tab->tabText(index) == title)
        {
            config_tab->removeTab(index);
            break;
        }
    }
#endif
}

/////////////////////////////////////////////////////////////////////////////

void FMCConsole::slotDeleteSplashscreen()
{
    delete m_splash;
    m_splash = 0;
}

/////////////////////////////////////////////////////////////////////////////

void FMCConsole::closeEvent(QCloseEvent* event )
{
#if VASFMC_GAUGE

    Q_UNUSED(event);

#else

    bool quit = true;

    if (m_main_config->getIntValue(CFG_ASK_FOR_QUIT) != 0)
    {
        switch(QMessageBox::question(this, "Quit", "Do you really want to quit?", QMessageBox::Yes, QMessageBox::No))
        {
            case (QMessageBox::Yes): {
                quit = true;
                break;
            }

            default: {
                quit = false;
                event->ignore();
                break;
            }
        }
    }

    if (quit)
    {
        m_cdu_left_handler->close();
        m_cdu_right_handler->close();
    }

#endif
}

/////////////////////////////////////////////////////////////////////////////

void FMCConsole::setupDefaultConfig()
{
    MYASSERT(m_main_config != 0);

    saveWindowGeometry();

#if VASFMC_GAUGE
    m_main_config->setValue(CFG_VASFMC_DIR, VasPath::getPath());
#else
    m_main_config->setValue(CFG_VASFMC_DIR, QDir::current().absolutePath());
#endif

    m_main_config->setValue(CFG_STYLE, CFG_STYLE_A);
    m_main_config->setValue(CFG_CONSOLE_MAX_LOGLINES, "500");
    m_main_config->setValue(CFG_PERSISTANCE_FILE, "persistence.dat");
    m_main_config->setValue(CFG_ENABLE_CONFIG_ACCESS, 0);
    m_main_config->setValue(CFG_BEST_ANTI_ALIASING, 1);
    m_main_config->setValue(CFG_FONT_NAME, "fmc.fnt");
    m_main_config->setValue(CFG_ACTIVE_FONT_SIZE, 15);
    m_main_config->setValue(CFG_ACTIVE_FONT_INDEX, 4);
    m_main_config->setValue(CFG_FONT_SIZE1, 12);
    m_main_config->setValue(CFG_FONT_SIZE2, 13);
    m_main_config->setValue(CFG_FONT_SIZE3, 14);
    m_main_config->setValue(CFG_FONT_SIZE4, 15);
    m_main_config->setValue(CFG_FONT_SIZE5, 16);
    m_main_config->setValue(CFG_FONT_SIZE6, 18);
    m_main_config->setValue(CFG_FONT_SIZE7, 20);
    m_main_config->setValue(CFG_FONT_SIZE8, 24);
    m_main_config->setValue(CFG_FONT_SIZE9, 32);
    m_main_config->setValue(CFG_ASK_FOR_QUIT, 0);
    m_main_config->setValue(CFG_FLIGHTPLAN_SUBDIR, "fps");
    m_main_config->setValue(CFG_AIRCRAFT_DATA_SUBDIR, "aircraft_data");
    m_main_config->setValue(CFG_CHECKLIST_SUBDIR, "checklists");
    m_main_config->setValue(CFG_GEODATA_FILE, "gshhs/gshhs_l.b");
    m_main_config->setValue(CFG_GEODATA_FILTER_LEVEL, 1);
    m_main_config->setValue(CFG_DECLINATION_DATAFILE, "WMM2005.cof");
    m_main_config->setValue(CFG_FLIGHTSTATUS_SMOOTHING_DELAY_MS, 150);
//Make sure, that only in WIN32-Environments the default FS_ACCESS_TYPE is MSFS
#ifdef Q_OS_WIN32
    m_main_config->setValue(CFG_FS_ACCESS_TYPE, FS_ACCESS_TYPE_MSFS);
#else
    m_main_config->setValue(CFG_FS_ACCESS_TYPE, FS_ACCESS_TYPE_XPLANE);
#endif
}

/////////////////////////////////////////////////////////////////////////////

void FMCConsole::loadWindowGeometry()
{
#if !VASFMC_GAUGE
    // setup window geometry

    if (m_main_config->contains(CFG_CONSOLE_POS_X) && m_main_config->contains(CFG_CONSOLE_POS_Y) &&
        m_main_config->contains(CFG_CONSOLE_WIDTH) && m_main_config->contains(CFG_CONSOLE_HEIGHT))
    {
//         Logger::log(QString("FMCConsole:loadWindowGeometry: resizing to %1/%2").
//                     arg(m_main_config->getIntValue(CFG_CONSOLE_WIDTH)).
//                     arg(m_main_config->getIntValue(CFG_CONSOLE_HEIGHT)));

        resize(m_main_config->getIntValue(CFG_CONSOLE_WIDTH), m_main_config->getIntValue(CFG_CONSOLE_HEIGHT));
        move(m_main_config->getIntValue(CFG_CONSOLE_POS_X), m_main_config->getIntValue(CFG_CONSOLE_POS_Y));
    }
#endif
}

/////////////////////////////////////////////////////////////////////////////

void FMCConsole::saveWindowGeometry()
{
#if !VASFMC_GAUGE
    // save window geometry
    m_main_config->setValue(CFG_CONSOLE_POS_X, QString().setNum(x()));
    m_main_config->setValue(CFG_CONSOLE_POS_Y, QString().setNum(y()));

    m_main_config->setValue(CFG_CONSOLE_WIDTH, QString().setNum(geometry().width()));
    m_main_config->setValue(CFG_CONSOLE_HEIGHT, QString().setNum(geometry().height()));
    m_main_config->setValue(CFG_CONSOLE_WINDOW_STATUS, (isMinimized() ? 0 : 1));
#endif
}

/////////////////////////////////////////////////////////////////////////////

void FMCConsole::slotNDLeftButton()
{
}

/////////////////////////////////////////////////////////////////////////////

void FMCConsole::slotPFDLeftButton()
{
}

/////////////////////////////////////////////////////////////////////////////

#if !VASFMC_GAUGE
void FMCConsole::slotNDRightButton()
{
}

/////////////////////////////////////////////////////////////////////////////

void FMCConsole::slotPFDRightButton()
{
}

/////////////////////////////////////////////////////////////////////////////

void FMCConsole::slotCDURightButton()
{
    m_cdu_right_handler->isVisible() ? m_cdu_right_handler->hide() : m_cdu_right_handler->show();    
}
#endif

/////////////////////////////////////////////////////////////////////////////

void FMCConsole::slotCDULeftButton()
{
    m_cdu_left_handler->isVisible() ? m_cdu_left_handler->hide() : m_cdu_left_handler->show();    
}

/////////////////////////////////////////////////////////////////////////////

void FMCConsole::slotFCUButton()
{
}

/////////////////////////////////////////////////////////////////////////////

void FMCConsole::slotGPSButton()
{
}

/////////////////////////////////////////////////////////////////////////////

void FMCConsole::slotUpperECAMButton()
{
}

/////////////////////////////////////////////////////////////////////////////

void FMCConsole::slotStyleA()
{
    Logger::log("FMCConsole:slotStyleA");

    m_main_config->setValue(CFG_STYLE, CFG_STYLE_A);
#if !VASFMC_GAUGE
    m_style_a_action->setChecked(m_main_config->getIntValue(CFG_STYLE) == CFG_STYLE_A);
    m_style_b_action->setChecked(m_main_config->getIntValue(CFG_STYLE) == CFG_STYLE_B);
    //TODOm_style_g_action->setChecked(m_main_config->getIntValue(CFG_STYLE) == CFG_STYLE_G);
    slotTriggerRestartFMC();
#endif
}

/////////////////////////////////////////////////////////////////////////////

void FMCConsole::slotStyleB()
{
    Logger::log("FMCConsole:slotStyleB");

    m_main_config->setValue(CFG_STYLE, CFG_STYLE_B);
#if !VASFMC_GAUGE
    m_style_a_action->setChecked(m_main_config->getIntValue(CFG_STYLE) == CFG_STYLE_A);
    m_style_b_action->setChecked(m_main_config->getIntValue(CFG_STYLE) == CFG_STYLE_B);
    //TODOm_style_g_action->setChecked(m_main_config->getIntValue(CFG_STYLE) == CFG_STYLE_G);
    slotTriggerRestartFMC();
#endif
}

/////////////////////////////////////////////////////////////////////////////

void FMCConsole::slotStyleG()
{
    Logger::log("FMCConsole:slotStyleG");

    m_main_config->setValue(CFG_STYLE, CFG_STYLE_G);
#if !VASFMC_GAUGE
    m_style_a_action->setChecked(m_main_config->getIntValue(CFG_STYLE) == CFG_STYLE_A);
    m_style_b_action->setChecked(m_main_config->getIntValue(CFG_STYLE) == CFG_STYLE_B);
    //TODOm_style_g_action->setChecked(m_main_config->getIntValue(CFG_STYLE) == CFG_STYLE_G);
    slotTriggerRestartFMC();
#endif
}

/////////////////////////////////////////////////////////////////////////////

void FMCConsole::slotFcuLeftOnlyModeChanged()
{
    Logger::log("FMCConsole:slotFcuLeftOnlyModeChanged");
}

/////////////////////////////////////////////////////////////////////////////

//TODO
// void FMCConsole::slotLowerECAMButton()
// {
//     m_lower_ecam_handler->isVisible() ? m_lower_ecam_handler->hide() : m_lower_ecam_handler->show();
// }

/////////////////////////////////////////////////////////////////////////////

void FMCConsole::slotLogging(const QString& text)
{
#if !VASFMC_GAUGE
    logtext->append(text);
    ++m_logline_count;

    if (m_logline_count >= m_main_config->getIntValue(CFG_CONSOLE_MAX_LOGLINES))
    {
        logtext->setPlainText(
            logtext->toPlainText().section(
                '\n', m_main_config->getIntValue(CFG_CONSOLE_MAX_LOGLINES)/2));

        m_logline_count /= 2;
    }

    logtext->verticalScrollBar()->setValue(logtext->verticalScrollBar()->maximum());
#else
    Q_UNUSED(text);
#endif
}

/////////////////////////////////////////////////////////////////////////////

void FMCConsole::slotTimeUsed(const QString& name, uint millisecs)
{
    if (millisecs == 0) return;
    m_time_used_list.append(QString("%1: %2: %3").
                            arg(QDateTime::currentDateTime().time().toString("hh:mm:ss:zzz")).arg(name).arg(millisecs));
}

// End of file
