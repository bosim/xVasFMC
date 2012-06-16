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

#ifndef CONFIG_H
#define CONFIG_H

#include "assert.h"
#include <QString>
#include <QMap>
#include <QObject>
#include <QColor>

#define LIMIT(expr, max) qMin(qMax(expr, -max), max)
#define LIMITMINMAX(expr, min, max) qMin(qMax(expr, min), max)

/////////////////////////////////////////////////////////////////////////////

class Config : public QObject
{
    Q_OBJECT

public:

    enum DataType { TYPE_STRING = 0,
                    TYPE_INT = 1,
                    TYPE_DOUBLE = 2
    };
    
    //! The filename shall be specified as a relative path (relative to the vasFMC directory)
    Config(const QString& filename, const QString& separator = "=");

    void setValue(const QString& key, const QString& value);    
    void setValue(const QString& key, const int& value);
    void setValue(const QString& key, const uint& value);
	void setValue(const QString& key, const double& value);

    //! returns true if the config contains the given key, false otherwise.
    bool contains(const QString& key) const;

    QString getValue(const QString& key) const;
    int getIntValue(const QString& key) const;
	double getDoubleValue(const QString& key) const;
    QColor getColorValue(const QString& key) const;

    void removeValue(const QString& key);

    const QString& filename() const { return m_filename; }

    bool loadfromFile();
    bool saveToFile();

    inline QStringList keyList() const { return m_data_map.keys(); }

    void checkKeyAndShowMessageBox(const QString& key) const;

signals:

    void signalChanged();
    
protected:
    
    QString m_filename;
    QString m_separator;
    QMap<QString, QString> m_data_map;

    mutable QMap<QString, int> m_int_cache_map;
    mutable QMap<QString, double> m_double_cache_map;
    mutable QMap<QString, QColor> m_color_cache_map;
};

/////////////////////////////////////////////////////////////////////////////

class ConfigWidgetProvider
{
public:

    ConfigWidgetProvider() {};
    virtual ~ConfigWidgetProvider() {};

    virtual void registerConfigWidget(const QString& title, Config* cfg) = 0;
    virtual void unregisterConfigWidget(const QString& title) = 0;

};

#endif
