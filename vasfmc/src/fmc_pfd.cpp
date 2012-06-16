/////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////

#include <QVBoxLayout>
#include <QTimer>
#include <QString>
#include <QStringList>
#include <QAction>
#include <QKeySequence>
#include <QShortcut>
#include "vas_gl_format.h"

#include "logger.h"
#include "navcalc.h"
#include "waypoint.h"
#include "flightstatus.h"
#include "projection_mercator.h"

#include "fmc_control.h"

#include "fmc_pfd.h"
#include "fmc_pfd_defines.h"
#include "fmc_pfd_glwidget_style_a.h"
#include "fmc_pfd_glwidget_style_b.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

/////////////////////////////////////////////////////////////////////////////

FMCPFD::FMCPFD(ConfigWidgetProvider* config_widget_provider,
               Config* main_config,
               const QString& pfd_config_filename,
               FMCControl* fmc_control,
               QWidget* parent, 
               Qt::WFlags fl,
               bool left_side)
    : VasWidget(parent, fl), m_config_widget_provider(config_widget_provider), 
      m_main_config(main_config), m_pfd_config(0), 
      m_fmc_control(fmc_control), m_gl_pfd(0), m_left_side(left_side)
{
    MYASSERT(m_config_widget_provider != 0);
    MYASSERT(m_fmc_control != 0);
#if !VASFMC_GAUGE
    setupUi(this);
    m_left_side ? setWindowTitle("PFD left") : setWindowTitle("PFD right");
    setFocusPolicy(Qt::StrongFocus);
    setFocus();
#endif

    // setup config

    m_pfd_config = new Config(pfd_config_filename);
    MYASSERT(m_pfd_config != 0);
 
    setupDefaultConfig();
    m_pfd_config->loadfromFile();
    loadWindowGeometry();
    m_pfd_config->saveToFile();

    // setup GL widget

    if (m_main_config->getIntValue(CFG_STYLE) == CFG_STYLE_A)
        m_gl_pfd = new GLPFDWidgetStyleA(
            config_widget_provider, main_config, m_pfd_config, m_fmc_control, this, m_left_side);
    else if (m_main_config->getIntValue(CFG_STYLE) == CFG_STYLE_B)
        m_gl_pfd = new GLPFDWidgetStyleB(
            config_widget_provider, main_config, m_pfd_config, m_fmc_control, this, m_left_side);

#if !VASFMC_GAUGE
    // setup GUI

    QVBoxLayout* layout = new QVBoxLayout(this);
    MYASSERT(layout != 0);
    layout->setSpacing(0);
    layout->setMargin(0);
    if (m_gl_pfd != 0) layout->addWidget(m_gl_pfd);
    setLayout(layout);
#endif
    
#if !VASFMC_GAUGE
    if (m_pfd_config->getIntValue(CFG_PFD_WINDOW_STATUS) == 0) hide(); else show();
#endif

    m_config_widget_provider->registerConfigWidget("PFD", m_pfd_config);
}

/////////////////////////////////////////////////////////////////////////////

FMCPFD::~FMCPFD() 
{
    m_config_widget_provider->unregisterConfigWidget("PFD");
    saveWindowGeometry();
    m_pfd_config->saveToFile();
    delete m_gl_pfd;
    delete m_pfd_config;
}

/////////////////////////////////////////////////////////////////////////////

#if VASFMC_GAUGE
/* virtual */ void FMCPFD::paintBitmapToGauge(PELEMENT_STATIC_IMAGE pelement)
{
    if (m_gl_pfd != 0) m_gl_pfd->paintBitmapToGauge(pelement);
}
#endif

/////////////////////////////////////////////////////////////////////////////

void FMCPFD::slotRefresh() 
{
    QTime timer;
    timer.start();

    if (m_gl_pfd != 0) m_gl_pfd->refreshPFD(); 

    emit signalTimeUsed("PF", timer.elapsed());
}

/////////////////////////////////////////////////////////////////////////////

