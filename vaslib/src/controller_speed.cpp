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

/*! \file    controller_speed.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/


#include "controller_speed.h"
#include "controller_throttle.h"

/////////////////////////////////////////////////////////////////////////////

ControllerSpeed::ControllerSpeed(const FlightStatus& flighstatus,
                                 ControllerThrottle* controller_throttle) :
    ControllerBase(flighstatus, 0.0, 100.0), 
    m_max_trend(2.0), m_trend_boost_factor(12.0), m_wrong_direction_trend_boost_factor(1.5), m_rate_factor(14.0),
    m_trend(0.0, 0.0), 
    m_controller_throttle(controller_throttle), m_controller_throttle_active(false), m_n1_limit(100.0)
{
    MYASSERT(m_controller_throttle != 0);
    setOutput(m_flightstatus.engine_data[1].throttle_lever_percent);
}

/////////////////////////////////////////////////////////////////////////////

const double& ControllerSpeed::output(const double& input)
{
    if (!m_init)
    {
        setOutput(m_flightstatus.engine_data[1].throttle_lever_percent);
        m_last_call_dt.start();
        m_init = true;
    }

    double call_time_elapsed_s = qMin(m_last_call_dt.elapsed(), 1000) / 1000.0;

    double trend = m_trend.trendPerSecond(input);
    double diff = m_target - (input + trend * m_trend_boost_factor);
    
    if (diff * trend > 0.0)
    {
        if (qAbs(trend) > m_max_trend) diff = 0.0;
        else                           diff *= 1 - (qAbs(trend) / m_max_trend);
    }

    diff = qMax(-10.0, qMin(10.0, diff));

    // boost when the trend points into the wrong direction
    if (diff * trend < 0.0 && qAbs(trend) > m_max_trend*0.3) diff *= m_wrong_direction_trend_boost_factor;

    //----- apply N1 limit

    double n1_trend;
    double n1 = m_flightstatus.engine_data[1].smoothedN1(&n1_trend);

    if (diff >= 0 && (n1 >= m_n1_limit || n1 + n1_trend * 2.0 >= m_n1_limit))
    {
//         Logger::log(QString("ControllerSpeed:output: limiting %1 by N1 limit %2 (target=%3, n1_trend=%4)").
//                     arg(m_output, 1).arg(m_n1_limit, 1).arg(m_target).arg(n1_trend, 3));

        if (!m_controller_throttle_active)
        {
            // make a smooth transition to the N1 controller
            m_controller_throttle->setOutput(m_output);
            m_controller_throttle_active = true;
        }

        m_controller_throttle->setTarget(m_n1_limit);
        m_output = m_controller_throttle->output(n1);
    }
    else
    {
        m_controller_throttle_active = false;
        m_output += (diff / m_rate_factor) * 8.0 * call_time_elapsed_s;
        m_output = qMax(m_min_output, qMin(m_max_output, m_output));
    }

//     Logger::log(QString("target=%1  in=%2  trend=%3  diff=%4  out=%5 (max %6)").
//                 arg(m_target, 1, 'f', 1).arg(input, 1, 'f', 1).arg(trend, 1, 'f', 1).
//                 arg(diff, 1, 'f', 1).arg(m_output, 1, 'f', 1).arg(m_max_output, 1, 'f', 1));

    m_last_call_dt.start();
    return m_output;
}

// End of file
