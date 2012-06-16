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

/*! \file    containerbase.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef CONTAINERBASE_H
#define CONTAINERBASE_H

#include <QHash>
#include <QString>
#include <QStringList>

//! base container
/*! more details ...
 */
class ContainerBase 
{
public:

	      enum CONTAINER_TYPE { TYPE_BASE = 0,
                                  TYPE_FLIGHTPLAN = 1,
                                  TYPE_TRACK = 2
    };

public:

    //! Standard Constructor
    ContainerBase(CONTAINER_TYPE type);

    //! Destructor
    virtual ~ContainerBase();

    //! returns the type of the container
    virtual CONTAINER_TYPE getType() const { return m_type; }

    //! returns the key of the container (e.g. used for lists)
    virtual const QString& getKey() const = 0;

    //! returns the value of the field with the given name.
    //! if the field does not exists, a null qstring will be returned.
    virtual const QString& get(const QString& name) const;

    //! set the field with the given name to the given value.
    //! returns true on success, false otherwise.
    virtual bool set(const QString& name, const QString& value);

    //! returns a list of all keys.
    virtual QStringList getKeyList() const { return m_field_hash.keys(); }

    //! returns the number of fields of the container
    virtual qint32 count() const { return m_field_hash.count(); }

    //! returns a string with the content of the container
    virtual QString toString() const;

protected:
    
    //! add the given value to the hash under the given name.
    //! the name must be unique and unused.
    virtual void addField(const QString& name, QString* value);

protected:

    //! the type of the container
    CONTAINER_TYPE m_type;

    //! name -> field pointer hash
    QHash<QString, QString*> m_field_hash;

    //! used for return references
    QString m_null_string;

private:
    //! Hidden copy-constructor
    ContainerBase(const ContainerBase&);
    //! Hidden assignment operator
    const ContainerBase& operator = (const ContainerBase&);
};

/////////////////////////////////////////////////////////////////////////////

//! list of containers
/*! more details ...
 */
class ContainerBaseList 
{
public:

	    enum CONTAINER_LIST_TYPE { TYPE_BASE_LIST = 0,
                                       TYPE_FLIGHTPLAN_LIST = 1,
                                       TYPE_TRACK_LIST = 2
    };

public:

    //! Standard Constructor
    ContainerBaseList(CONTAINER_LIST_TYPE type);

    //! Destructor
    virtual ~ContainerBaseList();

    //! returns the 
    virtual qint32 count() const { return m_container_hash.count(); }

    //! returns the type of the container
    virtual CONTAINER_LIST_TYPE getType() const { return m_type; }

    //! add the given container. 
    //! returns true on success, false otherwise.
    virtual bool add(ContainerBase* container);
    
    //! returns the container with the specified key.
    //! will return a null pointer on error.
    virtual ContainerBase* get(const QString& key) const;

    //! removes the container with the given key.
    //! return true on success, false otherwise.
    virtual bool remove(const QString& key);

    //! returns a list of all keys.
    virtual QStringList getKeyList() const { return m_container_hash.keys(); }

    //! returns a string with the content of the container list
    virtual QString toString() const;

protected:

    //! called whenever a container is added to the list (and maybe on
    //! other occassions).
    //! shoud return true if the given container meets all requirements 
    //! of the list, false otherwise.
    virtual bool isValidContainer(const ContainerBase* container) = 0;

protected:

    //! the type of the container list
    CONTAINER_LIST_TYPE m_type;

    //! name -> container hash
    QHash<QString, ContainerBase*> m_container_hash;

private:
    //! Hidden copy-constructor
    ContainerBaseList(const ContainerBaseList&);
    //! Hidden assignment operator
    const ContainerBaseList& operator = (const ContainerBaseList&);
};

#endif /* CONTAINERBASE_H */

// End of file

