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

/*! \file    fly_by_wire.h
  \author  Mark Foti and Alexander Wemmer, alex@wemmer.at
*/

#ifndef __FLY_BY_WIRE_H__
#define __FLY_BY_WIRE_H__

#include <QDateTime>
#include <QObject>

#include "smoothing.h"
#include "statistics.h"
#include "flightstatus.h"

class FSAccess;

/////////////////////////////////////////////////////////////////////////////

class BankController : public QObject
{
    Q_OBJECT

public:

    BankController(const FlightStatus* flightstatus,
                   const double& p_gain, 
                   const double& i_gain, 
                   const double& d_gain,
                   const double& max_bank_rate_deg_s,
                   const double& i_to_p_part_response_factor,
                   const double& max_idle_bank, 
                   const double& max_forced_bank,
                   bool do_statistics = false) :
        m_flightstatus(flightstatus), m_do_statistics(do_statistics), m_stat(0)
    {
        MYASSERT(m_flightstatus != 0);
        reset();
        setPIDParams(p_gain, i_gain, d_gain, max_bank_rate_deg_s, i_to_p_part_response_factor);
        setBankLimits(max_idle_bank, max_forced_bank);

        m_i_part = 0.0;
        m_i_to_p_part_response_factor = i_to_p_part_response_factor;
        m_last_bank_rate_diff_deg_s = 0.0;
        m_bank_target = 0.0;
        m_stable = false;
        m_override_active = false;
        m_override_joy_input = 0.0;

        setDoStatistics(do_statistics);
    }

    ~BankController()
    {
        delete m_stat;
    }

    void reset() { m_init = false; }
    const double& targetBank() const { return m_bank_target; }

    //! returns true if the joystick input is overridden
    bool overrideActive() const { return m_override_active; }

    bool isBankExcessive() const { return qAbs(m_flightstatus->bank.lastValue()) > (m_max_idle_bank + 5.0); }

    void setPIDParams(const double& p_gain, 
                      const double& i_gain, 
                      const double& d_gain,
                      const double& max_bank_rate_deg_s,
                      const double& i_to_p_part_response_factor)
    {
        MYASSERT(max_bank_rate_deg_s >= 2.5);
        m_p_gain = p_gain;
        m_i_gain = i_gain;
        m_d_gain = d_gain;
        m_max_bank_rate_deg_s = max_bank_rate_deg_s;
        m_i_to_p_part_response_factor = i_to_p_part_response_factor;
        Logger::log(QString("BankController:setPIDParams: p=%1, i=%2, d=%3, max_rate=%4, "
                            "i_to_p_part_response_factor=%5").
                    arg(p_gain).arg(i_gain).arg(d_gain).arg(max_bank_rate_deg_s).
                    arg(i_to_p_part_response_factor));
        emit signalParametersChanged();
    }

    void setBankLimits(const double& max_idle_bank, const double& max_forced_bank) 
    {
        MYASSERT(max_idle_bank >= 5.0);
        MYASSERT(max_forced_bank >= 10.0);
        MYASSERT(max_forced_bank >= max_idle_bank);
        m_max_idle_bank = max_idle_bank;
        m_max_forced_bank = max_forced_bank;
        m_bank_damping.setDampBorders(m_max_idle_bank, m_max_forced_bank);
        Logger::log(QString("BankController:setBankLimits: max_idle=%1 max_forced=%2").
                    arg(max_idle_bank).arg(max_forced_bank));
        emit signalParametersChanged();
    }

    bool execBankRate(FSAccess& fsaccess);

    const double& pGain() const { return m_p_gain; }
    const double& iGain() const { return m_i_gain; }
    const double& dGain() const { return m_d_gain; }

    const double& maxBankrate() const { return m_max_bank_rate_deg_s; }
    const double& maxIdleBank() const { return m_max_idle_bank; }
    const double& maxForcedBank() const { return m_max_forced_bank; }

    const double& IToPPartResponseFactor() const { return m_i_to_p_part_response_factor; }

