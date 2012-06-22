#include "radionav.h"
#include "simdata.h"
#include "owneddata.h"
#include "navcalc.h"
#include "protocolstreamer.h"

#include "fsaccess_xplane_refids.h"

#include <stdlib.h>
#include <cmath>

#include "stdint.h"

RadioNav::RadioNav(std::ostream& logfile):
        LogicHandler(logfile),
    m_navReceiverStates(0),
    m_nav1Freq(0),
    m_nav2Freq(0),
    m_adf1Freq(0),
    m_adf2Freq(0),
    m_n1HasDME(0),
    m_n2HasDME(0),
    m_airplaneLat(0),
    m_airplaneLon(0),

    vor1Tuned(0),
    vor1Id(0),
    vor1Name(0),
    vor1Lat(0),
    vor1Lon(0),
    vor1hasLoc(0),
    vor1Crs(0),
    vor2Tuned(0),
    vor2Id(0),
    vor2Name(0),
    vor2Lat(0),
    vor2Lon(0),
    vor2hasLoc(0),
    vor2Crs(0),
    ndb1Tuned(),
    ndb1Id(0),
    ndb1Name(0),
    ndb1Lat(0),
    ndb1Lon(0),
    ndb2Tuned(0),
    ndb2Id(0),
    ndb2Name(0),
    ndb2Lat(0),
    ndb2Lon(0),
    m_logfile(logfile)
{}

RadioNav::~RadioNav()
{
    delete m_navReceiverStates;
    delete m_nav1Freq;
    delete m_nav2Freq;
    delete m_adf1Freq;
    delete m_adf2Freq;
    delete m_n1HasDME;
    delete m_n2HasDME;
    delete m_airplaneLat;
    delete m_airplaneLon;

    delete ndb1Tuned;
    delete ndb1Id;
    delete ndb1Name;
    delete ndb1Lat;
    delete ndb1Lon;
    delete ndb2Tuned;
    delete ndb2Id;
    delete ndb2Name;
    delete ndb2Lat;
    delete ndb2Lon;

    delete vor1Tuned;
    delete vor1Id;
    delete vor1Name;
    delete vor1Lat;
    delete vor1Lon;
    delete vor1hasLoc;
    delete vor1Crs;
    delete vor2Tuned;
    delete vor2Id;
    delete vor2Name;
    delete vor2Lat;
    delete vor2Lon;
    delete vor2hasLoc;
    delete vor2Crs;
}

bool RadioNav::registerDataRefs()
{
    m_navReceiverStates = new SimData<std::vector<int> >("sim/cockpit/radios/nav_type", "Handler: Nav receiver states", RWType::ReadOnly, 1, 1, 0, 6);
    m_nav1Freq = new SimData<int>("sim/cockpit/radios/nav1_freq_hz","Handler: NAV1",RWType::ReadOnly,1,10);
    m_nav2Freq = new SimData<int>("sim/cockpit/radios/nav2_freq_hz","Handler: NAV2",RWType::ReadOnly,1,10);
    m_adf1Freq = new SimData<int>("sim/cockpit/radios/adf1_freq_hz","Handler: ADF1",RWType::ReadOnly,1,1000);
    m_adf2Freq = new SimData<int>("sim/cockpit/radios/adf2_freq_hz","Handler: ADF2",RWType::ReadOnly,1,1000);
    m_n1HasDME = new SimData<bool>("sim/cockpit/radios/nav1_has_dme", "Handler: VOR received from Nav1 receiver has a DME collocated",RWType::ReadOnly);
    m_n2HasDME = new SimData<bool>("sim/cockpit/radios/nav2_has_dme", "Handler: VOR received from Nav2 receiver has a DME collocated",RWType::ReadOnly);
    m_airplaneLat = new SimData<double>("sim/flightmodel/position/latitude","Handler: Latitude");
    m_airplaneLon = new SimData<double>("sim/flightmodel/position/longitude","Handler: Longitude");
    m_logfile << "Reading-Datarefs for Nav Receiver initialized" << std::endl;
    return true;
}

