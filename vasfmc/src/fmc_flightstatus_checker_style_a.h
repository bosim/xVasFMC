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

/*! \file    fmc_flighstatus_checker_style_a.h
    \author  Alexander Wemmer, alex@wemmer.at
*/


#ifndef __FMC_FLIGHSTATUS_CHECKER_STYLE_A_H__
#define __FMC_FLIGHSTATUS_CHECKER_STYLE_A_H__

#include "fmc_flightstatus_checker_base.h"

class FMCControl;

/////////////////////////////////////////////////////////////////////////////

//! a-style flight status checker
class FlightStatusCheckerStyleA : public FlightStatusCheckerBase
{
    Q_OBJECT

public:
    //! Standard Constructor
    FlightStatusCheckerStyleA(FlightStatus* flightstatus, const FMCControl* fmc_control);

    //! Destructor
    virtual ~FlightStatusCheckerStyleA() {};

    //-----
    
    virtual void doChecks(FSAccess& fsaccess);

protected:

    bool m_ap_alt_was_intercepted_within_750ft;
    bool m_ap_alt_was_intercepted_within_250ft;

    uint m_last_flaps_lever_notch;

private:
    //! Hidden copy-constructor
    FlightStatusCheckerStyleA(const FlightStatusCheckerStyleA&);
    //! Hidden assignment operator
    const FlightStatusCheckerStyleA& operator = (const FlightStatusCheckerStyleA&);
};



#endif /* __FMC_FLIGHSTATUS_CHECKER_STYLE_A_H__ */

// End of file

