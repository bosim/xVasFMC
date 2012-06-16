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

/*! \file    transport_layer_tcpserver.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include <QHostAddress>
#include <QTcpSocket>

#include "transport_layer_tcpserver_clientbuffer.h"
#include "transport_layer_tcpserver.h"

/////////////////////////////////////////////////////////////////////////////

TransportLayerTCPServer::TransportLayerTCPServer()
{
    m_tcp_server = new QTcpServer;
    MYASSERT(m_tcp_server != 0);
    MYASSERT(connect(m_tcp_server, SIGNAL(newConnection()),
                     this, SLOT(slotIncomingConnection())));
}

/////////////////////////////////////////////////////////////////////////////

TransportLayerTCPServer::~TransportLayerTCPServer()
{
    QHashIterator<QTcpSocket*, TransportLayerTCPServerClientBuffer*> iter(m_client_buffer_hash);
    while (iter.hasNext()) {
        iter.next();
        iter.value()->deleteLater();
    }

    m_tcp_server->close();
    m_tcp_server->deleteLater();
}

/////////////////////////////////////////////////////////////////////////////

void TransportLayerTCPServer::connectToHost(const QString& server, qint16 port)
{
    qDebug("TransportLayerTCPServer:connectToHost: (%s:%d)", server.toLatin1().data(), port);

    if (isConnected()) return;

    QHostAddress address(server);
    if (server.isEmpty()) address = QHostAddress::Any;
    if (!m_tcp_server->listen(address, port))
        emit signalDisconnected();
    else
        emit signalConnected();
}

/////////////////////////////////////////////////////////////////////////////

void TransportLayerTCPServer::slotIncomingConnection()
{
    while(m_tcp_server->hasPendingConnections())
    {
        QTcpSocket* client_socket = m_tcp_server->nextPendingConnection();

        TransportLayerTCPServerClientBuffer* client_buffer =
            new TransportLayerTCPServerClientBuffer(this, client_socket);
        MYASSERT(client_buffer);

        MYASSERT(connect(client_buffer, SIGNAL(signalClientDisconnected(QTcpSocket*)),
                         this, SLOT(slotClientDisconnected(QTcpSocket*))));

        MYASSERT(connect(client_buffer, SIGNAL(signalDataReceived(qint16, QByteArray&)),
                         this, SIGNAL(signalDataReceived(qint16, QByteArray&))));

        MYASSERT(connect(this, SIGNAL(signalSendData(qint16, const QByteArray&)),
                         client_buffer, SLOT(slotSendData(qint16, const QByteArray&))));

        m_client_buffer_hash.insert(client_socket, client_buffer);

//         qDebug("TransportLayerTCPServerClientBuffer:slotIncomingConnection: "
//                "added (%p) to hash", client_socket);

        emit signalClientConnected();
    }
}

/////////////////////////////////////////////////////////////////////////////

void TransportLayerTCPServer::slotClientDisconnected(QTcpSocket* socket)
{
    MYASSERT(socket);
    if (!m_client_buffer_hash.contains(socket))
    {
//         qCritical("TransportLayerTCPServer:slotClientDisconnected: "
//                   "hash does not contain (%p)", socket);
        return;
    }
    
    m_client_buffer_hash.take(socket)->deleteLater();
}

/////////////////////////////////////////////////////////////////////////////

bool TransportLayerTCPServer::sendData(qint16 data_type, const QByteArray& data)
{
    emit signalSendData(data_type, data);
    return data.count();
}

/////////////////////////////////////////////////////////////////////////////

// End of file
