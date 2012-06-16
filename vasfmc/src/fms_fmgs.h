/***************************************************************************
 *   Copyright (C) 2008 by Uwe Buchholz                                    *
 *   							                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
/*
 * This is the Flight Management & Guidance System FMGS, calculating the 
 * Flight Plan and Aircraft Navigation Conditions
 * 
 */
#ifndef FlightManagementGuidanceSystem_H_
#define FlightManagementGuidanceSystem_H_

#include "FMS_FMGS_DEFINES.h"
#include "LINEAR_EQUATION.h"
#include "FMS_FMGS_BASIC_FUNCTION.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <time.h>

#include "FMS_FMGC.h"
#include "A380_FMGC.h"
#include "A345_FMGC.h"
#include "A342_FMGC.h"
#include "A332_FMGC.h"
#include "A306_FMGC.h"
#include "A310_FMGC.h"
#include "A320_FMGC.h"
#include "B74X_FMGC.h"
#include "B73X_FMGC.h"
#include "maschd.h"

#include "flightroute.h"
#include "flightstatus.h"
#include "waypoint.h"
#include "airport.h"

using namespace std;

namespace FMS
{
//! Flight Management System Class
//! Inheritance from FMS_FMGS_BASIC_FUNCTION library

class FlightManagementGuidanceSystem : protected FMS_FMGS_BASIC_FUNCTION
{
public:
	
	//! Flight Management Guidance system. initialized by Aircraft Type 
	FlightManagementGuidanceSystem(string);
	
	//! Destructor for the Flight Management Guidance System
	virtual ~FlightManagementGuidanceSystem();

/********************************************************************
 * author uwe, Class functional description: 
 * 
 * FMGS Interface for Fuel Calculation 
 *
 ********************************************************************/
//! FMGS Interface for Fuel Calculation
	
	void fuelPrediction();
	
/********************************************************************
 * author uwe, Class functional description: 
 * 
 * FMGS Interface for Initialisation 
 *
 ********************************************************************/	
//! FMGS Interface for Initialisation
	
	void initFMGS(const FlightRoute&, const FlightRoute&, const FlightStatus&);
	
/********************************************************************
 * author uwe, Class functional description: 
 * 
 * FMGS temporary FPL calculation with VNAV profile  
 *
 ********************************************************************/
	
	void temporaryFlightPlan(	const double&, const double&, const double&, const double&, 
							const double&, const double&, const double&, const double&, 
							const double&, const double&, const double&);
	
/********************************************************************
 * author uwe, Class functional description: 
 * 
 * FMGS Optimum Flight Level 
 *
 ********************************************************************/
//! FMGS Optimum Flight Level
	
	double optimumFlightLevel(const FlightStatus&);
	
/********************************************************************
 * author uwe, Class functional description: 
 * 
 * FMGS Max Flight Level 
 *
 ********************************************************************/
//! FMGS Max Flight Level
	
	double maxFlightLevel(const FlightStatus&);
	
/********************************************************************
 * author uwe, Class functional description: 
 * 
 * FMGS Speed Limits and advisories for PFD and PFC for TO/CLB, APPR 
 * and in flight 
 *
 ********************************************************************/
//! FMGS Speed Limits and advisories for PFD and PFC for TO/CLB, APPR and in flight
	
	void speed(const FlightStatus&);
	
	
/********************************************************************
 * author uwe, Class functional description: 
 * 
 * FMGS /FADEC continous engine control
 *
 ********************************************************************/
//! FMGS /FADEC continous engine control
	
	void fadec(const string&, const double&, const double&, const int&, const double&);

/********************************************************************
 * author uwe, Class functional description: 
 * 
 * FMGS Estimated Fuel on Board EFOB during cruise
 *
 ********************************************************************/
//! FMGS Estimated Fuel on Board EFOB during cruise
	
	void estimatedFuelOnBoard(const FlightStatus&, const FlightRoute&);	
	
/********************************************************************
 * author uwe, Class functional description: 
 * 
 * FMGS Fuel On Board setting 
 *
 ********************************************************************/
//! FMGS Fuel On Board setting
	
	void	setAlternateFuel(double val)							{set_MAPVALUES(p_fob, "ALTN", val * 1000.);}
	
	void	setAlternateTime(double val)							{set_MAPVALUES(p_fob, "ALTN_TIME", val);}
	
	void	setFinalFuel(double val )								{set_MAPVALUES(p_fob, "FINAL", val * 1000.);}
	
	void 	setTripFuel(double val)								{set_MAPVALUES(p_fob, "TRIP", val * 1000.);}
	
	void 	setTripTime(double val)								{set_MAPVALUES(p_fob, "TRIP_TIME", val);}
	
