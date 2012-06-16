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

/*! \file    PushButton.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef PUSHBUTTON_H
#define PUSHBUTTON_H

#include "assert.h"
#include <QPushButton>
#include <QTimer>
#include <QMouseEvent>

/////////////////////////////////////////////////////////////////////////////

//! customized pushbutton with long click
class PushButton : public QPushButton
{
    Q_OBJECT

public:
    //! Standard Constructor
    PushButton(QWidget* parent=0);

    //! Destructor
    virtual ~PushButton() {};

    inline void setLongClickDelay(uint long_click_delay_ms) { m_long_click_delay_ms = long_click_delay_ms; }

signals:

    void signalLongClick(); 

protected slots:

    void slotLongClick();

protected:

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

protected:

    uint m_long_click_delay_ms;
    QTimer m_long_click_timer;
    bool m_long_click_emitted;

private:
    //! Hidden copy-constructor
    PushButton(const PushButton&);
    //! Hidden assignment operator
    const PushButton& operator = (const PushButton&);
};

#endif /* PUSHBUTTON_H */

// End of file

