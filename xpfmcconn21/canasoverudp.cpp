#include "canasoverudp.h"
#include "simdata.h"
#include "datacontainer.h"
#include "radionav.h"
#include "fsaccess_xplane_refids.h"
#include "xpapiface_msg.h"
#include "XPLMProcessing.h"

#include "plugin_defines.h"

#ifdef WIN_32
#include <windows.h>
#else
#include <netinet/in.h>
#endif

int32_t getIntFromCan(can_t can)
{
    MYASSERT(can.msg.aero.dataType == AS_LONG);
    return ntohl(can.msg.aero.data.sLong);
}

float getFloatFromCan(can_t can)
{
    MYASSERT(can.msg.aero.dataType == AS_FLOAT);
    can.msg.aero.data.sLong = ntohl(can.msg.aero.data.sLong);
    return can.msg.aero.data.flt;
}

double getDoubleFromCan(can_t can)
{
    MYASSERT(can.msg.aero.dataType == AS_FLOAT);
    can.msg.aero.data.sLong = ntohl(can.msg.aero.data.sLong);
    return double(can.msg.aero.data.flt);
}

bool getBoolFromCan(can_t can)
{
    MYASSERT(can.msg.aero.dataType == AS_UCHAR);
    return (can.msg.aero.data.uChar[0] == 1);
}

std::string getStdStringFomCan(can_t can)
{
    MYASSERT(can.msg.aero.dataType == FP_ACHAR5);
    uint length = can.dlc - 4;
    MYASSERT(length>0);
    char* cstr = new char(length+1);
    for ( uint i = 0 ; i < length && i < 4; i++)
        cstr[i] = can.msg.aero.data.aChar[i];
    if ( length == 5 )
        cstr[4] = can.msg.aero.serviceCode;
    cstr[length] = 0x0;
    return std::string(cstr);
}

unsigned char getUCharFromCan(can_t can, uint8_t position = 0)
{
    MYASSERT(can.msg.aero.dataType == AS_UCHAR || can.msg.aero.dataType == AS_UCHAR4);
    if (position > 0)
    {
        MYASSERT(position < 3);
        MYASSERT(can.msg.aero.dataType == AS_UCHAR4);
    }
    return can.msg.aero.data.uChar[position];
}

int CanASOverUDP::sendRequest(uint8_t service_code)
{
    can_t request;
    request.id = htonl(NSH_CH0_REQ);
    request.dlc = 4;
    request.id_is_29 = PLUGIN_USES_ID29;
    request.msg.aero.nodeId = VASFMC_NODE_ID;
    request.msg.aero.dataType = AS_NODATA;
    request.msg.aero.serviceCode = service_code;
    request.msg.aero.messageCode = 0;

    return writeSock->write(&request,sizeof(can_t));
}

CanASOverUDP::CanASOverUDP(std::ostream& logfile):
        LogicHandler(logfile),
        messageCode(0),
        readSock(new UDPReadSocket()),
        writeSock(new UDPWriteSocket()),
        configured(false),
        m_logfile(logfile),
        m_enabled(true)
{
    casprotocol = new Casprotocol(m_logfile, writeSock, PLUGIN_NODE_ID, PLUGIN_USES_ID29);
}

CanASOverUDP::~CanASOverUDP()
{
    delete readSock;
    readSock = 0;
    delete writeSock;
    writeSock = 0;

}

bool CanASOverUDP::configure() {
    // get writehost and writeport from config file and configure UDP Socket to write
    writeSock->configure(CFG_HOSTADDRESS_DEFAULT,
                         CFG_PORT_FROM_SIM_DEFAULT);

    readSock->configure(CFG_HOSTADDRESS_DEFAULT,
                        CFG_PORT_TO_SIM_DEFAULT);
    configured = true;
    return true;
}

