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

#ifndef FMC_DATA_H
#define FMC_DATA_H

#include <QObject>
#include <QTimer>
#include <QSet>

#include "logger.h"

#include "serialization_iface.h"
#include "fmc_data_provider.h"
#include "flightroute.h"
#include "airport.h"

class Config;
class Airport;
class FlightStatus;

/////////////////////////////////////////////////////////////////////////////

class FMCData : public QObject, public SerializationIface, public FMCDataProvider
{
    Q_OBJECT

public:

    static QString CHANGE_FLAG_FMC_DATA;

    FMCData(Config* config, const FlightStatus* flightstatus);
    virtual ~FMCData();

    void clear();

    virtual void operator<<(QDataStream& in);
    virtual void operator>>(QDataStream& out) const;

    //----- current route

    const FlightRoute& normalRoute() const { return m_normal_route; }
    FlightRoute& normalRoute() { return m_normal_route; }

    const FlightRoute& alternateRoute() const { return m_alternate_route; }
    FlightRoute& alternateRoute() { return m_alternate_route; }

    const FlightRoute& secondaryRoute() const { return m_secondary_route; }
    FlightRoute& secondaryRoute() { return m_secondary_route; }

    const FlightRoute& temporaryRoute() const { return m_temporary_route; }
    FlightRoute& temporaryRoute() { return m_temporary_route; }

    //----- calculated values (not synced to slaves)

    inline double distanceToActiveWptNm() const { return m_distance_to_active_wpt_nm; }
    inline void setDistanceToActiveWptNm(double distance) { m_distance_to_active_wpt_nm = distance; }

    inline double trueTrackToActiveWpt() const { return m_true_track_to_active_wpt; }
    inline void setTrueTrackToActiveWpt(double track) { m_true_track_to_active_wpt = track; }

    inline double hoursToActiveWpt() const { return m_hours_to_active_wpt; }
    inline void setHoursToActiveWpt(double hours) { m_hours_to_active_wpt = hours; }

    inline double distanceFromPreviousWptNm() const { return m_distance_from_previous_wpt_nm; }
    inline void setDistanceFromPreviousWptNm(double distance) { m_distance_from_previous_wpt_nm = distance; }

    inline double crossTrackDistanceNm() const { return m_cross_track_distance_nm; }
    inline void setCrossTrackDistanceNm(double distance) { m_cross_track_distance_nm = distance; }

    inline double distanceToTODNm() const { return m_distance_to_tod_nm; }
    inline void setDistanceToTODNm(double dist) { m_distance_to_tod_nm = dist; }

    inline double turnRadiusNm() const { return m_turn_radius_nm; }
    inline void setTurnRadiusNm(double radius) { m_turn_radius_nm = radius; }

    double hoursOverall() const { return m_normal_route.hoursActiveWptToDestination() + hoursToActiveWpt(); }
    double distanceOverallNm() const { return m_normal_route.distanceActiveWptToDestination() + distanceToActiveWptNm(); }

    //-----  surrounding stuff

    inline const WaypointPtrList& surroundingAirportList() const { return m_surrounding_airport_list; }
    inline WaypointPtrList& surroundingAirportList() { return m_surrounding_airport_list; }

    inline const WaypointPtrList& surroundingVorList() const { return m_surrounding_vor_list; }
    inline WaypointPtrList& surroundingVorList() { return m_surrounding_vor_list; }

    inline const WaypointPtrList& surroundingNdbList() const { return m_surrounding_ndb_list; }
    inline WaypointPtrList& surroundingNdbList() { return m_surrounding_ndb_list; }

    //----- speeds, performance stuff, etc.


    uint V1() const { return m_v1_kts; }
    uint Vr() const { return m_vr_kts; }
    uint V2() const { return m_v2_kts; }
    const double& takeoffTrim() const { return m_takeoff_trim; }
    uint takeoffFlapsNotch() const { return m_takeoff_flaps_notch; }
    uint landingFlapsNotch() const { return m_landing_flaps_notch; }
    uint transitionAltitudeAdep() const { return m_trans_altitude_adep; }
    uint transitionLevelAdes() const { return m_trans_level_ades; }

