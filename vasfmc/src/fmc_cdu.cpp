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

/*! \file    fmc_cdu.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include <QMouseEvent>
#include <QShortcut>
#include <QKeyEvent>

#include "assert.h"
#include "logger.h"

#include "fmc_control.h"
#include "fmc_cdu.h"
#include "fmc_cdu_style_a.h"

/////////////////////////////////////////////////////////////////////////////

FMCCDUStyleBase::FMCCDUStyleBase(const QString& style,
                                 ConfigWidgetProvider* config_widget_provider,
                                 Config* main_config,
                                 const QString& cdu_config_filename,
                                 FMCControl* fmc_control,
                                 QWidget* parent,
                                 Qt::WFlags fl,
                                 bool left_side) :
    VasWidget(parent, fl),
    m_config_widget_provider(config_widget_provider), 
    m_main_config(main_config), m_fmc_control(fmc_control), m_showKbdActiveIndicator(false),
    m_left_side(left_side)
{
    MYASSERT(m_config_widget_provider != 0);
    MYASSERT(m_main_config != 0);
    MYASSERT(m_fmc_control != 0);

    // setup cdu config

    m_cdu_config = new Config(cdu_config_filename);
    MYASSERT(m_cdu_config != 0);
    setupDefaultConfig(style);
    m_cdu_config->loadfromFile();
    loadWindowGeometry();
    m_cdu_config->saveToFile();
    m_config_widget_provider->registerConfigWidget("CDU", m_cdu_config);
}

/////////////////////////////////////////////////////////////////////////////

FMCCDUStyleBase::~FMCCDUStyleBase()
{
    m_config_widget_provider->unregisterConfigWidget("CDU");
    saveWindowGeometry();
    m_cdu_config->saveToFile();
    delete m_cdu_config;
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUStyleBase::slotShowKbdActiveIndicator(bool show)
{
    m_showKbdActiveIndicator=show;
    update();
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUStyleBase::setupDefaultConfig(const QString& style)
{
    saveWindowGeometry();    

    if ( m_left_side ) {
        m_cdu_config->setValue(CFG_CDU_POS_X, 0);
    } else {
        m_cdu_config->setValue(CFG_CDU_POS_X, 1000);
    }
    m_cdu_config->setValue(CFG_CDU_POS_Y, 210);

    m_cdu_config->setValue(CFG_CDU_REFRESH_PERIOD_MS, 500);
    m_cdu_config->setValue(CFG_CDU_BACKGROUND_IMAGE, "graphics/mcdu/mcdu.size2.jpg");
    m_cdu_config->setValue(CFG_CDU_FONTNAME, "Lucida Sans Typewriter");
    m_cdu_config->setValue(CFG_CDU_FONTSIZE, 18);
    m_cdu_config->setValue(CFG_CDU_DISPLAY_ONLY_MODE_FONTNAME, "Lucida Sans Typewriter");
    m_cdu_config->setValue(CFG_CDU_DISPLAY_ONLY_MODE_FONTSIZE, 18);
    m_cdu_config->setValue(CFG_SCROLL_MODE, SCROLL_MODE_REGULAR);
    m_cdu_config->setValue(CFG_CDU_INPUT_AREA_FILE, "cfg/cdu_input_area.cfg");

    if (style == "A")
    {
        m_cdu_config->setValue(CFG_CDU_INPUT_AREA_FILE, "cfg/cdu_input_area_style_a.cfg");
        m_cdu_config->setValue(CFG_CDU_DISPLAY_X, 49);
        m_cdu_config->setValue(CFG_CDU_DISPLAY_Y, 44);
        m_cdu_config->setValue(CFG_CDU_DISPLAY_WIDTH, 272);
        m_cdu_config->setValue(CFG_CDU_DISPLAY_HEIGHT, 200);
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUStyleBase::incFontSize()
{
    if (m_fmc_control->cduDisplayOnlyMode())
        m_cdu_config->setValue(CFG_CDU_DISPLAY_ONLY_MODE_FONTSIZE, 
                               m_cdu_config->getIntValue(CFG_CDU_DISPLAY_ONLY_MODE_FONTSIZE)+1);
    else
        m_cdu_config->setValue(CFG_CDU_FONTSIZE, m_cdu_config->getIntValue(CFG_CDU_FONTSIZE)+1);

    //TODO needed? emit signalRestart();
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUStyleBase::decFontSize()
{
    if (m_fmc_control->cduDisplayOnlyMode())
        m_cdu_config->setValue(CFG_CDU_DISPLAY_ONLY_MODE_FONTSIZE, 
                               qMax(5, m_cdu_config->getIntValue(CFG_CDU_DISPLAY_ONLY_MODE_FONTSIZE)-1));
    else
        m_cdu_config->setValue(CFG_CDU_FONTSIZE, qMax(5, m_cdu_config->getIntValue(CFG_CDU_FONTSIZE)-1));
    //TODO needed? emit signalRestart();
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUStyleBase::loadWindowGeometry()
{
    // setup window geometry

    if (m_cdu_config->contains(CFG_CDU_POS_X) && m_cdu_config->contains(CFG_CDU_POS_Y) &&
        m_cdu_config->contains(CFG_CDU_WIDTH) && m_cdu_config->contains(CFG_CDU_HEIGHT))
    {
//         Logger::log(QString("FMCCDU:loadWindowGeometry: resizing to %1/%2").
//                     arg(m_cdu_config->getIntValue(CFG_CDU_WIDTH)).
//                     arg(m_cdu_config->getIntValue(CFG_CDU_HEIGHT)));

        resize(m_cdu_config->getIntValue(CFG_CDU_WIDTH), m_cdu_config->getIntValue(CFG_CDU_HEIGHT));
        move(m_cdu_config->getIntValue(CFG_CDU_POS_X), m_cdu_config->getIntValue(CFG_CDU_POS_Y));
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUStyleBase::saveWindowGeometry()
{
#if !VASFMC_GAUGE
    // save window geometry

    m_cdu_config->setValue(CFG_CDU_POS_X, QString().setNum(x()));
    m_cdu_config->setValue(CFG_CDU_POS_Y, QString().setNum(y()));

    m_cdu_config->setValue(CFG_CDU_WIDTH, QString().setNum(geometry().width()));
    m_cdu_config->setValue(CFG_CDU_HEIGHT, QString().setNum(geometry().height()));

    m_cdu_config->setValue(CFG_CDU_WINDOW_STATUS, (m_is_visible ? 1 : 0));
#endif
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUStyleBase::slotProcessInput()
{
    QList<char>& fs_ctrls_list = 
        m_left_side ? m_fmc_control->flightStatus()->fsctrl_cdu_left_list :
        m_fmc_control->flightStatus()->fsctrl_cdu_right_list;

    while(!fs_ctrls_list.isEmpty())
    {
        QKeyEvent* key_event = 0;

        switch(fs_ctrls_list.first())
        {
            case(1): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier); break;
            case(2): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_B, Qt::NoModifier); break;
            case(3): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_C, Qt::NoModifier); break;
            case(4): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_D, Qt::NoModifier); break;
            case(5): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_E, Qt::NoModifier); break;
            case(6): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_F, Qt::NoModifier); break;
            case(7): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_G, Qt::NoModifier); break;
            case(8): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_H, Qt::NoModifier); break;
            case(9): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_I, Qt::NoModifier); break;
            case(10): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_J, Qt::NoModifier); break;
            case(11): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_K, Qt::NoModifier); break;
            case(12): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_L, Qt::NoModifier); break;
            case(13): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_M, Qt::NoModifier); break;
            case(14): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_N, Qt::NoModifier); break;
            case(15): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_O, Qt::NoModifier); break;
            case(16): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_P, Qt::NoModifier); break;
            case(17): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Q, Qt::NoModifier); break;
            case(18): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_R, Qt::NoModifier); break;
            case(19): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_S, Qt::NoModifier); break;
            case(20): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_T, Qt::NoModifier); break;
            case(21): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_U, Qt::NoModifier); break;
            case(22): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_V, Qt::NoModifier); break;
            case(23): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_W, Qt::NoModifier); break;
            case(24): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_X, Qt::NoModifier); break;
            case(25): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Y, Qt::NoModifier); break;
            case(26): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Z, Qt::NoModifier); break;

            case(27): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_1, Qt::NoModifier); break;
            case(28): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_2, Qt::NoModifier); break;
            case(29): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_3, Qt::NoModifier); break;
            case(30): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_4, Qt::NoModifier); break;
            case(31): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_5, Qt::NoModifier); break;
            case(32): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_6, Qt::NoModifier); break;
            case(33): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_7, Qt::NoModifier); break;
            case(34): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_8, Qt::NoModifier); break;
            case(35): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_9, Qt::NoModifier); break;
            case(36): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_0, Qt::NoModifier); break;

            case(37): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier); break;
            case(38): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Plus, Qt::NoModifier); break;
            case(39): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Minus, Qt::NoModifier); break;
            case(40): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier); break;
            case(41): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Period, Qt::NoModifier); break;
            case(42): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Slash, Qt::NoModifier); break;
            case(43): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Insert, Qt::NoModifier); break;
            case(44): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Asterisk, Qt::NoModifier); break;

            case(45): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_F1, Qt::NoModifier); break;
            case(46): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_F2, Qt::NoModifier); break;
            case(47): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_F3, Qt::NoModifier); break;
            case(48): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_F4, Qt::NoModifier); break;
            case(49): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_F5, Qt::NoModifier); break;
            case(50): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_F6, Qt::NoModifier); break;
            case(51): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_F7, Qt::NoModifier); break;
            case(52): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_F8, Qt::NoModifier); break;
            case(53): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_F9, Qt::NoModifier); break;
            case(54): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_F10, Qt::NoModifier); break;
            case(55): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_F11, Qt::NoModifier); break;
            case(56): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_F12, Qt::NoModifier); break;

            case(57): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Home, Qt::NoModifier); break;
            case(58): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_End, Qt::NoModifier); break;
            case(59): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_PageUp, Qt::NoModifier); break;
            case(60): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_PageDown, Qt::NoModifier); break;

            case(61): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_F1, Qt::ShiftModifier); break;
            case(62): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_F2, Qt::ShiftModifier); break;
            case(63): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_F3, Qt::ShiftModifier); break;
            case(64): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_F4, Qt::ShiftModifier); break;
            case(65): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_F5, Qt::ShiftModifier); break;
            case(66): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_F6, Qt::ShiftModifier); break;
            case(67): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_F7, Qt::ShiftModifier); break;
            case(68): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_F8, Qt::ShiftModifier); break;
            case(69): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_F9, Qt::ShiftModifier); break;
            case(80): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_F10, Qt::ShiftModifier); break;
            case(81): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_F11, Qt::ShiftModifier); break;
            case(82): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_F12, Qt::ShiftModifier); break;
            case(83): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_F1, Qt::ControlModifier); break;
            case(84): key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_F2, Qt::ControlModifier); break;

            case(70): isVisible() ? hide() : show(); break;
            case(71): m_fmc_control->toggleKeepOnTop(); break;
#if !VASFMC_GAUGE
            case(72): activateWindow(); break;
#endif
        }

        if (key_event != 0)
        {
            keyPressEvent(key_event);
            delete key_event;
        }

        fs_ctrls_list.removeFirst();
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

FMCCDUHandler::FMCCDUHandler(ConfigWidgetProvider* config_widget_provider,
                             Config* main_config,
                             const QString& cdu_config_filename,
                             FMCControl* fmc_control,
                             bool left_side) :
    m_config_widget_provider(config_widget_provider),
    m_main_config(main_config), m_cdu_config_filename(cdu_config_filename), 
    m_fmc_control(fmc_control), m_cdu(0), m_left_side(left_side)
{
    MYASSERT(m_config_widget_provider != 0);
    MYASSERT(m_main_config != 0);
    MYASSERT(m_fmc_control != 0);
    slotRestartCdu();
    (m_left_side) ? fmc_control->setCDULeftHandler(this) : fmc_control->setCDURightHandler(this);
}

/////////////////////////////////////////////////////////////////////////////

FMCCDUStyleBase* FMCCDUHandler::createCdu()
{
    switch(m_main_config->getIntValue(CFG_STYLE))
    {
        case(CFG_STYLE_A):
        case(CFG_STYLE_B): {
#if !VASFMC_GAUGE            
            if (m_fmc_control->doKeepOnTop())
                return new FMCCDUStyleA(m_config_widget_provider, m_main_config, m_cdu_config_filename, 
                                        m_fmc_control, 0, Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint, m_left_side);
            else
#endif
                return new FMCCDUStyleA(m_config_widget_provider, m_main_config, 
                                        m_cdu_config_filename, m_fmc_control, 0, 0, m_left_side);
        }
    }
    
    return 0;
}

// End of file