bool RadioNav::initState()
{
    if (m_navReceiverStates == 0)
        m_logfile << "Navaids unavail" << std::endl;
    else {
        m_navReceiverStates->poll();
        m_nav1Freq->poll();
        m_nav2Freq->poll();
        m_adf1Freq->poll();
        m_adf2Freq->poll();
        m_n1HasDME->poll();
        m_n2HasDME->poll();
        m_airplaneLat->poll();
        m_airplaneLon->poll();
        m_logfile << "RadioNav Handler initialized (from com.flightpanels.universal_communications_bus)" << std::endl;
    }
    return false;
}

bool RadioNav::processState()
{
    float treshold = 250;
    if (!m_is_ready)
        return false;
    m_navReceiverStates->poll();
    m_nav1Freq->poll();
    m_nav2Freq->poll();
    m_adf1Freq->poll();
    m_adf2Freq->poll();
    m_n1HasDME->poll();
    m_n2HasDME->poll();
    m_airplaneLat->poll();
    m_airplaneLon->poll();
    if (m_nav1Freq->hasChanged() || m_nav2Freq->hasChanged() ||
        m_adf1Freq->hasChanged() || m_adf2Freq->hasChanged() ||
        m_n1HasDME->hasChanged() || m_n2HasDME->hasChanged() ||
        m_navReceiverStates->hasChanged() || true )
    {
        m_has_changed = true;
        const std::vector<int>& navmode = m_navReceiverStates->data();
        std::vector<int> freqs;
        freqs.push_back(m_nav1Freq->data()); freqs.push_back(m_nav2Freq->data());
        freqs.push_back(m_adf1Freq->data()); freqs.push_back(m_adf2Freq->data());
        const bool& has_dme1 = m_n1HasDME->data();
        const bool& has_dme2 = m_n2HasDME->data();
        const double& lat = m_airplaneLat->data();
        const double& lon = m_airplaneLon->data();
        ///////

            if(navmode.at(0)==5 || navmode.at(0)==4 || navmode.at(0) == 10)
            {
                vor1Tuned->set(true);
                vor1hasLoc->set(true);
                navaid tunedNavAid = locateNavAid(freqs.at(0)/10,lat,lon,navmode.at(0),has_dme1);
                float dist = range(tunedNavAid.Lat, tunedNavAid.Lon, m_airplaneLat->data(), m_airplaneLon->data());
                if (dist < treshold) {
                    vor1Id->set(tunedNavAid.Id);
                    vor1Name->set(tunedNavAid.Name);
                    vor1Lat->set(tunedNavAid.Lat);
                    vor1Lon->set(tunedNavAid.Lon);
                    vor1Crs->set(tunedNavAid.crs);
                } else {
                    vor1Tuned->set(false);
                    vor1hasLoc->set(false);
                    vor1Id->set("");
                    vor1Name->set("");
                    vor1Lat->set(0);
                    vor1Lon->set(0);
                    vor1Crs->set(0);
                }
            }
            else if (navmode.at(0) == 3)
            {
                vor1Tuned->set(true);
                vor1hasLoc->set(false);
                navaid tunedNavAid = locateNavAid(freqs.at(0)/10,lat,lon,xplm_Nav_VOR,has_dme1);
                float dist = range(tunedNavAid.Lat, tunedNavAid.Lon, m_airplaneLat->data(), m_airplaneLon->data());
                if (dist < treshold) {
                    vor1Id->set(tunedNavAid.Id);
                    vor1Name->set(tunedNavAid.Name);
                    vor1Lat->set(tunedNavAid.Lat);
                    vor1Lon->set(tunedNavAid.Lon);
                } else {
                    vor1Tuned->set(false);
                    vor1hasLoc->set(false);
                    vor1Id->set("");
                    vor1Name->set("");
                    vor1Lat->set(0);
                    vor1Lon->set(0);
                    vor1Crs->set(0);
                }
        } else
        {
            vor1Tuned->set(false);
            vor1hasLoc->set(false);
            vor1Id->set("");
            vor1Name->set("");
            vor1Lat->set(0);
            vor1Lon->set(0);
            vor1Crs->set(0);
        }

            if(navmode.at(1)==5 || navmode.at(1)==4 || navmode.at(1) == 10)
            {
                vor2Tuned->set(true);
                vor2hasLoc->set(true);
                navaid tunedNavAid = locateNavAid(freqs.at(1)/10,lat,lon,navmode.at(1),has_dme2);
                float dist = range(tunedNavAid.Lat, tunedNavAid.Lon, m_airplaneLat->data(), m_airplaneLon->data());
                if (dist < treshold) {
                    vor2Id->set(tunedNavAid.Id);
                    vor2Name->set(tunedNavAid.Name);
                    vor2Lat->set(tunedNavAid.Lat);
                    vor2Lon->set(tunedNavAid.Lon);
                    vor2Crs->set(tunedNavAid.crs);
                } else {
                    vor2Tuned->set(false);
                    vor2hasLoc->set(false);
                    vor2Id->set("");
                    vor2Name->set("");
                    vor2Lat->set(0);
                    vor2Lon->set(0);
                    vor2Crs->set(0);
                }
            }
            else if (navmode.at(1) == 3)
            {
                vor2Tuned->set(true);
                vor2hasLoc->set(false);
                navaid tunedNavAid = locateNavAid(freqs.at(1)/10,lat,lon,xplm_Nav_VOR,has_dme2);
                float dist = range(tunedNavAid.Lat, tunedNavAid.Lon, m_airplaneLat->data(), m_airplaneLon->data());
                if (dist < treshold) {
                    vor2Id->set(tunedNavAid.Id);
                    vor2Name->set(tunedNavAid.Name);
                    vor2Lat->set(tunedNavAid.Lat);
                    vor2Lon->set(tunedNavAid.Lon);
                } else {
                    vor2Tuned->set(false);
                    vor2hasLoc->set(false);
                    vor2Id->set("");
                    vor2Name->set("");
                    vor2Lat->set(0);
                    vor2Lon->set(0);
                    vor2Crs->set(0);
                }

        } else
        {
            vor2Tuned->set(false);
            vor2hasLoc->set(false);
                    vor2Id->set("");
                    vor2Name->set("");
                    vor2Lat->set(0);
                    vor2Lon->set(0);
                    vor2Crs->set(0);
        }
        if(navmode.at(2)!=0)
        {
            ndb1Tuned->set(true);
            navaid tunedNavAid = locateNavAid(freqs.at(2)/1000,lat,lon,xplm_Nav_NDB);
            ndb1Name->set(tunedNavAid.Name);
            ndb1Id->set(tunedNavAid.Id);
            ndb1Lat->set(tunedNavAid.Lat);
            ndb1Lon->set(tunedNavAid.Lon);
        }
        else
        {
            ndb1Tuned->set(false);
        }

        if(navmode.at(3)!=0)
        {
            ndb2Tuned->set(true);
            navaid tunedNavAid = locateNavAid(freqs.at(3)/1000,lat,lon,xplm_Nav_NDB);
            ndb2Name->set(tunedNavAid.Name);
            ndb2Id->set(tunedNavAid.Id);
            ndb2Lat->set(tunedNavAid.Lat);
            ndb2Lon->set(tunedNavAid.Lon);
        }
        else
        {
            ndb2Tuned->set(false);
        }
        m_adf1Freq->resetChanged();
        m_adf2Freq->resetChanged();
        m_nav1Freq->resetChanged();
        m_nav2Freq->resetChanged();
        m_n1HasDME->resetChanged();
        m_n2HasDME->resetChanged();
        m_navReceiverStates->resetChanged();
    }
    return true;
}

