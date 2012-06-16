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

#include "owneddata.h"
#include <iostream>
#include <cstring>

template <>
void OwnedData<int>::registerRead()
{
    this->m_pDataRef = XPLMRegisterDataAccessor( this->m_dataRefIdentifier.c_str(), xplmType_Int, 0,
                                                 readFunc<int>, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                                                 NULL, NULL, NULL, NULL, this, NULL );
    ensureValid();
}
template <>
void OwnedData<float>::registerRead()
{
    this->m_pDataRef = XPLMRegisterDataAccessor( this->m_dataRefIdentifier.c_str(), xplmType_Float, 0,
                                                 NULL, NULL, readFunc<float>, NULL, NULL, NULL, NULL, NULL,
                                                 NULL, NULL, NULL, NULL, this, NULL );
    ensureValid();
}
template <>
void OwnedData<double>::registerRead()
{
    this->m_pDataRef = XPLMRegisterDataAccessor( this->m_dataRefIdentifier.c_str(), xplmType_Double, 0,
                                                 NULL, NULL, NULL, NULL, readFunc<double>, NULL, NULL, NULL,
                                                 NULL, NULL, NULL, NULL, this, NULL);
    ensureValid();
}

template <>
void OwnedData<std::string>::registerRead()
{
    this->m_pDataRef = XPLMRegisterDataAccessor( this->m_dataRefIdentifier.c_str(), xplmType_Data, 0,
                                                 NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                                                 NULL, NULL, readFuncStr, NULL, this, NULL );
    ensureValid();
}
template <>
void OwnedData<bool>::registerRead()
{
    this->m_pDataRef = XPLMRegisterDataAccessor( this->m_dataRefIdentifier.c_str(), xplmType_Int, 0,
                                                 readFunc<int>, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                                                 NULL, NULL, NULL, NULL, this, NULL );
    ensureValid();
}


//////////////////////////////

template <>
void OwnedData<int>::registerWrite()
{
    this->m_pDataRef = XPLMRegisterDataAccessor( this->m_dataRefIdentifier.c_str(), xplmType_Int, 1,
                                             NULL, writeFunc<int>, NULL, NULL, NULL, NULL, NULL, NULL,
                                             NULL, NULL, NULL, NULL, NULL, this );
    ensureValid();
}

template <>
void OwnedData<float>::registerWrite()
{
    this->m_pDataRef = XPLMRegisterDataAccessor( this->m_dataRefIdentifier.c_str(), xplmType_Float, 1,
                                             NULL, NULL, NULL, writeFunc<float>, NULL, NULL, NULL, NULL,
                                             NULL, NULL, NULL, NULL, NULL, this );
    ensureValid();
}

template <>
void OwnedData<double>::registerWrite()
{
    this->m_pDataRef = XPLMRegisterDataAccessor( this->m_dataRefIdentifier.c_str(), xplmType_Double, 1,
                                                 NULL, NULL, NULL, NULL, NULL, writeFunc<double>, NULL, NULL,
                                                 NULL, NULL, NULL, NULL, NULL, this );
    ensureValid();
}

template <>
void OwnedData<std::string>::registerWrite()
{
    this->m_pDataRef = XPLMRegisterDataAccessor( this->m_dataRefIdentifier.c_str(), xplmType_Data, 1,
                                                 NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                                                 NULL, NULL, NULL, writeFuncStr, NULL, this );
    ensureValid();
}

template <>
void OwnedData<bool>::registerWrite()
{
    this->m_pDataRef = XPLMRegisterDataAccessor( this->m_dataRefIdentifier.c_str(), xplmType_Int, 1,
                                             NULL, writeFunc<int>, NULL, NULL, NULL, NULL, NULL, NULL,
                                             NULL, NULL, NULL, NULL, NULL, this );
    ensureValid();
}

///////////////////

template <>
void OwnedData<int>::registerReadWrite()
{
    this->m_pDataRef = XPLMRegisterDataAccessor( this->m_dataRefIdentifier.c_str(), xplmType_Int, 1,
                                                 readFunc<int>, writeFunc<int>, NULL, NULL, NULL, NULL, NULL, NULL,
                                                 NULL, NULL, NULL, NULL, this, this );
    ensureValid();
}

template <>
void OwnedData<float>::registerReadWrite()
{
    this->m_pDataRef = XPLMRegisterDataAccessor( this->m_dataRefIdentifier.c_str(), xplmType_Float, 1,
                                                 NULL, NULL, readFunc<float>, writeFunc<float>, NULL, NULL, NULL, NULL,
                                                 NULL, NULL, NULL, NULL, this, this );
    ensureValid();
}

template <>
void OwnedData<double>::registerReadWrite()
{
    this->m_pDataRef = XPLMRegisterDataAccessor( this->m_dataRefIdentifier.c_str(), xplmType_Int, 1,
                                                 NULL, NULL, NULL, NULL, readFunc<double>, writeFunc<double>, NULL, NULL,
                                                 NULL, NULL, NULL, NULL, this, this );
    ensureValid();
}

template <>
void OwnedData<std::string>::registerReadWrite()
{
    this->m_pDataRef = XPLMRegisterDataAccessor( this->m_dataRefIdentifier.c_str(), xplmType_Data, 1,
                                                 NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                                                 NULL, NULL, readFuncStr, writeFuncStr, this, this );
    ensureValid();
}
template <>
void OwnedData<bool>::registerReadWrite()
{
    this->m_pDataRef = XPLMRegisterDataAccessor( this->m_dataRefIdentifier.c_str(), xplmType_Int, 1,
                                                 readFunc<int>, writeFunc<int>, NULL, NULL, NULL, NULL, NULL, NULL,
                                                 NULL, NULL, NULL, NULL, this, this );
    ensureValid();
}

//////////////////////////

template <>
bool readFunc(void* inRefCon)
{
    OwnedData<bool>* pOwnedData = static_cast<OwnedData<bool>*>(inRefCon);
    return (pOwnedData->data() == 1);
}

template <>
void writeFunc(void* inRefCon, bool data)
{
    OwnedData<bool>* pOwnedData = static_cast<OwnedData<bool>*>(inRefCon);
    pOwnedData->set(data==1);
}

/////////////////////////

long readFuncStr(void* inRefCon, void* outValue, int inOffset, long inMaxLength)
{
    OwnedData<std::string>* pOwnedData = static_cast<OwnedData<std::string>*>(inRefCon);
    if (outValue == NULL)
        return pOwnedData->data().length() + 1;
    strcpy(static_cast<char*>(outValue), pOwnedData->data().substr(inOffset,inMaxLength).c_str());
    return inMaxLength;
}

void writeFuncStr(void* inRefCon, void* inValue, int inOffset, long inMaxLength)
{
    OwnedData<std::string>* pOwnedData = static_cast<OwnedData<std::string>*>(inRefCon);
    char* str = static_cast<char*>(inValue);
    pOwnedData->set(std::string(str).substr(inOffset,inMaxLength));
}
