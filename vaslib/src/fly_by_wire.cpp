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

/*! \file    fly_by_wire.cpp
  \author  Mark Foti and Alexander Wemmer, alex@wemmer.at
*/

#include "assert.h"
#include "logger.h"

#include "fsaccess.h"

#include "fly_by_wire.h"

/////////////////////////////////////////////////////////////////////////////

bool BankController::execBankRate(FSAccess& fsaccess)
{
    if (!m_flightstatus->isValid() ||
        m_flightstatus->paused || 
        m_flightstatus->slew || 
        m_flightstatus->onground ||
        m_flightstatus->radarAltitude() < 55 ||
        (m_flightstatus->ap_enabled && 
         (m_flightstatus->ap_hdg_lock || m_flightstatus->ap_nav1_lock || 
          m_flightstatus->ap_gs_lock || m_flightstatus->ap_app_lock)))
    {
        fsaccess.freeAileronAxis();
        reset();
        return false;
    }

    double bank_rate_deg_s = -m_flightstatus->velocity_roll_deg_s;
    double bank_angle = m_flightstatus->bank.lastValue();

    if(!m_init)
    {
        m_i_part = m_flightstatus->aileron_percent;
        m_last_bank_rate_diff_deg_s = 0.0;
        m_bank_target = bank_angle;
        m_stable = false;
        m_override_active = false;
        m_override_joy_input = 0.0;
        m_init = true;
        m_last_call_dt.start();
        m_init_dt.start();
        return true;
    }

    if (m_init_dt.elapsed() < 500) return true;

    double call_time_elapsed_s = qMin(m_last_call_dt.elapsed(), 1000) / 1000.0;
    if (call_time_elapsed_s <= 0.01) return true;

    if (m_override_active && qAbs(bank_angle) < m_max_idle_bank)
    {
        Logger::log(QString("BankController: override deactivated"));
        m_override_active = false;
    }

    bool joy_input_active = qAbs(m_flightstatus->aileron_input_percent) > 3.0;
    bool joy_input_opposite_to_override = (m_override_joy_input * m_flightstatus->aileron_input_percent <= 0.0);
    bool joy_input_allowed = (!m_override_active || qAbs(bank_angle) < m_max_forced_bank || joy_input_opposite_to_override);

    //----- determine turn rate command by joystick input when available

    double bank_rate_cmd_deg_s = 0.0;

    if (joy_input_active && joy_input_allowed)
    {
        m_stable = false;
        bank_rate_cmd_deg_s = m_max_bank_rate_deg_s * m_flightstatus->aileron_input_percent * 0.01;
        
        if ((bank_angle < 0.5 && bank_rate_cmd_deg_s > 0.0) ||
            (bank_angle > 0.5 && bank_rate_cmd_deg_s < 0.0))
        { 
            bank_rate_cmd_deg_s *= m_bank_damping.dampFactor(qAbs(bank_angle));
        }
    }

    //----- determine turn rate command by bank target or override the joy input if the bank angle is excessive

    if (!joy_input_active || qAbs(bank_angle) >= m_max_forced_bank)
    {
        if (!joy_input_active)
        {
            if (!m_stable && qAbs(bank_rate_deg_s) < 0.35)
            {
                m_stable = true;
                m_bank_target = bank_angle + bank_rate_deg_s*0.5;
            }
            
            if (qAbs(m_bank_target) <= 0.7) m_bank_target = 0.0;
            else m_bank_target = LIMIT(m_bank_target, m_max_idle_bank);
        }
        else
        {
            if (qAbs(bank_angle) >= m_max_forced_bank && !m_override_active)
            {
                m_stable = true;
                m_override_active = true;
                m_bank_target = m_max_forced_bank * ((bank_angle < 0.0) ? -1.0 : 1.0);
                m_override_joy_input = m_flightstatus->aileron_input_percent;
                Logger::log(QString("BankController: override active"));
            }
        }
        
        if (m_stable)
        {
            bank_rate_cmd_deg_s = LIMIT((bank_angle - m_bank_target)*0.5, (m_max_bank_rate_deg_s*0.5));
        }
    }

    double bank_rate_diff_deg_s = bank_rate_cmd_deg_s - bank_rate_deg_s;
    
    double p_part = LIMIT(m_p_gain * bank_rate_diff_deg_s, 100.0);

    double i_part = m_i_gain * bank_rate_diff_deg_s * call_time_elapsed_s;
    m_i_part += i_part;
    if (m_i_part * p_part < 0.0) m_i_part += m_i_to_p_part_response_factor * p_part;
    LIMIT(m_i_part, 100.0);

    double d_part = LIMIT(m_d_gain * (bank_rate_diff_deg_s - m_last_bank_rate_diff_deg_s) / call_time_elapsed_s, 100.0);

    // controller output
    double output = LIMIT(p_part + m_i_part + d_part, 100.0);

    if (m_stat != 0)
    {
        m_stat->putItemInLine(QString::number(m_stat_timer.elapsed()/1000.0));
        m_stat->putItemInLine(QString::number(bank_rate_cmd_deg_s));
        m_stat->putItemInLine(QString::number(bank_rate_deg_s));
        m_stat->putItemInLine(QString::number(p_part));
        m_stat->putItemInLine(QString::number(m_i_part));
        m_stat->putItemInLine(QString::number(d_part));
        m_stat->putItemInLine(QString::number(output));
        m_stat->nextLine();
    }
// gnuplot command
/*
plot 'C:\devel\vas\vasfmc\stat.bank.csv' using 1:2 with lines, 'C:\devel\vas\vasfmc\stat.bank.csv' using 1:3 with lines, 'C:\devel\vas\vasfmc\stat.bank.csv' using 1:4 with lines, 'C:\devel\vas\vasfmc\stat.bank.csv' using 1:5 with lines, 'C:\devel\vas\vasfmc\stat.bank.csv' using 1:6 with lines, 'C:\devel\vas\vasfmc\stat.bank.csv' using 1:7 with lines 
*/

    fsaccess.setAileron(output);
    
    if (qAbs(output) > 95.0)
        Logger::log(QString("BC: cmd=%1 rate=%2 diff=%3 p=%4 i=%5 d=%6 out=%7,"
                            "stab=%8, tgt=%9, dt=%10").
                    arg(bank_rate_cmd_deg_s, -6, 'f', 3).
                    arg(bank_rate_deg_s, -6, 'f', 3).
                    arg(bank_rate_diff_deg_s, -7, 'f', 3).
                    arg(p_part, -8, 'f', 3).
                    arg(m_i_part, -8, 'f', 3).
                    arg(d_part, -8, 'f', 3).
                    arg(output, -8, 'f', 3).
                    arg(m_stable).
                    arg(m_bank_target).arg(call_time_elapsed_s));

    m_last_bank_rate_diff_deg_s = bank_rate_diff_deg_s;
    m_last_call_dt.start();

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool PitchController::execPitchRate(FSAccess& fsaccess)
{
    if (!m_flightstatus->isValid() ||
        m_flightstatus->paused || 
        m_flightstatus->slew || 
        m_flightstatus->onground ||
        m_flightstatus->radarAltitude() < 55 ||
        (m_flightstatus->ap_enabled && 
         (m_flightstatus->ap_alt_lock || m_flightstatus->ap_vs_lock || 
          m_flightstatus->ap_gs_lock || m_flightstatus->ap_app_lock)))
    {
        fsaccess.freeElevatorAxis();
        reset();
        return false;
    }

    double pitch_rate_deg_s;
    double pitch_angle = m_flightstatus->smoothedPitch(&pitch_rate_deg_s);
    pitch_rate_deg_s *= -1.0;
    pitch_rate_deg_s = LIMIT(pitch_rate_deg_s, 10.0);

    if(!m_init)
    {
        m_init = true;
        m_i_part = m_flightstatus->elevator_percent;
        m_last_pitch_rate_diff_deg_s = 0.0;
        m_pitch_target = pitch_angle;
        m_fpv_target = m_flightstatus->fpv_vertical.lastValue();
        m_stable = false;
        m_last_call_dt.start();
        m_init_dt.start();
        m_p_boost_when_stable = 0.0;
        return true;
    }

    double call_time_elapsed_s = qMin(m_last_call_dt.elapsed(), 1000) / 1000.0;
    if (m_init_dt.elapsed() < 500 || call_time_elapsed_s <= 0.01) return true;

    double fpv_rate_deg_s = (m_flightstatus->fpv_vertical.lastValue() - 
                             m_flightstatus->fpv_vertical_previous) / call_time_elapsed_s;

    if (m_override_active && !isPitchOutsideLimits(pitch_angle))
    {
        Logger::log(QString("PitchController: override deactivated"));
        m_override_active = false;
    }

    bool joy_input_active = qAbs(m_flightstatus->elevator_input_percent) > 3.0;
    bool joy_input_opposite_to_override = (m_override_joy_input *
                                           m_flightstatus->elevator_input_percent <= 0.0);
    bool joy_input_allowed = (!m_override_active || !isPitchOutsideLimits(pitch_angle) || joy_input_opposite_to_override);

    double good_trend_damp_factor = 1.0;

    //----- determine pitch rate command by joystick input when available

    double pitch_rate_cmd_deg_s = 0.0;

    if (joy_input_active && joy_input_allowed)
    {
        m_stable = false;
        m_p_boost_when_stable = 0.0;
        pitch_rate_cmd_deg_s = m_max_pitch_rate_deg_s * m_flightstatus->elevator_input_percent * 0.01;

        if (-pitch_angle < 0.0)
        {
            if (pitch_rate_cmd_deg_s < 0.0)
                pitch_rate_cmd_deg_s *= m_negative_pitch_damping.dampFactor(-pitch_angle);
        }
        else
        {
            if (pitch_rate_cmd_deg_s > 0.0)
                pitch_rate_cmd_deg_s *= m_positive_pitch_damping.dampFactor(-pitch_angle);
        }

        if ((pitch_rate_cmd_deg_s * pitch_rate_deg_s) > 0.0)
            good_trend_damp_factor = m_pid_good_trend_damping.dampFactor(qAbs(pitch_rate_cmd_deg_s));
    }

    //----- determine pitch rate command by pitch target or override the joy input if the pitch angle is excessive

    if (!joy_input_active)
    {
        if (!m_stable && qAbs(pitch_rate_deg_s) < 0.15 && qAbs(fpv_rate_deg_s) < 0.25)
        {
            m_stable = true;
            m_pitch_target = pitch_angle + pitch_rate_deg_s;
            m_fpv_target = m_flightstatus->fpv_vertical.lastValue() + fpv_rate_deg_s;
        }
        
        m_pitch_target = -qMin(qMax(-m_pitch_target, m_max_negative_pitch), m_max_positive_pitch);
    }
    else if (isPitchOutsideLimits(pitch_angle) && !m_override_active)
    {
        m_stable = true;
        m_override_active = true;
        m_pitch_target = -qMin(qMax(-m_pitch_target, m_max_negative_pitch), m_max_positive_pitch);
        m_fpv_target = m_flightstatus->fpv_vertical.lastValue() + fpv_rate_deg_s;
        m_override_joy_input = m_flightstatus->elevator_input_percent;
        Logger::log(QString("PitchController: override active"));
    }

    pitch_rate_cmd_deg_s = LIMIT(pitch_rate_cmd_deg_s, m_max_pitch_rate_deg_s);
    double pitch_rate_diff_deg_s = pitch_rate_cmd_deg_s - pitch_rate_deg_s;
    
    double stable_fpv_damp_factor = 1.0;
    double transition_phase_boost_factor = 1.0;

    double bank_boost = 1.0;
    if (qAbs(m_flightstatus->bank.lastValue()) <= 30)
        bank_boost += qMin(10.0, qAbs(m_flightstatus->velocity_roll_deg_s)) / m_bank_rate_boost_factor;

    if (m_stable)
    {
        if (m_override_active)
        {
            pitch_rate_diff_deg_s = LIMIT((pitch_angle - m_pitch_target), m_max_pitch_rate_deg_s) - pitch_rate_deg_s;
        }
        else if (!joy_input_active)
        {
            if (qAbs(m_fpv_target) <= 0.5) m_fpv_target = 0.0;

            // fpv stability when stable

            double fpv_diff = m_fpv_target - (m_flightstatus->fpv_vertical.lastValue());

            pitch_rate_diff_deg_s = LIMIT(LIMIT(fpv_diff / 4.0, 0.25) - fpv_rate_deg_s, 20.0);
            
            if (qAbs(fpv_diff < 1.0) && bank_boost <= 1.0) stable_fpv_damp_factor = m_stable_fpv_damp_factor;
        }
    }
    else
    {
        if (!joy_input_active) transition_phase_boost_factor = m_transition_boost_factor;
    }

//     Logger::log(QString("pgain=%1 diff=%2 gt_damp=%3  boost=%4 rate=p:%5/f:%6 time=%7").
//                 arg(m_p_gain).arg(pitch_rate_diff_deg_s).arg(good_trend_damp_factor).arg(bank_boost).
//                 arg(pitch_rate_deg_s).arg(fpv_rate_deg_s).arg(call_time_elapsed_s));

    double p_part = m_p_gain * pitch_rate_diff_deg_s * good_trend_damp_factor;
    if (transition_phase_boost_factor > 1.0) p_part *= transition_phase_boost_factor * 0.25;

    //TODO
    if (!joy_input_active && m_stable && !m_override_active) p_part *= bank_boost;

    p_part = LIMIT(p_part, 100.0);

    double i_part = m_i_gain * pitch_rate_diff_deg_s * call_time_elapsed_s * 
                    good_trend_damp_factor * stable_fpv_damp_factor * transition_phase_boost_factor *
                    qMax(1.0, bank_boost * 0.8);
    m_i_part = LIMIT(m_i_part + i_part, 100.0);

    // N1 boost
    double n1_trend = 0.0;
    m_flightstatus->engine_data[1].smoothedN1(&n1_trend);
    m_i_part -= (n1_trend * call_time_elapsed_s) / 2.0;

    // boost i part to follow the p part when both have different signs
    if (m_i_part * p_part < 0.0) 
        m_i_part = LIMIT(m_i_part + (m_i_to_p_part_response_factor * transition_phase_boost_factor * p_part / 
                                     qMax(1.0, bank_boost * 0.6)), 100.0);

    double d_part = LIMIT(m_d_gain * (pitch_rate_diff_deg_s - m_last_pitch_rate_diff_deg_s) / call_time_elapsed_s, 10.0);

    // boost by response factor
    if (!joy_input_active && m_stable && !m_override_active)
    {
//TODO
//         m_p_boost_when_stable = qMin(3.0, m_p_boost_when_stable + (1.0 * call_time_elapsed_s));
//         if (qAbs(p_part) < 10.0) p_part = LIMIT(p_part * m_p_boost_when_stable, 100.0);

        if ((pitch_rate_diff_deg_s * d_part) > 0.0) 
        {
            d_part = LIMIT(d_part * 5.0, 5.0);
            m_i_part += m_i_to_p_part_response_factor * d_part;
        }
    }

    // controller output
    double output = LIMIT(p_part + m_i_part + d_part, 100.0);

    if (m_stat != 0)
    {
        m_stat->putItemInLine(QString::number(m_stat_timer.elapsed()/1000.0));
        m_stat->putItemInLine(QString::number(pitch_rate_cmd_deg_s*10));
        if (m_stable)
            m_stat->putItemInLine(QString::number(fpv_rate_deg_s*10));
        else
            m_stat->putItemInLine(QString::number(pitch_rate_deg_s*10));
        m_stat->putItemInLine(QString::number(p_part));
        m_stat->putItemInLine(QString::number(m_i_part));
        m_stat->putItemInLine(QString::number(d_part));
        m_stat->putItemInLine(QString::number(output));
        m_stat->putItemInLine(m_stable ? "25" : "-25");
        if (m_stable)
            m_stat->putItemInLine(QString::number(m_fpv_target * 10));
        else 
            m_stat->putItemInLine(QString::number(m_pitch_target * 10));
        m_stat->nextLine();
    }
// gnuplot command
/*
plot 'C:\devel\vas\vasfmc\stat.pitch.csv' using 1:2 with lines, 'C:\devel\vas\vasfmc\stat.pitch.csv' using 1:3 with lines, 'C:\devel\vas\vasfmc\statistic.csv' using 1:4 with lines, 'C:\devel\vas\vasfmc\stat.pitch.csv' using 1:5 with lines, 'C:\devel\vas\vasfmc\stat.pitch.csv' using 1:6 with lines 
plot'C:\devel\vas\vasfmc\stat.pitch.csv' using 1:2 with lines, 'C:\devel\vas\vasfmc\stat.pitch.csv' using 1:3 with lines, 'C:\devel\vas\vasfmc\stat.pitch.csv' using 1:4 with lines, 'C:\devel\vas\vasfmc\stat.pitch.csv' using 1:5 with lines, 'C:\devel\vas\vasfmc\stat.pitch.csv' using 1:6 with lines, 'C:\devel\vas\vasfmc\stat.pitch.csv' using 1:7 with lines
plot'C:\devel\vas\vasfmc\stat.pitch.csv' using 1:2 with lines, 'C:\devel\vas\vasfmc\stat.pitch.csv' using 1:3 with lines, 'C:\devel\vas\vasfmc\stat.pitch.csv' using 1:4 with lines, 'C:\devel\vas\vasfmc\stat.pitch.csv' using 1:5 with lines, 'C:\devel\vas\vasfmc\stat.pitch.csv' using 1:6 with lines, 'C:\devel\vas\vasfmc\stat.pitch.csv' using 1:7 with lines, 'C:\devel\vas\vasfmc\stat.pitch.csv' using 1:8 with lines
plot'C:\devel\vas\vasfmc\stat.pitch.csv' using 1:2 with lines, 'C:\devel\vas\vasfmc\stat.pitch.csv' using 1:3 with lines, 'C:\devel\vas\vasfmc\stat.pitch.csv' using 1:4 with lines, 'C:\devel\vas\vasfmc\stat.pitch.csv' using 1:5 with lines, 'C:\devel\vas\vasfmc\stat.pitch.csv' using 1:6 with lines, 'C:\devel\vas\vasfmc\stat.pitch.csv' using 1:7 with lines, 'C:\devel\vas\vasfmc\stat.pitch.csv' using 1:8 with lines, 'C:\devel\vas\vasfmc\stat.pitch.csv' using 1:9 with lines
*/

    fsaccess.setElevator(output);

    if (m_flightstatus->elevator_percent > 10.0)
        fsaccess.setElevatorTrimPercent(m_flightstatus->elevator_trim_percent + 0.1);
    else if (m_flightstatus->elevator_percent > 5.0)
        fsaccess.setElevatorTrimPercent(m_flightstatus->elevator_trim_percent + 0.01);
    else if (m_flightstatus->elevator_percent < -10.0)
        fsaccess.setElevatorTrimPercent(m_flightstatus->elevator_trim_percent - 0.1);
    else if (m_flightstatus->elevator_percent < -5.0)
        fsaccess.setElevatorTrimPercent(m_flightstatus->elevator_trim_percent - 0.01);
    
    if (qAbs(output) > 95.0)
        Logger::log(QString("PC: cmd=%1 rate=%2 diff=%3 p=%4 i=%5 d=%6 out=%7, stab=%8, tgt=%9, dt=%10").
                    arg(pitch_rate_cmd_deg_s, -6, 'f', 3).
                    arg(m_stable ? fpv_rate_deg_s : pitch_rate_deg_s, -6, 'f', 3).
                    arg(pitch_rate_diff_deg_s, -7, 'f', 3).
                    arg(p_part, -8, 'f', 3).
                    arg(m_i_part, -8, 'f', 3).
                    arg(d_part, -8, 'f', 3).
                    arg(output, -8, 'f', 3).
                    arg(m_stable).
                    arg(m_pitch_target).
                    arg(call_time_elapsed_s));

    m_last_pitch_rate_diff_deg_s = pitch_rate_diff_deg_s;
    m_last_call_dt.start();

    return true;
}

// End of file
