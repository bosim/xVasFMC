///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2008 Alexander Wemmer 
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

/*! \file    iocp.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include <QTcpServer>
#include <QTcpSocket>

#include "config.h"
#include "flightstatus.h"
#include "fmc_control.h"

#include "iocp.h"

/////////////////////////////////////////////////////////////////////////////

#define CFG_TCP_PORT "tcp_port"

/////////////////////////////////////////////////////////////////////////////

IOCPServer::IOCPServer(const QString& config_filename, 
                       FMCControl* fmc_control) :
    m_fmc_control(fmc_control)
{
    // setup config

    m_cfg = new Config(config_filename);
    MYASSERT(m_cfg != 0);
    setupDefaultConfig();
    m_cfg->loadfromFile();
    m_cfg->saveToFile();

    m_tcp_server = new QTcpServer;
    MYASSERT(m_tcp_server != 0);
    MYASSERT(connect(m_tcp_server, SIGNAL(newConnection()), this, SLOT(slotNewConnection())));

    if (!m_tcp_server->listen(QHostAddress::Any, m_cfg->getIntValue(CFG_TCP_PORT)))
        Logger::log(QString("IOCPServer: could not open TCP server on port %1").arg(m_cfg->getIntValue(CFG_TCP_PORT)));
    else
        Logger::log(QString("IOCPServer: TCP server listening on port %1").arg(m_cfg->getIntValue(CFG_TCP_PORT)));
}

/////////////////////////////////////////////////////////////////////////////

IOCPServer::~IOCPServer()
{
    while(m_client_socket_list.count() > 0)
    {
        QTcpSocket *client_connection = m_client_socket_list[0];
        MYASSERT(disconnect(client_connection, SIGNAL(disconnected()), this, SLOT(slotGotClientData())));
        MYASSERT(disconnect(client_connection, SIGNAL(readyRead()), this, SLOT(slotGotClientData())));
        client_connection->write(QString("Arn.Fin:CL").toAscii());
        m_client_socket_list.removeAt(0);
        delete client_connection;
        client_connection = 0;
    }

    delete m_tcp_server;

    m_cfg->saveToFile();
    delete m_cfg;

    Logger::log("~IOCPServer: finished");
}

/////////////////////////////////////////////////////////////////////////////

void IOCPServer::setupDefaultConfig()
{
    m_cfg->setValue(CFG_TCP_PORT, "8092");

    m_cfg->setValue("5_17", "CDU_LEFT:67");
    m_cfg->setValue("5_19", "CDU_LEFT:69");
    m_cfg->setValue("5_20", "CDU_LEFT:68");
    m_cfg->setValue("5_21", "CDU_LEFT:81");
    m_cfg->setValue("5_22", "CDU_LEFT:80");
    m_cfg->setValue("5_24", "CDU_LEFT:82");
    m_cfg->setValue("5_25", "CDU_LEFT:61");
    m_cfg->setValue("5_27", "CDU_LEFT:63");
    m_cfg->setValue("5_28", "CDU_LEFT:62");
    m_cfg->setValue("5_29", "CDU_LEFT:65");
    m_cfg->setValue("5_3", "CDU_LEFT:52");
    m_cfg->setValue("5_30", "CDU_LEFT:64");
    m_cfg->setValue("5_32", "CDU_LEFT:66");
    m_cfg->setValue("5_33", "CDU_LEFT:57");
    m_cfg->setValue("5_35", "CDU_LEFT:1");
    m_cfg->setValue("5_36", "CDU_LEFT:59");
    m_cfg->setValue("5_37", "CDU_LEFT:3");
    m_cfg->setValue("5_38", "CDU_LEFT:2");
    m_cfg->setValue("5_39", "CDU_LEFT:5");
    m_cfg->setValue("5_4", "CDU_LEFT:51");
    m_cfg->setValue("5_40", "CDU_LEFT:4");
    m_cfg->setValue("5_41", "CDU_LEFT:83");
    m_cfg->setValue("5_42", "CDU_LEFT:50");
    m_cfg->setValue("5_43", "CDU_LEFT:49");
    m_cfg->setValue("5_44", "CDU_LEFT:84");
    m_cfg->setValue("5_45", "CDU_LEFT:47");
    m_cfg->setValue("5_46", "CDU_LEFT:48");
    m_cfg->setValue("5_47", "CDU_LEFT:45");
    m_cfg->setValue("5_48", "CDU_LEFT:46");
    m_cfg->setValue("5_49", "CDU_LEFT:28");
    m_cfg->setValue("5_5", "CDU_LEFT:54");
    m_cfg->setValue("5_50", "CDU_LEFT:27");
    m_cfg->setValue("5_51", "CDU_LEFT:11");
    m_cfg->setValue("5_52", "CDU_LEFT:29");
    m_cfg->setValue("5_53", "CDU_LEFT:13");
    m_cfg->setValue("5_54", "CDU_LEFT:12");
    m_cfg->setValue("5_55", "CDU_LEFT:15");
    m_cfg->setValue("5_56", "CDU_LEFT:14");
    m_cfg->setValue("5_57", "CDU_LEFT:58");
    m_cfg->setValue("5_59", "CDU_LEFT:6");
    m_cfg->setValue("5_6", "CDU_LEFT:53");
    m_cfg->setValue("5_60", "CDU_LEFT:60");
    m_cfg->setValue("5_61", "CDU_LEFT:8");
    m_cfg->setValue("5_62", "CDU_LEFT:7");
    m_cfg->setValue("5_63", "CDU_LEFT:10");
    m_cfg->setValue("5_64", "CDU_LEFT:9");
    m_cfg->setValue("5_65", "CDU_LEFT:34");
    m_cfg->setValue("5_66", "CDU_LEFT:33");
    m_cfg->setValue("5_67", "CDU_LEFT:21");
    m_cfg->setValue("5_68", "CDU_LEFT:35");
    m_cfg->setValue("5_69", "CDU_LEFT:23");
    m_cfg->setValue("5_7", "CDU_LEFT:56");
    m_cfg->setValue("5_70", "CDU_LEFT:22");
    m_cfg->setValue("5_71", "CDU_LEFT:25");
    m_cfg->setValue("5_72", "CDU_LEFT:24");
    m_cfg->setValue("5_73", "CDU_LEFT:31");
    m_cfg->setValue("5_74", "CDU_LEFT:30");
    m_cfg->setValue("5_75", "CDU_LEFT:16");
    m_cfg->setValue("5_76", "CDU_LEFT:32");
    m_cfg->setValue("5_77", "CDU_LEFT:18");
    m_cfg->setValue("5_78", "CDU_LEFT:17");
    m_cfg->setValue("5_79", "CDU_LEFT:20");
    m_cfg->setValue("5_8", "CDU_LEFT:55");
    m_cfg->setValue("5_80", "CDU_LEFT:19");
    m_cfg->setValue("5_81", "CDU_LEFT:36");
    m_cfg->setValue("5_82", "CDU_LEFT:41");
    m_cfg->setValue("5_83", "CDU_LEFT:26");
    m_cfg->setValue("5_84", "CDU_LEFT:39");
    m_cfg->setValue("5_85", "CDU_LEFT:40");
    m_cfg->setValue("5_86", "CDU_LEFT:42");
    m_cfg->setValue("5_87", "CDU_LEFT:37");
    m_cfg->setValue("5_88", "CDU_LEFT:44");
}

/////////////////////////////////////////////////////////////////////////////

void IOCPServer::slotNewConnection()
{
    QTcpSocket *client_connection = m_tcp_server->nextPendingConnection();
    m_client_socket_list.append(client_connection);
    MYASSERT(connect(client_connection, SIGNAL(disconnected()), this, SLOT(slotGotClientData())));
    MYASSERT(connect(client_connection, SIGNAL(readyRead()), this, SLOT(slotGotClientData())));

    Logger::log(QString("IOCPServer:slotNewConnection: client connected: %1:%2").
                arg(client_connection->peerAddress().toString()).
                arg(client_connection->peerPort()));

    client_connection->write(QString("Arn.TipoSer:vasFMC:CL\r\n").toAscii());
}

/////////////////////////////////////////////////////////////////////////////

void IOCPServer::slotGotClientData()
{
    for(int index=0; index < m_client_socket_list.count();)
    {
        QTcpSocket *client_connection = m_client_socket_list[index];

        if (client_connection->state() != QAbstractSocket::ConnectedState)
        {
            Logger::log("IOCPServer:slotGotClientData: connection closed");
            m_client_socket_list.removeAt(index);
            delete client_connection;
            client_connection = 0;
            continue;
        }    

        if (client_connection->bytesAvailable())
        {
            QByteArray data_in = client_connection->readAll();

            QStringList item_list = QString(data_in).split("\r\n");
            for(int item_index = 0; item_index < item_list.count(); ++item_index)
            {
                const QString& item = item_list[item_index].trimmed();
                if (item.isEmpty()) continue;
                
                //Logger::log(QString("IOCPServer:slotGotClientData: item (%1)").arg(item));
                if (!item.startsWith("Arn.Resp:")) continue;

                QStringList pair_list = item.mid(9).split(":", QString::SkipEmptyParts);

                for(int pair_index=0; pair_index < pair_list.count(); ++pair_index)
                {
                    QString key = pair_list[pair_index].section('=', 0, 0);
                    QString value = pair_list[pair_index].section('=', 1);
                    //Logger::log(QString("IOCPServer:slotGotClientData: item data (%1=%2)").arg(key).arg(value));
                    decode(key, value);
                }
            }

        }

        ++index;
    }

}

/////////////////////////////////////////////////////////////////////////////

void IOCPServer::decode(const QString& key, const QString& value)
{
    if (value == 0) return;
    
    //----- handle configured commands

    QString code = key+"_"+value;
    
    if (m_cfg->contains(code))
    {
        QStringList code_list = m_cfg->getValue(code).split(",");

        QStringList::const_iterator iter = code_list.begin();
        for(; iter != code_list.end(); ++iter) m_fmc_control->processExternalCMD(*iter);
    }
    else if (value != "0")
    {
        Logger::log(QString("IOCPServer:decode: unhandled data (%1_%2)").arg(key).arg(value));
    }
}

// End of file
