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
#include "fmc_ecam.h"
#include "fmc_ecam_defines.h"
#include "fmc_ecam_glwidget_style_a.h"
#include "fmc_ecam_glwidget_style_b.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

/////////////////////////////////////////////////////////////////////////////

FMCECAM::FMCECAM(bool upper_ecam,
                 ConfigWidgetProvider* config_widget_provider,
                 Config* main_config,
                 const QString& ecam_config_filename,
                 FMCControl* fmc_control,
                 QWidget* parent, 
                 Qt::WFlags fl)
    : VasWidget(parent, fl), m_upper_ecam(upper_ecam), 
      m_config_widget_provider(config_widget_provider), 
      m_main_config(main_config), m_ecam_config(0), 
      m_fmc_control(fmc_control), m_gl_ecam(0)
{
    MYASSERT(m_config_widget_provider != 0);
    MYASSERT(m_fmc_control != 0);
#if !VASFMC_GAUGE
    setupUi(this);
    if (m_upper_ecam) setWindowTitle("upper ECAM");
    else setWindowTitle("lower ECAM");
    setFocusPolicy(Qt::StrongFocus);
    setFocus();
#endif

    // setup config

    m_ecam_config = new Config(ecam_config_filename);
    MYASSERT(m_ecam_config != 0);
 
    setupDefaultConfig();
    m_ecam_config->loadfromFile();
    loadWindowGeometry();
    m_ecam_config->saveToFile();

    // setup GL widget

    if (m_main_config->getIntValue(CFG_STYLE) == CFG_STYLE_A)
        m_gl_ecam = new GLECAMWidgetStyleA(
            m_upper_ecam, config_widget_provider, main_config, m_ecam_config, m_fmc_control, this);
    else if (m_main_config->getIntValue(CFG_STYLE) == CFG_STYLE_B)
        m_gl_ecam = new GLECAMWidgetStyleB(
            m_upper_ecam, config_widget_provider, main_config, m_ecam_config, m_fmc_control, this);
    
#if !VASFMC_GAUGE
    // setup GUI

    QVBoxLayout* layout = new QVBoxLayout(this);
    MYASSERT(layout != 0);
    layout->setSpacing(0);
    layout->setMargin(0);
    if (m_gl_ecam != 0) layout->addWidget(m_gl_ecam);
    setLayout(layout);
#endif
    
#if !VASFMC_GAUGE
    if (m_ecam_config->getIntValue(CFG_ECAM_WINDOW_STATUS) == 0) hide(); else show();
#endif

    if (m_upper_ecam) m_config_widget_provider->registerConfigWidget("upper ECAM", m_ecam_config);
    else              m_config_widget_provider->registerConfigWidget("lower ECAM", m_ecam_config);
}

/////////////////////////////////////////////////////////////////////////////

FMCECAM::~FMCECAM() 
{
    if (m_upper_ecam)
        m_config_widget_provider->unregisterConfigWidget("upper ECAM");
    else
        m_config_widget_provider->unregisterConfigWidget("lower ECAM");

    saveWindowGeometry();
    m_ecam_config->saveToFile();
    delete m_gl_ecam;
    delete m_ecam_config;
}

/////////////////////////////////////////////////////////////////////////////

#if VASFMC_GAUGE
/* virtual */ void FMCECAM::paintBitmapToGauge(PELEMENT_STATIC_IMAGE pelement)
{
    if (m_gl_ecam != 0) m_gl_ecam->paintBitmapToGauge(pelement);
}
#endif

/////////////////////////////////////////////////////////////////////////////

void FMCECAM::slotRefresh() 
{
    QTime timer;
    timer.start();

    if (m_gl_ecam != 0) m_gl_ecam->refreshECAM(); 

    emit signalTimeUsed("EC", timer.elapsed());
}

/////////////////////////////////////////////////////////////////////////////