    const double& fmcTakeoffThrustN1() const { return m_fmc_takeoff_thrust_n1; }
    const double& fmcInitialClimbThrustN1() const { return m_fmc_climb_thrust_n1_initial; }
    const double& fmcMaxClimbThrustN1() const { return m_fmc_climb_thrust_n1_maximum; }
    uint climbSpeedKts() const { return m_climb_speed_kts; }
    const double& climbMach() const { return m_climb_mach; }
    const double& cruiseMach() const { return m_cruise_mach; }
    const double& descentMach() const { return m_descent_mach; }
    uint descentSpeedKts() const { return m_descent_speed_kts; }
    uint approachSpeedKts() const { return m_approach_speed_kts; }
    uint minDescentAltitudeFt() const { return m_minimum_descent_altitude_ft; }
    uint decisionHeightFt() const { return m_decision_height_ft; }
    virtual bool approachPhaseActive() const { return m_approach_phase_active; }

    bool isOnCruisFlightlevel() const;

    void setV1(const uint v1) 
    {
        m_v1_kts = v1; 
        emit signalDataChanged(CHANGE_FLAG_FMC_DATA, true, "FMCData:setV1");
    }

    void setVr(const uint vr) 
    {
        m_vr_kts = vr; 
        emit signalDataChanged(CHANGE_FLAG_FMC_DATA, true, "FMCData:setVr");
    }

    void setV2(const uint v2) 
    {
        m_v2_kts = v2; 
        emit signalDataChanged(CHANGE_FLAG_FMC_DATA, true, "FMCData:setV2");
    }

    void setTakeoffTrim(const double& takeoff_trim)
    {
        m_takeoff_trim = takeoff_trim;
        emit signalDataChanged(CHANGE_FLAG_FMC_DATA, true, "FMCData:setTakeoffTrim");
    }

    void setTakeoffFlapsNotch(uint takeoff_flaps_notch);

    void setLandingFlapsNotch(uint landing_flaps_notch);

    void setTransitionAltitudeAdep(const uint tl) 
    {
        m_trans_altitude_adep = tl; 
        emit signalDataChanged(CHANGE_FLAG_FMC_DATA, true, "FMCData:setTransitionAltitudeAdep");
    }

    void setTransitionLevelAdes(const uint tl) 
    {
        m_trans_level_ades = tl; 
        emit signalDataChanged(CHANGE_FLAG_FMC_DATA, true, "FMCData:setTransitionLevelAdes");
    }
    
    void setFMCTakeoffThrustN1(const double& fmc_takeoff_thrust_n1) 
    {
        m_fmc_takeoff_thrust_n1 = fmc_takeoff_thrust_n1; 
        emit signalDataChanged(CHANGE_FLAG_FMC_DATA, true, "FMCData:setFMCTakeoffThrustN1");
    }

    void setFMCInitialClimbThrustN1(const double& fmc_climb_thrust_n1_initial) 
    {
        m_fmc_climb_thrust_n1_initial = fmc_climb_thrust_n1_initial; 
        emit signalDataChanged(CHANGE_FLAG_FMC_DATA, true, "FMCData:setFMCInitialClimbThrustN1");
    }
    
    void setFMCMaxClimbThrustN1(const double& fmc_climb_thrust_n1_maximum) 
    {
        m_fmc_climb_thrust_n1_maximum = fmc_climb_thrust_n1_maximum; 
        emit signalDataChanged(CHANGE_FLAG_FMC_DATA, true, "FMCData:setFMCMaxClimbThrustN1");
    }

    void setClimbSpeedKts(uint climb_speed_kts) 
    {
        m_climb_speed_kts = climb_speed_kts; 
        emit signalDataChanged(CHANGE_FLAG_FMC_DATA, true, "FMCData:setClimbSpeedKts");
    }

    void setClimbMach(const double& climb_mach) 
    {
        m_climb_mach = climb_mach; 
        emit signalDataChanged(CHANGE_FLAG_FMC_DATA, true, "FMCData:setClimbMach");
    }

    void setCruiseMach(const double& cruise_mach) 
    {
        m_cruise_mach = cruise_mach; 
        emit signalDataChanged(CHANGE_FLAG_FMC_DATA, true, "FMCData:setCruiseMach");
    }

    void setDescentMach(const double& descent_mach) 
    {
        m_descent_mach = descent_mach; 
        emit signalDataChanged(CHANGE_FLAG_FMC_DATA, true, "FMCData:setDescentMach");
    }

    void setDescentSpeedKts(uint descent_speed_kts) 
    {
        m_descent_speed_kts = descent_speed_kts; 
        emit signalDataChanged(CHANGE_FLAG_FMC_DATA, true, "FMCData:setDescentSpeedKts");
    }
    