	void	setBlockFuel(double val)								{set_MAPVALUES(p_fob, "BLOCK", val * 1000.);}
	
	void	setReserveFuel(double val)							{set_MAPVALUES(p_fob, "RSV", val * 1000.);}
	
	void	setExtraFuel(double val)								{set_MAPVALUES(p_fob, "EXTRA", val * 1000.);}
	
	void	setExtraTime(double val)								{set_MAPVALUES(p_fob, "EXTRA_TIME", val);}
	
	bool	isExtraFuelValid();
	
	bool	isAlternateFuelValid();
	
	bool	isFinalFuelValid();
	
	bool 	isTripFuelValid();
	
	bool	isBlockFuelValid();
	
	bool	isReserveFuelValid();
	

	double getTaxiFuel() const									{return fob.find("TAXI")->second / 1000.;}
	
	double getAlternateFuel() const								{return fob.find("ALTN")->second / 1000.;}
	
	double getAlternateTime() const								{return fob.find("ALTN_TIME")->second;}
	
	double getFinalFuel() const									{return fob.find("FINAL")->second / 1000.;}
	
	double getFinalTime() const									{return fob.find("FINAL_TIME")->second;}
	
	double getTripFuel() const									{return fob.find("TRIP")->second / 1000.;}
	
	double getTripTime() const									{return fob.find("TRIP_TIME")->second;}
	
	double getBlockFuel() const									{return fob.find("BLOCK")->second / 1000.;}

	double getReserveFuel() const								{return fob.find("RSV")->second / 1000.;}
	
	double getReserveFuelPerc() const							{return fob.find("RSV_PERC")->second;}
	
	double getExtraFuel() const									{return fob.find("EXTRA")->second;}
	
	double getEstimatedFuelOnBoard() const						{return fob.find("EFOB")->second / 1000.;}
	
	double getFuelQuantityDeparture() const						{return fob.find("FQ_DEP")->second / 1000.;}
	
	double getFuelQuantityArrival() const						{return fob.find("FQ_ARR")->second / 1000.;}
/*
 * Aircraft operation & limit condition
 */	
/********************************************************************
 * author uwe, Class functional description: 
 * 
 * FMGS Fuel On Board setting 
 *
 ********************************************************************/
//! FMGS Fuel On Board setting
	
	double getSpeedVS() const										{return p_FMS_FMGC->get_SPEED("VS");}
	
	double getSpeedVAP() const									{return p_FMS_FMGC->get_SPEED("VAP");}
	
	double getSpeedVLS() const									{return p_FMS_FMGC->get_SPEED("VLS");}
	
	double getSpeedV1() const										{return p_FMS_FMGC->get_SPEED("V1");}
	
	double getSpeedVR() const										{return p_FMS_FMGC->get_SPEED("VR");}
	
	double getSpeedV2() const										{return p_FMS_FMGC->get_SPEED("V2");}
	
	double getSpeedF() const										{return p_FMS_FMGC->get_SPEED("F");}
	
	double getSpeedS() const										{return p_FMS_FMGC->get_SPEED("S");}
	
	double getSpeedVFE() const									{return p_FMS_FMGC->get_SPEED("VFE");}
	
	double getSpeedGD() const										{return p_FMS_FMGC->get_SPEED("GD");}
	
	double getSpeedVMAX() const									{return p_FMS_FMGC->get_SPEED("VMAX");}
	
	double getSpeedDesUP() const									{return p_FMS_FMGC->get_SPEED("DES_UP");}
	
	double getSpeedDesLOW() const								{return p_FMS_FMGC->get_SPEED("DES_LOW");}
	
	double getSpeedVAPP() const									{return p_FMS_FMGC->get_SPEED("VAPP");}
	
	double getSpeedVREF() const									{return p_FMS_FMGC->get_SPEED("VREF");}
	
/********************************************************************
 * author uwe, Class functional description: 
 * 
 * Engine setting 
 *
 ********************************************************************/
//! Engine setting
	
	double getEngineN1RLow() const								{return p_FMS_FMGC->get_ENGINE_COND("N1R_LOW");}
	
	double getEngineN1RHigh() const								{return p_FMS_FMGC->get_ENGINE_COND("N1R_HIGH");}

/********************************************************************
 * author uwe, Class functional description: 
 * 
 * Aircraft Weight 
 *
 ********************************************************************/	
//! Aircraft Weight
	
	double 	getTOW() const											{return TOW / 1000.;}
	
	double 	getLW() const											{return LW / 1000.;}
	
/********************************************************************
 * author uwe, Class functional description: 
 * 
 * FMGS messages and ident version 
 *
 ********************************************************************/
//! FMGS messages and ident version
	
	string 	getAircraftType() const								{return AC_MODEL;}
		
