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

/*! \file    fmc_data_provider.h
    \author  Alexander Wemmer, alex@wemmer.at
*/


#ifndef __FMC_DATA_PROVIDER_H__
#define __FMC_DATA_PROVIDER_H__

class FlightRoute;

//! FMC data provider interface
class FMCDataProvider 
{
public:
    //! Standard Constructor
    FMCDataProvider() {};

    //! Destructor
    virtual ~FMCDataProvider() {};

    //----- methods below must be implemented by derived classes

    virtual const FlightRoute& normalRoute() const = 0;
    virtual bool approachPhaseActive() const = 0;
};

#endif /* __FMC_DATA_PROVIDER_H__ */

// End of file

