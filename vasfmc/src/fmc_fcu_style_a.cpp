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

/*! \file    fmc_fcu_style_a.cpp
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
#include "fmc_autopilot.h"
#include "fmc_autothrottle.h"

#include "fmc_fcu_defines.h"
#include "fmc_fcu_style_a.h"

#define HSPACE 8
#define VSPACE 5

/////////////////////////////////////////////////////////////////////////////

FMCFCUStyleA::FMCFCUStyleA(ConfigWidgetProvider* config_widget_provider,
                           Config* main_config,
                           const QString& fcu_config_filename,
                           FMCControl* fmc_control,
                           QWidget* parent,
                           Qt::WFlags fl) :
    FMCFCUStyleBase("A", config_widget_provider, main_config, fcu_config_filename, fmc_control, parent, fl),
    m_alt_set_mode(ALT_SET_MODE_100FT), 
    m_last_range_left(-1), m_last_range_right(-1), m_last_mode_left(-1), m_last_mode_right(-1)    
{
    MYASSERT(fmc_control != 0);
#if !VASFMC_GAUGE
    setupUi(this);
    setWindowTitle("FCU");
    setFocusPolicy(Qt::StrongFocus);
    setFocus();
    display->installEventFilter(this);
    setMouseTracking(true);
    display->setMouseTracking(true);
#endif

    // setup background image

#if VASFMC_GAUGE
    m_background_pixmap = QPixmap(VasPath::prependPath(m_fcu_config->getValue(CFG_FCU_BACKGROUND_IMAGE_GAUGE)));
#else
    if (m_fmc_control->fcuLeftOnlyMode())
        m_background_pixmap = QPixmap(VasPath::prependPath(m_fcu_config->getValue(CFG_FCU_BACKGROUND_IMAGE_GAUGE)));
    else
        m_background_pixmap = QPixmap(VasPath::prependPath(m_fcu_config->getValue(CFG_FCU_BACKGROUND_IMAGE)));

#endif

    MYASSERT(!m_background_pixmap.isNull());

#if !VASFMC_GAUGE
    resize(m_background_pixmap.width(), m_background_pixmap.height());
    setMinimumSize(QSize(m_background_pixmap.width(), m_background_pixmap.height()));
    setMaximumSize(QSize(m_background_pixmap.width(), m_background_pixmap.height()));
    display->resize(QSize(m_background_pixmap.width(), m_background_pixmap.height()));
    resetBackgroundImage();
#else
    m_gauge_background_width = m_background_pixmap.width();
    m_gauge_background_height = m_background_pixmap.height();
#endif

    // setup small images

    m_range10_image = new QPixmap(VasPath::prependPath(m_fcu_config->getValue(CFG_FCU_RANGE10_IMAGE)));
    m_range20_image = new QPixmap(VasPath::prependPath(m_fcu_config->getValue(CFG_FCU_RANGE20_IMAGE)));
    m_range40_image = new QPixmap(VasPath::prependPath(m_fcu_config->getValue(CFG_FCU_RANGE40_IMAGE)));
    m_range80_image = new QPixmap(VasPath::prependPath(m_fcu_config->getValue(CFG_FCU_RANGE80_IMAGE)));
    m_range160_image = new QPixmap(VasPath::prependPath(m_fcu_config->getValue(CFG_FCU_RANGE160_IMAGE)));
    m_range320_image = new QPixmap(VasPath::prependPath(m_fcu_config->getValue(CFG_FCU_RANGE320_IMAGE)));

    m_mode_ils_image = new QPixmap(VasPath::prependPath(m_fcu_config->getValue(CFG_FCU_MODE_ILS_IMAGE)));
    m_mode_vor_image = new QPixmap(VasPath::prependPath(m_fcu_config->getValue(CFG_FCU_MODE_VOR_IMAGE)));
    m_mode_rose_image = new QPixmap(VasPath::prependPath(m_fcu_config->getValue(CFG_FCU_MODE_ROSE_IMAGE)));
    m_mode_arc_image = new QPixmap(VasPath::prependPath(m_fcu_config->getValue(CFG_FCU_MODE_ARC_IMAGE)));
    m_mode_plan_image = new QPixmap(VasPath::prependPath(m_fcu_config->getValue(CFG_FCU_MODE_PLAN_IMAGE)));

    // setup input area

    MYASSERT(connect(&m_input_area, SIGNAL(signalMouseClick(const QString&, QMouseEvent*)),
                     this, SLOT(slotInputMouseButton(const QString&, QMouseEvent*))));
    MYASSERT(connect(&m_input_area, SIGNAL(signalMouseWheel(const QString&, QWheelEvent*)),
                     this, SLOT(slotInputMouseWheel(const QString&, QWheelEvent*))));

    if (m_input_area.loadFromFile(m_fcu_config->getValue(CFG_FCU_INPUT_AREA_FILE)))
    {
        Logger::log(QString("FMCFCUStyleA: loaded input areas from %1").
                    arg(m_fcu_config->getValue(CFG_FCU_INPUT_AREA_FILE)));
    }
    else
    {
        InputAreaList input_area_list;

        //----- mitchell FCU values

        input_area_list.append(InputArea(FMC_FCU_INPUT_ALT_SET_HPA_LEFT, 86,55,20,14));
        input_area_list.append(InputArea(FMC_FCU_INPUT_ALT_SET_INHG_LEFT, 14,55,28,14));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_LEFT_ALT_SET, 52,71,24,24));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_LEFT_ALT_SET, 52,71,24,24, 0, 0, Qt::RightButton));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_LEFT_ALT_SET_LEFT, 25,71,23,24));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_LEFT_ALT_SET_LEFT, 25,71,23,24, 0, 0, Qt::RightButton));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_LEFT_ALT_SET_RIGHT , 81,71,23,24));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_LEFT_ALT_SET_RIGHT , 81,71,23,24, 0, 0, Qt::RightButton));

        input_area_list.append(InputArea(FMC_FCU_INPUT_ALT_SET_METRIC, 867,62,26,26));
        input_area_list.append(InputArea(FMC_FCU_INPUT_FLIGHTPATH_MODE, 685,62,26,26));

        input_area_list.append(InputArea(FMC_FCU_INPUT_ND_LEFT_CSTR, 125,13,40,22));
        input_area_list.append(InputArea(FMC_FCU_INPUT_ND_LEFT_TERR, 171,13,40,22));
        input_area_list.append(InputArea(FMC_FCU_INPUT_ND_LEFT_VOR, 217,13,40,22));
        input_area_list.append(InputArea(FMC_FCU_INPUT_ND_LEFT_NDB, 263,13,40,22));
        input_area_list.append(InputArea(FMC_FCU_INPUT_ND_LEFT_AIRPORT, 309,13,40,22));

        input_area_list.append(InputArea(FMC_FCU_INPUT_ND_LEFT_POINTER_LEFT_OFF, 167,149,20,16));
        input_area_list.append(InputArea(FMC_FCU_INPUT_ND_LEFT_POINTER_LEFT_NDB, 141,129,20,16));
        input_area_list.append(InputArea(FMC_FCU_INPUT_ND_LEFT_POINTER_LEFT_VOR, 195,129,20,16));
        input_area_list.append(InputArea(FMC_FCU_INPUT_ND_LEFT_POINTER_RIGHT_OFF,293,149,22,16));
        input_area_list.append(InputArea(FMC_FCU_INPUT_ND_LEFT_POINTER_RIGHT_NDB,267,129,20,16));
        input_area_list.append(InputArea(FMC_FCU_INPUT_ND_LEFT_POINTER_RIGHT_VOR,321,129,20,16));

        input_area_list.append(InputArea(FMC_FCU_INPUT_LEFT_FD_ENABLE,   23,133,42,24));
        input_area_list.append(InputArea(FMC_FCU_INPUT_LEFT_ILS_ENABLE,  71,133,42,24));
        input_area_list.append(InputArea(FMC_FCU_INPUT_RIGHT_FD_ENABLE,  1309,133,42,24));
        input_area_list.append(InputArea(FMC_FCU_INPUT_RIGHT_ILS_ENABLE, 1258,133,42,24));

        input_area_list.append(InputArea(FMC_FCU_INPUT_ATHROTTLE_ARM,     679,131,42,26));
        input_area_list.append(InputArea(FMC_FCU_INPUT_ATHROTTLE_SPDMACH, 393,63,24,24));
        input_area_list.append(InputArea(FMC_FCU_INPUT_ATHROTTLE_N1,      515,144,22,22));

        input_area_list.append(InputArea(FMC_FCU_INPUT_AP1_ARM, 645,97,40,26));
        input_area_list.append(InputArea(FMC_FCU_INPUT_AP2_ARM, 713,97,40,26));

        input_area_list.append(InputArea(FMC_FCU_INPUT_LOC_ARM, 567,129,42,24));
        input_area_list.append(InputArea(FMC_FCU_INPUT_APP_ARM, 931,129,42,24));

        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_ND_LEFT_RANGE_LEFT, 279,63,52,52));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_ND_LEFT_RANGE_RIGHT,279,63,52,52, 0, 0, Qt::RightButton));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_ND_LEFT_RANGE_LEFT, 256,63,21,52));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_ND_LEFT_RANGE_RIGHT, 333,63,21,52));

        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_ND_LEFT_MODE_LEFT,  151,63,52,52));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_ND_LEFT_MODE_RIGHT, 151,63,52,52, 0, 0, Qt::RightButton));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_ND_LEFT_MODE_LEFT,  127,63,21,52));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_ND_LEFT_MODE_RIGHT, 206,63,21,52));

        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_SPD,        445,73,32,32));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_SPD,        445,73,32,32, 0, 0, Qt::RightButton));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_SPD_LEFT,   422,73,19,32));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_SPD_LEFT,   422,73,19,32, 0, 0, Qt::RightButton));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_SPD_RIGHT,  482,73,19,32));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_SPD_RIGHT,  482,73,19,32, 0, 0, Qt::RightButton));

        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_HDG,        570,73,32,32));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_HDG,        570,73,32,32, 0, 0, Qt::RightButton));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_HDG_LEFT,   546,73,19,32));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_HDG_LEFT,   546,73,19,32, 0, 0, Qt::RightButton));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_HDG_RIGHT,  608,73,19,32));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_HDG_RIGHT,  608,73,19,32, 0, 0, Qt::RightButton));

        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_ALT,        796,73,32,32));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_ALT,        796,73,32,32, 0, 0, Qt::RightButton));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_ALT_LEFT,   772,73,19,32));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_ALT_LEFT,   772,73,19,32, 0, 0, Qt::RightButton));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_ALT_RIGHT,  834,73,19,32));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_ALT_RIGHT,  834,73,19,32, 0, 0, Qt::RightButton));

        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_VS,         932,73,32,32));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_VS,         932,73,32,32, 0, 0, Qt::RightButton));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_VS_LEFT,    908,73,19,32));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_VS_LEFT,    908,73,19,32, 0, 0, Qt::RightButton));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_VS_RIGHT,   970,73,19,32));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_VS_RIGHT,   970,73,19,32, 0, 0, Qt::RightButton));

        // right pilot side

        input_area_list.append(InputArea(FMC_FCU_INPUT_ALT_SET_HPA_RIGHT, 1331,55,20,14));
        input_area_list.append(InputArea(FMC_FCU_INPUT_ALT_SET_INHG_RIGHT, 1259,55,28,14));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_RIGHT_ALT_SET, 1297,71,24,24));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_RIGHT_ALT_SET, 1297,71,24,24, 0, 0, Qt::RightButton));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_RIGHT_ALT_SET_LEFT, 1270,71,24,24));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_RIGHT_ALT_SET_LEFT, 1270,71,24,24, 0, 0, Qt::RightButton));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_RIGHT_ALT_SET_RIGHT , 1323,71,24,24));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_RIGHT_ALT_SET_RIGHT , 1323,71,24,24, 0, 0, Qt::RightButton));
        
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_ALT_SET_100FT, 777,53,20,16));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_ALT_SET_1000FT, 826,53,24,16));

        input_area_list.append(InputArea(FMC_FCU_INPUT_ND_RIGHT_CSTR, 1207,13,40,22));
        input_area_list.append(InputArea(FMC_FCU_INPUT_ND_RIGHT_TERR, 1161,13,40,22));
        input_area_list.append(InputArea(FMC_FCU_INPUT_ND_RIGHT_VOR, 1115,13,40,22));
        input_area_list.append(InputArea(FMC_FCU_INPUT_ND_RIGHT_NDB, 1069,13,40,22));
        input_area_list.append(InputArea(FMC_FCU_INPUT_ND_RIGHT_AIRPORT, 1023,13,40,22));

        input_area_list.append(InputArea(FMC_FCU_INPUT_ND_RIGHT_POINTER_LEFT_OFF, 1065,149,20,16));
        input_area_list.append(InputArea(FMC_FCU_INPUT_ND_RIGHT_POINTER_LEFT_NDB, 1039,129,20,16));
        input_area_list.append(InputArea(FMC_FCU_INPUT_ND_RIGHT_POINTER_LEFT_VOR, 1093,129,20,16));
        input_area_list.append(InputArea(FMC_FCU_INPUT_ND_RIGHT_POINTER_RIGHT_OFF,1191,149,22,16));
        input_area_list.append(InputArea(FMC_FCU_INPUT_ND_RIGHT_POINTER_RIGHT_NDB,1165,129,20,16));
        input_area_list.append(InputArea(FMC_FCU_INPUT_ND_RIGHT_POINTER_RIGHT_VOR,1219,129,20,16));

        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_ND_RIGHT_RANGE_LEFT, 1176,63,52,52));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_ND_RIGHT_RANGE_RIGHT,1176,63,52,52, 0, 0, Qt::RightButton));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_ND_RIGHT_RANGE_LEFT, 1152,63,21,52));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_ND_RIGHT_RANGE_RIGHT, 1230,63,21,52));

        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_ND_RIGHT_MODE_LEFT,  1048,63,52,52));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_ND_RIGHT_MODE_RIGHT, 1048,63,52,52, 0, 0, Qt::RightButton));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_ND_RIGHT_MODE_LEFT,  1024,63,21,52));
        input_area_list.append(InputArea(FMC_FCU_INPUT_KNOB_ND_RIGHT_MODE_RIGHT, 1103,63,21,52));

        input_area_list.append(InputArea(FMC_FCU_INPUT_CDU_DISPLAY_ONLY, 880, 145, 24, 21));
        input_area_list.append(InputArea(FMC_FCU_INPUT_NEXT_CHECKLIST_ITEM, 379, 134, 30, 30));

        m_input_area.clear();
        m_input_area.setInputAreaList(input_area_list);

        if (!m_input_area.saveToFile(m_fcu_config->getValue(CFG_FCU_INPUT_AREA_FILE)))
            Logger::log("FMCFCUStyleA: could not save defauls FCU input areas");
        else
            Logger::log(QString("FMCFCUStyleA: saved default FCU input areas to (%1)").
                        arg(m_fcu_config->getValue(CFG_FCU_INPUT_AREA_FILE)));
    }

#if !VASFMC_GAUGE
    if (m_fcu_config->getIntValue(CFG_FCU_WINDOW_STATUS) == 0) hide(); else show();
#endif
}

/////////////////////////////////////////////////////////////////////////////

FMCFCUStyleA::~FMCFCUStyleA()
{
    delete m_range10_image;
    delete m_range20_image;
    delete m_range40_image;
    delete m_range80_image;
    delete m_range160_image;
    delete m_range320_image;
}

/////////////////////////////////////////////////////////////////////////////

void FMCFCUStyleA::slotRefresh()
{ 
    QTime timer;
    timer.start();

#if VASFMC_GAUGE
    paintToBitmap();
#else
    display->update();
#endif

    emit signalTimeUsed("FC", timer.elapsed());
}

/////////////////////////////////////////////////////////////////////////////

#if VASFMC_GAUGE
void FMCFCUStyleA::paintGauge(QPainter *pPainter)
{
    QRect rectWindow = pPainter->window();

    // Draw background image
    if(!m_background_pixmap.isNull())
    {
        // Scale background image if necessary
        if (m_background_pixmap_scaled.isNull() ||
            m_background_pixmap_scaled.width() != rectWindow.width() ||
            m_background_pixmap_scaled.height() != rectWindow.height())
        {
            m_background_pixmap_scaled = m_background_pixmap.scaled(
                rectWindow.width(), rectWindow.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }

        // Draw background image
        pPainter->drawPixmap(0, 0, m_background_pixmap_scaled);
    }

    // Draw display into pixmap first
    pPainter->setWindow(0, 0, m_background_pixmap.width(), m_background_pixmap.height());
    pPainter->setViewport(0, 0, m_background_pixmap.width(), m_background_pixmap.height());
    paintMe(*pPainter);
}
#endif

/////////////////////////////////////////////////////////////////////////////

void FMCFCUStyleA::slotInputMouseButton(const QString& text, QMouseEvent* event)
{
    if(event->modifiers() == Qt::MetaModifier) {
        QMouseEvent fake_mouse_event(
                QEvent::MouseButtonPress, event->pos(), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
        slotInputMouseButton(text, &fake_mouse_event);
        return;
    }

    const FlightStatus* flightstatus = m_fmc_control->flightStatus();

    if      (text == FMC_FCU_INPUT_CDU_DISPLAY_ONLY) m_fmc_control->slotToggleCduDisplayOnlyMode(true); 
    else if (text == FMC_FCU_INPUT_NEXT_CHECKLIST_ITEM) m_fmc_control->slotNextChecklistItem(); 
    else if (text == FMC_FCU_INPUT_ALT_SET_HPA_LEFT)    m_fmc_control->slotShowAltimterHPA(true);
    else if (text == FMC_FCU_INPUT_ALT_SET_INHG_LEFT)   m_fmc_control->slotShowAltimterInches(true);
    else if (text == FMC_FCU_INPUT_ALT_SET_HPA_RIGHT)   m_fmc_control->slotShowAltimterHPA(false);
    else if (text == FMC_FCU_INPUT_ALT_SET_INHG_RIGHT)  m_fmc_control->slotShowAltimterInches(false);
    else if (text == FMC_FCU_INPUT_ALT_SET_METRIC) m_fmc_control->slotToggleMetricAlt();
    else if (text == FMC_FCU_INPUT_FLIGHTPATH_MODE) m_fmc_control->fmcAutoPilot().slotToggleFlightPathMode();
    else if (text == FMC_FCU_INPUT_ND_LEFT_AIRPORT) m_fmc_control->toggleSurroundingAirports(true);
    else if (text == FMC_FCU_INPUT_ND_LEFT_NDB) m_fmc_control->toggleSurroundingNDBs(true);
    else if (text == FMC_FCU_INPUT_ND_LEFT_VOR) m_fmc_control->toggleSurroundingVORs(true);
    else if (text == FMC_FCU_INPUT_ND_LEFT_TERR) m_fmc_control->toggleGeodata();
    else if (text == FMC_FCU_INPUT_ND_LEFT_CSTR) m_fmc_control->slotToggleShowConstrains(true);
    else if (text == FMC_FCU_INPUT_ND_RIGHT_AIRPORT) m_fmc_control->toggleSurroundingAirports(false);
    else if (text == FMC_FCU_INPUT_ND_RIGHT_NDB) m_fmc_control->toggleSurroundingNDBs(false);
    else if (text == FMC_FCU_INPUT_ND_RIGHT_VOR) m_fmc_control->toggleSurroundingVORs(false);
    else if (text == FMC_FCU_INPUT_ND_RIGHT_TERR) m_fmc_control->toggleGeodata();
    else if (text == FMC_FCU_INPUT_ND_RIGHT_CSTR) m_fmc_control->slotToggleShowConstrains(false);
    else if (text == FMC_FCU_INPUT_ND_LEFT_POINTER_LEFT_OFF) m_fmc_control->slotNDNoLeftPointerAction(true);
    else if (text == FMC_FCU_INPUT_ND_LEFT_POINTER_LEFT_NDB) m_fmc_control->slotNDAdf1PointerAction(true);
    else if (text == FMC_FCU_INPUT_ND_LEFT_POINTER_LEFT_VOR) m_fmc_control->slotNDVor1PointerAction(true);
    else if (text == FMC_FCU_INPUT_ND_LEFT_POINTER_RIGHT_OFF) m_fmc_control->slotNDNoRightPointerAction(true);
    else if (text == FMC_FCU_INPUT_ND_LEFT_POINTER_RIGHT_NDB) m_fmc_control->slotNDAdf2PointerAction(true);
    else if (text == FMC_FCU_INPUT_ND_LEFT_POINTER_RIGHT_VOR) m_fmc_control->slotNDVor2PointerAction(true);
    else if (text == FMC_FCU_INPUT_ND_RIGHT_POINTER_LEFT_OFF) m_fmc_control->slotNDNoLeftPointerAction(false);
    else if (text == FMC_FCU_INPUT_ND_RIGHT_POINTER_LEFT_NDB) m_fmc_control->slotNDAdf1PointerAction(false);
    else if (text == FMC_FCU_INPUT_ND_RIGHT_POINTER_LEFT_VOR) m_fmc_control->slotNDVor1PointerAction(false);
    else if (text == FMC_FCU_INPUT_ND_RIGHT_POINTER_RIGHT_OFF) m_fmc_control->slotNDNoRightPointerAction(false);
    else if (text == FMC_FCU_INPUT_ND_RIGHT_POINTER_RIGHT_NDB) m_fmc_control->slotNDAdf2PointerAction(false);
    else if (text == FMC_FCU_INPUT_ND_RIGHT_POINTER_RIGHT_VOR) m_fmc_control->slotNDVor2PointerAction(false);
    else if (text == FMC_FCU_INPUT_KNOB_ND_LEFT_RANGE_LEFT)  m_fmc_control->nextNDRange(true, false);
    else if (text == FMC_FCU_INPUT_KNOB_ND_LEFT_RANGE_RIGHT) m_fmc_control->nextNDRange(true, true);
    else if (text == FMC_FCU_INPUT_KNOB_ND_LEFT_MODE_LEFT) m_fmc_control->nextNDMode(true, false);
    else if (text == FMC_FCU_INPUT_KNOB_ND_LEFT_MODE_RIGHT) m_fmc_control->nextNDMode(true, true);
    else if (text == FMC_FCU_INPUT_KNOB_ND_RIGHT_RANGE_LEFT)  m_fmc_control->nextNDRange(false, false);
    else if (text == FMC_FCU_INPUT_KNOB_ND_RIGHT_RANGE_RIGHT) m_fmc_control->nextNDRange(false, true);
    else if (text == FMC_FCU_INPUT_KNOB_ND_RIGHT_MODE_LEFT) m_fmc_control->nextNDMode(false, false);
    else if (text == FMC_FCU_INPUT_KNOB_ND_RIGHT_MODE_RIGHT) m_fmc_control->nextNDMode(false, true);
    else if (text == FMC_FCU_INPUT_LEFT_ILS_ENABLE)
    {
        m_fmc_control->slotToggleShowPFDILS(true);
    }
    else if (text == FMC_FCU_INPUT_RIGHT_ILS_ENABLE)
    {
        m_fmc_control->slotToggleShowPFDILS(false);
    }
    else if (text == FMC_FCU_INPUT_LEFT_FD_ENABLE || text == FMC_FCU_INPUT_RIGHT_FD_ENABLE)
    {
        m_fmc_control->fmcAutoPilot().slotToggleFlightDirector();
    }
    else if (text == FMC_FCU_INPUT_ATHROTTLE_ARM)
    {
        m_fmc_control->fmcAutothrottle().slotToggleAutothrottleArm();
    }
    else if (text == FMC_FCU_INPUT_ATHROTTLE_SPDMACH)
    {
        m_fmc_control->fmcAutothrottle().slotToggleAutothrottleSpeedMach();
    }
    else if (text == FMC_FCU_INPUT_ATHROTTLE_N1)
    {
        m_fmc_control->fmcAutothrottle().engageAPThrottleN1Hold();
    }
    else if (text == FMC_FCU_INPUT_AP1_ARM)
    {
        m_fmc_control->fmcAutoPilot().slotToggleAutopilot();
    }
    else if (text == FMC_FCU_INPUT_AP2_ARM)
    {
        m_fmc_control->fmcAutoPilot().slotToggleAutopilot();
    }
    else if (text == FMC_FCU_INPUT_APP_ARM)
    {
        m_fmc_control->fmcAutoPilot().setAPPHold();
    }
    else if (text == FMC_FCU_INPUT_LOC_ARM)
    {
        m_fmc_control->fmcAutoPilot().setLOCHold();
    }

    else if (text == FMC_FCU_INPUT_KNOB_SPD && event != 0)
    {
    	if (event->button() == Qt::LeftButton){
            //TODO managed speed
            m_fmc_control->fmcAutothrottle().engageAPThrottle();
        }

        else if (event->button() == Qt::RightButton)
        {
            m_fmc_control->fmcAutothrottle().engageAPThrottle();
        }
    }

    else if (text == FMC_FCU_INPUT_KNOB_SPD_LEFT && event != 0)
    {
        if (event->button() == Qt::LeftButton)
        {
            if (m_fmc_control->fmcAutothrottle().isAPThrottleModeSpeedSet())
                m_fmc_control->fsAccess().setAPAirspeed(qMin(qMax(flightstatus->APSpd()-1, 100), 399));
            else
                m_fmc_control->fsAccess().setAPMach(qMin(qMax(flightstatus->APMach()-0.01, 0.1), 0.99));
        }
        else if (event->button() == Qt::RightButton)
        {
            if (m_fmc_control->fmcAutothrottle().isAPThrottleModeSpeedSet())
                m_fmc_control->fsAccess().setAPAirspeed(qMin(qMax(flightstatus->APSpd()-10, 100), 399));
            else
                m_fmc_control->fsAccess().setAPMach(qMin(qMax(flightstatus->APMach()-0.1, 0.1), 0.99));
        }
    }
    else if (text == FMC_FCU_INPUT_KNOB_SPD_RIGHT && event != 0)
    {
        if (event->button() == Qt::LeftButton)
        {
            if (m_fmc_control->fmcAutothrottle().isAPThrottleModeSpeedSet())
                m_fmc_control->fsAccess().setAPAirspeed(qMax(100, qMin(flightstatus->APSpd()+1, 399)));
            else
                m_fmc_control->fsAccess().setAPMach(qMin(qMax(flightstatus->APMach()+0.01, 0.1), 0.99));
        }
        else if (event->button() == Qt::RightButton)
        {
            if (m_fmc_control->fmcAutothrottle().isAPThrottleModeSpeedSet())
                m_fmc_control->fsAccess().setAPAirspeed(qMax(100, qMin(flightstatus->APSpd()+10, 399)));
            else
                m_fmc_control->fsAccess().setAPMach(qMin(qMax(flightstatus->APMach()+0.1, 0.1), 0.99));
        }
    }

    else if (text == FMC_FCU_INPUT_KNOB_HDG && event != 0)
    {
    	if (event->button() == Qt::LeftButton){
            m_fmc_control->fmcAutoPilot().setNAVHold();
        }

        else if (event->button() == Qt::RightButton)
        {
            m_fmc_control->fmcAutoPilot().setHeadingHold(true);
        }
    }

    else if (text == FMC_FCU_INPUT_KNOB_HDG_LEFT && event != 0)
    {
        if (!m_fmc_control->fmcAutoPilot().isNAVCoupled())
        {
            if (event->button() == Qt::LeftButton)
                m_fmc_control->fsAccess().setAPHeading(Navcalc::trimHeading(((int)flightstatus->APHdg())-1));
            else if (event->button() == Qt::RightButton)
                m_fmc_control->fsAccess().setAPHeading(Navcalc::trimHeading(((int)flightstatus->APHdg())-10));
        }
    }
    else if (text == FMC_FCU_INPUT_KNOB_HDG_RIGHT && event != 0)
    {
        if (!m_fmc_control->fmcAutoPilot().isNAVCoupled())
        {
            if (event->button() == Qt::LeftButton)
                m_fmc_control->fsAccess().setAPHeading(Navcalc::trimHeading(((int)flightstatus->APHdg())+1));
            else if (event->button() == Qt::RightButton)
                m_fmc_control->fsAccess().setAPHeading(Navcalc::trimHeading(((int)flightstatus->APHdg())+10));
        }
    }
    else if (text == FMC_FCU_INPUT_KNOB_ALT && event != 0)
    {
    	if (event->button() == Qt::LeftButton){
            //TODO managed alt
            m_fmc_control->fmcAutoPilot().setFLChangeMode(true, false);
        }

        else if (event->button() == Qt::RightButton)
        {
            m_fmc_control->fmcAutoPilot().setFLChangeMode(true, false);
        }
    }

    else if (text == FMC_FCU_INPUT_KNOB_ALT_LEFT && event != 0)
    {
        if (event->button() == Qt::LeftButton && m_alt_set_mode == ALT_SET_MODE_100FT)
            m_fmc_control->fsAccess().setAPAlt(qMax(flightstatus->APAlt() - 100, 0));
        else if (event->button() == Qt::RightButton || m_alt_set_mode == ALT_SET_MODE_1000FT)
            m_fmc_control->fsAccess().setAPAlt(qMax(flightstatus->APAlt() - 1000, 0));
    }
    else if (text == FMC_FCU_INPUT_KNOB_ALT_RIGHT && event != 0)
    {
        if (event->button() == Qt::LeftButton && m_alt_set_mode == ALT_SET_MODE_100FT)
            m_fmc_control->fsAccess().setAPAlt(qMin(flightstatus->APAlt()+100, 50000));
        else if (event->button() == Qt::RightButton || m_alt_set_mode == ALT_SET_MODE_1000FT)
            m_fmc_control->fsAccess().setAPAlt(qMin(flightstatus->APAlt()+1000, 50000));
    }

    else if (text == FMC_FCU_INPUT_KNOB_VS && event != 0)
    {
    	if (event->button() == Qt::LeftButton){
            m_fmc_control->fmcAutoPilot().setVSFPAHold(true);
        }

        else if (event->button() == Qt::RightButton)
        {
            m_fmc_control->fmcAutoPilot().setVSFPAHold(false);
        }
    }

    else if (text == FMC_FCU_INPUT_KNOB_VS_LEFT && event != 0)
    {
        if (m_fmc_control->fmcAutoPilot().isVsModeActive())
        {
            if (event->button() == Qt::LeftButton)
                m_fmc_control->fmcAutoPilot().setVerticalSpeed(flightstatus->APVs() - 100);
            else if (event->button() == Qt::RightButton)
                m_fmc_control->fmcAutoPilot().setVerticalSpeed(flightstatus->APVs() - 1000);
        }
        else if (m_fmc_control->fmcAutoPilot().isFlightPathModeActive())
        {
            if (event->button() == Qt::LeftButton)
                m_fmc_control->fmcAutoPilot().setFlightPathAngle(
                    m_fmc_control->fmcAutoPilot().flightPath() - 0.1);
            else if (event->button() == Qt::RightButton)
                m_fmc_control->fmcAutoPilot().setFlightPathAngle(
                    m_fmc_control->fmcAutoPilot().flightPath() - 1.0);
        }
    }
    else if (text == FMC_FCU_INPUT_KNOB_VS_RIGHT && event != 0)
    {
        if (m_fmc_control->fmcAutoPilot().isVsModeActive())
        {
            if (event->button() == Qt::LeftButton)
                m_fmc_control->fmcAutoPilot().setVerticalSpeed(flightstatus->APVs() + 100);
            else if (event->button() == Qt::RightButton)
                m_fmc_control->fmcAutoPilot().setVerticalSpeed(flightstatus->APVs() + 1000);
        }
        else if (m_fmc_control->fmcAutoPilot().isFlightPathModeActive())
        {
            if (event->button() == Qt::LeftButton)
                m_fmc_control->fmcAutoPilot().setFlightPathAngle(
                    m_fmc_control->fmcAutoPilot().flightPath() + 0.1);
            else if (event->button() == Qt::RightButton)
                m_fmc_control->fmcAutoPilot().setFlightPathAngle(
                    m_fmc_control->fmcAutoPilot().flightPath() + 1.0);
        }
    }
    else if (text == FMC_FCU_INPUT_KNOB_LEFT_ALT_SET)
    {
        if (event->button() == Qt::LeftButton)
        {
            m_fmc_control->slotSetAltimeterIsSetToSTD(true, false);
        }
        else if (event->button() == Qt::RightButton)
        {
            m_fmc_control->slotSetAltimeterIsSetToSTD(true, true);
            m_fmc_control->fsAccess().setAltimeterHpa(Navcalc::STD_ALTIMETER_HPA);
        }
    }
    else if (text == FMC_FCU_INPUT_KNOB_RIGHT_ALT_SET)
    {
        if (event->button() == Qt::LeftButton)
        {
            m_fmc_control->slotSetAltimeterIsSetToSTD(false, false);
        }
        else if (event->button() == Qt::RightButton)
        {
            m_fmc_control->slotSetAltimeterIsSetToSTD(false, true);
            m_fmc_control->fsAccess().setAltimeterHpa(Navcalc::STD_ALTIMETER_HPA);
        }
    }
    else if (text == FMC_FCU_INPUT_KNOB_LEFT_ALT_SET_LEFT)
    {
        int diff = (event->button() == Qt::LeftButton ? -1 : -10);
        m_fmc_control->changeAltimeterSetting(true, diff);
    }
    else if (text == FMC_FCU_INPUT_KNOB_RIGHT_ALT_SET_LEFT)
    {
        int diff = (event->button() == Qt::LeftButton ? -1 : -10);
        m_fmc_control->changeAltimeterSetting(false, diff);
    }
    else if (text == FMC_FCU_INPUT_KNOB_LEFT_ALT_SET_RIGHT)
    {
        int diff = (event->button() == Qt::LeftButton ? 1 : 10);
        m_fmc_control->changeAltimeterSetting(true, diff);
    }
    else if (text == FMC_FCU_INPUT_KNOB_RIGHT_ALT_SET_RIGHT)
    {
        int diff = (event->button() == Qt::LeftButton ? 1 : 10);
        m_fmc_control->changeAltimeterSetting(false, diff);
    }
    else if (text == FMC_FCU_INPUT_KNOB_ALT_SET_100FT)
    {
        m_alt_set_mode = ALT_SET_MODE_100FT;
    }
    else if (text == FMC_FCU_INPUT_KNOB_ALT_SET_1000FT)
    {
        m_alt_set_mode = ALT_SET_MODE_1000FT;
    }

    slotRefresh();
}

/////////////////////////////////////////////////////////////////////////////

void FMCFCUStyleA::slotInputMouseWheel(const QString& text, QWheelEvent* event)
{
    MYASSERT(event != 0);

    //----- SPD

    if (text == FMC_FCU_INPUT_KNOB_SPD) //_PUSH || text == FMC_FCU_INPUT_KNOB_SPD_PULL)
    {
        QMouseEvent fake_mouse_event(
            QEvent::MouseButtonPress, event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());

        (event->delta() < 0) ?
            slotInputMouseButton(FMC_FCU_INPUT_KNOB_SPD_LEFT, &fake_mouse_event) :
            slotInputMouseButton(FMC_FCU_INPUT_KNOB_SPD_RIGHT, &fake_mouse_event);
    }
    else if (text == FMC_FCU_INPUT_KNOB_SPD_LEFT)
    {
        QMouseEvent fake_mouse_event(
            QEvent::MouseButtonPress, event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());

        slotInputMouseButton((event->delta() < 0) ? text : FMC_FCU_INPUT_KNOB_SPD_RIGHT, &fake_mouse_event);
    }
    else if (text == FMC_FCU_INPUT_KNOB_SPD_RIGHT)
    {
        QMouseEvent fake_mouse_event(
            QEvent::MouseButtonPress, event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());

        slotInputMouseButton((event->delta() < 0) ? FMC_FCU_INPUT_KNOB_SPD_LEFT : text, &fake_mouse_event);
    }

    //----- HDG

    else if (text == FMC_FCU_INPUT_KNOB_HDG) //_PUSH || text == FMC_FCU_INPUT_KNOB_HDG_PULL)
    {
        QMouseEvent fake_mouse_event(
            QEvent::MouseButtonPress, event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());

        (event->delta() < 0) ?
            slotInputMouseButton(FMC_FCU_INPUT_KNOB_HDG_LEFT, &fake_mouse_event) :
            slotInputMouseButton(FMC_FCU_INPUT_KNOB_HDG_RIGHT, &fake_mouse_event);
    }
    else if (text == FMC_FCU_INPUT_KNOB_HDG_LEFT)
    {
        QMouseEvent fake_mouse_event(
            QEvent::MouseButtonPress, event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());

        slotInputMouseButton((event->delta() < 0) ? text : FMC_FCU_INPUT_KNOB_HDG_RIGHT, &fake_mouse_event);
    }
    else if (text == FMC_FCU_INPUT_KNOB_HDG_RIGHT)
    {
        QMouseEvent fake_mouse_event(
            QEvent::MouseButtonPress, event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());

        slotInputMouseButton((event->delta() < 0) ? FMC_FCU_INPUT_KNOB_HDG_LEFT : text, &fake_mouse_event);
    }

    //----- ALT

    else if (text == FMC_FCU_INPUT_KNOB_ALT) // _PUSH || text == FMC_FCU_INPUT_KNOB_ALT_PULL)
    {
        QMouseEvent fake_mouse_event(
            QEvent::MouseButtonPress, event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());

        (event->delta() < 0) ?
            slotInputMouseButton(FMC_FCU_INPUT_KNOB_ALT_LEFT, &fake_mouse_event) :
            slotInputMouseButton(FMC_FCU_INPUT_KNOB_ALT_RIGHT, &fake_mouse_event);
    }
    else if (text == FMC_FCU_INPUT_KNOB_ALT_LEFT)
    {
        QMouseEvent fake_mouse_event(
            QEvent::MouseButtonPress, event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());

        slotInputMouseButton((event->delta() < 0) ? text : FMC_FCU_INPUT_KNOB_ALT_RIGHT, &fake_mouse_event);
    }
    else if (text == FMC_FCU_INPUT_KNOB_ALT_RIGHT)
    {
        QMouseEvent fake_mouse_event(
            QEvent::MouseButtonPress, event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());

        slotInputMouseButton((event->delta() < 0) ? FMC_FCU_INPUT_KNOB_ALT_LEFT : text, &fake_mouse_event);
    }

    //----- VS

    else if (text == FMC_FCU_INPUT_KNOB_VS) // _PUSH || text == FMC_FCU_INPUT_KNOB_VS_PULL)
    {
        QMouseEvent fake_mouse_event(
            QEvent::MouseButtonPress, event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());

        (event->delta() < 0) ?
            slotInputMouseButton(FMC_FCU_INPUT_KNOB_VS_LEFT, &fake_mouse_event) :
            slotInputMouseButton(FMC_FCU_INPUT_KNOB_VS_RIGHT, &fake_mouse_event);
    }
    else if (text == FMC_FCU_INPUT_KNOB_VS_LEFT)
    {
        QMouseEvent fake_mouse_event(
            QEvent::MouseButtonPress, event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());

        slotInputMouseButton((event->delta() < 0) ? text : FMC_FCU_INPUT_KNOB_VS_RIGHT, &fake_mouse_event);
    }
    else if (text == FMC_FCU_INPUT_KNOB_VS_RIGHT)
    {
        QMouseEvent fake_mouse_event(
            QEvent::MouseButtonPress, event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());

        slotInputMouseButton((event->delta() < 0) ? FMC_FCU_INPUT_KNOB_VS_LEFT : text, &fake_mouse_event);
    }

    //----- ND RANGE

    else if (text == FMC_FCU_INPUT_KNOB_ND_LEFT_RANGE_LEFT)
    {
        if (event->delta() > 0)
        {
            QMouseEvent fake_mouse_event(
                QEvent::MouseButtonPress, event->pos(), Qt::RightButton, Qt::RightButton, event->modifiers());

            slotInputMouseButton(FMC_FCU_INPUT_KNOB_ND_LEFT_RANGE_RIGHT, &fake_mouse_event);
        }
        else
        {
            QMouseEvent fake_mouse_event(
                QEvent::MouseButtonPress, event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());

            slotInputMouseButton(FMC_FCU_INPUT_KNOB_ND_LEFT_RANGE_LEFT, &fake_mouse_event);
        }
    }
    else if (text == FMC_FCU_INPUT_KNOB_ND_LEFT_RANGE_RIGHT)
    {
        if (event->delta() > 0)
        {
            QMouseEvent fake_mouse_event(
                QEvent::MouseButtonPress, event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());

            slotInputMouseButton(FMC_FCU_INPUT_KNOB_ND_LEFT_RANGE_LEFT, &fake_mouse_event);
        }
        else
        {
            QMouseEvent fake_mouse_event(
                QEvent::MouseButtonPress, event->pos(), Qt::RightButton, Qt::RightButton, event->modifiers());

            slotInputMouseButton(FMC_FCU_INPUT_KNOB_ND_LEFT_RANGE_RIGHT, &fake_mouse_event);
        }
    }
    else if (text == FMC_FCU_INPUT_KNOB_ND_RIGHT_RANGE_LEFT)
    {
        if (event->delta() > 0)
        {
            QMouseEvent fake_mouse_event(
                QEvent::MouseButtonPress, event->pos(), Qt::RightButton, Qt::RightButton, event->modifiers());

            slotInputMouseButton(FMC_FCU_INPUT_KNOB_ND_RIGHT_RANGE_RIGHT, &fake_mouse_event);
        }
        else
        {
            QMouseEvent fake_mouse_event(
                QEvent::MouseButtonPress, event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());

            slotInputMouseButton(FMC_FCU_INPUT_KNOB_ND_RIGHT_RANGE_LEFT, &fake_mouse_event);
        }
    }
    else if (text == FMC_FCU_INPUT_KNOB_ND_RIGHT_RANGE_RIGHT)
    {
        if (event->delta() > 0)
        {
            QMouseEvent fake_mouse_event(
                QEvent::MouseButtonPress, event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());

            slotInputMouseButton(FMC_FCU_INPUT_KNOB_ND_RIGHT_RANGE_LEFT, &fake_mouse_event);
        }
        else
        {
            QMouseEvent fake_mouse_event(
                QEvent::MouseButtonPress, event->pos(), Qt::RightButton, Qt::RightButton, event->modifiers());

            slotInputMouseButton(FMC_FCU_INPUT_KNOB_ND_RIGHT_RANGE_RIGHT, &fake_mouse_event);
        }
    }

    //----- ND MODE

    else if (text == FMC_FCU_INPUT_KNOB_ND_LEFT_MODE_LEFT)
    {
        if (event->delta() > 0)
        {
            QMouseEvent fake_mouse_event(
                QEvent::MouseButtonPress, event->pos(), Qt::RightButton, Qt::RightButton, event->modifiers());

            slotInputMouseButton(FMC_FCU_INPUT_KNOB_ND_LEFT_MODE_RIGHT, &fake_mouse_event);
        }
        else
        {
            QMouseEvent fake_mouse_event(
                QEvent::MouseButtonPress, event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());

            slotInputMouseButton(FMC_FCU_INPUT_KNOB_ND_LEFT_MODE_LEFT, &fake_mouse_event);
        }
    }
    else if (text == FMC_FCU_INPUT_KNOB_ND_LEFT_MODE_RIGHT)
    {
        if (event->delta() > 0)
        {
            QMouseEvent fake_mouse_event(
                QEvent::MouseButtonPress, event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());

            slotInputMouseButton(FMC_FCU_INPUT_KNOB_ND_LEFT_MODE_LEFT, &fake_mouse_event);
        }
        else
        {
            QMouseEvent fake_mouse_event(
                QEvent::MouseButtonPress, event->pos(), Qt::RightButton, Qt::RightButton, event->modifiers());

            slotInputMouseButton(FMC_FCU_INPUT_KNOB_ND_LEFT_MODE_RIGHT, &fake_mouse_event);
        }
    }
    else if (text == FMC_FCU_INPUT_KNOB_ND_RIGHT_MODE_LEFT)
    {
        if (event->delta() > 0)
        {
            QMouseEvent fake_mouse_event(
                QEvent::MouseButtonPress, event->pos(), Qt::RightButton, Qt::RightButton, event->modifiers());

            slotInputMouseButton(FMC_FCU_INPUT_KNOB_ND_RIGHT_MODE_RIGHT, &fake_mouse_event);
        }
        else
        {
            QMouseEvent fake_mouse_event(
                QEvent::MouseButtonPress, event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());

            slotInputMouseButton(FMC_FCU_INPUT_KNOB_ND_RIGHT_MODE_LEFT, &fake_mouse_event);
        }
    }
    else if (text == FMC_FCU_INPUT_KNOB_ND_RIGHT_MODE_RIGHT)
    {
        if (event->delta() > 0)
        {
            QMouseEvent fake_mouse_event(
                QEvent::MouseButtonPress, event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());

            slotInputMouseButton(FMC_FCU_INPUT_KNOB_ND_RIGHT_MODE_LEFT, &fake_mouse_event);
        }
        else
        {
            QMouseEvent fake_mouse_event(
                QEvent::MouseButtonPress, event->pos(), Qt::RightButton, Qt::RightButton, event->modifiers());

            slotInputMouseButton(FMC_FCU_INPUT_KNOB_ND_RIGHT_MODE_RIGHT, &fake_mouse_event);
        }
    }

    //----- ALT SET

    else if (text == FMC_FCU_INPUT_KNOB_LEFT_ALT_SET || text == FMC_FCU_INPUT_KNOB_RIGHT_ALT_SET)
    {
        QMouseEvent fake_mouse_event(
            QEvent::MouseButtonPress, event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());

        (event->delta() < 0) ?
            slotInputMouseButton(FMC_FCU_INPUT_KNOB_LEFT_ALT_SET_LEFT, &fake_mouse_event) :
            slotInputMouseButton(FMC_FCU_INPUT_KNOB_LEFT_ALT_SET_RIGHT, &fake_mouse_event);
    }
    else if (text == FMC_FCU_INPUT_KNOB_LEFT_ALT_SET_LEFT || text == FMC_FCU_INPUT_KNOB_RIGHT_ALT_SET_LEFT)
    {
        QMouseEvent fake_mouse_event(
            QEvent::MouseButtonPress, event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());

        slotInputMouseButton((event->delta() < 0) ? text : FMC_FCU_INPUT_KNOB_LEFT_ALT_SET_RIGHT, &fake_mouse_event);
    }
    else if (text == FMC_FCU_INPUT_KNOB_LEFT_ALT_SET_RIGHT || text == FMC_FCU_INPUT_KNOB_RIGHT_ALT_SET_RIGHT)
    {
        QMouseEvent fake_mouse_event(
            QEvent::MouseButtonPress, event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());

        slotInputMouseButton((event->delta() < 0) ? FMC_FCU_INPUT_KNOB_LEFT_ALT_SET_LEFT : text, &fake_mouse_event);
    }

}

/////////////////////////////////////////////////////////////////////////////

void FMCFCUStyleA::keyPressEvent(QKeyEvent *event)
{
    MYASSERT(event != 0);

    bool ctrl_mod = (event->modifiers() & Qt::ControlModifier ? 1 : 0);

    if ((event->modifiers() & Qt::ShiftModifier) != 0)
    {
        switch(event->key())
        {
            case(Qt::Key_Left):
                (ctrl_mod) ? resize(size().width()-10, size().height()) : move(pos().x()-10, pos().y()); break;
            case(Qt::Key_Right):
                (ctrl_mod) ? resize(size().width()+10, size().height()) : move(pos().x()+10, pos().y()); break;
            case(Qt::Key_Up):
                (ctrl_mod) ? resize(size().width(), size().height()-10) : move(pos().x(), pos().y()-10); break;
            case(Qt::Key_Down):
                (ctrl_mod) ? resize(size().width(), size().height()+10) : move(pos().x(), pos().y()+10); break;
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

    slotRefresh();
}

/////////////////////////////////////////////////////////////////////////////

void FMCFCUStyleA::mousePressEvent(QMouseEvent *event)
{
    m_input_area.slotMousePressEvent(event);
    m_last_mouse_position = event->globalPos();
}

/////////////////////////////////////////////////////////////////////////////

void FMCFCUStyleA::mouseMoveEvent(QMouseEvent *event)
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

void FMCFCUStyleA::wheelEvent(QWheelEvent* event)
{
    if (event->buttons() == Qt::NoButton) m_input_area.slotMouseWheelEvent(event);
}

/////////////////////////////////////////////////////////////////////////////

bool FMCFCUStyleA::eventFilter(QObject * watched_object, QEvent* event)
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

void FMCFCUStyleA::paintMe(QPainter& painter)
{
    const FlightStatus* flightstatus = m_fmc_control->flightStatus();

    painter.scale(width()/(double)m_background_pixmap.width(), height()/(double)m_background_pixmap.height());

    //----- draw left range knob

    if  (m_last_range_left != (int)m_fmc_control->getNDRangeNM(true))
    {
        QPixmap* range_image = m_range10_image;
        switch(m_fmc_control->getNDRangeNM(true))
        {
            case(20): range_image = m_range20_image; break;
            case(40): range_image = m_range40_image; break;
            case(80): range_image = m_range80_image; break;
            case(160): range_image = m_range160_image; break;
            case(320): range_image = m_range320_image; break;
        }

        QPainter background_painter(&m_background_pixmap);
        // TODO put these coords to the config!
        background_painter.drawPixmap(255, 45, *range_image);
        resetBackgroundImage();
        m_last_range_left = m_fmc_control->getNDRangeNM(true);
        QTimer::singleShot(0, this, SLOT(slotRefresh()));
    }

    //----- draw right range knob

    if  (!m_fmc_control->fcuLeftOnlyMode() && m_last_range_right != (int)m_fmc_control->getNDRangeNM(false))
    {
        QPixmap* range_image = m_range10_image;
        switch(m_fmc_control->getNDRangeNM(false))
        {
            case(20): range_image = m_range20_image; break;
            case(40): range_image = m_range40_image; break;
            case(80): range_image = m_range80_image; break;
            case(160): range_image = m_range160_image; break;
            case(320): range_image = m_range320_image; break;
        }

        QPainter background_painter(&m_background_pixmap);
        // TODO put these coords to the config!
        background_painter.drawPixmap(1151, 45, *range_image);
        resetBackgroundImage();
        m_last_range_right = m_fmc_control->getNDRangeNM(false);
        QTimer::singleShot(0, this, SLOT(slotRefresh()));
    }

    //----- draw left mode knob

    if (m_last_mode_left != m_fmc_control->currentNDMode(true))
    {
        QPixmap* mode_image = m_mode_arc_image;
        switch(m_fmc_control->currentNDMode(true))
        {
            case(CFG_ND_DISPLAY_MODE_NAV_ARC): mode_image = m_mode_arc_image; break;
            case(CFG_ND_DISPLAY_MODE_NAV_ROSE): mode_image = m_mode_rose_image; break;
            case(CFG_ND_DISPLAY_MODE_NAV_PLAN): mode_image = m_mode_plan_image; break;
            case(CFG_ND_DISPLAY_MODE_VOR_ROSE): mode_image = m_mode_vor_image; break;
            case(CFG_ND_DISPLAY_MODE_ILS_ROSE): mode_image = m_mode_ils_image; break;
        }

        QPainter background_painter(&m_background_pixmap);
        // TODO put these coords to the config!
        background_painter.drawPixmap(148, 57, *mode_image);
        resetBackgroundImage();
        m_last_mode_left = m_fmc_control->currentNDMode(true);
        QTimer::singleShot(0, this, SLOT(slotRefresh()));
    }

    //----- draw right mode knob

    if (!m_fmc_control->fcuLeftOnlyMode() && m_last_mode_right != m_fmc_control->currentNDMode(false))
    {
        QPixmap* mode_image = m_mode_arc_image;
        switch(m_fmc_control->currentNDMode(false))
        {
            case(CFG_ND_DISPLAY_MODE_NAV_ARC): mode_image = m_mode_arc_image; break;
            case(CFG_ND_DISPLAY_MODE_NAV_ROSE): mode_image = m_mode_rose_image; break;
            case(CFG_ND_DISPLAY_MODE_NAV_PLAN): mode_image = m_mode_plan_image; break;
            case(CFG_ND_DISPLAY_MODE_VOR_ROSE): mode_image = m_mode_vor_image; break;
            case(CFG_ND_DISPLAY_MODE_ILS_ROSE): mode_image = m_mode_ils_image; break;
        }

        QPainter background_painter(&m_background_pixmap);
        // TODO put these coords to the config!
        background_painter.drawPixmap(1046, 57, *mode_image);
        resetBackgroundImage();
        m_last_mode_right = m_fmc_control->currentNDMode(false);
        QTimer::singleShot(0, this, SLOT(slotRefresh()));
    }

    //-----

    if (m_fmc_control->showInputAreas())
    {
        QPen inputareas_pen(YELLOW);
        inputareas_pen.setWidth(1);
        painter.setPen(inputareas_pen);
        InputAreaListIterator iter(m_input_area.inputAreaList());
        while(iter.hasNext()) painter.drawRect(iter.next());
    }

    //-----

    if (!isVisible() || (flightstatus->isValid() && !flightstatus->battery_on)) return;

    //-----

    QFont lcd_font(m_fcu_config->getValue(CFG_FCU_LCD_FONTNAME));
    lcd_font.setPixelSize(m_fcu_config->getIntValue(CFG_FCU_LCD_FONTSIZE));
    QFont normal_font(m_fcu_config->getValue(CFG_FCU_NORMAL_FONTNAME));
    normal_font.setPixelSize(m_fcu_config->getIntValue(CFG_FCU_NORMAL_FONTSIZE));

    //-----

    //TODO move this values to the (input)config

    // TODO got a better reference value?
    int managed_dot_diameter = m_fcu_config->getIntValue(CFG_FCU_NORMAL_FONTSIZE);

    QRectF alt_setting_rect_left(33,23,68,26);
    QRectF alt_setting_rect_right(1273,23,68,26);

    QRectF spd_rect(433,11,56,42);
    QRectF spd_dot_rect(spd_rect.right(), spd_rect.top(), 30, spd_rect.height());

    QRectF hdg_rect(551,11,50,42);
    QRectF hdg_dot_rect(hdg_rect.right()+5, hdg_rect.top(), 30, hdg_rect.height());

    QRectF hdg_trk_text_rect(659,10,78,38);
    QRectF lat_text_rect(hdg_rect.right()+5, hdg_rect.top(), 30, hdg_rect.height());

    QRectF alt_rect(771,11,88,42);

    QRectF vs_rect(879,11,84,42);

    QRectF lvl_chg_text_rect(alt_rect.left(), alt_rect.top(), vs_rect.right()-alt_rect.left(), alt_rect.height());

    //-----

    painter.setFont(lcd_font);
    painter.setPen(ORANGE);

    // left altimeter setting
     
    QString alt_text;

    if (m_fmc_control->isAltimeterSetToSTD(true))
    {
        alt_text = "STD";
    }
    else
    {
        if (m_fmc_control->showAltHpa(true))
            alt_text = QString("%1").arg(flightstatus->AltPressureSettingHpa(), 0, 'f', 0);
        else
            alt_text = QString("%1").arg(Navcalc::getInchesFromHpa(flightstatus->AltPressureSettingHpa()), 0, 'f', 2);
    }

    painter.drawText(alt_setting_rect_left, Qt::AlignHCenter|Qt::AlignVCenter,
                     flightstatus->AltPressureSettingHpa() == 0.0 ? "----" : alt_text);

    // right altimeter setting
     
    if (m_fmc_control->isAltimeterSetToSTD(false))
    {
        alt_text = "STD";
    }
    else
    {
        if (m_fmc_control->showAltHpa(false))
            alt_text = QString("%1").arg(flightstatus->AltPressureSettingHpa(), 0, 'f', 0);
        else
            alt_text = QString("%1").arg(Navcalc::getInchesFromHpa(flightstatus->AltPressureSettingHpa()), 0, 'f', 2);
    }

    painter.drawText(alt_setting_rect_right, Qt::AlignHCenter|Qt::AlignVCenter,
                     flightstatus->AltPressureSettingHpa() == 0.0 ? "----" : alt_text);

    // AP displays

    // speed

    if (m_fmc_control->fmcAutoPilot().isTakeoffModeActiveVertical())
    {
        //TODO also do this in managed speed mode later

        painter.drawText(spd_rect, Qt::AlignHCenter|Qt::AlignBottom, "---");

        QBrush old_brush = painter.brush();
        painter.setBrush(QBrush(ORANGE, Qt::SolidPattern));
        painter.drawPie((int)spd_dot_rect.left(), 
                        (int)spd_dot_rect.bottom() - managed_dot_diameter*2,
                        managed_dot_diameter, managed_dot_diameter, 0, 16*360);
        painter.setBrush(old_brush);

    }
    else if (m_fmc_control->fmcAutothrottle().isAPThrottleModeMachSet())
    {
        painter.drawText(spd_rect, Qt::AlignHCenter|Qt::AlignBottom,
                         QString("%1").arg(flightstatus->APMach(), 0, 'f', 2));
    }
    else
    {
        painter.drawText(spd_rect, Qt::AlignHCenter|Qt::AlignBottom,
                         QString("%1").arg(Navcalc::round(flightstatus->APSpd()), 3, 10, QChar('0')));
    }

    // hdg

    if (m_fmc_control->fmcAutoPilot().isNAVCoupled() || m_fmc_control->fmcAutoPilot().isTakeoffModeActiveLateral())
    {
        painter.drawText(hdg_rect, Qt::AlignHCenter|Qt::AlignBottom, "---");
        
        QBrush old_brush = painter.brush();
        painter.setBrush(QBrush(ORANGE, Qt::SolidPattern));
        painter.drawPie((int)hdg_dot_rect.center().x() - managed_dot_diameter, 
                        (int)hdg_dot_rect.bottom() - managed_dot_diameter*2,
                        managed_dot_diameter, managed_dot_diameter, 0, 16*360);
        painter.setBrush(old_brush);
    }
    else
    {
        painter.drawText(hdg_rect, Qt::AlignHCenter|Qt::AlignBottom,
                         QString("%1").arg(flightstatus->APHdg(), 3, 10, QChar('0')));
    }

    // altitude

    painter.drawText(alt_rect, Qt::AlignHCenter|Qt::AlignBottom, QString::number((flightstatus->APAlt()/100)*100));

    // v/s & fpv

    //TODO draw managed mode alt dot

    QString vs_string = "-----";

    if (m_fmc_control->fmcAutoPilot().isFLChangeModeActive())
    {
        //do nothing
    }
    else if (m_fmc_control->fmcAutoPilot().isFlightPathModeActive())
    {
            vs_string = QString("%1").arg(qAbs(m_fmc_control->fmcAutoPilot().flightPath()), 0, 'f', 2);
            if (m_fmc_control->fmcAutoPilot().flightPath() >= 0) vs_string = "+" + vs_string;
            else vs_string = "-" + vs_string;
    }
    else if (m_fmc_control->fmcAutoPilot().isVsModeActive())
    {
        vs_string = QString("%1").arg(qAbs(flightstatus->APVs()), 4, 10, QChar('0'));
        if (flightstatus->APVs() >= 0) vs_string = "+" + vs_string;
        else vs_string = "-" + vs_string;
    }

    painter.drawText(vs_rect, Qt::AlignHCenter|Qt::AlignBottom, vs_string);

    //----- small buttons

    QPen pen(YELLOW);
    pen.setWidth(3);
    painter.setPen(pen);

    if (m_fmc_control->showSurroundingAirports(true))
    {
        const InputArea& area = m_input_area.inputArea(FMC_FCU_INPUT_ND_LEFT_AIRPORT);
        painter.drawLine(area.left()+HSPACE, area.top()+VSPACE, area.right()-HSPACE+2, area.top()+VSPACE);
    }

    if (m_fmc_control->showSurroundingNDBs(true))
    {
        const InputArea& area = m_input_area.inputArea(FMC_FCU_INPUT_ND_LEFT_NDB);
        painter.drawLine(area.left()+HSPACE, area.top()+VSPACE, area.right()-HSPACE+2, area.top()+VSPACE);
    }

    if (m_fmc_control->showSurroundingVORs(true))
    {
        const InputArea& area = m_input_area.inputArea(FMC_FCU_INPUT_ND_LEFT_VOR);
        painter.drawLine(area.left()+HSPACE, area.top()+VSPACE, area.right()-HSPACE+2, area.top()+VSPACE);
    }

    if (m_fmc_control->showGeoData())
    {
        const InputArea& area1 = m_input_area.inputArea(FMC_FCU_INPUT_ND_RIGHT_TERR);
        painter.drawLine(area1.left()+HSPACE, area1.top()+VSPACE, area1.right()-HSPACE+2, area1.top()+VSPACE);
        const InputArea& area2 = m_input_area.inputArea(FMC_FCU_INPUT_ND_LEFT_TERR);
        painter.drawLine(area2.left()+HSPACE, area2.top()+VSPACE, area2.right()-HSPACE+2, area2.top()+VSPACE);
    }

    if (m_fmc_control->showConstrains(true))
    {
        const InputArea& area = m_input_area.inputArea(FMC_FCU_INPUT_ND_LEFT_CSTR);
        painter.drawLine(area.left()+HSPACE, area.top()+VSPACE, area.right()-HSPACE+2, area.top()+VSPACE);
    }

    if (m_fmc_control->showSurroundingAirports(false))
    {
        const InputArea& area = m_input_area.inputArea(FMC_FCU_INPUT_ND_RIGHT_AIRPORT);
        painter.drawLine(area.left()+HSPACE, area.top()+VSPACE, area.right()-HSPACE+2, area.top()+VSPACE);
    }

    if (m_fmc_control->showSurroundingNDBs(false))
    {
        const InputArea& area = m_input_area.inputArea(FMC_FCU_INPUT_ND_RIGHT_NDB);
        painter.drawLine(area.left()+HSPACE, area.top()+VSPACE, area.right()-HSPACE+2, area.top()+VSPACE);
    }

    if (m_fmc_control->showSurroundingVORs(false))
    {
        const InputArea& area = m_input_area.inputArea(FMC_FCU_INPUT_ND_RIGHT_VOR);
        painter.drawLine(area.left()+HSPACE, area.top()+VSPACE, area.right()-HSPACE+2, area.top()+VSPACE);
    }

    if (m_fmc_control->showConstrains(false))
    {
        const InputArea& area = m_input_area.inputArea(FMC_FCU_INPUT_ND_RIGHT_CSTR);
        painter.drawLine(area.left()+HSPACE, area.top()+VSPACE, area.right()-HSPACE+2, area.top()+VSPACE);
    }

    if (m_fmc_control->showPFDILS(true))
    {
        const InputArea& area = m_input_area.inputArea(FMC_FCU_INPUT_LEFT_ILS_ENABLE);
        painter.drawLine(area.left()+HSPACE, area.top()+VSPACE, area.right()-HSPACE+2, area.top()+VSPACE);
    }

    if (m_fmc_control->showPFDILS(false))
    {
        const InputArea& area = m_input_area.inputArea(FMC_FCU_INPUT_RIGHT_ILS_ENABLE);
        painter.drawLine(area.left()+HSPACE, area.top()+VSPACE, area.right()-HSPACE+2, area.top()+VSPACE);
    }

    if (flightstatus->fd_active)
    {
        const InputArea& area1 = m_input_area.inputArea(FMC_FCU_INPUT_LEFT_FD_ENABLE);
        painter.drawLine(area1.left()+HSPACE, area1.top()+VSPACE, area1.right()-HSPACE+2, area1.top()+VSPACE);
        const InputArea& area2 = m_input_area.inputArea(FMC_FCU_INPUT_RIGHT_FD_ENABLE);
        painter.drawLine(area2.left()+HSPACE, area2.top()+VSPACE, area2.right()-HSPACE+2, area2.top()+VSPACE);
    }

    if (m_fmc_control->fmcAutoPilot().isAPPHoldActive())
    {
        const InputArea& area = m_input_area.inputArea(FMC_FCU_INPUT_APP_ARM);
        painter.drawLine(area.left()+HSPACE, area.top()+VSPACE, area.right()-HSPACE+2, area.top()+VSPACE);
    }

    if (m_fmc_control->fmcAutoPilot().isLOCHoldActive())
    {
        const InputArea& area = m_input_area.inputArea(FMC_FCU_INPUT_LOC_ARM);
        painter.drawLine(area.left()+HSPACE, area.top()+VSPACE, area.right()-HSPACE+2, area.top()+VSPACE);
    }

    //----- AP/ATHR buttons

    pen.setColor(YELLOW);
    pen.setWidth(3);
    painter.setPen(pen);

    if (m_fmc_control->fmcAutothrottle().isAPThrottleArmed())
    {
        const InputArea& area = m_input_area.inputArea(FMC_FCU_INPUT_ATHROTTLE_ARM);
        painter.drawLine(area.left()+HSPACE, area.top()+VSPACE, area.right()-HSPACE+2, area.top()+VSPACE);
    }

    if (flightstatus->ap_enabled)
    {
        const InputArea& area = m_input_area.inputArea(FMC_FCU_INPUT_AP1_ARM);
        painter.drawLine(area.left()+HSPACE, area.top()+VSPACE, area.right()-HSPACE+2, area.top()+VSPACE);

        if (m_fmc_control->fmcAutoPilot().isAPPHoldActive())
        {
            const InputArea& area2 = m_input_area.inputArea(FMC_FCU_INPUT_AP2_ARM);
            painter.drawLine(area2.left()+HSPACE, area2.top()+VSPACE, area2.right()-HSPACE+2, area2.top()+VSPACE);
        }
    }

    //----- small texts

    painter.setFont(normal_font);
    painter.setPen(ORANGE);

    if (m_fmc_control->fmcAutothrottle().isAPThrottleModeMachSet())
        painter.drawText(spd_rect, Qt::AlignRight|Qt::AlignTop, "MACH");
    else
        painter.drawText(spd_rect, Qt::AlignLeft|Qt::AlignTop, "SPD");

    painter.drawText(lat_text_rect, Qt::AlignLeft|Qt::AlignTop, "LAT");

    if (m_fmc_control->fmcAutoPilot().isNAVCoupled())
    {
        //TODO draw dot
     }

    QRectF vs_text_rect_real;

    if (m_fmc_control->fmcAutoPilot().isFlightPathModeEnabled())
    {
        //TODOpainter.drawText(hdg_rect, Qt::AlignRight|Qt::AlignTop, "TRK");
        //painter.drawText(hdg_trk_text_rect, Qt::AlignHCenter|Qt::AlignBottom, "TRK      FPA");
        painter.drawText(hdg_rect, Qt::AlignRight|Qt::AlignTop, "HDG");
        painter.drawText(hdg_trk_text_rect, Qt::AlignHCenter|Qt::AlignBottom, "HDG      FPA");
        painter.drawText(vs_rect, Qt::AlignHCenter|Qt::AlignTop, "FPA", &vs_text_rect_real);
    }
    else
    {
        painter.drawText(hdg_rect, Qt::AlignLeft|Qt::AlignTop, "HDG");
        painter.drawText(hdg_trk_text_rect, Qt::AlignHCenter|Qt::AlignVCenter, "HDG      V/S");
        painter.drawText(vs_rect, Qt::AlignHCenter|Qt::AlignTop, "V/S", &vs_text_rect_real);
    }

    QRectF alt_text_rect_real;
    QRectF lvl_chg_text_rect_real;

    painter.drawText(alt_rect, Qt::AlignHCenter|Qt::AlignTop, "ALT", &alt_text_rect_real);
    painter.drawText(lvl_chg_text_rect, Qt::AlignHCenter|Qt::AlignTop, "LVL/CH", &lvl_chg_text_rect_real);

    //----- lvl/ch line

    pen.setColor(ORANGE);
    pen.setWidth(1);
    painter.setPen(pen);

    double y = alt_text_rect_real.top() + alt_text_rect_real.height()/2.0;
    double yadd = alt_text_rect_real.height()/3.0;
    double x_space = 5;
    painter.drawLine(QPointF(alt_text_rect_real.right() + x_space, y),
                     QPointF(alt_text_rect_real.right() + x_space, y + yadd));
    painter.drawLine(QPointF(alt_text_rect_real.right() + x_space, y),
                     QPointF(lvl_chg_text_rect_real.left() - x_space, y));
    painter.drawLine(QPointF(lvl_chg_text_rect_real.right() + x_space, y),
                     QPointF(vs_text_rect_real.left() - x_space, y));
    painter.drawLine(QPointF(vs_text_rect_real.left() - x_space, y),
                     QPointF(vs_text_rect_real.left() - x_space, y + yadd));
}

/////////////////////////////////////////////////////////////////////////////

void FMCFCUStyleA::resetBackgroundImage()
{
#if VASFMC_GAUGE
    m_background_pixmap_scaled = QPixmap();
#else
    QPalette our_palette = palette();
    for (int index = 0; index < QPalette::NColorGroups; ++index) {
        QColor color = our_palette.brush(QPalette::ColorGroup(index), QPalette::Background).color();
        our_palette.setBrush(QPalette::ColorGroup(index), QPalette::Background, QBrush(color, m_background_pixmap));
    }

    setPalette(our_palette);
#endif
}

/////////////////////////////////////////////////////////////////////////////

// End of file
