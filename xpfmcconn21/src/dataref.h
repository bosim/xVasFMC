///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Philipp Muenzel
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

#ifndef dataref_h
#define dataref_h

#include <string>
#include <XPLMDataAccess.h>

/**
 * base class for providing an OO-style access to XP DataRefs.
 * does no binding or lookup, it is just a base for simdata (datarefs provided by X-Plane)
 * and owned data (datarefs provided by the plugin itself)
 * @author (c) 2009 Philipp Muenzel, Department of Mathematics, Technische Universitaet Darmstadt
 * @version 0.2
 * @file dataref.h
 *
*/
template <typename T>
class DataRef {

 public:

    /**
      * create an instance to store the opaque handle
      */
    DataRef();
    
    virtual ~DataRef();

    /**
      * create an instance to store the opaque handle and save its refname as a string.
      * Note that there is no binding or lookup, it just stores the string
      * @param dataRefIdentifier Identfier string for lookup in XPLM
      */
    DataRef(const std::string& dataRefIdentifier);

    /**
     * acessor function to get data (note that there's no polling at this moment)
     * @return the internally stored data
     */
    virtual T data();


    /**
     * write data to the local store (not written directly to X-Plane).
     * @param data the value to store locally
     * @return true if dataref is valid and data could be written
     */
    virtual bool set(T data);

 protected:

    /**
     * the internal value (not necessarily consistent with the value inside X-Plane)
     */
    T m_data;

    /**
     * stores the opaque handle to the XP-SDK Dataref
     */
    XPLMDataRef m_pDataRef;

    /**
     * reference string to the DataRefs Name as published in datarefs.txt
     */
    std::string m_dataRefIdentifier;
};

template <typename T>
DataRef<T>::DataRef():
            m_pDataRef(0),
            m_dataRefIdentifier("")
{}

template <typename T>
DataRef<T>::~DataRef()
{}

template <typename T>
DataRef<T>::DataRef(const std::string& dataRefIdentifier):
            m_data(T()),
            m_pDataRef(0),
            m_dataRefIdentifier(dataRefIdentifier)
{}

template <typename T>
bool DataRef<T>::set(T data)
{
    if (m_pDataRef != 0)
    {
        m_data = data;
        return true;
    } else
        return false;
}

template <typename T>
T DataRef<T>::data()
{
    if (m_pDataRef != 0)
        return m_data;
    else
        return T(0);
}

#endif
