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

/*! \file    meanvalue.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef MEANVALUE_H
#define MEANVALUE_H

/////////////////////////////////////////////////////////////////////////////

//! mean value
template <class TYPE> class MeanValue 
{
public:
    //! Standard Constructor
    MeanValue(uint length, const TYPE& init_value) : m_length(length), m_mean_value(init_value), m_mean_init_value(init_value)
    {};

    //! Destructor
    virtual ~MeanValue()
    {};

    //! calculates the new mean value with the given value
    inline void add(const TYPE& value) 
    { 
        if (m_value_list.count() >= (int)m_length)
        {
            m_mean_value -= m_value_list.first() / m_length;
            m_value_list.removeFirst();
        }
        m_value_list.append(value); 
        m_mean_value += value / m_length;
    }

    //! returns the calculated mean value
    inline const TYPE& mean() const { return m_mean_value; }

    void clear()
    {
        m_value_list.clear();
        m_mean_value = m_mean_init_value;
    }

protected:

    QList<TYPE> m_value_list;
    uint m_length;
    TYPE m_mean_value;
    TYPE m_mean_init_value;
};

#endif /* MEANVALUE_H */

// End of file

