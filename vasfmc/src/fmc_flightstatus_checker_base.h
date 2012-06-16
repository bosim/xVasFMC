///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2008 Alexander Wemmer 
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

/*! \file    fmc_flighstatus_checker_base.h
    \author  Alexander Wemmer, alex@wemmer.at
*/


#ifndef __FMC_FLIGHSTATUS_CHECKER_BASE_H__
#define __FMC_FLIGHSTATUS_CHECKER_BASE_H__

#include "logger.h"
#include "assert.h"

#include "flightstatus.h"
#include "fsaccess.h"

class FMCControl;

/////////////////////////////////////////////////////////////////////////////

//! style dependent flight status checker
class FlightStatusCheckerBase : public QObject
{
    Q_OBJECT

public:
    //! Standard Constructor
    FlightStatusCheckerBase(FlightStatus* flightstatus, const FMCControl* fmc_control);

    //! Destructor
    virtual ~FlightStatusCheckerBase() {};

    //-----

    virtual void beforeChecks(FSAccess& fsaccess);
    virtual void doChecks(FSAccess& fsaccess);
    virtual void afterChecks(FSAccess& fsaccess);

    bool isAltitudeDeviationAlert() const { return m_is_altitude_deviation; }
    double altitudeDeviationFt() const { return m_altitude_deviation_ft; }

    bool wasAirborneSinceLastEngineOff() const { return m_was_airborne_since_last_engine_off; }
    bool wasAbove1000FtSinceLastOnGroundTaxi() const { return m_was_above_1000ft_since_last_onground_taxi; }
    uint secondsSinceLastEngingeStart() const { return (uint)(m_time_since_last_engine_start.elapsed() / 1000.0); }
    bool thrustLeverClimbDetentRequest() const { return m_thrust_lever_climb_detent_request; }

    bool hasAltimeterWrongSetting(bool left_side) const 
    { return (left_side) ? m_altimeter_has_wrong_setting_left : m_altimeter_has_wrong_setting_right; }

signals:

    void signalReachingAltitude();
    void signal1000FtToGo();

protected:

    bool m_init;

    FlightStatus* m_flightstatus;
    const FMCControl* m_fmc_control;

    double m_last_ap_alt;
    QTime m_ap_alt_last_change_time;

    bool m_is_altitude_deviation;
    double m_altitude_deviation_ft;

    double m_abs_alt_diff;
    bool m_do_1000_to_go_callout;

    QTime m_time_since_last_engine_start;
    bool m_was_airborne_since_last_engine_off;
    bool m_was_above_1000ft_since_last_onground_taxi;

    bool m_thrust_lever_climb_detent_request;

    bool m_altimeter_has_wrong_setting_left;
    bool m_altimeter_has_wrong_setting_right;

    bool m_was_approaching_and_spoilers_armed;
    bool m_spoilers_are_managed;

private:
    //! Hidden copy-constructor
    FlightStatusCheckerBase(const FlightStatusCheckerBase&);
    //! Hidden assignment operator
    const FlightStatusCheckerBase& operator = (const FlightStatusCheckerBase&);
};

#endif /* __FMC_FLIGHSTATUS_CHECKER_BASE_H__ */

// End of file

