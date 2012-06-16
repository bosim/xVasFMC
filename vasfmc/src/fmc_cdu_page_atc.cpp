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

/*! \file    fmc_cdu_page_atc.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "fmc_cdu_defines.h"
#include "fmc_cdu_page_manager.h"
#include "fmc_cdu_page_atc.h"

/////////////////////////////////////////////////////////////////////////////

FMCCDUPageStyleAATC::FMCCDUPageStyleAATC(const QString& page_name, FMCCDUPageManager* page_manager) :
    FMCCDUPageBase(page_name, page_manager)
{
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAATC::paintPage(QPainter& painter) const
{
    painter.setBackground(QBrush(BLACK));
    painter.fillRect(painter.window(), painter.background());

    setFont(painter, NORM_FONT, QFont::Bold);
    drawTextCenter(painter, 1, "ATSU DATALINK");

    drawTextCenter(painter, 13, "NOT IMPLEMENTED");
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAATC::processAction(const QString& action)
{
    int llsk_index = -1;
    int rlsk_index = -1;

    if (action.startsWith("LLSK"))
    {
        bool convok = false;
        llsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);
    }
    else if (action.startsWith("RLSK"))
    {
        bool convok = false;
        rlsk_index = action.right(1).toUInt(&convok);
        MYASSERT(convok);
    }
    else
    {
        return;
    }

    QString text = m_page_manager->scratchpad().text();
    if (text.isEmpty()) return;

    //TODO implement ATC page
}

// End of file
