#ifndef CASPROTOCOL_H
#define CASPROTOCOL_H

#include "protocolstreamer.h"
#include "paketwriter.h"
#include "datatosend.h"
#include "canas.h"
#include <string>
#include <queue>
#include <stdint.h>

class Casprotocol : public ProtocolStreamer
{
public:
    Casprotocol(std::ostream& logfile, PaketWriter* paketwriter, uint8_t node_id = 1, bool id29 = true):
            ProtocolStreamer(logfile),
            m_writer(paketwriter),
            m_node_id(node_id),
            m_id29(id29),
            m_logfile(logfile)
    {}
    virtual ~Casprotocol(){}
    virtual void protocolWrite(DataToSend<int>& data);
    virtual void protocolWrite(DataToSend<float>& data);
    virtual void protocolWrite(DataToSend<double>& data);
    virtual void protocolWrite(DataToSend<bool>& data);
    virtual void protocolWrite(DataToSend<std::string>& data);
    virtual void protocolWrite(DataToSend<std::vector<float> >& data);
    virtual void protocolWrite(uint32_t id, float data, uint8_t message_code);
    virtual void protocolWrite(uint32_t id, int data, uint8_t message_code);
    virtual void protocolWrite(uint32_t id, bool data, uint8_t message_code);
    virtual void protocolWrite(uint32_t id, std::string data, uint8_t message_code);
    virtual unsigned int writeAll();
    virtual unsigned int writeMax(unsigned int max);
    unsigned int lengthOfQueue() {return m_sendqueue.size();}
private:
    PaketWriter* m_writer;
    uint8_t m_node_id;
    bool m_id29;
    std::ostream& m_logfile;
    std::queue<can_t> m_sendqueue;
};

#endif // CASPROTOCOL_H
