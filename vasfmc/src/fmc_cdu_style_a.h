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

/*! \file    fmc_cdu_style_a.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __FMC_CDU_STYLE_A_H__
#define __FMC_CDU_STYLE_A_H__

#include <QSignalMapper>
#include <QPushButton>
#include <QPalette>
#include <QWheelEvent>

#include "mouse_input_area.h"

#include "ui_fmc_cdu.h"

#include "fmc_cdu.h"

class FMCCDUPageManager;

/////////////////////////////////////////////////////////////////////////////

class FMCCDUStyleA : public FMCCDUStyleBase
#if !VASFMC_GAUGE
                   , private Ui::FMCCDU
#endif
{
    Q_OBJECT

public:

    FMCCDUStyleA(ConfigWidgetProvider* config_widget_provider,
                 Config* main_config,
                 const QString& cdu_config_filename,
                 FMCControl* fmc_control,
                 QWidget* parent,
                 Qt::WFlags fl,
                 bool left_side);
       
    virtual ~FMCCDUStyleA();

#if VASFMC_GAUGE
    virtual void paintGauge(QPainter *pPainter);
#endif

public slots:

    virtual void slotRefresh();

protected slots:

    void slotInput(const QString& text, QMouseEvent*);
    void slotInputLongClick(const QString& text, QMouseEvent*);
    void slotClearAll();

protected:

    bool eventFilter(QObject * watched_object, QEvent* event);

    void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event) { m_input_area.slotMouseReleaseEvent(event); }
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent* event);

protected:

    FMCCDUPageManager* m_page_manager;
    MouseInputArea m_input_area;
    QPoint m_last_mouse_position;

private:

    //! Hidden copy-constructor
    FMCCDUStyleA(const FMCCDUStyleA&);
    //! Hidden assignment operator
    const FMCCDUStyleA& operator = (const FMCCDUStyleA&);

    void paintMe(QPainter& painter);

    QPixmap m_background_image;
#if VASFMC_GAUGE
    QPixmap m_background_image_scaled;
#endif
};

#endif /* __FMC_CDU_STYLE_A_H__ */

// End of file

