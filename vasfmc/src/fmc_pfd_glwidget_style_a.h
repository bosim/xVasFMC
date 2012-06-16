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

#ifndef FMCPFD_GLWIDGET_STYLE_A_H
#define FMCPFD_GLWIDGET_STYLE_A_H

#include <QTimer>

#include "fmc_pfd_glwidget_base.h"

#include "fmc_autopilot.h"
#include "fmc_autothrottle.h"

class Statistics;

/////////////////////////////////////////////////////////////////////////////

class GLPFDWidgetStyleA : public GLPFDWidgetBase
{
    Q_OBJECT

public:

    GLPFDWidgetStyleA(ConfigWidgetProvider* config_widget_provider,
                      Config* main_config,
                      Config* pfd_config,
                      FMCControl* fmc_control,
                      VasWidget *parent,
                      bool left_side);
    
    virtual ~GLPFDWidgetStyleA();

 	void refreshPFD();

protected slots:

    void slotAltimeterBlinkTimer() { m_show_altimeter_setting = !m_show_altimeter_setting; }

    void slotFBWParametersChanged() { m_recalc_pfd = true; }

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

    void enableClipping(GLuint gllist_id);
    void disableClipping();

    inline void drawText(const double& x,const double& y, const QString& text) { m_font->drawText(x, y, text); }
    inline double getTextWidth(const QString& text) const { return m_font->getWidth(text); }

    inline void enableAltimeterBlink() 
    { if (!m_altimeter_blink_timer.isActive()) m_altimeter_blink_timer.start(500); }

    inline void disableAltimeterBlink() 
    {
        m_altimeter_blink_timer.stop();
        m_show_altimeter_setting = true;
    }

    QString speedFMAText(FMCAutothrottle::SPEED_MODE mode);
    QString verticalFMAText(FMCAutopilot::VERTICAL_MODE mode);
    QString lateralFMAText(FMCAutopilot::LATERAL_MODE mode);

protected:
    
    OpenGLText* m_font;
    double m_font_height;
    double m_font_height_half;
    double m_font_height_double;
    
    bool m_recalc_pfd;
	QPointF m_position_center;

	QTime m_fps_counter_time;
	unsigned int m_fps;
	unsigned int m_fps_count;

    bool m_onground;
    bool m_fbw_was_enabled;
    bool m_draw_joystick_input_when_engines_running;

    //-----

    GLuint m_horizon_clip_gllist;
    GLuint m_horizon_gllist;
    GLuint m_horizon_static_marker_gllist;
    GLuint m_horizon_bank_marker_gllist;

    //-----

    double m_horizon_odd_pitch_line_width;
    double m_horizon_even_pitch_line_width;
    double m_horizon_limit_y_offset;

    double m_fd_width;
    double m_fd_height;

    double m_horizon_middle_marking_width;
    double m_horizon_middle_marking_height;
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

    double m_alt_last_two_digit_offset;
    double m_alt_band_horiz_offset;
    double m_alt_band_vert_gap;
    double m_alt_band_width;
    double m_alt_value_horiz_offset;
    int m_alt_range;
    double m_alt_pixel_per_ft;
    double m_alt_tick_width;
    double m_alt_marker_offset;
    double m_alt_marker_width;

    double m_rocd_band_width;
    double m_rocd_band_horiz_offset;
    double m_rocd_root_offset;
    double m_rocd_mark_500ft_y;

    double m_ils_circle_size;
    double m_ils_circle_offset;
    double m_gs_bug_height;
    double m_gs_bug_width;
   
    double m_speed_band_width;
    double m_speed_band_horiz_offset;
    double m_speed_repaint_y_offset;
    double m_speed_repaint_height;
    double m_speed_marker_width;
    double m_speed_marker_offset;
    double m_ap_speed_marker_width;

    int m_speed_range;
    double m_spd_pixel_per_knot;

    double m_hdg_band_vert_offset;
    double m_hdg_band_height;
    int m_hdg_range;
    double m_hdg_pixel_per_deg;
    double m_hdg_marker_height;
    double m_hdg_marker_offset;
    double m_hdg_repaint_x_offset;
    double m_hdg_repaint_width;
    double m_ap_hdg_marker_height;

    // FS status

    double m_pitch;
    double m_bank;
    double m_mag_hdg;
    double m_vs;
    double m_alt_readout;

    bool m_is_below_mda;
    bool m_was_on_cruise_fl;

private:

    QTimer m_altimeter_blink_timer;
    bool m_show_altimeter_setting;

    QTime m_radar_alt_blink_timer;

    QTime m_ils_warning_blink_timer;
    bool m_show_ils_warning;

    QTime m_altitude_window_blink_timer;

    QTime m_thr_lvr_clb_blink_timer;
    bool m_show_thr_lvr_clb;

    Statistics* m_stat;
    QTime m_stat_timer;
};

#endif // FMCPFD_GLWIDGET_STYLE_A_H
