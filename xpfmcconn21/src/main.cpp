// includes from standardlibrary
#include <iostream>
#include <fstream>
#include <vector>

// includes from X-Plane SDK
#include "XPLMPlugin.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"

// includes from XPCL-SDK
#include "plugin_defines.h"
#include "navcalc.h"
#include "myassert.h"
#include "canasoverudp.h"
#include "datacontainer.h"
#include "logichandler.h"
#include "radionav.h"
#include "apxplane9standard.h"
#include "fbwjoystickinterface.h"
#include "owneddata.h"
#include "simdata.h"

// includes from vaslib
#include "fsaccess_xplane_refids.h"

std::fstream m_logfile;

bool m_enabled = false;
bool m_handlersRegistered = false;

static float returnTime;
static const int maxDataItems = 30;
DataContainer<double> doubleData;
DataContainer<float> floatData;
DataContainer<int> intData;
DataContainer<bool> boolData;
DataContainer<std::vector<float> > floatvectorData;
DataContainer<std::vector<int> > intvectorData;
DataContainer<std::string> stringData;

LogicHandler* NavAidsHandler;
LogicHandler* AutoPilotHandler;

std::vector<LogicHandler*> Handlers;

CanASOverUDP* myCommunicator;

void registerInternalHandlersDataRefs();

static float readHighPrioSimDataCallback(float, float, int, void*)
{
    if (!m_enabled) return 1;

    m_logfile << "readHighPrioSimDataCallback called" << std::endl;

    doubleData.updateHighPrio();
    floatData.updateHighPrio();

    // update every seventh loop
    return -7;
}

static float prepareSendQueueCallback (float, float, int, void*)
{
    // if plugin is disabled, do nothing and try again in 1 seconds
    if (!m_enabled) return 1;

    double secs = XPLMGetElapsedTime();
    int ticks = XPLMGetCycleNumber();

    doubleData.updateAll();
    floatData.updateAll();
    boolData.updateAll();
    intData.updateAll();
    floatvectorData.updateAll();
    stringData.updateAll();

    myCommunicator->processOutput(intData, floatData, doubleData, boolData, floatvectorData,
                                  intvectorData, stringData, Handlers, ticks, secs, maxDataItems);

    // send UDP messages according to time policy
    return returnTime;
}

static float sendQueuedDataCallback (float, float, int, void*)
{
    myCommunicator->flush(maxDataItems);
    return 0.08f;
}

float readDataCallback(float, float, int, void*)
{
    if (!m_enabled) return 1;

    double secs = XPLMGetElapsedTime();
    int ticks = XPLMGetCycleNumber();
    myCommunicator->processInput(intData, floatData, doubleData, boolData, floatvectorData,
                                 intvectorData, stringData, Handlers, ticks, secs);


    // check three times a second for new UDP messages
    return 0.33f;
}

void setup()
{
    m_logfile << "setup" << std::endl;

    // setup config

    myCommunicator = new CanASOverUDP(m_logfile);
    myCommunicator->configure();
    returnTime = 0.17f;

    m_logfile << "setup: success" << std::endl;
}

void shutDown()
{
    m_logfile << "shutting down and closing connections.\n";
    delete myCommunicator;
    myCommunicator = 0;
    m_logfile << "All weapon systems offline, Captain. Shutdown finished." << std::endl;
}

