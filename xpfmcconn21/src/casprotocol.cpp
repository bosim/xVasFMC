#include "casprotocol.h"
#include "myassert.h"

#ifdef WIN_32
#include <windows.h>
#else
#include <netinet/in.h>
#endif

void Casprotocol::protocolWrite(DataToSend<int>& data)
{
    protocolWrite(data.id(), data.data(), data.messageCode());
    data.incMessageCode();
}

void Casprotocol::protocolWrite(DataToSend<float>& data)
{
    protocolWrite(data.id(), data.data(), data.messageCode());
    data.incMessageCode();
}

void Casprotocol::protocolWrite(DataToSend<double>& data)
{
    protocolWrite(data.id(), float(data.data()), data.messageCode());
    data.incMessageCode();
}

void Casprotocol::protocolWrite(DataToSend<bool>& data)
{
    protocolWrite(data.id(), data.data(), data.messageCode());
    data.incMessageCode();
}

void Casprotocol::protocolWrite(DataToSend<std::string>& data)
{
    protocolWrite(data.id(), data.data(), data.messageCode());
    data.incMessageCode();
}

void Casprotocol::protocolWrite(DataToSend<std::vector<float> >& data)
{
    for (uint i = 0 ; i < data.data().size() ; i++)
    {
        canAS_t message;
        can_t can;
        can.id = htonl(uint32_t( data.id() ));
        can.id_is_29 = uint8_t(m_id29 ? 1 : 0);
        message.nodeId = uint8_t(m_node_id);
        message.messageCode = data.messageCode();
        data.incMessageCode();
        message.serviceCode = i;
        message.data.flt = data.data().at(i);
        message.data.sLong = htonl(message.data.sLong);
        message.dataType = AS_FLOAT;
        can.dlc = 8;
        can.msg.aero = message;
        m_sendqueue.push(can);
    }
}


void Casprotocol::protocolWrite(uint32_t id, float data, uint8_t message_code)
{
    canAS_t message;
    can_t can;
    can.id = htonl(uint32_t( id ));
    can.id_is_29 = uint8_t(m_id29 ? 1 : 0);
    message.nodeId = uint8_t(m_node_id);
    message.messageCode = message_code;
    message.serviceCode = 0;
    message.data.flt = data;
    message.data.sLong = htonl(message.data.sLong);
    message.dataType = AS_FLOAT;
    can.dlc = 8;
    can.msg.aero = message;
    m_sendqueue.push(can);
}

void Casprotocol::protocolWrite(uint32_t id, int data, uint8_t message_code)
{
    canAS_t message;
    can_t can;
    can.id = htonl(uint32_t( id ));
    can.id_is_29 = uint8_t(m_id29 ? 1 : 0);
    message.nodeId = uint8_t(m_node_id);
    message.messageCode = message_code;
    message.serviceCode = 0;
    message.data.sLong = htonl(int32_t(data));
    message.dataType = AS_LONG;
    can.dlc = 8;
    can.msg.aero = message;
    m_sendqueue.push(can);
}

void Casprotocol::protocolWrite(uint32_t id, bool data, uint8_t message_code)
{
    canAS_t message;
    can_t can;
    can.id = htonl(uint32_t( id ));
    can.id_is_29 = uint8_t(m_id29 ? 1 : 0);
    message.nodeId = uint8_t(m_node_id);
    message.messageCode = message_code;
    message.serviceCode = 0;
    message.data.uChar[0] = uint8_t( data ? 1:0);
    message.dataType = AS_UCHAR;
    can.dlc = 5;
    can.msg.aero = message;
    m_sendqueue.push(can);
}
void Casprotocol::protocolWrite(uint32_t id, std::string data, uint8_t message_code)
{
    int strlen = data.size();
    if (strlen > 0)
    {
        if (strlen > 5)
        {
            m_logfile << "Error, unable to send strings longer than 5 chars with this CANaerospace distribution!" << std::endl;
            MYASSERT(false);
        }
        const char* achar = data.c_str();
        canAS_t message;
        can_t can;
        can.id = htonl(uint32_t( id ));
        can.id_is_29 = uint8_t(m_id29 ? 1 : 0);
        message.nodeId = uint8_t(m_node_id);
        message.messageCode = message_code;
        message.serviceCode = 0;
        message.data.aChar[0] = achar[0];
        if (strlen > 1)
            message.data.aChar[1] = achar[1];
        if (strlen > 2)
            message.data.aChar[2] = achar[2];
        if (strlen > 3)
            message.data.aChar[3] = achar[3];
        if(strlen > 4)
            message.serviceCode = achar[4];
        message.dataType = FP_ACHAR5;
        can.dlc = 4+strlen;
        can.msg.aero = message;
        m_sendqueue.push(can);
        }
}

unsigned int Casprotocol::writeAll()
{
    unsigned int i = 0;
    while (!m_sendqueue.empty())
    {
        can_t can = m_sendqueue.front();
        if( !( m_writer->write(&can, sizeof(can)) == sizeof(can_t)))
            m_logfile << "Not all bytes were sent. This indicates network problems." << std::endl;
        m_sendqueue.pop();
        i++;
    }
    return i;
}

unsigned int Casprotocol::writeMax(unsigned int max)
{
    static unsigned int history_queue_length = 0;
    static unsigned int grown_in_a_row = 0;
    unsigned int i = 0;
    while (!m_sendqueue.empty() && i < max )
    {
        can_t can = m_sendqueue.front();
        if( !( m_writer->write(&can, sizeof(can)) == sizeof(can_t)))
            m_logfile << "Not all bytes were sent. This indicates network problems." << std::endl;
        m_sendqueue.pop();
        i++;
    }
    if (history_queue_length < m_sendqueue.size())
        grown_in_a_row++;
    history_queue_length = m_sendqueue.size();
    if (grown_in_a_row > 5 && m_sendqueue.size()>150)
    {
        m_logfile << "The sendqueue grows too large, skipping old items now." << std::endl;
        while (!m_sendqueue.empty())
            m_sendqueue.pop();
        grown_in_a_row = 0;
    }
    return i;
}
