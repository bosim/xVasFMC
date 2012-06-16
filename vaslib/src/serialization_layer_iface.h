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

/*! \file    serialization_layer_iface.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __SERIALIZATION_LAYER_IFACE_H__
#define __SERIALIZATION_LAYER_IFACE_H__

#include <QDataStream>

#include "transport_layer_iface.h"

#include "containerbase.h"

//! serialization interface
/*! more details ...
 */
class SerializationLayerIface : public QObject
{
    Q_OBJECT

public:
    
    typedef enum SERIALIZATION_TYPE { TYPE_BASE = 0,
                                      TYPE_PLAIN = 1,
    };

public:
    //! Standard Constructor
    SerializationLayerIface(SERIALIZATION_TYPE type,
                            TransportLayerIface* transport_layer = 0);

    //! Destructor
    virtual ~SerializationLayerIface();

    //! encodes the given container to the given array.
    //! returns true on success, false otherwise.
    virtual bool encode(const ContainerBaseList* containerlist, QDataStream& stream) = 0;

    //! encodes the given container and send the result to the underlying
    //! transport layer. returns true on success, false otherwise.    
    virtual bool encodeAndSend(const ContainerBaseList* containerlist);
    
    //! decodes the given array and returns a pointer to the decoded container. 
    //! ATTENTION: the caller is responsible to delete the returned container!!
    //! will return a null pointer on error.
    virtual ContainerBaseList* decode(QDataStream& stream) = 0;
    
signals:

    //! emitted whenever a new container was received by the underlying
    //! transport layer. 
    void signalContainerReceived(const ContainerBaseList* containerlist);

protected slots:

    //! called when the underlying transport layer received data
    void slotDataReceived(QByteArray& received_data);

protected:

    //! the type of serialization
    SERIALIZATION_TYPE m_type;

    //! the transport layer for message sending
    TransportLayerIface* m_transport_layer;

private:
    //! Hidden copy-constructor
    SerializationLayerIface(const SerializationLayerIface&);
    //! Hidden assignment operator
    const SerializationLayerIface& operator = (const SerializationLayerIface&);
};

#endif /* __SERIALIZATION_LAYER_IFACE_H__ */

// End of file

