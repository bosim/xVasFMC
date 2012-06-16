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

/*! \file    controller_throttle.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __CONTROLLER_THROTTLE_H__
#define __CONTROLLER_THROTTLE_H__

#include "trend.h"

#include "controller_base.h"

/////////////////////////////////////////////////////////////////////////////

//! throttle controller
class ControllerThrottle : public ControllerBase
{
public:
    //! Standard Constructor
    ControllerThrottle(const FlightStatus& flighstatus);

    //! Destructor
    virtual ~ControllerThrottle() {};

    //-----

    void setMaxRate(const double max_rate) { m_max_rate = max_rate; }
    const double getMaxRate() const { return m_max_rate; }

    void setTrendBoostFactor(const double trend_boost_factor) { m_trend_boost_factor = trend_boost_factor; }
    const double getTrendBoostFactor() const { return m_trend_boost_factor; }

    void setRateFactor(const double rate_factor) { m_rate_factor = rate_factor; }
    const double getRateFactor() const { return m_rate_factor; }
    
    //-----

    virtual const double& output(const double& input);

    //-----

    const double& currentN1Limit() const { return m_n1_limit; }
    void setN1Limit(const double& n1_limit) { m_n1_limit = n1_limit; }

    void setDoFastIdling(bool yes) { m_do_fast_idling = yes; }
    double rateReduceFactor() const { return m_rate_reduce_factor; }
    void setRateReduceFactor(const double& factor) { m_rate_reduce_factor = qMax(factor, 1.0); }

protected:

    double m_max_rate;
    double m_trend_boost_factor;
    double m_rate_factor;

    Trend<double> m_trend;
    double m_n1_limit;

    bool m_do_fast_idling;
    double m_rate_reduce_factor;

private:
    //! Hidden copy-constructor
    ControllerThrottle(const ControllerThrottle&);
    //! Hidden assignment operator
    const ControllerThrottle& operator = (const ControllerThrottle&);
};

#endif /* __CONTROLLER_THROTTLE_H__ */

// End of file