void registerInternalHandlersDataRefs()
{
    bool ap = false;
    bool radio = false;
    bool joystick = false;
    for ( std::vector<LogicHandler*>::const_iterator it = Handlers.begin () ; it!= Handlers.end() ; ++it )
    {
        std::string name = (*it)->name();
        m_logfile << "Handler is present:" << name << std::endl;
        if ( name == "APXPlane9Standard" )
        {
            ap = true;
        }
        if ( name == "FBWJoystickInterface" )
        {
            joystick = true;
        }
        if ( name == "RadioNav" )
        {
            radio = true;
        }
    }

    if (ap)
    {
        m_logfile << "standard autopilot handler setup success" << std::endl;
        intData.addDataRef(APSTATE, RWType::ReadOnly, "Autoilot State handled by standard handler",
                           "plugins/org/vasproject/xpfmcconn/autopilot/autopilot_state",PrioType::Middle,1);
        floatData.addDataRef(APVS, RWType::ReadOnly, "Autopilot Vertical Speed Selected",
                             "plugins/org/vasproject/xpfmcconn/autopilot/vertical_velocity",PrioType::Middle,50);
        floatData.addDataRef(APHDG,RWType::ReadWrite,"AP HDG",
                             "plugins/org/vasproject/xpfmcconn/autopilot/heading",PrioType::Middle,0.5);
    } else
    {
        m_logfile << "standard autopilot handler setup FAILED" << std::endl;
        intData.removeAtId(APSTATE);
        floatData.removeAtId(APVS);
        floatData.removeAtId(APHDG);
    }

    m_handlersRegistered = true;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID, long inMessage, void* inParam)
{
    //TODO: hook in and off handlers and keep track
    if (m_enabled) {
        if( inMessage == XPLM_MSG_PLANE_LOADED)
        {
            int* planeNo = static_cast<int*>(inParam);
            if (planeNo == 0)
            {
                // determine number of engines and change engine data vectors accordingly
                floatvectorData.removeAtId(ENGN1);
                floatvectorData.removeAtId(ENGN2);
                floatvectorData.removeAtId(ENGEGT);
                floatvectorData.removeAtId(ENGFF);
                floatvectorData.removeAtId(TAI);
                floatvectorData.removeAtId(ENGREV);
                floatvectorData.removeAtId(ENGTHRO);
                floatvectorData.removeAtId(FLAPDETPOS);
                intData.updateAll();
                int number_of_engines = intData.valueAtId(NOENGINES);
                int number_of_flapnotches = intData.valueAtId(FLAPDET);
                floatvectorData.addDataRef(ENGN1,RWType::ReadOnly,"Array containing ENG N1 percent","sim/flightmodel/engine/ENGN_N1_",PrioType::Middle, 0.1f, 1, 0, number_of_engines);
                floatvectorData.addDataRef(ENGN2,RWType::ReadOnly,"Array containing ENG N2 percent","sim/flightmodel/engine/ENGN_N2_",PrioType::Middle, 0.1f, 1, 0, number_of_engines);
                floatvectorData.addDataRef(ENGEGT,RWType::ReadOnly,"Array containing ENG EGT deg celsius","sim/flightmodel/engine/ENGN_EGT_c",PrioType::Middle,10, 1, 0, number_of_engines);
                floatvectorData.addDataRef(ENGFF,RWType::ReadOnly,"Array containing ENG FF","sim/flightmodel/engine/ENGN_FF_",PrioType::Middle,1/360,3600, 0, number_of_engines);
                floatvectorData.addDataRef(TAI,RWType::ReadOnly,"ENG Anti-ice on per engine","sim/cockpit/switches/anti_ice_engine_air",PrioType::Constant,0.5, 1, 0, number_of_engines);
                floatvectorData.addDataRef(ENGREV,RWType::ReadOnly,"Thrust reversers","sim/flightmodel2/engines/thrust_reverser_deploy_ratio",PrioType::Constant,0.1,100,0,number_of_engines);
                floatvectorData.addDataRef(ENGTHRO,RWType::ReadOnly,"Throttle lever percent","sim/flightmodel/engine/ENGN_thro",PrioType::Middle,0.01,100,0,number_of_engines);
                floatvectorData.addDataRef(FLAPDETPOS,RWType::ReadOnly,"Positions of the flap lever detents", "sim/aircraft/controls/acf_flap_dn",PrioType::Constant,0.1,1,0,number_of_flapnotches);
                if (!m_handlersRegistered)
                {
                    m_logfile << "Initializing Handler first time (plugin startup)" << std::endl;
                    std::vector<LogicHandler*>::iterator it;
                    for ( it = Handlers.begin() ; it < Handlers.end() ; it++)
                    {
                        m_logfile << "Hooking " << (*it)->name() << std::endl;
                        (*it)->hookMeIn();
                    }
                    registerInternalHandlersDataRefs();
                } else {
                    // Do whatever is neccessary to switch handlers when switching aircrafts
                    m_logfile << " airplane changed, changing handlers as necessary" << std::endl;
                }
            }
        }
    }
}


