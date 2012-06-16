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

/*! \file    trend.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef TREND_H
#define TREND_H

#include <QTime>

#include "logger.h"

/////////////////////////////////////////////////////////////////////////////

template <class TYPE> class Trend
{
public:

    Trend(const TYPE& default_value, const TYPE& init_trend) : 
        m_init(false), m_last_value(default_value), m_last_trend(init_trend)
    {};

    ~Trend() {};

    //-----

    const TYPE& trendPerSecond(const TYPE& next_value)
    {
        if (!m_init)
        {
            m_init = true;
            m_last_value = next_value;
            m_last_value_dt.start();
            return m_last_trend;
        }

        m_last_trend = (next_value - m_last_value) / m_last_value_dt.elapsed() * 1000;

        m_last_value_dt.start();
        m_last_value = next_value;
        return m_last_trend;
    }

    const TYPE& getLastTrendPerSecond() const { return m_last_trend; }

protected:

    bool m_init;
    TYPE m_last_value;
    QTime m_last_value_dt;
    TYPE m_last_trend;
};

#endif /* TREND_H */

// End of file

