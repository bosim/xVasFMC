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

/*! \file    controller_speed.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __CONTROLLER_SPEED_H__
#define __CONTROLLER_SPEED_H__

#include "trend.h"

#include "controller_base.h"

class ControllerThrottle;

/////////////////////////////////////////////////////////////////////////////

//! speed controller
class ControllerSpeed : public ControllerBase
{
public:
    //! Standard Constructor
    ControllerSpeed(const FlightStatus& flighstatus, 
                    ControllerThrottle* controller_throttle);

    //! Destructor
    virtual ~ControllerSpeed() {};

    //-----

    void setMaxTrend(const double max_trend) { m_max_trend = max_trend; }
    const double getMaxTrend() const { return m_max_trend; }

    void setTrendBoostFactor(const double trend_boost_factor) { m_trend_boost_factor = trend_boost_factor; }
    const double getTrendBoostFactor() const { return m_trend_boost_factor; }

    void setWrongDirectionTrendBoostFactor(const double wrong_direction_trend_boost_factor) { m_wrong_direction_trend_boost_factor = wrong_direction_trend_boost_factor; }
    const double getWrongDirectionTrendBoostFactor() const { return m_wrong_direction_trend_boost_factor; }

    void setRateFactor(const double rate_factor) { m_rate_factor = rate_factor; }
    const double getRateFactor() const { return m_rate_factor; }

    //-----

    virtual const double& output(const double& input);

    //-----

    const double& currentN1Limit() const { return m_n1_limit; }
    void setN1Limit(const double& n1_limit) { m_n1_limit = n1_limit; }

protected:

    double m_max_trend;
    double m_trend_boost_factor;
    double m_wrong_direction_trend_boost_factor;
    double m_rate_factor;

    Trend<double> m_trend;

    ControllerThrottle* m_controller_throttle;
    bool m_controller_throttle_active;

    double m_n1_limit;

private:
    //! Hidden copy-constructor
    ControllerSpeed(const ControllerSpeed&);
    //! Hidden assignment operator
    const ControllerSpeed& operator = (const ControllerSpeed&);
};

#endif /* __CONTROLLER_SPEED_H__ */

// End of file