bool RadioNav::processInput(long)
{
    return true;
}

bool RadioNav::publishData()
{
    ndb1Tuned = new OwnedData<bool>("plugins/org/vasproject/xpfmcconn/ndb1tuned");
    ndb1Id = new OwnedData<std::string>("plugins/org/vasproject/xpfmcconn/ndb1Id");
    ndb1Name = new OwnedData<std::string>("plugins/org/vasproject/xpfmcconn/ndb1Name");
    ndb1Lat = new OwnedData<float>("plugins/org/vasproject/xpfmcconn/ndb1Lat");
    ndb1Lon = new OwnedData<float>("plugins/org/vasproject/xpfmcconn/ndb1Lon");

    ndb2Tuned = new OwnedData<bool>("plugins/org/vasproject/xpfmcconn/ndb2tuned");
    ndb2Id = new OwnedData<std::string>("plugins/org/vasproject/xpfmcconn/ndb2Id");
    ndb2Name = new OwnedData<std::string>("plugins/org/vasproject/xpfmcconn/ndb2Name");
    ndb2Lat = new OwnedData<float>("plugins/org/vasproject/xpfmcconn/ndb2Lat");
    ndb2Lon = new OwnedData<float>("plugins/org/vasproject/xpfmcconn/ndb2Lon");

    vor1Tuned = new OwnedData<bool>("plugins/org/vasproject/xpfmcconn/vor1tuned");
    vor1Id = new OwnedData<std::string>("plugins/org/vasproject/xpfmcconn/vor1Id");
    vor1Name = new OwnedData<std::string>("plugins/org/vasproject/xpfmcconn/vor1Name");
    vor1Lat = new OwnedData<float>("plugins/org/vasproject/xpfmcconn/vor1Lat");
    vor1Lon = new OwnedData<float>("plugins/org/vasproject/xpfmcconn/vor1Lon");
    vor1hasLoc = new OwnedData<bool>("plugins/org/vasproject/xpfmcconn/vor1hasLoc");
    vor1Crs = new OwnedData<int>("plugins/org/vasproject/xpfmcconn/vor1crs");

    vor2Tuned = new OwnedData<bool>("plugins/org/vasproject/xpfmcconn/vor2tuned");
    vor2Id = new OwnedData<std::string>("plugins/org/vasproject/xpfmcconn/vor2Id");
    vor2Name = new OwnedData<std::string>("plugins/org/vasproject/xpfmcconn/vor2Name");
    vor2Lat = new OwnedData<float>("plugins/org/vasproject/xpfmcconn/vor2Lat");
    vor2Lon = new OwnedData<float>("plugins/org/vasproject/xpfmcconn/vor2Lon");
    vor2hasLoc = new OwnedData<bool>("plugins/org/vasproject/xpfmcconn/vor2hasLoc");
    vor2Crs = new OwnedData<int>("plugins/org/vasproject/xpfmcconn/vor2crs");

    ndb1Tuned->registerRead();
    ndb1Id->registerRead();
    ndb1Name->registerRead();
    ndb1Lat->registerRead();
    ndb1Lon->registerRead();
    ndb2Tuned->registerRead();
    ndb2Id->registerRead();
    ndb2Name->registerRead();
    ndb2Lat->registerRead();
    ndb2Lon->registerRead();

    vor1Tuned->registerRead();
    vor1Id->registerRead();
    vor1Name->registerRead();
    vor1Lat->registerRead();
    vor1Lon->registerRead();
    vor1hasLoc->registerRead();
    vor1Crs->registerRead();
    vor2Tuned->registerRead();
    vor2Id->registerRead();
    vor2Name->registerRead();
    vor2Lat->registerRead();
    vor2Lon->registerRead();
    vor2hasLoc->registerRead();
    vor2Crs->registerRead();

    m_logfile << "Published Datarefs for Nav Receiver initialized" << std::endl;

    m_is_ready = ( ndb1Id->isRegistered() && ndb1Name->isRegistered() && ndb1Lat->isRegistered() &&
             ndb1Lon->isRegistered() && ndb2Id->isRegistered() && ndb2Name->isRegistered() &&
             ndb2Lat->isRegistered() && ndb2Lon->isRegistered() &&
             vor1Id->isRegistered() && vor1Name->isRegistered() && vor1Lat->isRegistered() &&
             vor1Lon->isRegistered() && vor1hasLoc->isRegistered() && vor1Crs->isRegistered() &&
             vor2Id->isRegistered() && vor2Name->isRegistered() && vor2Lat->isRegistered() &&
             vor2Lon->isRegistered() && vor2hasLoc->isRegistered() && vor2Crs->isRegistered());

    return m_is_ready;
}

