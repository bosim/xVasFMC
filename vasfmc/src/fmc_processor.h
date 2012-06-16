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

/*! \file    fmc_processor.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __FMC_PROCESSOR_H__
#define __FMC_PROCESSOR_H__

#include <QObject>
#include <QTime>

class FMCData;
class FlightStatus;
class ProjectionBase;
class Navdata;
class FMCControl;
class Config;
class ConfigWidgetProvider;

/////////////////////////////////////////////////////////////////////////////

//! FMC data processor
class FMCProcessor : public QObject
{
    Q_OBJECT

public:
    //! Standard Constructor
    FMCProcessor(ConfigWidgetProvider* config_widget_provider, const QString& processor_cfg_filename, 
                 FMCControl* fmc_control, Navdata* navdata);

    //! Destructor
    virtual ~FMCProcessor();

    //! returns a const pointer to the used LAT/LON -> X/Y projection
    inline const ProjectionBase* projection() const { return m_projection; }

signals:

    void signalTimeUsed(const QString& name, uint millisecs);

public slots:

    void slotRefresh(bool force);

    void slotDataChanged(const QString&, bool, const QString&) { m_data_changed = true; }

protected:

    void clearCalculatedValues();

    void setupDefaultConfig();

protected:

    ConfigWidgetProvider* m_config_widget_provider;
    Config* m_processor_cfg;
    FMCData& m_fmc_data;
    FMCControl* m_fmc_control;
    FlightStatus* m_flightstatus;
    ProjectionBase* m_projection;
    Navdata* m_navdata;

    bool m_flightstatus_was_valid_once;

    QTime  m_refresh_timer;

    QTime  m_descent_estimate_recalc_timer;

    QTime  m_wpt_times_recalc_timer;
    int    m_wpt_times_recalc_wpt_index;

    QTime  m_alt_reach_recalc_timer;
    bool   m_data_changed;

    double m_last_toc_eod_ground_speed_kts;
    double m_last_toc_eod_vs_ftmin;
    double m_last_toc_eod_ap_diff_alt;

    double m_last_descend_distance_nm;

    int m_project_recalc_index_route_normal;
    int m_project_recalc_index_route_temporary;
    int m_project_recalc_index_route_alternate;
    int m_project_recalc_index_route_secondary;

    int m_project_recalc_airports;
    int m_project_recalc_vors;
    int m_project_recalc_ndbs;
    int m_project_recalc_geo;

    int m_normal_route_view_wpt_index;

    //TODO for testing
    int m_next_wpt_with_alt_constraint_index;

private:
    //! Hidden copy-constructor
    FMCProcessor(const FMCProcessor&);
    //! Hidden assignment operator
    const FMCProcessor& operator = (const FMCProcessor&);
};

#endif /* __FMC_PROCESSOR_H__ */

// End of file

