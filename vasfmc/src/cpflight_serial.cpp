//////////////////////////////////////////////////////////////////////////////
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

/*! \file    cpflight_serial.cpp
  \author  Alexander Wemmer, alex@wemmer.at
*/

#include <QCoreApplication>

#ifdef SERIAL
#include <qextserialport.h>
#endif

#include "assert.h"
#include "logger.h"
#include "config.h"
#include "flightstatus.h"
#include "fmc_control.h"
#include "fmc_autopilot.h"
#include "fmc_autothrottle.h"

#include "cpflight_serial.h"

/////////////////////////////////////////////////////////////////////////////

#define CFG_PORT "port"
#define CFG_VOR1 "vor1"
#define CFG_VOR2 "vor2"

#define VOR_MODE_VOR_LEFT "vor_left"
#define VOR_MODE_VOR_RIGHT "vor_right"
#define VOR_MODE_QNH_LEFT "qnh_left"
#define VOR_MODE_QNH_RIGHT "qnh_right"
#define VOR_MODE_MAP_MODE_LEFT "mapmode_left"
#define VOR_MODE_MAP_MODE_RIGHT "mapmode_right"
#define VOR_MODE_MAP_RANGE_LEFT "maprange_left"
#define VOR_MODE_MAP_RANGE_RIGHT "maprange_right"

/////////////////////////////////////////////////////////////////////////////

CPFlightSerial::CPFlightSerial(const QString& config_filename, 
                               FlightStatus* flightstatus,
                               FMCControl* fmc_control) :
    m_flightstatus(flightstatus), m_fmc_control(fmc_control), 
    m_last_vor1(-1), m_last_vor2(-1), m_mcp_athr_on(false),
    m_mcp_ready(false), m_mcp_is_on(false), m_write_athr_discrepancy(true)
{
    MYASSERT(m_flightstatus != 0);
    MYASSERT(m_fmc_control != 0);

    // setup config

    m_cfg = new Config(config_filename);
    MYASSERT(m_cfg != 0);
    setupDefaultConfig();
    m_cfg->loadfromFile();
    m_cfg->saveToFile();

    // setup serial connection

#ifdef SERIAL
    m_serial_port = new QextSerialPort(m_cfg->getValue(CFG_PORT), QextSerialPort::EventDriven);
    MYASSERT(m_serial_port != 0);

    m_serial_port->setFlowControl(FLOW_OFF);
    m_serial_port->setParity(PAR_NONE);    
    m_serial_port->setDataBits(DATA_8);   
    m_serial_port->setStopBits(STOP_1);
    m_serial_port->setBaudRate(BAUD38400);

    Logger::log(QString("CPFlightSerial: opening serial port (%1)").arg(m_cfg->getValue(CFG_PORT)));
    m_serial_port->open(QIODevice::ReadWrite);
    if (!m_serial_port->isOpen())
    {
        Logger::log("CPFlightSerial: Could not open serial port - aborting!");
        return;
    }

    m_serial_port->setFlowControl(FLOW_OFF);
    m_serial_port->setParity(PAR_NONE);    
    m_serial_port->setDataBits(DATA_8);   
    m_serial_port->setStopBits(STOP_1);
    m_serial_port->setBaudRate(BAUD38400);
    m_serial_port->setRts(false);
    m_serial_port->setDtr(true);

    MYASSERT(connect(m_serial_port, SIGNAL(readyRead()), this, SLOT(slotReceive())));

    m_write_inhibit_timer.start();
    m_write_timer.start();

    m_serial_port->write("K999\0", 5);

#else
    Logger::log("CPFlightSerial: serial support not compiled in");
#endif
}

/////////////////////////////////////////////////////////////////////////////

CPFlightSerial::~CPFlightSerial()
{
#ifdef SERIAL
    if (m_serial_port->isOpen()) 
    {
        m_serial_port->write("K999\0", 5);
        m_serial_port->close();
    }
    delete m_serial_port;
#endif

    m_cfg->saveToFile();
    delete m_cfg;

    Logger::log("~CPFlightSerial: finished");
}

/////////////////////////////////////////////////////////////////////////////

