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

#ifndef simdata_h
#define simdata_h

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <math.h>

#include "dataref.h"
#include "rwtype.h"

#include "XPLMDataAccess.h"

#ifndef isnan
#define isnan _isnan
#endif

extern std::fstream m_logfile;

typedef unsigned int uint;

    /**
     * wrapper for access to datarefs published by XP SDK itself (not for plugin-owned data).
     * It wraps the lookup of datarefs, type-safe getting of data, and converting to the
     * needed units of measure by scaling and offsetting values.
     * By creating a flightstatusdata instance, it is bound to a specific dataref
     * specified in the constructor call.
     * @author (c) 2009 by Philipp Muenzel, Technische Universitaet Darmstadt, Department of Mathematics
     * @version 0.2
     * @file simdata.h
     */
template <typename T>
class SimData : public DataRef<T>{

 public:

    /**
      * constructor call which sets up the dataref connection.
      * looks up the datarefs and stores handler locally, also checks for correct type of data and
      * correct read-/writeability
      * @param dataRefIdentifier The identifier as in datarefs.txt as std::string (or implicitly convertable)
      * @param name a descriptive name, mainly for debugging purposes
      * @param readWrite the writeability as a short int as defined in RWType (use this class!)
      * @param eps a treshold value. When the internal data differs from X-Plane provided data by more than eps, the update is triggered by poll
      * @param no_of_items
      * @see rwtype
      */
    SimData(const std::string& dataRefIdentifier, const std::string& name, short readWrite = RWType::ReadOnly,
            double eps = 0.01f, double scale = 1.0, double offset = 0.0, unsigned int no_of_items = 0);

    /**
     * ask X-Plane for the latest value and updated the internal value if they differ more than epsilon
     * @return true if dataref is valid and data could be read
     */
    virtual bool poll();


    /**
     * public access to the internal value, tries to write to XPlane afterwards
     * @param data The value that shall be set internally and written to X-Plane afterwards
     * @return true if dataref is valid and data could be written
     */
    virtual bool set(T data);

    /**
      * public access to the internal value
      * @return the internal value, scale and offset applied (no polling occurs at that moment!)
      */
    virtual T data();


    /**
      * public access to the Name (user-defined) of the SimData instance
      * @return the internal name
      */
    virtual std::string name() {return m_name; }

    /**
      * has the value changed since last poll() by more than epsilon
      * @return
      */
    virtual bool hasChanged() { return m_hasChanged; }

    /**
      * reset the changed flag
      */
    virtual void resetChanged() { m_hasChanged = false; }

 protected:

    /**
     *  internal value was changed since last resetChanged() call
     */
    bool m_hasChanged;

 private:

    /**
     *  lookup the dataref via the SDK and store the opaque handle
     * @return true if dataref was bound and the handler stored
     */
    virtual bool lookUp();


    /**
     *  check if the datatype of the template matches the type of the dataref according to the SDK
     * @return true if datatype matches the type provided by X-Plane SDK
     */
    virtual bool checkDataType();


    /**
     *  check if dataref is writable according to the SDK if m_read_write is not set to write_only
     * @return true if writeability matches the possibilities of X-Plane SDK
     */
    virtual bool checkWriteabilityIsValid();


    /**
     *  update the internally stored value to the actual value from XP
     */
    virtual bool updateValue(T data);


    /**
     *  write the value directly into X-Plane if the dataref is writeable
     * @param data input
     */
    virtual void setValue(T data);

    /**
     *  descriptive name, mainly for debugging purposes (user-defined)
     */
    std::string m_name;

    /**
     *  is the dataref writeable (use RWType !)
     */
    short m_readWrite;

    /**
     *  an update to the internal value occurs, when it differs from the XP value by more than this epsilon
     */
    double m_epsilon;

    /**
     *  scale to apply to the value to put it out in correct units of measure. it is applied when getting the data via data() or setting via set()
     */
    double m_scale;

    /**
     *  offset to apply to the value to put it out in correct units of measure. it is applied when getting the data via data() or setting via set()
     */
    double m_offset;

    unsigned long m_no_of_items;
};


template <typename T>
SimData<T>::SimData(const std::string& dataRefIdentifier, const std::string& name, short readWrite, 
                    double eps, double scale, double offset, unsigned int no_of_items):
