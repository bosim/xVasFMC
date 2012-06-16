///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2007 Alexander Wemmer 
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

/*! \file    logger.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

//#include <execinfo.h>
// #include <stdio.h>
// #include <stdlib.h>

#include "logger.h"

Logger* Logger::m_logger = 0;

/////////////////////////////////////////////////////////////////////////////

Logger::Logger()
{
    m_logfile = 0;
    m_logfilestream = 0;
}

/////////////////////////////////////////////////////////////////////////////

Logger::~Logger() 
{
    delete m_logfilestream;
    m_logfilestream = 0;
    m_logfile->flush();
    delete m_logfile;
    m_logfile = 0;
}

/////////////////////////////////////////////////////////////////////////////

void Logger::setLogFile(const QString& logfilename)
{
    MYASSERT(!logfilename.isEmpty());
    if (m_logfile != 0) delete m_logfile;
    if (m_logfilestream != 0) delete m_logfilestream;
    
    m_logfile = new QFile(logfilename);
    MYASSERT(m_logfile != 0);
    if (!m_logfile->open(QIODevice::WriteOnly))
    {
#if! VASFMC_GAUGE
        QMessageBox::critical(0, "LOGFILE", QString("Could not open logfile (%1)").arg(logfilename));
        MYASSERT(false);
#endif
    }
    MYASSERT(m_logfile->isWritable());
    MYASSERT(m_logfile->resize(0));
    
    m_logfilestream = new QTextStream(m_logfile);
    MYASSERT(m_logfilestream != 0);
}

/////////////////////////////////////////////////////////////////////////////

void Logger::logText(const QString& text)
{
    QString logtext = QString("%1: %2").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss:zzz")).arg(text);
    
    // write out the text
    printf("%s\n", logtext.toLatin1().data());
    fflush(stdout);
    
    // write the text to the logfile
    if (m_logfilestream != 0) *m_logfilestream << logtext << endl;
    
    emit signalLogging(logtext);
}

/////////////////////////////////////////////////////////////////////////////

void Logger::logTextToFileOnly(const QString& text)
{
    QString logtext = QString("%1: %2").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss:zzz")).arg(text);
    
    // write the text to the logfile
    if (m_logfilestream != 0) *m_logfilestream << logtext << endl;
}

// End of file
