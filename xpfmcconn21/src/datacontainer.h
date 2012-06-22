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

#ifndef datacontainer_h
#define datacontainer_h

#include <vector>
#include <utility>
#include <queue>
#include <iostream>

#include "protocolstreamer.h"
#include "datatosend.h"

using std::pair;
using std::queue;

/**
  * Container Class for FlightStatusData, based on a std::vector.
  * Provides functionality to update a whole bunch of data (getting the values from XPlane)
  * and to write id and data-packets to a buffer, sending either all data or only a max number
  * of data that is really is outdated or outtimed. Keeps track on which data has been sent
  * the last call.
  * @version 0.2
  * @author Philipp Muenzel
  * @file datacontainer.h
  */
template <typename T>
class DataContainer : private std::vector<DataToSend<T> > {

 public:

    DataContainer();

    /**
      * @param id please don't hard code ids in this call, but rather use the enum provided in getrefid.h to let vaslib and xpfmcconn share a common database
      * @param readWrite use either RWType::ReadOnly, RWType:WriteOnly or RWType::ReadWrite
      * @param name a descriptive name is useful for debugging purposes
      * @param DataRef here you provide the path to the Xplane DataRef
      * @param prio use either PrioType::Constant, PrioType::Low, PrioType::Middle, or PrioType::High. Data with high priority is updated and send EVERY time you touch the container for updating or sending. Data of non-high priority is only sent when there's a real need for it.
      * @param eps for non-high prio data, you can provide a margin for the change in value that triggers the data to be considered worth sending
      * @param scale:since in some cases Xplane may use different units of measure than vasfmc, you can provide a scale factor that is applied to the value when sending to vasfmc or receiving from vasfmc via network. Please resist from hard-coding the factors here and use the factors provided by the Navcalc-class instead
      * @param offset: if there is a need to offset the value, as e.g. for converting from °F to centigrade, the offset can be provided here and is applied to the value when it is sent or received via network
      */
    void addDataRef(uint32_t id, short readWrite, const std::string& name,
                            const std::string& dataRefIdentifier, unsigned char prio = PrioType::Constant,
                            double eps=0.01, double scale = 1.0, double offset = 0.0, int no_of_items = 0);

    /**
      * remove the datatosend-instance from the vector
      * @param the id as defined by the refids enum
      */
    void removeAtId(uint32_t id);

    /**
      * updata all data in the container to the value in X-Plane
      */
    void updateAll();

    /**
      * update only data with prio==PrioType::High to the value in X-Plane
      */
    void updateHighPrio();

    /**
      * return the value from the instance with the id provided
      * @param id the sending id as defined in refids enum
      */
    T valueAtId(uint32_t id);

    unsigned int writeOutdated(ProtocolStreamer* streamer, int ticks, double secs);

    void outDateAll();

    /**
      * set the dataref with id internally to value data
      * @param id the id as defined in refids enum
      * @param data value to store
      */
    bool setDataRefAtId(uint32_t id, T data);


 private:

    void resetPositionPointer() { m_pos = 0; }

    unsigned int m_counter;

    unsigned int m_pos;

};

template <typename T>
DataContainer<T>::DataContainer():
    std::vector<DataToSend<T> >(),
    m_counter(0),
    m_pos(0)
{
}

template <typename T>
void DataContainer<T>::removeAtId(uint32_t id)
{
    for ( unsigned int i = 0 ; i<this->size() ; i++)
        if ( this->at(i).id() == id )
        {
            this->erase(this->begin()+i);
            m_counter--;
        }
}

template <typename T>
void DataContainer<T>::addDataRef(uint32_t id, short readWrite, const std::string& name,
                            const std::string& dataRefIdentifier, unsigned char prio,
                            double eps, double scale, double offset, int no_of_items)
{
    bool exists = false;
    for (uint i = 0 ; i < this->size() ; i++)
        if (this->at(i).id() == id)
            exists = true;
    if (!exists)
    {
        this->push_back( DataToSend<T>( id, readWrite, name, dataRefIdentifier, prio, eps, scale, offset, no_of_items) );
        m_counter++;
    } else
    {
        m_logfile << "DataRef with id " << id << "name : " << name
                << " already existing, skipping instantiation" << std::endl;
    }
}

template <typename T>
void DataContainer<T>::updateAll()
{
    for ( unsigned int i = 0 ; i < this->size() ; i++ )
        this->at(i).poll();
}

template <typename T>
void DataContainer<T>::updateHighPrio()
{
    for ( unsigned int i = 0; i < this->size() ; i++ )
        if ( this->at(i).prio() == PrioType::High )
            this->at(i).poll();
}

template <typename T>
T DataContainer<T>::valueAtId(uint32_t id)
{
    for ( unsigned int i = 0 ; i<this->size() ; i++ )
        if ( this->at(i).id() == id )
            return this->at(i).data();
    return T(0);
}

template <typename T>
unsigned int DataContainer<T>::writeOutdated(ProtocolStreamer* streamer, int ticks, double secs)
{
    unsigned int no_of_sent_items = 0;
    for ( unsigned int i = 0 ; i < this->size() ; i++ )
        {
            if (this->at(i).needsSending(ticks,secs))
            {
                streamer->protocolWrite(this->at(i));
                this->at(i).markAsSent(ticks, secs);
                no_of_sent_items++;
            }
        }
    return no_of_sent_items;
}

template <typename T>
void DataContainer<T>::outDateAll()
{
    for ( unsigned int i = 0 ; i<this->size() ; i++ )
    {
        this->at(i).makeOutDated();
    }
}

template <typename T>
bool DataContainer<T>::setDataRefAtId(uint32_t id, T data)
{
    for ( unsigned int i = 0 ; i<this->size() ; i++ )
    {
        if ( this->at(i).id() == id) {
            return this->at(i).set(data);
        }
    }
    return false;
}

#endif
