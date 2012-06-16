#ifndef CanASOverUDP_H
#define CanASOverUDP_H

#include "logichandler.h"
#include "communicatorbase.h"
#include "udpreadsocket.h"
#include "udpwritesocket.h"
#include "casprotocol.h"
#include "myassert.h"
#include "canas.h"


class CanASOverUDP : public LogicHandler, public CommunicatorBase
{
public:
    CanASOverUDP(std::ostream& logfile);

    virtual ~CanASOverUDP();

    virtual bool configure();

    virtual bool processOutput(DataContainer<int>&, DataContainer<float>&, DataContainer<double>&,
                               DataContainer<bool>&, DataContainer<std::vector<float> >&,
                               DataContainer<std::vector<int> >&, DataContainer<std::string>&,
                               std::vector<LogicHandler*>, int ticks = 0, double secs = 0.0f, unsigned int maxDataItems = 256);

    virtual bool processInput(DataContainer<int>&, DataContainer<float>&, DataContainer<double>&,
                               DataContainer<bool>&, DataContainer<std::vector<float> >&,
                               DataContainer<std::vector<int> >&, DataContainer<std::string>&,
                               std::vector<LogicHandler*>, int ticks = 0, double secs = 0.0f);

    virtual bool registerDataRefs();

    virtual bool initState();

    virtual bool processState();

    virtual bool processInput(long input);

    virtual bool publishData();

    virtual bool unpublishData();

    virtual float processingFrequency();

    virtual bool writeToStream(ProtocolStreamer*) {return false;}

    virtual bool needsSpecialWrite() {return false;}

    virtual void suspend(bool yes) { m_suspended = yes; }

    //TODO: Look if suspended can be mapped to enabled ??? Goes along with TODO: use logichandler part more efficiently (seperate high.prio callback)
    virtual bool isSuspended() { return m_suspended; }

    virtual std::string name() { return "CanASOverUDP";}

    void flush(unsigned int maxDataItems);

private:

    void inc_msgCode();

    int sendRequest(uint8_t service_code);

    uint8_t messageCode;

    UDPReadSocket* readSock;

    UDPWriteSocket* writeSock;

    ProtocolStreamer* casprotocol;

    bool configured;

    std::ostream& m_logfile;

    float m_last_heared_from_vasfmc;

    bool m_enabled;

    bool m_suspended;
};

#endif // CanASOverUDP_H
