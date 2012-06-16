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

#ifndef owneddata_h
#define owneddata_h

#include "dataref.h"
#include "XPLMDataAccess.h"

template <typename T>
T readFunc(void*);
template <typename T>
void writeFunc(void*, T);

long readFuncStr(void*, void*, int, long);
void writeFuncStr(void*, void*, int, long);

/**
  * Data owned by the plugin and published as a XPLM DataRef, create an instance and call a register function
  * to make data accessible to other plugins
  * @author Philipp Muenzel
  * @version 0.2
  * @file owneddata.h
  */
template <typename T>
class OwnedData : public DataRef<T> {

 public:

    /**
      * create owned date for sharing.
      * the identifier provided is the string identifier later used for the dataref.
      * Please consult the XP SDK naming convetions on how to name things right
      * @param std::string& identifier
      */
    OwnedData(const std::string& identifier);

    /**
      * register the data as dataref with read-only access
      */
    virtual void registerRead();

    /**
      * register the data as dataref with write-only access
      */
    virtual void registerWrite();

    /**
      * register the data as dataref with read-and-write access
      */
    virtual void registerReadWrite();

    /**
      * unregister the dataref from XPLM
      */
    virtual void unregister();

    /**
      * @return true if the dataref registered and accessible ?
      */
    virtual bool isRegistered() { return m_isRegistered; }

  private:

    void ensureValid();

    friend T readFunc<T>(void* inRefCon);

    friend void writeFunc<T>(void* inRefCon, T data);

    friend long readFuncStr(void* inRefCon, void* outValue, int inOffset, long inMaxLength);

    friend void writeFuncStr(void* inRefCon, void* inValue, int inOffset, long inLength);

    bool m_isRegistered;

};


template <typename T>
T readFunc(void* inRefCon)
{
    OwnedData<T>* pOwnedData = static_cast<OwnedData<T>*>(inRefCon);
    return pOwnedData->data();
}
template <>
bool readFunc(void* inRefCon);

template <typename T>
void writeFunc(void* inRefCon, T data)
{
    OwnedData<T>* pOwnedData = static_cast<OwnedData<T>*>(inRefCon);
    pOwnedData->set(data);
}
template <>
void writeFunc(void* inRefCon, bool data);

template <typename T>
OwnedData<T>::OwnedData(const std::string& identifier):
                DataRef<T>(identifier), m_isRegistered(false)
{
}

template <typename T>
void OwnedData<T>::unregister()
{
    if (m_isRegistered && this->m_pDataRef != 0)
        XPLMUnregisterDataAccessor(this->m_pDataRef);
    m_isRegistered = false;
}

template <typename T>
void OwnedData<T>::ensureValid()
{
    if (this->m_pDataRef != 0)
        m_isRegistered = true;
    else
        m_isRegistered = false;
}

template <>
void OwnedData<int>::registerRead();
template <>
void OwnedData<float>::registerRead();
template <>
void OwnedData<double>::registerRead();
template <>
void OwnedData<std::string>::registerRead();
template <>
void OwnedData<bool>::registerRead();

template <>
void OwnedData<int>::registerWrite();
template <>
void OwnedData<float>::registerWrite();
template <>
void OwnedData<double>::registerWrite();
template <>
void OwnedData<std::string>::registerWrite();
template <>
void OwnedData<bool>::registerWrite();

template <>
void OwnedData<int>::registerReadWrite();
template <>
void OwnedData<float>::registerReadWrite();
template <>
void OwnedData<double>::registerReadWrite();
template <>
void OwnedData<std::string>::registerReadWrite();
template <>
void OwnedData<bool>::registerReadWrite();

#endif
