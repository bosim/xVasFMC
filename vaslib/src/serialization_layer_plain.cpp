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

/*! \file    serialization_layer_plain.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "containerfactory.h"

#include "serialization_layer_plain.h"

/////////////////////////////////////////////////////////////////////////////

SerializationLayerPlain::SerializationLayerPlain(TransportLayerIface* transport_layer) :
    SerializationLayerIface(SerializationLayerIface::TYPE_PLAIN, transport_layer)
{
}

/////////////////////////////////////////////////////////////////////////////

SerializationLayerPlain::~SerializationLayerPlain()
{
}

/////////////////////////////////////////////////////////////////////////////

bool SerializationLayerPlain::encode(const ContainerBaseList* containerlist, QDataStream& stream)
{
    MYASSERT(containerlist);

    stream << (qint32)m_type
           << (qint32)containerlist->getType()
           << (qint32)containerlist->count();

    //----- write all containers

    QStringListIterator container_iter(containerlist->getKeyList());
    while(container_iter.hasNext())
    {
        ContainerBase* container = containerlist->get(container_iter.next());
        MYASSERT(container);

        stream << (qint32) container->getType()
               << (qint32) container->count();

        QStringListIterator field_iter(container->getKeyList());
        while(field_iter.hasNext())
        {
            const QString& key = field_iter.next();
            stream << key 
                   << container->get(key);
        }
    }

    return (stream.status() == QDataStream::Ok);
}

/////////////////////////////////////////////////////////////////////////////    

ContainerBaseList* SerializationLayerPlain::decode(QDataStream& stream)
{
    qint32 serialization_type = 0;
    qint32 containerlist_type = 0;
    qint32 container_count = 0;

    stream >> serialization_type
           >> containerlist_type
           >> container_count;

    if (serialization_type != m_type) 
    {
        qCritical("SerializationLayerPlain:decode: "
                  "received serialization type (%d) != our type (%d)",
                  serialization_type, m_type);
        return NULL;
    }

    ContainerBaseList* containerlist = 
        ContainerFactory::getContainerListByType(
            (ContainerBaseList::CONTAINER_LIST_TYPE)containerlist_type);

    if (!containerlist)
    {
        qCritical("SerializationLayerPlain:decode: "
                  "Could not get container list of type (%d)",
                  containerlist_type);
        return NULL;
    }

    //----- read all containers    

    for (int list_index=0; list_index < container_count; ++list_index)
    {
        qint32 container_type = 0;
        qint32 field_count = 0;

        stream >> container_type
               >> field_count;

        ContainerBase* container = 
            ContainerFactory::getContainerByType(
                (ContainerBase::CONTAINER_TYPE) container_type);

        if (!container)
        {
            qCritical("SerializationLayerPlain:decode: "
                      "Could not get container of type (%d)",
                      container_type);
            return NULL;
        }

        for (int field_index=0; field_index<field_count; ++field_index)
        {
            QString key;
            QString value;

            stream >> key
                   >> value;

            if (!container->set(key, value))
            {
                qCritical("SerializationLayerPlain:decode: "
                          "Could not set key (%s) to value (%s) for "
                          "container type (%d)",
                          key.toLatin1().data(), value.toLatin1().data(), container_type);
            }
        }   

        if (!containerlist->add(container))
        {
            qCritical("SerializationLayerPlain:decode: "
                      "Could not add container of type (%d) to list of type (%d)",
                      container_type, containerlist_type);
        }
    }

    return containerlist;
}

/////////////////////////////////////////////////////////////////////////////

// End of file
