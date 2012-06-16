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

/*! \file    transport_layer_tcpserver_clientbuffer.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __TRANSPORT_LAYER_TCPSERVER_CLIENTBUFFER_H__
#define __TRANSPORT_LAYER_TCPSERVER_CLIENTBUFFER_H__

#include <QByteArray>
#include <QTcpSocket>
#include <QObject>

/////////////////////////////////////////////////////////////////////////////

class TransportLayerTCPServerClientBuffer : public QObject
{
    Q_OBJECT

public:

    TransportLayerTCPServerClientBuffer(QObject* parent, QTcpSocket* socket);

    virtual ~TransportLayerTCPServerClientBuffer();

signals:

    void signalDataReceived(qint16 data_type, QByteArray& data);
    void signalClientDisconnected(QTcpSocket* socket);
    
public slots:

    void slotDataReceived();
    void slotDisconnected();
    void slotSendData(qint16 data_type, const QByteArray& data);

protected:

    QTcpSocket* m_tcp_socket;
    QByteArray m_receive_buffer;
};

/////////////////////////////////////////////////////////////////////////////

#endif /* __TRANSPORT_LAYER_TCPSERVER_CLIENTBUFFER_H__ */

// End of file