    void setApproachSpeedKts(uint approach_speed_kts) 
    {
        m_approach_speed_kts = approach_speed_kts; 
        emit signalDataChanged(CHANGE_FLAG_FMC_DATA, true, "FMCData:setApproachSpeedKts");
    }

    void setMinDescentAltitudeFt(uint minimum_descent_altitude_ft) 
    { 
        m_minimum_descent_altitude_ft = minimum_descent_altitude_ft; 
        m_decision_height_ft = 0;
        emit signalDataChanged(CHANGE_FLAG_FMC_DATA, true, "FMCData:setMinDescentAltitudeFt");
    }

    void setDecisionHeightFt(uint decision_height_ft) 
    {
        m_decision_height_ft = decision_height_ft; 
        m_minimum_descent_altitude_ft = 0;
        emit signalDataChanged(CHANGE_FLAG_FMC_DATA, true, "FMCData:setDecisionHeightFt");
    }

    void setApproachPhaseActive(bool yes);

signals:

    void signalDataChanged(const QString& flag, bool direct_change, const QString& comment);

    void signalApproachPhaseActivated();

protected slots:

    void slotRouteChanged(const QString& flag, bool direct_change, const QString& comment) 
    { 
        Logger::logToFileOnly(QString("FMCData:slotRouteChanged: %1/%2/%3").
                              arg(flag).arg(direct_change).arg(comment));
        m_last_changed_route_flag_set.insert(flag);
        m_last_changed_route_reason_indexed_by_flag[flag] |= direct_change;
        m_change_delay_timer.start(1);
    }

    void slotEmitDataChanged()
    {
        QSet<QString>::iterator iter = m_last_changed_route_flag_set.begin();
        for(; iter != m_last_changed_route_flag_set.end(); ++iter) 
        {
            emit signalDataChanged(*iter, m_last_changed_route_reason_indexed_by_flag[*iter], 
                                   QString("FMCData:slotEmitDataChanged: %1").arg(*iter));
        }
        m_last_changed_route_flag_set.clear();
        m_last_changed_route_reason_indexed_by_flag.clear();
    }

    void slotDestinationAirportChanged();

protected:

 	Config* m_cfg;
    const FlightStatus* m_flightstatus;

    QTimer m_change_delay_timer;
    QSet<QString> m_last_changed_route_flag_set;
    QMap<QString, bool> m_last_changed_route_reason_indexed_by_flag;

    //----- route stuff

    FlightRoute m_normal_route;
    FlightRoute m_alternate_route;
    FlightRoute m_secondary_route;
    FlightRoute m_temporary_route;

    //----- calculated values - TODO move to flightroute!!
    
    //! distance from our current position to the active waypoint
    double m_distance_to_active_wpt_nm;

    // true heading from our current position to the active waypoint
    double m_true_track_to_active_wpt;

    //! time in hours to active wpt
    double m_hours_to_active_wpt;

    //! distance from our current position to the previous waypoint
    double m_distance_from_previous_wpt_nm;

    //! cross track distance between our current position and the active leg
    double m_cross_track_distance_nm;

    //! distance to the TOD
 	double m_distance_to_tod_nm;

    //! the turn radius at the current speed
    double m_turn_radius_nm;

    //----- surrounding stuff

    WaypointPtrList m_surrounding_airport_list;
    WaypointPtrList m_surrounding_vor_list;
    WaypointPtrList m_surrounding_ndb_list;

    //----- speeds, performance stuff, etc.

    uint m_v1_kts;
    uint m_vr_kts;
    uint m_v2_kts;
    double m_takeoff_trim;
    uint m_takeoff_flaps_notch;
    uint m_landing_flaps_notch;
    uint m_trans_altitude_adep;
    uint m_trans_level_ades;

    double m_fmc_takeoff_thrust_n1;
    double m_fmc_climb_thrust_n1_initial;
    double m_fmc_climb_thrust_n1_maximum;
    uint m_climb_speed_kts;
    double m_climb_mach;
    double m_cruise_mach;
    double m_descent_mach;
    uint m_descent_speed_kts;
    uint m_approach_speed_kts;
    uint m_minimum_descent_altitude_ft;
    uint m_decision_height_ft;

    bool m_approach_phase_active;
};

#endif
