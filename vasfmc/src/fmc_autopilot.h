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

/*! \file    fmc_autopilot.h
  \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __FMC_AUTOPILOT_H__
#define __FMC_AUTOPILOT_H__

#include <QObject>
#include <QTime>
#include <QTimer>

#include "config.h"
#include "serialization_iface.h"

#include "fmc_autopilot_defines.h"

class FMCControl;
class FMCData;
class FlightStatus;
class Waypoint;
class ConfigWidgetProvider;

/////////////////////////////////////////////////////////////////////////////

//! FMC Autopilot Control
class FMCAutopilot : public QObject, public SerializationIface
{
    Q_OBJECT

public:

    enum LATERAL_MODE { LATERAL_MODE_NONE = 0,
                        LATERAL_MODE_HDG,
                        LATERAL_MODE_LNAV,
                        LATERAL_MODE_LOC,
                        LATERAL_MODE_LOC_CAPTURE,
                        LATERAL_MODE_VOR,
                        LATERAL_MODE_TAKEOFF,
                        LATERAL_MODE_LANDING,
                        LATERAL_MODE_FLARE,
                        LATERAL_MODE_ROLLOUT
    };

    enum VERTICAL_MODE { VERTICAL_MODE_NONE = 0,
                         VERTICAL_MODE_ALT,
                         VERTICAL_MODE_ALT_CAPTURE,
                         VERTICAL_MODE_ALTCRZ,
                         VERTICAL_MODE_GS,
                         VERTICAL_MODE_GS_CAPTURE,
                         VERTICAL_MODE_VS,
                         VERTICAL_MODE_FLCH,
                         VERTICAL_MODE_VNAV,
                         VERTICAL_MODE_FPV,
                         VERTICAL_MODE_TAKEOFF,
                         VERTICAL_MODE_LANDING,
                         VERTICAL_MODE_FLARE,
                         VERTICAL_MODE_ROLLOUT
    };

    enum ILS_MODE { ILS_MODE_NONE,
                    ILS_MODE_CAT1,
                    ILS_MODE_CAT2
    };

    //! Standard Constructor
    FMCAutopilot(ConfigWidgetProvider* config_widget_provider,
                 Config* main_config,
                 const QString& autopilot_cfg_filename,
                 FMCControl* fmc_control);
    
    //! Destructor
    virtual ~FMCAutopilot();

    virtual void operator<<(QDataStream& in);
    virtual void operator>>(QDataStream& out) const;

    //----- ILS mode

    ILS_MODE ilsMode() const { return m_ils_mode; }

    //----- lateral stuff

    bool isTakeoffModeActiveLateral() const { return m_takeoff_active_lateral; }
    void setTakeoffModeLateral(bool yes);

    LATERAL_MODE lateralModeActive() const { return m_lateral_mode_active; }
    LATERAL_MODE lateralModeArmed() const { return m_lateral_mode_armed; }
    uint lateralModeActiveChangeTimeMs() const;

    bool isNAVCoupled() const;
    void setNAVCouple(bool yes);
    double lastNAVCalculatedHeading() const { return m_last_nav_calculated_heading; }

    bool isNAVHoldActive() const;
    void setNAVHold();

    bool isHeadingHoldActive() const;
    void setHeadingHold(bool set_current_hdg);

    void setAPPHold();
    bool isAPPHoldActive() const;
    void setLOCHold();
    bool isLOCHoldActive() const;

    //----- vertical stuff

    bool isTakeoffModeActiveVertical() const { return m_takeoff_active_vertical; }
    void setTakeoffModeVertical(bool yes);

    VERTICAL_MODE verticalModeActive() const { return m_vertical_mode_active; }
    VERTICAL_MODE verticalModeArmed() const { return m_vertical_mode_armed; }
    uint verticalModeActiveChangeTimeMs() const;

    // ALT mode

    void setALTHold(bool set_current_alt);
    bool isALTHoldActive() const;

    // FPV / VS mode

    void setVSFPAHold(bool zero_vs);
    void enableFlightPathMode(bool yes);

    bool isVsModeEnabled() const;
    bool isFlightPathModeEnabled() const;

    bool isVsModeActive() const;
    bool isFlightPathModeActive() const;

    double verticalSpeed() const { return m_vertical_speed; }
    double flightPath() const { return m_flightpath_angle; }

    void setVerticalSpeed(int vs);
    void setFlightPathAngle(const double& fpv);

    // FLCH mode

    bool isFLChangeModeActive() const;
    void setFLChangeMode(bool yes, bool set_spd_to_current);
    double FLChangeIASTrendTarget() const { return m_flch_mode_ias_trend_target; }    

public slots:

    void slotRefresh();

    void slotToggleFlightDirector();
    void slotToggleAutopilot();

    void slotToggleNAVCouple() { setNAVCouple(!isNAVCoupled()); }
    void slotToggleFlightPathMode() { enableFlightPathMode(!isFlightPathModeEnabled()); }

    void slotSwitchLateralFlightDirectorToExternalSource();
    void slotSwitchVerticalFlightDirectorToExternalSource();

protected:

    void setupDefaultConfig();

    //! init
    void initValues();

    //! calculates the flight director state at certain modes
    void calculateFlightDirector();

    //! calculates and sets the lateral autopilot control
    void calculateLateral();

    //! calculates and sets the lateral autopilot control
    void calculateVertical();

    //! calculates the lateral AP mode
    void calculateLateralMode();

    //! calculates the vertical AP mode
    void calculateVerticalMode();

    //! returns the AP heading for the given active wpt, which has assigned a holding
    double getHeadingForHolding(Waypoint& active_wpt) const;

    //! returns the AP heading to intercept the track defined by the given from and to waypoints.
    double getInterceptHeadingToTrack(Waypoint& from_wpt, const Waypoint& to_wpt) const;

    //----- internal setter

    void setFlightPathAngleInternal(const double& fpv);    
    void setVerticalSpeedInternal(int vs);

protected:

    Config* m_main_config;
    Config* m_autopilot_config;

    bool m_values_initialized;

    FMCControl* m_fmc_control;
    FMCData& m_fmc_data;
    FlightStatus* m_flightstatus;

    ILS_MODE m_ils_mode;

    QTime m_refresh_detector;

    QTime m_lateral_mode_calc_timer;
    LATERAL_MODE m_lateral_mode_active;
    LATERAL_MODE m_lateral_mode_armed;
    QTime m_lateral_mode_active_changed_time;

    QTime m_vertical_mode_calc_timer;
    VERTICAL_MODE m_vertical_mode_active;
    VERTICAL_MODE m_vertical_mode_armed;
    QTime m_vertical_mode_active_changed_time;

    double m_flightpath_angle;
    int m_vertical_speed;

    bool m_mode_fl_change_active;
    double m_fl_change_cmd_vs;
    double m_flch_mode_ias_trend_target;

    bool m_do_set_vs_to_ap;
    bool m_do_reset_after_landing;

    //----- takeoff mode stuff

    bool m_takeoff_active_lateral;
    LATERAL_MODE m_takeoff_lateral_mode_armed;
    bool m_takeoff_active_vertical;
    VERTICAL_MODE m_takeoff_vertical_mode_armed;

    double m_takeoff_lateral_target_track;

    double m_takeoff_vertical_speed_hold_kts;
    QTime m_takeoff_vertical_speed_hold_engaged_dt;

    QTimer m_lateral_fd_source_reset_timer;
    QTimer m_vertical_fd_source_reset_timer;

    //----- master FMC values (values not listed here will be set directly when in slave mode)

    uint m_master_fmc_lateral_mode_active_change_time_ms;
    bool m_master_fmc_nav_coupled;
    double m_last_nav_calculated_heading;
    bool m_master_fmc_nav_hold_active;
    bool m_master_fmc_hdg_hold_active;
    uint m_master_fmc_vertical_mode_active_change_time_ms;
    bool m_master_fmc_vs_mode_enabled;
    bool m_master_fmc_fpa_mode_enabled;
    bool m_master_fmc_vs_mode_active;
    bool m_master_fmc_fpa_mode_active;

private:
    //! Hidden copy-constructor
    FMCAutopilot(const FMCAutopilot&);
    //! Hidden assignment operator
    const FMCAutopilot& operator = (const FMCAutopilot&);
};



#endif /* __FMC_AUTOPILOT_H__ */

// End of file

