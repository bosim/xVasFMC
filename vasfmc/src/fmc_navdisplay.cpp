
//////////////////////////////////////////////////////////////////////////////
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
#include <QMessageBox>
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

#include "fmc_navdisplay.h"
#include "fmc_navdisplay_defines.h"
#include "fmc_navdisplay_glwidget.h"
#include "fmc_navdisplay_style_a.h"
#include "fmc_navdisplay_style_b.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

/////////////////////////////////////////////////////////////////////////////

FMCNavdisplay::FMCNavdisplay(ConfigWidgetProvider* config_widget_provider,
                             Config* main_config,
                             const QString& navdisplay_config_filename,
                             const QString& tcas_config_filename,
                             FMCControl* fmc_control,
                             QWidget* parent, 
                             Qt::WFlags fl,
                             bool left_side)
    : VasWidget(parent, fl),
      m_config_widget_provider(config_widget_provider), 
      m_main_config(main_config), m_navdisplay_config(0), 
      m_fmc_control(fmc_control), m_gl_navdisp(0), m_left_side(left_side)
{
    MYASSERT(m_config_widget_provider != 0);
    MYASSERT(m_fmc_control != 0);
#if !VASFMC_GAUGE
    setupUi(this);
    m_left_side ? setWindowTitle("ND left") : setWindowTitle("ND right");
    setFocusPolicy(Qt::StrongFocus);
    setFocus();
#endif

    // setup config

    m_navdisplay_config = new Config(navdisplay_config_filename);
    MYASSERT(m_navdisplay_config != 0);
    m_tcas_config = new Config(tcas_config_filename);
    MYASSERT(m_tcas_config != 0);

    setupDefaultConfig();
    m_navdisplay_config->loadfromFile();
    m_tcas_config->loadfromFile();
    loadWindowGeometry();
    m_navdisplay_config->saveToFile();
    m_tcas_config->saveToFile();

    // setup GL widget

    m_gl_navdisp = new GLNavdisplayWidget(config_widget_provider, main_config, m_navdisplay_config, 
                                          m_tcas_config, m_fmc_control, this, m_left_side);
    MYASSERT(m_gl_navdisp != 0);

#if !VASFMC_GAUGE
    // setup GUI

    QVBoxLayout* layout = new QVBoxLayout(this);
    MYASSERT(layout != 0);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_gl_navdisp);
    setLayout(layout);
#endif
    
#if !VASFMC_GAUGE
    if (m_navdisplay_config->getIntValue(CFG_NAVWIN_WINDOW_STATUS) == 0) hide(); else show();
#endif

    m_config_widget_provider->registerConfigWidget("TCAS", m_tcas_config);
    m_config_widget_provider->registerConfigWidget("ND", m_navdisplay_config);
}

/////////////////////////////////////////////////////////////////////////////

FMCNavdisplay::~FMCNavdisplay() 
{
    m_config_widget_provider->unregisterConfigWidget("TCAS");
    m_config_widget_provider->unregisterConfigWidget("ND");
    saveWindowGeometry();
    m_navdisplay_config->saveToFile();
    m_tcas_config->saveToFile();
    delete m_gl_navdisp;
    delete m_navdisplay_config;
    delete m_tcas_config;
}

/////////////////////////////////////////////////////////////////////////////

