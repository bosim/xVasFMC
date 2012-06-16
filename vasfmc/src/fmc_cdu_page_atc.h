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

/*! \file    fmc_cdu_page_atc.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __FMC_CDU_PAGE_ATC_H__
#define __FMC_CDU_PAGE_ATC_H__

#include "weather.h"
#include "vroute.h"

#include "fmc_cdu_page_base.h"

/////////////////////////////////////////////////////////////////////////////

//! ATC page
class FMCCDUPageStyleAATC : public FMCCDUPageBase
{
    Q_OBJECT

public:

    FMCCDUPageStyleAATC(const QString& page_name, FMCCDUPageManager* page_manager);
    virtual ~FMCCDUPageStyleAATC() {};

    virtual void paintPage(QPainter& painter) const;
    virtual void processAction(const QString& action);

protected slots:

protected:

protected:

};

#endif /* __FMC_CDU_PAGE_ATC_H__ */

// End of file