void CPFlightSerial::setupDefaultConfig()
{
    m_cfg->setValue(CFG_PORT, "COM1");
    m_cfg->setValue(CFG_VOR1, VOR_MODE_VOR_LEFT);
    m_cfg->setValue(CFG_VOR2, VOR_MODE_VOR_RIGHT);

    m_cfg->setValue("K017", "PFD_LEFT:82");
    m_cfg->setValue("K018", "PFD_LEFT:83");
    m_cfg->setValue("K019", "FCU:26");
    m_cfg->setValue("K020", "FCU:34");
    m_cfg->setValue("K021", "FCU:30");
    m_cfg->setValue("K022", "FCU:27");
    m_cfg->setValue("K023", "FCU:31");
    m_cfg->setValue("K024", "FCU:60");
    m_cfg->setValue("K025", "FCU:3");
    m_cfg->setValue("K026", "ND_LEFT:63");
    m_cfg->setValue("K027", "FCU:1");
    m_cfg->setValue("K028", "FCU:7");
    m_cfg->setValue("K029", "FCU:9");
    m_cfg->setValue("K030", "FCU:11");
    m_cfg->setValue("K031", "FCU:16");
    m_cfg->setValue("K032", "FCU:25");
    m_cfg->setValue("K033", "PFD_LEFT:75");
    m_cfg->setValue("K034", "ND_LEFT:65");
    m_cfg->setValue("K035", "ND_LEFT:60");
    m_cfg->setValue("K037", "FCU:20");
    m_cfg->setValue("K038", "FCU:21");
    m_cfg->setValue("K040", "FCU:41");
    m_cfg->setValue("K041", "FCU:40");

}

/////////////////////////////////////////////////////////////////////////////

void CPFlightSerial::decode(const QString& code)
{
	Q_UNUSED (code);
#ifdef SERIAL

	if (code == "KCPF") return;

    //----- remeber MCP A/THR state
    
    if (code == "K019") m_mcp_athr_on = true;
    else if (code == "K020") m_mcp_athr_on = false;

    //----- check for MCP startup

    if (!m_mcp_ready)
    {
        Logger::log(QString("CPFlightSerial:decode: "
                            "MCP not ready, unprocessed (%1)").arg(code));

        if (code.startsWith("G01") || 
            code.startsWith("G02") || 
            code.startsWith("G03"))
        {
            slotWriteValues(true);
            m_mcp_ready = true;
        }
        
        if (!m_mcp_ready) return;
    }

    //----- handle configured commands

    if (m_cfg->contains(code))
    {
        QStringList code_list = m_cfg->getValue(code).split(",");

        bool cmd_handled = false;

        QStringList::const_iterator iter = code_list.begin();
        for(; iter != code_list.end(); ++iter) 
            cmd_handled |= m_fmc_control->processExternalCMD(*iter);

        if (cmd_handled) return;
    }

    //----- handle fixed commands
    
    if (code.startsWith("V02 ")) 
    {
        if (m_fmc_control->fmcAutothrottle().isAPThrottleModeSpeedSet())
            m_fmc_control->fsAccess().setAPAirspeed(code.mid(4).toInt());
        else
            m_fmc_control->fsAccess().setAPMach(code.mid(4).toDouble());
    }
    else if (code.startsWith("V03 ")) m_fmc_control->fsAccess().setAPHeading(code.mid(4).toInt());
    else if (code.startsWith("V04")) m_fmc_control->fsAccess().setAPAlt(code.mid(3).toInt());
    else if (code.startsWith("V05")) m_fmc_control->fmcAutoPilot().setVerticalSpeed(code.mid(3).toInt());

    else if (code.startsWith("V01"))
    {
        int vor1 = code.mid(4).toInt();

        if (m_last_vor1 < 0)
        {
            m_last_vor1 = vor1;
            return;
        }

        int diff = -Navcalc::round(Navcalc::getSignedHeadingDiff(vor1, m_last_vor1));

        if (m_cfg->getValue(CFG_VOR1) == VOR_MODE_VOR_LEFT)
            m_fmc_control->fsAccess().setNavOBS(vor1, 0);
        else if (m_cfg->getValue(CFG_VOR1) == VOR_MODE_VOR_RIGHT)
            m_fmc_control->fsAccess().setNavOBS(vor1, 1);
        else if (m_cfg->getValue(CFG_VOR1) == VOR_MODE_QNH_LEFT)
            m_fmc_control->changeAltimeterSetting(true, LIMIT(diff, 1));
        else if (m_cfg->getValue(CFG_VOR1) == VOR_MODE_QNH_RIGHT)
            m_fmc_control->changeAltimeterSetting(false, LIMIT(diff, 1));
        else if (m_cfg->getValue(CFG_VOR1) == VOR_MODE_MAP_RANGE_LEFT)
            m_fmc_control->nextNDRange(true, diff > 0);
        else if (m_cfg->getValue(CFG_VOR1) == VOR_MODE_MAP_RANGE_RIGHT)
            m_fmc_control->nextNDRange(false, diff > 0);
        else if (m_cfg->getValue(CFG_VOR1) == VOR_MODE_MAP_MODE_LEFT)
            m_fmc_control->nextNDMode(true, diff > 0);
        else if (m_cfg->getValue(CFG_VOR1) == VOR_MODE_MAP_MODE_RIGHT)
            m_fmc_control->nextNDMode(false, diff > 0);
        else
        {
            Logger::log("CPFlightSerial:decode: invalid VOR1 command in config");
        }

        m_last_vor1 = vor1;
    }

    else if (code.startsWith("V06"))
    {
        int vor2 = code.mid(4).toInt();
        
        if (m_last_vor2 < 0)
        {
            m_last_vor2 = vor2;
            return;
        }

        int diff = -Navcalc::round(Navcalc::getSignedHeadingDiff(vor2, m_last_vor2));

        if (m_cfg->getValue(CFG_VOR2) == VOR_MODE_VOR_LEFT)
            m_fmc_control->fsAccess().setNavOBS(vor2, 0);
        else if (m_cfg->getValue(CFG_VOR2) == VOR_MODE_VOR_RIGHT)
            m_fmc_control->fsAccess().setNavOBS(vor2, 1);
        else if (m_cfg->getValue(CFG_VOR2) == VOR_MODE_QNH_LEFT)
            m_fmc_control->changeAltimeterSetting(true, LIMIT(diff, 1));
        else if (m_cfg->getValue(CFG_VOR2) == VOR_MODE_QNH_RIGHT)
            m_fmc_control->changeAltimeterSetting(false, LIMIT(diff, 1));
        else if (m_cfg->getValue(CFG_VOR2) == VOR_MODE_MAP_RANGE_LEFT)
            m_fmc_control->nextNDRange(true, diff > 0);
        else if (m_cfg->getValue(CFG_VOR2) == VOR_MODE_MAP_RANGE_RIGHT)
            m_fmc_control->nextNDRange(false, diff > 0);
        else if (m_cfg->getValue(CFG_VOR2) == VOR_MODE_MAP_MODE_LEFT)
            m_fmc_control->nextNDMode(true, diff > 0);
        else if (m_cfg->getValue(CFG_VOR2) == VOR_MODE_MAP_MODE_RIGHT)
            m_fmc_control->nextNDMode(false, diff > 0);
        else
        {
            Logger::log("CPFlightSerial:decode: invalid VOR2 command in config");
        }

        m_last_vor2 = vor2;
    }

    else Logger::log(QString("CPFlightSerial:decode: unhandled (%1)").arg(code));

#endif
}