    bool doStatistics() const { return m_do_statistics; }
    void setDoStatistics(bool do_statistics)
    {
        Logger::log(QString("BankController:setDoStatistics: %1").arg(do_statistics));
        m_do_statistics = do_statistics;

        if (m_do_statistics)
        {
            m_stat = new Statistics("stat.bank.csv", " ");
            MYASSERT(m_stat != 0);
            m_stat_timer.start();
        }
        else
        {
            delete m_stat;
            m_stat = 0;
        }
    }

signals:
 
    void signalParametersChanged();

protected:

    const FlightStatus* m_flightstatus;
    bool m_init;					
    QTime m_init_dt;
    double m_bank_target;
    bool m_stable;
    bool m_override_active;
    double m_override_joy_input;
    QTime m_last_call_dt;

    double m_p_gain;
    double m_i_gain;
    double m_d_gain;
    double m_max_bank_rate_deg_s;

    double m_i_part;
    double m_last_bank_rate_diff_deg_s;

    double m_i_to_p_part_response_factor;

    double m_max_idle_bank;
    double m_max_forced_bank;

    Damping m_bank_damping;

    bool m_do_statistics;
    Statistics *m_stat;
    QTime m_stat_timer;
};

/////////////////////////////////////////////////////////////////////////////

class PitchController : public QObject
{
    Q_OBJECT

public:

    PitchController(const FlightStatus* flightstatus,
                    const double& p_gain, 
                    const double& i_gain, 
                    const double& d_gain,
                    const double& max_pitch_rate_deg_s,
                    const double& pid_good_trend_damping_factor,
                    const double& i_to_p_part_response_factor,
                    const double& bank_rate_boost_factor,
                    const double& stable_fpv_damp_factor,
                    const double& transition_boost_factor,
                    const double& max_negative_pitch,
                    const double& max_positive_pitch,
                    bool do_statistics = false) :
        m_flightstatus(flightstatus), m_do_statistics(do_statistics), m_stat(0)
    {
        MYASSERT(m_flightstatus != 0);
        reset();
        setPIDParams(p_gain, i_gain, d_gain, max_pitch_rate_deg_s, 
                     pid_good_trend_damping_factor, i_to_p_part_response_factor, 
                     bank_rate_boost_factor, stable_fpv_damp_factor, transition_boost_factor);
        setPitchLimits(max_negative_pitch, max_positive_pitch);

        setDoStatistics(do_statistics);
    }

    ~PitchController()
    {
        delete m_stat;
    }

    void reset()
    {
        m_init = false;
        m_fpv_target = 0.0;
        m_pitch_target = 0.0;
        m_stable = false;
        m_override_active = false;
        m_override_joy_input = 0.0;
    }

    bool execPitchRate(FSAccess& fsaccess);

    void setPIDParams(const double& p_gain, 
                      const double& i_gain, 
                      const double& d_gain,
                      const double& max_pitch_rate_deg_s,
                      const double& pid_good_trend_damping_factor,
                      const double& i_to_p_part_response_factor,
                      const double& bank_rate_boost_factor,
                      const double& stable_fpv_damp_factor,
                      const double& transition_boost_factor)
    {
        MYASSERT(max_pitch_rate_deg_s > 0.1);
        m_p_gain = p_gain;
        m_i_gain = i_gain;
        m_d_gain = d_gain;
        m_max_pitch_rate_deg_s = max_pitch_rate_deg_s;
        m_pid_good_trend_damping_factor = pid_good_trend_damping_factor;
        m_pid_good_trend_damping.setDampBorders(0, pid_good_trend_damping_factor * m_max_pitch_rate_deg_s);
        m_i_to_p_part_response_factor = i_to_p_part_response_factor;
        m_bank_rate_boost_factor = bank_rate_boost_factor;
        m_stable_fpv_damp_factor = stable_fpv_damp_factor;
        m_transition_boost_factor = transition_boost_factor;
        Logger::log(QString("PitchController:setPIDParams: p=%1 i=%2 d=%3 max_rate=%4 "
                            "good_trend_damp=%5, i_to_p_response=%6, bank_rate_boost=%7, "
                            "stable_fpv_damp_factor=%8, transition_boost_factor=%9").
                    arg(p_gain).arg(i_gain).arg(d_gain).arg(max_pitch_rate_deg_s).
                    arg(pid_good_trend_damping_factor).arg(i_to_p_part_response_factor).
                    arg(bank_rate_boost_factor).arg(stable_fpv_damp_factor).
                    arg(transition_boost_factor));
        emit signalParametersChanged();
    }