float RadioNav::processingFrequency()
{
    return 1.0f;
}

bool RadioNav::unpublishData()
{
    ndb1Tuned->unregister();
    ndb1Id->unregister();
    ndb1Name->unregister();
    ndb1Lat->unregister();
    ndb1Lon->unregister();
    ndb2Tuned->unregister();
    ndb2Id->unregister();
    ndb2Name->unregister();
    ndb2Lat->unregister();
    ndb2Lon->unregister();

    vor1Tuned->unregister();
    vor1Id->unregister();
    vor1Name->unregister();
    vor1Lat->unregister();
    vor1Lon->unregister();
    vor1hasLoc->unregister();
    vor1Crs->unregister();
    vor2Tuned->unregister();
    vor2Id->unregister();
    vor2Name->unregister();
    vor2Lat->unregister();
    vor2Lon->unregister();
    vor2hasLoc->unregister();
    vor2Crs->unregister();

    delete ndb1Tuned;
    delete ndb1Id;
    delete ndb1Name;
    delete ndb1Lat;
    delete ndb1Lon;
    delete ndb2Tuned;
    delete ndb2Id;
    delete ndb2Name;
    delete ndb2Lat;
    delete ndb2Lon;

    delete vor1Tuned;
    delete vor1Id;
    delete vor1Name;
    delete vor1Lat;
    delete vor1Lon;
    delete vor1hasLoc;
    delete vor1Crs;
    delete vor2Tuned;
    delete vor2Id;
    delete vor2Name;
    delete vor2Lat;
    delete vor2Lon;
    delete vor2hasLoc;
    delete vor2Crs;

    ndb1Tuned = 0;
    ndb1Id = 0;
    ndb1Name = 0;
    ndb1Lat = 0;
    ndb1Lon = 0;
    ndb2Tuned = 0;
    ndb2Id = 0;
    ndb2Name = 0;
    ndb2Lat = 0;
    ndb2Lon = 0;

    vor1Tuned = 0;
    vor1Id = 0;
    vor1Name = 0;
    vor1Lat = 0;
    vor1Lon = 0;
    vor1hasLoc = 0;
    vor1Crs = 0;
    vor2Tuned = 0;
    vor2Id = 0;
    vor2Name = 0;
    vor2Lat = 0;
    vor2Lon = 0;
    vor2hasLoc = 0;
    vor2Crs = 0;

    return true;
}

