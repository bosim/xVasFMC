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

/*! \file    iocp.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __IOCP_H__
#define __IOCP_H__

#include <QObject>

#include "assert.h"
#include "logger.h"

class Config;
class FMCControl;
class QTcpServer;
class QTcpSocket;

/////////////////////////////////////////////////////////////////////////////

//! IOCP server
class IOCPServer : public QObject
{
    Q_OBJECT

public:

    //! Standard Constructor
    IOCPServer(const QString& config_filename,
               FMCControl* fmc_control);

    //! Destructor
    virtual ~IOCPServer();

protected slots:

    void slotNewConnection();

    void slotGotClientData();

protected:

    void setupDefaultConfig();

    void decode(const QString& key, const QString& value);

protected:

    //! config
    Config* m_cfg;

    //! FMC control
    FMCControl* m_fmc_control;

    QTcpServer* m_tcp_server;
    QList<QTcpSocket*> m_client_socket_list;

private:

    //! Hidden copy-constructor
    IOCPServer(const IOCPServer&);
    //! Hidden assignment operator
    const IOCPServer& operator = (const IOCPServer&);
};

#endif /* __IOCP_H__ */

// End of file

