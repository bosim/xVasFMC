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

#ifndef FMCPFD_GLWIDGET_STYLE_B_H
#define FMCPFD_GLWIDGET_STYLE_B_H

#include "fmc_pfd_glwidget_base.h"

#include "fmc_autopilot.h"
#include "fmc_autothrottle.h"

/////////////////////////////////////////////////////////////////////////////

class GLPFDWidgetStyleB : public GLPFDWidgetBase
{
    Q_OBJECT

public:

    GLPFDWidgetStyleB(ConfigWidgetProvider* config_widget_provider,
                      Config* main_config,
                      Config* pfd_config,
                      FMCControl* fmc_control,
                      VasWidget *parent,
                      bool left_side);
    
    virtual ~GLPFDWidgetStyleB();

 	void refreshPFD();

protected:

    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

	void setupStateBeforeDraw();
	virtual void drawDisplay();

    inline double getVerticalOffsetByPitch(const double& pitch_angle)
    { return qMax(-90.0, qMin(90.0, pitch_angle)) * -m_horizon_size.height()/(2.0*25.0); }

    inline double getHorizontalOffsetByHeading(const double& hdg_angle)
    { return qMax(-90.0, qMin(90.0, hdg_angle)) * -m_horizon_size.width()/(2.0*25.0); }

    virtual void drawHorizon();
    virtual void drawAltitude();
    virtual void drawSpeed();
    virtual void drawHeading();
    virtual void drawInfoTexts();

    void drawFPS();

    void createHorizonClipping();
    void createHorizon();
    void createHorizonStaticMarkings();
    void createHorizonBankMarkings();
    void createAltitudeClipping();
    void createCompass();

    void enableClipping(GLuint gllist_id);
    void disableClipping();

    inline void drawText(const double& x,const double& y, const QString& text) { m_font->drawText(x, y, text); }
    inline double getTextWidth(const QString& text) const { return m_font->getWidth(text); }

    QString speedFMAText(FMCAutothrottle::SPEED_MODE mode);
    QString verticalFMAText(FMCAutopilot::VERTICAL_MODE mode);
    QString lateralFMAText(FMCAutopilot::LATERAL_MODE mode);

protected:
    
    QSize m_size;
    OpenGLText* m_font;
    double m_font_height;
    double m_font_height_half;
    
    bool m_recalc_pfd;
	QPointF m_position_center;

	QTime m_fps_counter_time;
	unsigned int m_fps;
	unsigned int m_fps_count;

    bool m_onground;

    //-----

    GLuint m_horizon_clip_gllist;
    GLuint m_horizon_gllist;
    GLuint m_horizon_static_marker_gllist;
    GLuint m_horizon_bank_marker_gllist;
    GLuint m_compass_gllist;

    //-----

    double m_horizon_odd_pitch_line_width;
    double m_horizon_even_pitch_line_width;
    double m_horizon_limit_y_offset;

    double m_fd_width;
    double m_fd_height;

    double m_horizon_middle_marking_width;
    double m_horizon_middle_marking_height;
    double m_horizon_side_marking_offset;
    double m_horizon_side_marking_width;
    double m_horizon_side_marking_height;
    double m_horizon_marking_border;

    QPointF m_horizon_offset;
    QSizeF m_horizon_size;
    double m_horizon_half_width;
    double m_horizon_half_height;

    double m_horizon_radius;
    double m_horizon_angle;

    double m_bank_marking_angle_inc;
    double m_bank_marking_width;
    double m_bank_marking_height;

    GLuint m_alt_value_clip_gllist;
    double m_alt_last_two_digit_offset;
    double m_alt_band_half_height;
    double m_alt_band_horiz_offset;
    double m_alt_band_width;
    double m_alt_value_horiz_offset;
    double m_alt_value_box_height;
    int m_alt_range;
    double m_alt_pixel_per_ft;
    double m_alt_tick_width;

    double m_rocd_band_half_height;
    double m_rocd_band_width;
    double m_rocd_band_horiz_offset;
    double m_rocd_root_offset;
    double m_rocd_mark_500ft_y;

    double m_ils_circle_size;
    double m_ils_bug_size;

    double m_speed_band_half_height;
    double m_speed_band_width;
    double m_speed_band_horiz_offset;
    double m_speed_repaint_y_offset;
    double m_speed_repaint_height;

    int m_speed_range;
    double m_speed_pixel_per_knot;
    double m_speed_tick_width;

    double m_hdg_half_angle;
    double m_hdg_band_radius;
    double m_hdg_band_bottom_y_offset;
    double m_hdg_band_vert_offset;
    double m_hdg_tick_width;

    int m_hdg_range;
    double m_hdg_pixel_per_deg;

    double m_fma_box_half_width;
    double m_fma_box_height;
    double m_fma_box_item_half_width;
    double m_fma_y_offset;

    // FS status

    double m_pitch;
    double m_bank;
};

#endif // FMCPFD_GLWIDGET_STYLE_B_H
