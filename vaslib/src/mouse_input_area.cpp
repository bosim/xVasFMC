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

/*! \file    mouse_input_area.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include <QWidget>
#include <QCursor>
#include <QFile>
#include <QTextStream>

#include "vas_path.h"
#include "mouse_input_area.h"

/////////////////////////////////////////////////////////////////////////////

QString InputArea::saveToString() const
{
    return QString("%1,%2,%3,%4,%5,%6,%7").
        arg(left()).arg(top()).arg(width()).arg(height()).
        arg(m_repeat_period_ms).arg(m_long_click_delay_ms).arg(m_mouse_buttons);
}

/////////////////////////////////////////////////////////////////////////////

bool InputArea::loadFromString(const QString& text)
{
    QStringList items = text.split(",");
    if (items.count() != 7)
    {
        Logger::log(QString("InputArea:loadFromString: Could not parse %1").arg(text));
        return false;
    }

    bool convok = false;

    //-----
    uint left = items[0].toUInt(&convok);
    if (!convok)
    {
        Logger::log(QString("InputArea:loadFromString: Could not parse (left) %1").arg(text));
        return false;
    }
    //-----
    uint top = items[1].toUInt(&convok);
    if (!convok)
    {
        Logger::log(QString("InputArea:loadFromString: Could not parse (top) %1").arg(text));
        return false;
    }
    //-----
    uint width = items[2].toUInt(&convok);
    if (!convok)
    {
        Logger::log(QString("InputArea:loadFromString: Could not parse (width) %1").arg(text));
        return false;
    }
    //-----
    uint height = items[3].toUInt(&convok);
    if (!convok)
    {
        Logger::log(QString("InputArea:loadFromString: Could not parse (height) %1").arg(text));
        return false;
    }
    //-----
    uint repeat_period_ms = items[4].toUInt(&convok);
    if (!convok)
    {
        Logger::log(QString("InputArea:loadFromString: Could not parse (repeat_period_ms) %1").arg(text));
        return false;
    }
    //-----
    uint long_click_delay = items[5].toUInt(&convok);
    if (!convok)
    {
        Logger::log(QString("InputArea:loadFromString: Could not parse (long_click_delay) %1").arg(text));
        return false;
    }
    //-----
    int mouse_button = items[6].toUInt(&convok);
    if (!convok)
    {
        Logger::log(QString("InputArea:loadFromString: Could not parse (mouse_button) %1").arg(text));
        return false;
    }

    setX(left);
    setY(top);
    setWidth(width);
    setHeight(height);
    m_repeat_period_ms = repeat_period_ms;
    m_long_click_delay_ms = long_click_delay;
    m_mouse_buttons = (Qt::MouseButton)mouse_button;
    
    return true;
}

/////////////////////////////////////////////////////////////////////////////

void MouseInputArea::slotMousePressEvent(QMouseEvent *event)
{
    InputAreaListIterator iter(m_input_area_list);
    while(iter.hasNext())
    {
        const InputArea& area = iter.next();

        if (area.contains(event->pos()) && area.mouseButtons() == event->buttons())
        {
            m_current_area = InputArea();
            m_repeat_period_ms = 0;
            m_emit_long_click = false;

            if (area.repeatPeriodMs() > 0)
            {
                m_current_area = area;
                m_repeat_elapse_timer.start();
                m_repeat_period_ms = 300;
            }

            if (area.longClickDelayMs() > 0)
            {
                m_current_area = area;
                m_long_click_elapse_timer.start();
                m_emit_long_click = true;
            }
            else
            {
                // if we have a long click, we emit the signal when the button is released
                emit signalMouseClick(area.name(), event);
            }

            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void MouseInputArea::setInputAreaList(const InputAreaList& input_area_list) 
{
    clear();
    m_input_area_list = input_area_list;
}

/////////////////////////////////////////////////////////////////////////////

void MouseInputArea::slotMouseReleaseEvent(QMouseEvent *event)
{
    // we emit the normal mouse click event if this is a long click area and
    // the long click was not emitted yet
    if (!m_current_area.isEmpty() && m_current_area.longClickDelayMs() > 0 && m_emit_long_click)
        emit signalMouseClick(m_current_area.name(), event);

    m_current_area = InputArea();
    m_repeat_period_ms = 0;
    m_emit_long_click = false;
}

/////////////////////////////////////////////////////////////////////////////

void MouseInputArea::slotInputTimer()
{
    if (m_current_area.name().isEmpty()) return;

    if (m_repeat_period_ms > 0 && m_repeat_elapse_timer.elapsed() >= (int)m_repeat_period_ms)
    {
        emit signalMouseClick(m_current_area.name(), 0);
        m_repeat_elapse_timer.start();
        m_repeat_period_ms = m_current_area.repeatPeriodMs();
    }

    if (m_current_area.longClickDelayMs() >  0 && m_emit_long_click && 
        m_long_click_elapse_timer.elapsed() >= (int)m_current_area.longClickDelayMs())
    {
        m_emit_long_click = false;
        emit signalMouseLongClick(m_current_area.name(), 0);
    }

}

/////////////////////////////////////////////////////////////////////////////

void MouseInputArea::slotMouseMoveEvent(QMouseEvent* event, QWidget* widget, const QCursor& cursor)
{
    MYASSERT(widget != 0);
    InputAreaListIterator iter(m_input_area_list);
    while(iter.hasNext())
    {
        if (iter.next().contains(event->pos()))
        {
            widget->setCursor(cursor);
            return;
        }
    }    

    widget->unsetCursor();
}

/////////////////////////////////////////////////////////////////////////////

void MouseInputArea::slotMouseWheelEvent(QWheelEvent *event)
{
    InputAreaListIterator iter(m_input_area_list);
    while(iter.hasNext())
    {
        const InputArea& area = iter.next();

        if (area.contains(event->pos()))
        {
            emit signalMouseWheel(area.name(), event);
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void MouseInputArea::clear()
{
    m_input_area_list.clear();
    m_current_area = InputArea();
    m_repeat_period_ms = 0;
    m_emit_long_click = false;
}

/////////////////////////////////////////////////////////////////////////////

bool MouseInputArea::saveToFile(const QString& filename)
{
    MYASSERT(!filename.isEmpty());
    QFile file(VasPath::prependPath(filename));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) 
    {
        Logger::log(QString("MouseInputArea:saveToFile: Could not open file %1 - %2").
                    arg(filename).arg(file.errorString()));
        return false;
    }

    QTextStream outstream(&file);

    InputAreaListIterator iter(m_input_area_list);
    while(iter.hasNext())
    {
        const InputArea& area = iter.next();
        outstream << area.name() << "=" << area.saveToString() << "\r\n";
    }    
    
    file.close();
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool MouseInputArea::loadFromFile(const QString& filename)
{
    clear();

    MYASSERT(!filename.isEmpty());
    QFile file(VasPath::prependPath(filename));
    if (!file.open(QIODevice::ReadOnly)) 
    {
        Logger::log(QString("MouseInputArea:loadfromFile: Could not open file %1 - %2").
                    arg(filename).arg(file.errorString()));
        return false;
    }
    
    unsigned long line_counter = 0;
    QTextStream instream(&file);

    while (!instream.atEnd())
    {
        ++line_counter;

        QString line = instream.readLine();
        QStringList items = line.split("=");
        if (items.count() != 2)
        {
            Logger::log(QString("Config:loadfromFile: Could not parse line %1 (%2)").arg(line_counter).arg(line));
            return false;
        }
    
        InputArea new_area(items[0]);
        if (!new_area.loadFromString(items[1]))
        {
            Logger::log(QString("Config:loadfromFile: Could not parse input aree %1 (%2)").arg(line_counter).arg(line));
            return false;
        }

        m_input_area_list.append(new_area);
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////

const InputArea& MouseInputArea::inputArea(const QString& name)
{
    MYASSERT(!name.isEmpty());
    
    InputAreaListIterator iter(m_input_area_list);
    while(iter.hasNext())
    {
        const InputArea& area = iter.next();
        if (area.name() == name) return area;
    }
    
    return m_empty_area;
}

// End of file
