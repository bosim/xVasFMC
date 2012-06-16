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

/*! \file    smoothing.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef SMOOTHING_H
#define SMOOTHING_H

#include <QDateTime>

#include "logger.h"
#include "median.h"
#include "navcalc.h"
#include "meanvalue.h"
#include "median.h"

/////////////////////////////////////////////////////////////////////////////

//! loww pass filter
template <class TYPE> class LowPass
{
public:

    LowPass(const TYPE& time_constant, const TYPE& init_value) : 
        m_first_update(true), m_time_constant(time_constant), 
        m_init_value(init_value), m_current_value(init_value) {}

    virtual ~LowPass() {}

    void clear()
    {
        m_current_value = m_init_value;
    }

    //! the bigger the time constant, the slower the output signal
    void setTimeConstant(const TYPE& time_constant) { m_time_constant = time_constant; }

    TYPE nextValue(const TYPE& new_value)
    {
        if (m_first_update)
        {
            m_first_update = false;
            m_current_value = new_value;
        }
        else
        {
            double delta_t = qMax(0.0001, m_update_dt.elapsed() / 1000.0);
            m_current_value = m_current_value + ((delta_t / (m_time_constant + delta_t)) * (new_value - m_current_value));
        }

        m_update_dt.start();
        return m_current_value;
    }

protected:
    
    bool m_first_update;
    TYPE m_time_constant;
    QTime m_update_dt;
    TYPE m_init_value;
    TYPE m_current_value;
};

/////////////////////////////////////////////////////////////////////////////

template <class TYPE> class ValueWithTimeStamp
{
public:

    ValueWithTimeStamp(const TYPE& value)
    {
        m_value = value;
        m_dt = QTime::currentTime();
    }

    inline const TYPE& value() const { return m_value; }
    inline const QTime& dt() const { return m_dt; }

protected:

    TYPE m_value;
    QTime m_dt;
};

/////////////////////////////////////////////////////////////////////////////

//! smoothed value with delay
template <class TYPE> class SmoothedValueWithDelay
{
public:

    SmoothedValueWithDelay(bool is_heading, bool is_coordinate, uint length, uint delay_ms, bool calc_trend = false) : 
        m_is_heading(is_heading), m_is_coordinate(is_coordinate), m_calc_trend(calc_trend), 
        m_length(length), m_delay_ms(delay_ms), m_trend_median(2*length, 0.0), m_trend_median_mean(2*length, 0.0),
        m_do_smoothing_mean(false), m_smoothing_mean(length, 0.0), 
        m_do_low_pass(false), m_low_pass(0.0, 0.0), m_do_trend_low_pass(false), m_trend_low_pass(0.0, 0.0),
        m_verbose(false)
    {}
    
    virtual ~SmoothedValueWithDelay() {}

    //! sets the name used for logging
    void setName(const QString& name) { m_name = name; }

    inline void doSmoothingMean(bool yes) { m_do_smoothing_mean = yes; }

    inline void doLowPass(bool yes, const TYPE& time_constant) 
    { 
        m_do_low_pass = yes;
        m_low_pass.setTimeConstant(time_constant);        
    }

    inline void doTrendLowPass(bool yes, const TYPE& time_constant) 
    { 
        m_do_trend_low_pass = yes;
        m_trend_low_pass.setTimeConstant(time_constant);        
    }

    inline void setDelayMs(uint delay_ms) { m_delay_ms = delay_ms; }

    inline void setVerbose(bool verbose) { m_verbose = verbose; }

    inline void clear()
    {
        if (!m_name.isEmpty())
            Logger::log(QString("SmoothedValueWithDelay:clear(%1)").arg(m_name));

        m_value_list.clear();
        m_trend_median.clear();
        m_trend_median_mean.clear();
        m_smoothing_mean.clear();
        m_low_pass.clear();
        m_trend_low_pass.clear();
    }

    TYPE value(TYPE* trend_per_second = 0) const
    {
        if (m_value_list.count() < 1) 
        {
            if (!m_name.isEmpty())
                Logger::logToFileOnly(QString("SmoothedValueWithDelay:value(%1): count < 1").arg(m_name));
            return 0;
        }

        const ValueWithTimeStamp<TYPE>& newest_value = m_value_list.last();
        if (m_value_list.count() < 2) 
        {
            if (!m_name.isEmpty())
                Logger::logToFileOnly(QString("SmoothedValueWithDelay:value(%1): count < 2 -> newest value").arg(m_name));
            return newest_value.value();
        }

        QTime wanted_dt = QTime::currentTime().addMSecs(-m_delay_ms);
        if (newest_value.dt() < wanted_dt) 
        {
            if (!m_name.isEmpty())
                Logger::logToFileOnly(QString("SmoothedValueWithDelay:value(%1): " 
                                              "newest value (%2) < wanted_dt (%3) (cur=%4, del=%5) -> newest value").
                                      arg(m_name).arg(newest_value.dt().toString("hh:mm:ss:zzz")).
                                      arg(wanted_dt.toString("hh:mm:ss:zzz")).
                                      arg(QTime::currentTime().toString("hh:mm:ss:zzz")).
                                      arg(m_delay_ms));
            return newest_value.value();
        }

        // get value older than the reference time

        int index = m_value_list.count() - 1;
        MYASSERT(index > 0);

        while(wanted_dt < m_value_list[index].dt())
        {
            if (index == 0)
            {
                if (!m_name.isEmpty())
                    Logger::logToFileOnly(QString("SmoothedValueWithDelay:value(%1): "
                                        "wanted_dt (%2) < oldest value (%3) -> newest value").
                                arg(m_name).arg(wanted_dt.toString("hh:mm:ss:zzz")).
                                arg(m_value_list[index].dt().toString("hh:mm:ss:zzz")));
                return newest_value.value();
            }
            --index;
        }

        // interpolate value

        int timediff = m_value_list[index].dt().msecsTo(newest_value.dt());
        TYPE per_time_change = 0;

        if (timediff > 0) 
        {
            TYPE absolut_change = newest_value.value() - m_value_list[index].value();

            if (m_is_heading || m_is_coordinate)
            {
                if (absolut_change > 180.0) absolut_change -= 360.0;
                else if (absolut_change < - 180.0) absolut_change += 360;
            }

            per_time_change = absolut_change / timediff;
            
            if (m_verbose)
                Logger::log(QString("new=%1 old=%2 abs=%3 timediff=%4 pertime=%5").
                            arg(newest_value.value()).arg(m_value_list[index].value()).
                            arg(absolut_change).arg(timediff).arg(per_time_change));
        }

        if (m_calc_trend)
        {
            m_trend_median.add(per_time_change * 1000.0);
            m_trend_median_mean.add(m_trend_median.median());
            if (trend_per_second != 0) 
            {
                *trend_per_second = m_trend_median_mean.mean();

                if (m_do_trend_low_pass)
                    *trend_per_second = m_trend_low_pass.nextValue(*trend_per_second);
            }
        }
        else 
        {
            MYASSERT(trend_per_second == 0);
        }
        
        double correction_value = per_time_change * m_value_list[index].dt().msecsTo(wanted_dt);

        if (m_verbose)
            Logger::log(QString("refdtdiff=%1 correction=%2").
                        arg(m_value_list[index].dt().msecsTo(wanted_dt)).arg(correction_value));

        TYPE ret;
        if (m_is_heading) ret= Navcalc::trimHeading(m_value_list[index].value() + correction_value);
        ret = m_value_list[index].value() + correction_value;

        if (m_do_smoothing_mean) 
        {
            m_smoothing_mean.add(ret);
            ret = m_smoothing_mean.mean();
        }

        if (m_do_low_pass)
            ret = m_low_pass.nextValue(ret);

        return ret;
    }

    void operator=(const TYPE& value)
    {
        m_value_list.append(ValueWithTimeStamp<TYPE>(value));
        if (m_value_list.count() > (int)m_length) m_value_list.removeFirst();

    }

    const TYPE lastValue() const 
    {
        if (m_value_list.isEmpty()) return 0;
        return m_value_list.last().value(); 
    }

protected:

    bool m_is_heading;
    bool m_is_coordinate;
    bool m_calc_trend;
    uint m_length;
    uint m_delay_ms;
    QList< ValueWithTimeStamp<TYPE> > m_value_list;
    mutable Median<TYPE> m_trend_median;
    mutable MeanValue<TYPE> m_trend_median_mean;

    bool m_do_smoothing_mean;
    mutable MeanValue<TYPE> m_smoothing_mean;

    bool m_do_low_pass;
    mutable LowPass<TYPE> m_low_pass;

    bool m_do_trend_low_pass;
    mutable LowPass<TYPE> m_trend_low_pass;

    bool m_verbose;
    QString m_name;
};

/////////////////////////////////////////////////////////////////////////////

//! smoothed value
// template <class TYPE> class SmoothedValue
// {
// public:

//     SmoothedValue(const TYPE& init_value, bool is_heading_value)
//     {
//         m_update_count = 0;
//         m_is_heading_value = is_heading_value;
//         m_current_value = m_previous_value = init_value;
//         m_change_speed = 0.0;
//     }
    
//     virtual ~SmoothedValue() {}

//     const TYPE value() const
//     {
//         if (m_update_count < 2) return m_current_value;
//         int time_diff_ms = qMin(m_current_value_dt.msecsTo(QTime::currentTime()), 1000);
//         if (m_is_heading_value) return Navcalc::trimHeading(m_current_value + m_change_speed * time_diff_ms);
//         return m_current_value + m_change_speed * time_diff_ms;
//     }

//     void operator=(const TYPE& value)
//     {
//         ++m_update_count;
        
//         m_previous_value = m_current_value;
//         m_previous_value_dt = m_current_value_dt;
        
//         m_current_value = value;
//         m_current_value_dt = QTime::currentTime();
        
//         if (m_update_count < 2) return;
        
//         int time_diff_ms = m_previous_value_dt.msecsTo(m_current_value_dt);
//         if (time_diff_ms > 0 ) m_change_speed = (m_current_value - m_previous_value) / time_diff_ms;
//     }
    
// protected:

//     int m_update_count;
//     bool m_is_heading_value;

//     TYPE m_current_value;
//     QTime m_current_value_dt;

//     TYPE m_previous_value;
//     QTime m_previous_value_dt;

//     double m_change_speed;
// };

// /////////////////////////////////////////////////////////////////////////////

// //! smoothed value
// template <class TYPE> class SmoothedValueWithMedian
// {
// public:

//     SmoothedValueWithMedian(const TYPE& init_value, bool is_heading_value, uint median_length) :
//         m_change_speed_median(median_length, init_value)
//     {
//         m_update_count = 0;
//         m_is_heading_value = is_heading_value;
//         m_current_value = m_previous_value = init_value;
//     }
    
//     virtual ~SmoothedValueWithMedian() {}

//     const TYPE value() const
//     {
//         if (m_update_count < m_change_speed_median.length()) return m_current_value;

//         int time_diff_ms = qMin(m_current_value_dt.msecsTo(QTime::currentTime()), 1000);
//         if (m_is_heading_value) return Navcalc::trimHeading(m_current_value + m_change_speed_median.median() * time_diff_ms);
//         return m_current_value + m_change_speed_median.median() * time_diff_ms;
//     }

//     void operator=(const TYPE& value)
//     {
//         ++m_update_count;
        
//         m_previous_value = m_current_value;
//         m_previous_value_dt = m_current_value_dt;
        
//         m_current_value = value;
//         m_current_value_dt = QTime::currentTime();
  
//         if (m_update_count < 2) return;
        
//         int time_diff_ms = m_previous_value_dt.msecsTo(m_current_value_dt);
//         double change = m_current_value - m_previous_value;

//         if (time_diff_ms > 0 ) m_change_speed_median.add(change / time_diff_ms);
//     }

// protected:

//     uint m_update_count;
//     bool m_is_heading_value;

//     TYPE m_current_value;
//     QTime m_current_value_dt;

//     TYPE m_previous_value;
//     QTime m_previous_value_dt;

//     Median<double> m_change_speed_median;
// };


/////////////////////////////////////////////////////////////////////////////

//! damping, can be used to calculate a linear damping factor dependend on an
//! damping start limit and damping end limit. The damping factor will be 1.0
//! until the damping start and will then be interpolated down to reach 0.0 at
//! the damping end value.
class Damping 
{
public:
    //! Standard Constructor
    Damping() {};

    //! Destructor
    virtual ~Damping() {};

    void setDampBorders(const double& x_damp_start, const double& x_damp_end);

    double dampFactor(const double& x);

protected:

    double m_x_damp_start;
    double m_x_damp_end;
    double m_k;
    double m_d;

private:
    //! Hidden copy-constructor
    Damping(const Damping&);
    //! Hidden assignment operator
    const Damping& operator = (const Damping&);
};

#endif /* SMOOTHING_H */

// End of file

