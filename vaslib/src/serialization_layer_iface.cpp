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

/*! \file    serialization_layer_iface.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "serialization_layer_iface.h"

/////////////////////////////////////////////////////////////////////////////

SerializationLayerIface::SerializationLayerIface(
    SERIALIZATION_TYPE type, TransportLayerIface* transport_layer) :
    m_type(type), m_transport_layer(transport_layer)
{
    if (m_transport_layer)
    {
        MYASSERT(connect(transport_layer, SIGNAL(signalDataReceived(QByteArray&)),
                         this, SLOT(slotDataReceived(QByteArray&))));
    }
}

/////////////////////////////////////////////////////////////////////////////

SerializationLayerIface::~SerializationLayerIface()
{
}

/////////////////////////////////////////////////////////////////////////////

bool SerializationLayerIface::encodeAndSend(const ContainerBaseList* containerlist)
{
    MYASSERT(containerlist);
    MYASSERT(m_transport_layer);

    QByteArray buffer;
    QDataStream writestream(&buffer, QIODevice::WriteOnly);
    if (!encode(containerlist, writestream)) 
    {
        qCritical("SerializationLayerIface:encodeAndSend: "
                  "Could not encode data from container (%d)",
                  containerlist->getType());
        return false;
    }

    if (!m_transport_layer->sendData(buffer))
    {
        qCritical("SerializationLayerIface:encodeAndSend: "
                  "Could not send data from container (%d)",
                  containerlist->getType());
        return false;
    }
    
    return true;    
}

/////////////////////////////////////////////////////////////////////////////

void SerializationLayerIface::slotDataReceived(QByteArray& received_data)
{
    QDataStream readstream(&received_data, QIODevice::ReadOnly);
    
    ContainerBaseList* containerlist = decode(readstream);

    if (!containerlist)
    {
        qCritical("SerializationLayerIface:slotDataReceived: "
                  "Could not decode data from buffer of size (%d)",
                  received_data.count());
        return;
    }

    emit signalContainerReceived(containerlist);
    delete containerlist;
    containerlist = 0;
}

/////////////////////////////////////////////////////////////////////////////

// End of file
