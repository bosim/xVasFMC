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

#include <QApplication>

#include <signal.h>

#include "defines.h"
#include "assert.h"
#include "config.h"
#include "logger.h"

#include "fmc_console.h"

/////////////////////////////////////////////////////////////////////////////

// qt message logger
void myMessageOutput(QtMsgType type, const char *msg)
{
    switch (type) {
        case QtDebugMsg:
            Logger::log(QString("QtDebug: %1").arg(msg));
            break;
        case QtWarningMsg:
            Logger::log(QString("QtWarning: %1").arg(msg));
            break;
        case QtCriticalMsg:
            Logger::log(QString("QtCritical: %1").arg(msg));
            break;
        case QtFatalMsg:
            Logger::log(QString("QtFatal: %1").arg(msg));
            break;
        default:
            Logger::log(QString("Qtunknown: %1").arg(msg));
            break;             
    }
}

/////////////////////////////////////////////////////////////////////////////

void signalHandler(int signal_nr)
{
    Logger::log(QString("signalHandler: (%1) -----  HANDLER -----").arg(signal_nr));
    qApp->quit();
}

/////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
    Logger::getLogger()->setLogFile(CFG_LOGFILE_NAME);
    Logger::log("     ----- Startup -----");
    qInstallMsgHandler(myMessageOutput);
    QApplication app(argc, argv);

    // setup console
    FMCConsole* console = new FMCConsole(0, 0);
    MYASSERT(console != 0);
    
    //  signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGILL, signalHandler);
    signal(SIGFPE, signalHandler);
    //signal(SIGSEGV, signalHandler);
    signal(SIGTERM, signalHandler);
#ifdef Q_OS_WIN32
    signal(SIGBREAK, signalHandler);
#endif
    signal(SIGABRT, signalHandler);

    // start the application
    Logger::log("     ----- Starting processing loop -----");
    app.exec();

    // clean up
    Logger::log("     ----- Shutting down -----");
    delete console;

    // store config 
    Logger::log("     ----- Shutdown finished -----");
    Logger::finish();
    return 0;
}
