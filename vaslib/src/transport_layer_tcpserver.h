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

/*! \file    transport_layer_tcpserver.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __TRANSPORT_LAYER_TCPSERVER_H__
#define __TRANSPORT_LAYER_TCPSERVER_H__

#include <QTcpServer>
#include <QHash>

#include "transport_layer_iface.h"

class TransportLayerTCPServerClientBuffer;

/////////////////////////////////////////////////////////////////////////////

//! tcp server transport layer
/*! more details ...
 */
class TransportLayerTCPServer : public TransportLayerIface
{
    Q_OBJECT

public:
    //! Standard Constructor
    TransportLayerTCPServer();

    //! Destructor
    virtual ~TransportLayerTCPServer();

    //! Starts listening on the given host/port. If "server" is empty, we
    //! listen on all interfaces.
    //! When already connected (isConnected = true) no actions are performed.
    virtual void connectToHost(const QString& server, qint16 port);

    //! Disconnects and lets the internal receive buffer untouched.
    virtual void disconnectFromHost(bool) { m_tcp_server->close(); }

    //! Returns true if connected and ready, false otherwise.
    virtual bool isConnected() const { return m_tcp_server->isListening(); }

    //! sends the given data, returns the true on success, false otherwise.
    virtual bool sendData(qint16 data_type, const QByteArray& data);

    //! returns the number of connected clients
    int clientCount() const { return m_client_buffer_hash.count(); }

signals:

    void signalClientConnected();

    void signalSendData(qint16 data_type, const QByteArray& data);

protected slots:

    //! called for connecing clients
    void slotIncomingConnection();

    //! called for disconnting clients
    void slotClientDisconnected(QTcpSocket* socket);

protected:

    QTcpServer* m_tcp_server;

    // hash pointing from client sockets to client buffer objects.
    QHash<QTcpSocket*, TransportLayerTCPServerClientBuffer*> m_client_buffer_hash;

private:
    //! Hidden copy-constructor
    TransportLayerTCPServer(const TransportLayerTCPServer&);
    //! Hidden assignment operator
    const TransportLayerTCPServer& operator = (const TransportLayerTCPServer&);
};

#endif /* __TRANSPORT_LAYER_TCPSERVER_H__ */

// End of file

