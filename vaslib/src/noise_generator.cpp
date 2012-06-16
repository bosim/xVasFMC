///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2009 Alexander Wemmer 
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

/*! \file    noise_generator.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/


#include "noise_generator.h"

/////////////////////////////////////////////////////////////////////////////

NoiseGenerator::NoiseGenerator(uint max_noise_update_interval_ms, 
                               double max_noise_inc_per_update,
                               double max_noise) :
    m_max_noise_update_interval_ms(max_noise_update_interval_ms),
    m_max_noise_inc_per_update(max_noise_inc_per_update),
    m_max_noise(max_noise), m_smoothed_noise(false, false, 40, max_noise_update_interval_ms*2)
{
    srand(QTime::currentTime().second());
    m_last_noise_update_timer.start();
    m_smoothed_noise.doLowPass(true, 0.5);
}

/////////////////////////////////////////////////////////////////////////////

void NoiseGenerator::reset()
{
    m_smoothed_noise.clear();
}

/////////////////////////////////////////////////////////////////////////////

double NoiseGenerator::getNoise()
{
    if (m_last_noise_update_timer.elapsed() < (int)m_max_noise_update_interval_ms) return m_smoothed_noise.value();
    m_last_noise_update_timer.start();

    double last_noise_value = m_smoothed_noise.lastValue();

    double noise_inc = (2.0 * m_max_noise_inc_per_update * (((double)rand())/RAND_MAX)) -
                       m_max_noise_inc_per_update + 
                       (-0.1 * last_noise_value);

    noise_inc = qMax(-m_max_noise, qMin(m_max_noise, noise_inc));

    last_noise_value += noise_inc;
    if (last_noise_value < -m_max_noise)      last_noise_value -= noise_inc;
    else if (last_noise_value >  m_max_noise) last_noise_value -= noise_inc;

    if (qAbs(last_noise_value) > m_max_noise) last_noise_value = m_max_noise;

    m_smoothed_noise = last_noise_value;
    return m_smoothed_noise.value();
}

// End of file
