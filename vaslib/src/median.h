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

/*! \file    median.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef MEDIAN_H
#define MEDIAN_H

#include "assert.h"
#include "navcalc.h"

/////////////////////////////////////////////////////////////////////////////

//! mean value
template <class TYPE> class Median 
{
public:
    //! Standard Constructor
    Median(uint length, const TYPE& init_value) : 
        m_init_value(init_value), m_middle_index(Navcalc::round(length/2.0)), m_length(length)
    {
        for(uint index=0; index<length; ++index) m_value_list.append(init_value);
        m_sorted_list = m_value_list;
    };

    //! Destructor
    virtual ~Median()
    {};

    //! calculates the new mean value with the given value
    inline void add(const TYPE& value) 
    { 
        m_value_list.removeFirst();
        m_value_list.append(value);
        MYASSERT(m_value_list.count() == (int)m_length);
    
        m_sorted_list = m_value_list;
        qSort(m_sorted_list.begin(), m_sorted_list.end());
    }
    
    //! returns the calculated mean value
    inline const TYPE& median() const { return m_value_list[m_middle_index]; }

    inline uint length() const { return m_length; }

    void clear()
    {
        m_value_list.clear();
        m_sorted_list.clear();
        for(uint index=0; index<m_length; ++index) m_value_list.append(m_init_value);
        m_sorted_list = m_value_list;
    }
    
protected:
    
    TYPE m_init_value;
    int m_middle_index;
    QList<TYPE> m_value_list;
    QList<TYPE> m_sorted_list;
    uint m_length;
};

#endif /* MEDIAN_H */

// End of file

