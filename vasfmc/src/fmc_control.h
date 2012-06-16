///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2007 Alexander Wemmer 
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

/*! \file    fmc_control.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __FMC_CONTROL_H__
#define __FMC_CONTROL_H__

#include <QObject>
#include <QTimer>

#include "navdata.h"
#include "waypoint.h"
#include "sid.h"
#include "star.h"
#include "flightstatus.h"
#include "fsaccess.h"
#include "declination.h"

#include "fmc_control_defines.h"
#include "fmc_data.h"
#include "fmc_processor.h"

class Config;
class FMCData;
class FMCAutopilot;
class FMCAutothrottle;
class ProjectionBase;
class GeoData;
class ConfigWidgetProvider;
class OpenGLText;
class FlightStatusCheckerBase;
class AircraftData;
class FMCGPSHandler;
class FMCFCUHandler;
class FMCCDUHandler;
class FMCPFDHandler;
class FMCNavdisplayHandler;
class FMCECAMHandler;
class CPFlightSerial;
class IOCPServer;
class FMCSoundsHandler;
class BankController;
class PitchController;
class TransportLayerTCPClient;
class TransportLayerTCPServer;
class FlightModeTracker;
class ChecklistManager;
class NoiseGenerator;

/////////////////////////////////////////////////////////////////////////////

//! FMC Control
class FMCControl : public QObject, FlightStatusProvider, FSAccessProvider
{
    Q_OBJECT

public:

    enum SYNC_DATA_TYPE { SYNC_DATA_TYPE_NORMAL_ROUTE = 1,
                          SYNC_DATA_TYPE_TEMPORARY_ROUTE = 2,
                          SYNC_DATA_TYPE_ALTERNATE_ROUTE = 3,
                          SYNC_DATA_TYPE_SECONDARY_ROUTE = 4,
                          SYNC_DATA_TYPE_AUTOTHROTTLE = 100,
                          SYNC_DATA_TYPE_AUTOPILOT = 200,
                          SYNC_DATA_TYPE_FMCDATA = 300
    };

    //! Standard Constructor
    FMCControl(ConfigWidgetProvider* config_widget_provider, 
               Config* cfg, 
               const QString& control_cfg_filename);

    //! Destructor
    virtual ~FMCControl();

    //-----
    
    //! access to the geometrical database
    inline FMCData& fmcData() { return *m_fmc_data; }
    //! const access to the geometrical database
    inline const FMCData& fmcData() const { return *m_fmc_data; }

    const FMCProcessor* fmcProcessor() const { return m_fmc_processor; }

    const FlightModeTracker& flightModeTracker() const { return *m_flight_mode_tracker; }

    //----- GL font

    OpenGLText* getGLFont() const;
    void setGLFontSize(uint size) { emit signalSetGLFontSize(size); }
    int glFontSize() const;
    int glFontIndex() const;

    //----- PFD

    void setPFDLeftHandler(FMCPFDHandler* pfd_left_handler) { m_pfd_left_handler = pfd_left_handler; }
    void setPFDRightHandler(FMCPFDHandler* pfd_right_handler) { m_pfd_right_handler = pfd_right_handler; }

    //----- ND

    void setNDLeftHandler(FMCNavdisplayHandler* nd_left_handler) { m_nd_left_handler = nd_left_handler; }
    void setNDRightHandler(FMCNavdisplayHandler* nd_right_handler) { m_nd_right_handler = nd_right_handler; }

    //----- ECAM

    void setUpperEcamHandler(FMCECAMHandler* upper_ecam_handler) { m_upper_ecam_handler = upper_ecam_handler; }

    //----- FCU

    void setFCUHandler(FMCFCUHandler* fcu_handler) { m_fcu_handler = fcu_handler; }
    void incFCUFontSize();
    void decFCUFontSize();

    //----- CDU

    void setCDULeftHandler(FMCCDUHandler* cdu_left_handler) { m_cdu_left_handler = cdu_left_handler; }
    void setCDURightHandler(FMCCDUHandler* cdu_right_handler) { m_cdu_right_handler = cdu_right_handler; }
    void incCDUFontSize();
    void decCDUFontSize();

    //----- GPS

    void setGPSHandler(FMCGPSHandler* gps_handler) { m_gps_handler = gps_handler; }
    void incGPSFontSize();
    void decGPSFontSize();

    //----- SOUND

    void setSoundHandler(FMCSoundsHandler* fmc_sounds_handler);

    //-----

    //! access to the main config
    inline Config& mainConfig() { return *m_main_config; }
    //! access to the main config
    inline const Config& mainConfig() const { return *m_main_config; }

    //! gives access to the flight status
    virtual const FlightStatus* flightStatus() const { return m_flightstatus; }
    inline FlightStatus* flightStatus() { return m_flightstatus; }

    //! give access to the flightsim. all modules should use this method
    //! because the access module may change during runtime.
    virtual FSAccess& fsAccess() { return *m_fs_access; }

    //! gives const access to the FMC autopilot
    inline const FMCAutopilot& fmcAutoPilot() const { return *m_fmc_autopilot; }
    //! gives access to the FMC autopilot
    inline FMCAutopilot& fmcAutoPilot() { return *m_fmc_autopilot; }

    //! gives const access to the FMC autothrottle
    inline const FMCAutothrottle& fmcAutothrottle() const { return *m_fmc_autothrottle; }
    //! gives access to the FMC autothrottle
    inline FMCAutothrottle& fmcAutothrottle() { return *m_fmc_autothrottle; }

    //! gives const access to the current flight status checker
    inline const FlightStatusCheckerBase& flightStatusChecker() { return *m_flight_status_checker; }

    //! returns a const pointer to the used LAT/LON -> X/Y projection
    inline const ProjectionBase* projection() const { return m_fmc_processor->projection(); }

    //! const access to the navigation database
    inline const Navdata& navdata() const { return *m_navdata; }

    //! access to the geometrical database
    inline GeoData& geoData() { return *m_geodata; }
    //! const access to the geometrical database
    inline const GeoData& geoData() const { return *m_geodata; }

    //-----

    void recalcFlightstatus();

    void switchToMSFS();
    void switchToXPlane();
    void switchToFGFS();

    bool isMSFSActive() const;

    void setupFlightStatusChecker();

    //TODO move to FMCAutopilot!!
    BankController* bankController() const { return m_bank_controller; }
    PitchController* pitchController() const { return m_pitch_controller; }

    const AircraftData& aircraftData() const { return *m_aircraft_data; }
    const QString aircraftDataPath() const;
    bool loadAircraftData(const QString& filename);

    const ChecklistManager& checklistManager() const { return *m_checklist_manager; }
    ChecklistManager& checklistManager() { return *m_checklist_manager; }

    bool isAirbusFlapsHandlingModeEnabled() const
    { return m_control_cfg->getIntValue(CFG_ENABLE_AIRBUS_FLAP_HANDLING_MODE) != 0; }
    void setAirbusFlapsHandlingMode(bool yes)
    { m_control_cfg->setValue(CFG_ENABLE_AIRBUS_FLAP_HANDLING_MODE, yes ? 1 : 0); }

    bool isSeperateThrottleLeverInputModeEnabled() const 
    { return m_control_cfg->getIntValue(CFG_ENABLE_SEPARATE_THROTTLE_LEVER_INPUTS) != 0; }
    void setSeparateThrottleLeverInputMode(bool yes)
    { m_control_cfg->setValue(CFG_ENABLE_SEPARATE_THROTTLE_LEVER_INPUTS, yes ? 1 : 0); }

    //-----

    bool allowFPLoad() const;

    inline FlightRoute& normalRoute() { return m_fmc_data->normalRoute(); }
    inline const FlightRoute& normalRoute() const { return m_fmc_data->normalRoute(); }

    inline const FlightRoute& alternateRoute() const { return m_fmc_data->alternateRoute(); }
    inline FlightRoute& alternateRoute() { return m_fmc_data->alternateRoute(); }

    inline const FlightRoute& secondaryRoute() const { return m_fmc_data->secondaryRoute(); }
    inline FlightRoute& secondaryRoute() { return m_fmc_data->secondaryRoute(); }

    inline const FlightRoute& temporaryRoute() const { return m_fmc_data->temporaryRoute(); }
    inline FlightRoute& temporaryRoute() { return m_fmc_data->temporaryRoute(); }

    //----- navdata stuff

    //! Searches for an airway from "from_waypoint" to "to_waypoint" and adds all waypoints to
    //! "result_wpt_list". Returns true on success, false otherwise.
    bool getWaypointsByAirway(const Waypoint& from_waypoint,
                              const QString& airway,
                              const QString& to_waypoint,
                              WaypointPtrList& result_wpt_list,
                              QString& error_text);

    //! returns the PBD waypoint for the given values
    Waypoint getPBDWaypoint(const Waypoint& ref_wpt, double mag_bearing, double distance_nm);

    //----- other FMC control stuff

    QString transponderHandlingModeString() const;
    void setTransponderHandlingDisabled()
    { m_control_cfg->setValue(CFG_SQUAWKBOX_HANDLING_MODE, CFG_SQUAWKBOX_HANDLING_MODE_DISABLED); }
    void setTransponderHandlingAutomatic()
    { m_control_cfg->setValue(CFG_SQUAWKBOX_HANDLING_MODE, CFG_SQUAWKBOX_HANDLING_MODE_AUTOMATIC); }
    void setTransponderHandlingOff()
    { m_control_cfg->setValue(CFG_SQUAWKBOX_HANDLING_MODE, CFG_SQUAWKBOX_HANDLING_MODE_OFF); }
    void setTransponderHandlingOn()
    { m_control_cfg->setValue(CFG_SQUAWKBOX_HANDLING_MODE, CFG_SQUAWKBOX_HANDLING_MODE_ON); }
    void setTransponderIdent() { fsAccess().setSBoxIdent(); }

    bool isAltimeterSetToSTD(bool left_nd) const 
    { return (left_nd ? m_control_cfg->getIntValue(CFG_ALTIMETER_LEFT_IS_SET_TO_STD) != 0 :
              m_control_cfg->getIntValue(CFG_ALTIMETER_RIGHT_IS_SET_TO_STD) != 0);}
    
    bool showAltHpa(bool left_nd) const 
    { return (left_nd ? m_control_cfg->getIntValue(CFG_ALTIMETER_LEFT) == ALTIMETER_HPA :
              m_control_cfg->getIntValue(CFG_ALTIMETER_RIGHT) == ALTIMETER_HPA); }
    
    void toggleAltimeterPressure(bool left_nd);

    void changeAltimeterSetting(bool left_pfd, int diff);
    
    bool showMetricAlt() const { return m_control_cfg->getIntValue(CFG_SHOW_METRIC_ALT) != 0; }

    bool showPFDILS(bool left_nd) const 
    { return (left_nd ? m_control_cfg->getIntValue(CFG_PFD_LEFT_DISPLAY_ILS) :
              m_control_cfg->getIntValue(CFG_PFD_RIGHT_DISPLAY_ILS)); }
    
    void setShowPFDILS(bool left_nd, bool yes) 
    { left_nd ? m_control_cfg->setValue(CFG_PFD_LEFT_DISPLAY_ILS, (int)(yes?1:0)) :
            m_control_cfg->setValue(CFG_PFD_RIGHT_DISPLAY_ILS, (int)(yes?1:0)); }

    bool showSurroundingAirports(bool left_nd) const 
    { return (left_nd ? m_control_cfg->getIntValue(CFG_DRAW_LEFT_SURROUNDING_AIRPORTS) != 0 :
              m_control_cfg->getIntValue(CFG_DRAW_RIGHT_SURROUNDING_AIRPORTS) != 0); }

    bool showSurroundingVORs(bool left_nd) const 
    { return (left_nd ? m_control_cfg->getIntValue(CFG_DRAW_LEFT_SURROUNDING_VORS) != 0 :
              m_control_cfg->getIntValue(CFG_DRAW_RIGHT_SURROUNDING_VORS) != 0); }

    bool showSurroundingNDBs(bool left_nd) const 
    { return (left_nd ? m_control_cfg->getIntValue(CFG_DRAW_LEFT_SURROUNDING_NDBS) != 0 :
              m_control_cfg->getIntValue(CFG_DRAW_RIGHT_SURROUNDING_NDBS) != 0); }

    bool showGeoData() const { return m_control_cfg->getIntValue(CFG_DRAW_GEODATA) != 0; }

    void toggleSurroundingAirports(bool left_nd)
    { (left_nd ? m_control_cfg->getIntValue(CFG_DRAW_LEFT_SURROUNDING_AIRPORTS) == 0 :
       m_control_cfg->getIntValue(CFG_DRAW_RIGHT_SURROUNDING_AIRPORTS) == 0) ? 
            slotShowSurroundingAirports(left_nd) : slotHideSurroundingAirports(left_nd); }

    void toggleSurroundingVORs(bool left_nd)
    { (left_nd ? m_control_cfg->getIntValue(CFG_DRAW_LEFT_SURROUNDING_VORS) == 0 :
       m_control_cfg->getIntValue(CFG_DRAW_RIGHT_SURROUNDING_VORS) == 0) ? 
            slotShowSurroundingVors(left_nd) : slotHideSurroundingVors(left_nd); }

    void toggleSurroundingNDBs(bool left_nd)
    { (left_nd ? m_control_cfg->getIntValue(CFG_DRAW_LEFT_SURROUNDING_NDBS) == 0 :
       m_control_cfg->getIntValue(CFG_DRAW_RIGHT_SURROUNDING_NDBS) == 0) ? 
            slotShowSurroundingNdbs(left_nd) : slotHideSurroundingNdbs(left_nd); }

    void toggleGeodata()
    { m_control_cfg->getIntValue(CFG_DRAW_GEODATA) == 0 ? slotShowGeoData() : slotHideGeoData(); }

    uint getNDRangeNM(bool left_nd) const { 
        return (left_nd ? (uint)m_control_cfg->getIntValue(CFG_ND_LEFT_DISPLAY_RANGE_NM) :
                (uint)m_control_cfg->getIntValue(CFG_ND_RIGHT_DISPLAY_RANGE_NM)); }

    void setupNDRangesList();
    //! call to increace/decrease the range by one step
    void nextNDRange(bool left_nd, bool increase);
    //! set the range to value of the given index
    void setNDRange(bool left_nd, uint range_index);

    int currentNDMode(bool left_nd) const 
    { return (left_nd ? m_control_cfg->getIntValue(CFG_ND_LEFT_DISPLAY_MODE) :
              m_control_cfg->getIntValue(CFG_ND_RIGHT_DISPLAY_MODE)); }
    //! call to switch to next/prev ND mode
    void nextNDMode(bool left_nd, bool next);

    uint currentNDNavaidPointerLeft(bool left_nd) const 
    { return (left_nd ? m_control_cfg->getIntValue(CFG_ND_LEFT_LEFT_NAVAID_POINTER_TYPE) :
              m_control_cfg->getIntValue(CFG_ND_RIGHT_LEFT_NAVAID_POINTER_TYPE)); }
    uint currentNDNavaidPointerRight(bool left_nd) const 
    { return (left_nd ? m_control_cfg->getIntValue(CFG_ND_LEFT_RIGHT_NAVAID_POINTER_TYPE) :
              m_control_cfg->getIntValue(CFG_ND_RIGHT_RIGHT_NAVAID_POINTER_TYPE)); }

    //! call to switch left/right navaid pointer
    void nextNDNavaidPointer(bool left_nd, bool left);
    //! set the navaid pointer to the given type
    void setNDNavaidPointer(bool left_nd, bool left, uint type);

    bool soundsEnabled() const { return m_control_cfg->getIntValue(CFG_SOUNDS_ENABLED) != 0; }
    void enableSounds() { m_control_cfg->setValue(CFG_SOUNDS_ENABLED, 1); }
    void disableSounds() { m_control_cfg->setValue(CFG_SOUNDS_ENABLED, 0); }

    bool soundChannelsEnabled() const { return m_control_cfg->getIntValue(CFG_SOUND_CHANNELS_ENABLED) != 0; }
    void enableSoundChannels() { m_control_cfg->setValue(CFG_SOUND_CHANNELS_ENABLED, 1); }
    void disableSoundChannels() { m_control_cfg->setValue(CFG_SOUND_CHANNELS_ENABLED, 0); }

    bool timeSyncEnabled() const { return m_control_cfg->getIntValue(CFG_SYNC_CLOCK_TIME) != 0; }
    bool dateSyncEnabled() const { return m_control_cfg->getIntValue(CFG_SYNC_CLOCK_DATE) != 0; }
    bool fmcAutothrustEnabled() const { return m_control_cfg->getIntValue(CFG_FMC_AUTOTHRUST_ENABLED) != 0; }
    bool showInputAreas() const { return m_control_cfg->getIntValue(CFG_SHOW_INPUTAREAS) != 0; }
    bool showGeoDataFilled() const { return m_control_cfg->getIntValue(CFG_SHOW_GEO_DATA_FILLED) != 0; }

    bool showConstrains(bool left) const 
    { return (left ? m_control_cfg->getIntValue(CFG_SHOW_LEFT_CONSTRAINS) != 0 :
              m_control_cfg->getIntValue(CFG_SHOW_RIGHT_CONSTRAINS) != 0); }

    bool fcuLeftOnlyMode() const { return m_control_cfg->getIntValue(CFG_FCU_LEFT_ONLY_MODE) != 0; }
    bool cduDisplayOnlyMode() const { return m_control_cfg->getIntValue(CFG_CDU_DISPLAY_ONLY_MODE) != 0; }

    bool cduNormalScrollMode() const;
    void setCduScrollModeNormal();
    void setCduScrollModeInverse();

    bool ndNormalScrollMode() const;
    void setNdScrollModeNormal();
    void setNdScrollModeInverse();

    bool ndWindCorrection() const;
    void toggleNdWindCorrection();

    bool fbwEnabled() const { return m_control_cfg->getIntValue(CFG_FBW_ENABLED) != 0; }

    bool isTCASOn() const;
    bool isTCASStandby() const;
    bool isTCASOff() const;
    void toggleTCAS();

    void startPushBack(uint pushback_dist_before_turn_m,
                       bool pushback_turn_direction_clockwise,
                       uint pushback_turn_degrees,
                       uint pushback_dist_after_turn_m);

    void stopPushBack();

    double getAdf1Noise();
    double getAdf2Noise();
    double getVor1Noise();
    double getVor2Noise();
    double getIls1Noise();
    double getIls2Noise();

    //----- external interfaces

    bool useCPFlight() const { return m_control_cfg->getIntValue(CFG_USE_CPFLIGHT_SERIAL) != 0; }
    bool useIOCPServer() const { return m_control_cfg->getIntValue(CFG_USE_IOCP_SERVER) != 0; }

    //! processes an the given command which has the syntax  UNIT:OFFSET.
    //! Returns true when the CMD was handled successfully.
    bool processExternalCMD(const QString& cmd_code);

    //----- IRS

    void setIRSAlignNeeded(bool yes) { m_is_irs_aligned = yes; }
    bool isIRSAlignNeeded() const { return m_is_irs_aligned; }

    //----- missed app mcdu scroll

    inline void setMissedAppWaypointVisibleOnCDU(bool yes, bool left)
    { (left ? m_missed_approach_visible_on_cdu_left : m_missed_approach_visible_on_cdu_right) = yes; }

    inline bool isMissedAppWaypointVisibleOnCDU(bool left) const
    { return (left ? m_missed_approach_visible_on_cdu_left : m_missed_approach_visible_on_cdu_right); }

    //----- FMC connect mode

    QString getFMCConnectMode() const { return m_control_cfg->getValue(CFG_FMC_CONNECT_MODE); }

    bool isFMCConnectModeSingle() const 
    { return m_control_cfg->getValue(CFG_FMC_CONNECT_MODE) == CFG_FMC_CONNECT_MODE_SINGLE; }
    bool isFMCConnectModeMaster() const 
    { return m_control_cfg->getValue(CFG_FMC_CONNECT_MODE) == CFG_FMC_CONNECT_MODE_MASTER; }
    bool isFMCConnectModeSlave() const 
    { return m_control_cfg->getValue(CFG_FMC_CONNECT_MODE) == CFG_FMC_CONNECT_MODE_SLAVE; }

    void setFMCConnectModeSingle() { m_control_cfg->setValue(CFG_FMC_CONNECT_MODE, CFG_FMC_CONNECT_MODE_SINGLE); }
    void setFMCConnectModeMaster() { m_control_cfg->setValue(CFG_FMC_CONNECT_MODE, CFG_FMC_CONNECT_MODE_MASTER); }
    void setFMCConnectModeSlave() { m_control_cfg->setValue(CFG_FMC_CONNECT_MODE, CFG_FMC_CONNECT_MODE_SLAVE); }

    void switchToNextFMCConnectMode()
    {
        if (isFMCConnectModeSingle()) setFMCConnectModeMaster();
        else if (isFMCConnectModeMaster()) setFMCConnectModeSlave();
        else if (isFMCConnectModeSlave()) setFMCConnectModeSingle();
        else setFMCConnectModeSingle();
    }

    void forceFMCConnectRestart() { m_last_fmc_connect_mode.clear(); }

    int getFMCConnectModeMasterPort() const 
    { return m_control_cfg->getIntValue(CFG_FMC_CONNECT_MODE_MASTER_SERVER_PORT); }
    QString getFMCConnectModeSlaveMasterIP() const 
    { return m_control_cfg->getValue(CFG_FMC_CONNECT_MODE_SLAVE_MASTER_IP); }
    int getFMCConnectModeSlaveMasterPort() const 
    { return m_control_cfg->getIntValue(CFG_FMC_CONNECT_MODE_SLAVE_MASTER_PORT); }

    void setFMCConnectModeMasterPort(int port)
    {
        forceFMCConnectRestart();
        return m_control_cfg->setValue(CFG_FMC_CONNECT_MODE_MASTER_SERVER_PORT, port); 
    }

    void setFMCConnectModeSlaveMasterIP(const QString& ip)
    {
        forceFMCConnectRestart();
        return m_control_cfg->setValue(CFG_FMC_CONNECT_MODE_SLAVE_MASTER_IP, ip); 
    }

    void setFMCConnectModeSlaveMasterPort(int port)
    {
        forceFMCConnectRestart();
        return m_control_cfg->setValue(CFG_FMC_CONNECT_MODE_SLAVE_MASTER_PORT, port); 
    }

    bool isFMCConnectRemoteConnected() const;

    int getFMCConnectModeMasterNrClients() const;

    bool isVrouteAiracRestrictionSet() const { return m_control_cfg->getIntValue(CFG_VROUTE_AIRAC_RESTRICTION) != 0; }
    void setVrouteAiracRestriction(bool yes) { m_control_cfg->setValue(CFG_VROUTE_AIRAC_RESTRICTION, yes ? 1 : 0); }

    bool areAircraftDataConfirmed() const { return m_aircraft_data_confirmed; }
    void setAircraftDataConfirmed() { m_aircraft_data_confirmed = true; }

signals:

    void signalSetGLFontSize(uint);
    void signalDataChanged(const QString& flag);
    void signalGeoDataChanged();
    void signalStyleA();
    void signalStyleB();
    void signalStyleG();
    void signalFcuLeftOnlyModeChanged();
    void signalTimeUsed(const QString& name, uint millisecs);
    void signalRestartFMC();
    void signalRestartCDU();

public slots:

    void slotSetAltimeterIsSetToSTD(bool left_nd, bool std) 
    { left_nd ? m_control_cfg->setValue(CFG_ALTIMETER_LEFT_IS_SET_TO_STD, std) :
            m_control_cfg->setValue(CFG_ALTIMETER_RIGHT_IS_SET_TO_STD, std); }

    void slotShowAltimterHPA(bool left_nd) 
    { left_nd ? m_control_cfg->setValue(CFG_ALTIMETER_LEFT, ALTIMETER_HPA) :
            m_control_cfg->setValue(CFG_ALTIMETER_RIGHT, ALTIMETER_HPA); }

    void slotShowAltimterInches(bool left_nd) 
    { left_nd ? m_control_cfg->setValue(CFG_ALTIMETER_LEFT, ALTIMETER_INCHES) :
            m_control_cfg->setValue(CFG_ALTIMETER_RIGHT, ALTIMETER_INCHES); }

    void slotToggleMetricAlt() 
    { m_control_cfg->setValue(CFG_SHOW_METRIC_ALT, m_control_cfg->getIntValue(CFG_SHOW_METRIC_ALT) == 0); }

    void slotShowMetricAlt()
    { m_control_cfg->setValue(CFG_SHOW_METRIC_ALT, 1); }

    void slotHideMetricAlt()
    { m_control_cfg->setValue(CFG_SHOW_METRIC_ALT, 0); }

    void slotToggleShowPFDILS(bool left_nd) 
    { left_nd ? m_control_cfg->setValue(CFG_PFD_LEFT_DISPLAY_ILS,m_control_cfg->getIntValue(CFG_PFD_LEFT_DISPLAY_ILS) == 0) :
            m_control_cfg->setValue(CFG_PFD_RIGHT_DISPLAY_ILS,m_control_cfg->getIntValue(CFG_PFD_RIGHT_DISPLAY_ILS) == 0); }

    void slotShowSurroundingAirports(bool left_nd) 
    { left_nd ? m_control_cfg->setValue(CFG_DRAW_LEFT_SURROUNDING_AIRPORTS, 1) :
            m_control_cfg->setValue(CFG_DRAW_RIGHT_SURROUNDING_AIRPORTS, 1); }

    void slotHideSurroundingAirports(bool left_nd) 
    { left_nd ? m_control_cfg->setValue(CFG_DRAW_LEFT_SURROUNDING_AIRPORTS, 0) :
            m_control_cfg->setValue(CFG_DRAW_RIGHT_SURROUNDING_AIRPORTS, 0); }

    void slotShowSurroundingVors(bool left_nd) 
    { left_nd ? m_control_cfg->setValue(CFG_DRAW_LEFT_SURROUNDING_VORS, 1) :
            m_control_cfg->setValue(CFG_DRAW_RIGHT_SURROUNDING_VORS, 1); }

    void slotHideSurroundingVors(bool left_nd) 
    { left_nd ? m_control_cfg->setValue(CFG_DRAW_LEFT_SURROUNDING_VORS, 0) :
            m_control_cfg->setValue(CFG_DRAW_RIGHT_SURROUNDING_VORS, 0); }

    void slotShowSurroundingNdbs(bool left_nd) 
    { left_nd ? m_control_cfg->setValue(CFG_DRAW_LEFT_SURROUNDING_NDBS, 1) :
            m_control_cfg->setValue(CFG_DRAW_RIGHT_SURROUNDING_NDBS, 1); }

    void slotHideSurroundingNdbs(bool left_nd) 
    { left_nd ? m_control_cfg->setValue(CFG_DRAW_LEFT_SURROUNDING_NDBS, 0) :
            m_control_cfg->setValue(CFG_DRAW_RIGHT_SURROUNDING_NDBS, 0); }

    void slotShowGeoData() { m_control_cfg->setValue(CFG_DRAW_GEODATA, 1); }
    void slotHideGeoData() { m_control_cfg->setValue(CFG_DRAW_GEODATA, 0); }

    void slotEnableNDModeNavArc(bool left_nd)
    { left_nd ? m_control_cfg->setValue(CFG_ND_LEFT_DISPLAY_MODE, CFG_ND_DISPLAY_MODE_NAV_ARC) :
            m_control_cfg->setValue(CFG_ND_RIGHT_DISPLAY_MODE, CFG_ND_DISPLAY_MODE_NAV_ARC); }
    void slotEnableNDModeNavRose(bool left_nd) 
    { left_nd ? m_control_cfg->setValue(CFG_ND_LEFT_DISPLAY_MODE, CFG_ND_DISPLAY_MODE_NAV_ROSE) :
            m_control_cfg->setValue(CFG_ND_RIGHT_DISPLAY_MODE, CFG_ND_DISPLAY_MODE_NAV_ROSE); }
    void slotEnableNDModePlanRose(bool left_nd) 
    {
        left_nd ? m_control_cfg->setValue(CFG_ND_LEFT_DISPLAY_MODE, CFG_ND_DISPLAY_MODE_NAV_PLAN) :
            m_control_cfg->setValue(CFG_ND_RIGHT_DISPLAY_MODE, CFG_ND_DISPLAY_MODE_NAV_PLAN); 
        
        //TODO override this for now, fix the PLAN display on the right ND later
        if (!left_nd) m_control_cfg->setValue(CFG_ND_RIGHT_DISPLAY_MODE, CFG_ND_DISPLAY_MODE_NAV_ARC);
    }
    void slotEnableNDModeVorRose(bool left_nd)  
    { left_nd ? m_control_cfg->setValue(CFG_ND_LEFT_DISPLAY_MODE, CFG_ND_DISPLAY_MODE_VOR_ROSE) : 
            m_control_cfg->setValue(CFG_ND_RIGHT_DISPLAY_MODE, CFG_ND_DISPLAY_MODE_VOR_ROSE); }
    void slotEnableNDModeIlsRose(bool left_nd)  
    { left_nd ? m_control_cfg->setValue(CFG_ND_LEFT_DISPLAY_MODE, CFG_ND_DISPLAY_MODE_ILS_ROSE) : 
            m_control_cfg->setValue(CFG_ND_RIGHT_DISPLAY_MODE, CFG_ND_DISPLAY_MODE_ILS_ROSE); }

    void slotNDNoLeftPointerAction(bool left_nd) 
    { left_nd ? m_control_cfg->setValue(CFG_ND_LEFT_LEFT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_OFF) : 
            m_control_cfg->setValue(CFG_ND_RIGHT_LEFT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_OFF); }
    void slotNDAdf1PointerAction(bool left_nd) 
    { left_nd ? m_control_cfg->setValue(CFG_ND_LEFT_LEFT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_NDB) :
            m_control_cfg->setValue(CFG_ND_RIGHT_LEFT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_NDB); }
    void slotNDVor1PointerAction(bool left_nd) 
    { left_nd ? m_control_cfg->setValue(CFG_ND_LEFT_LEFT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_VOR) : 
            m_control_cfg->setValue(CFG_ND_RIGHT_LEFT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_VOR); }
    void slotNDNoRightPointerAction(bool left_nd) 
    { left_nd ? m_control_cfg->setValue(CFG_ND_LEFT_RIGHT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_OFF) :
            m_control_cfg->setValue(CFG_ND_RIGHT_RIGHT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_OFF); }
    void slotNDAdf2PointerAction(bool left_nd) 
    { left_nd ? m_control_cfg->setValue(CFG_ND_LEFT_RIGHT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_NDB) :
            m_control_cfg->setValue(CFG_ND_RIGHT_RIGHT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_NDB); }
    void slotNDVor2PointerAction(bool left_nd) 
    { left_nd ? m_control_cfg->setValue(CFG_ND_LEFT_RIGHT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_VOR) :
            m_control_cfg->setValue(CFG_ND_RIGHT_RIGHT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_VOR); }

    void slotToggleSounds() { m_control_cfg->setValue(CFG_SOUNDS_ENABLED, !soundsEnabled()); }
    void slotToggleSoundChannels() { m_control_cfg->setValue(CFG_SOUND_CHANNELS_ENABLED, !soundChannelsEnabled()); }

    void slotToggleTimeSync() { m_control_cfg->setValue(CFG_SYNC_CLOCK_TIME, !timeSyncEnabled()); }
    void slotToggleDateSync() { m_control_cfg->setValue(CFG_SYNC_CLOCK_DATE, !dateSyncEnabled()); }

    //TODO move to FMCAutothrottle class
    void slotToggleFmcAutothrustEnabled();
    
    void slotToggleShowInputAreas() { m_control_cfg->setValue(CFG_SHOW_INPUTAREAS, !showInputAreas()); }
    
    void slotToggleShowGeoDataFilled() { m_control_cfg->setValue(CFG_SHOW_GEO_DATA_FILLED, !showGeoDataFilled()); }

    void slotShowConstrains(bool left) 
    { left ? m_control_cfg->setValue(CFG_SHOW_LEFT_CONSTRAINS, 1) : 
            m_control_cfg->setValue(CFG_SHOW_RIGHT_CONSTRAINS, 1); }
    void slotHideConstrains(bool left) 
    { left ? m_control_cfg->setValue(CFG_SHOW_LEFT_CONSTRAINS, 0) : 
            m_control_cfg->setValue(CFG_SHOW_RIGHT_CONSTRAINS, 0); }
    void slotToggleShowConstrains(bool left) 
    { left ? m_control_cfg->setValue(CFG_SHOW_LEFT_CONSTRAINS, !showConstrains(left)) :
            m_control_cfg->setValue(CFG_SHOW_RIGHT_CONSTRAINS, !showConstrains(left)); }
    
    void slotToggleFcuLeftOnlyMode() 
    {
        m_control_cfg->setValue(CFG_FCU_LEFT_ONLY_MODE, !fcuLeftOnlyMode()); 
        emit signalFcuLeftOnlyModeChanged();
    }

    void slotToggleCduDisplayOnlyMode(bool only_when_already_display_only)
    {
        if (only_when_already_display_only && !cduDisplayOnlyMode()) return;
        m_control_cfg->setValue(CFG_CDU_DISPLAY_ONLY_MODE, (!cduDisplayOnlyMode()) ? "1" : "0"); 
        emit signalRestartCDU();
    }

    void slotPreviousChecklistItem();
    void slotNextChecklistItem();

    void slotShowFbw() { m_control_cfg->setValue(CFG_FBW_ENABLED, 1); }
    void slotHideFbw() { m_control_cfg->setValue(CFG_FBW_ENABLED, 0); }
    void slotToggleFbw() { m_control_cfg->setValue(CFG_FBW_ENABLED, !fbwEnabled()); }    

    void slotToggleUseCPFlight();
    void slotToggleUseIOCPServer();

    //----- refresh rates

    uint getPFDNDRefreshRateMs() const { return m_control_cfg->getIntValue(CFG_PFDND_REFRESH_PERIOD_MS); }
    void setPFDNDRefreshRateMs(uint ms) 
    { m_control_cfg->setValue(CFG_PFDND_REFRESH_PERIOD_MS, LIMITMINMAX((int)ms, 25, 1000)); }

    uint getECAMRefreshRateMs() const { return m_control_cfg->getIntValue(CFG_ECAM_REFRESH_PERIOD_MS); }
    void setECAMRefreshRateMs(uint ms) 
    { m_control_cfg->setValue(CFG_ECAM_REFRESH_PERIOD_MS, LIMITMINMAX((int)ms, 25, 1000)); }

    uint getCDUFCURefreshRateMs() const { return m_control_cfg->getIntValue(CFG_CDUFCU_REFRESH_PERIOD_MS); }
    void setCDUFCURefreshRateMs(uint ms) 
    { m_control_cfg->setValue(CFG_CDUFCU_REFRESH_PERIOD_MS, LIMITMINMAX((int)ms, 100, 5000)); }

    bool showFps() const { return m_control_cfg->getIntValue(CFG_SHOW_FPS) != 0; }
    void setShowFps(bool yes) { m_control_cfg->setValue(CFG_SHOW_FPS, yes ? 1 : 0); }    

    bool doKeepOnTop() const { return m_control_cfg->getIntValue(CFG_KEEP_ON_TOP) != 0; }
    void setKeepOnTop(bool yes);
    void toggleKeepOnTop() { setKeepOnTop(!doKeepOnTop()); }    

protected slots:

    void slotCentralTimer();

    void slotControlTimer();
    void slotDataChanged(const QString& routeflag, bool direct_change, const QString& comment);

    void slotPushBackTimer();

    void slotRemoteFMCConnected();
    void slotReceivedRemoteFMCData(qint16 data_type, QByteArray& data);

    void slotApproachPhaseActivated();

protected:

    void setupDefaultConfig();

    //! Checks if the given waypoint ID specifies an overfly waypoint.
    //! If so, the waypoint_id will be altered and true will be returned, false otherwise.
    bool checkForOverflyWaypoint(QString& waypoint_id);

    void setupFsAccess();

    void syncDateTime();

    void calcNoiseLimits();

protected:
    
    //! the global opengl font
    mutable OpenGLText* m_gl_font;

    //! fmc control config
    Config* m_control_cfg;

    //! config_widget_provider
    ConfigWidgetProvider* m_config_widget_provider;

    //! global config
    Config* m_main_config;

    //! FMC data (routes, etc.)
    FMCData* m_fmc_data;

    //! tracks the current flight mode
    FlightModeTracker* m_flight_mode_tracker;

    //! FMC sounds
    FMCSoundsHandler* m_fmc_sounds_handler;

    //! filename of the persistant route
    QString  m_persistance_filename;

    //! flight status data fed by the flightsim access module
    FlightStatus* m_flightstatus;
    
    //! access to the flight simulator
    FSAccess* m_fs_access;

    //! flight status checker
    FlightStatusCheckerBase* m_flight_status_checker;
    int m_last_flight_status_checker_style;

    //! triggers processing for all submodules
    QTimer m_central_timer;
    
    //! triggers control processing
    QTime m_control_timer;

    //! navdata access
    Navdata* m_navdata;    

    //! geo database
    GeoData* m_geodata;
    
    //! controlling the autopilot of the flightsim
    FMCAutopilot* m_fmc_autopilot;

    //! controlling the autothrottle of the flightsim
    FMCAutothrottle* m_fmc_autothrottle;

    //! combines and calculates FMC values periodically
    FMCProcessor* m_fmc_processor;

    uint m_pbd_counter;

    QTime m_sbox_transponder_timer;

    QTime m_date_time_sync_timer;

    QList<int> m_nd_possible_ranges_nm_list;

    Declination m_declination_calc;

    //TODO move to fmc_autopilot
    BankController* m_bank_controller;
    PitchController* m_pitch_controller;

    AircraftData *m_aircraft_data;
    bool m_aircraft_data_confirmed;

    ChecklistManager *m_checklist_manager;

    FMCPFDHandler* m_pfd_left_handler;
    FMCPFDHandler* m_pfd_right_handler;
    FMCNavdisplayHandler* m_nd_left_handler;
    FMCNavdisplayHandler* m_nd_right_handler;
    FMCGPSHandler* m_gps_handler;
    FMCFCUHandler* m_fcu_handler;
    FMCCDUHandler* m_cdu_left_handler;
    FMCCDUHandler* m_cdu_right_handler;
    FMCECAMHandler* m_upper_ecam_handler;

    //----- pushback

    QTimer m_pushback_timer;
    bool m_stop_bushback;
    Waypoint m_last_pushback_location;
    int m_pushback_start_heading;

    uint m_pushback_dist_before_turn_m;
    bool m_pushback_turn_direction_clockwise;
    int m_pushback_turn_degrees;
    uint m_pushback_dist_after_turn_m;

    //-----

    CPFlightSerial* m_cpflight_serial;
    IOCPServer* m_iocp_server;

    //-----

    bool m_is_irs_aligned;
    bool m_missed_approach_visible_on_cdu_left;
    bool m_missed_approach_visible_on_cdu_right;

    //-----

    QString m_last_fmc_connect_mode;
    TransportLayerTCPClient* m_fmc_connect_slave_tcp_client;
    TransportLayerTCPServer* m_fmc_connect_master_tcp_server;
    QTime m_fmc_connect_master_mode_sync_timer;

    //----- refresh timer

    QTime m_pfdnd_refresh_timer;
    int m_pfdnd_refresh_ms;
    int m_pfdnd_refresh_index;

    QTime m_ecam_refresh_timer;
    int m_ecam_refresh_ms;
    //TODOint m_ecam_refresh_index;

    QTime m_ap_athr_refresh_timer;
    int m_ap_athr_refresh_ms;
    int m_ap_athr_refresh_index;

    QTime m_cdufcu_refresh_timer;
    int m_cdufcu_refresh_ms;
    int m_cdufcu_refresh_index;

    QTime m_fsctrl_poll_timer;
    int m_fsctrl_poll_index;

    // used to gather 
    FMCStatusData m_fmc_status_data;

    NoiseGenerator* m_adf1_noise_generator;
    NoiseGenerator* m_adf2_noise_generator;
    NoiseGenerator* m_vor1_noise_generator;
    NoiseGenerator* m_vor2_noise_generator;
    NoiseGenerator* m_ils1_noise_generator;
    NoiseGenerator* m_ils2_noise_generator;
    QTime m_noise_limit_update_timer;
    Damping m_noise_damping;
    uint m_noise_calc_index;

private:
    //! Hidden copy-constructor
    FMCControl(const FMCControl&);
    //! Hidden assignment operator
    const FMCControl& operator = (const FMCControl&);
};

#endif /* __FMC_CONTROL_H__ */

// End of file

