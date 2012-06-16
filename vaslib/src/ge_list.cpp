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

/*! \file    ge_list.cpp
  \author  Alexander Wemmer, alex@wemmer.at
*/

#include "assert.h"
#include "ge.h"

#include "ge_list.h"

/////////////////////////////////////////////////////////////////////////////

GraphicElementList::GraphicElementList(bool enable_autodelete) :
    m_enable_autodelete(enable_autodelete)
{
}

/////////////////////////////////////////////////////////////////////////////

GraphicElementList::~GraphicElementList()
{
    if (m_enable_autodelete) qDeleteAll(m_element_list);
    m_element_list.clear();
}

/////////////////////////////////////////////////////////////////////////////

void GraphicElementList::clear() 
{ 
    if (m_enable_autodelete) qDeleteAll(m_element_list); 
    m_element_list.clear();
}

/////////////////////////////////////////////////////////////////////////////

void GraphicElementList::addElement(GraphicElementBase* element)
{
    MYASSERT(element != 0);
    m_element_list.append(element);
    element->setTerminationReceiver(this);
}

/////////////////////////////////////////////////////////////////////////////

GraphicElementBase* GraphicElementList::getElementByIndex(unsigned int index) const
{
    MYASSERT(index < count());
    return m_element_list.at(index);
}

/////////////////////////////////////////////////////////////////////////////

void GraphicElementList::deleteElementByIndex(unsigned int index)
{
    MYASSERT(index < count());
    GraphicElementBase* element = m_element_list.at(index);
    m_element_list.removeAt(index);
    if (m_enable_autodelete) delete element;
}

/////////////////////////////////////////////////////////////////////////////

void GraphicElementList::unselectAll()
{
    unsigned int max_count = count();
    for(unsigned int index=0; index < max_count; ++index)
    {
        GraphicElementBase* iter_element = m_element_list.at(index);
        MYASSERT(iter_element != 0);
        iter_element->setSelected(false);
    }
}

/////////////////////////////////////////////////////////////////////////////

GraphicElementBase* GraphicElementList::mouseClick(QPoint point, bool& already_used)
{
    GraphicElementBase* selected_element = 0;

    unsigned int max_count = count();
    for(unsigned int index=0; index < max_count; ++index)
    {
        GraphicElementBase* iter_element = m_element_list.at(index);
        MYASSERT(iter_element != 0);
        GraphicElementBase* element = iter_element->mouseClick(point, already_used);
        if (element != 0) selected_element = element;
    }
  
    return selected_element;
}

/////////////////////////////////////////////////////////////////////////////

bool GraphicElementList::mouseMove(QPoint point, bool already_used, bool do_highlight)
{
    unsigned int max_count = count();
    for(unsigned int index=0; index < max_count; ++index)
    {
        GraphicElementBase* element = m_element_list.at(index);
        MYASSERT(element != 0);
        already_used |= element->mouseMove(point, already_used, do_highlight);
    }

    return already_used;
}

/////////////////////////////////////////////////////////////////////////////

void GraphicElementList::mouseRelease(QPoint point)
{
    unsigned int max_count = count();
    for(unsigned int index=0; index < max_count; ++index)
    {
        GraphicElementBase* element = m_element_list.at(index);
        MYASSERT(element != 0);
        element->mouseRelease(point);
    }
}

/////////////////////////////////////////////////////////////////////////////

void GraphicElementList::terminated(TermiationCallbackEmitter* emitter)
{
    MYASSERT(emitter != 0);
    if (m_element_list.contains(((GraphicElementBase*)emitter))) 
        m_element_list.removeAll(((GraphicElementBase*)emitter));
}

// End of file