	map<string,double> getFMGCAircraftDefinition()	const		{return p_FMS_FMGC->get_FMGC_AC_DEFINITION();}

	string 	getFMGSIdent()	const									{return FMS_FMGS_IDENT;}
	
	multimap<string,string> getFMGSMessage() const				{return FMGS_MESSAGE;}
	
	multimap<string,string> getFMGCMessage() const				{return p_FMS_FMGC->get_FMGC_MESSAGE();}
	
protected:
	
/********************************************************************
 * author uwe, Class functional description: 
 * 
 * LNAV Methods
 *
 ********************************************************************/	
//! LNAV Methods
	
	void 	greatCircleCalculation(const double&, const double&, const double&, const double&);
	
	void 	greatCircleCalcInterimWaypoint(const double&, const double&, const double&, const double&, const double&);
		
	void 	positionRadialDistance(const double&, const double&, const double&, const double&);

	void 	initLateralNavigation();	
	
	void 	speedLateralNavigation(const double&, const double&, const double&, const double&);

	void 	windLateralNavigation(const double&, const double&, const double&, const double&);	

	void 	windPrediction();	
	
/********************************************************************
 * author uwe, Class functional description: 
 * 
 * VNAV Methods
 *
 ********************************************************************/
//! VNAV Methods
	
	void 	profileVerticalNavigation();
	
	void 	speedProfileVerticalNavigation();
	
	void	envelopeVerticalNavigation();
	
	void	managedVerticalNavigation();
	
	void	managedVerticalNavigationTO();
	
	void	managedVerticalNavigationCLB(int);
	
	void	managedVerticalNavigationCRZ(int);
	
	void	managedVerticalNavigationDES(int);
	
	void	managedVerticalNavigationAPP(int);
	
	double	heightPredictionVerticalNavigation(double, double, double);
		
	double	heightDistancePredictionVerticalNavigation(double, double, double);
	
	void	speedSegmentsVerticalNavigation();
							
/********************************************************************
 * author uwe, Class functional description: 
 * 
 * Fuel Calculation Methods
 *
 ********************************************************************/
//! Fuel Calculation Methods
	
	void estimatedFuelOnBoard();

	void fuelCalculation();
	
	void fuelAnalysis(int&);

	
private:
	
	typedef pair <string, double> dpair;
	
	FMS_FMGC*		p_FMS_FMGC;
	
	string 			AC_MODEL;
	bool			AC_MODEL_flag;
	string			file;
	char			c[100];
	string			read_buffer;
	string			s1;
	double			eps;	// calculation precision of computer
	string			log_display;
	
	bool			clb_flag;
	bool			crz_flag;

	
	map <string, double>		AC_USER_DEF;
	multimap <string, string>	FMGS_MESSAGE;
/*
 * Software identifier
 */
	
	string FMS_FMGS_IDENT;

/*
 * AC Database
 */
	map <string,string>	acdb_oem;
	map <string,string>	acdb_designator;
	map <string,string>	acdb_type;
	map <string,string>	acdb_wt_class;
	
	map <string,string>*	p_acdb_oem;
	map <string,string>*	p_acdb_designator;
	map <string,string>*	p_acdb_type;
	map <string,string>*	p_acdb_wt_class;

/*
 * A/C operation conditions
 */
	const Airport*				p_airport_const;
	Waypoint*					p_waypoint;
	const Waypoint*			p_waypoint_const;
	WaypointMetaData*			p_waypoint_meta_data;
	WaypointMetaData			waypoint_meta_data;
	WaypointRestrictions*		p_waypoint_restrictions;
	WaypointRestrictions		waypoint_restrictions;
	const FlightRoute*		p_active_route;
	
	map <string, double> 	ac_ops_cond;
	map <string, double>*	p_ac_ops_cond;	
	map <string, string> 	flap_config;
	
	double					prel_gw;
	double					TOW;
	double					LW;
	double					ZFW;
	double 					Flx_temp;
	double					Derated_N1;
	double					Thrust_alt;
	double					Accl_alt;
	double					Trans_alt;
	double					mach;
	double					SAT;			// Cruise SAT programmed by pilot
	double					OAT;			// Outside AirTemperature at dep. Airport
	double 					crz_alt;
	double 					cost_index;
	int						IAF;
	int						F_CONF;
	string					F_conf;
	
	double					envelope_clb_ang;
	double					envelope_clb_dist;
	double					envelope_des_ang;
	double					envelope_des_dist;
/*
 * Leg_data_variables
 */
	double 					distance;
	double					alpha_global;			//course in 360 rose definition
	double					alpha_init;				//GC initial Course
	double 					phi_b;
	double					lambda_b;
	map <string, double> 	gc_nav;
	string					coordinates;
	
