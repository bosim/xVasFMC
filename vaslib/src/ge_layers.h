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

/*! \file    ge_layers.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef GE_LAYERS_H
#define GE_LAYERS_H

#include <QMap>
#include <QPainter>

#include "ge_list.h"

class GraphicElementBase;

/////////////////////////////////////////////////////////////////////////////

typedef QMapIterator<unsigned int,GraphicElementList*> GraphicElementLayerIterator;

/////////////////////////////////////////////////////////////////////////////

//! layers of lists of graphical elements
class GraphicElementLayerList
{
public:
    //! Standard Constructor
    GraphicElementLayerList();

    //! Destructor
    virtual ~GraphicElementLayerList();

    //! Deletes all elements of all layers
    void clear();

    //! Returns the number of layers
    unsigned int count() const { return m_layer_map.count(); }

    //! Adds the given element to the given layer with autodelete.
    //! Layers will be drawn in an ascending manner.
    void addElementByLayer(unsigned int layer, GraphicElementBase* element);

    //! Draws the elements of this layer list in ascending order regarding to
    //! the layer number.
    void draw(QPainter& painter) const;

    //! Returns an iterator to the layer lists sorted by the layer index.
    GraphicElementLayerIterator iterator() const 
    { return GraphicElementLayerIterator(m_layer_map); }

    GraphicElementList* getLayer(unsigned int layer) 
    { 
        addLayerWhenNotExisting(layer);
        return m_layer_map.find(layer).value(); 
    }

    //! Returns the selected element
    GraphicElementBase* mouseClick(QPoint point);
    void mouseMove(QPoint point, bool do_highlight);
    void mouseRelease(QPoint point);
    void unselectAll();

protected:

    void addLayerWhenNotExisting(unsigned int layer);

protected:

    //! layer list of graphical elements
    QMap<unsigned int, GraphicElementList*> m_layer_map;

private:
    //! Hidden copy-constructor
    GraphicElementLayerList(const GraphicElementLayerList&);
    //! Hidden assignment operator
    const GraphicElementLayerList& operator = (const GraphicElementLayerList&);
};

#endif /* GE_LAYERS_H */

// End of file