bool RadioNav::writeToStream(ProtocolStreamer* streamer)
{
    static uint8_t mc_vor1tnd = 0;
    static uint8_t mc_vor1loc = 0;
    static uint8_t mc_vor1id = 0;
    static uint8_t mc_vor1lat = 0;
    static uint8_t mc_vor1lon = 0;
    static uint8_t mc_vor1loccrs = 0;
    static uint8_t mc_vor2tnd = 0;
    static uint8_t mc_vor2loc = 0;
    static uint8_t mc_vor2id = 0;
    static uint8_t mc_vor2lat = 0;
    static uint8_t mc_vor2lon = 0;
    static uint8_t mc_vor2loccrs = 0;
    static uint8_t mc_ndb1tnd = 0;
    static uint8_t mc_ndb1id = 0;
    static uint8_t mc_ndb1lat = 0;
    static uint8_t mc_ndb1lon = 0;
    static uint8_t mc_ndb2tnd = 0;
    static uint8_t mc_ndb2id = 0;
    static uint8_t mc_ndb2lat = 0;
    static uint8_t mc_ndb2lon = 0;

    if (!m_is_ready)
        return false;
    //if (m_has_changed)
    //{
        m_has_changed = false;
        if (vor1Tuned->data() && !vor1hasLoc->data())
        {
            streamer->protocolWrite(VOR1TND,true,mc_vor1tnd++);
            streamer->protocolWrite(VOR1LOC, false,mc_vor1loc++);
            streamer->protocolWrite(VOR1ID, vor1Id->data(),mc_vor1id++);
            streamer->protocolWrite(VOR1LAT, float(vor1Lat->data()),mc_vor1lat++);
            streamer->protocolWrite(VOR1LON, float(vor1Lon->data()),mc_vor1lon++);
        } else if (vor1Tuned->data() && vor1hasLoc->data())
        {
            streamer->protocolWrite(VOR1TND, true, mc_vor1tnd++);
            streamer->protocolWrite(VOR1LOC, true, mc_vor1loc++);
            streamer->protocolWrite(VOR1ID, vor1Id->data(), mc_vor1id++);
            streamer->protocolWrite(VOR1LAT, float(vor1Lat->data()), mc_vor1lat++);
            streamer->protocolWrite(VOR1LON, float(vor1Lon->data()), mc_vor1lon++);
            streamer->protocolWrite(VOR1LOCCRS, (int)vor1Crs->data(), mc_vor1loccrs++);
        } else
        {
            streamer->protocolWrite(VOR1TND, false, mc_vor1tnd++);
        }

        if (vor2Tuned->data() && !vor2hasLoc->data())
        {
            streamer->protocolWrite(VOR2TND,true, mc_vor2tnd++);
            streamer->protocolWrite(VOR2LOC, false, mc_vor2loc++);
            streamer->protocolWrite(VOR2ID, vor2Id->data(), mc_vor2id++);
            streamer->protocolWrite(VOR2LAT, float(vor2Lat->data()), mc_vor2lat++);
            streamer->protocolWrite(VOR2LON, float(vor2Lon->data()), mc_vor2lon++);
        } else if (vor2Tuned->data() && vor2hasLoc->data())
        {
            streamer->protocolWrite(VOR2TND,true, mc_vor2tnd++);
            streamer->protocolWrite(VOR2LOC, false, mc_vor2loc++);
            streamer->protocolWrite(VOR2ID, vor2Id->data(), mc_vor2id++);
            streamer->protocolWrite(VOR2LAT, float(vor2Lat->data()), mc_vor2lat++);
            streamer->protocolWrite(VOR2LON, float(vor2Lon->data()), mc_vor2lon++);
            streamer->protocolWrite(VOR2LOCCRS, (int)vor2Crs->data(), mc_vor2loccrs++);

        } else
        {
            streamer->protocolWrite(VOR2TND, false, mc_vor2tnd++);
        }

        if (ndb1Tuned->data())
        {
            streamer->protocolWrite(NDB1TND, true, mc_ndb1tnd++);
            streamer->protocolWrite(NDB1ID, ndb1Id->data(), mc_ndb1id++);
            streamer->protocolWrite(NDB1LAT, float(ndb1Lat->data()), mc_ndb1lat++);
            streamer->protocolWrite(NDB1LON, float(ndb1Lon->data()), mc_ndb1lon++);
        } else
        {
            streamer->protocolWrite(NDB1TND, false, mc_ndb1tnd++);
        }

        if (ndb2Tuned->data())
        {
            streamer->protocolWrite(NDB2TND, true, mc_ndb2tnd++);
            streamer->protocolWrite(NDB2ID, ndb2Id->data(), mc_ndb2id++);
            streamer->protocolWrite(NDB2LAT, float(ndb2Lat->data()), mc_ndb2lat++);
            streamer->protocolWrite(NDB2LON, float(ndb2Lon->data()), mc_ndb2lon++);
        } else
        {
            streamer->protocolWrite(NDB2TND, false, mc_ndb2tnd++);
        }
    //}
    return true;
}

