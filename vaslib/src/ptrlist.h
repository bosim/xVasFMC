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

/*! \file    ptrlist.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef PTRLIST_H
#define PTRLIST_H

#include "assert.h"
#include <QList>

/////////////////////////////////////////////////////////////////////////////

//! pointerlist
template <class TYPE> class PtrList: public QList<TYPE* >
{
public:
    //! Standard Constructor
    PtrList(bool auto_delete = true) : m_auto_delete(auto_delete) {};

    //! Destructor
    virtual ~PtrList() { if (m_auto_delete) qDeleteAll(*this); }

    void setAutoDelete(bool auto_delete) { m_auto_delete = auto_delete; }
    bool autoDelete() const { return m_auto_delete; }

    PtrList(const PtrList& other):QList<TYPE* >(other)
    {
        *this = other;
    }

    PtrList& operator=(const PtrList& other)
    {
        if (&other == this) return *this;
        clear();
        for(int index=0; index < other.count(); ++index) append(other.at(index)->deepCopy());
        return *this;
    }

    void removeAt(int pos)
    {
		TYPE* item = (*this)[pos];
		MYASSERT(item != 0);
		QList<TYPE*>::removeAt(pos);
		if (m_auto_delete) delete item;
		item = 0;
    }

    void clear()
    {
		if (m_auto_delete) qDeleteAll(*this);
		QList<TYPE*>::clear();
    }

    //! do a deepcopy
    //! ATTENTION: the caller is responsible to delete the returned list!
    PtrList* deepCopy() const
    {
        PtrList* copied_list = new PtrList;
        MYASSERT(copied_list != 0);

        for(int index=0; index < QList<TYPE* >::count(); ++index)
            copied_list->append(QList<TYPE* >::at(index)->deepCopy());

        return copied_list;
	}

    //! will insert the given value sorted by value::operator<
    void insertSorted(TYPE* value)
    {
        int index = 0;
        for(; index < QList<TYPE* >::count(); ++index) if (*value < *QList<TYPE* >::at(index)) break;
        QList<TYPE* >::insert(index, value);
    }

protected:

    bool m_auto_delete;
};

#endif /* PTRLIST_H */

// End of file

