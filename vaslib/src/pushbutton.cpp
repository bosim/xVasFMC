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

/*! \file    PushButton.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "pushbutton.h"

/////////////////////////////////////////////////////////////////////////////

PushButton::PushButton(QWidget* parent) : 
    QPushButton(parent), m_long_click_delay_ms(500), m_long_click_emitted(false)
{
    MYASSERT(connect(&m_long_click_timer, SIGNAL(timeout()), this, SLOT(slotLongClick())));
    m_long_click_timer.setSingleShot(true);
}

/////////////////////////////////////////////////////////////////////////////

void PushButton::slotLongClick()
{
    m_long_click_emitted = true;
    emit signalLongClick();
}

/////////////////////////////////////////////////////////////////////////////

void PushButton::mousePressEvent(QMouseEvent *event)
{
    m_long_click_emitted = false;
    if (m_long_click_delay_ms > 0) m_long_click_timer.start(m_long_click_delay_ms);
    QPushButton::mousePressEvent(event);
}

/////////////////////////////////////////////////////////////////////////////

void PushButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_long_click_emitted) 
    {
        setDown(false);
        return;
    }

    m_long_click_timer.stop();
    QPushButton::mouseReleaseEvent(event);
}

/////////////////////////////////////////////////////////////////////////////

// End of file
