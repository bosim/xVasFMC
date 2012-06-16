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

/*! \file    xml.model.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include <QFile>

#include "xml.model.h"

/////////////////////////////////////////////////////////////////////////////

XMLModel::XMLModel(bool save_on_close) : m_save_on_close(save_on_close), m_dirty(false)
{
}

/////////////////////////////////////////////////////////////////////////////

XMLModel::~XMLModel()
{ 
    close();
}

/////////////////////////////////////////////////////////////////////////////

void XMLModel::close()
{
    if (m_save_on_close && m_dirty && !m_xml_dom.isNull()) saveToXMLFile();
    m_xml_filename = QString::null;
    m_xml_dom = QDomDocument();
}

/////////////////////////////////////////////////////////////////////////////

bool XMLModel::loadFromXMLFile(const QString& filename, QString& err_msg)
{
    close();

    QFile xmlfile(filename);
    if (!xmlfile.open(QIODevice::ReadOnly))
    {
        printf("XMLModel:loadFromXMLFile: ERROR: Could not open (%s)\n",
               filename.toLatin1().data());
        fflush(stdout);
        return false;
    }

    QString xml_error;
    int xml_error_line=0, xml_error_col=0;

    if (!m_xml_dom.setContent(&xmlfile, true, &xml_error, &xml_error_line, &xml_error_col))
    {
        err_msg = 
            QString("Line %1, Column %2: %3").
            arg(xml_error_line).arg(xml_error_col).arg(xml_error);

        printf("XMLModel:loadFromXMLFile: ERROR: Could not load file (%s): %s\n",
               filename.toLatin1().data(), err_msg.toLatin1().data());
        fflush(stdout);
        return false;
    }

    bool ret = parseDOM(err_msg);
    if (ret) 
    {
        m_xml_filename = filename;
        emit signalNewFilename(m_xml_filename);
    }

    m_dirty = false;
    return ret;
}

/////////////////////////////////////////////////////////////////////////////

bool XMLModel::saveToXMLFile(const QString& filename)
{
    emit signalBeforeSave();

    QString eff_filename = filename;
    if (eff_filename.isEmpty()) eff_filename = m_xml_filename;

    if (eff_filename.isEmpty())
    {
        printf("XMLModel:saveToXMLFile: No filename\n");
        fflush(stdout);
        return false;
    }
    
    QFile xmlfile(eff_filename);
    if (!xmlfile.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        printf("XMLModel:saveToXMLFile: ERROR: Could not open (%s)\n",
               eff_filename.toLatin1().data());
        fflush(stdout);
        return false;
    }
    
    xmlfile.write(m_xml_dom.toByteArray());
    m_dirty = false;

    printf("XMLModel:saveToXMLFile: saved to file (%s)\n", eff_filename.toLatin1().data());
    fflush(stdout);

    m_xml_filename = eff_filename;
    emit signalNewFilename(m_xml_filename);
    return true;
}

// End of file
