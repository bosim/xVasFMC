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

/*! \file    controller_base.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __CONTROLLER_BASE_H__
#define __CONTROLLER_BASE_H__

#include <QTime>

#include "flightstatus.h"

/////////////////////////////////////////////////////////////////////////////

//! controller interface class
class ControllerBase 
{
public:
    //! Standard Constructor
    ControllerBase(const FlightStatus& flighstatus, const double& min_output, const double& max_output) : 
        m_flightstatus(flighstatus), m_init(false), m_target(0.0), 
        m_min_output(min_output), m_max_output(max_output), m_output(min_output)
    {};

    //! Destructor
    virtual ~ControllerBase()
    {};

    //-----

    const double& target() const { return m_target; }
    virtual void setTarget(const double& target) { m_target = target; }

    virtual const double& output(const double& input) = 0;
    virtual void setOutput(const double& output) { m_output = output; }

    virtual void setMinOutput(const double& min_output) { m_min_output = min_output; }
    virtual void setMaxOutput(const double& max_output) { m_max_output = max_output; }

protected:

    const FlightStatus& m_flightstatus;

    bool m_init;
    double m_target;
    double m_min_output;
    double m_max_output;
    double m_output;

    QTime m_last_call_dt;

private:
    //! Hidden copy-constructor
    ControllerBase(const ControllerBase&);
    //! Hidden assignment operator
    const ControllerBase& operator = (const ControllerBase&);
};

#endif /* __CONTROLLER_BASE_H__ */

// End of file