void FMCPFD::processFSControls()
{
    QList<char>& fs_ctrls_list = 
        m_left_side ? m_fmc_control->flightStatus()->fsctrl_pfd_left_list :
        m_fmc_control->flightStatus()->fsctrl_pfd_right_list;

    while(!fs_ctrls_list.isEmpty())
    {
        switch(fs_ctrls_list.first())
        {
            case(70): isVisible() ? hide() : show(); break;
            case(71): m_fmc_control->toggleKeepOnTop(); break;
#if !VASFMC_GAUGE
            case(72): activateWindow(); break;
#endif
            case(73): m_fmc_control->toggleAltimeterPressure(m_left_side); break;
            case(74): m_fmc_control->slotToggleMetricAlt(); break;
            case(75): m_fmc_control->slotToggleShowPFDILS(m_left_side); break;

            case(80): m_fmc_control->slotShowAltimterHPA(m_left_side); break;
            case(81): m_fmc_control->slotShowAltimterInches(m_left_side); break;
            case(82): m_fmc_control->slotShowMetricAlt(); break;
            case(83): m_fmc_control->slotHideMetricAlt(); break;
            case(84): m_fmc_control->setShowPFDILS(m_left_side, true); break;
            case(85): m_fmc_control->setShowPFDILS(m_left_side, false); break;
        }

        fs_ctrls_list.removeFirst();
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCPFD::keyPressEvent(QKeyEvent *event)
{
    MYASSERT(event != 0);
    
    int inc = (event->modifiers() & Qt::ShiftModifier ? 10 : 1);
    bool do_resize = (event->modifiers() & Qt::ControlModifier ? 1 : 0);

    if ((event->modifiers() & Qt::AltModifier) != 0)
    {
        switch(event->key())
        {
            case(Qt::Key_Left):  
                (do_resize) ? resize(size().width()-inc, size().height()) : move(pos().x()-inc, pos().y()); break;
            case(Qt::Key_Right): 
                (do_resize) ? resize(size().width()+inc, size().height()) : move(pos().x()+inc, pos().y()); break;
            case(Qt::Key_Up):
                (do_resize) ? resize(size().width(), size().height()-inc) : move(pos().x(), pos().y()-inc); break;
            case(Qt::Key_Down):  
                (do_resize) ? resize(size().width(), size().height()+inc) : move(pos().x(), pos().y()+inc); break;
                
            case(Qt::Key_K): m_fmc_control->toggleKeepOnTop(); break;
            case(Qt::Key_U): move(0,0); break;
            case(Qt::Key_Q): resize(size().width(), size().width()); break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCPFD::mouseMoveEvent(QMouseEvent *event)
{
#if !VASFMC_GAUGE
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
#else
    Q_UNUSED(event);
#endif
}

/////////////////////////////////////////////////////////////////////////////

void FMCPFD::wheelEvent(QWheelEvent* event)
{
    MYASSERT(event);
//     if (event->delta() < 0) setRange(m_pfd_config->getIntValue(CFG_SCROLL_MODE) ? false : true);
//     else                    setRange(m_pfd_config->getIntValue(CFG_SCROLL_MODE) ? true : false);
    event->accept();
}

/////////////////////////////////////////////////////////////////////////////

void FMCPFD::resizeEvent(QResizeEvent*)
{
    if (m_gl_pfd != 0) m_gl_pfd->reset();
}

/////////////////////////////////////////////////////////////////////////////

void FMCPFD::loadWindowGeometry()
{
    // setup window geometry

    if (m_pfd_config->contains(CFG_PFD_POS_X) && m_pfd_config->contains(CFG_PFD_POS_Y) &&
        m_pfd_config->contains(CFG_PFD_WIDTH) && m_pfd_config->contains(CFG_PFD_HEIGHT))
    {
        if (m_pfd_config->getIntValue(CFG_PFD_WIDTH) > 10 && m_pfd_config->getIntValue(CFG_PFD_HEIGHT) > 10)
            resize(m_pfd_config->getIntValue(CFG_PFD_WIDTH), m_pfd_config->getIntValue(CFG_PFD_HEIGHT));
        move(m_pfd_config->getIntValue(CFG_PFD_POS_X), m_pfd_config->getIntValue(CFG_PFD_POS_Y));
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCPFD::saveWindowGeometry()
{
#if !VASFMC_GAUGE
    // save window geometry

    m_pfd_config->setValue(CFG_PFD_POS_X, QString().setNum(x()));
    m_pfd_config->setValue(CFG_PFD_POS_Y, QString().setNum(y()));

    m_pfd_config->setValue(CFG_PFD_WIDTH, QString().setNum(geometry().width()));
    m_pfd_config->setValue(CFG_PFD_HEIGHT, QString().setNum(geometry().height()));

    m_pfd_config->setValue(CFG_PFD_WINDOW_STATUS, (m_is_visible ? 1 : 0));
#endif
}

/////////////////////////////////////////////////////////////////////////////

void FMCPFD::setupDefaultConfig()
{
    saveWindowGeometry();

    if ( m_left_side ) {
        m_pfd_config->setValue(CFG_PFD_POS_X, 375);
    } else {
        m_pfd_config->setValue(CFG_PFD_POS_X, 750);
    }
    m_pfd_config->setValue(CFG_PFD_POS_Y, 210);

}

/////////////////////////////////////////////////////////////////////////////

FMCPFDHandler::FMCPFDHandler(ConfigWidgetProvider* config_widget_provider, 
                             Config* main_config,
                             const QString& pfd_config_filename,
                             FMCControl* fmc_control,
                             bool left_side) :
    m_config_widget_provider(config_widget_provider),
    m_main_config(main_config), m_pfd_config_filename(pfd_config_filename),
    m_fmc_control(fmc_control), m_pfd(0), m_left_side(left_side)
{
    MYASSERT(m_config_widget_provider != 0);
    (m_left_side) ? fmc_control->setPFDLeftHandler(this) : fmc_control->setPFDRightHandler(this);
    slotRestartPFD();
}

/////////////////////////////////////////////////////////////////////////////

FMCPFD* FMCPFDHandler::createPFD()
{
#if !VASFMC_GAUGE
    if (m_fmc_control->doKeepOnTop())
        return new FMCPFD(m_config_widget_provider,
                          m_main_config, m_pfd_config_filename,
                          m_fmc_control, 0, Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint,
                          m_left_side);
    else
#endif
        return new FMCPFD(m_config_widget_provider,
                          m_main_config, m_pfd_config_filename,
                          m_fmc_control, 0, 0, m_left_side);
}

/////////////////////////////////////////////////////////////////////////////