/////////////////////////////////////////////////////////////////////////////

void CPFlightSerial::slotReceive()
{
#ifndef SERIAL
    Logger::log("CPFlightSerial:slotReceive: serial support not compiled - aborting");
    return;
#else

	char data[1024];
	int bytes_read = m_serial_port->read(data, 1024);

    if (bytes_read > 0) m_mcp_is_on = true;

    for(int index=0; index < bytes_read; ++index)
    {
        if (data[index] == '\0')
        {
            if (!m_received_text.isEmpty())
            {
                decode(m_received_text);
                m_received_text.clear();
                m_write_inhibit_timer.start();
            }
        }
        else
        {
            m_received_text += data[index];
        }
    }
#endif
}

/////////////////////////////////////////////////////////////////////////////

void CPFlightSerial::writeToSerial(const QString& text)
{
	Q_UNUSED (text);
#ifdef SERIAL
    if (!m_serial_port->isOpen()) return;
    m_serial_port->write(text.toAscii(), text.length());
#endif
}

/////////////////////////////////////////////////////////////////////////////

void CPFlightSerial::slotWriteValues(bool force_write)
{
	Q_UNUSED (force_write);
#ifndef SERIAL
    Logger::log("CPFlightSerial:slotWriteValues: serial support not compiled - aborting");
    return;
#else

    if (!m_serial_port->isOpen()) return;

    if (!force_write &&
        (m_write_inhibit_timer.elapsed() < 1000 || m_flightstatus->slew || m_write_timer.elapsed() < 1000)) return;

    m_write_timer.start();

    if (m_flightstatus->isValid() && !m_flightstatus->battery_on)
    {
        if (m_mcp_is_on) m_serial_port->write("K999\0", 5);
        m_mcp_is_on = false;
        if (!force_write) return;
    }
    else if (!m_mcp_is_on)
    {
        //Logger::log(QString("CPFlightSerial:lineStatus: %1").arg(m_serial_port->lineStatus()));
        Logger::log("CPFlightSerial:slotWriteValues: sending start cmd");
        m_serial_port->write("Q001\0", 5);
        return;
    }

    QString data;
    
    // AP speed
    if (m_fmc_control->fmcAutothrottle().isAPThrottleModeSpeedSet())
        writeToSerial(QString("V02%1\0").arg(m_flightstatus->APSpd(), 3, 10, QChar('0')));
    else
        writeToSerial(QString("V02.%1\0").arg(Navcalc::round(m_flightstatus->APMach()*100.0)));
 
    // AP hdg
    writeToSerial(QString("V03%1\0").arg(m_flightstatus->APHdg(), 3, 10, QChar('0')));

    // AP alt
    writeToSerial(QString("V04%1\0").arg(m_flightstatus->APAlt(), 5, 10, QChar('0')));

    //AP VS
    if (m_flightstatus->ap_vs_lock ||
        (m_flightstatus->ap_alt_lock &&
         qAbs(m_flightstatus->smoothed_altimeter_readout.lastValue() - m_flightstatus->APAlt()) > 50))
    {
        QString vs_string;

        if (!m_fmc_control->fmcAutoPilot().isFLChangeModeActive())
        {
            vs_string = QString("%1").arg(qAbs(m_flightstatus->APVs()), 4, 10, QChar('0'));
            if (m_flightstatus->APVs() >= 0) vs_string = "+" + vs_string;
            else vs_string = "-" + vs_string;
        }
        else
        {
            vs_string = "+0000";
        }

        writeToSerial(QString("V05%1\0").arg(vs_string));
        writeToSerial("X1105\0");
    }
    else
    {
        writeToSerial("X1005\0");
    }

    // A/THR discrepancy

    if (m_fmc_control->fmcAutothrottle().isAPThrottleArmed() != m_mcp_athr_on && m_write_athr_discrepancy)
    {
        writeToSerial("L0097\0");
        m_write_athr_discrepancy = false;
    }
    else
    {
        m_write_athr_discrepancy = true;
    }

    // A/THR lights

    if (m_fmc_control->fmcAutothrottle().isAPThrottleModeN1Engaged())
    {
        writeToSerial("L1122\0");
        writeToSerial("L0121\0");
    }
    else
    {
        writeToSerial("L1121\0");
        if (m_fmc_control->fmcAutothrottle().isAPThrottleEngaged()) writeToSerial("L0122\0");
        else writeToSerial("L1122\0");
    }    

    // AP lights

    //----- lateral

    if (m_fmc_control->fmcAutoPilot().isTakeoffModeActiveLateral())
    {
        writeToSerial("L1127\0"); //LNAV
        writeToSerial("L1125\0"); //HDG
        writeToSerial("L1128\0"); //VORLOC
        writeToSerial("L1129\0"); //APP
    }
    else
    {
        // HDG
        (m_flightstatus->ap_hdg_lock && !m_fmc_control->fmcAutoPilot().isNAVCoupled()) ? 
            writeToSerial("L0125\0") : writeToSerial("L1125\0");
        
        // LNAV
        (m_flightstatus->ap_hdg_lock && m_fmc_control->fmcAutoPilot().isNAVCoupled()) ? 
            writeToSerial("L0127\0") : writeToSerial("L1127\0");
        
        // VOR/LOC
        (m_flightstatus->ap_nav1_lock) ? writeToSerial("L0128\0") : writeToSerial("L1128\0");
        
        // APP
        (m_flightstatus->ap_app_lock) ? writeToSerial("L0129\0") : writeToSerial("L1129\0");
    }

    //----- vertical

    if (m_fmc_control->fmcAutoPilot().isTakeoffModeActiveVertical())
    {
        writeToSerial("L1130\0"); //ALT HLD
        writeToSerial("L1131\0"); //VS
        writeToSerial("L1124\0"); //FLCH
    }
    else
    {
        // ALT/VS , FLCH
        if (m_fmc_control->fmcAutoPilot().isFLChangeModeActive())
        {
            writeToSerial("L1130\0"); //ALT HLD
            writeToSerial("L1131\0"); //VS
            writeToSerial("L0124\0"); //FLCH
        }
        else if (m_flightstatus->ap_alt_lock &&
                 m_fmc_control->fmcAutoPilot().verticalModeActive() == FMCAutopilot::VERTICAL_MODE_ALT_CAPTURE ||
                 m_fmc_control->fmcAutoPilot().verticalModeActive() == FMCAutopilot::VERTICAL_MODE_ALT ||
                 m_fmc_control->fmcAutoPilot().verticalModeActive() == FMCAutopilot::VERTICAL_MODE_ALTCRZ)
        {
            writeToSerial("L1124\0"); //FLCH
            writeToSerial("L1131\0"); //VS
            writeToSerial("L0130\0"); //ALT HLD
        }
        else if (!m_flightstatus->ap_app_lock)
        {
            writeToSerial("L1124\0"); //FLCH
            writeToSerial("L1130\0"); //ALT HLD
            writeToSerial("L0131\0"); //VS
        }
        else
        {
            writeToSerial("L1124\0"); //FLCH
            writeToSerial("L1130\0"); //ALT HLD
            writeToSerial("L1131\0"); //VS
        }
    }
    
    // CMD A
    (m_flightstatus->ap_enabled) ? writeToSerial("L0132\0") : writeToSerial("L1132\0");

#endif
}

// End of file
