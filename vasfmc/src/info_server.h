///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2012 Bo Simonsen
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

/*! \file    info_server.h
    \author  Bo Simonsen, bo@geekworld.dk
*/

#ifndef __INFO_SERVER_H__
#define __INFO_SERVER_H__


#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

#include <fstream>

#include "assert.h"
#include "logger.h"

/////////////////////////////////////////////////////////////////////////////

#define CFG_TCP_PORT "50018"

/////////////////////////////////////////////////////////////////////////////

using namespace std;

class InfoServer : public QObject
{
    Q_OBJECT

    public:
        InfoServer() {
            // setup config
            m_tcp_server = new QTcpServer;
            MYASSERT(m_tcp_server != 0);
            MYASSERT(connect(m_tcp_server, SIGNAL(newConnection()), this, SLOT(slotNewConnection())));

            if (!m_tcp_server->listen(QHostAddress::Any, 50019))
                Logger::log(QString("InfoServer: could not open TCP server on port 50018"));
            else
                Logger::log(QString("InfoServer: TCP server listening on port 50018"));
        }
        virtual ~InfoServer() {
            delete m_tcp_server;
            Logger::log("~InfoServer: finished");

        }
     protected slots:
        void slotNewConnection() {
            QTcpSocket *client_connection = m_tcp_server->nextPendingConnection();
            Logger::log(QString("InfoServer:slotNewConnection: client connected: %1:%2").
                        arg(client_connection->peerAddress().toString()).
                        arg(client_connection->peerPort()));

            if(client_connection->waitForReadyRead()) {
                QByteArray data_in = client_connection->readLine(256);
                QString s = QString(data_in);

                Logger::log(QString("InfoServer:slotNewConnection: got ") + s);
                if(s.startsWith("GET /persistence.dat")) {
                    Logger::log(QString("InfoServer:slotNewConnection: sending file"));
                    char buf[256];
                    fstream fh("C:\\Program Files (x86)\\vasfmc-2.0a9\\persistence.dat", ios::in);

                    while(!fh.eof()) {
                        fh.getline(buf, 256);
                        client_connection->write(buf);
                        client_connection->write("\r\n");
                    }
                }
                client_connection->flush();
                delete client_connection;
            }
        }

    private:
        QTcpServer* m_tcp_server;

        //! Hidden copy-constructor
        InfoServer(const InfoServer&);
        //! Hidden assignment operator
        const InfoServer& operator = (const InfoServer&);
};

#endif

// End of file
