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

#include "fsuipc.h"

static const int FSUIPC_BUFFER_SIZE=30000;

/////////////////////////////////////////////////////////////////////////////

FSUIPC::FSUIPC()
{
  m_buffer=0;

  FSUIPC_Close();
#if VASFMC_GAUGE
  // Can't open link yet because that would cause a deadlock
  // (FSUIPC would try to send a message to the main thread, but that thread
  // is still within the window procedure)
  m_link_ok = false;
#else
  m_link_ok = openLink();
#endif

//   m_debug_write_counter = 0;
//   m_debug_timer.start();
};

/////////////////////////////////////////////////////////////////////////////
  
FSUIPC::~FSUIPC()
{
    FSUIPC_Close();

    delete [] m_buffer;
};

/////////////////////////////////////////////////////////////////////////////

bool FSUIPC::openLink()
{
    closeLink();

    DWORD error = 0;
#if VASFMC_GAUGE
    delete [] m_buffer;
    m_buffer=new BYTE[FSUIPC_BUFFER_SIZE];
    int ret = FSUIPC_Open2(SIM_ANY, &error, m_buffer, FSUIPC_BUFFER_SIZE);
#else
    int ret = FSUIPC_Open(SIM_ANY, &error);
#endif
    if (!ret || error != FSUIPC_ERR_OK) 
    {
//         printf("FSUIPC:openLink: Error: %s\n", getErrorText(error).toLatin1().data());
//         fflush(stdout);
        return false;
    }

    m_link_ok = true;
    return true;
};

/////////////////////////////////////////////////////////////////////////////
  
void FSUIPC::closeLink()
{
    FSUIPC_Close();
    m_link_ok = false;
};


/////////////////////////////////////////////////////////////////////////////
  
QString FSUIPC::getErrorText(int err_nr)
{
    switch(err_nr) 
    {
        case(FSUIPC_ERR_OK): return QString("No error");
        case(FSUIPC_ERR_OPEN): return QString("Attempt to Open when already Open");
        case(FSUIPC_ERR_NOFS): return QString("Cannot link to FSUIPC or WideClient");
        case(FSUIPC_ERR_REGMSG): return QString("Failed to Register common message with Windows");
        case(FSUIPC_ERR_ATOM): return QString("Failed to create Atom for mapping filename");
        case(FSUIPC_ERR_MAP): return QString("Failed to create a file mapping object");
        case(FSUIPC_ERR_VIEW): return QString("Failed to open a view to the file map");
        case(FSUIPC_ERR_VERSION): return QString("Incorrect version of FSUIPC, or not FSUIPC");
        case(FSUIPC_ERR_WRONGFS): return QString("Sim is not version requested");
        case(FSUIPC_ERR_NOTOPEN): return QString("Call cannot execute, link not Open");
        case(FSUIPC_ERR_NODATA): return QString("Call cannot execute: no requests accumulated");	
        case(FSUIPC_ERR_TIMEOUT): return QString("IPC timed out all retries");
        case(FSUIPC_ERR_SENDMSG): return QString("IPC sendmessage failed all retries");
        case(FSUIPC_ERR_DATA): return QString(" IPC request contains bad data");	
        case(FSUIPC_ERR_RUNNING): return QString("Maybe running on WideClient, "
                                                 "but FS not running on Server, or wrong FSUIPC");
        case(FSUIPC_ERR_SIZE): return QString("Read or Write request cannot be added, "
                                              "memory for Process is full");
        default: return QString("Unknown error code");
    }
}

/////////////////////////////////////////////////////////////////////////////

bool FSUIPC::write(DWORD offset, DWORD size, void* data)
{
    if (!m_link_ok) m_link_ok = openLink();
    if (!m_link_ok) return false;
    DWORD error = 0;
    bool ret = FSUIPC_Write(offset, size, data, &error);
    if (!ret) 
    {
        Logger::log(QString("FSUIPC:write: Error: %1").arg(getErrorText(error)));
        fflush(stdout);
        closeLink();
    }
    return ret;
}

// eof
