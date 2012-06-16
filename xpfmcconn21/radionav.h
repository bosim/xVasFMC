#ifndef RADIONAV_H
#define RADIONAV_H

#include "logichandler.h"

#include <string>
#include "XPLMNavigation.h"

struct navaid {
    std::string Id,
                Name;
    float Lat, Lon;
    int Freq;
    bool HasDME, HasLoc;
    int crs;
};

template <typename T>
class SimData;
template <typename T>
class OwnedData;

class RadioNav : public LogicHandler
{
public:

    RadioNav(std::ostream& logfile);

    virtual ~RadioNav();

    /**
     *  reimplement to register for the data needed for calculation
     */
    virtual bool registerDataRefs();


    /**
     *  reimplement to do the setups that have to be done once when data is acessible
     */
    virtual bool initState();


    /**
     *  reimplement to do the calculations that are done periodically (via the flightloop)
     */
    virtual bool processState();


    /**
     *  reimplement to process a signal from outside (e.g. interplugin message)
     */
    virtual bool processInput(long input);


    /**
     *  reimplement to register the acessors to the owned data to form a custom dataref(s)
     */
    virtual bool publishData();


    /**
     *  reimplement to unregister the custom dataref(s)
     */
    virtual bool unpublishData();


    /**
     *  reimplement to return the value for the next call of your processing- positive for seconds, negative for ticks
     */
    virtual float processingFrequency();

    virtual std::string name() { return "RadioNav";}

    virtual bool writeToStream(ProtocolStreamer*);

    virtual bool needsSpecialWrite() { return true; }

    virtual void suspend(bool yes) { m_suspended = yes; }

    virtual bool isSuspended() { return m_suspended; }

  private:

    float range(float, float, float, float);

    bool m_is_ready;

    bool m_has_changed;

    navaid locateNavAid(int freq, float lat, float lon, XPLMNavType type, bool has_dme=false);

    SimData<std::vector<int> >* m_navReceiverStates;
    SimData<int>* m_nav1Freq;
    SimData<int>* m_nav2Freq;
    SimData<int>* m_adf1Freq;
    SimData<int>* m_adf2Freq;
    SimData<bool>* m_n1HasDME;
    SimData<bool>* m_n2HasDME;
    SimData<double>* m_airplaneLat;
    SimData<double>* m_airplaneLon;

    OwnedData<bool>* vor1Tuned;
    OwnedData<std::string>* vor1Id;
    OwnedData<std::string>* vor1Name;
    OwnedData<float>* vor1Lat;
    OwnedData<float>* vor1Lon;
    OwnedData<bool>* vor1hasLoc;
    OwnedData<int>* vor1Crs;
    OwnedData<bool>* vor2Tuned;
    OwnedData<std::string>* vor2Id;
    OwnedData<std::string>* vor2Name;
    OwnedData<float>* vor2Lat;
    OwnedData<float>* vor2Lon;
    OwnedData<bool>* vor2hasLoc;
    OwnedData<int>* vor2Crs;
    OwnedData<bool>* ndb1Tuned;
    OwnedData<std::string>* ndb1Id;
    OwnedData<std::string>* ndb1Name;
    OwnedData<float>* ndb1Lat;
    OwnedData<float>* ndb1Lon;
    OwnedData<bool>* ndb2Tuned;
    OwnedData<std::string>* ndb2Id;
    OwnedData<std::string>* ndb2Name;
    OwnedData<float>* ndb2Lat;
    OwnedData<float>* ndb2Lon;

    std::ostream& m_logfile;

    bool m_suspended;

};

#endif // RADIONAV_H
