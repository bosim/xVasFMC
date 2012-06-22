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

#ifndef datatosend_h
#define datatosend_h

#include "simdata.h"
#include "priotype.h"
#include "rwtype.h"

#ifndef WIN_32
#include "unistd.h"
#endif


    /**
     *  Simdata that keeps track of when it needs to be synced with outside, e.g. via UDP
     *  @author Philipp Muenzel
     * @version 0.2
     * @file datatosend.h
     */
template <typename T>
class DataToSend: public SimData<T> {

 public:

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
    DataToSend(uint32_t id, short readWrite, const std::string& name, const std::string& dataRefIdentifier,
               unsigned char prio = PrioType::Constant, double eps=0.001,
               double scale = 1.0, double offset = 0.0, unsigned int no_of_items=0);

    /**
     *  @return priority as defined by PrioType
     */
    virtual unsigned char prio() { return m_priority; }


    /**
      * @return id for streaming
      */
    virtual uint32_t id() { return m_id; }


    /**
     *  has the value changed or has time passed longer than priority permits
     * @param int ticks internal tick counter at the moment
     * @param double secs internal seconds counter at the moment
     */
    virtual bool needsSending(int, double);


    /**
     *  reset counters
     * @param int ticks internal tick counter at the moment
     * @param double secs internal seconds counter at the moment
     */
    virtual bool markAsSent(int, double);

    void makeOutDated();

    uint8_t messageCode() { return m_message_code; }

    void incMessageCode() { m_message_code = (m_message_code + 1) % 256;}

 private:

    /**
     *  ticks by last markAsSent()
     */
    int m_ticksLastSent;

    /**
     *  seconds by last markAsSent()-call
     */
    float m_secsLastSent;

    /**
     *  priority mask according to PrioType
     */
    unsigned char m_priority;

    /**
     *  id used for communication to identify the type and data behind
     */
    uint32_t m_id;

    bool m_force_outdated;

    uint8_t m_message_code;

};

template <typename T>
DataToSend<T>::DataToSend(uint32_t id, short readWrite, const std::string& name,
                          const std::string& dataRefIdentifier, unsigned char prio,
                          double eps, double scale, double offset, unsigned int no_of_items):
    SimData<T>(dataRefIdentifier, name, readWrite, eps, scale, offset, no_of_items),
    m_priority(prio),
    m_id(id),
    m_force_outdated(false),
    m_message_code(0)
{
}


template <typename T>
bool DataToSend<T>::needsSending(int, double secs)
{
    double interval;
    //TODO: Check for ticks if necessary
    switch (m_priority)
    {
        case PrioType::High : interval = 0.5;
            break;
        case PrioType::Middle : interval = 1;
            break;
        case PrioType::Low : interval = 3;
            break;
        case PrioType::Constant : interval = 15;
            break;
        default: interval = 15;
    }
    return (this->m_hasChanged || secs - m_secsLastSent > interval || m_force_outdated);
}

template <typename T>
bool DataToSend<T>::markAsSent(int ticks, double secs)
{
    m_ticksLastSent = ticks;
    m_secsLastSent = secs;
    m_force_outdated = false;
    this->m_hasChanged = false;
    return (m_ticksLastSent != 0);
}

template <typename T>
void DataToSend<T>::makeOutDated()
{
    m_force_outdated = true;
}

#endif
