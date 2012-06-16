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

/*! \file    fmc_cdu_style_a.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include <QFrame>
#include <QPaintEvent>
#include <QPainter>
#include <QWindowsStyle>
#include <QPalette>
#include <QGridLayout>

#include "config.h"
#include "assert.h"
#include "logger.h"
#include "pushbutton.h"
#include "vas_path.h"

#include "fmc_control.h"

#include "fmc_cdu_defines.h"
#include "fmc_cdu_page_manager.h"

#include "fmc_cdu_style_a.h"

#if VASFMC_GAUGE
#include "vasfmc_gauge_main.h"
#endif


/////////////////////////////////////////////////////////////////////////////

FMCCDUStyleA::FMCCDUStyleA(ConfigWidgetProvider* config_widget_provider,
                           Config* main_config,
                           const QString& cdu_config_filename,
                           FMCControl* fmc_control,
                           QWidget* parent,
                           Qt::WFlags fl,
                           bool left_side) :
    FMCCDUStyleBase("A", config_widget_provider, main_config, cdu_config_filename, fmc_control, parent, fl, left_side), 
    m_page_manager(0)
{
    MYASSERT(fmc_control != 0);
#if !VASFMC_GAUGE
    setupUi(this);
    (m_left_side) ? setWindowTitle("MCDU left") : setWindowTitle("MCDU right");
    setFocusPolicy(Qt::StrongFocus);
    setFocus();
    display->installEventFilter(this);
    setMouseTracking(true);
    display->setMouseTracking(true);
#endif

    // setup page manager

    m_page_manager = new FMCCDUPageManagerStyleA(m_cdu_config, m_fmc_control, left_side);
    MYASSERT(m_page_manager != 0);

    // set to display only mode when configured
    
    if (m_fmc_control->cduDisplayOnlyMode())
    {
#if !VASFMC_GAUGE
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        setMinimumSize(QSize(0, 0));
        setMaximumSize(QSize(16777215, 16777215));
        display->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        QGridLayout* new_layout = new QGridLayout(this);
        MYASSERT(new_layout != 0);
        new_layout->setHorizontalSpacing(0);
        new_layout->setVerticalSpacing(0);
        new_layout->setContentsMargins(0,0,0,0);
        new_layout->addWidget(display);
        setLayout(new_layout);
        loadWindowGeometry();
#endif
    }
    else
    {
        // setup background image
        
        QPixmap background_image(VasPath::prependPath(m_cdu_config->getValue(CFG_CDU_BACKGROUND_IMAGE)));
        m_background_image = background_image;
        
        if (!m_background_image.isNull())
        {
#if !VASFMC_GAUGE

            setMinimumSize(QSize(background_image.width(), background_image.height()));
            setMaximumSize(QSize(background_image.width(), background_image.height()));
            resize(QSize(background_image.width(), background_image.height()));

            QPalette our_palette = palette();
            
            for (int index = 0; index < QPalette::NColorGroups; ++index) {
                QColor color = our_palette.brush(QPalette::ColorGroup(index), QPalette::Background).color();
                our_palette.setBrush(QPalette::ColorGroup(index), QPalette::Background, QBrush(color, background_image));
            }
            
            setPalette(our_palette);

            display->move(m_cdu_config->getIntValue(CFG_CDU_DISPLAY_X), 
                          m_cdu_config->getIntValue(CFG_CDU_DISPLAY_Y));
            display->resize(m_cdu_config->getIntValue(CFG_CDU_DISPLAY_WIDTH), 
                            m_cdu_config->getIntValue(CFG_CDU_DISPLAY_HEIGHT));
#else
            m_gauge_background_width = m_background_image.width();
            m_gauge_background_height = m_background_image.height();
#endif
        }
        else
        {
            Logger::log("FMCCDUStyleA: could not load background image");
        }

        // setup input area

        MYASSERT(connect(&m_input_area, SIGNAL(signalMouseClick(const QString&, QMouseEvent*)), 
                         this, SLOT(slotInput(const QString&, QMouseEvent*))));
        MYASSERT(connect(&m_input_area, SIGNAL(signalMouseLongClick(const QString&, QMouseEvent*)), 
                         this, SLOT(slotInputLongClick(const QString&, QMouseEvent*))));

        if (m_input_area.loadFromFile(m_cdu_config->getValue(CFG_CDU_INPUT_AREA_FILE)))
        {
            Logger::log(QString("FMCCDUStyleA: loaded input areas from %1").
                        arg(m_cdu_config->getValue(CFG_CDU_INPUT_AREA_FILE)));
        }
        else
        {
            InputAreaList input_area_list;
            input_area_list.append(InputArea(FMCCDUPageManagerStyleA::PAGE_DIRECT, 47, 278, 40, 27));
            input_area_list.append(InputArea(FMCCDUPageManagerStyleA::PAGE_PROG, 95, 278, 40, 27));
            input_area_list.append(InputArea(FMCCDUPageManagerStyleA::PAGE_PERF, 142, 278, 40, 27));
            input_area_list.append(InputArea(FMCCDUPageManagerStyleA::PAGE_INIT, 193, 278, 40, 27));
            input_area_list.append(InputArea(FMCCDUPageManagerStyleA::PAGE_DATA, 244, 278, 40, 27));
            input_area_list.append(InputArea(FMCCDUPageManagerStyleA::PAGE_TEST, 293, 278, 40, 27));
            
            input_area_list.append(InputArea(FMCCDUPageManagerStyleA::PAGE_FPLAN, 47, 314, 40, 27));
            input_area_list.append(InputArea(FMCCDUPageManagerStyleA::PAGE_NAV, 95, 314, 40, 27));
            input_area_list.append(InputArea(FMCCDUPageManagerStyleA::PAGE_FPRED, 142, 314, 40, 27));
            input_area_list.append(InputArea(FMCCDUPageManagerStyleA::PAGE_SECFP, 193, 314, 40, 27));
            input_area_list.append(InputArea(FMCCDUPageManagerStyleA::PAGE_ATC, 244, 314, 40, 27));
            input_area_list.append(InputArea(FMCCDUPageManagerStyleA::PAGE_MENU, 293, 314, 40, 27));
            
            input_area_list.append(InputArea(FMCCDUPageManagerStyleA::PAGE_AIRPORT, 47, 349, 40, 27));
            
            input_area_list.append(InputArea(FMCCDUPageBase::ACTION_LLSK1, 15, 73, 24, 16));
            input_area_list.append(InputArea(FMCCDUPageBase::ACTION_LLSK2, 15, 102, 24, 16));
            input_area_list.append(InputArea(FMCCDUPageBase::ACTION_LLSK3, 15, 130, 24, 16));
            input_area_list.append(InputArea(FMCCDUPageBase::ACTION_LLSK4, 15, 158, 24, 16));
            input_area_list.append(InputArea(FMCCDUPageBase::ACTION_LLSK5, 15, 186, 24, 16));
            input_area_list.append(InputArea(FMCCDUPageBase::ACTION_LLSK6, 15, 215, 24, 16));
            
            input_area_list.append(InputArea(FMCCDUPageBase::ACTION_RLSK1, 331, 73, 24, 16));
            input_area_list.append(InputArea(FMCCDUPageBase::ACTION_RLSK2, 331, 102, 24, 16));
            input_area_list.append(InputArea(FMCCDUPageBase::ACTION_RLSK3, 331, 130, 24, 16));
            input_area_list.append(InputArea(FMCCDUPageBase::ACTION_RLSK4, 331, 158, 24, 16));
            input_area_list.append(InputArea(FMCCDUPageBase::ACTION_RLSK5, 331, 186, 24, 16));
            input_area_list.append(InputArea(FMCCDUPageBase::ACTION_RLSK6, 331, 215, 24, 16));
            
            input_area_list.append(InputArea(FMCCDUPageBase::ACTION_PREV, 47, 382, 40, 27, 100));
            input_area_list.append(InputArea(FMCCDUPageBase::ACTION_NEXT, 47, 414, 40, 27, 100));
            input_area_list.append(InputArea(FMCCDUPageBase::ACTION_UP, 95, 382, 40, 27, 100));
            input_area_list.append(InputArea(FMCCDUPageBase::ACTION_DOWN, 95, 414, 40, 27, 100));
            
            input_area_list.append(InputArea(FMCCDUPageBase::ACTION_CLIPBOARD, 95, 349, 41, 27));

            input_area_list.append(InputArea("A", 153, 361, 31, 31));
            input_area_list.append(InputArea("B", 189, 361, 31, 31));
            input_area_list.append(InputArea("C", 225, 361, 31, 31));
            input_area_list.append(InputArea("D", 260, 361, 31, 31));
            input_area_list.append(InputArea("E", 296, 361, 31, 31));
            input_area_list.append(InputArea("F", 153, 395, 31, 31));
            input_area_list.append(InputArea("G", 189, 395, 31, 31));
            input_area_list.append(InputArea("H", 225, 395, 31, 31));
            input_area_list.append(InputArea("I", 260, 395, 31, 31));
            input_area_list.append(InputArea("J", 296, 395, 31, 31));
            input_area_list.append(InputArea("K", 153, 431, 31, 31));
            input_area_list.append(InputArea("L", 189, 431, 31, 31));
            input_area_list.append(InputArea("M", 225, 431, 31, 31));
            input_area_list.append(InputArea("N", 260, 431, 31, 31));
            input_area_list.append(InputArea("O", 296, 431, 31, 31));
            input_area_list.append(InputArea("P", 153, 466, 31, 31));
            input_area_list.append(InputArea("Q", 189, 466, 31, 31));
            input_area_list.append(InputArea("R", 225, 466, 31, 31));
            input_area_list.append(InputArea("S", 260, 466, 31, 31));
            input_area_list.append(InputArea("T", 296, 466, 31, 31));
            input_area_list.append(InputArea("U", 153, 502, 31, 31));
            input_area_list.append(InputArea("V", 189, 502, 31, 31));
            input_area_list.append(InputArea("W", 225, 502, 31, 31));
            input_area_list.append(InputArea("X", 260, 502, 31, 31));
            input_area_list.append(InputArea("Y", 296, 502, 31, 31));
            input_area_list.append(InputArea("Z", 153, 538, 31, 31));
            input_area_list.append(InputArea("/", 189, 538, 31, 31));
            input_area_list.append(InputArea(" ", 225, 538, 31, 31));
            input_area_list.append(InputArea(FMCCDUPageBase::ACTION_OVERFLY, 260, 538, 31, 31));
            input_area_list.append(InputArea(FMCCDUPageBase::ACTION_CLR, 296, 538, 31, 31, 0, 500));

            input_area_list.append(InputArea("1", 47, 457, 24, 24));
            input_area_list.append(InputArea("2", 80, 457, 24, 24));
            input_area_list.append(InputArea("3", 116, 457, 24, 24));
            input_area_list.append(InputArea("4", 47, 485, 24, 24));
            input_area_list.append(InputArea("5", 80, 485, 24, 24));
            input_area_list.append(InputArea("6", 116, 485, 24, 24));
            input_area_list.append(InputArea("7", 47, 514, 24, 24));
            input_area_list.append(InputArea("8", 80, 514, 24, 24));
            input_area_list.append(InputArea("9", 116, 514, 24, 24));
            input_area_list.append(InputArea(".", 47, 545, 24, 24));
            input_area_list.append(InputArea("0", 80, 545, 24, 24));
            input_area_list.append(InputArea(FMCCDUPageBase::ACTION_PLUSMINUS, 116, 545, 24, 24, 100));
            // Always have to add this input area, even when running as
            // non-gauge, so that it will appear in the config file
            input_area_list.append(InputArea(FMCCDUPageBase::ACTION_TOGGLE_KEYBOARD, 47, 35, 276, 219));
                        
            m_input_area.clear();
            m_input_area.setInputAreaList(input_area_list);

            if (!m_input_area.saveToFile(m_cdu_config->getValue(CFG_CDU_INPUT_AREA_FILE)))
                Logger::log("FMCCDUStyleA: could not save default CDU input areas");
            else
                Logger::log(QString("FMCCDUStyleA: saved default CDU input areas to (%1)").
                            arg(m_cdu_config->getValue(CFG_CDU_INPUT_AREA_FILE)));
        }
    }

#if !VASFMC_GAUGE
    if (m_cdu_config->getIntValue(CFG_CDU_WINDOW_STATUS) == 0) hide(); else show();
#endif
}

/////////////////////////////////////////////////////////////////////////////

FMCCDUStyleA::~FMCCDUStyleA()
{
    delete m_page_manager;
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUStyleA::slotRefresh()
{ 
    QTime timer;
    timer.start();

#if VASFMC_GAUGE
    paintToBitmap();
#else
    display->update();
#endif

    emit signalTimeUsed("CD", timer.elapsed());
}

/////////////////////////////////////////////////////////////////////////////

#if VASFMC_GAUGE
/* virtual */ void FMCCDUStyleA::paintGauge(QPainter *pPainter)
{
    QRect rectWindow=pPainter->window();

    if (m_fmc_control->cduDisplayOnlyMode())
    {
        pPainter->setViewport(10, 10, rectWindow.width()-20, rectWindow.height()-20);
        paintMe(*pPainter);
    }
    else
    {
        //TODO let the background be painted by the FS !?
        // Draw background image
        if(!m_background_image.isNull())
        {
            // Scale background image if necessary
            if(m_background_image_scaled.isNull() ||
               m_background_image_scaled.width()!=rectWindow.width() ||
               m_background_image_scaled.height()!=rectWindow.height())
            {
                m_background_image_scaled=m_background_image.scaled(
                    rectWindow.width(), rectWindow.height(),
                    Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            }
        
            // Draw background image
            pPainter->drawPixmap(0, 0, m_background_image_scaled);
        }

        // Draw display into pixmap first
        pPainter->setWindow(0, 0, 272, 200);
        pPainter->setViewport(49*rectWindow.width()/m_background_image.width(),
                              44*rectWindow.height()/m_background_image.height(),
                              272*rectWindow.width()/m_background_image.width(),
                              200*rectWindow.height()/m_background_image.height());
        paintMe(*pPainter);
    }
}
#endif

/////////////////////////////////////////////////////////////////////////////

void FMCCDUStyleA::slotClearAll()
{
    m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUStyleA::slotInput(const QString& text, QMouseEvent*)
{
    if (m_fmc_control->flightStatus()->isValid() && !m_fmc_control->flightStatus()->battery_on) return;

    if (text == FMCCDUPageManagerStyleA::PAGE_DIRECT ||
        text == FMCCDUPageManagerStyleA::PAGE_INIT ||
        text == FMCCDUPageManagerStyleA::PAGE_TEST ||
        text == FMCCDUPageManagerStyleA::PAGE_FPLAN ||
        text == FMCCDUPageManagerStyleA::PAGE_NAV||
        text == FMCCDUPageManagerStyleA::PAGE_MENU||
        text == FMCCDUPageManagerStyleA::PAGE_PROG||
        text == FMCCDUPageManagerStyleA::PAGE_PERF||
        text == FMCCDUPageManagerStyleA::PAGE_DATA||
        text == FMCCDUPageManagerStyleA::PAGE_FPRED||
        text == FMCCDUPageManagerStyleA::PAGE_SECFP||
        text == FMCCDUPageManagerStyleA::PAGE_ATC||
        text == FMCCDUPageManagerStyleA::PAGE_AIRPORT)
    {
        m_page_manager->setCurrentPage(text);
    }
    else if (text == FMCCDUPageBase::ACTION_LLSK1 ||
             text == FMCCDUPageBase::ACTION_LLSK2 ||
             text == FMCCDUPageBase::ACTION_LLSK3 ||
             text == FMCCDUPageBase::ACTION_LLSK4 ||
             text == FMCCDUPageBase::ACTION_LLSK5 ||
             text == FMCCDUPageBase::ACTION_LLSK6 ||
             text == FMCCDUPageBase::ACTION_RLSK1 ||
             text == FMCCDUPageBase::ACTION_RLSK2 ||
             text == FMCCDUPageBase::ACTION_RLSK3 ||
             text == FMCCDUPageBase::ACTION_RLSK4 ||
             text == FMCCDUPageBase::ACTION_RLSK5 ||
             text == FMCCDUPageBase::ACTION_RLSK6 ||
             text == FMCCDUPageBase::ACTION_PREV ||
             text == FMCCDUPageBase::ACTION_NEXT ||
             text == FMCCDUPageBase::ACTION_UP ||
             text == FMCCDUPageBase::ACTION_DOWN)
    {
        FMCCDUPageBase* page = m_page_manager->activePage();
        if (page == 0) return;
        page->processAction(text);
    }
#if VASFMC_GAUGE
    else if (text == FMCCDUPageBase::ACTION_TOGGLE_KEYBOARD)
    {
        Logger::log("FMCCDUStyleA:slotInput: toggle KBD");
        toggleMcduKeyboard();
    }
#endif
    else
    {
        m_page_manager->scratchpad().processAction(text);
    }

    slotRefresh();
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUStyleA::slotInputLongClick(const QString& text, QMouseEvent*)
{
    if (text == FMCCDUPageBase::ACTION_CLR)
    {
        m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
        slotRefresh();
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUStyleA::keyPressEvent(QKeyEvent *event)
{
    MYASSERT(event != 0);
    
    bool ctrl_mod = (event->modifiers() & Qt::ControlModifier ? 1 : 0);

    if ((event->modifiers() & Qt::ShiftModifier) != 0)
    {
        switch(event->key())
        {
            case(Qt::Key_F1): m_page_manager->setCurrentPage(FMCCDUPageManagerStyleA::PAGE_DIRECT); break;
            case(Qt::Key_F2): m_page_manager->setCurrentPage(FMCCDUPageManagerStyleA::PAGE_PROG); break;
            case(Qt::Key_F3): m_page_manager->setCurrentPage(FMCCDUPageManagerStyleA::PAGE_PERF); break;
            case(Qt::Key_F4): m_page_manager->setCurrentPage(FMCCDUPageManagerStyleA::PAGE_INIT); break;
            case(Qt::Key_F5): m_page_manager->setCurrentPage(FMCCDUPageManagerStyleA::PAGE_DATA); break;
            case(Qt::Key_F6): m_page_manager->setCurrentPage(FMCCDUPageManagerStyleA::PAGE_TEST); break;
                
            case(Qt::Key_F7): m_page_manager->setCurrentPage(FMCCDUPageManagerStyleA::PAGE_FPLAN); break;
            case(Qt::Key_F8): m_page_manager->setCurrentPage(FMCCDUPageManagerStyleA::PAGE_NAV); break;
            case(Qt::Key_F9): m_page_manager->setCurrentPage(FMCCDUPageManagerStyleA::PAGE_FPRED); break;
            case(Qt::Key_F10): m_page_manager->setCurrentPage(FMCCDUPageManagerStyleA::PAGE_SECFP); break;
            case(Qt::Key_F11): m_page_manager->setCurrentPage(FMCCDUPageManagerStyleA::PAGE_ATC); break;
            case(Qt::Key_F12): m_page_manager->setCurrentPage(FMCCDUPageManagerStyleA::PAGE_MENU); break;

            case(Qt::Key_Slash): m_page_manager->scratchpad().processAction("/"); break;
            case(Qt::Key_Asterisk): m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_OVERFLY); break;

            case(Qt::Key_Left):  
                (ctrl_mod) ? resize(size().width()-10, size().height()) : move(pos().x()-10, pos().y()); break;
            case(Qt::Key_Right): 
                (ctrl_mod) ? resize(size().width()+10, size().height()) : move(pos().x()+10, pos().y()); break;
            case(Qt::Key_Up):
                (ctrl_mod) ? resize(size().width(), size().height()-10) : move(pos().x(), pos().y()-10); break;
            case(Qt::Key_Down):  
                (ctrl_mod) ? resize(size().width(), size().height()+10) : move(pos().x(), pos().y()+10); break;
                
            case(Qt::Key_PageUp): {
                incFontSize();
                break;
            }

            case(Qt::Key_PageDown): {
                decFontSize();
                break;
            }
        }
    }
    else if ((event->modifiers() & Qt::AltModifier) != 0)
    {
        switch(event->key())
        {
            case(Qt::Key_Left):  
                (ctrl_mod) ? resize(size().width()-1, size().height()) : move(pos().x()-1, pos().y()); break;
            case(Qt::Key_Right): 
                (ctrl_mod) ? resize(size().width()+1, size().height()) : move(pos().x()+1, pos().y()); break;
            case(Qt::Key_Up):
                (ctrl_mod) ? resize(size().width(), size().height()-1) : move(pos().x(), pos().y()-1); break;
            case(Qt::Key_Down):  
                (ctrl_mod) ? resize(size().width(), size().height()+1) : move(pos().x(), pos().y()+1); break;

            case(Qt::Key_U): move(0,22); break;
            case(Qt::Key_K): m_fmc_control->toggleKeepOnTop(); break;
            case(Qt::Key_D): m_fmc_control->slotToggleCduDisplayOnlyMode(false); break;
        }
    }
    else
    {
        switch(event->key())
        {
            case(Qt::Key_Backspace): m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLR); break;
            case(Qt::Key_Plus) : m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_PLUSMINUS); break;
            case(Qt::Key_Minus): m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_PLUSMINUS); break;
            case(Qt::Key_A): m_page_manager->scratchpad().processAction("A"); break;
            case(Qt::Key_B): m_page_manager->scratchpad().processAction("B"); break;
            case(Qt::Key_C): m_page_manager->scratchpad().processAction("C"); break;
            case(Qt::Key_D): m_page_manager->scratchpad().processAction("D"); break;
            case(Qt::Key_E): m_page_manager->scratchpad().processAction("E"); break;
            case(Qt::Key_F): m_page_manager->scratchpad().processAction("F"); break;
            case(Qt::Key_G): m_page_manager->scratchpad().processAction("G"); break;
            case(Qt::Key_H): m_page_manager->scratchpad().processAction("H"); break;
            case(Qt::Key_I): m_page_manager->scratchpad().processAction("I"); break;
            case(Qt::Key_J): m_page_manager->scratchpad().processAction("J"); break;
            case(Qt::Key_K): m_page_manager->scratchpad().processAction("K"); break;
            case(Qt::Key_L): m_page_manager->scratchpad().processAction("L"); break;
            case(Qt::Key_M): m_page_manager->scratchpad().processAction("M"); break;
            case(Qt::Key_N): m_page_manager->scratchpad().processAction("N"); break;
            case(Qt::Key_O): m_page_manager->scratchpad().processAction("O"); break;
            case(Qt::Key_P): m_page_manager->scratchpad().processAction("P"); break;
            case(Qt::Key_Q): m_page_manager->scratchpad().processAction("Q"); break;
            case(Qt::Key_R): m_page_manager->scratchpad().processAction("R"); break;
            case(Qt::Key_S): m_page_manager->scratchpad().processAction("S"); break;
            case(Qt::Key_T): m_page_manager->scratchpad().processAction("T"); break;
            case(Qt::Key_U): m_page_manager->scratchpad().processAction("U"); break;
            case(Qt::Key_V): m_page_manager->scratchpad().processAction("V"); break;
            case(Qt::Key_W): m_page_manager->scratchpad().processAction("W"); break;
            case(Qt::Key_X): m_page_manager->scratchpad().processAction("X"); break;
            case(Qt::Key_Y): m_page_manager->scratchpad().processAction("Y"); break;
            case(Qt::Key_Z): m_page_manager->scratchpad().processAction("Z"); break;

            case(Qt::Key_1): m_page_manager->scratchpad().processAction("1"); break;
            case(Qt::Key_2): m_page_manager->scratchpad().processAction("2"); break;
            case(Qt::Key_3): m_page_manager->scratchpad().processAction("3"); break;
            case(Qt::Key_4): m_page_manager->scratchpad().processAction("4"); break;
            case(Qt::Key_5): m_page_manager->scratchpad().processAction("5"); break;
            case(Qt::Key_6): m_page_manager->scratchpad().processAction("6"); break;
            case(Qt::Key_7): m_page_manager->scratchpad().processAction("7"); break;
            case(Qt::Key_8): m_page_manager->scratchpad().processAction("8"); break;
            case(Qt::Key_9): m_page_manager->scratchpad().processAction("9"); break;
            case(Qt::Key_0): m_page_manager->scratchpad().processAction("0"); break;

            case(Qt::Key_Space): m_page_manager->scratchpad().processAction(" "); break;
            case(Qt::Key_Period): m_page_manager->scratchpad().processAction("."); break;
            case(Qt::Key_Slash): m_page_manager->scratchpad().processAction("/"); break;

            case(Qt::Key_Insert): m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLIPBOARD); break;
            case(Qt::Key_Asterisk): m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_OVERFLY); break;

            case(Qt::Key_F1): {
                if (!ctrl_mod)
                    m_page_manager->activePage()->processAction(FMCCDUPageBase::ACTION_LLSK1);
                else
                    m_page_manager->setCurrentPage(FMCCDUPageManagerStyleA::PAGE_AIRPORT);
                break;
            }

            case(Qt::Key_F2): {
                if (!ctrl_mod)
                    m_page_manager->activePage()->processAction(FMCCDUPageBase::ACTION_LLSK2);
                else
                    m_page_manager->activePage()->processAction(FMCCDUPageBase::ACTION_CLIPBOARD);
                break;
            }

            case(Qt::Key_F3): m_page_manager->activePage()->processAction(FMCCDUPageBase::ACTION_LLSK3); break;
            case(Qt::Key_F4): m_page_manager->activePage()->processAction(FMCCDUPageBase::ACTION_LLSK4); break;
            case(Qt::Key_F5): m_page_manager->activePage()->processAction(FMCCDUPageBase::ACTION_LLSK5); break;
            case(Qt::Key_F6): m_page_manager->activePage()->processAction(FMCCDUPageBase::ACTION_LLSK6); break;
            
            case(Qt::Key_F7): m_page_manager->activePage()->processAction(FMCCDUPageBase::ACTION_RLSK1); break;
            case(Qt::Key_F8): m_page_manager->activePage()->processAction(FMCCDUPageBase::ACTION_RLSK2); break;
            case(Qt::Key_F9): m_page_manager->activePage()->processAction(FMCCDUPageBase::ACTION_RLSK3); break;
            case(Qt::Key_F10): m_page_manager->activePage()->processAction(FMCCDUPageBase::ACTION_RLSK4); break;
            case(Qt::Key_F11): m_page_manager->activePage()->processAction(FMCCDUPageBase::ACTION_RLSK5); break;

            case(Qt::Key_F12): {

                if (!ctrl_mod)
                    m_page_manager->activePage()->processAction(FMCCDUPageBase::ACTION_RLSK6); 
                else
                    m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLIPBOARD); break;
                break;
            }

            case(Qt::Key_Left):
            case(Qt::Key_Home): m_page_manager->activePage()->processAction(FMCCDUPageBase::ACTION_PREV); break;
            case(Qt::Key_Right):
            case(Qt::Key_End): m_page_manager->activePage()->processAction(FMCCDUPageBase::ACTION_NEXT); break;

            case(Qt::Key_Up):
            case(Qt::Key_PageUp): 
                m_page_manager->activePage()->processAction(
                    inverseScrollMode() ? FMCCDUPageBase::ACTION_DOWN : FMCCDUPageBase::ACTION_UP);
                break;
            case(Qt::Key_Down):
            case(Qt::Key_PageDown): 
                m_page_manager->activePage()->processAction(
                    inverseScrollMode() ? FMCCDUPageBase::ACTION_UP : FMCCDUPageBase::ACTION_DOWN);
                break;
        }
    }
     
    slotRefresh();
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUStyleA::mousePressEvent(QMouseEvent *event)
{
    if (m_fmc_control->cduDisplayOnlyMode())
    {
#if VASFMC_GAUGE
        toggleMcduKeyboard();
#endif
    }
    else
    {
        m_input_area.slotMousePressEvent(event);
    }

    m_last_mouse_position = event->globalPos();
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUStyleA::mouseMoveEvent(QMouseEvent *event)
{
#if !VASFMC_GAUGE
    if (event->buttons() == Qt::NoButton)
    {
        m_input_area.slotMouseMoveEvent(event, this, Qt::PointingHandCursor);
    }
    else
    {
        QPoint mouse_move_diff = event->globalPos() - m_last_mouse_position;
        
        if (!isMaximized())
        {
            if (event->modifiers() == Qt::ShiftModifier && event->buttons() & Qt::LeftButton)
            {
                move(pos() + mouse_move_diff);
            }
            else if (event->modifiers() == Qt::ControlModifier && event->buttons() &  Qt::LeftButton)
            {
                int x_inc = mouse_move_diff.x();
                resize(size() + QSize(x_inc, mouse_move_diff.y()));
            }
        }
        
        m_last_mouse_position = event->globalPos();
    }
#else
    Q_UNUSED(event);
#endif
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUStyleA::wheelEvent(QWheelEvent* event)
{
    if (event->delta() < 0) 
        m_page_manager->activePage()->processAction(
            inverseScrollMode() ? FMCCDUPageBase::ACTION_UP : FMCCDUPageBase::ACTION_DOWN);
    else                    
        m_page_manager->activePage()->processAction(
            inverseScrollMode() ? FMCCDUPageBase::ACTION_DOWN : FMCCDUPageBase::ACTION_UP);

    slotRefresh();
}

/////////////////////////////////////////////////////////////////////////////

bool FMCCDUStyleA::eventFilter(QObject * watched_object, QEvent* event)
{
#if !VASFMC_GAUGE
    if (watched_object == display)
    {
        switch(event->type())
        {
            case(QEvent::Paint): {
                QPainter painter(display);
                paintMe(painter);
                return true;
            }
            case(QEvent::Resize): {
                m_page_manager->resetAllPages();
                return true;
            }

            default: break;
        }
    }
#else
    Q_UNUSED(watched_object);
#endif

    event->ignore();
    
    return false;
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUStyleA::paintMe(QPainter& painter)
{
    if (!isVisible() || (m_fmc_control->flightStatus()->isValid() && !m_fmc_control->flightStatus()->battery_on)) 
    {
        painter.setBackground(QBrush(BLACK));
        painter.fillRect(painter.window(), painter.background());
        return;
    }

    //-----

    const FMCCDUPageBase* page = m_page_manager->activePage();
    if (page != 0)
    {
        page->paintPage(painter);
        page->drawUpDownArrows(painter);
        page->drawLeftRightArrows(painter);

#if VASFMC_GAUGE        
        if(m_showKbdActiveIndicator)
            page->drawKbdActiveIndicator(painter);
#endif
    }
    
    m_page_manager->scratchpad().paintPage(painter);

#if !VASFMC_GAUGE
    if (m_fmc_control->showInputAreas())
    {
        painter.setPen(YELLOW);
        painter.drawRect(0, 0, display->width()-1, display->height()-1);
    }
#endif
}

/////////////////////////////////////////////////////////////////////////////

// End of file
