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

/*! \file    noise_generator.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __NOISE_GENERATOR_H__
#define __NOISE_GENERATOR_H__

#include <QTime>

#include "smoothing.h"

/////////////////////////////////////////////////////////////////////////////

//! noise generator, generates a smoothed noise
class NoiseGenerator 
{
public:
    //! Standard Constructor
    NoiseGenerator(uint max_noise_update_interval_ms, 
                   double max_noise_inc_per_update, 
                   double max_noise);

    //! Destructor
    virtual ~NoiseGenerator() {};

    //-----

    void reset();

    uint maxNoiseUpdateInterval() const { return m_max_noise_update_interval_ms; }
    void setMaxNoiseUpdateInterval(uint max_noise_update_interval_ms) 
    {
        m_max_noise_update_interval_ms = max_noise_update_interval_ms; 
        m_smoothed_noise.setDelayMs(max_noise_update_interval_ms * 2);
    }

    const double& maxNoiseIncPerUpdate() const { return m_max_noise_inc_per_update; }
    void setMaxNoiseIncPerUpdate(double max_noise_inc_per_update)
    { m_max_noise_inc_per_update = max_noise_inc_per_update; }

    const double& maxNoise() const { return m_max_noise; }
    void setMaxNoise(const double& max_noise) { m_max_noise = max_noise; }

    double getNoise();

protected:
    
    QTime m_last_noise_update_timer;
    uint m_max_noise_update_interval_ms;
    double m_max_noise_inc_per_update;
    double m_max_noise;

    SmoothedValueWithDelay<double> m_smoothed_noise;

private:
    //! Hidden copy-constructor
    NoiseGenerator(const NoiseGenerator&);
    //! Hidden assignment operator
    const NoiseGenerator& operator = (const NoiseGenerator&);
};



#endif /* __NOISE_GENERATOR_H__ */

// End of file

