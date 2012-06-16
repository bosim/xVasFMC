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

/*! \file    fmc_sounds_style_b.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __FMC_SOUNDS_STYLE_B_H__
#define __FMC_SOUNDS_STYLE_B_H__

#include <QTime>

#include "fmc_sounds.h"

class QSound;

/////////////////////////////////////////////////////////////////////////////

//! A style sounds
class FMCSoundStyleB : public FMCSounds
{
    Q_OBJECT

public:
    //! Standard Constructor
    FMCSoundStyleB(Config* main_config, FMCControl* fmc_control);

    //! Destructor
    virtual ~FMCSoundStyleB();

protected:

    virtual void setupDefaultConfig();
    virtual void checkSounds();

protected:

private:
    //! Hidden copy-constructor
    FMCSoundStyleB(const FMCSoundStyleB&);
    //! Hidden assignment operator
    const FMCSoundStyleB& operator = (const FMCSoundStyleB&);
};

#endif /* __FMC_SOUNDS_STYLE_B_H__ */

// End of file

