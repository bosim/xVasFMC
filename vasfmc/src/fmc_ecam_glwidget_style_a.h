///////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////

#ifndef FMCECAM_GLWIDGET_STYLE_A_H
#define FMCECAM_GLWIDGET_STYLE_A_H

#include "fmc_ecam_glwidget_base.h"

/////////////////////////////////////////////////////////////////////////////

class GLECAMWidgetStyleA : public GLECAMWidgetBase
{
    Q_OBJECT

public:

    GLECAMWidgetStyleA(bool upper_ecam,
                       ConfigWidgetProvider* config_widget_provider,
                       Config* main_config,
                       Config* ecam_config,
                       FMCControl* fmc_control,
                       VasWidget *parent = 0);
    
    virtual ~GLECAMWidgetStyleA();

 	void refreshECAM();

protected:

    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

	void setupStateBeforeDraw();
	virtual void drawDisplay();

    void drawSeparators();
    void drawEnginges();
    void drawTexts();

    void drawFPS();

    void createHorizonClipping();

//     void enableClipping(GLuint gllist_id);
//     void disableClipping();

    double getN1Angle(const double& n1) const;

    inline void drawText(const double& x,const double& y, const QString& text) { m_font->drawText(x, y, text); }
    inline double getTextWidth(const QString& text) const { return m_font->getWidth(text); }

protected:
    
    QSize m_size;
    OpenGLText* m_font;
    double m_font_height;
    
    bool m_recalc_ecam;

	QTime m_fps_counter_time;
	unsigned int m_fps;
	unsigned int m_fps_count;

    bool m_onground;

    //----- separators

    double m_separator_horizontal_left_x;
    double m_separator_horizontal_right_x;
    double m_separator_horizontal_y;
    double m_separator_vertical_x;
    double m_separator_vertical_upper_y;
    double m_separator_vertical_lower_y;
    double m_separator_left_len;
    double m_separator_right_len;
    double m_separator_upper_len;
    double m_separator_lower_len;

    //----- engines

    QPointF m_engine1_center_n1;
    QPointF m_engine2_center_n1;
    double m_engine_n1_radius;
    
    double m_engine_n1_angle_left;
    double m_engine_n1_angle_right;
    double m_n1_angle_range;

    QPointF m_engine1_center_egt;
    QPointF m_engine2_center_egt;
    double m_engine_egt_radius;

    double m_engine_egt_angle_left;
    double m_engine_egt_angle_right;
    double m_egt_angle_range;

    QPointF m_engine1_center_n2;
    QPointF m_engine2_center_n2;

    QPointF m_engine1_center_ff;
    QPointF m_engine2_center_ff;

    //----- texts

    QPointF m_left_text_ref;
    QPointF m_right_text_ref;

    //----- flaps

    double m_flap_text_spacer;
    double m_flap_text_x_left;
    double m_flap_text_x_right;
    double m_flap_text_middle;
    double m_flap_text_y;

    double m_slat_x;
    double m_slat_y;
    double m_slat_x_inc;
    double m_slat_y_inc;

    double m_flap_x;
    double m_flap_y;
    double m_flap_x_inc;
    double m_flap_y_inc;

    QList<QPointF> m_slat_points;
    QList<QPointF> m_flap_points;

    // misc

    bool m_at_was_connected;
    QTime m_at_disco_timestamp;

    bool m_takeoff_inhibit;
    bool m_landing_inhibit;
};

#endif // FMCECAM_GLWIDGET_STYLE_A_H
