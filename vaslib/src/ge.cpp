///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2006 Alexander Wemmer 
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

/*! \file    ge.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "ge.h"

/////////////////////////////////////////////////////////////////////////////

GraphicElementBase::GraphicElementBase(bool is_moveable = false) : 
    m_is_moveable(is_moveable), m_used(false), m_is_selected(0), m_is_highlighted(0),
    m_termination_receiver(0), m_data_element(0)
{
    FULL_ARC = 16 * 359;
    m_normal_color = m_active_color = Qt::black;
    m_selected_color = m_highlighted_color = Qt::red; // QColor("#5475F7");
}

/////////////////////////////////////////////////////////////////////////////

GraphicElementBase::~GraphicElementBase() 
{
    if (m_termination_receiver != 0) m_termination_receiver->terminated(this);
}

/////////////////////////////////////////////////////////////////////////////

GraphicElementBase* GraphicElementBase::mouseClick(QPoint point, bool& already_used)
{
    m_last_mouse_click_pos = point;
    
    if (already_used) 
    {
        setSelected(false);
    }
    else
    {
        recalcBoundingAndSelectorElements();
        if (m_selector_path.contains(point)) setSelected(true); 
        else setSelected(false);
    }
    
    already_used |= m_is_selected;
    if (m_is_selected) return this;
    return 0;
}

/////////////////////////////////////////////////////////////////////////////

bool GraphicElementBase::mouseMove(QPoint point, bool already_used, bool do_highlight) 
{
    if (!do_highlight) return false;

    if (already_used)
    {
        setHighlighted(false);
    }
    else
    {
        recalcBoundingAndSelectorElements();
        if (m_selector_path.contains(point)) setHighlighted(true);
        else setHighlighted(false);
    }

    return m_is_highlighted;
};

/////////////////////////////////////////////////////////////////////////////

void GraphicElementBase::mouseRelease(QPoint point)
{
    setSelected(false);
}

/////////////////////////////////////////////////////////////////////////////

void GraphicElementBase::setSelected(bool is_selected) 
{
    m_is_selected = is_selected;
    setColor();
}

/////////////////////////////////////////////////////////////////////////////

void GraphicElementBase::setHighlighted(bool is_highlighted) 
{
    m_is_highlighted = is_highlighted;
    setColor();
}

/////////////////////////////////////////////////////////////////////////////

void GraphicElementBase::setColor()
{
//     if (!m_is_moveable) 
//     {
//         m_active_color = m_normal_color;
//         return;
//     }
    
    if (m_is_selected) m_active_color = m_selected_color;
    else if (m_is_highlighted) m_active_color = m_highlighted_color;
    else m_active_color = m_normal_color;
}

// End of file
