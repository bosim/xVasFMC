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

/*! \file    weather.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include <QFtp>
#include <QRegExp>

#include "assert.h"
#include "logger.h"
#include "config.h"

#include "weather.h"

/////////////////////////////////////////////////////////////////////////////

#define CFG_WEATHER_SERVER "weather_server"
#define CFG_METAR_DIR "metar_dir"
#define CFG_STAF_DIR "staf_dir"
#define CFG_TAF_DIR "taf_dir"
#define CFG_FILE_EXTENSION "file_extension"
#define CFG_RESULT_REGEXP "result_regexp"

/////////////////////////////////////////////////////////////////////////////

Weather::Weather(const QString& weather_config_filename) : m_ftp(0)
{
    // init weather config
    m_weather_config = new Config(weather_config_filename);
    MYASSERT(m_weather_config);
    setupDefaultConfig();
    m_weather_config->loadfromFile();
    m_weather_config->saveToFile();
}

/////////////////////////////////////////////////////////////////////////////

Weather::~Weather()
{
    delete m_ftp;
    m_weather_config->saveToFile();
    delete m_weather_config;
}

/////////////////////////////////////////////////////////////////////////////

void Weather::setupDefaultConfig()
{
    m_weather_config->setValue(CFG_WEATHER_SERVER, "tgftp.nws.noaa.gov");
    m_weather_config->setValue(CFG_METAR_DIR, "/data/observations/metar/stations");
    m_weather_config->setValue(CFG_STAF_DIR, "/data/forecasts/shorttaf/stations");
    m_weather_config->setValue(CFG_TAF_DIR, "/data/forecasts/taf/stations");
    m_weather_config->setValue(CFG_FILE_EXTENSION, ".TXT");
    m_weather_config->setValue(CFG_RESULT_REGEXP, "\\s*([^\\r\\n]*)\\s*[\\r\\n](.*)$");
}

/////////////////////////////////////////////////////////////////////////////

void Weather::setupFtp()
{
    delete m_ftp;
    m_ftp = new QFtp;
    MYASSERT(m_ftp != 0);
    MYASSERT(connect(m_ftp, SIGNAL(commandFinished(int, bool)), this, SLOT(slotCmdFin(int, bool))));
    MYASSERT(connect(m_ftp, SIGNAL(readyRead()), this, SLOT(slotReadyRead())));
}

/////////////////////////////////////////////////////////////////////////////

void Weather::requestMetar(const QString& airport) 
{
    requestWeather(airport, m_weather_config->getValue(CFG_METAR_DIR)); 
}

/////////////////////////////////////////////////////////////////////////////

void Weather::requestSTAF(const QString& airport) 
{
    requestWeather(airport, m_weather_config->getValue(CFG_STAF_DIR));
}

/////////////////////////////////////////////////////////////////////////////

void Weather::requestTAF(const QString& airport) 
{
    requestWeather(airport, m_weather_config->getValue(CFG_TAF_DIR));
}

/////////////////////////////////////////////////////////////////////////////

void Weather::requestWeather(const QString airport, const QString& directory)
{
    setupFtp();
    m_airport = airport.trimmed().toUpper();
    m_ftp->connectToHost(m_weather_config->getValue(CFG_WEATHER_SERVER));
    m_ftp->login();
    m_ftp->cd(directory);
    m_ftp->get(m_airport+m_weather_config->getValue(CFG_FILE_EXTENSION), 0, QFtp::Ascii);
}

/////////////////////////////////////////////////////////////////////////////

void Weather::slotCmdFin(int, bool error)
{
    //Logger::log(QString("Weather:slotCmdFin: id=%1 error=%2").arg(id).arg(error));
    
    if (error) 
    {
        emit signalError(m_ftp->errorString());
        return;
    }
}

/////////////////////////////////////////////////////////////////////////////

void Weather::slotReadyRead()
{
    QString raw_metar = m_ftp->readAll();

    QRegExp weather_regexp(m_weather_config->getValue(CFG_RESULT_REGEXP));
    if (!weather_regexp.isValid())
    {
        emit signalError("Result RegExp is invalid");
        return;
    }

    if (weather_regexp.indexIn(raw_metar) < 0)
    {
        emit signalError("RegExp did not match");
        return;
    }

    emit signalGotWeather(m_airport, weather_regexp.cap(1), weather_regexp.cap(2));
    m_airport.clear();
}

/////////////////////////////////////////////////////////////////////////////

// End of file
