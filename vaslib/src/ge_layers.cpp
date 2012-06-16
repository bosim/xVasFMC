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

/*! \file    ge_layers.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "ge.h"
#include "ge_layers.h"

/////////////////////////////////////////////////////////////////////////////

GraphicElementLayerList::GraphicElementLayerList()
{
}

/////////////////////////////////////////////////////////////////////////////

GraphicElementLayerList::~GraphicElementLayerList()
{
    qDeleteAll(m_layer_map);
    m_layer_map.clear();
}

/////////////////////////////////////////////////////////////////////////////

void GraphicElementLayerList::clear()
{
    qDeleteAll(m_layer_map);
    m_layer_map.clear(); 
}

/////////////////////////////////////////////////////////////////////////////

void GraphicElementLayerList::addLayerWhenNotExisting(unsigned int layer)
{
    if (m_layer_map.find(layer) == m_layer_map.end())
    {
        GraphicElementList* new_list = new GraphicElementList;
        MYASSERT(new_list);
        m_layer_map.insert(layer, new_list);
    }

    MYASSERT(m_layer_map.find(layer) != m_layer_map.end());
}


/////////////////////////////////////////////////////////////////////////////

void GraphicElementLayerList::addElementByLayer(unsigned int layer, GraphicElementBase* element)
{
    MYASSERT(element != 0);

    addLayerWhenNotExisting(layer);
    GraphicElementList *list = m_layer_map.find(layer).value();
    MYASSERT(list != 0);
    list->addElement(element);
}

/////////////////////////////////////////////////////////////////////////////

void GraphicElementLayerList::draw(QPainter& painter) const
{
    painter.save();

    GraphicElementLayerIterator iter = iterator();
    while(iter.hasNext())
    {
        iter.next();

        GraphicElementList* list = iter.value();
        MYASSERT(list != 0);

        unsigned int max_count = list->count();
        for(unsigned int index=0; index<max_count; ++index)
        {
            GraphicElementBase* element = list->getElementByIndex(index);
            MYASSERT(element != 0);
            element->draw(painter);
        }
    }

    painter.restore();
}

/////////////////////////////////////////////////////////////////////////////

void GraphicElementLayerList::unselectAll()
{
    GraphicElementLayerIterator iter = iterator();
    iter.toBack();
    while(iter.hasPrevious())
    {
        iter.previous();
        GraphicElementList* list = iter.value();
        MYASSERT(list != 0);
        list->unselectAll();
    }  
}

/////////////////////////////////////////////////////////////////////////////

GraphicElementBase* GraphicElementLayerList::mouseClick(QPoint point)
{
    bool already_used = false;

    GraphicElementBase* selected_element = 0;

    GraphicElementLayerIterator iter = iterator();
    iter.toBack();
    while(iter.hasPrevious())
    {
        iter.previous();

        GraphicElementList* list = iter.value();
        MYASSERT(list != 0);
        GraphicElementBase* element = list->mouseClick(point, already_used);
        if (element != 0) selected_element = element;
    }  

    return selected_element;
}

/////////////////////////////////////////////////////////////////////////////

void GraphicElementLayerList::mouseMove(QPoint point, bool do_highlight)
{
    bool already_used = false;

    GraphicElementLayerIterator iter = iterator();
    iter.toBack();
    while(iter.hasPrevious())
    {
        iter.previous();
        
        GraphicElementList* list = iter.value();
        MYASSERT(list != 0);
        already_used |= list->mouseMove(point, already_used, do_highlight);
    }  
}

/////////////////////////////////////////////////////////////////////////////

void GraphicElementLayerList::mouseRelease(QPoint point)
{
    GraphicElementLayerIterator iter = iterator();
    iter.toBack();
    while(iter.hasPrevious())
    {
        iter.previous();
        
        GraphicElementList* list = iter.value();
        MYASSERT(list != 0);
        list->mouseRelease(point);
    }  
}

// End of file
