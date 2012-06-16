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

/*! \file    configwidget.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef CONFIGWIDGET_H
#define CONFIGWIDGET_H

#include <QTableWidget>
#include <QTableWidgetItem>

#include "logger.h"

class Config;

/////////////////////////////////////////////////////////////////////////////

//! config widget item
class ConfigWidgetItem : public QObject, public QTableWidgetItem
{
    Q_OBJECT

public:

    enum EntryType{ TYPE_KEY = 0,
                    TYPE_VALUE = 1
    };

    //! Standard Constructor
    ConfigWidgetItem(Config* cfg, const QString& key, EntryType type);

    //! Destructor
    virtual ~ConfigWidgetItem() {};

    virtual void setData(int role, const QVariant &value);
    virtual QVariant data(int role) const;

protected slots:

    void slotConfigChanged();
    void slotConfigDeleted();

protected:

    Config* m_cfg;
    QString m_key;
    EntryType m_type;
    bool m_disabled;

private:
    //! Hidden copy-constructor
    ConfigWidgetItem(const ConfigWidgetItem&);
    //! Hidden assignment operator
    const ConfigWidgetItem& operator = (const ConfigWidgetItem&);
};

/////////////////////////////////////////////////////////////////////////////

//! config widget
class ConfigWidget : public QTableWidget
{
    Q_OBJECT

public:
    //! Standard Constructor
    ConfigWidget(Config* cfg, QWidget* parent = 0);

    //! Destructor
    virtual ~ConfigWidget();

protected slots:

    void slotConfigChanged();
    void slotConfigDeleted();

protected:

    void setup();

protected:

    Config* m_cfg;
    QStringList m_config_key_list;

    bool m_disabled;

private:
    //! Hidden copy-constructor
    ConfigWidget(const ConfigWidget&);
    //! Hidden assignment operator
    const ConfigWidget& operator = (const ConfigWidget&);
};



#endif /* CONFIGWIDGET_H */

// End of file

