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

/*! \file    cpflight_serial.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __CPFLIGHT_SERIAL_H__
#define __CPFLIGHT_SERIAL_H__

#include <QObject>
#include <QTime>

class Config;
class QextSerialPort;
class FlightStatus;
class FMCControl;

/////////////////////////////////////////////////////////////////////////////

//! CPFlight serial
class CPFlightSerial : public QObject
{
    Q_OBJECT

public:

    //! Standard Constructor
    CPFlightSerial(const QString& config_filename, 
                   FlightStatus* flightstatus,
                   FMCControl* fmc_control);

    //! Destructor
    virtual ~CPFlightSerial();

public slots:

    void slotWriteValues(bool force_write = false);

protected slots:

    void slotReceive();

protected:

    void setupDefaultConfig();

    void decode(const QString& code);

    void writeToSerial(const QString& text);

protected:

    //! config
    Config* m_cfg;

    //! flight status holder
    FlightStatus* m_flightstatus;

    //! FMC control
    FMCControl* m_fmc_control;

    //! CPflight serial port
#ifdef SERIAL
    QextSerialPort* m_serial_port;
#endif

    //----- read stuff

    //! stores the received bytes
    QString m_received_text;

    int m_last_vor1;
    int m_last_vor2;
    bool m_mcp_athr_on;

    //----- write stuff

    bool m_mcp_ready;
    QTime m_write_timer;
    QTime m_write_inhibit_timer;
    bool m_mcp_is_on;
    bool m_write_athr_discrepancy;

private:
    //! Hidden copy-constructor
    CPFlightSerial(const CPFlightSerial&);
    //! Hidden assignment operator
    const CPFlightSerial& operator = (const CPFlightSerial&);
};

#endif /* __CPFLIGHT_SERIAL_H__ */

// End of file