bool CanASOverUDP::processOutput(DataContainer<int>& intData, DataContainer<float>& floatData,
                            DataContainer<double>& doubleData, DataContainer<bool>& boolData,
                            DataContainer<std::vector<float> >& floatvectorData,
                            DataContainer<std::vector<int> >& ,
                            DataContainer<std::string>& stringData, std::vector<LogicHandler*> vct,
                            int ticks, double secs, unsigned int maxDataItems)
{
    if (m_enabled) {
    doubleData.writeOutdated(casprotocol, ticks, secs);
    floatData.writeOutdated(casprotocol, ticks, secs);
    intData.writeOutdated(casprotocol, ticks, secs);
    boolData.writeOutdated(casprotocol, ticks, secs);
    floatvectorData.writeOutdated(casprotocol, ticks, secs);
    stringData.writeOutdated(casprotocol, ticks, secs);
    for ( std::vector<LogicHandler*>::const_iterator it = vct.begin () ; it!= vct.end() ; ++it )
        if ((*it)->needsSpecialWrite())
            (*it)->writeToStream(casprotocol);
    }

    return true;
}

void CanASOverUDP::flush(unsigned int maxDataItems)
{
    casprotocol->writeMax(maxDataItems);
}

bool CanASOverUDP::processInput(DataContainer<int>& intData, DataContainer<float>& floatData,
                           DataContainer<double>& doubleData, DataContainer<bool>& boolData,
                           DataContainer<std::vector<float> >& floatvectorData ,
                           DataContainer<std::vector<int> >& ,
                           DataContainer<std::string>&,
                           std::vector<LogicHandler*> Handlers,
                           int ticks, double secs)
{
    static float last_sent_sts = 0;
    // check if we were connected to vasfmc in the last 15 seconds, if not, send a state transmission service request
    if (m_last_heared_from_vasfmc + 15.0f < secs && last_sent_sts + 15.0f < secs)
    {
        sendRequest(STS);
        last_sent_sts = secs;
    }
    // if we are disconnected from vasfmc (no response to STS in 2 seconds, shutdown sending and suspend all logichandlers
    if (m_last_heared_from_vasfmc + 17.0f < secs && m_enabled)
    {
        m_enabled=false;
        for ( std::vector<LogicHandler*>::const_iterator it = Handlers.begin () ; it!= Handlers.end() ; ++it )
            (*it)->suspend(true);
        m_logfile << "Timeout from vasFMC, going offline" << std::endl;
    }
    LogicHandler* apHandler = 0;
    for ( std::vector<LogicHandler*>::const_iterator it = Handlers.begin () ; it!= Handlers.end() ; ++it )
        if ( (*it)->name() == "APXPlane9Standard")
            apHandler = *it;
    can_t message;
    while ( readSock->read(&message, sizeof(message)) > 0)
    {
        // if we receive a message from vasfmc while being disabled, enabled all handlers and start sending again
        if (!m_enabled)
        {
            for ( std::vector<LogicHandler*>::const_iterator it = Handlers.begin () ; it!= Handlers.end() ; ++it )
                (*it)->suspend(false);
            m_enabled = true;
            m_logfile << "Data from vasFMC again, going online" << std::endl;
        }
        m_last_heared_from_vasfmc = secs;
        float f;
        int i;
        bool b;
        std::vector<float> vf;
        uint32_t id = ntohl(message.id);
        if(id == RSRVD) continue;
        if (id == NSH_CH0_REQ )
        {
            // we got a node service request. process this and then ignore the rest of the loop.
            // are we adressed directly or is it a broadcast then process it
            if (message.msg.aero.nodeId == 0 || message.msg.aero.nodeId == PLUGIN_NODE_ID)
            {
                switch (message.msg.aero.serviceCode)
                {
                    case 0: // handle IDS
                        can_t response;
                        response.id = htonl(NSH_CH0_RES);
                        response.dlc = 8;
                        response.id_is_29 = PLUGIN_USES_ID29;
                        response.msg.aero.nodeId = PLUGIN_NODE_ID;
                        response.msg.aero.dataType = AS_UCHAR4;
                        response.msg.aero.serviceCode = 0;
                        response.msg.aero.messageCode = message.msg.aero.messageCode;
                        response.msg.aero.data.uChar[0] = PLUGIN_HARDWARE_REVISION;
                        response.msg.aero.data.uChar[1] = PLUGIN_SOFTWARE_REVISION;
                        response.msg.aero.data.uChar[2] = DISTRIBUTION_FP;
                        response.msg.aero.data.uChar[3] = HEADER_TYPE_CANAS;
                        writeSock->write(&response, sizeof(can_t));
                        break;
                    case 7: // hande STS
                        intData.outDateAll();
                        floatData.outDateAll();
                        doubleData.outDateAll();
                        boolData.outDateAll();
                        floatvectorData.outDateAll();
                        break;
                    default: m_logfile << "ERROR: Cannot handle Node Service Request with service code " << message.msg.aero.serviceCode << std::endl;
                }
                // requests handled, no further processing necessary, continue loop immediately to fetch fresh data
                continue;
            } else
            {
                // we are not adressed, continue the loop immediately to fetch fresh data
                continue;
            }
        }
        switch( id )
            {
            case ZTIME: f = getFloatFromCan(message); if(!floatData.setDataRefAtId(ZTIME,f)) m_logfile << "Failure to write UTC time to " << f << std::endl; break;
            case ZDATE: i = getIntFromCan(message); if(!intData.setDataRefAtId(ZDATE,i-1)) m_logfile << "Failure to write date to " << i << std::endl; break;
            case APHDG: f = getFloatFromCan(message); if(!floatData.setDataRefAtId(APHDG,f)) m_logfile << "Failure to write APHDG to" << f << std::endl; break;
            case APALT: f = getFloatFromCan(message); if(!floatData.setDataRefAtId(APALT,f)) m_logfile << "Failure to write APALT to" << f << std::endl; break;
            case APSPD: f = getFloatFromCan(message); if(!floatData.setDataRefAtId(APSPD,f)) m_logfile << "Failure to write APSPD to" << f << std::endl; break;
            case APSPDMACH: b = getBoolFromCan(message); if(!boolData.setDataRefAtId(APSPDMACH,b)) m_logfile << "Failure to write APSPDMACH" << std::endl; break;
            case APVS: f = getFloatFromCan(message); if(!floatData.setDataRefAtId(APVS,f)) m_logfile << "Failure to write APVS to " << f << std::endl; break;
            case FDON: i = getIntFromCan(message); if(!intData.setDataRefAtId(FDON,i)) m_logfile << "Failure to write FDON" << std::endl; break;
            case NAV1: i = getIntFromCan(message); if(!intData.setDataRefAtId(NAV1,i)) m_logfile << "Failure to write NAV1 to" << i << std::endl; break;
            case NAV2: i = getIntFromCan(message); if(!intData.setDataRefAtId(NAV2,i)) m_logfile << "Failure to write NAV2 to" << i << std::endl; break;
            case ADF1: i = getIntFromCan(message); if(!intData.setDataRefAtId(ADF1,i)) m_logfile << "Failure to write ADF1 to" << i << std::endl; break;
            case ADF2: i = getIntFromCan(message); if(!intData.setDataRefAtId(ADF2,i)) m_logfile << "Failure to write ADF2 to" << i << std::endl; break;
            case OBS1: f = getFloatFromCan(message); if(!floatData.setDataRefAtId(OBS1,f)) m_logfile << "Failure to write OBS1 to" << f << std::endl; break;
            case OBS2: f = getFloatFromCan(message); if(!floatData.setDataRefAtId(OBS2,f)) m_logfile << "Failure to write OBS2 to" << f << std::endl; break;
            case ALTSET:f = getFloatFromCan(message); if(!floatData.setDataRefAtId(ALTSET,f)) m_logfile << "Failure to write ALTSET to" << f << std::endl; break;
            case FLAPRQST:f = getFloatFromCan(message);if(!floatData.setDataRefAtId(FLAPRQST,f)) m_logfile << "Failure to write FLAPRQST to" << f << std::endl; break;
           /* case ENGTHRO: static floatvec engthro;
                getFloatVectorFromCan(message, engthro);
                for( uint k=0 ; k<engthro.size ; k++ )
                {
                    f = engthro.value[k];
                    vf.push_back(f);
                }
                if (!floatvectorData.setDataRefAtId(ENGTHRO,vf)) printf("Failure to write ENGTHR to%f\n",vf[0]);
                break;*/
            case THROVRDPOS: f = getFloatFromCan(message); boolData.setDataRefAtId(THROVRD,true); if(!floatData.setDataRefAtId(THROVRDPOS,f)) printf("Failure to write THROVRDPOS to%f\n",f); break;
            case APSTATE: i = getIntFromCan(message);
                if (apHandler != 0) {
                    int msg = 0;
                    switch (i) {
                                            case 0x4:   msg = XPAPIFACE_COMM_HDG_HOLD_ON;    /*printf("HDG ON\n");*/    break;
                                            case ~0x4:  msg = XPAPIFACE_COMM_HDG_HOLD_OFF;   /*printf("HDG OFF\n");*/   break;
                                            case 0x8:   msg = XPAPIFACE_COMM_ALT_HOLD_ON;    /*printf("ALT ON\n");*/    break;
                                            case ~0x8:  msg = XPAPIFACE_COMM_ALT_HOLD_OFF;   /*printf("ALT OFF\n");*/   break;
                                            case 0xF :  msg = XPAPIFACE_COMM_SPEED_HOLD_ON;  /*printf("SPD ON\n");*/    break;
                                            case ~0xF:  msg = XPAPIFACE_COMM_SPEED_HOLD_OFF; /*printf("SPD OFF\n");*/   break;
                                            case 0x20:  msg = XPAPIFACE_COMM_MACH_HOLD_ON;   /*printf("MACH ON\n");*/   break;
                                            case ~0x20: msg = XPAPIFACE_COMM_MACH_HOLD_OFF;  /*printf("MACH OFF\n");*/  break;
                                            case 0x1000: msg = XPAPIFACE_COMM_AT_ARM_ON;     /*printf("AT ARM\n");*/    break;
                                            case ~0x1000:msg = XPAPIFACE_COMM_AT_ARM_OFF;    /*printf("AT DISARM\n");*/ break;
                                            case 0x80:  msg = XPAPIFACE_COMM_NAV1_HOLD_ON;   /*printf("NAV1 ON\n");*/   break;
                                            case ~0x80: msg = XPAPIFACE_COMM_NAV1_HOLD_OFF;  /*printf("NAV1 OFF\n");*/  break;
                                            case 0x200: msg = XPAPIFACE_COMM_APP_HOLD_ON;    /*printf("APP ON\n");*/    break;
                                            case ~0x200:msg = XPAPIFACE_COMM_APP_HOLD_OFF;   /*printf("APP OFF\n");*/   break;
                                            default : m_logfile << "Unknown autopilot command, don't know how to handle " << i  << std::endl; break;
                                        }
                    apHandler->processInput(msg);
                } else {
                    m_logfile << "ERROR in Autopilot communication, command discarded, unable!" << std::endl;
                }
                break;
            case THROVRD: b = getBoolFromCan(message);if(!boolData.setDataRefAtId(THROVRD,b)) m_logfile << "Failure to write THROVRD" << std::endl; if(!b) floatData.setDataRefAtId(THROVRDPOS,-2.0f);break;
            case PITCHOVRDPOS: f = getFloatFromCan(message); break;
            default: m_logfile << "ERROR: Got unrecognized ID: of " << id << std::endl; return false;
            }
    }
    return true;
}

bool CanASOverUDP::registerDataRefs()
{
    // No refs to be registered here
    return true;
}

bool CanASOverUDP::initState()
{
    if(!configured)
    {
        m_logfile << "UDP Sockets have not been configured. Stopping" << std::endl;
        MYASSERT(false);
    }
    return true;
}

bool CanASOverUDP::processState()
{
    return true;
}

bool CanASOverUDP::processInput(long)
{
    // we don't take any input at the moment
    return true;
}

bool CanASOverUDP::publishData()
{
    // there is nothing to publish here
    return true;
}

bool CanASOverUDP::unpublishData()
{
    // there is nothing to un-publish here
    return true;
}

float CanASOverUDP::processingFrequency()
{
    return -7;
}

void CanASOverUDP::inc_msgCode()
{
    if ( messageCode > 254)
        messageCode = 0;
    else
        ++messageCode;
}

