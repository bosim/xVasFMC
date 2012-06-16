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

/*! \file    ge_list.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __GE_LIST_H__
#define __GE_LIST_H__

#include <QList>
#include <QPoint>

#include "termination_callback.h"

class GraphicElementBase;

/////////////////////////////////////////////////////////////////////////////

//! list of graphic elements with optional autodelete.
class GraphicElementList : public TermiationCallbackReceiver
{
public:
    //! Standard Constructor
    GraphicElementList(bool enable_autodelete = false);

    //! Destructor
    virtual ~GraphicElementList();

    //! Delete all elements from the list.
    void clear();

    //! Returns the number of elements in the list
    unsigned int count() const { return m_element_list.count(); }

    //! Adds the given element to the list with autodelete
    void addElement(GraphicElementBase* element);

    //! Returns the elements with the given index
    GraphicElementBase* getElementByIndex(unsigned int index) const;

    //! Deletes the element with the given index
    void deleteElementByIndex(unsigned int index);

    //! Returns the selected element
    GraphicElementBase* mouseClick(QPoint point, bool& already_used);
    bool mouseMove(QPoint point, bool already_used, bool do_highlight);
    void mouseRelease(QPoint point);
    void unselectAll();

    void terminated(TermiationCallbackEmitter* emitter);

protected:
    
    //! The list of graphical elements
    QList<GraphicElementBase*> m_element_list;

    bool m_enable_autodelete;

private:
    //! Hidden copy-constructor
    GraphicElementList(const GraphicElementList&);
    //! Hidden assignment operator
    const GraphicElementList& operator = (const GraphicElementList&);
};

#endif /* __GE_LIST_H__ */

// End of file
