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

/*! \file    fmc_navdisplay_style.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __FMC_NAVDISPLAY_STYLE_H__
#define __FMC_NAVDISPLAY_STYLE_H__

#include "math.h"

#include <QPainter>
#include <QMap>
#include <QDateTime>

#include "vas_gl.h"

#include "projection.h"
#include "flightstatus.h"
#include "logger.h"
#include "vas_gl_widget.h"

#include "gldraw.h"
#include "opengltext.h"

class Config;
class FMCData;
class FMCControl;
class FlightRoute;

/////////////////////////////////////////////////////////////////////////////

class FMCNavdisplayStyle : public QObject
{
    Q_OBJECT

public:

    FMCNavdisplayStyle(VasGLWidget* parent,
                       Config* main_config,
                       Config* navdisplay_config,
                       Config* tcas_config,
                       FMCControl* fmc_control,
                       const ProjectionBase* projection,
                       const QSize& size,
                       const double& display_top_offset,
                       const int& max_drawable_y,
                       const QPointF& position_center,
                       const double& dist_scale_factor,
                       bool left_side);

    virtual ~FMCNavdisplayStyle();

    inline Config* config() { return m_navdisplay_style_config; }

    virtual void reset(const QSize& size,
                       const double& display_top_offset,
                       const int& max_drawable_y,
                       const QPointF& position_center,
                       const double& dist_scale_factor);

    //----- pure virtual

    virtual void drawAvionicsOff() = 0;

	virtual void drawRanges() = 0;
	virtual void drawPlanePoly() = 0;
    virtual void drawCompass(double north_track_rotation) = 0;
    virtual void drawWindArrow() = 0;
    virtual void drawAltitudeReachDisplay(const double& dist_scale_factor) = 0;
    virtual void drawInfoTexts(const FlightRoute& route) = 0;
    virtual void drawAdfPointer(uint adf_index) = 0;
    virtual void drawVorPointer(uint vor_index) = 0;

    virtual void drawSurroundingItems(const double& north_track_rotation);

    virtual void drawAirportRunways(const FlightRoute& route, const double& north_track_rotation) = 0;

    virtual void drawRouteNormalMode(const FlightRoute& route, const double& north_track_rotation) = 0;

    virtual void drawRouteMapMode(const FlightRoute& route) = 0;
    
    virtual void drawHSI(const double& north_track_rotation) = 0;

    virtual void drawGeoData(const double& north_track_rotation, const QColor& color, bool filled);

    virtual void drawTcas(const double& north_track_rotation);

    virtual void drawAirport(const Airport& airport,
                             const double& north_track_rotation,
                             const QColor& airport_symbol_color,
                             bool draw_airport, 
                             bool draw_runways);

    virtual void drawVor(const Waypoint& vor,
                         bool has_dme,
                         const double& north_track_rotation,
                         const QColor& vor_symbol_color);

    virtual void drawNdb(const Ndb& ndb,
                         const double& north_track_rotation,
                         const QColor& ndb_symbol_color);

    inline void drawText(const double& x,const double& y, const QString& text) { m_font->drawText(x, y, text); }
    inline double getTextWidth(const QString& text) const { return m_font->getWidth(text); }

    virtual void drawLeftNavaid();
    virtual void drawRightNavaid();

    virtual void drawEnergyCircle(const QColor& color, bool draw_dashed);
    
protected:

    bool doWindCorrection() const;

    inline double scaleXY(const double& coord) { return coord * m_dist_scale_factor; }

    inline void moveToCorrectedProjectionPosition(const double& north_track_rotation)
    {
        glRotated(-north_track_rotation, 0, 0, 1.0);
        glTranslated(-scaleXY(m_flightstatus->current_position_smoothed.x()),
                     -scaleXY(m_flightstatus->current_position_smoothed.y()), 0.0);
        glRotated(north_track_rotation, 0, 0, 1.0);
    }

    void compileTcasGLLists();
    void compileHoldingGLLists();

    void startPerfTimer(const QString& text)
    {
        m_performance_text = text;
        m_performance_timer.start();
    }

    void stopPerfTimer()
    {
        Logger::log(QString("FMCNavdisplayStyle:stopPerfTimer: %1 -> %2ms").
                    arg(m_performance_timer.elapsed()).arg(m_performance_text));
    }

protected:

    VasGLWidget* m_parent;
    Config* m_main_config;
    Config* m_navdisplay_style_config;
    Config* m_navdisplay_config;
    Config* m_tcas_config;
    FMCControl* m_fmc_control;
    const FMCData& m_fmc_data;
    const FlightStatus* m_flightstatus;
    const ProjectionBase* m_projection;
    OpenGLText* m_font;

    double m_font_height;
    QSize m_size;
    double m_display_top_offset;
    int m_max_drawable_y;
    QPointF m_position_center;
    double m_dist_scale_factor;

    QTime m_performance_timer;
    QString m_performance_text;

    GLuint m_tcas_empty_item_gllist;
    GLuint m_tcas_full_item_gllist;
    GLuint m_airport_item_gllist;
    GLuint m_vor_dme_item_gllist;
    GLuint m_vor_wo_dme_item_gllist;
    GLuint m_ndb_item_gllist;
    GLuint m_toc_item_gllist;
    GLuint m_eod_item_gllist;
    GLuint m_tod_item_gllist;
    GLuint m_left_holding_item_gllist;
    GLuint m_right_holding_item_gllist;
    double m_last_holding_gllist_calculation_speed_kts;

    bool m_left_side;

    FMCNavdisplayStyle();
};

#endif /* __FMC_NAVDISPLAYSTYLE_H__ */

// End of file