PLUGIN_API int XPluginStart(char *outName, char *outSig, char *outDesc)
{
    //m_logfile = new QFile("xpfmcconn.txt");
    m_logfile.open(LOG_FILENAME, std::ios_base::trunc | std::ios_base::out);
    //m_logfile->open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Unbuffered);
    //m_logstream = new QTextStream(m_logfile);
    MYASSERT(m_logfile);
    m_logfile << "XPluginStart:" << std::endl;

    m_enabled = false;

    strcpy(outName, "XPFMCCONN 2.1");
    strcpy(outSig, "org.vas-project.xpfmcconn21");
    strcpy(outDesc, "Connector for vasFMC2.09 or higher. Sends data via UDP. Look at xpfmcconn.cfg for network settings.");

    XPLMRegisterFlightLoopCallback( readHighPrioSimDataCallback, -1, 0 );
    XPLMRegisterFlightLoopCallback( sendQueuedDataCallback, -4, 0 );
    XPLMRegisterFlightLoopCallback( readDataCallback, -2, 0 );
    XPLMRegisterFlightLoopCallback( prepareSendQueueCallback, -3, 0);

    doubleData.addDataRef(LAT,RWType::ReadOnly,"Latitude","sim/flightmodel/position/latitude",PrioType::High,0.0001);
    doubleData.addDataRef(LON,RWType::ReadOnly,"Longitude","sim/flightmodel/position/longitude",PrioType::High,0.0001);
    doubleData.addDataRef(TALT,RWType::ReadOnly,"True Alt","sim/flightmodel/position/elevation",PrioType::High,1, Navcalc::METER_TO_FEET);
    intData.addDataRef(ZDATE,RWType::ReadOnly,"Date in days since Jan 1st", "sim/time/local_date_days", PrioType::Constant,1);
    floatData.addDataRef(ZTIME,RWType::ReadOnly,"UTC Time","sim/time/zulu_time_sec",PrioType::Middle,1);
    floatData.addDataRef(V_S0,RWType::ReadOnly,"VS0, stall speed full flaps","sim/aircraft/view/acf_Vso",PrioType::Constant);
    floatData.addDataRef(V_S,RWType::ReadOnly,"VS, stall speed clean","sim/aircraft/view/acf_Vs",PrioType::Constant);
    floatData.addDataRef(V_NO,RWType::ReadOnly,"Vno, normal operating speed","sim/aircraft/view/acf_Vno",PrioType::Constant);
    floatData.addDataRef(M_MO,RWType::ReadOnly,"Mmo, Mach maximum operating","sim/aircraft/view/acf_Mmo",PrioType::Constant);
    floatData.addDataRef(V_FE,RWType::ReadOnly,"Vfe, max speed with flaps extended","sim/aircraft/view/acf_Vfe",PrioType::Constant);
    floatData.addDataRef(SOS,RWType::ReadOnly,"Speed of Sound at actual position","sim/weather/speed_sound_ms",PrioType::Constant,1,Navcalc::METER_PER_SECOND_TO_KNOTS);
    floatData.addDataRef(YAGL,RWType::ReadOnly,"height above ground meters","sim/flightmodel/position/y_agl",PrioType::Middle,1,Navcalc::METER_TO_FEET);
    floatData.addDataRef(INDALT,RWType::ReadOnly,"altitude indicated","sim/flightmodel/misc/h_ind",PrioType::High,1);
    floatData.addDataRef(THDG,RWType::ReadOnly,"True HDG","sim/flightmodel/position/psi",PrioType::High,0.09);
    floatData.addDataRef(IAS,RWType::ReadOnly,"Indicated Airspeed","sim/flightmodel/position/indicated_airspeed",PrioType::High,0.3);
    floatData.addDataRef(MACH,RWType::ReadOnly,"Mach no","sim/flightmodel/misc/machno",PrioType::Middle);
    floatData.addDataRef(VS,RWType::ReadOnly,"Indicated Vertical Speed","sim/flightmodel/position/vh_ind_fpm",PrioType::High,10);
    floatData.addDataRef(PITCH,RWType::ReadOnly,"Pitch","sim/flightmodel/position/theta",PrioType::High,0.01,-1.0);
    floatData.addDataRef(BANK,RWType::ReadOnly,"Bank","sim/flightmodel/position/phi",PrioType::High,0.01,-1.0);

    floatData.addDataRef(GS,RWType::ReadOnly,"Groundspeed","sim/flightmodel/position/groundspeed",PrioType::Middle,1, Navcalc::METER_PER_SECOND_TO_KNOTS);
    floatData.addDataRef(TAS,RWType::ReadOnly,"True Airspeed","sim/flightmodel/position/true_airspeed",PrioType::Middle,1, Navcalc::METER_PER_SECOND_TO_KNOTS);
    floatData.addDataRef(MAGVAR,RWType::ReadOnly,"Magnetic Variation", "sim/flightmodel/position/magnetic_variation",PrioType::Constant, 0.05,-1.0);
    floatData.addDataRef(WINDSPEED,RWType::ReadOnly,"Windspeed","sim/weather/wind_speed_kt",PrioType::Middle,0.1, Navcalc::METER_PER_SECOND_TO_KNOTS);
    floatData.addDataRef(WINDDIR,RWType::ReadOnly,"Wind Direction","sim/weather/wind_direction_degt",PrioType::Middle,1);
    floatData.addDataRef(QNH,RWType::ReadOnly,"environmental QNH","sim/weather/barometer_sealevel_inhg",PrioType::Middle,0.0001,1/Navcalc::HPA_TO_INHG);
    floatData.addDataRef(OAT,RWType::ReadOnly,"OAT in degC","sim/weather/temperature_ambient_c",PrioType::Low,1);
    floatData.addDataRef(DEW,RWType::ReadOnly,"dewpoint in degC","sim/weather/dewpoi_sealevel_c",PrioType::Low,1);
    floatData.addDataRef(TAT,RWType::ReadOnly,"TAT in degC","sim/weather/temperature_le_c",PrioType::Low,1);
    //floatData.addDataRef(APHDG,RWType::ReadWrite, "AP HDG","sim/cockpit/autopilot/nav_steer_deg_mag",PrioType::High,0.0001);
    floatData.addDataRef(APALT,RWType::ReadOnly,"AP ALT","sim/cockpit/autopilot/altitude",PrioType::Middle,1);
    floatData.addDataRef(APSPD,RWType::ReadOnly,"AP SPD","sim/cockpit/autopilot/airspeed",PrioType::Middle,0.009);
    floatData.addDataRef(ALTSET,RWType::ReadOnly,"pilots altimeter setting","sim/cockpit/misc/barometer_setting",PrioType::Low,0.009,(1/Navcalc::HPA_TO_INHG));
    //floatData.addDataRef(APVS,RWType::ReadOnly,"AP VS","sim/cockpit/autopilot/vertical_velocity",PrioType::Middle,99);
    floatData.addDataRef(ADF1BRG,RWType::ReadOnly,"ADF1 bearing","sim/cockpit/radios/adf1_dir_degt",PrioType::Middle);
    floatData.addDataRef(ADF2BRG,RWType::ReadOnly,"ADF2 bearing","sim/cockpit/radios/adf2_dir_degt",PrioType::Middle);
    floatData.addDataRef(N1DME,RWType::ReadOnly,"DME NAV 1","sim/cockpit/radios/nav1_dme_dist_m",PrioType::Middle);
    floatData.addDataRef(N2DME,RWType::ReadOnly,"DME NAV 1","sim/cockpit/radios/nav2_dme_dist_m",PrioType::Middle);
    floatData.addDataRef(VOR1HDEF,RWType::ReadOnly,"NAV1 VOR-pointer deflection percent","sim/cockpit/radios/nav1_hdef_dot",PrioType::Middle,0.01,62);
    floatData.addDataRef(VOR2HDEF,RWType::ReadOnly,"NAV2 VOR-pointer deflection percent","sim/cockpit/radios/nav2_hdef_dot",PrioType::Middle,0.01,62);
    floatData.addDataRef(ILS1VDEF,RWType::ReadOnly,"NAV1  GS-pointer deflection percent","sim/cockpit/radios/nav1_vdef_dot",PrioType::Middle,0.01,62);
    boolData.addDataRef(APSPDMACH, RWType::ReadOnly,"AP C/O SPD MACH","sim/cockpit/autopilot/airspeed_is_mach",PrioType::Low);
    intData.addDataRef(EFIS1SELCPT, RWType::ReadOnly,"EFIS 1 Select Switch Cpt", "sim/cockpit2/EFIS/EFIS_1_selection_pilot",PrioType::Middle,1);
    intData.addDataRef(EFIS2SELCPT, RWType::ReadOnly,"EFIS 2 Select Switch Cpt", "sim/cockpit2/EFIS/EFIS_2_selection_pilot",PrioType::Middle,1);
    intData.addDataRef(FDON,RWType::ReadOnly,"FD","sim/cockpit/autopilot/autopilot_mode",PrioType::Low,0.9);
    intData.addDataRef(NAV1,RWType::ReadOnly,"NAV 1","sim/cockpit/radios/nav1_freq_hz",PrioType::Low,1,10);
    intData.addDataRef(NAV2,RWType::ReadOnly,"NAV 2","sim/cockpit/radios/nav2_freq_hz",PrioType::Low,1,10);
    intData.addDataRef(ADF1,RWType::ReadOnly,"ADF 1","sim/cockpit/radios/adf1_freq_hz",PrioType::Low,1,1000);
    intData.addDataRef(ADF2,RWType::ReadOnly,"ADF 2","sim/cockpit/radios/adf2_freq_hz",PrioType::Low,1,1000);
    //intvectorData.addDataRef(NAVMODE,RWType::ReadOnly, "NAV Type", "sim/cockpit/radios/nav_type", PrioType::Middle);
    intData.addDataRef(N1FROMTO,RWType::ReadOnly,"Nav 1 from/to","sim/cockpit/radios/nav1_fromto",PrioType::Low);
    intData.addDataRef(N2FROMTO,RWType::ReadOnly,"Nav 2 from/to","sim/cockpit/radios/nav2_fromto",PrioType::Low);
    boolData.addDataRef(N1HASDME,RWType::ReadOnly,"Nav1 has dme","sim/cockpit/radios/nav1_has_dme",PrioType::Low);
    boolData.addDataRef(N2HASDME,RWType::ReadOnly,"Nav2 has dme","sim/cockpit/radios/nav2_has_dme",PrioType::Low);
    //intData.addDataRef(APMODE,RWType::ReadOnly,"AP state","sim/cockpit/autopilot/autopilot_state",PrioType::Low,0.9);
    floatData.addDataRef(OBS1,RWType::ReadOnly,"OBS 1 in mag deg","sim/cockpit/radios/nav1_obs_degm",PrioType::Low);
    floatData.addDataRef(OBS2,RWType::ReadOnly,"OBS 2 in mag deg","sim/cockpit/radios/nav2_obs_degm",PrioType::Low);
    floatData.addDataRef(FDROLL,RWType::ReadOnly,"FD roll","sim/cockpit/autopilot/flight_director_roll",PrioType::High,-1);
    floatData.addDataRef(FDPITCH,RWType::ReadOnly,"FD pitch","sim/cockpit/autopilot/flight_director_pitch",PrioType::High,-1);
    floatData.addDataRef(TOTWT,RWType::ReadOnly,"Total weight kg","sim/flightmodel/weight/m_total",PrioType::Constant);
    floatData.addDataRef(FUELWT,RWType::ReadOnly,"Total fuel kg","sim/flightmodel/weight/m_fuel_total",PrioType::Constant,100);
    floatData.addDataRef(FUELCAP,RWType::ReadOnly,"total fuel capacity kg","sim/aircraft/weight/acf_m_fuel_tot",PrioType::Constant);
    floatvectorData.addDataRef(ENGN1,RWType::ReadOnly,"Array containing ENG N1 percent","sim/flightmodel/engine/ENGN_N1_",PrioType::Middle);
    floatvectorData.addDataRef(ENGN2,RWType::ReadOnly,"Array containing ENG N2 percent","sim/flightmodel/engine/ENGN_N2_",PrioType::Middle);
    floatvectorData.addDataRef(ENGEGT,RWType::ReadOnly,"Array containing ENG EGT deg celsius","sim/flightmodel/engine/ENGN_EGT_c",PrioType::Middle,10);
    floatvectorData.addDataRef(ENGFF,RWType::ReadOnly,"Array containing ENG FF","sim/flightmodel/engine/ENGN_FF_",PrioType::Middle,1/360,3600);
    floatvectorData.addDataRef(TAI,RWType::ReadOnly,"ENG Anti-ice on per engine","sim/cockpit/switches/anti_ice_engine_air",PrioType::Constant,0.5);
    floatvectorData.addDataRef(ENGREV,RWType::ReadOnly,"Thrust reversers","sim/flightmodel2/engines/thrust_reverser_deploy_ratio",PrioType::Constant,0.1,100);
    floatvectorData.addDataRef(ENGTHRO,RWType::ReadOnly,"Throttle lever percent","sim/flightmodel/engine/ENGN_thro",PrioType::Middle,0.01,100);

    intData.addDataRef(NOENGINES,RWType::ReadOnly,"No of engines","sim/aircraft/engine/acf_num_engines",PrioType::Constant);
    boolData.addDataRef(AVIONICS,RWType::ReadOnly,"Avionics Power","sim/cockpit/electrical/avionics_on",PrioType::Constant);
    boolData.addDataRef(BATTERY,RWType::ReadOnly,"Battery Power","sim/cockpit/electrical/battery_on",PrioType::Constant);
    boolData.addDataRef(ONGROUND,RWType::ReadOnly,"on ground","sim/flightmodel/failures/onground_any",PrioType::Constant);
    boolData.addDataRef(BEACON,RWType::ReadOnly,"Beacon","sim/cockpit/electrical/beacon_lights_on",PrioType::Constant);
    boolData.addDataRef(STROBE,RWType::ReadOnly,"Strobe","sim/cockpit/electrical/strobe_lights_on",PrioType::Constant);
    boolData.addDataRef(LDGLT,RWType::ReadOnly,"LDG Light","sim/cockpit/electrical/landing_lights_on",PrioType::Constant);
    boolData.addDataRef(TAXILT,RWType::ReadOnly,"Taxi Lights","sim/cockpit/electrical/taxi_light_on",PrioType::Constant);
    boolData.addDataRef(NAVLT,RWType::ReadOnly,"Nav Lights","sim/cockpit/electrical/nav_lights_on",PrioType::Constant);
    boolData.addDataRef(PITOTHT,RWType::ReadOnly,"Pitot Heat","sim/cockpit/switches/pitot_heat_on",PrioType::Constant);
    boolData.addDataRef(PAUSE,RWType::ReadOnly,"Sim paused","sim/time/paused",PrioType::Constant);
    floatData.addDataRef(PRKBRK,RWType::ReadOnly,"Park brake deployment","sim/flightmodel/controls/parkbrake",PrioType::Constant,0.1);
    floatvectorData.addDataRef(GEAR,RWType::ReadOnly,"Gear deployment ratio","sim/flightmodel2/gear/deploy_ratio",PrioType::Constant,0.1,100,0,3);
    floatData.addDataRef(FLAPS,RWType::ReadOnly,"FLAPS percent","sim/flightmodel/controls/flaprat",PrioType::Constant,0.09,100);
    floatData.addDataRef(FLAPRQST,RWType::ReadOnly,"FLAP REQUEST percent","sim/flightmodel/controls/flaprqst",PrioType::Constant,0.09,100);
    boolData.addDataRef(THROVRD,RWType::ReadOnly,"Override throttles?","sim/operation/override/override_throttles",PrioType::Low);
    floatData.addDataRef(THROVRDPOS,RWType::ReadOnly,"Position of overriden Thr in sim","sim/flightmodel/engine/ENGN_thro_override",PrioType::Middle);
    intData.addDataRef(FLAPDET,RWType::ReadOnly,"Number on flap lever detents","sim/aircraft/controls/acf_flap_detents",PrioType::Constant);
    floatvectorData.addDataRef(FLAPDETPOS,RWType::ReadOnly,"Positions of the flap lever detents", "sim/aircraft/controls/acf_flap_dn",PrioType::Constant);
    floatData.addDataRef(SPDBRK,RWType::ReadOnly,"Position of speedbrake handle", "sim/cockpit2/controls/speedbrake_ratio",PrioType::Low,0.1);

    Handlers.push_back(new APXPlane9Standard(m_logfile));
    /*Handlers.push_back(new RadioNav(m_logfile));
    Handlers.push_back(new FBWJoystickInterface(m_logfile));*/

    std::vector<LogicHandler*>::iterator it;
    for ( it = Handlers.begin() ; it < Handlers.end() ; it++)
    {
        if (!(*it)->registerDataRefs())
            m_logfile << "XPluginStart: failed to register DataRefs for LogicHandler " << (*it)->name() << std::endl;
    }

    m_logfile << "XPluginStart: Plugin is started" << std::endl;
    return 1;
}

PLUGIN_API void XPluginStop(void)
{
    m_logfile << "XPluginStop:" << std::endl;

    XPLMUnregisterFlightLoopCallback(readHighPrioSimDataCallback, 0);
    XPLMUnregisterFlightLoopCallback(sendQueuedDataCallback, 0);
    XPLMUnregisterFlightLoopCallback(readDataCallback, 0);

    std::vector<LogicHandler*>::iterator it;

    for ( it = Handlers.begin() ; it < Handlers.end() ; it++)
    {
        (*it)->hookMeOff();
        (*it)->unpublishData();
        delete (*it);
        (*it) = 0;
    }
    Handlers.erase(Handlers.begin(),Handlers.end());

    m_logfile << "XPluginStop: success" << std::endl;

    m_logfile.close();
}

PLUGIN_API void XPluginDisable(void)
{
    m_logfile << "XPluginDisable"  << std::endl;
    m_enabled = false;

    shutDown();
    m_logfile << "XPluginDisable: disabled" << std::endl;
}

PLUGIN_API int XPluginEnable(void)
{
    m_logfile << "XPluginEnable" << std::endl;
    setup();
    m_enabled = true;
    m_logfile << "XPluginEnable: success" << std::endl;
    return 1;
}
