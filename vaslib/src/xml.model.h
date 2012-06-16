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

/*! \file    xml.model.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __XML_MODEL_H__
#define __XML_MODEL_H__

#include <QDomDocument>
#include <QString>
#include <QObject>

//! Data model for xmls
class XMLModel : public QObject
{
    Q_OBJECT

public:
    //! Standard Constructor
    XMLModel(bool save_on_close = false);

    //! Destructor
    virtual ~XMLModel();

    //! clears all xml model content
    virtual void close();

    //! returns if the model has been changed since the last save or load
    bool isDirty() const { return m_dirty; }

    //! sets the dirty flag, should be called whenever the model changes
    void setDirty() { m_dirty = true; }

    //! Returns the filename of the currently open XML file, or a null string
    //! when no file os open.    
    virtual const QString& getCurrentFilename() const { return m_xml_filename; }

    //! returns true if the xml is saved on exit
    void setSaveOnExit(bool save_on_close) { m_save_on_close = save_on_close; }

    //! loads the xml from the given file and returns true on success
    bool loadFromXMLFile(const QString& filename, QString& err_msg);

    //! saves the xml to the given file. If the given filename is empty, the
    //! file will be saved to the opened file.
    bool saveToXMLFile(const QString& filename = QString::null);

    //! parses the xml DOM and returns true on success
    virtual bool parseDOM(QString& err_msg) { /* does nothing for the base class*/ return false; };

signals:

    void signalChanged();
    void signalBeforeSave();
    void signalNewFilename(const QString& filename);
    
protected:

    bool m_save_on_close;
    QString m_xml_filename;
    QDomDocument m_xml_dom;
    bool m_dirty;

private:
    //! Hidden copy-constructor
    XMLModel(const XMLModel&);
    //! Hidden assignment operator
    const XMLModel& operator = (const XMLModel&);
};

#endif /* __XML.MODEL_H__ */

// End of file
