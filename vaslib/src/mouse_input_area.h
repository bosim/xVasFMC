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

/*! \file    mouse_input_area.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef MOUSE_INPUT_AREA_H
#define MOUSE_INPUT_AREA_H

#include <QRect>
#include <QObject>
#include <QMouseEvent>
#include <QTimer>
#include <QDateTime>

#include "logger.h"
#include "assert.h"

class QWidget;
class QCursor;

/////////////////////////////////////////////////////////////////////////////

//! input area definition
class InputArea : public QRect
{
public:

    InputArea() : 
        m_repeat_period_ms(0), m_long_click_delay_ms(0), m_mouse_buttons(Qt::LeftButton) {};

    InputArea(const QString& name) : 
        m_name(name), m_repeat_period_ms(0), m_long_click_delay_ms(0), m_mouse_buttons(Qt::LeftButton)
    {
        MYASSERT(!name.isEmpty());
    };

    //! Standard Constructor
    InputArea(const QString&name, int x, int y, int w, int h,
              uint repeat_period_ms = 0, uint long_click_delay_ms = 0,
              const Qt::MouseButton& mouse_buttons = Qt::LeftButton) : 
        QRect(x, y, w, h), m_name(name), m_repeat_period_ms(repeat_period_ms), 
        m_long_click_delay_ms(long_click_delay_ms), m_mouse_buttons(mouse_buttons)
    {
        MYASSERT(!name.isEmpty());
    };
    
    //! Destructor
    virtual ~InputArea() {};

    const QString& name() const { return m_name; }
    uint repeatPeriodMs() const { return m_repeat_period_ms; }
    uint longClickDelayMs() const { return m_long_click_delay_ms; }
    const Qt::MouseButton& mouseButtons() const { return m_mouse_buttons; }

    QString saveToString() const;
    bool loadFromString(const QString& text);
    
protected:

    QString m_name;
    uint m_repeat_period_ms;
    uint m_long_click_delay_ms;
    Qt::MouseButton m_mouse_buttons;
};

typedef QList<InputArea> InputAreaList;
typedef QListIterator<InputArea> InputAreaListIterator;

/////////////////////////////////////////////////////////////////////////////

//! MouseInputArea
class MouseInputArea : public QObject
{
    Q_OBJECT

public:

    //! Standard Constructor
    MouseInputArea() : m_repeat_period_ms(0), m_emit_long_click(false)
    {
        MYASSERT(connect(&m_input_timer, SIGNAL(timeout()), this, SLOT(slotInputTimer())));
        m_input_timer.start(100);
    };

    //! Destructor
    virtual ~MouseInputArea() {};

    //! clears all area definitions and resets all values
    void clear();
    //! Sets the input area list to the given one, the existing will be cleared.
    void setInputAreaList(const InputAreaList& input_area_list);
    //! returns a const reference to the input list
    const InputAreaList& inputAreaList() const { return m_input_area_list; }

    //! saves the area definitions to the given file
    bool saveToFile(const QString& filename);
    //! clears the existing area definitions and loads the new ones from the given file
    bool loadFromFile(const QString& filename);

    //! Returns the first input area with the given name or an empty if none was found
    const InputArea& inputArea(const QString& name);

signals:

    //! ATTENTION: the event may be NULL
    void signalMouseClick(const QString& text, QMouseEvent* event);
    //! ATTENTION: the event may be NULL
    void signalMouseLongClick(const QString& text, QMouseEvent* event); 
    void signalMouseWheel(const QString& text, QWheelEvent* event); 

public slots:

    // sets the mouse cursor of the given widget if the cursor is inside a watched area to the given cursor
    void slotMouseMoveEvent(QMouseEvent* event, QWidget* widget, const QCursor& cursor);
    void slotMousePressEvent(QMouseEvent *event);
    void slotMouseReleaseEvent(QMouseEvent *event);
    void slotMouseWheelEvent(QWheelEvent *event);

protected slots:

    void slotInputTimer();

protected:

    InputAreaList m_input_area_list;
    InputArea m_current_area;
    QTime m_repeat_elapse_timer;
    uint m_repeat_period_ms;
    QTime m_long_click_elapse_timer;
    bool m_emit_long_click;
    QTimer m_input_timer;

    InputArea m_empty_area;

private:
    //! Hidden copy-constructor
    MouseInputArea(const MouseInputArea&);
    //! Hidden assignment operator
    const MouseInputArea& operator = (const MouseInputArea&);
};

#endif /* MOUSE_INPUT_AREA_H */

// End of file