#if VASFMC_GAUGE
void FMCNavdisplay::paintBitmapToGauge(PELEMENT_STATIC_IMAGE pelement)
{
    m_gl_navdisp->paintBitmapToGauge(pelement);
}
#endif

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplay::slotRefresh() 
{
    QTime timer;
    timer.start();

    m_fmc_control->recalcFlightstatus();
    m_gl_navdisp->refreshNavDisplay(); 

    emit signalTimeUsed("ND", timer.elapsed());
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplay::processFSControls()
{
    QList<char>& fs_ctrls_list = 
        m_left_side ? m_fmc_control->flightStatus()->fsctrl_nd_left_list : 
        m_fmc_control->flightStatus()->fsctrl_nd_right_list;

    while(!fs_ctrls_list.isEmpty())
    {
        switch(fs_ctrls_list.first())
        {
            case(1):  m_fmc_control->nextNDRange(m_left_side, true);  break;
            case(-1): m_fmc_control->nextNDRange(m_left_side, false); break;
            case(+2): m_fmc_control->nextNDMode(m_left_side, true);   break;
            case(-2): m_fmc_control->nextNDMode(m_left_side, false);  break;

            case(10): m_fmc_control->setNDRange(m_left_side, 0); break;
            case(11): m_fmc_control->setNDRange(m_left_side, 1); break;
            case(12): m_fmc_control->setNDRange(m_left_side, 2); break;
            case(13): m_fmc_control->setNDRange(m_left_side, 3); break;
            case(14): m_fmc_control->setNDRange(m_left_side, 4); break;
            case(15): m_fmc_control->setNDRange(m_left_side, 5); break;
            case(16): m_fmc_control->setNDRange(m_left_side, 6); break;
            case(17): m_fmc_control->setNDRange(m_left_side, 7); break;
            case(18): m_fmc_control->setNDRange(m_left_side, 8); break;

            case(20): m_fmc_control->slotEnableNDModeIlsRose(m_left_side);  break;
            case(21): m_fmc_control->slotEnableNDModeVorRose(m_left_side);  break;
            case(22): m_fmc_control->slotEnableNDModeNavRose(m_left_side);  break;
            case(23): m_fmc_control->slotEnableNDModeNavArc(m_left_side);   break;
            case(24): m_fmc_control->slotEnableNDModePlanRose(m_left_side); break;

            case(50): m_fmc_control->nextNDNavaidPointer(m_left_side, true); break;
            case(51): m_fmc_control->nextNDNavaidPointer(m_left_side, false); break;
            case(52): m_fmc_control->setNDNavaidPointer(m_left_side, true, CFG_ND_NAVAID_POINTER_TYPE_OFF); break;
            case(53): m_fmc_control->setNDNavaidPointer(m_left_side, true, CFG_ND_NAVAID_POINTER_TYPE_NDB); break;
            case(54): m_fmc_control->setNDNavaidPointer(m_left_side, true, CFG_ND_NAVAID_POINTER_TYPE_VOR); break;
            case(55): m_fmc_control->setNDNavaidPointer(m_left_side, false, CFG_ND_NAVAID_POINTER_TYPE_OFF); break;
            case(56): m_fmc_control->setNDNavaidPointer(m_left_side, false, CFG_ND_NAVAID_POINTER_TYPE_NDB); break;
            case(57): m_fmc_control->setNDNavaidPointer(m_left_side, false, CFG_ND_NAVAID_POINTER_TYPE_VOR); break;
                
            case(60): m_fmc_control->toggleSurroundingAirports(m_left_side); break;
            case(61): m_fmc_control->toggleSurroundingVORs(m_left_side); break;
            case(62): m_fmc_control->toggleSurroundingNDBs(m_left_side); break;
            case(63): m_fmc_control->toggleGeodata(); break;
            case(64): m_fmc_control->toggleTCAS(); break;
            case(65): m_fmc_control->slotToggleShowConstrains(m_left_side); break;

            case(70): isVisible() ? hide() : show(); break;
            case(71): m_fmc_control->toggleKeepOnTop(); break;
#if !VASFMC_GAUGE
            case(72): activateWindow(); break;
#endif

            case(80): m_fmc_control->slotShowSurroundingAirports(m_left_side); break;
            case(81): m_fmc_control->slotHideSurroundingAirports(m_left_side); break;
            case(82): m_fmc_control->slotShowSurroundingVors(m_left_side); break;
            case(83): m_fmc_control->slotHideSurroundingVors(m_left_side); break;
            case(84): m_fmc_control->slotShowSurroundingNdbs(m_left_side); break;
            case(85): m_fmc_control->slotHideSurroundingNdbs(m_left_side); break;
            case(86): if (!m_fmc_control->showGeoData()) m_fmc_control->toggleGeodata(); break;
            case(87): if (m_fmc_control->showGeoData()) m_fmc_control->toggleGeodata(); break;
            case(88): m_fmc_control->slotShowConstrains(m_left_side); break;
            case(89): m_fmc_control->slotHideConstrains(m_left_side); break;
        }

        fs_ctrls_list.removeFirst();
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplay::keyPressEvent(QKeyEvent *event)
{
    MYASSERT(event != 0);
    
    int inc = (event->modifiers() & Qt::ShiftModifier ? 10 : 1);
    bool do_resize = (event->modifiers() & Qt::ControlModifier ? 1 : 0);

    if ((event->modifiers() & Qt::AltModifier) != 0)
    {
        switch(event->key())
        {
            case(Qt::Key_Plus) : m_fmc_control->nextNDRange(m_left_side, true);  break;
            case(Qt::Key_Minus): m_fmc_control->nextNDRange(m_left_side, false); break;                                     

            case(Qt::Key_M): {

                if (event->modifiers() & Qt::ShiftModifier) m_fmc_control->nextNDMode(m_left_side, false);
                else m_fmc_control->nextNDMode(m_left_side, true);
                break;
            }

            case(Qt::Key_L): m_fmc_control->nextNDNavaidPointer(m_left_side, true); break;
            case(Qt::Key_R): m_fmc_control->nextNDNavaidPointer(m_left_side, false); break;
            case(Qt::Key_T): m_fmc_control->toggleTCAS(); break;
            case(Qt::Key_A): m_fmc_control->toggleSurroundingAirports(m_left_side); break;
            case(Qt::Key_V): m_fmc_control->toggleSurroundingVORs(m_left_side); break;
            case(Qt::Key_N): m_fmc_control->toggleSurroundingNDBs(m_left_side); break;
            case(Qt::Key_D): m_fmc_control->toggleGeodata(); break;

            case(Qt::Key_W): {
                if (m_main_config->getIntValue(CFG_STYLE) == CFG_STYLE_B) toggleWindCorrection();
                break;
            }

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

    slotRefresh();
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplay::mouseMoveEvent(QMouseEvent *event)
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

void FMCNavdisplay::wheelEvent(QWheelEvent* event)
{
    MYASSERT(event);
    if (event->delta() < 0) m_fmc_control->nextNDRange(m_left_side, m_navdisplay_config->getIntValue(CFG_SCROLL_MODE) ? false : true);
    else                    m_fmc_control->nextNDRange(m_left_side, m_navdisplay_config->getIntValue(CFG_SCROLL_MODE) ? true : false);
    event->accept();

    slotRefresh();
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplay::resizeEvent(QResizeEvent*)
{
    m_gl_navdisp->reset();
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplay::loadWindowGeometry()
{
    // setup window geometry

    if (m_navdisplay_config->contains(CFG_NAVWIN_POS_X) && m_navdisplay_config->contains(CFG_NAVWIN_POS_Y) &&
        m_navdisplay_config->contains(CFG_NAVWIN_WIDTH) && m_navdisplay_config->contains(CFG_NAVWIN_HEIGHT))
    {
//         Logger::log(QString("FMCNavdisplay:loadWindowGeometry: resizing to %1/%2").
//                     arg(m_navdisplay_config->getIntValue(CFG_NAVWIN_WIDTH)).
//                     arg(m_navdisplay_config->getIntValue(CFG_NAVWIN_HEIGHT)));

        resize(m_navdisplay_config->getIntValue(CFG_NAVWIN_WIDTH), m_navdisplay_config->getIntValue(CFG_NAVWIN_HEIGHT));
        move(m_navdisplay_config->getIntValue(CFG_NAVWIN_POS_X), m_navdisplay_config->getIntValue(CFG_NAVWIN_POS_Y));
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplay::saveWindowGeometry()
{
#if !VASFMC_GAUGE
    // save window geometry
    m_navdisplay_config->setValue(CFG_NAVWIN_POS_X, QString().setNum(x()));
    m_navdisplay_config->setValue(CFG_NAVWIN_POS_Y, QString().setNum(y()));
    m_navdisplay_config->setValue(CFG_NAVWIN_WIDTH, QString().setNum(geometry().width()));
    m_navdisplay_config->setValue(CFG_NAVWIN_HEIGHT, QString().setNum(geometry().height()));
    m_navdisplay_config->setValue(CFG_NAVWIN_WINDOW_STATUS, (m_is_visible ? 1 : 0));
#endif
}

/////////////////////////////////////////////////////////////////////////////

void FMCNavdisplay::setupDefaultConfig()
{
    saveWindowGeometry();

    if ( m_left_side ) {
        m_navdisplay_config->setValue(CFG_NAVWIN_POS_X, 375);
    } else {
        m_navdisplay_config->setValue(CFG_NAVWIN_POS_X, 750);
    }
    m_navdisplay_config->setValue(CFG_NAVWIN_POS_Y, 560);

    m_navdisplay_config->setValue(CFG_DRAW_RANGE_RINGS, 1);
    m_navdisplay_config->setValue(CFG_WIND_CORRECTION, 0);
    m_navdisplay_config->setValue(CFG_ROUTE_DISPLAY_MAX_DISTANCE_NM, 1000);
    m_navdisplay_config->setValue(CFG_SCROLL_MODE, SCROLL_MODE_REGULAR);
    m_navdisplay_config->setValue(CFG_GEO_DATA_COLOR, "#000AFF");

	m_tcas_config->setValue(CFG_TCAS_MAX_FL_DIFF, 100);
	m_tcas_config->setValue(CFG_TCAS_MIN_OTHER_SPEED, 50);
    m_tcas_config->setValue(CFG_TCAS_MIN_OWN_SPEED, 50); 
    m_tcas_config->setValue(CFG_TCAS_ALERT_DIST, 3);
    m_tcas_config->setValue(CFG_TCAS_ALERT_FL_DIFF, 9);
    m_tcas_config->setValue(CFG_TCAS_HINT_DIST, 5);
    m_tcas_config->setValue(CFG_TCAS_HINT_FL_DIFF, 14);
    m_tcas_config->setValue(CFG_TCAS_FULL_DIST, 10);
    m_tcas_config->setValue(CFG_TCAS_FULL_FL_DIFF, 19);
    m_tcas_config->setValue(CFG_TCAS_MIN_VS_CLIMB_DESCENT_DETECTION, 100);
    m_tcas_config->setValue(CFG_TCAS_NORMAL_COL, "white");
    m_tcas_config->setValue(CFG_TCAS_HINT_COL, "yellow");
    m_tcas_config->setValue(CFG_TCAS_ALERT_COL, "red");
    m_tcas_config->setValue(CFG_TCAS_FULL_COL, "white");
    m_tcas_config->setValue(CFG_TCAS_DATA_COL, "white");
}

/////////////////////////////////////////////////////////////////////////////

FMCNavdisplayHandler::FMCNavdisplayHandler(ConfigWidgetProvider* config_widget_provider, 
                                           Config* main_config,
                                           const QString& navdisplay_config_filename,
                                           const QString& tcas_config_filename,
                                           FMCControl* fmc_control,
                                           bool left_side) :
    m_config_widget_provider(config_widget_provider),
    m_main_config(main_config), m_navdisplay_config_filename(navdisplay_config_filename),
    m_tcas_config_filename(tcas_config_filename), m_fmc_control(fmc_control), m_navdisplay(0),
        m_left_side(left_side)
{
    MYASSERT(m_config_widget_provider != 0);
    (m_left_side) ? fmc_control->setNDLeftHandler(this) : fmc_control->setNDRightHandler(this);
    
    slotRestartNavdisplay();
}

/////////////////////////////////////////////////////////////////////////////

FMCNavdisplay* FMCNavdisplayHandler::createNavdisplay()
{
#if !VASFMC_GAUGE
    if (m_fmc_control->doKeepOnTop())
        return new FMCNavdisplay(m_config_widget_provider,
                                 m_main_config, m_navdisplay_config_filename, m_tcas_config_filename, 
                                 m_fmc_control, 0, Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint, m_left_side);
    else
#endif
        return new FMCNavdisplay(m_config_widget_provider,
                                 m_main_config, m_navdisplay_config_filename, m_tcas_config_filename, 
                                 m_fmc_control, 0, 0, m_left_side);
}