navaid RadioNav::locateNavAid(int freq, float lat, float lon, XPLMNavType type, bool has_dme)
{
    XPLMNavRef navaidref = XPLMFindNavAid(NULL,NULL,&lat,&lon,&freq,type);
    if(navaidref == XPLM_NAV_NOT_FOUND)
        return navaid();
    XPLMNavType outType = 0;
    float outLatitude = 0;
    float outLongitude = 0;
    float outHeight = 0;
    float outHeading = 0;
    int outFrequency = 0;
    char outID[32];
    char outName[512];
    XPLMGetNavAidInfo(navaidref, &outType, &outLatitude, &outLongitude, &outHeight, &outFrequency, &outHeading, outID, outName, NULL);
    navaid out;
        out.Id = outID;
        out.Name = outName;
        out.Lat = outLatitude;
        out.Lon = outLongitude;
        out.Freq = outFrequency;
    switch(type)
    {
        case (xplm_Nav_NDB):
            break;
        case (xplm_Nav_VOR):
            out.HasDME = has_dme;
            out.HasLoc = false;
            break;
        case (xplm_Nav_ILS):
        case (xplm_Nav_Localizer):
        case (xplm_Nav_GlideSlope):
            out.crs = Navcalc::round(outHeading);
            out.HasDME = has_dme;
            out.HasLoc = true;
            break;
    }    return out;
}

float RadioNav::range(float lat1, float lon1, float lat2, float lon2)
{
    double navAidLat = lat1 * (M_PI/180);
    double navAidLon = lon1 * (M_PI/180);
    double planeLat = lat2 * (M_PI/180);
    double planeLon = lon2 * (M_PI/180);
    // get the distance in radians
    double distance = 2*asin(sqrt(pow(sin((navAidLat-planeLat)/2),2) + cos(navAidLat)*cos(planeLat)*pow(sin((navAidLon-planeLon)/2),2)));
    //now convert radians to nm
    distance = ((180*60)/M_PI)*distance;
    return float(distance);
}
