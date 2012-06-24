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

/*! \file    fmc_control.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include <QRegExp>

#include "defines.h"
#include "assert.h"
#include "config.h"
#include "logger.h"
#include "vas_path.h"

#include "navcalc.h"
#include "airway.h"
#include "holding.h"
#include "fsaccess_xplane.h"
#include "projection.h"
#include "geodata.h"
#include "aircraft_data.h"
#include "checklist.h"
#include "fly_by_wire.h"
#include "opengltext.h"
#include "flight_mode_tracker.h"
#include "noise_generator.h"

#include "fmc_sounds_handler.h"
#include "fmc_autopilot.h"
#include "fmc_autothrottle.h"
#include "fmc_cdu.h"

#include "fmc_flightstatus_checker_style_a.h"

#include "cpflight_serial.h"
#include "iocp.h"
#include "info_server.h"

#include "transport_layer_tcpclient.h"
#include "transport_layer_tcpserver.h"

#include "fmc_control.h"


const static QString DCT_WPT_TEXT = "DCT";
const static QString OVERFLY_WPT_DESIGNATOR = "*";
const static QChar AIRWAY_WAYPOINT_SEPARATOR = '.';

#define DO_TIMER_LOGGING 0

/////////////////////////////////////////////////////////////////////////////

FMCControl::FMCControl(ConfigWidgetProvider* config_widget_provider,
                       Config* cfg,
                       const QString& control_cfg_filename) :
    m_gl_font(0), m_config_widget_provider(config_widget_provider), m_main_config(cfg),
    m_fmc_data(0), m_flight_mode_tracker(0), m_fmc_sounds_handler(0),
    m_flightstatus(new FlightStatus(cfg->getIntValue(CFG_FLIGHTSTATUS_SMOOTHING_DELAY_MS))),
    m_fs_access(0), m_flight_status_checker(0), m_last_flight_status_checker_style(-1),
    m_navdata(0), m_pbd_counter(0), m_declination_calc(cfg->getValue(CFG_DECLINATION_DATAFILE)),
    m_aircraft_data(new AircraftData(m_flightstatus)), m_aircraft_data_confirmed(false),
    m_checklist_manager(0),
    m_cdu_left_handler(0), m_cdu_right_handler(0),
    m_pushback_start_heading(-1), m_pushback_dist_before_turn_m(0), m_pushback_turn_direction_clockwise(false),
    m_pushback_turn_degrees(0), m_pushback_dist_after_turn_m(0), m_cpflight_serial(0), m_iocp_server(0),
    m_is_irs_aligned(false), m_missed_approach_visible_on_cdu_left(false), m_missed_approach_visible_on_cdu_right(false),
    m_fmc_connect_slave_tcp_client(0), m_fmc_connect_master_tcp_server(0),
    m_adf1_noise_generator(0), m_adf2_noise_generator(0), m_vor1_noise_generator(0), m_vor2_noise_generator(0),
    m_ils1_noise_generator(0), m_ils2_noise_generator(0), m_noise_calc_index(0)
{
    MYASSERT(m_config_widget_provider != 0);
    MYASSERT(m_main_config != 0);
    MYASSERT(m_flightstatus != 0);
    m_sbox_transponder_timer.start();
    m_date_time_sync_timer.start();
    MYASSERT(Declination::globalDeclination() != 0);
    MYASSERT(m_aircraft_data != 0);
    m_fmc_connect_master_mode_sync_timer.start();

    m_fmc_data = new FMCData(cfg, m_flightstatus);
    MYASSERT(m_fmc_data != 0);

    m_flight_mode_tracker = new FlightModeTracker(m_flightstatus, m_fmc_data);
    MYASSERT(m_flight_mode_tracker != 0);

    m_checklist_manager = new ChecklistManager(
        m_main_config->getValue(CFG_VASFMC_DIR) + "/" + m_main_config->getValue(CFG_CHECKLIST_SUBDIR) + "/");
    MYASSERT(m_checklist_manager != 0);

    // setup config

    m_control_cfg = new Config(control_cfg_filename);
    MYASSERT(m_control_cfg);
    setupDefaultConfig();
    m_control_cfg->loadfromFile();
    m_control_cfg->saveToFile();
    m_config_widget_provider->registerConfigWidget("Control", m_control_cfg);
    setupNDRangesList();

    // init FBW

    Logger::log("setup bank controller");
    m_bank_controller = new BankController(
        m_flightstatus,
        m_control_cfg->getDoubleValue(CFG_FBW_BANK_P_GAIN),
        m_control_cfg->getDoubleValue(CFG_FBW_BANK_I_GAIN),
        m_control_cfg->getDoubleValue(CFG_FBW_BANK_D_GAIN),
        m_control_cfg->getDoubleValue(CFG_FBW_BANK_MAX_RATE),
        m_control_cfg->getDoubleValue(CFG_FBW_BANK_I_TO_P_RESPONSE_FACTOR),
        m_control_cfg->getDoubleValue(CFG_FBW_BANK_IDLE_LIMIT),
        m_control_cfg->getDoubleValue(CFG_FBW_BANK_FORCED_LIMIT));

    MYASSERT(m_bank_controller != 0);

    Logger::log("setup pitch controller");
    m_pitch_controller = new PitchController(
        m_flightstatus,
        m_control_cfg->getDoubleValue(CFG_FBW_PITCH_P_GAIN),
        m_control_cfg->getDoubleValue(CFG_FBW_PITCH_I_GAIN),
        m_control_cfg->getDoubleValue(CFG_FBW_PITCH_D_GAIN),
        m_control_cfg->getDoubleValue(CFG_FBW_PITCH_MAX_RATE),
        m_control_cfg->getDoubleValue(CFG_FBW_PITCH_GOOD_TREND_DAMPING_FACTOR),
        m_control_cfg->getDoubleValue(CFG_FBW_PITCH_I_TO_P_RESPONSE_FACTOR),
        m_control_cfg->getDoubleValue(CFG_FBW_PITCH_BANK_RATE_BOOST_FACTOR),
        m_control_cfg->getDoubleValue(CFG_FBW_PITCH_STABLE_FPV_DAMP_FACTOR),
        m_control_cfg->getDoubleValue(CFG_FBW_PITCH_TRANSITION_BOOST_FACTOR),
        m_control_cfg->getDoubleValue(CFG_FBW_PITCH_NEG_LIMIT),
        m_control_cfg->getDoubleValue(CFG_FBW_PITCH_POS_LIMIT));

    MYASSERT(m_pitch_controller != 0);

    // init aircraft data

    Logger::log("setup aircraft data");
    MYASSERT(loadAircraftData(m_control_cfg->getValue(CFG_ACFT_DATA_LAST_FILE)));

    // init navdata

    Logger::log("setup navdata");
    m_navdata = new Navdata(CFG_NAVDATA_FILENAME, CFG_NAVDATA_INDEX_FILENAME);
    MYASSERT(m_navdata);

    // init flight status checker

    setupFlightStatusChecker();

    // init geodata
    m_geodata = new GeoData;
    MYASSERT(m_geodata != 0);
    m_geodata->setFilenames(QStringList(m_main_config->getValue(CFG_GEODATA_FILE)));
    m_geodata->readData(m_main_config->getIntValue(CFG_GEODATA_FILTER_LEVEL));
    MYASSERT(connect(m_geodata, SIGNAL(signalActiveRouteChanged()), this, SIGNAL(signalGeoDataChanged())));

    // init flightsim access
    setupFsAccess();

    // init autopilot control
    m_fmc_autopilot = new FMCAutopilot(m_config_widget_provider, m_main_config, CFG_AUTOPILOT_FILENAME, this);
    MYASSERT(m_fmc_autopilot != 0);

    // init autothrottle control
    m_fmc_autothrottle = new FMCAutothrottle(m_config_widget_provider, m_main_config, CFG_AUTOTHROTTLE_FILENAME, this);
    MYASSERT(m_fmc_autothrottle != 0);

    // init processor
    m_fmc_processor = new FMCProcessor(m_config_widget_provider, CFG_PROCESSOR_FILENAME, this, m_navdata);
    MYASSERT(m_fmc_processor != 0);

    // init fmc data

    MYASSERT(connect(m_fmc_data, SIGNAL(signalDataChanged(const QString&, bool, const QString&)),
                     this, SLOT(slotDataChanged(const QString&, bool, const QString&))));

    MYASSERT(connect(m_fmc_data, SIGNAL(signalApproachPhaseActivated()),
                     this, SLOT(slotApproachPhaseActivated())));

    MYASSERT(projection() != 0);
    m_fmc_data->normalRoute().setProjection(projection());
    m_fmc_data->alternateRoute().setProjection(projection());
    m_fmc_data->secondaryRoute().setProjection(projection());
    m_fmc_data->temporaryRoute().setProjection(projection());

    // setup central timer and refresh timers

    MYASSERT(connect(&m_central_timer, SIGNAL(timeout()), this, SLOT(slotCentralTimer())));
    m_central_timer.start(2);

    m_pfdnd_refresh_timer.start();
    m_ecam_refresh_timer.start();
    m_ap_athr_refresh_timer.start();
    m_cdufcu_refresh_timer.start();
    m_fsctrl_poll_timer.start();

    m_pfdnd_refresh_ms = 1000;
    m_ecam_refresh_ms = 1000;
    m_ap_athr_refresh_ms = 1000;
    m_cdufcu_refresh_ms = 1000;

    m_pfdnd_refresh_index = 0;
    m_ap_athr_refresh_index = 0;
    m_cdufcu_refresh_index = 0;
    m_fsctrl_poll_index = 0;

    // setup control timer
    m_control_timer.start();

    MYASSERT(connect(&m_pushback_timer, SIGNAL(timeout()), this, SLOT(slotPushBackTimer())));

    // load persistant route

    m_persistance_filename = m_main_config->getValue(CFG_VASFMC_DIR)+"/"+m_main_config->getValue(CFG_PERSISTANCE_FILE);
    if (QFile::exists(m_persistance_filename))
        if (!m_fmc_data->normalRoute().loadFP(m_persistance_filename, &navdata()))
            Logger::log(QString("FMCControl: could not read persistance file (%1)").arg(m_persistance_filename));

    // init CPFlight serial

    if (useCPFlight())
    {
        m_cpflight_serial = new CPFlightSerial(CFG_CPFLIGHT_FILENAME, m_flightstatus, this);
        MYASSERT(m_cpflight_serial != 0);
    }

    // init IOCP server
    
    if (useIOCPServer())
    {
        m_iocp_server = new IOCPServer(CFG_IOCP_FILENAME, this);
        MYASSERT(m_iocp_server != 0);
    }

    m_info_server = new InfoServer(m_main_config->getValue(CFG_VASFMC_DIR)+"/"+m_main_config->getValue(CFG_PERSISTANCE_FILE));

    // init noise

    m_noise_limit_update_timer.start();

    m_adf1_noise_generator = new NoiseGenerator(m_control_cfg->getIntValue(CFG_NOISE_GENERATION_INTERVAL_MS),
                                                m_control_cfg->getDoubleValue(CFG_ADF_NOISE_INC_LIMIT_DEG),
                                                m_control_cfg->getDoubleValue(CFG_ADF_NOISE_LIMIT_DEG));
    MYASSERT(m_adf1_noise_generator != 0);

    m_adf2_noise_generator = new NoiseGenerator(m_control_cfg->getIntValue(CFG_NOISE_GENERATION_INTERVAL_MS),
                                                m_control_cfg->getDoubleValue(CFG_ADF_NOISE_INC_LIMIT_DEG),
                                                m_control_cfg->getDoubleValue(CFG_ADF_NOISE_LIMIT_DEG));
    MYASSERT(m_adf2_noise_generator != 0);

    m_vor1_noise_generator = new NoiseGenerator(m_control_cfg->getIntValue(CFG_NOISE_GENERATION_INTERVAL_MS),
                                                m_control_cfg->getDoubleValue(CFG_VOR_NOISE_INC_LIMIT_DEG),
                                                m_control_cfg->getDoubleValue(CFG_VOR_NOISE_LIMIT_DEG));

    MYASSERT(m_vor1_noise_generator != 0);

    m_vor2_noise_generator = new NoiseGenerator(m_control_cfg->getIntValue(CFG_NOISE_GENERATION_INTERVAL_MS),
                                                m_control_cfg->getDoubleValue(CFG_VOR_NOISE_INC_LIMIT_DEG),
                                                m_control_cfg->getDoubleValue(CFG_VOR_NOISE_LIMIT_DEG));
    MYASSERT(m_vor2_noise_generator != 0);

    m_ils1_noise_generator = new NoiseGenerator(m_control_cfg->getIntValue(CFG_NOISE_GENERATION_INTERVAL_MS),
                                                m_control_cfg->getDoubleValue(CFG_ILS_NOISE_INC_LIMIT),
                                                m_control_cfg->getDoubleValue(CFG_ILS_NOISE_LIMIT));

    MYASSERT(m_ils1_noise_generator != 0);

    m_ils2_noise_generator = new NoiseGenerator(m_control_cfg->getIntValue(CFG_NOISE_GENERATION_INTERVAL_MS),
                                                m_control_cfg->getDoubleValue(CFG_ILS_NOISE_INC_LIMIT),
                                                m_control_cfg->getDoubleValue(CFG_ILS_NOISE_LIMIT));
    MYASSERT(m_ils2_noise_generator != 0);
}

/////////////////////////////////////////////////////////////////////////////

FMCControl::~FMCControl()
{
    m_central_timer.stop();

    if (!m_fmc_data->normalRoute().saveFP(m_persistance_filename))
        Logger::log(QString("~FMCControl: could not save persistant route to (%1)").
                    arg(m_persistance_filename));

    m_control_cfg->setValue(CFG_FBW_BANK_P_GAIN, m_bank_controller->pGain());
    m_control_cfg->setValue(CFG_FBW_BANK_I_GAIN, m_bank_controller->iGain());
    m_control_cfg->setValue(CFG_FBW_BANK_D_GAIN, m_bank_controller->dGain());
    m_control_cfg->setValue(CFG_FBW_BANK_I_TO_P_RESPONSE_FACTOR, m_bank_controller->IToPPartResponseFactor());
    m_control_cfg->setValue(CFG_FBW_BANK_MAX_RATE, m_bank_controller->maxBankrate());
    m_control_cfg->setValue(CFG_FBW_BANK_IDLE_LIMIT, m_bank_controller->maxIdleBank());
    m_control_cfg->setValue(CFG_FBW_BANK_FORCED_LIMIT, m_bank_controller->maxForcedBank());

    m_control_cfg->setValue(CFG_FBW_PITCH_P_GAIN, m_pitch_controller->pGain());
    m_control_cfg->setValue(CFG_FBW_PITCH_I_GAIN, m_pitch_controller->iGain());
    m_control_cfg->setValue(CFG_FBW_PITCH_D_GAIN, m_pitch_controller->dGain());
    m_control_cfg->setValue(CFG_FBW_PITCH_MAX_RATE, m_pitch_controller->maxPitchrate());
    m_control_cfg->setValue(CFG_FBW_PITCH_NEG_LIMIT, m_pitch_controller->maxNegativePitch());
    m_control_cfg->setValue(CFG_FBW_PITCH_POS_LIMIT, m_pitch_controller->maxPositivePitch());
    m_control_cfg->setValue(CFG_FBW_PITCH_GOOD_TREND_DAMPING_FACTOR, m_pitch_controller->PIDGoodTrendDampingFactor());
    m_control_cfg->setValue(CFG_FBW_PITCH_I_TO_P_RESPONSE_FACTOR, m_pitch_controller->IToPPartResponseFactor());
    m_control_cfg->setValue(CFG_FBW_PITCH_BANK_RATE_BOOST_FACTOR, m_pitch_controller->bankRateBoostFactor());
    m_control_cfg->setValue(CFG_FBW_PITCH_STABLE_FPV_DAMP_FACTOR, m_pitch_controller->stableFPVDampFactor());
    m_control_cfg->setValue(CFG_FBW_PITCH_TRANSITION_BOOST_FACTOR, m_pitch_controller->transitionBoostFactor());


    m_control_cfg->saveToFile();
    delete m_control_cfg;

    delete m_fmc_processor;
    delete m_fmc_autopilot;
    delete m_fmc_autothrottle;
    delete m_fs_access;
    delete m_flight_mode_tracker;
    delete m_flightstatus;
    delete m_fmc_data;
    delete m_navdata;
    delete m_geodata;
    delete m_gl_font;
    delete m_flight_status_checker;
    delete m_bank_controller;
    delete m_pitch_controller;
    delete m_aircraft_data;
    delete m_checklist_manager;
    delete m_cpflight_serial;
    delete m_iocp_server;

    delete m_adf1_noise_generator;
    delete m_adf2_noise_generator;
    delete m_vor1_noise_generator;
    delete m_vor2_noise_generator;
    delete m_ils1_noise_generator;
    delete m_ils2_noise_generator;
}

/////////////////////////////////////////////////////////////////////////////

bool FMCControl::loadAircraftData(const QString& filename)
{
    //TODOm_aircraft_data->writeToFile(m_main_config->getValue(CFG_AIRCRAFT_DATA_SUBDIR) + "/" + "default.cfg");

    if (!m_aircraft_data->readFromFile(m_main_config->getValue(CFG_AIRCRAFT_DATA_SUBDIR) + "/" + filename))
    {
        Logger::log(QString("FMCControl: could not load aircraft data (%1/%2)").
                    arg(m_main_config->getValue(CFG_AIRCRAFT_DATA_SUBDIR)).arg(filename));
        return false;
    }

    checklistManager().clear();
    if (!aircraftData().checklistFilename().isEmpty())
        checklistManager().loadFromFile(aircraftData().checklistFilename());

    m_control_cfg->setValue(CFG_ACFT_DATA_LAST_FILE, filename);
    return true;
}

/////////////////////////////////////////////////////////////////////////////

const QString FMCControl::aircraftDataPath() const
{
    return m_main_config->getValue(CFG_VASFMC_DIR) + "/" + m_main_config->getValue(CFG_AIRCRAFT_DATA_SUBDIR) + "/";
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::setupDefaultConfig()
{
    m_control_cfg->setValue(CFG_PFDND_REFRESH_PERIOD_MS, 50);
    m_control_cfg->setValue(CFG_ECAM_REFRESH_PERIOD_MS, 200);
    m_control_cfg->setValue(CFG_CDUFCU_REFRESH_PERIOD_MS, 500);

    m_control_cfg->setValue(CFG_SHOW_FPS, 0);
    m_control_cfg->setValue(CFG_KEEP_ON_TOP, 0);

    m_control_cfg->setValue(CFG_SQUAWKBOX_HANDLING_MODE, CFG_SQUAWKBOX_HANDLING_MODE_AUTOMATIC);
    m_control_cfg->setValue(CFG_ALTIMETER_LEFT, ALTIMETER_HPA);
    m_control_cfg->setValue(CFG_ALTIMETER_RIGHT, ALTIMETER_HPA);
    m_control_cfg->setValue(CFG_SHOW_METRIC_ALT, 0);
    m_control_cfg->setValue(CFG_PFD_LEFT_DISPLAY_ILS, 0);
    m_control_cfg->setValue(CFG_PFD_RIGHT_DISPLAY_ILS, 0);
    m_control_cfg->setValue(CFG_DRAW_LEFT_SURROUNDING_AIRPORTS, 1);
    m_control_cfg->setValue(CFG_DRAW_LEFT_SURROUNDING_VORS, 0);
    m_control_cfg->setValue(CFG_DRAW_LEFT_SURROUNDING_NDBS, 0);
    m_control_cfg->setValue(CFG_DRAW_RIGHT_SURROUNDING_AIRPORTS, 1);
    m_control_cfg->setValue(CFG_DRAW_RIGHT_SURROUNDING_VORS, 0);
    m_control_cfg->setValue(CFG_DRAW_RIGHT_SURROUNDING_NDBS, 0);
    m_control_cfg->setValue(CFG_DRAW_GEODATA, 0);
    m_control_cfg->setValue(CFG_ND_LEFT_DISPLAY_RANGE_NM, 20);
    m_control_cfg->setValue(CFG_ND_RIGHT_DISPLAY_RANGE_NM, 20);
    m_control_cfg->setValue(CFG_ND_RANGES_NM, "10,20,40,80,160,320");
    m_control_cfg->setValue(CFG_ND_LEFT_DISPLAY_MODE, CFG_ND_DISPLAY_MODE_NAV_ARC);
    m_control_cfg->setValue(CFG_ND_RIGHT_DISPLAY_MODE, CFG_ND_DISPLAY_MODE_NAV_ARC);
    m_control_cfg->setValue(CFG_ND_LEFT_LEFT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_OFF);
    m_control_cfg->setValue(CFG_ND_LEFT_RIGHT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_OFF);
    m_control_cfg->setValue(CFG_ND_RIGHT_LEFT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_OFF);
    m_control_cfg->setValue(CFG_ND_RIGHT_RIGHT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_OFF);
    m_control_cfg->setValue(CFG_SOUNDS_ENABLED, 1);
    m_control_cfg->setValue(CFG_SOUND_CHANNELS_ENABLED, 1);
    m_control_cfg->setValue(CFG_SYNC_CLOCK_TIME, 0);
    m_control_cfg->setValue(CFG_SYNC_CLOCK_DATE, 0);
    m_control_cfg->setValue(CFG_TCAS, CFG_TCAS_ON);

    m_control_cfg->setValue(CFG_NOISE_GENERATION_INTERVAL_MS, 200);
    m_control_cfg->setValue(CFG_ADF_NOISE_LIMIT_DEG, 45.0);
    m_control_cfg->setValue(CFG_ADF_NOISE_INC_LIMIT_DEG, 15.0);
    m_control_cfg->setValue(CFG_VOR_NOISE_LIMIT_DEG, 20.0);
    m_control_cfg->setValue(CFG_VOR_NOISE_INC_LIMIT_DEG, 10.0);
    m_control_cfg->setValue(CFG_ILS_NOISE_LIMIT, 35);
    m_control_cfg->setValue(CFG_ILS_NOISE_INC_LIMIT, 4.0);

#if VASFMC_GAUGE
    m_control_cfg->setValue(CFG_FMC_AUTOTHRUST_ENABLED, 1);
#else
    m_control_cfg->setValue(CFG_FMC_AUTOTHRUST_ENABLED, 0);
#endif

    m_control_cfg->setValue(CFG_SHOW_INPUTAREAS, 0);
    m_control_cfg->setValue(CFG_SHOW_GEO_DATA_FILLED, 0);
    m_control_cfg->setValue(CFG_ALTIMETER_LEFT_IS_SET_TO_STD, 0);
    m_control_cfg->setValue(CFG_ALTIMETER_RIGHT_IS_SET_TO_STD, 0);
    m_control_cfg->setValue(CFG_SHOW_LEFT_CONSTRAINS, 0);
    m_control_cfg->setValue(CFG_SHOW_RIGHT_CONSTRAINS, 0);

    m_control_cfg->setValue(CFG_FCU_LEFT_ONLY_MODE, 0);
    m_control_cfg->setValue(CFG_CDU_DISPLAY_ONLY_MODE, 0);

    m_control_cfg->setValue(CFG_FBW_ENABLED, 0);
    m_control_cfg->setValue(CFG_FBW_BANK_P_GAIN, 3.8);
    m_control_cfg->setValue(CFG_FBW_BANK_I_GAIN, 5.0);
    m_control_cfg->setValue(CFG_FBW_BANK_D_GAIN, 0.2);
    m_control_cfg->setValue(CFG_FBW_BANK_I_TO_P_RESPONSE_FACTOR, 0.5);
    m_control_cfg->setValue(CFG_FBW_BANK_MAX_RATE, 9.0);
    m_control_cfg->setValue(CFG_FBW_BANK_IDLE_LIMIT, 30.0);
    m_control_cfg->setValue(CFG_FBW_BANK_FORCED_LIMIT, 45.0);

    m_control_cfg->setValue(CFG_FBW_PITCH_P_GAIN, 5.5);
    m_control_cfg->setValue(CFG_FBW_PITCH_I_GAIN, 6.3);
    m_control_cfg->setValue(CFG_FBW_PITCH_D_GAIN, 0.2);
    m_control_cfg->setValue(CFG_FBW_PITCH_MAX_RATE, 3.0);
    m_control_cfg->setValue(CFG_FBW_PITCH_NEG_LIMIT, -15.0);
    m_control_cfg->setValue(CFG_FBW_PITCH_POS_LIMIT, 30.0);
    m_control_cfg->setValue(CFG_FBW_PITCH_GOOD_TREND_DAMPING_FACTOR, 3.0);
    m_control_cfg->setValue(CFG_FBW_PITCH_I_TO_P_RESPONSE_FACTOR, 0.45);
    m_control_cfg->setValue(CFG_FBW_PITCH_BANK_RATE_BOOST_FACTOR, 8.0);
    m_control_cfg->setValue(CFG_FBW_PITCH_STABLE_FPV_DAMP_FACTOR, 0.4);
    m_control_cfg->setValue(CFG_FBW_PITCH_TRANSITION_BOOST_FACTOR, 1.4);

    m_control_cfg->setValue(CFG_ACFT_DATA_LAST_FILE, "a320.cfg");

    m_control_cfg->setValue(CFG_USE_CPFLIGHT_SERIAL, 0);
    m_control_cfg->setValue(CFG_USE_IOCP_SERVER, 0);

    m_control_cfg->setValue(CFG_FMC_CONNECT_MODE, CFG_FMC_CONNECT_MODE_SINGLE);
    m_control_cfg->setValue(CFG_FMC_CONNECT_MODE_MASTER_SERVER_PORT, 8001);
    m_control_cfg->setValue(CFG_FMC_CONNECT_MODE_SLAVE_MASTER_IP, "127.0.0.1");
    m_control_cfg->setValue(CFG_FMC_CONNECT_MODE_SLAVE_MASTER_PORT, 8001);
    
    m_control_cfg->setValue(CFG_VROUTE_AIRAC_RESTRICTION, 0);

#if VASFMC_GAUGE
    m_control_cfg->setValue(CFG_ENABLE_AIRBUS_FLAP_HANDLING_MODE, 1);
#else
    m_control_cfg->setValue(CFG_ENABLE_AIRBUS_FLAP_HANDLING_MODE, 0);
#endif

    m_control_cfg->setValue(CFG_ENABLE_SEPARATE_THROTTLE_LEVER_INPUTS, 0);
}

/////////////////////////////////////////////////////////////////////////////

OpenGLText* FMCControl::getGLFont() const
{
    if (m_gl_font == 0)
    {
        m_gl_font = new OpenGLText(
            VasPath::prependPath(m_main_config->getValue(CFG_FONT_NAME)),
            m_main_config->getIntValue(CFG_ACTIVE_FONT_SIZE));
        MYASSERT(m_gl_font != 0);
    }

    return m_gl_font;
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::setSoundHandler(FMCSoundsHandler* fmc_sounds_handler)
{
    m_fmc_sounds_handler = fmc_sounds_handler;
    m_checklist_manager->setSoundHandler(m_fmc_sounds_handler);

    MYASSERT(m_flight_status_checker != 0);
    MYASSERT(m_fmc_sounds_handler->fmcSounds() != 0);

    MYASSERT(connect(m_flight_status_checker, SIGNAL(signalReachingAltitude()),
                     m_fmc_sounds_handler->fmcSounds(), SLOT(slotPlayAltReachAlert())));

    MYASSERT(connect(m_flight_status_checker, SIGNAL(signal1000FtToGo()),
                     m_fmc_sounds_handler->fmcSounds(), SLOT(slotPlay1000FtToGo())));
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::setupFsAccess()
{
    delete m_fs_access;
    m_fs_access = 0;

    if (m_main_config->getValue(CFG_FS_ACCESS_TYPE) == FS_ACCESS_TYPE_XPLANE)
    {
        Logger::log("Switching to XPLANE access");
        m_fs_access = new FSAccessXPlane(m_config_widget_provider, CFG_XPLANE_FILENAME, m_flightstatus);
    }

    //switchToXPlane();

    MYASSERT(m_fs_access != 0);

    m_fs_access->setMode(isFMCConnectModeSlave() ? FSAccess::MODE_SLAVE : FSAccess::MODE_MASTER);
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::switchToXPlane()
{
    m_main_config->setValue(CFG_FS_ACCESS_TYPE, FS_ACCESS_TYPE_XPLANE);
    setupFsAccess();
}


/////////////////////////////////////////////////////////////////////////////

void FMCControl::recalcFlightstatus()
{
    projection()->convertLatLonToXY(m_flightstatus->current_position_raw);
    m_flightstatus->updateSmoothedPosition();
    projection()->convertLatLonToXY(m_flightstatus->current_position_smoothed);
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::slotCentralTimer()
{
    QTime overall_timer;
    overall_timer.start();

    //TODO implement load balancing
    slotControlTimer();

    if (m_fmc_processor != 0) m_fmc_processor->slotRefresh(false);
    if (m_cpflight_serial != 0) m_cpflight_serial->slotWriteValues(false);
    if (m_fmc_sounds_handler->fmcSounds() != 0) m_fmc_sounds_handler->fmcSounds()->slotCheckSoundsTimer();

    m_flight_mode_tracker->slotCheckAndSetFlightMode();

    //----- process FS CTRL input

    if (m_fsctrl_poll_timer.elapsed() > 50)
    {
        switch(m_fsctrl_poll_index)
        {
            case(3): {
                if (m_cdu_left_handler != 0 && m_cdu_left_handler->fmcCduBase() != 0) 
                    m_cdu_left_handler->fmcCduBase()->slotProcessInput();
                if (m_cdu_right_handler != 0 && m_cdu_right_handler->fmcCduBase() != 0) 
                    m_cdu_right_handler->fmcCduBase()->slotProcessInput();
                break;
            }
        }

        m_fsctrl_poll_timer.start();
        m_fsctrl_poll_index = ++m_fsctrl_poll_index % 5;
    }

    //----- either call the CDU or FCU in one run

    if (m_cdufcu_refresh_timer.elapsed() >= m_cdufcu_refresh_ms)
    {
        switch(m_cdufcu_refresh_index)
        {
            case(0): {
                if (m_cdu_left_handler != 0 && m_cdu_left_handler->fmcCduBase() != 0) 
                    m_cdu_left_handler->fmcCduBase()->slotRefresh();
                if (m_cdu_right_handler != 0 && m_cdu_right_handler->fmcCduBase() != 0) 
                    m_cdu_right_handler->fmcCduBase()->slotRefresh();
                break;
            }
        }

        m_cdufcu_refresh_index = 1 - m_cdufcu_refresh_index;
        m_cdufcu_refresh_timer.start();
    }

    //----- either call the AP or ATHR in one run

    if (m_ap_athr_refresh_timer.elapsed() >= m_ap_athr_refresh_ms)
    {
        switch(m_ap_athr_refresh_index)
        {
            case(0): {
                m_fmc_autopilot->slotRefresh();
                break;
            }
            case(1): {
                m_fmc_autothrottle->slotRefresh();
                break;
            }
        }

        m_ap_athr_refresh_index = 1 - m_ap_athr_refresh_index;
        m_ap_athr_refresh_timer.start();
    }
    
    if (overall_timer.elapsed() > 100) 
        Logger::log(QString("FMCControl:slotCentralTimer: elapsed = %1ms").arg(overall_timer.elapsed()));

    emit signalTimeUsed("CT", overall_timer.elapsed());
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::slotControlTimer()
{
    if (m_control_timer.elapsed() < 250) return;
    m_control_timer.start();

#if DO_TIMER_LOGGING
    QTime overall_timer;
    overall_timer.start();
#endif

    //Logger::log("FMCControl:slotControlTimer:");

    // recalc refresh times

    m_ap_athr_refresh_ms = Navcalc::round(m_main_config->getIntValue(CFG_FLIGHTSTATUS_SMOOTHING_DELAY_MS)/8.0);
    m_pfdnd_refresh_ms = Navcalc::round(m_control_cfg->getIntValue(CFG_PFDND_REFRESH_PERIOD_MS) / 2.0);
    m_ecam_refresh_ms = Navcalc::round(m_control_cfg->getIntValue(CFG_ECAM_REFRESH_PERIOD_MS) / 1.0);
    m_cdufcu_refresh_ms = Navcalc::round(m_control_cfg->getIntValue(CFG_CDUFCU_REFRESH_PERIOD_MS) / 2.0);

    // check FMC connection mode

    if (m_last_fmc_connect_mode != getFMCConnectMode())
    {
        delete m_fmc_connect_master_tcp_server;
        m_fmc_connect_master_tcp_server = 0;
        delete m_fmc_connect_slave_tcp_client;
        m_fmc_connect_slave_tcp_client = 0;

        Logger::log(QString("FMCControl:slotControlTimer: FMC connect mode changed to %1").arg(getFMCConnectMode()));

        if (isFMCConnectModeMaster())
        {
            m_fs_access->setMode(FSAccess::MODE_MASTER);

            m_fmc_connect_master_tcp_server = new TransportLayerTCPServer;
            MYASSERT(m_fmc_connect_master_tcp_server != 0);

            MYASSERT(connect(m_fmc_connect_master_tcp_server, SIGNAL(signalClientConnected()),
                             this, SLOT(slotRemoteFMCConnected())));
            MYASSERT(connect(m_fmc_connect_master_tcp_server, SIGNAL(signalDataReceived(qint16, QByteArray&)),
                             this, SLOT(slotReceivedRemoteFMCData(qint16, QByteArray&))));

            m_fmc_connect_master_tcp_server->connectToHost(QString::null, getFMCConnectModeMasterPort());
        }
        else if (isFMCConnectModeSlave())
        {
            m_fs_access->setMode(FSAccess::MODE_SLAVE);

            m_fmc_connect_slave_tcp_client = new TransportLayerTCPClient;
            MYASSERT(m_fmc_connect_slave_tcp_client != 0);

            MYASSERT(connect(m_fmc_connect_slave_tcp_client, SIGNAL(signalDataReceived(qint16, QByteArray&)),
                             this, SLOT(slotReceivedRemoteFMCData(qint16, QByteArray&))));

            m_fmc_connect_slave_tcp_client->connectToHost(
                getFMCConnectModeSlaveMasterIP(), getFMCConnectModeSlaveMasterPort());
        }
        else
        {
            m_fs_access->setMode(FSAccess::MODE_MASTER);
        }
    }

    m_last_fmc_connect_mode = getFMCConnectMode();

    // sync status to slave FMCs

    if (isFMCConnectModeMaster() &&
        getFMCConnectModeMasterNrClients() > 0 &&
        m_fmc_connect_master_mode_sync_timer.elapsed() >= 500)
    {
        m_fmc_connect_master_mode_sync_timer.start();

        QByteArray data;

        // sync autothrottle state
        QDataStream ds1(&data, QIODevice::WriteOnly);
        *m_fmc_autothrottle >> ds1;
        m_fmc_connect_master_tcp_server->sendData(SYNC_DATA_TYPE_AUTOTHROTTLE, data);

        data.clear();
        QDataStream ds2(&data, QIODevice::WriteOnly);
        *m_fmc_autopilot >> ds2;
        m_fmc_connect_master_tcp_server->sendData(SYNC_DATA_TYPE_AUTOPILOT, data);
    }

    // process FS controls

    while(!m_flightstatus->fsctrl_fmc_list.isEmpty())
    {
        switch(m_flightstatus->fsctrl_fmc_list.first())
        {
            case(30):
                emit signalStyleA();
                    break;
            case(31):
                emit signalStyleB();
                break;
            case(32):
                emit signalStyleG();
                break;
            case(50):
                slotPreviousChecklistItem();
                break;
            case(51):
                slotNextChecklistItem();
                break;
        }

        m_flightstatus->fsctrl_fmc_list.removeFirst();
    }

    // set noise limits

    calcNoiseLimits();

    // check FS status

    if (!m_flightstatus->isValid() || m_flightstatus->slew) return;

    // trigger flight status checker

    m_flight_status_checker->doChecks(fsAccess());

    //----- !! we exit here if we are in slave mode !!

    if (isFMCConnectModeSlave()) return;

    // set FSAccess throttle mode

    m_fs_access->setSeperateThrottleLeverMode(isSeperateThrottleLeverInputModeEnabled());

    // check altimeter value

    if (isAltimeterSetToSTD(true) && m_flightstatus->AltPressureSettingHpa() != Navcalc::STD_ALTIMETER_HPA)
        m_fs_access->setAltimeterHpa(Navcalc::STD_ALTIMETER_HPA);
    if (isAltimeterSetToSTD(false) && m_flightstatus->AltPressureSettingHpa() != Navcalc::STD_ALTIMETER_HPA)
        m_fs_access->setAltimeterHpa(Navcalc::STD_ALTIMETER_HPA);

    // sync date and time

    syncDateTime();

    // check landing flaps notch

    if (m_fmc_data->landingFlapsNotch() > m_flightstatus->flaps_lever_notch_count)
        m_fmc_data->setLandingFlapsNotch(m_flightstatus->flaps_lever_notch_count);

    // set SBOX transponder

    if (m_sbox_transponder_timer.elapsed() > 1000)
    {
        switch(m_control_cfg->getIntValue(CFG_SQUAWKBOX_HANDLING_MODE))
        {
            case(CFG_SQUAWKBOX_HANDLING_MODE_AUTOMATIC):
                m_fs_access->setSBoxTransponder(m_flightstatus->onground ? false : true);
                break;
            case(CFG_SQUAWKBOX_HANDLING_MODE_OFF):
                m_fs_access->setSBoxTransponder(false);
                break;
            case(CFG_SQUAWKBOX_HANDLING_MODE_ON):
                m_fs_access->setSBoxTransponder(true);
                break;
        }

        m_sbox_transponder_timer.start();
    }

    // reset V-speeds

    if (!m_flightstatus->onground)
    {
        // reset V-speeds

        if ((m_fmc_data->normalRoute().accelerationAltitudeFt() > 0 && 
             m_flightstatus->smoothed_altimeter_readout.lastValue() > m_fmc_data->normalRoute().accelerationAltitudeFt())
            ||
            m_flightstatus->radarAltitude() > 15000)
        {
            if (m_fmc_data->V1() != 0) m_fmc_data->setV1(0);
            if (m_fmc_data->Vr() != 0) m_fmc_data->setVr(0);
            if (m_fmc_data->V2() != 0) m_fmc_data->setV2(0);
        }
    }

    // reset approach phase

    if (m_fmc_data->approachPhaseActive() && m_flightstatus->onground)
        m_fmc_data->setApproachPhaseActive(false);

    // check acceleration/thrust reduction altitudes

    if (m_flightstatus->onground)
    {
        if (m_fmc_data->normalRoute().thrustReductionAltitudeFt() - (m_flightstatus->ground_alt_ft + 500.0) <= 0)
        {
            Logger::log("FMCControl:slotControlTimer: resetting thrust reduction alt");
            m_fmc_data->normalRoute().setThrustReductionAltitudeFt((int)m_flightstatus->ground_alt_ft + 1000);
        }

        if (m_fmc_data->normalRoute().accelerationAltitudeFt() - (m_flightstatus->ground_alt_ft + 500.0) <= 0)
        {
            Logger::log("FMCControl:slotControlTimer: resetting acceleration alt");
            m_fmc_data->normalRoute().setAccelerationAltitudeFt((int)m_flightstatus->ground_alt_ft + 1000);
        }
    }

#if DO_TIMER_LOGGING
    emit signalTimeUsed("FC", overall_timer.elapsed());
#endif

    // check landing flap setting, fix it to full when still on ground

    if ((m_flight_mode_tracker->isPreflight() || m_flight_mode_tracker->isTaxiing()) &&
        m_fmc_data->landingFlapsNotch() != m_flightstatus->flaps_lever_notch_count-1)
    {
        m_fmc_data->setLandingFlapsNotch(m_flightstatus->flaps_lever_notch_count-1);
    }

    // write status to flightsim

    if (m_main_config->getValue(CFG_FS_ACCESS_TYPE) == FS_ACCESS_TYPE_MSFS)
    {
        m_fmc_status_data.fd_engaged = m_flightstatus->fd_active;
        
        m_fmc_status_data.athr_armed = m_fmc_autothrottle->isAPThrottleArmed();
        m_fmc_status_data.athr_engaged = m_fmc_autothrottle->isAPThrottleEngaged();
        m_fmc_status_data.athr_speed_mode = m_fmc_autothrottle->isAPThrottleModeSpeedSet();
        m_fmc_status_data.athr_mach_mode = m_fmc_autothrottle->isAPThrottleModeMachSet();
        m_fmc_status_data.athr_n1_mode = m_fmc_autothrottle->isAPThrottleModeN1Engaged();
        
        m_fmc_status_data.ap_engaged = m_flightstatus->ap_enabled;
        m_fmc_status_data.ap_both_app_mode = m_fmc_autopilot->isLOCHoldActive();
        
        m_fmc_status_data.ap_horiz_hdg_mode = m_fmc_autopilot->isHeadingHoldActive();
        m_fmc_status_data.ap_horiz_lnav_mode = m_fmc_autopilot->isNAVHoldActive();
        m_fmc_status_data.ap_horiz_loc_mode = m_fmc_autopilot->isLOCHoldActive();
        
        m_fmc_status_data.ap_vert_vs_mode = m_fmc_autopilot->isVsModeActive();
        m_fmc_status_data.ap_vert_flch_mode = m_fmc_autopilot->isFLChangeModeActive();
        //TODO m_fmc_status_data.ap_vert_vnav_mode = m_fmc_autopilot->;
        m_fmc_status_data.ap_vert_alt_hold = m_fmc_autopilot->isALTHoldActive();
        m_fmc_status_data.ap_vert_fpa_mode = m_fmc_autopilot->isFlightPathModeActive();

        m_fs_access->writeFMCStatusToSim(m_fmc_status_data);
    }

#if DO_TIMER_LOGGING
    if (overall_timer.elapsed() > 20) 
        Logger::log(QString("FMCControl:slotControlTimer: elapsed end = %1ms").arg(overall_timer.elapsed()));
#endif
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::slotApproachPhaseActivated()
{
    Logger::log("FMCControl:slotApproachPhaseActivated:");
    
    const Airport* adep = normalRoute().destinationAirport();
    if (adep != 0 && adep->hasActiveRunway() && adep->activeRunway().hasILS())
    {
        Logger::log(QString("tuning ILS of %1 RWY %2 (%3/%4)").
                    arg(adep->id()).
                    arg(adep->activeRunway().id()).
                    arg(adep->activeRunway().ILSFreq()/1000.0, 0, 'f', 2).
                    arg(adep->activeRunway().ILSHdg()));
        
        m_fs_access->setNavFrequency(adep->activeRunway().ILSFreq(), 0);
        m_fs_access->setNavOBS(adep->activeRunway().ILSHdg(), 0);
    }
    
    //TODO do this in the autothrust module when in managed mode
    if (m_flightstatus->smoothed_ias.lastValue() > aircraftData().getGreenDotSpeed())
    {
        Logger::log(QString("FMCControl:slotApproachPhaseActivated: "
                            "setting green dot speed %1kts").arg(aircraftData().getGreenDotSpeed()));
        m_fs_access->setAPAirspeed((int)aircraftData().getGreenDotSpeed());
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::syncDateTime()
{
    if (isFMCConnectModeSlave() || m_date_time_sync_timer.elapsed() < 3000) return;
    m_date_time_sync_timer.start();

	//----- sync fs time

 	if (timeSyncEnabled() != 0 && m_flightstatus->isValid() && !m_flightstatus->paused)
    {
 		QTime current_utc = QDateTime::currentDateTime().toUTC().time();
 		int diff = qAbs(current_utc.secsTo(m_flightstatus->fs_utc_time));

 		if (diff >= FS_TIME_SYNC_MAX_DIFF_SEC)
 		{
            Logger::log(QString("FMCControl:syncDateTime: fs:%1 pc:%2 diff:%3 - syncing time").
                        arg(m_flightstatus->fs_utc_time.toString().toLatin1().data()).
                        arg(current_utc.toString().toLatin1().data()).
                        arg(diff));

            m_fs_access->setUTCTime(current_utc);
 		}
 	}

 	//----- sync fs date

 	if (dateSyncEnabled() && m_flightstatus->isValid() && !m_flightstatus->paused)
    {
 		QDate current_utc = QDateTime::currentDateTime().toUTC().date();
 		int diff = qAbs(current_utc.daysTo(m_flightstatus->fs_utc_date));

 		if (diff >= 1)
		{
            Logger::log(QString("FMCControl:syncDateTime: fs:%1 pc:%2 diff:%3 - syncing date").
                        arg(m_flightstatus->fs_utc_date.toString().toLatin1().data()).
                        arg(current_utc.toString().toLatin1().data()).
                        arg(diff));

			m_fs_access->setUTCDate(current_utc);
		}
 	}
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::slotDataChanged(const QString& flag, bool direct_change, const QString& comment)
{
    Logger::logToFileOnly(QString("FMCControl:slotDataChanged: %1/%2/%3").
                          arg(flag).arg(direct_change).arg(comment));

    emit signalDataChanged(flag);

    if (flag == Route::FLAG_NORMAL)
        if (!m_fmc_data->normalRoute().saveFP(m_persistance_filename))
            Logger::log(QString("FMCControl:slotDataChanged: could not save persistant route to (%1)").
                        arg(m_persistance_filename));

    m_fs_access->setReadFp();

    //----- sync data to the remove FMCs if any

    if ((m_fmc_connect_master_tcp_server != 0 && getFMCConnectModeMasterNrClients() > 0) ||
        (m_fmc_connect_slave_tcp_client != 0 && direct_change))
    {
        QByteArray data;
        QDataStream ds(&data, QIODevice::WriteOnly);
        int data_type = 0;

        if (flag == Route::FLAG_NORMAL)
        {
            m_fmc_data->normalRoute() >> ds;
            data_type = SYNC_DATA_TYPE_NORMAL_ROUTE;
        }
        else if (flag == Route::FLAG_TEMPORARY)
        {
            m_fmc_data->temporaryRoute() >> ds;
            data_type = SYNC_DATA_TYPE_TEMPORARY_ROUTE;
        }
        else if (flag == Route::FLAG_ALTERNATE)
        {
            m_fmc_data->alternateRoute() >> ds;
            data_type = SYNC_DATA_TYPE_ALTERNATE_ROUTE;
        }
        else if (flag == Route::FLAG_SECONDARY)
        {
            m_fmc_data->secondaryRoute() >> ds;
            data_type = SYNC_DATA_TYPE_SECONDARY_ROUTE;
        }
        else if (flag == FMCData::CHANGE_FLAG_FMC_DATA)
        {
            *m_fmc_data >> ds;
            data_type = SYNC_DATA_TYPE_FMCDATA;
        }

        if (data.count() > 0 && data_type != 0)
        {
            if (m_fmc_connect_master_tcp_server != 0)
                m_fmc_connect_master_tcp_server->sendData(data_type, data);
            else if (m_fmc_connect_slave_tcp_client != 0)
                m_fmc_connect_slave_tcp_client->sendData(data_type, data);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

Waypoint FMCControl::getPBDWaypoint(const Waypoint& ref_wpt, double mag_bearing, double distance_nm)
{
    Waypoint pbd_wpt = Navcalc::getPBDWaypoint(ref_wpt, mag_bearing, distance_nm, &m_declination_calc);
    ++m_pbd_counter;
    pbd_wpt.setId(QString("PBD%1").arg(m_pbd_counter, 3, 10, QChar('0')));
    return pbd_wpt;
}

/////////////////////////////////////////////////////////////////////////////

bool FMCControl::checkForOverflyWaypoint(QString& waypoint_id)
{
    bool ret = false;

    if (waypoint_id.startsWith(OVERFLY_WPT_DESIGNATOR))
    {
        waypoint_id = waypoint_id.mid(1);
        ret = true;
    }

    if (waypoint_id.endsWith(OVERFLY_WPT_DESIGNATOR))
    {
        waypoint_id = waypoint_id.left(waypoint_id.length()-1);
        ret = true;
    }

    return ret;
}

/////////////////////////////////////////////////////////////////////////////

bool FMCControl::getWaypointsByAirway(const Waypoint& from_waypoint,
                                      const QString& airway,
                                      const QString& to_waypoint,
                                      WaypointPtrList& result_wpt_list,
                                      QString& error_text)
{
    if (!from_waypoint.isValid() || airway.isEmpty() || to_waypoint.isEmpty())
    {
        Logger::log("FMCControl:getWaypointsByAirway: inputdata invalid");
        error_text = "Input data invalid";
        return false;
    }

    // if we got an overfly waypoint, strip the designator
    QString my_to_waypoint = to_waypoint;
    bool is_overfly_wpt = checkForOverflyWaypoint(my_to_waypoint);

    if (!m_navdata->getWaypointsByAirway(from_waypoint, airway, my_to_waypoint, result_wpt_list, error_text))
    {
        Logger::log(QString("FMCControl:getWaypointsByAirway: error with airway %1").arg(airway));
        return false;
    }

    if (is_overfly_wpt)
    {
        Waypoint* last_wpt = result_wpt_list.last();
        last_wpt->restrictions().setOverflyRestriction(true);
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////

QString FMCControl::transponderHandlingModeString() const
{
    switch(m_control_cfg->getIntValue(CFG_SQUAWKBOX_HANDLING_MODE))
    {
        case(CFG_SQUAWKBOX_HANDLING_MODE_AUTOMATIC):  return "AUTO";
        case(CFG_SQUAWKBOX_HANDLING_MODE_OFF):        return "OFF";
        case(CFG_SQUAWKBOX_HANDLING_MODE_ON):         return "ON";
        case(CFG_SQUAWKBOX_HANDLING_MODE_DISABLED):   return "MANUAL";
    }

    return "UNKNOWN";
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::toggleAltimeterPressure(bool left_nd)
{
    if (left_nd)
    {
        if (m_control_cfg->getIntValue(CFG_ALTIMETER_LEFT) == ALTIMETER_HPA)         slotShowAltimterInches(true);
        else if (m_control_cfg->getIntValue(CFG_ALTIMETER_LEFT) == ALTIMETER_INCHES) slotShowAltimterHPA(true);
    }
    else
    {
        if (m_control_cfg->getIntValue(CFG_ALTIMETER_RIGHT) == ALTIMETER_HPA)         slotShowAltimterInches(false);
        else if (m_control_cfg->getIntValue(CFG_ALTIMETER_RIGHT) == ALTIMETER_INCHES) slotShowAltimterHPA(false);
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::setupNDRangesList()
{
    QStringList items = m_control_cfg->getValue(CFG_ND_RANGES_NM).split(',', QString::SkipEmptyParts);
    QStringList::const_iterator iter = items.begin();

    uint prev_range = 0;

    for(; iter != items.end(); ++iter)
    {
        bool convok = false;
        uint range = (*iter).trimmed().toUInt(&convok);
        if (!convok)
            Logger::log(QString("FMCControl:setupRangesList: skipping invalid range entry (%1)").arg(*iter));
        else
            m_nd_possible_ranges_nm_list.append(range);

        if (prev_range != 0) { MYASSERT(range > prev_range); }
        prev_range = range;
    }

    MYASSERT(m_nd_possible_ranges_nm_list.count() > 0);

    if (!m_nd_possible_ranges_nm_list.contains(m_control_cfg->getIntValue(CFG_ND_LEFT_DISPLAY_RANGE_NM)))
    {
        Logger::log("FMCControl:setupRangesList: resetting invalid display range to smallest range");
        m_control_cfg->setValue(CFG_ND_LEFT_DISPLAY_RANGE_NM, m_nd_possible_ranges_nm_list.at(0));
    }

    if (!m_nd_possible_ranges_nm_list.contains(m_control_cfg->getIntValue(CFG_ND_RIGHT_DISPLAY_RANGE_NM)))
    {
        Logger::log("FMCControl:setupRangesList: resetting invalid display range to smallest range");
        m_control_cfg->setValue(CFG_ND_RIGHT_DISPLAY_RANGE_NM, m_nd_possible_ranges_nm_list.at(0));
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::nextNDRange(bool left, bool increase)
{
    if (left)
    {
        int current_range_index =
            m_nd_possible_ranges_nm_list.indexOf(m_control_cfg->getIntValue(CFG_ND_LEFT_DISPLAY_RANGE_NM));
        MYASSERT(current_range_index >= 0);

        if (increase && current_range_index+1 < m_nd_possible_ranges_nm_list.count())
            m_control_cfg->setValue(CFG_ND_LEFT_DISPLAY_RANGE_NM, m_nd_possible_ranges_nm_list.at(current_range_index+1));
        else if (!increase && current_range_index-1 >= 0)
            m_control_cfg->setValue(CFG_ND_LEFT_DISPLAY_RANGE_NM, m_nd_possible_ranges_nm_list.at(current_range_index-1));
    }
    else
    {
        int current_range_index =
            m_nd_possible_ranges_nm_list.indexOf(m_control_cfg->getIntValue(CFG_ND_RIGHT_DISPLAY_RANGE_NM));
        MYASSERT(current_range_index >= 0);

        if (increase && current_range_index+1 < m_nd_possible_ranges_nm_list.count())
            m_control_cfg->setValue(CFG_ND_RIGHT_DISPLAY_RANGE_NM, m_nd_possible_ranges_nm_list.at(current_range_index+1));
        else if (!increase && current_range_index-1 >= 0)
            m_control_cfg->setValue(CFG_ND_RIGHT_DISPLAY_RANGE_NM, m_nd_possible_ranges_nm_list.at(current_range_index-1));
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::setNDRange(bool left, uint range_index)
{
    m_control_cfg->setValue(
        (left ? CFG_ND_LEFT_DISPLAY_RANGE_NM : CFG_ND_RIGHT_DISPLAY_RANGE_NM),
        m_nd_possible_ranges_nm_list[qMin((uint)m_nd_possible_ranges_nm_list.count(), range_index)]);
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::nextNDMode(bool left, bool next)
{
    if (!next)
    {
        switch(m_control_cfg->getIntValue(left ? CFG_ND_LEFT_DISPLAY_MODE : CFG_ND_RIGHT_DISPLAY_MODE))
        {
            case(CFG_ND_DISPLAY_MODE_ILS_ROSE): break;
            case(CFG_ND_DISPLAY_MODE_VOR_ROSE): slotEnableNDModeIlsRose(left);  break;
            case(CFG_ND_DISPLAY_MODE_NAV_ROSE): slotEnableNDModeVorRose(left);  break;
            case(CFG_ND_DISPLAY_MODE_NAV_ARC):  slotEnableNDModeNavRose(left);  break;
            case(CFG_ND_DISPLAY_MODE_NAV_PLAN): slotEnableNDModeNavArc(left);   break;
            default: slotEnableNDModeNavArc(left); break;
        }
    }
    else
    {
        switch(m_control_cfg->getIntValue(left ? CFG_ND_LEFT_DISPLAY_MODE : CFG_ND_RIGHT_DISPLAY_MODE))
        {
            case(CFG_ND_DISPLAY_MODE_ILS_ROSE): slotEnableNDModeVorRose(left); break;
            case(CFG_ND_DISPLAY_MODE_VOR_ROSE): slotEnableNDModeNavRose(left); break;
            case(CFG_ND_DISPLAY_MODE_NAV_ROSE): slotEnableNDModeNavArc(left); break;
            case(CFG_ND_DISPLAY_MODE_NAV_ARC): slotEnableNDModePlanRose(left); break;
            case(CFG_ND_DISPLAY_MODE_NAV_PLAN): break;
            default: slotEnableNDModeNavArc(left); break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::nextNDNavaidPointer(bool left_nd, bool left)
{
    if (left_nd)
    {
        if (left)
        {
            switch(m_control_cfg->getIntValue(CFG_ND_LEFT_LEFT_NAVAID_POINTER_TYPE))
            {
                case(CFG_ND_NAVAID_POINTER_TYPE_OFF):
                    m_control_cfg->setValue(CFG_ND_LEFT_LEFT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_NDB); break;
                case(CFG_ND_NAVAID_POINTER_TYPE_NDB):
                    m_control_cfg->setValue(CFG_ND_LEFT_LEFT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_VOR); break;
                case(CFG_ND_NAVAID_POINTER_TYPE_VOR):
                    m_control_cfg->setValue(CFG_ND_LEFT_LEFT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_OFF); break;
            }
        }
        else
        {
            switch(m_control_cfg->getIntValue(CFG_ND_LEFT_RIGHT_NAVAID_POINTER_TYPE))
            {
                case(CFG_ND_NAVAID_POINTER_TYPE_OFF):
                    m_control_cfg->setValue(CFG_ND_LEFT_RIGHT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_NDB); break;
                case(CFG_ND_NAVAID_POINTER_TYPE_NDB):
                    m_control_cfg->setValue(CFG_ND_LEFT_RIGHT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_VOR); break;
                case(CFG_ND_NAVAID_POINTER_TYPE_VOR):
                    m_control_cfg->setValue(CFG_ND_LEFT_RIGHT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_OFF); break;
            }
        }
    }
    else
    {
        if (left)
        {
            switch(m_control_cfg->getIntValue(CFG_ND_RIGHT_LEFT_NAVAID_POINTER_TYPE))
            {
                case(CFG_ND_NAVAID_POINTER_TYPE_OFF):
                    m_control_cfg->setValue(CFG_ND_RIGHT_LEFT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_NDB); break;
                case(CFG_ND_NAVAID_POINTER_TYPE_NDB):
                    m_control_cfg->setValue(CFG_ND_RIGHT_LEFT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_VOR); break;
                case(CFG_ND_NAVAID_POINTER_TYPE_VOR):
                    m_control_cfg->setValue(CFG_ND_RIGHT_LEFT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_OFF); break;
            }
        }
        else
        {
            switch(m_control_cfg->getIntValue(CFG_ND_RIGHT_RIGHT_NAVAID_POINTER_TYPE))
            {
                case(CFG_ND_NAVAID_POINTER_TYPE_OFF):
                    m_control_cfg->setValue(CFG_ND_RIGHT_RIGHT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_NDB); break;
                case(CFG_ND_NAVAID_POINTER_TYPE_NDB):
                    m_control_cfg->setValue(CFG_ND_RIGHT_RIGHT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_VOR); break;
                case(CFG_ND_NAVAID_POINTER_TYPE_VOR):
                    m_control_cfg->setValue(CFG_ND_RIGHT_RIGHT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_OFF); break;
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::setNDNavaidPointer(bool left_nd, bool left, uint type)
{
    if (left_nd)
    {
        if (left)
        {
            switch(type)
            {
                case(CFG_ND_NAVAID_POINTER_TYPE_OFF):
                    m_control_cfg->setValue(CFG_ND_LEFT_LEFT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_OFF); break;
                case(CFG_ND_NAVAID_POINTER_TYPE_NDB):
                    m_control_cfg->setValue(CFG_ND_LEFT_LEFT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_NDB); break;
                case(CFG_ND_NAVAID_POINTER_TYPE_VOR):
                    m_control_cfg->setValue(CFG_ND_LEFT_LEFT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_VOR); break;
            }
        }
        else
        {
            switch(type)
            {
                case(CFG_ND_NAVAID_POINTER_TYPE_OFF):
                    m_control_cfg->setValue(CFG_ND_LEFT_RIGHT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_OFF); break;
                case(CFG_ND_NAVAID_POINTER_TYPE_NDB):
                    m_control_cfg->setValue(CFG_ND_LEFT_RIGHT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_NDB); break;
                case(CFG_ND_NAVAID_POINTER_TYPE_VOR):
                    m_control_cfg->setValue(CFG_ND_LEFT_RIGHT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_VOR); break;
            }
        }
    }
    else
    {
        if (left)
        {
            switch(type)
            {
                case(CFG_ND_NAVAID_POINTER_TYPE_OFF):
                    m_control_cfg->setValue(CFG_ND_RIGHT_LEFT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_OFF); break;
                case(CFG_ND_NAVAID_POINTER_TYPE_NDB):
                    m_control_cfg->setValue(CFG_ND_RIGHT_LEFT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_NDB); break;
                case(CFG_ND_NAVAID_POINTER_TYPE_VOR):
                    m_control_cfg->setValue(CFG_ND_RIGHT_LEFT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_VOR); break;
            }
        }
        else
        {
            switch(type)
            {
                case(CFG_ND_NAVAID_POINTER_TYPE_OFF):
                    m_control_cfg->setValue(CFG_ND_RIGHT_RIGHT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_OFF); break;
                case(CFG_ND_NAVAID_POINTER_TYPE_NDB):
                    m_control_cfg->setValue(CFG_ND_RIGHT_RIGHT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_NDB); break;
                case(CFG_ND_NAVAID_POINTER_TYPE_VOR):
                    m_control_cfg->setValue(CFG_ND_RIGHT_RIGHT_NAVAID_POINTER_TYPE, CFG_ND_NAVAID_POINTER_TYPE_VOR); break;
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::slotToggleFmcAutothrustEnabled()
{
    Logger::log("FMCControl:slotToggleFmcAutothrustEnabled");
    m_control_cfg->setValue(CFG_FMC_AUTOTHRUST_ENABLED, !fmcAutothrustEnabled());
    m_fmc_autothrottle->disengageAPThrottle();
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::setupFlightStatusChecker()
{
    if (m_flight_status_checker == 0 ||
        m_main_config->getIntValue(CFG_STYLE) != m_last_flight_status_checker_style)
    {
        delete m_flight_status_checker;

        if (m_main_config->getIntValue(CFG_STYLE) == CFG_STYLE_A)
            m_flight_status_checker = new FlightStatusCheckerStyleA(m_flightstatus, this);
        else
            //TODO add b-style checker
            m_flight_status_checker = new FlightStatusCheckerBase(m_flightstatus, this);
    }

    MYASSERT(m_flight_status_checker != 0);

    if (m_fmc_sounds_handler != 0)
    {
        MYASSERT(connect(m_flight_status_checker, SIGNAL(signalReachingAltitude()),
                         m_fmc_sounds_handler->fmcSounds(), SLOT(slotPlayAltReachAlert())));

        MYASSERT(connect(m_flight_status_checker, SIGNAL(signal1000FtToGo()),
                         m_fmc_sounds_handler->fmcSounds(), SLOT(slotPlay1000FtToGo())));
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::startPushBack(uint pushback_dist_before_turn_m,
                               bool pushback_turn_direction_clockwise,
                               uint pushback_turn_degrees,
                               uint pushback_dist_after_turn_m)
{
    m_pushback_dist_before_turn_m = pushback_dist_before_turn_m;
    m_pushback_turn_direction_clockwise = pushback_turn_direction_clockwise;
    m_pushback_turn_degrees = pushback_turn_degrees;
    m_pushback_dist_after_turn_m = pushback_dist_after_turn_m;
    m_fs_access->setPushback(FSAccess::PUSHBACK_STOP);
    m_last_pushback_location = Waypoint();
    m_pushback_start_heading = -1;
    m_pushback_timer.start(500);
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::slotPushBackTimer()
{
    if(m_flightstatus->radarAltitude() > 100)
    {
        stopPushBack();
        return;
    }

    if (m_flightstatus->pushback_status == FSAccess::PUSHBACK_STOP)
    {
        if(!m_flightstatus->parking_brake_set)
        {
            if (m_pushback_dist_before_turn_m > 0)
            {
                Logger::log(QString("pushing %1m straight back before turn").arg(m_pushback_dist_before_turn_m));
                m_last_pushback_location = m_flightstatus->current_position_raw;
                m_fs_access->setPushback(FSAccess::PUSHBACK_STRAIGHT);
            }
            else if (m_pushback_turn_degrees > 0)
            {
                Logger::log(QString("turning %1� %2 during push").arg(m_pushback_turn_degrees).
                            arg(m_pushback_turn_direction_clockwise ? "clockwise" : "counter clockwise"));
                
                m_pushback_start_heading = (int)m_flightstatus->smoothedMagneticHeading();
                
                m_fs_access->setPushback(m_pushback_turn_direction_clockwise ?
                                         FSAccess::PUSHBACK_LEFT : FSAccess::PUSHBACK_RIGHT);
            }
            else if (m_pushback_dist_after_turn_m > 0)
            {
                Logger::log(QString("pushing %1m straight back after turn").arg(m_pushback_dist_after_turn_m));
                m_last_pushback_location = m_flightstatus->current_position_raw;
                m_fs_access->setPushback(FSAccess::PUSHBACK_STRAIGHT);
            }
            else
            {
                Logger::log("pushback finished");
                stopPushBack();
            }
        }
    }
    else
    {
        if(m_flightstatus->parking_brake_set)
        {
            stopPushBack();
        }
        else
        {
            if (!m_last_pushback_location.isValid()) m_last_pushback_location = m_flightstatus->current_position_raw;

            double distance = Navcalc::getDistBetweenWaypoints(
                m_last_pushback_location, m_flightstatus->current_position_raw) / Navcalc::METER_TO_NM;

            if (m_pushback_dist_before_turn_m > 0)
            {
                if (distance >= m_pushback_dist_before_turn_m)
                {
                    Logger::log("push straight back before turn completed");
                    m_pushback_dist_before_turn_m = 0;
                    m_fs_access->setPushback(FSAccess::PUSHBACK_STOP);
                }
            }
            else if (m_pushback_start_heading >= 0)
            {
                if (Navcalc::getAbsHeadingDiff(
                        m_pushback_start_heading, m_flightstatus->smoothedMagneticHeading()) >=
                    (m_pushback_turn_degrees - 4) ||
                    m_flightstatus->pushback_status == FSAccess::PUSHBACK_STRAIGHT)
                {
                    Logger::log("push turn completed");
                    m_pushback_turn_degrees = 0;
                    m_pushback_start_heading = -1;
                    m_fs_access->setPushback(FSAccess::PUSHBACK_STOP);
                }
            }
            else if (m_pushback_dist_after_turn_m > 0)
            {
                if (distance >= m_pushback_dist_after_turn_m)
                {
                    Logger::log("push straight back after turn completed");
                    m_pushback_dist_after_turn_m = 0;
                    m_fs_access->setPushback(FSAccess::PUSHBACK_STOP);
                }
            }
        }
   }
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::stopPushBack()
{
    m_fs_access->setPushback(FSAccess::PUSHBACK_STOP);
    m_pushback_timer.stop();
}

/////////////////////////////////////////////////////////////////////////////

double FMCControl::getAdf1Noise() { return m_adf1_noise_generator->getNoise(); }
double FMCControl::getAdf2Noise() { return m_adf2_noise_generator->getNoise(); }
double FMCControl::getVor1Noise() { return m_vor1_noise_generator->getNoise(); }
double FMCControl::getVor2Noise() { return m_vor2_noise_generator->getNoise(); }
double FMCControl::getIls1Noise() { return m_ils1_noise_generator->getNoise(); }
double FMCControl::getIls2Noise() { return m_ils2_noise_generator->getNoise(); }

/////////////////////////////////////////////////////////////////////////////

int FMCControl::glFontSize() const
{
    return m_main_config->getIntValue(CFG_ACTIVE_FONT_SIZE);
}

/////////////////////////////////////////////////////////////////////////////

int FMCControl::glFontIndex() const
{
    return m_main_config->getIntValue(CFG_ACTIVE_FONT_INDEX);
}


/////////////////////////////////////////////////////////////////////////////

void FMCControl::incCDUFontSize()
{
    if (m_cdu_left_handler != 0 && m_cdu_left_handler->fmcCduBase() != 0) m_cdu_left_handler->fmcCduBase()->incFontSize();
    if (m_cdu_right_handler != 0 && m_cdu_right_handler->fmcCduBase() != 0) m_cdu_right_handler->fmcCduBase()->incFontSize();
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::decCDUFontSize()
{
    if (m_cdu_left_handler != 0 && m_cdu_left_handler->fmcCduBase() != 0) m_cdu_left_handler->fmcCduBase()->decFontSize();
    if (m_cdu_right_handler != 0 && m_cdu_right_handler->fmcCduBase() != 0) m_cdu_right_handler->fmcCduBase()->decFontSize();
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::toggleTCAS()
{
    switch(m_control_cfg->getIntValue(CFG_TCAS))
    {
        case(CFG_TCAS_OFF):
            m_control_cfg->setValue(CFG_TCAS, CFG_TCAS_STDBY);
            break;
        case(CFG_TCAS_STDBY):
            m_control_cfg->setValue(CFG_TCAS, CFG_TCAS_ON);
            break;
        default:
            m_control_cfg->setValue(CFG_TCAS, CFG_TCAS_OFF);
            break;
    }
}

/////////////////////////////////////////////////////////////////////////////

bool FMCControl::isTCASOff() const { return m_control_cfg->getIntValue(CFG_TCAS) == CFG_TCAS_OFF; }
bool FMCControl::isTCASStandby() const { return m_control_cfg->getIntValue(CFG_TCAS) == CFG_TCAS_STDBY; }
bool FMCControl::isTCASOn() const { return m_control_cfg->getIntValue(CFG_TCAS) == CFG_TCAS_ON; }

/////////////////////////////////////////////////////////////////////////////

void FMCControl::setKeepOnTop(bool yes)
{
    bool prev_state = doKeepOnTop();
    m_control_cfg->setValue(CFG_KEEP_ON_TOP, yes ? "1" : "0");
    if (doKeepOnTop() != prev_state) emit signalRestartFMC();
}

/////////////////////////////////////////////////////////////////////////////

bool FMCControl::cduNormalScrollMode() const
{
    if (m_cdu_left_handler != 0 && m_cdu_left_handler->fmcCduBase() != 0) 
        return m_cdu_left_handler->fmcCduBase()->normalScrollMode();
    return true;
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::setCduScrollModeNormal()
{
    if (m_cdu_left_handler != 0 && 
        m_cdu_left_handler->fmcCduBase() != 0 && 
        !m_cdu_left_handler->fmcCduBase()->normalScrollMode()) m_cdu_left_handler->fmcCduBase()->toggleScrollMode();
    if (m_cdu_right_handler != 0 &&
        m_cdu_right_handler->fmcCduBase() != 0 && 
        !m_cdu_right_handler->fmcCduBase()->normalScrollMode()) m_cdu_right_handler->fmcCduBase()->toggleScrollMode();
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::setCduScrollModeInverse()
{
    if (m_cdu_left_handler != 0 && 
        m_cdu_left_handler->fmcCduBase() != 0 && 
        !m_cdu_left_handler->fmcCduBase()->inverseScrollMode()) m_cdu_left_handler->fmcCduBase()->toggleScrollMode();
    if (m_cdu_right_handler != 0 && 
        m_cdu_right_handler->fmcCduBase() != 0 && 
        !m_cdu_right_handler->fmcCduBase()->inverseScrollMode()) m_cdu_right_handler->fmcCduBase()->toggleScrollMode();
}

/////////////////////////////////////////////////////////////////////////////

bool FMCControl::ndNormalScrollMode() const
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::setNdScrollModeNormal()
{
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::setNdScrollModeInverse()
{
}

/////////////////////////////////////////////////////////////////////////////

bool FMCControl::ndWindCorrection() const
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::toggleNdWindCorrection()
{
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::slotToggleUseCPFlight()
{
    delete m_cpflight_serial;
    m_cpflight_serial = 0;

    m_control_cfg->setValue(CFG_USE_CPFLIGHT_SERIAL, !useCPFlight());

    if (useCPFlight())
    {
        m_cpflight_serial = new CPFlightSerial(CFG_CPFLIGHT_FILENAME, m_flightstatus, this);
        MYASSERT(m_cpflight_serial != 0);
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::slotToggleUseIOCPServer()
{
    delete m_iocp_server;
    m_iocp_server = 0;

    m_control_cfg->setValue(CFG_USE_IOCP_SERVER, !useIOCPServer());

    if (useIOCPServer())
    {
        m_iocp_server = new IOCPServer(CFG_IOCP_FILENAME, this);
        MYASSERT(m_iocp_server != 0);
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::changeAltimeterSetting(bool left_pfd, int diff)
{
    if (isAltimeterSetToSTD(left_pfd)) return;

    if (showAltHpa(left_pfd))
    {
        fsAccess().setAltimeterHpa(
            LIMITMINMAX(900, 1100, Navcalc::round(m_flightstatus->AltPressureSettingHpa()) + diff));
    }
    else
    {
        double inch_diff = (diff / 100.0);
        fsAccess().setAltimeterHpa(
            LIMITMINMAX(900.0, 1100.0, m_flightstatus->AltPressureSettingHpa() + Navcalc::getHpaFromInches(inch_diff)));
    }
}

/////////////////////////////////////////////////////////////////////////////

bool FMCControl::processExternalCMD(const QString& cmd_code)
{
    QStringList cmd = cmd_code.split(":");
    if  (cmd.count() != 2)
    {
        Logger::log(QString("FMCControl:processExternalCMD: "
                            "invalid CMD for (%1)").arg(cmd_code));
        return false;
    }

    QList<char>* list = 0;

    if (cmd[0] == "ND_LEFT") list = &m_flightstatus->fsctrl_nd_left_list;
    else if (cmd[0] == "PFD_LEFT") list = &m_flightstatus->fsctrl_pfd_left_list;
    else if (cmd[0] == "CDU_LEFT") list = &m_flightstatus->fsctrl_cdu_left_list;
    else if (cmd[0] == "ECAM") list = &m_flightstatus->fsctrl_ecam_list;
    else if (cmd[0] == "FMC") list = &m_flightstatus->fsctrl_fmc_list;
    else if (cmd[0] == "FCU") list = &m_flightstatus->fsctrl_fcu_list;
    else if (cmd[0] == "ND_RIGHT") list = &m_flightstatus->fsctrl_nd_right_list;
    else if (cmd[0] == "PFD_RIGHT") list = &m_flightstatus->fsctrl_pfd_right_list;
    else if (cmd[0] == "CDU_RIGHT") list = &m_flightstatus->fsctrl_cdu_right_list;

    if (list == 0)
    {
        Logger::log(QString("FMCControl:processExternalCMD: "
                            "no handler found for CMD (%1)").arg(cmd_code));
        return false;
    }

    bool convok = false;
    int cmd_value = cmd[1].toInt(&convok);

    if (!convok)
    {
        Logger::log(QString("FMCControl:processExternalCMD: "
                            "could not convert value for CMD (%1)").arg(cmd_code));
        return false;
    }

    //Logger::log(QString("FMCControl:processExternalCMD: CMD (%1) (%2)").arg(cmd_code).arg(cmd_value));
    list->append(cmd_value);
    return true;
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::slotRemoteFMCConnected()
{
    if (m_fmc_connect_master_tcp_server != 0)
    {
        QByteArray data;
        QDataStream ds1(&data, QIODevice::ReadWrite);
        m_fmc_data->normalRoute() >> ds1;
        m_fmc_connect_master_tcp_server->sendData(SYNC_DATA_TYPE_NORMAL_ROUTE, data);

        data.clear();
        QDataStream ds2(&data, QIODevice::ReadWrite);
        m_fmc_data->temporaryRoute() >> ds2;
        m_fmc_connect_master_tcp_server->sendData(SYNC_DATA_TYPE_TEMPORARY_ROUTE, data);

        data.clear();
        QDataStream ds3(&data, QIODevice::ReadWrite);
        m_fmc_data->alternateRoute() >> ds3;
        m_fmc_connect_master_tcp_server->sendData(SYNC_DATA_TYPE_ALTERNATE_ROUTE, data);

        data.clear();
        QDataStream ds4(&data, QIODevice::ReadWrite);
        m_fmc_data->secondaryRoute() >> ds4;
        m_fmc_connect_master_tcp_server->sendData(SYNC_DATA_TYPE_SECONDARY_ROUTE, data);

        data.clear();
        QDataStream ds5(&data, QIODevice::ReadWrite);
        *m_fmc_data >> ds5;
        m_fmc_connect_master_tcp_server->sendData(SYNC_DATA_TYPE_FMCDATA, data);
    }
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::slotReceivedRemoteFMCData(qint16 data_type, QByteArray& data)
{
    QDataStream ds(data);

    switch(data_type)
    {
        case(SYNC_DATA_TYPE_NORMAL_ROUTE): {
            m_fmc_data->normalRoute() << ds;
            MYASSERT(ds.atEnd());
            Logger::log("FMCControl:slotReceivedRemoteFMCData: got primary route from remote FMC");
            break;
        }
        case(SYNC_DATA_TYPE_TEMPORARY_ROUTE): {
            m_fmc_data->temporaryRoute() << ds;
            MYASSERT(ds.atEnd());
            Logger::log("FMCControl:slotReceivedRemoteFMCData: got temporary route from remote FMC");
            break;
        }
        case(SYNC_DATA_TYPE_ALTERNATE_ROUTE): {
            m_fmc_data->alternateRoute() << ds;
            MYASSERT(ds.atEnd());
            Logger::log("FMCControl:slotReceivedRemoteFMCData: got alternate route from remote FMC");
            break;
        }
        case(SYNC_DATA_TYPE_SECONDARY_ROUTE): {
            m_fmc_data->secondaryRoute() << ds;
            MYASSERT(ds.atEnd());
            Logger::log("FMCControl:slotReceivedRemoteFMCData: got secondary route from remote FMC");
            break;
        }
        case(SYNC_DATA_TYPE_AUTOTHROTTLE): {
            if (!isFMCConnectModeSlave()) return;
            *m_fmc_autothrottle << ds;
            MYASSERT(ds.atEnd());
            break;
        }
        case(SYNC_DATA_TYPE_AUTOPILOT): {
            if (!isFMCConnectModeSlave()) return;
            *m_fmc_autopilot << ds;
            MYASSERT(ds.atEnd());
            break;
        }
        case(SYNC_DATA_TYPE_FMCDATA): {
            //TODOif (!isFMCConnectModeSlave()) return;
            *m_fmc_data << ds;
            MYASSERT(ds.atEnd());
            break;
        }
        default: {
            Logger::log(QString("FMCControl:slotReceivedRemoteFMCData: got data with unknown type %1 - discarding").
                        arg(data_type));
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

bool FMCControl::isFMCConnectRemoteConnected() const
{
    if (m_fmc_connect_slave_tcp_client != 0 && m_fmc_connect_slave_tcp_client->isConnected()) return true;
    if (m_fmc_connect_master_tcp_server != 0 && m_fmc_connect_master_tcp_server->isConnected()) return true;
    return false;
}

/////////////////////////////////////////////////////////////////////////////

int FMCControl::getFMCConnectModeMasterNrClients() const
{
    if (m_fmc_connect_master_tcp_server == 0) return 0;
    return m_fmc_connect_master_tcp_server->clientCount();
}

/////////////////////////////////////////////////////////////////////////////

bool FMCControl::allowFPLoad() const
{
    return !m_flightstatus->isValid() || m_flight_mode_tracker->isPreflight() || m_flight_mode_tracker->isTaxiing(); 
}

/////////////////////////////////////////////////////////////////////////////

bool FMCControl::isMSFSActive() const 
{
    return m_main_config->getValue(CFG_FS_ACCESS_TYPE) == FS_ACCESS_TYPE_MSFS; 
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::slotPreviousChecklistItem()
{
    m_checklist_manager->decChecklistItemIndex();
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::slotNextChecklistItem()
{
    m_checklist_manager->incChecklistItemIndex();
}

/////////////////////////////////////////////////////////////////////////////

void FMCControl::calcNoiseLimits()
{
    if (m_noise_limit_update_timer.elapsed() < 500) return;
    m_noise_limit_update_timer.start();

#if DO_TIMER_LOGGING
    QTime overall_timer;
    overall_timer.start();
#endif

    double distance = 0.0;
    WaypointPtrList wpt_selection_list;

    // we calc only one limit at one time - load balancing

    m_noise_calc_index = (m_noise_calc_index + 1) % 4;
    switch(m_noise_calc_index)
    {
        case(0): {

            // ADF1
            if (!m_flightstatus->adf1.id().isEmpty())
            {
                m_navdata->getNdbListByCoordinates(m_flightstatus->current_position_raw, 2, 100, wpt_selection_list);

                Waypoint* ndb_wpt = 0;
                WaypointPtrListIterator iter(wpt_selection_list);
                while(iter.hasNext())
                {
                    Waypoint* wpt = iter.next();
                    if (wpt->id() == m_flightstatus->adf1.id())
                    {
                        ndb_wpt = wpt;
                        break;
                    }
                }

                if (ndb_wpt != 0)
                {
                    distance = Navcalc::getDistBetweenWaypoints(*ndb_wpt, m_flightstatus->current_position_raw);
                
                    m_noise_damping.setDampBorders(0.0, 80.0);
                    m_adf1_noise_generator->setMaxNoise((1.0 - m_noise_damping.dampFactor(distance)) * 
                                                        m_control_cfg->getDoubleValue(CFG_ADF_NOISE_LIMIT_DEG));
                
                    m_adf1_noise_generator->setMaxNoiseIncPerUpdate((1.0 - m_noise_damping.dampFactor(distance)) * 
                                                                    m_control_cfg->getDoubleValue(CFG_ADF_NOISE_INC_LIMIT_DEG));

//                     Logger::log(QString("ADF1 dist=%1 max=%2 inc=%3").
//                                 arg(distance).
//                                 arg(m_adf1_noise_generator->maxNoise()).
//                                 arg(m_adf1_noise_generator->maxNoiseIncPerUpdate()));
                }
                else
                {
                    // fallback if we do not find the navaid
                    m_adf1_noise_generator->setMaxNoise(0.25 * m_control_cfg->getDoubleValue(CFG_ADF_NOISE_LIMIT_DEG));
                    m_adf1_noise_generator->setMaxNoiseIncPerUpdate(0.25 * m_control_cfg->getDoubleValue(CFG_ADF_NOISE_INC_LIMIT_DEG));
                }
            }

#if DO_TIMER_LOGGING
    if (overall_timer.elapsed() > 20) 
        Logger::log(QString("FMCControl:calcNoiseLimits: elapsed ADF1 = %1ms").arg(overall_timer.elapsed()));
#endif
            break;
        }
                
        case(1): {

            // ADF2
            if (!m_flightstatus->adf2.id().isEmpty())
            {
                m_navdata->getNdbListByCoordinates(m_flightstatus->current_position_raw, 2, 100, wpt_selection_list);

                Waypoint* ndb_wpt = 0;
                WaypointPtrListIterator iter(wpt_selection_list);
                while(iter.hasNext())
                {
                    Waypoint* wpt = iter.next();
                    if (wpt->id() == m_flightstatus->adf2.id())
                    {
                        ndb_wpt = wpt;
                        break;
                    }
                }

                if (ndb_wpt != 0)
                {
                    distance = Navcalc::getDistBetweenWaypoints(*ndb_wpt, m_flightstatus->current_position_raw);
                
                    m_noise_damping.setDampBorders(0.0, 80.0);
                    m_adf2_noise_generator->setMaxNoise((1.0 - m_noise_damping.dampFactor(distance)) * 
                                                        m_control_cfg->getDoubleValue(CFG_ADF_NOISE_LIMIT_DEG));
                
                    m_adf2_noise_generator->setMaxNoiseIncPerUpdate((1.0 - m_noise_damping.dampFactor(distance)) * 
                                                                    m_control_cfg->getDoubleValue(CFG_ADF_NOISE_INC_LIMIT_DEG));

//                     Logger::log(QString("ADF2 dist=%1 max=%2 inc=%3").
//                                 arg(distance).
//                                 arg(m_adf2_noise_generator->maxNoise()).
//                                 arg(m_adf2_noise_generator->maxNoiseIncPerUpdate()));
                }
                else
                {
                    // fallback if we do not find the navaid
                    m_adf2_noise_generator->setMaxNoise(0.25 * m_control_cfg->getDoubleValue(CFG_ADF_NOISE_LIMIT_DEG));
                    m_adf2_noise_generator->setMaxNoiseIncPerUpdate(0.25 * m_control_cfg->getDoubleValue(CFG_ADF_NOISE_INC_LIMIT_DEG));
                }
            }

#if DO_TIMER_LOGGING
    if (overall_timer.elapsed() > 20) 
        Logger::log(QString("FMCControl:calcNoiseLimits: elapsed ADF2 = %1ms").arg(overall_timer.elapsed()));
#endif
            break;
        }

        case(2): {

            // VOR1/ILS1
            if (!m_flightstatus->nav1.id().isEmpty())
            {
                distance = Navcalc::getDistBetweenWaypoints(m_flightstatus->nav1,
                                                            m_flightstatus->current_position_raw);

                if (m_flightstatus->nav1_has_loc)
                {
                    m_noise_damping.setDampBorders(0.0, 30.0);
                    m_ils1_noise_generator->setMaxNoise((1.0 - m_noise_damping.dampFactor(distance)) * 
                                                        m_control_cfg->getDoubleValue(CFG_ILS_NOISE_LIMIT));
                
                    m_ils1_noise_generator->setMaxNoiseIncPerUpdate((1.0 - m_noise_damping.dampFactor(distance)) * 
                                                                    m_control_cfg->getDoubleValue(CFG_ILS_NOISE_INC_LIMIT));
                }
                else
                {
                    m_noise_damping.setDampBorders(0.0, 200.0);
                    m_vor1_noise_generator->setMaxNoise((1.0 - m_noise_damping.dampFactor(distance)) * 
                                                        m_control_cfg->getDoubleValue(CFG_VOR_NOISE_LIMIT_DEG));
                
                    m_vor1_noise_generator->setMaxNoiseIncPerUpdate((1.0 - m_noise_damping.dampFactor(distance)) * 
                                                                    m_control_cfg->getDoubleValue(CFG_VOR_NOISE_INC_LIMIT_DEG));
                }   
            }

#if DO_TIMER_LOGGING
    if (overall_timer.elapsed() > 20) 
        Logger::log(QString("FMCControl:calcNoiseLimits: elapsed VOR1 = %1ms").arg(overall_timer.elapsed()));
#endif
            break;
        }

        case(3): {

            // VOR2/ILS2
            if (!m_flightstatus->nav2.id().isEmpty())
            {                    //TODO
                distance = Navcalc::getDistBetweenWaypoints(m_flightstatus->nav2,
                                                            m_flightstatus->current_position_raw);

                if (m_flightstatus->nav2_has_loc)
                {
                    m_noise_damping.setDampBorders(0.0, 30.0);
                    m_ils2_noise_generator->setMaxNoise((1.0 - m_noise_damping.dampFactor(distance)) * 
                                                        m_control_cfg->getDoubleValue(CFG_ILS_NOISE_LIMIT));
                
                    m_ils2_noise_generator->setMaxNoiseIncPerUpdate((1.0 - m_noise_damping.dampFactor(distance)) * 
                                                                    m_control_cfg->getDoubleValue(CFG_ILS_NOISE_INC_LIMIT));
                }
                else
                {
                    m_noise_damping.setDampBorders(0.0, 200.0);
                    m_vor2_noise_generator->setMaxNoise((1.0 - m_noise_damping.dampFactor(distance)) * 
                                                        m_control_cfg->getDoubleValue(CFG_VOR_NOISE_LIMIT_DEG));
                
                    m_vor2_noise_generator->setMaxNoiseIncPerUpdate((1.0 - m_noise_damping.dampFactor(distance)) * 
                                                                    m_control_cfg->getDoubleValue(CFG_VOR_NOISE_INC_LIMIT_DEG));
                }

//                 Logger::log(QString("VOR2 dist=%1 max=%2 inc=%3").
//                             arg(distance).
//                             arg(m_vor2_noise_generator->maxNoise()).
//                             arg(m_vor2_noise_generator->maxNoiseIncPerUpdate()));
            }

#if DO_TIMER_LOGGING
    if (overall_timer.elapsed() > 20) 
        Logger::log(QString("FMCControl:calcNoiseLimits: elapsed VOR2 = %1ms").arg(overall_timer.elapsed()));
#endif
            break;
        }    
    }

#if DO_TIMER_LOGGING
    if (overall_timer.elapsed() > 20) 
        Logger::log(QString("FMCControl:calcNoiseLimits: elapsed end = %1ms").arg(overall_timer.elapsed()));
#endif

}

// End of file
