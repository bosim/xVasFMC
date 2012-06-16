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

#include <QFile>
#include <QStringList>
#include <QTextStream>
#include <QDir>
#include <QMessageBox>

#include "config.h"
#include "logger.h"
#include "vas_path.h"

#define COMMENT "#"

/////////////////////////////////////////////////////////////////////////////

Config::Config(const QString& filename, const QString& separator)
{
    if (filename.length() <= 0) qFatal("Config: filename empty");
    m_filename = VasPath::prependPath(filename);
  
    if (separator.length() <= 0) qFatal("Config: separator empty");
    m_separator = separator;
}

/////////////////////////////////////////////////////////////////////////////

bool Config::contains(const QString& key) const
{
    bool ret = m_data_map.contains(key);
    //if (!ret) Logger::log(QString("Config:contains: does not contain key (%1)").arg(key));
    return ret;
}

/////////////////////////////////////////////////////////////////////////////

QString Config::getValue(const QString& key) const
{
    if (!contains(key)) Logger::log(QString("Config:getValue: key not found: [%1]").arg(key));
    checkKeyAndShowMessageBox(key);
    return m_data_map[key];
}

/////////////////////////////////////////////////////////////////////////////

int Config::getIntValue(const QString& key) const
{
    if (!contains(key)) Logger::log(QString("Config:getIntValue: key not found: [%1]").arg(key));
    checkKeyAndShowMessageBox(key);
    if (m_int_cache_map.contains(key)) return m_int_cache_map[key];
    bool convok = false;
    int value = m_data_map[key].toInt(&convok);
    if (!convok) Logger::log(QString("Config:getIntValue: could not convert to integer[%1]=%2").
                             arg(key).arg(m_data_map[key]));
    MYASSERT(convok);
    m_int_cache_map.insert(key, value);
    return value;
}

/////////////////////////////////////////////////////////////////////////////

double Config::getDoubleValue(const QString& key) const
{
    if (!contains(key)) Logger::log(QString("Config:getDoubleValue: key not found: [%1]").arg(key));
    checkKeyAndShowMessageBox(key);
    if (m_double_cache_map.contains(key)) return m_double_cache_map[key];
	bool convok = false;
    double value = m_data_map[key].toDouble(&convok);
	if (!convok) value = m_data_map[key].toInt(&convok);
    if (!convok) Logger::log(QString("Config:getDoubleValue: could not convert to double [%1]=%2").
                             arg(key).arg(m_data_map[key]));
    MYASSERT(convok);
    m_double_cache_map.insert(key, value);
	return value;
}
    
/////////////////////////////////////////////////////////////////////////////

QColor Config::getColorValue(const QString& key) const
{
    if (!contains(key)) Logger::log(QString("Config:getColorValue: key not found: [%1]").arg(key));
    checkKeyAndShowMessageBox(key);
    if (m_color_cache_map.contains(key)) return m_color_cache_map[key];
    QColor value = QColor(m_data_map[key]);
    if (!value.isValid()) Logger::log(QString("Config:getColorValue: could not convert to color[%1]=%2").
                                      arg(key).arg(m_data_map[key]));
    MYASSERT(value.isValid());
    m_color_cache_map.insert(key, value);
	return value;
}

/////////////////////////////////////////////////////////////////////////////

void Config::setValue(const QString& key, const QString& value)
{
    m_data_map.insert(key, value);
    m_int_cache_map.remove(key);
    m_double_cache_map.remove(key);
    m_color_cache_map.remove(key);
    emit signalChanged();
}
  
/////////////////////////////////////////////////////////////////////////////

void Config::setValue(const QString& key, const int& value)
{
    m_data_map.insert(key, QString::number(value));
    m_int_cache_map.remove(key);
    m_double_cache_map.remove(key);
    m_color_cache_map.remove(key);
    emit signalChanged();
}

/////////////////////////////////////////////////////////////////////////////

void Config::setValue(const QString& key, const uint& value)
{
    m_data_map.insert(key, QString::number(value));
    m_int_cache_map.remove(key);
    m_double_cache_map.remove(key);
    m_color_cache_map.remove(key);
    emit signalChanged();
}

/////////////////////////////////////////////////////////////////////////////

void Config::setValue(const QString& key, const double& value)
{
    m_data_map.insert(key, QString::number(value));
    m_int_cache_map.remove(key);
    m_double_cache_map.remove(key);
    m_color_cache_map.remove(key);
    emit signalChanged();
}

/////////////////////////////////////////////////////////////////////////////

void Config::removeValue(const QString& key)
{
    m_data_map.remove(key);
    m_int_cache_map.remove(key);
    m_double_cache_map.remove(key);
    m_color_cache_map.remove(key);
    emit signalChanged();
}

/////////////////////////////////////////////////////////////////////////////

bool Config::loadfromFile()
{
    //Logger::log(QString("Config:loadfromFile: (%1)").arg(m_filename));

    QFile file(m_filename);
    if (!file.open(QIODevice::ReadOnly)) 
    {
        Logger::log(QString("Config:loadfromFile: Could not open file %1\n%2").arg(m_filename).arg(file.errorString()));
        file.close();
        return false;
    }

    //Logger::log(QString("Config:loadfromFile: loading from file (%1)").arg(m_filename));
    
    unsigned long line_counter = 0;
    bool error_while_reading = false;
    QTextStream instream(&file);

    while (!instream.atEnd()) 
    {
        ++line_counter;

        QString line = instream.readLine();
        if (line.length() <= 0 || line.startsWith(COMMENT)) continue;

        QStringList items = line.split(m_separator);
        if (items.count() != 2)
        {
            Logger::log(QString("Config:loadfromFile: Could not parse line %1 (%2)").arg(line_counter).arg(line));
            error_while_reading = true;
            continue;
        }
      
        setValue(items[0].trimmed(), items[1].trimmed());
    }

    file.close();
    return !error_while_reading;
}

/////////////////////////////////////////////////////////////////////////////

bool Config::saveToFile()
{
    //Logger::log(QString("Config:saveToFile: (%1/%2)").arg(QDir::current().dirName()).arg(m_filename));

    QFile file(m_filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) 
    {
        Logger::log(QString("Config:saveToFile: Could not open file %1\n%2").arg(m_filename).arg(file.errorString()));
        file.close();
        return false;
    }

    //Logger::log(QString("Config:saveToFile: saving to file (%1)").arg(m_filename));
    
    QTextStream outstream(&file);
    
    QMap<QString, QString>::iterator iter = m_data_map.begin();
    for(; iter != m_data_map.end(); ++iter)
    {
        outstream << iter.key() << m_separator << (*iter) << "\r\n";
    }
    
    file.close();
    return true;
}

/////////////////////////////////////////////////////////////////////////////

void Config::checkKeyAndShowMessageBox(const QString& key) const
{
    if (!contains(key))
    {
#if! VASFMC_GAUGE
        QMessageBox::critical(0, "CONFIG", QString("Configfile (%1) does not contain key (%2)").
                              arg(m_filename).arg(key));
#endif
        Logger::log(QString("Configfile (%1) does not contain key (%2)").arg(m_filename).arg(key));
        MYASSERT(false);
    }
}

/////////////////////////////////////////////////////////////////////////////