	vector <double>		cstr_spd;
	vector <double>		cstr_alt;
	vector <double>		cstr_dist;
	vector <string>		cstr_alt_mode;
	vector <string>		cstr_spd_mode;


	
	map <string, double> 	vnav_speed_seg;
	map <string, double>* 	p_vnav_speed_seg;

/*
 *  relative Wind components and environment
 */
	double					aoa_true;
	double					true_leg_spd;
	double					avg_wind_spd;
	double					avg_wind_hdg;
	
/*
 * FPLN / Leg computation
 */

	vector	<int>		fpl_mode;
	vector	<string>	fpl_name;
	vector 	<double>	fpl_lat;
	vector 	<double>	fpl_lon;
	vector 	<double>	fpl_alt;	
	vector	<double>	fpl_ias;
	vector	<double>	fpl_vs;
	vector	<double>	fpl_windhdg;
	vector	<double>	fpl_windspd;
	vector	<string>	fpl_thrust_cond;		//leg thrust condition per leg;
	vector	<double>	fpl_eng_n1;
	vector	<double>	fpl_estGW;
	vector	<double>	fpl_efob;
	vector	<double>	fpl_eta;
	vector	<double>	fpl_leg_dist;			//leg distance from starting point;
	vector	<double>	fpl_leg_dist_inv;		//leg distance from arrival point;



	vector 	<double>	leg_dist;				//leg distance
	vector	<double>	leg_phi;				//leg course in 360°
	vector	<double>	leg_hdg;				//leg ac heading after wind correction;
	vector 	<double> 	leg_tas;				//leg true air speed
	vector	<double>	leg_gs;					//leg avg. ground speed
	vector 	<double> 	leg_ete;				//leg estimated time enroute
	vector	<double>	leg_fuel;				//leg fuel con from starting point;
	vector	<double>	leg_windhdg;
	vector	<double>	leg_windspd;
	vector	<string>	leg_ops_mode;

	vector	<int>*		p_fpl_mode;
	vector	<string>*	p_fpl_name;
	vector 	<double>*	p_fpl_lat;
	vector 	<double>*	p_fpl_lon;
	vector 	<double>*	p_fpl_alt;
	vector 	<double>*	p_fpl_frq;
	vector	<double>*	p_fpl_ias;
	vector	<double>*	p_fpl_vs;
	vector	<double>*	p_fpl_windhdg;
	vector	<double>*	p_fpl_windspd;
	vector	<string>*	p_fpl_thrust_cond;
	vector	<double>*	p_fpl_eng_n1;
	vector	<double>*	p_fpl_estGW;
	vector	<double>*	p_fpl_efob;
	vector	<double>*	p_fpl_eta;
	vector	<double>*	p_fpl_leg_dist;		
	vector	<double>*	p_fpl_leg_dist_inv;

	
	vector 	<double>*	p_leg_dist;			
	vector	<double>*	p_leg_phi;				
	vector	<double>*	p_leg_hdg;				
	vector 	<double>*	p_leg_tas;				
	vector	<double>*	p_leg_gs;				
	vector 	<double>*	p_leg_ete;				
	vector	<double>*	p_leg_fuel;
	vector	<double>*	p_leg_windhdg;
	vector	<double>*	p_leg_windspd;
	vector	<string>*	p_leg_ops_mode;
	
	double				total_leg_dist;
	double				altn_dist;
	double				opt_alt;
	
/*
 * Alternate Airport Definition
 */
	map <string, double>	alternate_airport;

/*
 * Time calculation
 */
	time_t	rawtime;
	struct	tm *ptm;
	int		hour;
	int		min;
	string	ETO;
	
/*
 *  FF calculation
 */
	double							est_fob;
	double 							total_efob;
	double 							total_flight_time;
	map	<string,double>			fob;
	map <string,double>* 			p_fob;
	map <string,double>::iterator 	it_fob1;
	map <string,double>::iterator 	it_fob2;
/*
 * EFOB calculation
 */
	vector	<double>	gc_dist_delta;
	vector	<double>	gc_bearing;
	vector	<double> 	efob_cache;
	vector	<double>	fob_cache;

/*
 * Tools
 */
//! Tools
	
	void messageCheck();
	
	void slatsFlapsConfig(const int&);
	
	void engineDeration(string&);

/********************************************************************
 * author uwe, Class functional description: 
 * 
 * Aircraft database and Aircraft checks / classification
 *
 ********************************************************************/	
//! Aircraft database and Aircraft checks / classification
	
	void aircraftDataBase();

	void aircraftDataBaseConvert(string&);	

	bool aircraftDataBaseScan(string&);	
	
	bool aircraftDefinition();

};

}

#endif /*FlightManagementGuidanceSystem_H_*/