    void setPitchLimits(const double& max_negative_pitch,
                        const double& max_positive_pitch)
    {    
        MYASSERT(max_negative_pitch <= -10.0);
        m_max_negative_pitch = max_negative_pitch;
        Logger::log(QString("PitchController:setPitchLimits: neg_max=%1").arg(max_negative_pitch));
        m_negative_pitch_damping.setDampBorders(m_max_negative_pitch+10.0, m_max_negative_pitch);

        MYASSERT(max_positive_pitch >= 10.0);
        m_max_positive_pitch = max_positive_pitch;
        Logger::log(QString("PitchController:setPitchLimits: pos_max=%1").arg(max_positive_pitch));
        m_positive_pitch_damping.setDampBorders(m_max_positive_pitch-10.0, m_max_positive_pitch);
        emit signalParametersChanged();
    }

    bool isPitchOutsideLimits(const double pitch_angle)
    { return (-pitch_angle < m_max_negative_pitch || -pitch_angle > m_max_positive_pitch); }

    const double& flightPathVerticalTarget() const { return m_fpv_target; }

    //! returns true if the joystick input is overridden
    bool overrideActive() const { return m_override_active; }

    bool isStable() const { return m_stable; }

    const double& pGain() const { return m_p_gain; }
    const double& iGain() const { return m_i_gain; }
    const double& dGain() const { return m_d_gain; }

    const double& PIDGoodTrendDampingFactor() const { return m_pid_good_trend_damping_factor; }
    const double& IToPPartResponseFactor() const { return m_i_to_p_part_response_factor; }
    const double& bankRateBoostFactor() const { return m_bank_rate_boost_factor; }
    const double& stableFPVDampFactor() const { return m_stable_fpv_damp_factor; }
    const double& transitionBoostFactor() const { return m_transition_boost_factor; }

    const double& maxPitchrate() const { return m_max_pitch_rate_deg_s; }
    const double& maxNegativePitch() const { return m_max_negative_pitch; }
    const double& maxPositivePitch() const { return m_max_positive_pitch; }

    bool doStatistics() const { return m_do_statistics; }
    void setDoStatistics(bool do_statistics)
    {
        Logger::log(QString("PitchController:setDoStatistics: %1").arg(do_statistics));
        m_do_statistics = do_statistics;

        if (m_do_statistics)
        {
            m_stat = new Statistics("stat.pitch.csv", " ");
            MYASSERT(m_stat != 0);
            m_stat_timer.start();
        }
        else
        {
            delete m_stat;
            m_stat = 0;
        }
    }

signals:
 
    void signalParametersChanged();

protected:

    const FlightStatus* m_flightstatus;
    bool m_init;					
    QTime m_init_dt;
    double m_fpv_target;
    double m_pitch_target;
    bool m_stable;
    bool m_override_active;
    double m_override_joy_input;
    QTime m_last_call_dt;

    double m_p_gain;
    double m_i_gain;
    double m_d_gain;
    double m_max_pitch_rate_deg_s;

    double m_i_part;
    double m_last_pitch_rate_diff_deg_s;

    double m_max_positive_pitch;
    double m_max_negative_pitch;

    Damping m_positive_pitch_damping;
    Damping m_negative_pitch_damping;

    double m_pid_good_trend_damping_factor;
    Damping m_pid_good_trend_damping;
    double m_i_to_p_part_response_factor;
    double m_bank_rate_boost_factor;
    double m_stable_fpv_damp_factor;
    double m_transition_boost_factor;
    double m_p_boost_when_stable;
    
    bool m_do_statistics;
    Statistics *m_stat;
    QTime m_stat_timer;
};

#endif /* __FLY_BY_WIRE_H__ */

// End of file