void FMCECAM::processFSControls()
{
    while (!m_fmc_control->flightStatus()->fsctrl_ecam_list.isEmpty())
    {
        switch(m_fmc_control->flightStatus()->fsctrl_ecam_list.first())
        {
            case(70): isVisible() ? hide() : show(); break;
            case(71): m_fmc_control->toggleKeepOnTop(); break;
#if !VASFMC_GAUGE
            case(72): activateWindow(); break;
#endif
        }

        m_fmc_control->flightStatus()->fsctrl_ecam_list.removeFirst();
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCECAM::keyPressEvent(QKeyEvent *event)
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

void FMCECAM::mouseMoveEvent(QMouseEvent *event)
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

void FMCECAM::wheelEvent(QWheelEvent* event)
{
    MYASSERT(event);
//     if (event->delta() < 0) setRange(m_ecam_config->getIntValue(CFG_SCROLL_MODE) ? false : true);
//     else                    setRange(m_ecam_config->getIntValue(CFG_SCROLL_MODE) ? true : false);
    event->accept();
}

/////////////////////////////////////////////////////////////////////////////

void FMCECAM::resizeEvent(QResizeEvent*)
{
    if (m_gl_ecam != 0) m_gl_ecam->reset();
}

/////////////////////////////////////////////////////////////////////////////

void FMCECAM::loadWindowGeometry()
{
    // setup window geometry

    if (m_ecam_config->contains(CFG_ECAM_POS_X) && m_ecam_config->contains(CFG_ECAM_POS_Y) &&
        m_ecam_config->contains(CFG_ECAM_WIDTH) && m_ecam_config->contains(CFG_ECAM_HEIGHT))
    {
        if (m_ecam_config->getIntValue(CFG_ECAM_WIDTH) > 10 && m_ecam_config->getIntValue(CFG_ECAM_HEIGHT) > 10)
            resize(m_ecam_config->getIntValue(CFG_ECAM_WIDTH), m_ecam_config->getIntValue(CFG_ECAM_HEIGHT));
        move(m_ecam_config->getIntValue(CFG_ECAM_POS_X), m_ecam_config->getIntValue(CFG_ECAM_POS_Y));
    }

}

/////////////////////////////////////////////////////////////////////////////

void FMCECAM::saveWindowGeometry()
{
#if !VASFMC_GAUGE
    // save window geometry

    m_ecam_config->setValue(CFG_ECAM_POS_X, QString().setNum(x()));
    m_ecam_config->setValue(CFG_ECAM_POS_Y, QString().setNum(y()));

    m_ecam_config->setValue(CFG_ECAM_WIDTH, QString().setNum(geometry().width()));
    m_ecam_config->setValue(CFG_ECAM_HEIGHT, QString().setNum(geometry().height()));

    m_ecam_config->setValue(CFG_ECAM_WINDOW_STATUS, (m_is_visible ? 1 : 0));
#endif
}

/////////////////////////////////////////////////////////////////////////////

void FMCECAM::setupDefaultConfig()
{
    saveWindowGeometry();
    m_ecam_config->setValue(CFG_ECAM_POS_X, 705);
    m_ecam_config->setValue(CFG_ECAM_POS_Y, 560);
}

/////////////////////////////////////////////////////////////////////////////

FMCECAMHandler::FMCECAMHandler(bool upper_ecam,
                               ConfigWidgetProvider* config_widget_provider, 
                               Config* main_config,
                               const QString& ecam_config_filename,
                               FMCControl* fmc_control) :
    m_upper_ecam(upper_ecam),
    m_config_widget_provider(config_widget_provider),
    m_main_config(main_config), m_ecam_config_filename(ecam_config_filename),
    m_fmc_control(fmc_control), m_ecam(0)
{
    MYASSERT(m_config_widget_provider != 0);
    if (m_upper_ecam) fmc_control->setUpperEcamHandler(this);
    slotRestartECAM();
}

/////////////////////////////////////////////////////////////////////////////

FMCECAM* FMCECAMHandler::createECAM()
{
#if !VASFMC_GAUGE
    if (m_fmc_control->doKeepOnTop())
        return new FMCECAM(m_upper_ecam, m_config_widget_provider,
                           m_main_config, m_ecam_config_filename,
                           m_fmc_control, 0, Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    else
#endif
        return new FMCECAM(m_upper_ecam, m_config_widget_provider,
                           m_main_config, m_ecam_config_filename,
                           m_fmc_control, 0, 0); //Qt::SubWindow);
}

/////////////////////////////////////////////////////////////////////////////
