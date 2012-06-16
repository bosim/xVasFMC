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

/*! \file    containerbase.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "assert.h"

#include "containerbase.h"

/////////////////////////////////////////////////////////////////////////////

ContainerBase::ContainerBase(CONTAINER_TYPE type) :
    m_type(type)
{
}

/////////////////////////////////////////////////////////////////////////////

ContainerBase::~ContainerBase()
{
}

/////////////////////////////////////////////////////////////////////////////

void ContainerBase::addField(const QString& name, QString* value)
{
    MYASSERT(!name.isEmpty());
    MYASSERT(value);
    MYASSERT(!m_field_hash.contains(name));
    m_field_hash.insert(name, value);
}

/////////////////////////////////////////////////////////////////////////////

const QString& ContainerBase::get(const QString& name) const
{
    if (!m_field_hash.contains(name)) return m_null_string;
    return *(m_field_hash[name]);
}

/////////////////////////////////////////////////////////////////////////////

bool ContainerBase::set(const QString& name, const QString& value)
{
    if (!m_field_hash.contains(name)) return false;
    *(m_field_hash[name]) = value;
    return true;
}

/////////////////////////////////////////////////////////////////////////////

QString ContainerBase::toString() const
{
    QString result = QString("Container(%1): ").arg(getType());

    QStringListIterator field_iter(getKeyList());
    while(field_iter.hasNext())
    {
        const QString& key = field_iter.next();
        result += QString("[%1]=%2").arg(key).arg(get(key));
        if (field_iter.hasNext()) result += ", ";
    }

    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

ContainerBaseList::ContainerBaseList(CONTAINER_LIST_TYPE type) :
    m_type(type)
{
}

/////////////////////////////////////////////////////////////////////////////

ContainerBaseList::~ContainerBaseList()
{
}

/////////////////////////////////////////////////////////////////////////////

bool ContainerBaseList::add(ContainerBase* container)
{
    MYASSERT(container);
    MYASSERT(!container->getKey().isEmpty());
    
    if (m_container_hash.contains(container->getKey())) return false;
    m_container_hash.insert(container->getKey(), container);
    return true;
}

/////////////////////////////////////////////////////////////////////////////

ContainerBase* ContainerBaseList::get(const QString& key) const
{
    if (!m_container_hash.contains(key)) return NULL;
    return m_container_hash[key];
}

/////////////////////////////////////////////////////////////////////////////

bool ContainerBaseList::remove(const QString& key)
{
    if (!m_container_hash.contains(key)) return false;
    m_container_hash.remove(key);
    return true;
}

/////////////////////////////////////////////////////////////////////////////

QString ContainerBaseList::toString() const
{
    QString result = QString("ContainerList(%1): ").arg(getType());

    QStringListIterator container_iter(getKeyList());
    while(container_iter.hasNext())
    {
        const QString& key = container_iter.next();
        result += key;
        //FIXXME print all containers
        if (container_iter.hasNext()) result += ", ";
    }

    return result;
}

/////////////////////////////////////////////////////////////////////////////

// End of file
