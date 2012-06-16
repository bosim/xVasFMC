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

/*! \file    configwidget.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "config.h"
#include "assert.h"

#include "configwidget.h"

/////////////////////////////////////////////////////////////////////////////

ConfigWidgetItem::ConfigWidgetItem(Config* cfg, const QString& key, EntryType type) : 
    QTableWidgetItem(QTableWidgetItem::UserType+10), m_cfg(cfg), m_key(key), m_type(type), m_disabled(false)
{
    MYASSERT(cfg != 0);
    MYASSERT(cfg->contains(key));

    setTextAlignment(Qt::AlignLeft);

    if (m_type == TYPE_VALUE) setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
    else                      setFlags(Qt::ItemIsEnabled);

    MYASSERT(connect(m_cfg, SIGNAL(signalChanged()), this, SLOT(slotConfigChanged())));
}

/////////////////////////////////////////////////////////////////////////////

void ConfigWidgetItem::slotConfigChanged()
{
    if (m_type == TYPE_VALUE) setText(data(Qt::DisplayRole).toString());
}

/////////////////////////////////////////////////////////////////////////////

void ConfigWidgetItem::slotConfigDeleted()
{
    m_disabled = true;
}

/////////////////////////////////////////////////////////////////////////////

void ConfigWidgetItem::setData(int role, const QVariant &value)
{
    if (role == Qt::EditRole)
    {
        if (m_disabled) return;
        if (m_type == TYPE_VALUE) m_cfg->setValue(m_key, value.toString());
    }
    else
    {
        QTableWidgetItem::setData(role, value);
    }
}

/////////////////////////////////////////////////////////////////////////////

QVariant ConfigWidgetItem::data(int role) const 
{
   
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        if (m_disabled) return "";
        if (m_type == TYPE_KEY) return m_key;
        else return m_cfg->getValue(m_key);
    }
    else
    {
        return QTableWidgetItem::data(role);
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

ConfigWidget::ConfigWidget(Config* cfg, QWidget* parent) : 
    QTableWidget(parent), m_cfg(cfg), m_disabled(false)
{
    MYASSERT(m_cfg != 0);
    setSelectionMode(QAbstractItemView::NoSelection);
    setup();

    MYASSERT(connect(m_cfg, SIGNAL(signalChanged()), this, SLOT(slotConfigChanged())));
    MYASSERT(connect(m_cfg, SIGNAL(destroyed()), this, SLOT(slotConfigDeleted())));
}

/////////////////////////////////////////////////////////////////////////////

ConfigWidget::~ConfigWidget()
{

}

/////////////////////////////////////////////////////////////////////////////

void ConfigWidget::slotConfigChanged()
{
    setup();
}

/////////////////////////////////////////////////////////////////////////////

void ConfigWidget::slotConfigDeleted()
{
    m_disabled = true;
    deleteLater();
}

/////////////////////////////////////////////////////////////////////////////

void ConfigWidget::setup()
{
    if (m_disabled) return;

    QStringList config_key_list = m_cfg->keyList();
    
    if (config_key_list != m_config_key_list) 
    {
        clear();
        m_config_key_list = config_key_list;

        setRowCount(config_key_list.count());
        setColumnCount(1);
        setHorizontalHeaderLabels(QStringList("Value"));

        for(int index=0; index<config_key_list.count(); ++index)
        {
            setVerticalHeaderItem(index, new ConfigWidgetItem(m_cfg, config_key_list[index], ConfigWidgetItem::TYPE_KEY));
            setItem(index, 0, new ConfigWidgetItem(m_cfg, config_key_list[index], ConfigWidgetItem::TYPE_VALUE));
        }
    }
}

// End of file
