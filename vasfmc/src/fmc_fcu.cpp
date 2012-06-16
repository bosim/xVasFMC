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

/*! \file    fmc_fcu.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include <QMouseEvent>
#include <QShortcut>
#include <QKeyEvent>

#include "assert.h"
#include "logger.h"

#include "defines.h"

#include "fmc_autopilot.h"
#include "fmc_autothrottle.h"

#include "fmc_control.h"
#include "fmc_fcu.h"
#include "fmc_fcu_style_a.h"

/////////////////////////////////////////////////////////////////////////////

FMCFCUStyleBase::FMCFCUStyleBase(const QString& style,
                                 ConfigWidgetProvider* config_widget_provider,
                                 Config* main_config,
                                 const QString& fcu_config_filename,
                                 FMCControl* fmc_control,
                                 QWidget* parent,
                                 Qt::WFlags fl) :
    VasWidget(parent, fl),
    m_config_widget_provider(config_widget_provider), 
    m_main_config(main_config), m_fmc_control(fmc_control)
{
    MYASSERT(m_config_widget_provider != 0);
    MYASSERT(m_main_config != 0);
    MYASSERT(m_fmc_control != 0);

    // setup fcu config

    m_fcu_config = new Config(fcu_config_filename);
    MYASSERT(m_fcu_config != 0);
    setupDefaultConfig(style);
    m_fcu_config->loadfromFile();
    loadWindowGeometry();
    m_fcu_config->saveToFile();
    m_config_widget_provider->registerConfigWidget("FCU", m_fcu_config);
}

/////////////////////////////////////////////////////////////////////////////

FMCFCUStyleBase::~FMCFCUStyleBase()
{
    m_config_widget_provider->unregisterConfigWidget("FCU");
    saveWindowGeometry();
    m_fcu_config->saveToFile();
    delete m_fcu_config;
}

/////////////////////////////////////////////////////////////////////////////

void FMCFCUStyleBase::setupDefaultConfig(const QString& style)
{
    saveWindowGeometry();    

    m_fcu_config->setValue(CFG_FCU_POS_X, 0);
    m_fcu_config->setValue(CFG_FCU_POS_Y, 22);

    m_fcu_config->setValue(CFG_FCU_REFRESH_PERIOD_MS, 500);
    m_fcu_config->setValue(CFG_FCU_LCD_FONTNAME, "LCD"); //TXPDR
    m_fcu_config->setValue(CFG_FCU_LCD_FONTSIZE, 26);
    m_fcu_config->setValue(CFG_FCU_NORMAL_FONTNAME, "ARIAL");
    m_fcu_config->setValue(CFG_FCU_NORMAL_FONTSIZE, 10);

    if (style == "A")
    {
        // override default values for A style
        m_fcu_config->setValue(CFG_FCU_INPUT_AREA_FILE, "cfg/fcu_input_area_style_a.cfg");
        m_fcu_config->setValue(CFG_FCU_BACKGROUND_IMAGE_GAUGE, "graphics/fcu/fcu.mitchell.png");
        m_fcu_config->setValue(CFG_FCU_BACKGROUND_IMAGE, "graphics/fcu/fcu.mitchell.full.png");

        m_fcu_config->setValue(CFG_FCU_RANGE10_IMAGE, "graphics/fcu/fcu.mitchell.range.knob.10.png");
        m_fcu_config->setValue(CFG_FCU_RANGE20_IMAGE, "graphics/fcu/fcu.mitchell.range.knob.20.png");
        m_fcu_config->setValue(CFG_FCU_RANGE40_IMAGE, "graphics/fcu/fcu.mitchell.range.knob.40.png");
        m_fcu_config->setValue(CFG_FCU_RANGE80_IMAGE, "graphics/fcu/fcu.mitchell.range.knob.80.png");
        m_fcu_config->setValue(CFG_FCU_RANGE160_IMAGE, "graphics/fcu/fcu.mitchell.range.knob.160.png");
        m_fcu_config->setValue(CFG_FCU_RANGE320_IMAGE, "graphics/fcu/fcu.mitchell.range.knob.320.png");

        m_fcu_config->setValue(CFG_FCU_MODE_ILS_IMAGE, "graphics/fcu/fcu.mitchell.mode.knob.ils.png");
        m_fcu_config->setValue(CFG_FCU_MODE_VOR_IMAGE, "graphics/fcu/fcu.mitchell.mode.knob.vor.png");
        m_fcu_config->setValue(CFG_FCU_MODE_ROSE_IMAGE, "graphics/fcu/fcu.mitchell.mode.knob.rose.png");
        m_fcu_config->setValue(CFG_FCU_MODE_ARC_IMAGE, "graphics/fcu/fcu.mitchell.mode.knob.arc.png");
        m_fcu_config->setValue(CFG_FCU_MODE_PLAN_IMAGE, "graphics/fcu/fcu.mitchell.mode.knob.plan.png");
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCFCUStyleBase::incFontSize()
{
    m_fcu_config->setValue(CFG_FCU_LCD_FONTSIZE, m_fcu_config->getIntValue(CFG_FCU_LCD_FONTSIZE)+1);
    m_fcu_config->setValue(CFG_FCU_NORMAL_FONTSIZE, m_fcu_config->getIntValue(CFG_FCU_NORMAL_FONTSIZE)+1);
}

/////////////////////////////////////////////////////////////////////////////

void FMCFCUStyleBase::decFontSize()
{
    m_fcu_config->setValue(CFG_FCU_LCD_FONTSIZE, qMax(5, m_fcu_config->getIntValue(CFG_FCU_LCD_FONTSIZE)-1));
    m_fcu_config->setValue(CFG_FCU_NORMAL_FONTSIZE, m_fcu_config->getIntValue(CFG_FCU_NORMAL_FONTSIZE)-1);
}

/////////////////////////////////////////////////////////////////////////////

void FMCFCUStyleBase::loadWindowGeometry()
{
    // setup window geometry

    if (m_fcu_config->contains(CFG_FCU_POS_X) && m_fcu_config->contains(CFG_FCU_POS_Y) &&
        m_fcu_config->contains(CFG_FCU_WIDTH) && m_fcu_config->contains(CFG_FCU_HEIGHT))
    {
//         Logger::log(QString("FMCFCU:loadWindowGeometry: resizing to %1/%2").
//                     arg(m_fcu_config->getIntValue(CFG_FCU_WIDTH)).
//                     arg(m_fcu_config->getIntValue(CFG_FCU_HEIGHT)));

        resize(m_fcu_config->getIntValue(CFG_FCU_WIDTH), m_fcu_config->getIntValue(CFG_FCU_HEIGHT));
        move(m_fcu_config->getIntValue(CFG_FCU_POS_X), m_fcu_config->getIntValue(CFG_FCU_POS_Y));
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCFCUStyleBase::saveWindowGeometry()
{
#if !VASFMC_GAUGE
    // save window geometry

    m_fcu_config->setValue(CFG_FCU_POS_X, QString().setNum(x()));
    m_fcu_config->setValue(CFG_FCU_POS_Y, QString().setNum(y()));

    m_fcu_config->setValue(CFG_FCU_WIDTH, QString().setNum(geometry().width()));
    m_fcu_config->setValue(CFG_FCU_HEIGHT, QString().setNum(geometry().height()));

    m_fcu_config->setValue(CFG_FCU_WINDOW_STATUS, (m_is_visible ? 1 : 0));
#endif
}

/////////////////////////////////////////////////////////////////////////////

void FMCFCUStyleBase::slotProcessInput()
{
    while(!m_fmc_control->flightStatus()->fsctrl_fcu_list.isEmpty())
    {
        QKeyEvent* key_event = 0;

        switch(m_fmc_control->flightStatus()->fsctrl_fcu_list.first())
        {
            case(1):  m_fmc_control->fmcAutoPilot().setNAVHold(); break;

            case(3):  m_fmc_control->fmcAutoPilot().setHeadingHold(true); break;
            case(4):  m_fmc_control->fmcAutoPilot().setHeadingHold(false); break;

            case(7):  m_fmc_control->fmcAutoPilot().setLOCHold(); break;

            case(9):  m_fmc_control->fmcAutoPilot().setAPPHold(); break;

            case(11): m_fmc_control->fmcAutoPilot().setALTHold(true); break;
            case(12): m_fmc_control->fmcAutoPilot().setALTHold(false); break;


            case(15): m_fmc_control->fmcAutoPilot().setVSFPAHold(true); break;
            case(16): m_fmc_control->fmcAutoPilot().setVSFPAHold(false); break;

            case(20): m_fmc_control->fsAccess().setFDOnOff(true); break;
            case(21): m_fmc_control->fsAccess().setFDOnOff(false); break;
            case(22): m_fmc_control->fmcAutoPilot().slotToggleFlightDirector(); break;
            case(23): m_fmc_control->fsAccess().setAPOnOff(true); break;
            case(24): m_fmc_control->fsAccess().setAPOnOff(false); break;
            case(25): m_fmc_control->fmcAutoPilot().slotToggleAutopilot(); break;
            case(26): m_fmc_control->fmcAutothrottle().armAPThrottle(); break;
            case(27): m_fmc_control->fmcAutothrottle().engageAPThrottle(); break;
            case(28): m_fmc_control->fmcAutothrottle().slotAutothrottleEngageSpeed(); break;
            case(29): m_fmc_control->fmcAutothrottle().slotAutothrottleEngageMach(); break;
            case(30): m_fmc_control->fmcAutothrottle().engageAPThrottleN1Hold(); break;
            case(31): m_fmc_control->fmcAutothrottle().slotToggleAutothrottleSpeedMach(); break;
            case(32): m_fmc_control->fmcAutothrottle().setAPThrottleModeSpeed(); break;
            case(33): m_fmc_control->fmcAutothrottle().setAPThrottleModeMach(); break;
            case(34): m_fmc_control->fmcAutothrottle().disengageAPThrottle(); break;
            case(35): m_fmc_control->fmcAutothrottle().slotToggleAutothrottleArm(); break;

            case(40): m_fmc_control->slotSetAltimeterIsSetToSTD(true, true); break;
            case(41): m_fmc_control->slotSetAltimeterIsSetToSTD(true, false); break;
            case(42): m_fmc_control->slotSetAltimeterIsSetToSTD(false, true); break;
            case(43): m_fmc_control->slotSetAltimeterIsSetToSTD(false, false); break;
                
            case(50): m_fmc_control->fmcAutoPilot().enableFlightPathMode(true); break;
            case(51): m_fmc_control->fmcAutoPilot().enableFlightPathMode(false); break;
            case(52): m_fmc_control->fmcAutoPilot().slotToggleFlightPathMode(); break;

            case(60): m_fmc_control->fmcAutoPilot().setFLChangeMode(true, false); break;
            case(61): m_fmc_control->fmcAutoPilot().setFLChangeMode(false, false); break;
            case(62): m_fmc_control->fmcAutoPilot().setFLChangeMode(
                !m_fmc_control->fmcAutoPilot().isFLChangeModeActive(), false);
                break;

#if !VASFMC_GAUGE
            case(70): isVisible() ? hide() : show(); break;
            case(71): m_fmc_control->toggleKeepOnTop(); break;
            case(72): activateWindow(); break;
#endif
        }
        
        if (key_event != 0)
        {
            keyPressEvent(key_event);
            delete key_event;
        }
    
        m_fmc_control->flightStatus()->fsctrl_fcu_list.removeFirst();
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

FMCFCUHandler::FMCFCUHandler(ConfigWidgetProvider* config_widget_provider,
                             Config* main_config,
                             const QString& fcu_config_filename,
                             FMCControl* fmc_control) :
    m_config_widget_provider(config_widget_provider),
    m_main_config(main_config), m_fcu_config_filename(fcu_config_filename), m_fmc_control(fmc_control), m_fcu(0)
{
    MYASSERT(m_config_widget_provider != 0);
    MYASSERT(m_main_config != 0);
    MYASSERT(m_fmc_control != 0);
    slotRestartFCU();
    fmc_control->setFCUHandler(this);
}

/////////////////////////////////////////////////////////////////////////////

FMCFCUStyleBase* FMCFCUHandler::createFcu()
{
    switch(m_main_config->getIntValue(CFG_STYLE))
    {
        case(CFG_STYLE_A):
        case(CFG_STYLE_B): {

#if !VASFMC_GAUGE
            if (m_fmc_control->doKeepOnTop())
                return new FMCFCUStyleA(m_config_widget_provider, m_main_config, m_fcu_config_filename, 
                                        m_fmc_control, 0, Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
            else
#endif
                return new FMCFCUStyleA(m_config_widget_provider, m_main_config, m_fcu_config_filename, m_fmc_control, 0, 0);
        }
    }

    return 0;
}

// End of file