DataRef<T>(dataRefIdentifier),
m_name(name),
m_readWrite(readWrite),
m_epsilon(eps),
m_scale(scale),
m_offset(offset),
m_no_of_items(no_of_items)
{
    if (!lookUp())
    {
        m_logfile << "DataRef not found for " << m_name << std::endl;
        exit(1);
    }
    if(!checkDataType())
    {
        m_logfile << "Wrong Datatype for " << m_name << std::endl;
        exit(1);
    }
    if(!checkWriteabilityIsValid())
    {
        m_logfile << m_name << " was declared to be writeable, but XPlane says it is read-only" << std::endl;
        exit(1);
    }
    m_hasChanged = true;
}


template <typename T>
bool SimData<T>::poll()
{
    m_logfile << "Poll called with incorrect type for " << m_name << std::endl;
    return false;
}

// specialized functions defined in .cpp-File
template <>
bool SimData<int>::poll();
template <>
bool SimData<float>::poll();
template <>
bool SimData<double>::poll();
template <>
bool SimData<bool>::poll();
template <>
bool SimData<std::vector<float> >::poll();
template <>
bool SimData<std::vector<int> >::poll();
template <>
bool SimData<std::string>::poll();

// is the same for double and float
template <typename T>
bool SimData<T>::set(T data)
{
    if(m_readWrite == RWType::WriteOnly) return false;
    m_hasChanged = true;
    if (m_scale != 1)
        setValue(static_cast<float>((data-m_offset)/m_scale));
    else
        setValue(data);
    return m_hasChanged;
}
template <>
bool SimData<int>::set(int data);
template <>
bool SimData<bool>::set(bool data);
template <>
bool SimData<std::vector<float> >::set(std::vector<float> data);
template <>
bool SimData<std::vector<int> >::set(std::vector<int> data);
template <>
bool SimData<std::string>::set(std::string data);

template <typename T>
bool SimData<T>::lookUp()
{
    this->m_pDataRef = XPLMFindDataRef(this->m_dataRefIdentifier.c_str());
    if(!this->m_pDataRef || this->m_pDataRef==0)
        return false;
    return true;
}

template <typename T>
bool SimData<T>::checkDataType()
{
    m_logfile << "Error: datatype of SimData<T>-instance unknown" << std::endl;
    return false;
}
template <>
bool SimData<double>::checkDataType();
template <>
bool SimData<float>::checkDataType();
template <>
bool SimData<int>::checkDataType();
template <>
bool SimData<bool>::checkDataType();
template <>
bool SimData<std::vector<float> >::checkDataType();
template <>
bool SimData<std::vector<int> >::checkDataType();
template <>
bool SimData<std::string>::checkDataType();


template <typename T>
bool SimData<T>::checkWriteabilityIsValid()
{
    if(m_readWrite==RWType::WriteOnly || m_readWrite==RWType::ReadWrite)
        return XPLMCanWriteDataRef(this->m_pDataRef);
    return true;
}


template <typename T>
bool SimData<T>::updateValue(T data)
{
    if (isnan(data)) {
        m_logfile << "update value returned nan in " << this->name() << std::endl;
        this->m_data = 0;
        //exit(1);
    }
    if (fabs(this->m_data - data) > m_epsilon) {
        this->m_data = data;
        m_hasChanged = true;
        return (m_hasChanged);
    }
    return true;
}
template <>
bool SimData<int>::updateValue(int data);
template <>
bool SimData<bool>::updateValue(bool data);
template <>
bool SimData<std::vector<float> >::updateValue(std::vector<float> data);
template <>
bool SimData<std::vector<int> >::updateValue(std::vector<int> data);
template <>
bool SimData<std::string>::updateValue(std::string data);

template <typename T>
T SimData<T>::data()
{
    if (isnan(this->m_data)){
        m_logfile << "data() returned nan in " << this->name() << std::endl;
        this->m_data = 0;
        //exit(1);
    }
    if (m_scale == 1 && m_offset == 0)
        return this->m_data;
    return static_cast<float>(this->m_data * m_scale + m_offset);
}
template <>
int SimData<int>::data();
template <>
std::vector<float> SimData<std::vector<float> >::data();
template <>
std::vector<int> SimData<std::vector<int> >::data();
template <>
std::string SimData<std::string>::data();

template <>
void SimData<double>::setValue(double data);
template <>
void SimData<float>::setValue(float data);
template <>
void SimData<int>::setValue(int data);
template <>
void SimData<bool>::setValue(bool data);
template <>
void SimData<std::vector<float> >::setValue(std::vector<float> data);
template <>
void SimData<std::vector<int> >::setValue(std::vector<int> data);
template <>
void SimData<std::string>::setValue(std::string data);

#endif
