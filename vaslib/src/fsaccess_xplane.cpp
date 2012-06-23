/*! \file    xplane.fsaccess.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "fsaccess_xplane.h"
#include "fsaccess_xplane_defines.h"
#include "fsaccess_xplane_refids.h"
#include <queue>


void checkMessageCode(can_t received)
{
    static uint8_t messageCodes[500];
    uint32_t id = ntohl(received.id);
    uint8_t stored = messageCodes[id];
    if ((stored + 1)%256 != received.msg.aero.messageCode)
        Logger::log(QString("Packet loss id %1 from packet %2 to %3")
                    .arg(id).arg(stored).arg(received.msg.aero.messageCode));
    messageCodes[id] = received.msg.aero.messageCode;
}

float barberpole(float vno, float mmo, float sos)
{
    // get the KIAS equivalent of Mmo at the current speed of sound
    float current_mmo_kts = mmo*sos;
    // return the limiting speed
    return qMin(current_mmo_kts, vno);
}

float hardDetentsPercent(float value, float border){
    if (value < border)
        return 0;
    else if (value > 100 - border)
        return 100;
    else
        return value;
}

uint8_t getIndexFromCan(can_t can)
{
    return can.msg.aero.serviceCode;
}

int32_t getIntFromCan(can_t can)
{
    MYASSERT(can.msg.aero.dataType == AS_LONG);
    checkMessageCode(can);
    return ntohl(can.msg.aero.data.sLong);
}

float getFloatFromCan(can_t can)
{
    MYASSERT(can.msg.aero.dataType == AS_FLOAT);
    checkMessageCode(can);
    can.msg.aero.data.sLong = ntohl(can.msg.aero.data.sLong);
    return can.msg.aero.data.flt;
}

double getDoubleFromCan(can_t can)
{
    MYASSERT(can.msg.aero.dataType == AS_FLOAT);
    checkMessageCode(can);
    can.msg.aero.data.sLong = ntohl(can.msg.aero.data.sLong);
    return double(can.msg.aero.data.flt);
}

bool getBoolFromCan(can_t can)
{
    MYASSERT(can.msg.aero.dataType == AS_UCHAR);
    checkMessageCode(can);
    return (can.msg.aero.data.uChar[0] == 1);
}

QString getQStringFomCan(can_t can)
{
    MYASSERT(can.msg.aero.dataType == FP_ACHAR5);
    checkMessageCode(can);
    uint length = can.dlc - 4;
    MYASSERT(length>0);
    char* cstr = new char(length+1);
    for ( uint i = 0 ; i < length && i < 4; i++)
        cstr[i] = can.msg.aero.data.aChar[i];
    if ( length == 5 )
        cstr[4] = can.msg.aero.serviceCode;
    cstr[length] = 0x0;
    return QString(cstr);
}

unsigned char getUCharFromCan(can_t can, uint8_t position = 0)
{
    MYASSERT(can.msg.aero.dataType == AS_UCHAR || can.msg.aero.dataType == AS_UCHAR4);
    checkMessageCode(can);
    if (position > 0)
    {
        MYASSERT(position < 4);
        MYASSERT(can.msg.aero.dataType == AS_UCHAR4);
    }
    return can.msg.aero.data.uChar[position];
}

FSAccessXPlane::FSAccessXPlane(ConfigWidgetProvider* config_widget_provider,
                               const QString& cfg_file,
                               FlightStatus* flightstatus) :
FSAccess(flightstatus),
m_cfg(cfg_file),
apstate(0),
m_message_code(0)
{
    MYASSERT(config_widget_provider != 0);

    m_multicastActive = false;

    // setup config
    m_cfg.setValue(CFG_HOSTADDRESS, "239.40.41.42");
    m_cfg.setValue(CFG_PORT_FROM_SIM, 50707);
    m_cfg.setValue(CFG_PORT_TO_SIM, 63703);
    m_cfg.loadfromFile();
    m_cfg.saveToFile();
    config_widget_provider->registerConfigWidget("XPLANE Access", &m_cfg);

    //TODO    MYASSERT(connect(&m_cfg, SIGNAL(signalChanged()), this, SLOT(slotConfigChanged())));

    // init the read socket

    m_read_socketdevice = new QUdpSocket;
    MYASSERT(m_read_socketdevice);
    QHostAddress addr;
    addr.setAddress("0.0.0.0");
    MYASSERT(m_read_socketdevice->bind(addr,m_cfg.getIntValue(CFG_PORT_FROM_SIM), QUdpSocket::ShareAddress));
    uint32_t address = inet_addr ( m_cfg.getValue(CFG_HOSTADDRESS).toLatin1().constData() );

    // If the hostaddress given is a valid multicast group, join it.
    // For Windos this must happen after the bind to the socket.
    if ( (ntohl(address) >= 0xe0000000) && (ntohl(address) <= 0xefffffff) )
    {
        Logger::log("Using multicast");
//        struct ip_mreq mreq;
        m_multicastAddr.imr_multiaddr.s_addr = address;
        m_multicastAddr.imr_interface.s_addr = INADDR_ANY;
        int r = ::setsockopt(m_read_socketdevice->socketDescriptor(), IPPROTO_IP, IP_ADD_MEMBERSHIP,
                            (const char *)&m_multicastAddr, sizeof(m_multicastAddr));
        qDebug("setsockopt returned %d", r);
        
        if ( r == 0 )
        {
            m_multicastActive = true;
        }
        
    }
    MYASSERT(connect(m_read_socketdevice, SIGNAL(readyRead()), this, SLOT(slotSocketRead())));

    // init the write socket

    MYASSERT(m_write_hostaddress.setAddress(m_cfg.getValue(CFG_HOSTADDRESS)));
    m_writeport = m_cfg.getIntValue(CFG_PORT_TO_SIM);

    m_write_socketdevice = new QUdpSocket;
    MYASSERT(m_write_socketdevice);

    // init read timeout

    MYASSERT(connect(&m_read_timout_timer, SIGNAL(timeout()), this, SLOT(slotReadTimeout())));
    m_read_timout_timer.start(1000);
}

/////////////////////////////////////////////////////////////////////////////

FSAccessXPlane::~FSAccessXPlane()
{
    m_cfg.saveToFile();

	if ( m_multicastActive )
	{
        ::setsockopt(m_read_socketdevice->socketDescriptor(), IPPROTO_IP, IP_DROP_MEMBERSHIP,
                            (const char *)&m_multicastAddr, sizeof(m_multicastAddr));
	}


    delete m_read_socketdevice;
    delete m_write_socketdevice;
}

template<>
bool FSAccessXPlane::sendValue<int>(int id, int value)
{
    can_t can;
    canAS_t aero;
    can.id = htonl(id);
    can.dlc = 8;
    can.id_is_29 = PLUGIN_USES_ID29;
    aero.nodeId = VASFMC_NODE_ID;
    aero.serviceCode = 0;
    aero.messageCode = m_message_code;
    aero.dataType = AS_LONG;
    aero.data.sLong = htonl(value);
    can.msg.aero = aero;
    inc_msgCode();
    long sent_bytes = m_write_socketdevice->writeDatagram((char*)&can, sizeof(can), m_write_hostaddress, m_writeport);
    m_write_socketdevice->flush();

    return  sent_bytes != -1;
}
template<>
bool FSAccessXPlane::sendValue<float>(int id, float value)
{
    can_t can;
    canAS_t aero;
    can.id = htonl(id);
    can.dlc = 8;
    can.id_is_29 = PLUGIN_USES_ID29;
    aero.nodeId = VASFMC_NODE_ID;
    aero.serviceCode = 0;
    aero.messageCode = m_message_code;
    aero.dataType = AS_FLOAT;
    aero.data.flt = value;
    aero.data.sLong = htonl(aero.data.sLong);
    can.msg.aero = aero;
    inc_msgCode();
    long sent_bytes = m_write_socketdevice->writeDatagram((char*)&can, sizeof(can), m_write_hostaddress, m_writeport);
    m_write_socketdevice->flush();

    return  sent_bytes != -1;
}
template<>
bool FSAccessXPlane::sendValue<bool>(int id, bool value)
{
    can_t can;
    canAS_t aero;
    can.id = htonl(id);
    can.dlc = 5;
    can.id_is_29 = PLUGIN_USES_ID29;
    aero.nodeId = VASFMC_NODE_ID;
    aero.serviceCode = 0;
    aero.messageCode = m_message_code;
    aero.dataType = AS_UCHAR;
    aero.data.uChar[0] = value?1:0;
    can.msg.aero = aero;
    inc_msgCode();
    long sent_bytes = m_write_socketdevice->writeDatagram((char*)&can, sizeof(can), m_write_hostaddress, m_writeport);
    m_write_socketdevice->flush();

    return  sent_bytes != -1;
}

template<>
bool FSAccessXPlane::sendValue<std::vector<float> >(int id, std::vector<float> value)
{
    std::queue<can_t> sendqueue;
    uint size = value.size();
    canAS_t message;
    can_t can;
    for (uint i = 0 ; i < size ; i++ )
    {
        can.id = htonl(uint32_t(id));
        can.id_is_29 = PLUGIN_USES_ID29;
        message.nodeId = PLUGIN_NODE_ID;
        message.messageCode = m_message_code;
        message.serviceCode = i;
        message.data.flt = value[i];
        message.data.sLong = htonl(message.data.sLong);
        message.dataType = AS_FLOAT;
        can.dlc = 8;
        can.msg.aero = message;
        sendqueue.push(can);
        inc_msgCode();
    }
    bool success = true;
    while(!sendqueue.empty())
    {
        can_t item = sendqueue.front();
        long sent_bytes = m_write_socketdevice->writeDatagram((char*)&item, sizeof(item), m_write_hostaddress, m_writeport);
        m_write_socketdevice->flush();
        if (sent_bytes < 0) success = false;
        sendqueue.pop();
    }
    return success;
}

int FSAccessXPlane::sendRequest(uint8_t service_code)
{
    MYASSERT(service_code == IDS || service_code == STS);
    can_t request;
    request.id = htonl(NSH_CH0_REQ);
    request.dlc = 4;
    request.id_is_29 = PLUGIN_USES_ID29;
    request.msg.aero.nodeId = PLUGIN_NODE_ID;
    request.msg.aero.dataType = AS_NODATA;
    request.msg.aero.serviceCode = service_code;
    request.msg.aero.messageCode = 0;
    long sent_bytes = m_write_socketdevice->writeDatagram((char*)&request, sizeof(can_t), m_write_hostaddress, m_writeport);
    m_write_socketdevice->flush();

    return sent_bytes != -1;
}
/////////////////////////////////////////////////////////////////////////////

bool FSAccessXPlane::setNavFrequency(int freq, uint nav_index)
{
    if(!m_flightstatus->isValid()) return false;
    if (nav_index == 0)
        return sendValue(NAV1,freq);
    else if (nav_index == 1)
        return sendValue(NAV2,freq);
    else
        return false;
}

bool FSAccessXPlane::setAdfFrequency(int freq, uint adf_index)
{
    if(!m_flightstatus->isValid()) return false;
    if (adf_index == 0)
        return sendValue(ADF1,freq);
    else if (adf_index == 1)
        return sendValue(ADF2,freq);
    else
        return false;
}

bool FSAccessXPlane::setNavOBS(int degrees, uint nav_index)
{
    if(!m_flightstatus->isValid()) return false;
    if (nav_index == 0)
        return sendValue(OBS1,(float)degrees);
    else if (nav_index == 1)
        return sendValue(OBS2,(float)degrees);
    else
        return false;
}

bool FSAccessXPlane::setFDOnOff(bool on)
{
    if (!m_flightstatus->isValid()) return false;

    //m_flightstatus->fd_active = on;
    return sendValue(FDON,on?1:0);
}

bool FSAccessXPlane::setAPOnOff(bool on)
{
    if (!m_flightstatus->isValid()) return false;

    //m_flightstatus->ap_enabled = on;
    return sendValue(FDON,on?2:1);
}

bool FSAccessXPlane::setAPHeading(double heading)
{
    if (!m_flightstatus->isValid()) return false;

    m_flightstatus->setAPHdgInternal(Navcalc::round(Navcalc::trimHeading(heading)));
    return  sendValue(APHDG,(float)heading);
}

bool FSAccessXPlane::setAPAlt(unsigned int alt)
{
    if (!m_flightstatus->isValid()) return false;

    m_flightstatus->setAPAltInternal(alt);
    return sendValue(APALT,(float)alt);;
}

bool FSAccessXPlane::setAPAirspeed(unsigned short speed_kts)
{
    if (!m_flightstatus->isValid()) return false;

    sendValue(APSPDMACH,false);
    m_flightstatus->setAPSpdInternal(speed_kts);
    return  sendValue(APSPD,(float)speed_kts);
}

bool FSAccessXPlane::setAPMach(double mach)
{
    if (!m_flightstatus->isValid()) return false;

    sendValue(APSPDMACH,true);
    m_flightstatus->setAPMachInternal(mach);
    return sendValue(APSPD,(float)mach);
}

bool FSAccessXPlane::setAPHeadingHold(bool on)
{
    if (!m_flightstatus->isValid()) return false;
    //m_flightstatus->ap_hdg_lock=on;
    apstate = (on)? 0x4 : ~0x4;
    Logger::log(QString("HDG Command: %1, bool on%2").arg(apstate).arg(on));
    return sendValue(APSTATE, apstate);
}

bool FSAccessXPlane::setAPAltHold(bool on)
{
    if (!m_flightstatus->isValid()) return false;
    //m_flightstatus->ap_alt_lock=on;
    apstate = (on)? 0x8 : ~0x8;
    return sendValue(APSTATE,apstate);
}

bool FSAccessXPlane::setAutothrustSpeedHold(bool on)
{
    if (!m_flightstatus->isValid()) return false;
    apstate = (on)? 0xF :~0xF;
    return sendValue(APSTATE,apstate);
}

bool FSAccessXPlane::setAutothrustMachHold(bool on)
{
    if (!m_flightstatus->isValid()) return false;
    apstate = on ? 0x20: ~0x20;
    return sendValue(APSTATE,apstate);
}

bool FSAccessXPlane::setAutothrustArm(bool on)
{
    if (!m_flightstatus->isValid()) return false;
    apstate = on ? 0x1000: ~0x1000;
    return sendValue(APSTATE,apstate);
}

bool FSAccessXPlane::setNAV1Arm(bool on)
{
    if (!m_flightstatus->isValid()) return false;
    apstate = on ? 0x80: ~0x80;
    return sendValue(APSTATE,apstate);
}

bool FSAccessXPlane::setAPPArm(bool on)
{
    if (!m_flightstatus->isValid()) return false;
    apstate = on ? 0x200: ~0x200;
    return sendValue(APSTATE,apstate);
}


bool FSAccessXPlane::setAPVs(int vs_ft_min)
{
    if (!m_flightstatus->isValid()) return false;
    if (vs_ft_min < -9000) vs_ft_min = -9000;
    if (vs_ft_min > 9000) vs_ft_min = 9000;
    vs_ft_min = vs_ft_min / 100 * 100;
    m_flightstatus->setAPVsInternal(vs_ft_min);
    return sendValue(APVS,(float)vs_ft_min);
}

bool FSAccessXPlane::setAltimeterHpa(const double& hpa)
{
    if (!m_flightstatus->isValid()) return false;

    m_flightstatus->setAltPressureSettingHpaInternal(hpa);
    return sendValue(ALTSET,float(hpa));
}

bool FSAccessXPlane::setUTCTime(const QTime& utc_time)
{
    Logger::log(QString("FSAccessXplane::setUTCTime: %1").arg(utc_time.toString()));
    int secsFromHours = utc_time.hour()*3600;
    int secsFromMinutes = utc_time.minute()*60;
    float secs = secsFromHours + secsFromMinutes + utc_time.second();
    return sendValue(ZTIME,secs);
}

bool FSAccessXPlane::setUTCDate(const QDate& utc_date)
{
    Logger::log(QString("FSAccessXplane::setUTCDate: %1").arg(utc_date.toString()));
    return sendValue(ZDATE,utc_date.dayOfYear());
}

bool FSAccessXPlane::setFlaps(uint notch)
{
    if (!m_flightstatus->isValid()) return false;
    if (notch >= m_flightstatus->flaps_lever_notch_count) return false;
    float value = m_flaps_inc_per_notch * notch;
    return sendValue(FLAPRQST,value);
}

bool FSAccessXPlane::setThrottle(double percent)
{
    if (!m_flightstatus->isValid()) return false;
    //std::vector<float> throttle_settings;
    //for (int i=0; i < m_flightstatus->nr_of_engines; i++)
    //    throttle_settings.push_back(percent/100.0f);
    return sendValue(THROVRDPOS, (float)percent/100.0f);
}

bool FSAccessXPlane::freeThrottleAxes()
{
    return sendValue(THROVRD,(bool)false);
}

bool FSAccessXPlane::setAileron(double percent)
{
    if (!m_flightstatus->isValid()) return false;
    return sendValue(ROLLOVRDPOS, (float)percent);
}

bool FSAccessXPlane::setElevator(double percent)
{
    if (!m_flightstatus->isValid()) return false;
    return sendValue(PITCHOVRDPOS, (float)percent);
}

bool FSAccessXPlane::setReadFp() {
    if (!m_flightstatus->isValid()) return false;
    return sendValue(READFP, 1);
}

/////////////////////////////////////////////////////////////////////////////

void FSAccessXPlane::slotSocketRead()
{
    static bool was_ever_connected = false;
    static bool sent_request = false;
    static int count_wait_response = 0;
    if (!was_ever_connected && !sent_request) {
            sendRequest(IDS);
            sent_request = true;
            Logger::log("Waiting for plugin to identify itself");
    }
    while(m_read_socketdevice->hasPendingDatagrams())
    {
        if (!was_ever_connected && sent_request && count_wait_response >= 2000)
        {
            QMessageBox::critical(0, "PLUGIN DOESN'T IDENTIFY ITSELF",
                                  QString("X-Plane Plugin does not respond correctly. Probably you use a too old plugin. Please download plugin revision %1").arg(PLUGIN_SOFTWARE_REVISION));
            qFatal("X-Plane Plugin does not respond correctly. Probably you use a too old plugin. Please download plugin revision %i!", PLUGIN_SOFTWARE_REVISION);
        }
        else {
        if (!m_read_timout_timer.isActive())
        {
            Logger::log("FSAccessXPlane:slotSocketRead: got data");
            // ask X-Plane plugin to transmit all its can messages
            sendRequest(STS);
        }
        m_read_timout_timer.start(1000);

        QByteArray buffer;
        buffer.resize(m_read_socketdevice->pendingDatagramSize());
        can_t canmsg;
        long read_bytes = m_read_socketdevice->readDatagram((char*)&canmsg, buffer.size());
        if (!(buffer.size() == sizeof(can_t)))
        {
            Logger::log("Wrong buffer size. Network problems or incompatible plugin ?");
            return;
        }

        if (read_bytes <= 0)
        {
            Logger::log(QString("FSAccessXPlane:slotSocketRead: ERROR: no data read from socket: %1").arg(read_bytes));
            return;
        }

        uint32_t id = ntohl(canmsg.id);

        static bool ap_spd_is_mach = false;
        static float true_alt_ft = 0;
        static float kias_maximum_operating, mach_maximum_operating, speed_of_sound;
        bool b;
        float f;
        double d;
        int i;
        static float flapNotches[10]={0,0,0,0,0,0,0,0,0,0};

        static QString ndb1Id, ndb2Id, vor1Id, vor2Id;
        static float adf1_bearing, adf2_bearing, nav1_dme, nav2_dme, ndb1Lat, ndb1Lon, ndb2Lat, ndb2Lon, vor1Lat, vor1Lon, vor2Lat, vor2Lon;
        static int adf1_freq, adf2_freq, nav1_flg, nav2_flg, vor1LocCrs, vor2LocCrs;
        static bool n1hasDME, n2hasDME, vor1hasLOC, vor2hasLOC;
        static bool isInOverride = false;
        static int year = QDate::currentDate().year();


            if(id == RSRVD) break;
            if(id == NSH_CH0_RES) {
                uint8_t plugin_hardware_revision = getUCharFromCan(canmsg, 0);
                uint8_t plugin_software_revision = getUCharFromCan(canmsg, 1);
                uint8_t plugin_identifier_distribution = getUCharFromCan(canmsg, 2);
                uint8_t plugin_header_type = getUCharFromCan(canmsg, 3);
                if (plugin_hardware_revision == PLUGIN_HARDWARE_REVISION &&
                    plugin_software_revision == PLUGIN_SOFTWARE_REVISION &&
                    plugin_identifier_distribution == DISTRIBUTION_FP &&
                    plugin_header_type == HEADER_TYPE_CANAS)
                {
                    was_ever_connected = true;
                    sendRequest(STS);
                    Logger::log("Plugin version is compatible, vasFMC is connected to X-Plane now");
                } else
                {
                    QMessageBox::critical(0, "PLUGIN INCOMPATIBLE",
                                          QString("X-Plane Plugin is incompatible to this version of vasFMC. Please download plugin revision %1").arg(PLUGIN_SOFTWARE_REVISION));
                    qFatal("X-Plane Plugin is incompatible to this version of vasFMC. Please download plugin revision %i!", PLUGIN_SOFTWARE_REVISION);
                }
                continue;
            } else if (!was_ever_connected && sent_request)
            {
                count_wait_response++;
                continue;
            }
            switch(id)
            {
            case NSH_CH0_REQ: // Node service request, look if we are affected
                if (canmsg.msg.aero.nodeId == 0 || canmsg.msg.aero.nodeId == VASFMC_NODE_ID)
                {
                    switch (canmsg.msg.aero.serviceCode)
                    {
                    case STS: // handle STS by sending some keep-alive packet
                            // TODO: state vasfmcs messages
                        Logger::log("STS request from X-Plane plugin");
                        sendValue(RSRVD,(int)0);
                    break;
                    default: Logger::log(QString("Service request with service code %1 could not be handled").arg(canmsg.msg.aero.serviceCode));
                    }
                }
                break;
            case PITCHOVRDPOS: f = getFloatFromCan(canmsg); //m_flightstatus->elevator_percent = f;
                break;
            case ROLLOVRDPOS: f = getFloatFromCan(canmsg); //m_flightstatus->aileron_percent = f;
                break;
            case PITCHINPUT: f = getFloatFromCan(canmsg); //m_flightstatus->elevator_input_percent = f;
                break;
            case ROLLINPUT: f = getFloatFromCan(canmsg); //m_flightstatus->aileron_input_percent = f;
                break;
            case ROLLOVRD: b = getBoolFromCan(canmsg); break;
            case PITCHOVRD: b = getBoolFromCan(canmsg); break;
            case FLAPS: f = getFloatFromCan(canmsg);
                m_flightstatus->flaps_percent_left = hardDetentsPercent(f,7);
                m_flightstatus->flaps_percent_right = hardDetentsPercent(f,7);
                break;
            case FLAPRQST: f = getFloatFromCan(canmsg);
                if (m_flightstatus->flaps_lever_notch_count >0)
                    m_flightstatus->current_flap_lever_notch = Navcalc::round(f/m_flaps_inc_per_notch);
                break;
            case FLAPDET : i = getIntFromCan(canmsg); m_flightstatus->flaps_lever_notch_count = i+1; m_flaps_inc_per_notch = 100 / i; break;
            case FLAPDETPOS:
                if (getIndexFromCan(canmsg) < m_flightstatus->flaps_lever_notch_count)
                    flapNotches[getIndexFromCan(canmsg)] = getFloatFromCan(canmsg);
                if (m_flightstatus->current_flap_lever_notch < m_flightstatus->flaps_lever_notch_count)
                    m_flightstatus->flaps_degrees = flapNotches[m_flightstatus->current_flap_lever_notch];
                break;
            case SPDBRK: f = getFloatFromCan(canmsg);
                if (f>=0) m_flightstatus->spoiler_lever_percent = f*100;
                m_flightstatus->spoilers_armed = (f==-0.5f);
                break;
            case THROVRDPOS: f = getFloatFromCan(canmsg); break;
            case THROVRD: b = getBoolFromCan(canmsg); isInOverride=b; break;
                //TODO handle m_separate_throttle_lever_mode
            case THRAXIS: f = getFloatFromCan(canmsg); m_flightstatus->setAllThrottleLeversInputPercent(f);break;
            case ZTIME: f = getFloatFromCan(canmsg); m_flightstatus->fs_utc_time = QTime().addSecs(int(floor(f))); break;
            case ZDATE: i = getIntFromCan(canmsg); m_flightstatus->fs_utc_date = QDate(year,1,1).addDays(i); break;
            case V_S0: f = getFloatFromCan(canmsg); m_flightstatus->speed_vs0_kts = uint(floor(f)); break;
            case V_S: f = getFloatFromCan(canmsg); m_flightstatus->speed_vs1_kts = uint(floor(f));
                // green dot speed = 1.35 * stall speed in clean config
                m_flightstatus->speed_min_drag_kts = uint(floor(f*1.35));
                break;

            case SOS: f = getFloatFromCan(canmsg); speed_of_sound = f;
                m_flightstatus->barber_pole_speed = barberpole(kias_maximum_operating, mach_maximum_operating, speed_of_sound);
                break;
            case M_MO: f = getFloatFromCan(canmsg); mach_maximum_operating = f; break;
            case V_NO: f = getFloatFromCan(canmsg); kias_maximum_operating = f; break;
            case V_FE: f = getFloatFromCan(canmsg); break;
            case QNH: f = getFloatFromCan(canmsg); m_flightstatus->qnh = f; break;
            case OAT: f = getFloatFromCan(canmsg); m_flightstatus->oat = f; break;
            case DEW: f = getFloatFromCan(canmsg); m_flightstatus->dew = f; break;
            case TAT: f = getFloatFromCan(canmsg); m_flightstatus->tat = f; break;
            case ALTSET:f = getFloatFromCan(canmsg); m_flightstatus->setAltPressureSettingHpaExternal(f); break;
            case NAV1:i = getIntFromCan(canmsg); m_flightstatus->nav1_freq=i; break;
            case NAV2:i = getIntFromCan(canmsg); m_flightstatus->nav2_freq=i; break;
            case ADF1:i = getIntFromCan(canmsg); adf1_freq = i; break;
            case ADF2:i = getIntFromCan(canmsg); adf2_freq = i; break;
            case ADF1BRG: f = getFloatFromCan(canmsg); adf1_bearing =f; break;
            case ADF2BRG: f = getFloatFromCan(canmsg); adf2_bearing =f; break;
            case VOR1HDEF:f = getFloatFromCan(canmsg); m_flightstatus->obs1_loc_needle = Navcalc::round(f); break;
            case VOR2HDEF:f = getFloatFromCan(canmsg); m_flightstatus->obs2_loc_needle = Navcalc::round(f); break;
            case ILS1VDEF:f = getFloatFromCan(canmsg); m_flightstatus->obs1_gs_needle = Navcalc::round(f);  break;
            case OBS1:f = getFloatFromCan(canmsg); m_flightstatus->obs1 = Navcalc::round(Navcalc::trimHeading(f));break;
            case OBS2:f = getFloatFromCan(canmsg); m_flightstatus->obs2 = Navcalc::round(Navcalc::trimHeading(f));break;
            case N1DME:f = getFloatFromCan(canmsg); nav1_dme=f; break;
            case N2DME:f = getFloatFromCan(canmsg); nav2_dme=f; break;
            case THDG:f = getFloatFromCan(canmsg); m_flightstatus->setTrueHeading(f); break;
            case LAT: d = getDoubleFromCan(canmsg); m_flightstatus->lat=d; break;
            case LON: d = getDoubleFromCan(canmsg); m_flightstatus->lon=d; break;
            case YAGL:f = getFloatFromCan(canmsg); m_flightstatus->ground_alt_ft=true_alt_ft-f; break;
            case IAS: f = getFloatFromCan(canmsg); m_flightstatus->smoothed_ias = qMax(30.0, (double)f); break;
            case MACH:f = getFloatFromCan(canmsg); m_flightstatus->mach = f; break;
            case VS:  f = getFloatFromCan(canmsg); m_flightstatus->smoothed_vs = f; break;
            case PITCH: f = getFloatFromCan(canmsg); m_flightstatus->pitch = f; break;
            case BANK: f = getFloatFromCan(canmsg); m_flightstatus->bank = f; break;
            case TALT: d = getDoubleFromCan(canmsg); true_alt_ft=d; m_flightstatus->alt_ft = d; break;
            case INDALT:f = getFloatFromCan(canmsg); m_flightstatus->smoothed_altimeter_readout = f; break;
            case GS: f = getFloatFromCan(canmsg); m_flightstatus->ground_speed_kts=f; break;
            case TAS: f = getFloatFromCan(canmsg); m_flightstatus->tas=f; break;
            case APALT: f = getFloatFromCan(canmsg); m_flightstatus->setAPAltExternal(Navcalc::round(f+0.5));break;
            case APHDG: f = getFloatFromCan(canmsg); m_flightstatus->setAPHdgExternal(Navcalc::round(Navcalc::trimHeading(f)));break;
            case APSPD: f = getFloatFromCan(canmsg); ap_spd_is_mach ? m_flightstatus->setAPMachExternal(f): m_flightstatus->setAPSpdExternal(Navcalc::round(f));break;
            case APVS:  f = getFloatFromCan(canmsg); m_flightstatus->setAPVsExternal(Navcalc::round(f)); break;
            case EFIS1SELCPT:  i = getIntFromCan(canmsg); break;
            case EFIS2SELCPT:  i = getIntFromCan(canmsg); break;

            case APMODE:i = getIntFromCan(canmsg); break ;
            case APSTATE: i = getIntFromCan(canmsg);
                apstate = i;
                //printf("=============\n");
                m_flightstatus->ap_available = (i&1); //if (i&1) printf("AP avail\n");
                //m_flightstatus->ap_enabled = (i&2);
                m_flightstatus->ap_hdg_lock = (i&4); //if (i&4) printf("AP HDG\n");
                m_flightstatus->ap_alt_lock = (i&8); //if (i&8) printf("AP ALT (FS)\n");
                m_flightstatus->ap_speed_lock = (i&16); //if (i&16) printf("AP SPEED\n");
                m_flightstatus->ap_mach_lock = (i&32); //if (i&32) printf("AP MACH\n");
                m_flightstatus->ap_vs_lock = (i&64); //if (i&64) printf("AP VS\n");
                m_flightstatus->ap_nav1_lock = (i&128); //if (i&128) printf("AP NAV1\n");
                m_flightstatus->ap_gs_lock = (i&256); //if (i&256) printf("AP GS\n");
                m_flightstatus->ap_app_lock = (i&512); //if (i&512) printf("AP APP\n");
                m_flightstatus->ap_app_bc_lock = (i&1024); //if (i&1024) printf("AP APP_BC\n");
                m_flightstatus->at_toga = (i&2048); //if (i&2048) printf("AT TOGA\n");
                m_flightstatus->at_arm = (i&4096); //if (i&4096) printf("AT ARM\n");
                m_flightstatus->gps_enabled = (i&8192); //if (i&8192) printf("GPS MODE CPL\n");
                break;
            case FDON:  i = getIntFromCan(canmsg); m_flightstatus->fd_active = (i!=0); m_flightstatus->ap_enabled=(i==2);break;
            case FDROLL: f = getFloatFromCan(canmsg); m_flightstatus->setFlightDirectorBankInputFromExternal(true); m_flightstatus->setFlightDirectorBankExternal(-f); break;
            case FDPITCH: f = getFloatFromCan(canmsg); m_flightstatus->setFlightDirectorPitchInputFromExternal(true); m_flightstatus->setFlightDirectorPitchExternal(-f);break;
            case APSPDMACH: b = getBoolFromCan(canmsg); ap_spd_is_mach = b; break;
            case WINDSPEED: f = getFloatFromCan(canmsg);m_flightstatus->wind_speed_kts = f; break;
            case WINDDIR: f = getFloatFromCan(canmsg);m_flightstatus->wind_dir_deg_true = f; break;
            case MAGVAR: f = getFloatFromCan(canmsg); m_flightstatus->magvar = f; if (m_flightstatus->magvar > 180.0) m_flightstatus->magvar -= 360; break;
            case TOTWT:  f = getFloatFromCan(canmsg); m_flightstatus->total_weight_kg = uint(floor(f)); break;
            case FUELCAP: f = getFloatFromCan(canmsg); m_flightstatus->total_fuel_capacity_kg = f; break;
            case FUELWT: f = getFloatFromCan(canmsg); m_flightstatus->zero_fuel_weight_kg = m_flightstatus->total_weight_kg - uint(floor(f)); break;
            case AVIONICS: b = getBoolFromCan(canmsg); m_flightstatus->avionics_on = b; break;
            case BATTERY: b = getBoolFromCan(canmsg); m_flightstatus->battery_on = b; break;
            case ONGROUND: b = getBoolFromCan(canmsg); m_flightstatus->onground = b; break;
            case BEACON: b = getBoolFromCan(canmsg); m_flightstatus->lights_beacon = b; break;
            case STROBE: b = getBoolFromCan(canmsg); m_flightstatus->lights_strobe = b; break;
            case LDGLT:  b = getBoolFromCan(canmsg); m_flightstatus->lights_landing = b; break;
            case NAVLT:  b = getBoolFromCan(canmsg); m_flightstatus->lights_navigation = b; break;
            case TAXILT: b = getBoolFromCan(canmsg); m_flightstatus->lights_taxi = b; break;
            case PITOTHT: b = getBoolFromCan(canmsg); m_flightstatus->pitot_heat_on = b; break;
            case PAUSE:  b = getBoolFromCan(canmsg); m_flightstatus->paused = b; break;
            case PRKBRK: f = getFloatFromCan(canmsg); m_flightstatus->parking_brake_set = (f>0.9); break;
            case NOENGINES: i = getIntFromCan(canmsg);m_flightstatus->nr_of_engines=i; break;
            case ENGTHRO:
                //TODO handle m_separate_throttle_lever_mode
                f = getFloatFromCan(canmsg);
                if (getIndexFromCan(canmsg) == 1)
                    m_flightstatus->setAllThrottleLeversInputPercent(f);
                if (getIndexFromCan(canmsg) < m_flightstatus->nr_of_engines)
                        m_flightstatus->engine_data[getIndexFromCan(canmsg) + 1].throttle_lever_percent = f;
                break;
            case ENGN2:
                    if(getIndexFromCan(canmsg)<m_flightstatus->nr_of_engines)
                        m_flightstatus->engine_data[getIndexFromCan(canmsg) + 1].n2_percent = getFloatFromCan(canmsg);
                break;
            case ENGN1:
                if(getIndexFromCan(canmsg)<m_flightstatus->nr_of_engines)
                    m_flightstatus->engine_data[getIndexFromCan(canmsg) + 1].smoothed_n1 = getFloatFromCan(canmsg);
                break;
            case ENGEGT:
                if(getIndexFromCan(canmsg)<m_flightstatus->nr_of_engines)
                    m_flightstatus->engine_data[getIndexFromCan(canmsg) + 1].egt_degrees = getFloatFromCan(canmsg);;
                break;
            case ENGFF:
                if(getIndexFromCan(canmsg)<m_flightstatus->nr_of_engines)
                        m_flightstatus->engine_data[getIndexFromCan(canmsg) + 1].ff_kg_per_hour = getFloatFromCan(canmsg);;
                break;
            case GEAR:
                switch(getIndexFromCan(canmsg))
                    {
                            case 0: m_flightstatus->gear_nose_position_percent=hardDetentsPercent(getFloatFromCan(canmsg),10); break;
                            case 1: m_flightstatus->gear_left_position_percent=hardDetentsPercent(getFloatFromCan(canmsg),10); break;
                            case 2: m_flightstatus->gear_right_position_percent=hardDetentsPercent(getFloatFromCan(canmsg),10); break;
                            default : break;
                    }
                break;
            case TAI:
                if(getIndexFromCan(canmsg)<m_flightstatus->nr_of_engines)
                    m_flightstatus->engine_data[getIndexFromCan(canmsg) + 1].anti_ice_on = (getFloatFromCan(canmsg)==1);
                break;
            case ENGREV:
                if(getIndexFromCan(canmsg)<m_flightstatus->nr_of_engines)
                        m_flightstatus->engine_data[getIndexFromCan(canmsg) + 1].reverser_percent = hardDetentsPercent(getFloatFromCan(canmsg),10);
                break;
            case N1FROMTO: nav1_flg = getIntFromCan(canmsg); break;
            case N2FROMTO: nav2_flg = getIntFromCan(canmsg); break;
            case N1HASDME: n1hasDME = getBoolFromCan(canmsg); break;
            case N2HASDME: n2hasDME = getBoolFromCan(canmsg); break;
            case NDB1TND:
                if (getBoolFromCan(canmsg))
                {
                    m_flightstatus->adf1 = Ndb(ndb1Id,QString(""),ndb1Lat,ndb1Lon,adf1_freq,50,0,QString(""));
                    m_flightstatus->adf1_bearing = adf1_bearing;
                } else
                {
                    m_flightstatus->adf1 = Ndb(QString::null, QString::null, 0.0, 0.0, adf1_freq, 0, 0, QString::null);
                    m_flightstatus->adf1_bearing.clear();
                }
                break;
            case NDB1ID: ndb1Id = getQStringFomCan(canmsg); break;
            case NDB1LAT: ndb1Lat = getFloatFromCan(canmsg); break;
            case NDB1LON: ndb1Lon = getFloatFromCan(canmsg); break;
            case NDB2TND:
                if (getBoolFromCan(canmsg))
                {
                    m_flightstatus->adf2 = Ndb(ndb2Id,QString(""),ndb2Lat,ndb2Lon,adf2_freq,50,0,QString(""));
                    m_flightstatus->adf2_bearing = adf2_bearing;
                } else
                {
                    m_flightstatus->adf2 = Ndb(QString::null, QString::null, 0.0, 0.0, adf2_freq, 0, 0, QString::null);
                    m_flightstatus->adf2_bearing.clear();
                }
                break;
            case NDB2ID: ndb2Id = getQStringFomCan(canmsg); break;
            case NDB2LAT: ndb2Lat = getFloatFromCan(canmsg); break;
            case NDB2LON: ndb2Lon = getFloatFromCan(canmsg); break;
            case VOR1TND:
                if (getBoolFromCan(canmsg))
                {
                    m_flightstatus->nav1.setId(vor1Id);
                    m_flightstatus->nav1.setLat(vor1Lat);
                    m_flightstatus->nav1.setLon(vor1Lon);
                    if (vor1hasLOC)
                    {
                        m_flightstatus->nav1_has_loc = true;
                        m_flightstatus->nav1_loc_mag_heading =(int)Navcalc::trimHeading(vor1LocCrs);
                    }
                    else
                    {
                        m_flightstatus->nav1_has_loc=false;
                        m_flightstatus->nav1_loc_mag_heading = 0;
                    }
                    if(n1hasDME)
                        m_flightstatus->nav1_distance_nm = QString("%1").arg(nav1_dme,0,'f',1);
                    else
                        m_flightstatus->nav1_distance_nm = QString::null;
                    m_flightstatus->nav1_bearing = Navcalc::getSignedHeadingDiff(Navcalc::trimHeading(m_flightstatus->smoothedTrueHeading()), Navcalc::getTrackBetweenWaypoints(m_flightstatus->current_position_smoothed, m_flightstatus->nav1));
                    m_flightstatus->obs1_to_from = nav1_flg;
                }
                else
                {
                    m_flightstatus->nav1 = Waypoint();
                    m_flightstatus->nav1_has_loc = false;
                    m_flightstatus->nav1_distance_nm = QString::null;
                    m_flightstatus->nav1_bearing.clear();
                }
            break;
            case VOR1ID: vor1Id = getQStringFomCan(canmsg); break;
            case VOR1LAT: vor1Lat = getFloatFromCan(canmsg); break;
            case VOR1LON: vor1Lon = getFloatFromCan(canmsg); break;
            case VOR1LOC: vor1hasLOC = getBoolFromCan(canmsg); break;
            case VOR1LOCCRS: vor1LocCrs = getIntFromCan(canmsg); break;
            case VOR2TND:
                if (getBoolFromCan(canmsg))
                {
                    m_flightstatus->nav2.setId(vor2Id);
                    m_flightstatus->nav2.setLat(vor2Lat);
                    m_flightstatus->nav2.setLon(vor2Lon);
                    if (vor2hasLOC)
                    {
                        m_flightstatus->nav2_has_loc = true;
                        //m_flightstatus->nav2_loc_mag_heading =(int)Navcalc::trimHeading(vor2LocCrs);
                    }
                    else
                    {
                        m_flightstatus->nav2_has_loc=false;
                        //m_flightstatus->nav2_loc_mag_heading = 0;
                    }
                    if(n2hasDME)
                        m_flightstatus->nav2_distance_nm = QString("%2").arg(nav2_dme,0,'f',1);
                    else
                        m_flightstatus->nav2_distance_nm = QString::null;
                    m_flightstatus->nav2_bearing = Navcalc::getSignedHeadingDiff(Navcalc::trimHeading(m_flightstatus->smoothedTrueHeading()), Navcalc::getTrackBetweenWaypoints(m_flightstatus->current_position_smoothed, m_flightstatus->nav2));
                    m_flightstatus->obs2_to_from = nav2_flg;
                }
                else
                {
                    m_flightstatus->nav2 = Waypoint();
                    m_flightstatus->nav2_has_loc = false;
                    m_flightstatus->nav2_distance_nm = QString::null;
                    m_flightstatus->nav2_bearing.clear();
                }
            break;
            case VOR2ID: vor2Id = getQStringFomCan(canmsg); break;
            case VOR2LAT: vor2Lat = getFloatFromCan(canmsg); break;
            case VOR2LON: vor2Lon = getFloatFromCan(canmsg); break;
            case VOR2LOC: vor2hasLOC = getBoolFromCan(canmsg); break;
            case VOR2LOCCRS: vor2LocCrs = getIntFromCan(canmsg); break;
            case TOTALNUM: Logger::log(QString("Total count in this block %1").arg(getIntFromCan(canmsg))); break;

            default: Logger::log(QString("FSAccessXPlane:slotSocketRead: ERROR: Got unrecognized ID of: %1").arg(id));
            } // end of switch
        // set data to valid
        m_flightstatus->recalcAndSetValid();
        m_read_timout_timer.start(READ_TIMEOUT_PERIOD_MS);
    } // end of else (ever connected)
    } // end of while
}

/////////////////////////////////////////////////////////////////////////////

void FSAccessXPlane::slotReadTimeout()
{
    Logger::log("FSAccessXPlane:slotReadTimeout:");
    m_flightstatus->invalidate();
    m_read_timout_timer.stop();
}

// End of file
