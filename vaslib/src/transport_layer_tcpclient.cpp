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

/*! \file    transport_layer_tcpclient.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "transport_layer_tcpclient.h"

/////////////////////////////////////////////////////////////////////////////

TransportLayerTCPClient::TransportLayerTCPClient(bool self_reconnect) :
    m_tcp_socket(0), m_port(0), m_self_reconnect(self_reconnect), m_forced_disconnect(false)
{
    m_tcp_socket = new QTcpSocket(this);
    MYASSERT(m_tcp_socket);
    MYASSERT(connect(m_tcp_socket, SIGNAL(connected()), this, SLOT(slotConnected())));
    MYASSERT(connect(m_tcp_socket, SIGNAL(disconnected()), this, SLOT(slotDisconnected())));
    MYASSERT(connect(m_tcp_socket, SIGNAL(readyRead()), this, SLOT(slotDataReceived())));
    MYASSERT(connect(m_tcp_socket, SIGNAL(error(QAbstractSocket::SocketError)),
                     this, SLOT(slotError(QAbstractSocket::SocketError))));
    MYASSERT(connect(&m_reconnect_timer, SIGNAL(timeout()), this, SLOT(slotReconnect())));
    m_reconnect_timer.setSingleShot(true);
}

/////////////////////////////////////////////////////////////////////////////

// Note: m_tcp_socket will be deleted automatically
TransportLayerTCPClient::~TransportLayerTCPClient()
{
    m_reconnect_timer.stop();
}

/////////////////////////////////////////////////////////////////////////////

void TransportLayerTCPClient::connectToHost(const QString& host, qint16 port)
{
    MYASSERT(!host.isEmpty());
    MYASSERT(port > 0);

    m_host = host;
    m_port = port;
   
    if (isConnected()) return;

    qDebug("TransportLayerTCPClient:connectToHost: (%s:%d):", host.toLatin1().data(), port);

    m_forced_disconnect = false;
    m_receive_buffer.clear();

    m_tcp_socket->connectToHost(m_host, m_port);
}

/////////////////////////////////////////////////////////////////////////////

void TransportLayerTCPClient::disconnectFromHost(bool forced) 
{
    // qDebug("TransportLayerTCPClient:disconnectFromHost: forced=%d", forced);

    m_tcp_socket->disconnectFromHost();
    m_forced_disconnect = forced;
    
    if (m_forced_disconnect)
    {
        m_host = QString::null;
        m_port = 0;
    }

    if (m_self_reconnect && !m_forced_disconnect && !m_reconnect_timer.isActive())
        m_reconnect_timer.start(3000);
}

/////////////////////////////////////////////////////////////////////////////

void TransportLayerTCPClient::slotConnected()
{
    //qDebug("TransportLayerTCPClient:slotConnected");
    emit signalConnected();
    m_forced_disconnect = false;
}

/////////////////////////////////////////////////////////////////////////////

void TransportLayerTCPClient::slotDisconnected()
{
    //qDebug("TransportLayerTCPClient:slotDisconnected");
    emit signalDisconnected();
    if (m_self_reconnect && !m_forced_disconnect && !m_reconnect_timer.isActive())
        m_reconnect_timer.start(3000);
}

/////////////////////////////////////////////////////////////////////////////

void TransportLayerTCPClient::slotReconnect()
{
    //qDebug("TransportLayerTCPClient:slotReconnect");
    if (!m_host.isEmpty() && m_port > 0)
        m_tcp_socket->connectToHost(m_host, m_port);
}

/////////////////////////////////////////////////////////////////////////////
    
void TransportLayerTCPClient::slotError(QAbstractSocket::SocketError error)
{
    if (error != 0) qDebug("TransportLayerTCPClient:slotError: %d", error);
    emit signalError(error, TransportLayerIface::getSocketErrorText(error));
    disconnectFromHost(false);
}

/////////////////////////////////////////////////////////////////////////////

void TransportLayerTCPClient::slotDataReceived()
{
    m_receive_buffer.append(m_tcp_socket->readAll());
    qint16 data_type = 0;
    QByteArray deframed_data;
    while (deframeReceivedData(m_receive_buffer, data_type, deframed_data))
    {
        emit signalDataReceived(data_type, deframed_data);
    
//         qDebug("TransportLayerTCPClient:slotDataReceived:"
//                "reveive buffer size: %d", m_receive_buffer.count());
    }
}

/////////////////////////////////////////////////////////////////////////////

bool TransportLayerTCPClient::sendData(qint16 data_type, const QByteArray& data)
{
    if (!isConnected()) return false;

    QByteArray framed_data;
    enframeData(data_type, data, framed_data);
    qint64 bytes_written = m_tcp_socket->write(framed_data);
                                      
    if (bytes_written < 0 || bytes_written != framed_data.count())
    {
        qCritical("TransportLayerTCPClient:sendData: "
                  "Could not write data to server %s:%d",
                  m_tcp_socket->peerName().toLatin1().data(),
                  m_tcp_socket->peerPort());
        
        disconnectFromHost(false);
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////

// End of file
