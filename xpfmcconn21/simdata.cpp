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

#include "simdata.h"
#include <math.h>
#include <cstring>
#include <fstream>

extern std::fstream m_logfile;

template <>
bool SimData<int>::poll()
{
    if(m_readWrite == RWType::WriteOnly) return false;
    return updateValue((int)XPLMGetDatai(m_pDataRef));
}

template <>
bool SimData<float>::poll()
{
    if(m_readWrite == RWType::WriteOnly) return false;
    return updateValue((float)XPLMGetDataf(m_pDataRef));
}

template <>
bool SimData<double>::poll()
{
    if (m_pDataRef == 0) {
        m_logfile << "In polling no DataRef for " << this->name() << std::endl;
        exit(1);
    }
    double temp = (double)XPLMGetDatad(m_pDataRef);
    if (isnan(temp)){
        m_logfile << "In polling nan from XPlane for " << this->name() << std::endl;
        updateValue(0);
        //exit(1);
    }
    if(m_readWrite == RWType::WriteOnly) return false;
    return updateValue(temp);
}

template <>
bool SimData<bool>::poll()
{
    if(m_readWrite == RWType::WriteOnly) return false;
    return updateValue(((int)XPLMGetDatai(m_pDataRef))==1);
}

template <>
bool SimData<std::vector<float> >::poll()
{
    if(m_readWrite == RWType::WriteOnly) return false;
    float* values = new float(m_no_of_items);
    XPLMGetDatavf(this->m_pDataRef,values,0,m_no_of_items);

    return updateValue(std::vector<float>(values,values + sizeof(values)/sizeof(float)));
}

template <>
bool SimData<std::vector<int> >::poll()
{
    if(m_readWrite == RWType::WriteOnly) return false;
    int* values = new int(m_no_of_items);
    XPLMGetDatavi(this->m_pDataRef,values,0,m_no_of_items);
    return updateValue(std::vector<int>(values, values + sizeof(values)/sizeof(int)));
}

template <>
bool SimData<std::string>::poll()
{
    if(m_readWrite == RWType::WriteOnly) return false;
    long n = XPLMGetDatab(this->m_pDataRef,NULL,0,0);
    char* outString = new char(n);
    XPLMGetDatab(this->m_pDataRef, outString, 0, n);
    return updateValue(std::string(outString));
}

/////////////////////////////


template <>
bool SimData<int>::set(int data)
{
    if(m_readWrite == RWType::WriteOnly) return false;
    m_hasChanged = true;
    if (m_scale != 1)
        setValue(int(ceil((data-m_offset)/m_scale)));
    else
        setValue(data);
    return m_hasChanged;
}

template <>
bool SimData<bool>::set(bool data)
{
if(m_readWrite == RWType::WriteOnly) return false;
    m_hasChanged = true;
    setValue(data);
    return m_hasChanged;
}

template <>
bool SimData<std::vector<float> >::set(std::vector<float> data)
{
    if(m_readWrite == RWType::WriteOnly) return false;
    m_hasChanged = true;
    if (data.size() > m_no_of_items)
    {
        m_logfile << "Requested set for vector "<< this->name() << " too long, cropping from "
                << data.size() << " to " << m_no_of_items << " items." << std::endl;
        data.resize(m_no_of_items);
    }
    if (m_scale != 1) {
        for( uint i=0 ; i<data.size() ; i++ )
        {
            data[i] = (data[i]-m_offset)/m_scale;
        }
    }
    setValue(data);
    return m_hasChanged;
}

template <>
bool SimData<std::vector<int> >::set(std::vector<int> data)
{
    if(m_readWrite == RWType::WriteOnly) return false;
    m_hasChanged = true;
    if (data.size() > m_no_of_items)
    {
        m_logfile << "Requested set for vector "<< this->name() << " too long, cropping from "
                << data.size() << " to " << m_no_of_items << " items." << std::endl;
        data.resize(m_no_of_items);
    }
    if (m_scale != 1) {
        for( uint i=0 ; i<data.size() ; i++ )
        {
            data[i] = int(ceil((data[i]-m_offset)/m_scale));
        }
    }
    setValue(data);
    return m_hasChanged;
}

template<>
bool SimData<std::string>::set(std::string data)
{
    if(m_readWrite == RWType::WriteOnly) return false;
    m_hasChanged = true;
    setValue(data);
    return m_hasChanged;
}

////////////////////////////////

template <>
bool SimData<float>::checkDataType()
{
    XPLMDataTypeID DataTypeID = XPLMGetDataRefTypes(this->m_pDataRef);
    return (DataTypeID == xplmType_Float );
}

template <>
bool SimData<double>::checkDataType()
{
    XPLMDataTypeID DataTypeID = XPLMGetDataRefTypes(this->m_pDataRef);
    return (DataTypeID == (xplmType_Float | xplmType_Double));
}

template <>
bool SimData<int>::checkDataType()
{
    XPLMDataTypeID DataTypeID = XPLMGetDataRefTypes(this->m_pDataRef);
    return (DataTypeID == xplmType_Int);
}

