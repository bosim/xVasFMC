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

/*! \file    ge.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __GE_H__
#define __GE_H__

#include <QObject>
#include <QPainter>
#include <QPainterPath>
#include <QPoint>
#include <QRect>

#include "treebase.h"
#include "termination_callback.h"

/////////////////////////////////////////////////////////////////////////////

//! graphic element base class
class GraphicElementBase : public QObject, public TermiationCallbackEmitter
{
    Q_OBJECT

signals:

    void signalSelectionChanged(GraphicElementBase* element, bool is_selected);

public:
    //! Standard Constructor
    GraphicElementBase(bool is_moveable);

    //! Destructor
    virtual ~GraphicElementBase();

    //-----

    //! Draws this element using the given painter
    virtual void draw(QPainter& painter) = 0;

    //! Returns the bounding rect of the element
    virtual const QRect& getBoundingRect() { return m_bounding_rect; }

    //! Returns the rect of the element used for selection (with the mouse)
    virtual const QPainterPath& getSelectorPath() { return m_selector_path; }

    //! returns the selected element
    virtual GraphicElementBase* mouseClick(QPoint point, bool& already_used);
    virtual bool mouseMove(QPoint point, bool already_used, bool do_highlight);
    virtual void mouseRelease(QPoint point);

    void setUsed(const bool used) { m_used = used; }
    const bool isUsed() const { return m_used; }

    bool isSelected() const { return m_is_selected; }
    bool isHighlighted() const { return m_is_highlighted; }

    virtual void setSelected(bool is_selected);
    virtual void setHighlighted(bool is_highlighted);

    const QColor& getNormalColor() const { return m_normal_color; }
    virtual void setNormalColor(const QColor& color) { m_normal_color = color; }

    void setTerminationReceiver(TermiationCallbackReceiver* receiver)
    { m_termination_receiver = receiver; }

    void setDataElement(TreeBase* data_element) { m_data_element = data_element; }
    TreeBase* getDataElement() const { return m_data_element; }

protected:

    virtual void recalcBoundingAndSelectorElements() = 0;
    virtual void setColor();
    
protected:

    int FULL_ARC;
    bool m_is_moveable;
    bool m_used;

    QRect m_bounding_rect;
    QPainterPath m_selector_path;

    QColor m_active_color;
    QColor m_normal_color;

    bool m_is_selected;
    QColor m_selected_color;

    bool m_is_highlighted;
    QColor m_highlighted_color;

    QPoint m_last_mouse_click_pos;
    TermiationCallbackReceiver* m_termination_receiver;
    TreeBase* m_data_element;

private:
    //! Hidden copy-constructor
    GraphicElementBase(const GraphicElementBase&);
    //! Hidden assignment operator
    const GraphicElementBase& operator = (const GraphicElementBase&);
};

#endif /* __GE_H__ */

// End of file

