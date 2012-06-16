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

/*! \file    controller_throttle.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/


#include "controller_throttle.h"

/////////////////////////////////////////////////////////////////////////////

ControllerThrottle::ControllerThrottle(const FlightStatus& flighstatus) :
    ControllerBase(flighstatus, 0.0, 100.0), 
    m_max_rate(7.0), m_trend_boost_factor(4.0), m_rate_factor(13.0), 
    m_trend(0.0, 0.0), m_n1_limit(100.0), m_do_fast_idling(true), m_rate_reduce_factor(1.0)
{
    setOutput(m_flightstatus.engine_data[1].throttle_lever_percent);
}

/////////////////////////////////////////////////////////////////////////////

const double& ControllerThrottle::output(const double& input)
{
    if (!m_init)
    {
        setOutput(m_flightstatus.engine_data[1].throttle_lever_percent);
        m_last_call_dt.start();
        m_init = true;
    }

    double call_time_elapsed_s = qMin(m_last_call_dt.elapsed(), 1000) / 1000.0;
    double limited_target = qMin(m_target, m_n1_limit);

    double trend = m_trend.trendPerSecond(input);
    double diff = limited_target - (input + (trend * m_trend_boost_factor));

    if (diff * trend > 0.0) diff *= 1 - (qAbs(trend) / m_max_rate);

    m_output += (diff / (m_rate_factor * m_rate_reduce_factor)) * 8.0 * call_time_elapsed_s;
    m_output = qMax(m_min_output, qMin(m_max_output, m_output));

    // if we are idle pull back the lever immediately
    if (m_do_fast_idling && m_target <= 0.1) m_output = 0.0;
    // if we are not idle advance the lever at least 0.1%
    else if (m_target > 0.01) m_output = qMax(0.1, m_output); 
    else m_output = qMax(0.0, m_output); 

//     Logger::log(QString("target=%1  in=%2  trend=%3  diff=%4  out=%5").
//                 arg(limited_target, 1, 'f', 1).arg(input, 1, 'f', 1).arg(trend, 1, 'f', 1).
//                 arg(diff, 1, 'f', 1).arg(m_output, 1, 'f', 1));

    m_last_call_dt.start();
    return m_output;
}

// End of file
