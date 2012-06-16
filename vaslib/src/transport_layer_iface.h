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

/*! \file    transport_layer_iface.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __TRANSPORT_LAYER_IFACE_H__
#define __TRANSPORT_LAYER_IFACE_H__

#include <QObject>
#include <QByteArray>
#include <QAbstractSocket>

#include "assert.h"

//#include "serialization_layer_iface.h"

//! raw transport layer
/*! more details ...
 */
class TransportLayerIface : public QObject
{
    Q_OBJECT

public:

    //! Standard Constructor
    TransportLayerIface() {};

    //! Destructor
    virtual ~TransportLayerIface() {};

    //! sends the given data, returns the true on success, false otherwise.
    virtual bool sendData(qint16 data_type, const QByteArray& data) = 0;

    //! Clears the internal receive buffer and connects to the given host.
    //! When already connected (isConnected = true) no actions are performed.
    virtual void connectToHost(const QString& server, qint16 port) = 0;

    //! Disconnects and lets the internal receive buffer untouched.
    virtual void disconnectFromHost(bool forced = true) = 0;

    //! Returns true if connected and ready, false otherwise.
    virtual bool isConnected() const = 0;

    //! returns a the error text of the given socket error
    static QString getSocketErrorText(QAbstractSocket::SocketError error);

    //! adds a frame to the data and returns the framed data in "enframed_data".
    static void enframeData(qint16 data_type, const QByteArray& data, QByteArray& enframed_data);

    //! tries to decode a frame from the data. Returnes true if a frame was
    //! decoded successfully and returns the deframed data in "deframed_data".
    //! Returns false on error.
    static bool deframeReceivedData(QByteArray& data, 
                                    qint16& data_type, 
                                    QByteArray& deframed_data);
    
signals:

    //! emitted when new data were received
    void signalDataReceived(qint16 data_type, QByteArray& receive_buffer);

    void signalConnected();
    void signalDisconnected();
    void signalError(QAbstractSocket::SocketError error, const QString& error_msg);

private:
    //! Hidden copy-constructor
    TransportLayerIface(const TransportLayerIface&);
    //! Hidden assignment operator
    const TransportLayerIface& operator = (const TransportLayerIface&);
};

#endif /* __TRANSPORT_LAYER_IFACE_H__ */

// End of file