template <>
bool SimData<bool>::checkDataType()
{
    XPLMDataTypeID DataTypeID = XPLMGetDataRefTypes(this->m_pDataRef);
    return (DataTypeID == xplmType_Int);
}

template <>
bool SimData<std::vector<float> >::checkDataType()
{
    XPLMDataTypeID DataTypeID = XPLMGetDataRefTypes(this->m_pDataRef);
    if (m_no_of_items == 0)
        m_no_of_items = XPLMGetDatavf(this->m_pDataRef,NULL,0,0);
    m_data.resize(m_no_of_items);
    return (DataTypeID == xplmType_FloatArray);
}

template <>
bool SimData<std::vector<int> >::checkDataType()
{
    XPLMDataTypeID DataTypeID = XPLMGetDataRefTypes(this->m_pDataRef);
    if (m_no_of_items == 0)
        m_no_of_items = XPLMGetDatavi(this->m_pDataRef,NULL,0,0);
    m_data.resize(m_no_of_items);
    return (DataTypeID == xplmType_IntArray);
}

template <>
bool SimData<std::string>::checkDataType()
{
    XPLMDataTypeID DataTypeID = XPLMGetDataRefTypes(this->m_pDataRef);
    return (DataTypeID == xplmType_Data);
}

///////////////

template <>
bool SimData<bool>::updateValue(bool data)
{
    if (data != m_data) {
        m_data = data;
        m_hasChanged = true;
        return (m_hasChanged);
    }
    return true;
}
// use abs instead of fabs for int data
template <>
bool SimData<int>::updateValue(int data)
{
    if (abs(m_data - data) > m_epsilon) {
        m_data = data;
        m_hasChanged = true;
        return (m_hasChanged);
    }
    return true;
}

template <>
bool SimData<std::vector<float> >::updateValue(std::vector<float> data)
{

    for(uint i=0;i<data.size();i++)
    {
        if (fabs(data[i] - m_data[i]) > m_epsilon) {
            m_data[i] = data[i];
            m_hasChanged = true;
        }
    }

    return true;
}
template <>
bool SimData<std::vector<int> >::updateValue(std::vector<int> data)
{
    for(uint i=0;i<data.size();i++)
    {
        if (abs(data[i] - m_data[i]) > m_epsilon) {
            m_data[i] = data[i];
            m_hasChanged = true;
        }
    }
    return true;
}

template<>
bool SimData<std::string>::updateValue(std::string data)
{
    m_data.resize(data.size());
    m_data.assign(data);
    m_hasChanged = true;
    return m_hasChanged;
}

///////////////////////

template <>
void SimData<double>::setValue(double data)
{
    XPLMSetDatad(this->m_pDataRef, data);
}

template <>
void SimData<float>::setValue(float data)
{
    XPLMSetDataf(this->m_pDataRef, data);
}

template <>
void SimData<int>::setValue(int data)
{
    XPLMSetDatai(this->m_pDataRef, data);
}

template <>
void SimData<bool>::setValue(bool data)
{
    if(data)
        XPLMSetDatai(this->m_pDataRef, 1);
    else
        XPLMSetDatai(this->m_pDataRef, 0);
}

template <>
void SimData<std::vector<float> >::setValue(std::vector<float> data)
{
    float* values = new float(m_no_of_items);
    for (uint i=0 ; i < m_no_of_items ; i++)
        values[i] = data[i];
    XPLMSetDatavf(this->m_pDataRef,values,0,m_no_of_items);
}
template <>
void SimData<std::vector<int> >::setValue(std::vector<int> data)
{
    int* values = new int(m_no_of_items);
    for (uint i = 0 ; i < m_no_of_items ; i++)
        values[i]=data[i];
    XPLMSetDatavi(this->m_pDataRef,values,0,m_no_of_items);
}

template<>
void SimData<std::string>::setValue(std::string data)
{
    int n = data.size()+1;
    char* cstr = new char(n);
    strcpy (cstr, data.c_str());
    XPLMSetDatab(this->m_pDataRef, cstr , 0, n);
}


//////////////////////////////

template <>
int SimData<int>::data()
{
    if (m_scale == 1 && m_offset == 0)
        return m_data;
    return int(ceil(m_data * m_scale + m_offset));
}

template<>
std::vector<float> SimData<std::vector<float> >::data()
{

    if (m_scale == 1 && m_offset == 0)
        return m_data;
    std::vector<float> returnvector;
    returnvector.resize(m_data.size());
    for (uint i=0 ; i<m_data.size() ; i++)
        returnvector[i]=m_data[i]*m_scale + m_offset;
    return std::vector<float>(returnvector.begin(),returnvector.end());

}

template<>
std::vector<int> SimData<std::vector<int> >::data()
{
    if (m_scale == 1 && m_offset == 0)
        return m_data;
    std::vector<int> returnvector;
    returnvector.resize(m_data.size());
    for (uint i=0 ; i<m_data.size() ; i++)
        returnvector[i]=int(ceil(m_data[i]*m_scale + m_offset));
    return std::vector<int>(returnvector.begin(),returnvector.end());
}

template<>
std::string SimData<std::string>::data()
{
    return m_data;
}
