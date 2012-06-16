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

/*! \file    fmc_fcu_style_a.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __FMC_FCU_STYLE_A_H__
#define __FMC_FCU_STYLE_A_H__

#include <QSignalMapper>
#include <QPushButton>
#include <QPalette>
#include <QMouseEvent>
#include <QWheelEvent>

#include "mouse_input_area.h"

#include "ui_fmc_fcu.h"

#include "fmc_fcu.h"

/////////////////////////////////////////////////////////////////////////////

class FMCFCUStyleA : public FMCFCUStyleBase
#if !VASFMC_GAUGE
                   , private Ui::FMCFCU
#endif
{
    Q_OBJECT

public:
	
    enum ALT_SET_MODE { ALT_SET_MODE_100FT,
                        ALT_SET_MODE_1000FT
    };

    FMCFCUStyleA(ConfigWidgetProvider* config_widget_provider,
                 Config* main_config,
                 const QString& fcu_config_filename,
                 FMCControl* fmc_control,
                 QWidget* parent,
                 Qt::WFlags fl);
       
    virtual ~FMCFCUStyleA();

#if VASFMC_GAUGE
    virtual void paintGauge(QPainter *pPainter);
#endif

public slots:

    virtual void slotRefresh();

protected slots:

    void slotInputMouseButton(const QString& text, QMouseEvent* event);
    void slotInputMouseWheel(const QString& text, QWheelEvent* event);

protected:

    bool eventFilter(QObject * watched_object, QEvent* event);

    void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event) { m_input_area.slotMouseReleaseEvent(event); }
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent* event);

    void resetBackgroundImage();

protected:

    MouseInputArea m_input_area;
    QPoint m_last_mouse_position;
    ALT_SET_MODE m_alt_set_mode;

private:

    //! Hidden copy-constructor
    FMCFCUStyleA(const FMCFCUStyleA&);
    //! Hidden assignment operator
    const FMCFCUStyleA& operator = (const FMCFCUStyleA&);

    void paintMe(QPainter& painter);

    QPixmap m_background_pixmap;

    QPixmap *m_range10_image;
    QPixmap *m_range20_image;
    QPixmap *m_range40_image;
    QPixmap *m_range80_image;
    QPixmap *m_range160_image;
    QPixmap *m_range320_image;
    int m_last_range_left;
    int m_last_range_right;

    QPixmap *m_mode_ils_image;
    QPixmap *m_mode_vor_image;
    QPixmap *m_mode_rose_image;
    QPixmap *m_mode_arc_image;
    QPixmap *m_mode_plan_image;
    int m_last_mode_left;
    int m_last_mode_right;
    
#if VASFMC_GAUGE
    QPixmap m_background_pixmap_scaled;
#endif
};

#endif /* __FMC_FCU_STYLE_A_H__ */

// End of file

