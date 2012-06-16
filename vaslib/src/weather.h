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

/*! \file    weather.h
  \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef WEATHER_H
#define WEATHER_H

#include <QObject>
#include <QString>

class QFtp;
class Config;

/////////////////////////////////////////////////////////////////////////////

//! weather data access
class Weather : public QObject
{
    Q_OBJECT

public:

    Weather(const QString& weather_config_filename);
    ~Weather();

    void requestMetar(const QString& airport);
    void requestSTAF(const QString& airport);
    void requestTAF(const QString& airport);

signals:

    void signalGotWeather(const QString& airport, const QString& date_string, const QString& weather_string);
    void signalError(const QString& error_string);

protected slots:

    void slotCmdFin(int id, bool error);
    void slotReadyRead();

protected:

    void setupDefaultConfig();
    void setupFtp();
    void requestWeather(const QString airport, const QString& directory);

protected:

    Config* m_weather_config;
    QFtp* m_ftp;
    QString m_airport;

private:
    //! Hidden copy-constructor
    Weather(const Weather&);
    //! Hidden assignment operator
    const Weather& operator = (const Weather&);
};

#endif /* WEATHER_H */

// End of file

