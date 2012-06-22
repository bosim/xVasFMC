#ifndef PROTOCOLSTREAMER_H
#define PROTOCOLSTREAMER_H

#include <vector>
#include <string>

#include <stdint.h>

template<typename T>
class DataToSend;

class ProtocolStreamer
{
public:
    ProtocolStreamer(std::ostream&){}
    virtual ~ProtocolStreamer() {}
    virtual void protocolWrite(DataToSend<int>& data) = 0;
    virtual void protocolWrite(DataToSend<float>& data) = 0;
    virtual void protocolWrite(DataToSend<double>& data) = 0;
    virtual void protocolWrite(DataToSend<bool>& data) = 0;
    virtual void protocolWrite(DataToSend<std::string>& data) = 0;
    virtual void protocolWrite(DataToSend<std::vector<float> >& data) = 0;
    virtual void protocolWrite(uint32_t id, float data, uint8_t message_code) = 0;
    virtual void protocolWrite(uint32_t id, int data, uint8_t message_code) = 0;
    virtual void protocolWrite(uint32_t id, bool data, uint8_t message_code) = 0;
    virtual void protocolWrite(uint32_t id, std::string data, uint8_t message_code) = 0;
    virtual unsigned int writeAll() = 0;
    virtual unsigned int writeMax(unsigned int max) = 0;
};

#endif // PROTOCOLSTREAMER_H
