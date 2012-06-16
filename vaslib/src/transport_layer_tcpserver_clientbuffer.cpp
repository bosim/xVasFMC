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

/*! \file    transport_layer_tcpserver_clientbuffer.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "transport_layer_iface.h"
#include "transport_layer_tcpserver_clientbuffer.h"

/////////////////////////////////////////////////////////////////////////////

TransportLayerTCPServerClientBuffer::TransportLayerTCPServerClientBuffer(
    QObject* parent, QTcpSocket* socket) :
    QObject(parent), m_tcp_socket(socket)
{
    MYASSERT(m_tcp_socket);
    MYASSERT(connect(m_tcp_socket, SIGNAL(readyRead()), this, SLOT(slotDataReceived())));
    MYASSERT(connect(socket, SIGNAL(disconnected()), this, SLOT(slotDisconnected())));
};

/////////////////////////////////////////////////////////////////////////////

TransportLayerTCPServerClientBuffer::~TransportLayerTCPServerClientBuffer() 
{
    delete m_tcp_socket;
};

/////////////////////////////////////////////////////////////////////////////

void TransportLayerTCPServerClientBuffer::slotDataReceived()
{
    m_receive_buffer.append(m_tcp_socket->readAll());
    qint16 data_type = 0;
    QByteArray deframed_data;
    while (TransportLayerIface::deframeReceivedData(m_receive_buffer, data_type, deframed_data)) 
    {
        emit signalDataReceived(data_type, deframed_data);

//         qDebug("TransportLayerTCPServerClientBuffer:slotDataReceived:"
//                "reveive buffer size: %d", m_receive_buffer.count());
    }
}

/////////////////////////////////////////////////////////////////////////////

void TransportLayerTCPServerClientBuffer::slotDisconnected() 
{
    emit signalClientDisconnected(m_tcp_socket); 
}

/////////////////////////////////////////////////////////////////////////////

void TransportLayerTCPServerClientBuffer::slotSendData(qint16 data_type, const QByteArray& data)
{
    QByteArray framed_data;
    TransportLayerIface::enframeData(data_type, data, framed_data);
    qint64 bytes_written = m_tcp_socket->write(framed_data);

    if (bytes_written < 0 || bytes_written != framed_data.count())
    {
        qCritical("TransportLayerTCPServerClientBuffer:slotSendData: "
                  "Could not write data to client %s:%d",
                  m_tcp_socket->peerName().toLatin1().data(),
                  m_tcp_socket->peerPort());

        emit signalClientDisconnected(m_tcp_socket);
    }
}

// End of file
