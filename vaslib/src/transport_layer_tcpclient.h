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

/*! \file    transport_layer_tcpclient.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __TRANSPORT_LAYER_TCPCLIENT_H__
#define __TRANSPORT_LAYER_TCPCLIENT_H__

#include <QTcpSocket>
#include <QString>
#include <QTimer>

#include "transport_layer_iface.h"

//! tcp client transport layer
/*! more details ...
 */
class TransportLayerTCPClient : public TransportLayerIface
{
    Q_OBJECT

public:
    //! Standard Constructor
    TransportLayerTCPClient(bool self_reconnect = true);

    //! Destructor
    virtual ~TransportLayerTCPClient();

    //! Clears the internal receive buffer and connects to the given host.
    //! When already connected (isConnected = true) no actions are performed.
    virtual void connectToHost(const QString& host, qint16 port);

    //! Disconnects and lets the internal receive buffer untouched.
    virtual void disconnectFromHost(bool forced = true);

    //! Returns true if connected and ready, false otherwise.
    virtual bool isConnected() const { return m_tcp_socket->state() == QAbstractSocket::ConnectedState; }

    //! Sends the given data to the server.
    //! Returns true on success, false on error.
    virtual bool sendData(qint16 data_type, const QByteArray& data);

protected slots:

    void slotConnected();
    void slotDisconnected();
    void slotError(QAbstractSocket::SocketError error);
    void slotDataReceived();

    void slotReconnect();

protected:

    QTcpSocket* m_tcp_socket;

    QString m_host;
    int m_port;

    bool m_self_reconnect;
    bool m_forced_disconnect;

    QTimer m_reconnect_timer;

    QByteArray m_receive_buffer;

private:
    //! Hidden copy-constructor
    TransportLayerTCPClient(const TransportLayerTCPClient&);
    //! Hidden assignment operator
    const TransportLayerTCPClient& operator = (const TransportLayerTCPClient&);
};




#endif /* __TRANSPORT_LAYER_TCPCLIENT_H__ */

// End of file

