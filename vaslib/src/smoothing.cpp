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

/*! \file    smoothing.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/


#include "smoothing.h"

void Damping::setDampBorders(const double& x_damp_start, const double& x_damp_end)
{
    if (x_damp_start < 0.0) { MYASSERT(x_damp_end < x_damp_start); }
    else                    { MYASSERT(x_damp_end > x_damp_start); }
    
    m_x_damp_start = x_damp_start;
    m_x_damp_end = x_damp_end;
    
    m_k = -1.0 / (m_x_damp_end - m_x_damp_start);
    m_d = -m_k * m_x_damp_end;
}

/////////////////////////////////////////////////////////////////////////////

double Damping::dampFactor(const double& x)
{ 
    if (m_x_damp_start * x < 0.0) return 1.0; // cover divergent signs
    return (qAbs(x) <= qAbs(m_x_damp_start) ? 1.0 : m_k * x + m_d);
}

// End of file
