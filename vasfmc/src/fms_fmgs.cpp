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


#include "fms_fmgs.h"

namespace FMS
{

FlightManagementGuidanceSystem::FlightManagementGuidanceSystem(string ACMODEL) : FMS_FMGS_BASIC_FUNCTION()
{	
	if (ACMODEL.length() == 4)
	{
		AC_MODEL = ACMODEL;
		AC_MODEL_flag = false;
	}
	else if (ACMODEL.length() == 5)
	{
		AC_MODEL = ACMODEL.substr(0,4);
		AC_MODEL_flag = true;
	}
	else
	{
		AC_MODEL = "A332";
		AC_MODEL_flag = false;
	}
	

	if(LOG == 1)
	{
		cout << AC_MODEL << endl;
	}
	
	clb_flag = false;
	crz_flag = false;
		
	aircraftDataBase();
	
	if (aircraftDataBaseScan(AC_MODEL) == false )
	{
		FMGS_MESSAGE.insert( pair<string, string> ("FMGS DATA CHECK", "A/C not in database, FMGS uses vasfmc A330 -200"));
		AC_MODEL == "A332";
	}
	else
	{
		if (aircraftDefinition() == false)
		{
			FMGS_MESSAGE.insert( pair<string, string> ("FMGS DATA CHECK", "Helicopter specified, FMGS terminated"));
			return;
		}
	}
	
	static map<string,double> FMGC_AC_DEFINITION;
	
	FMGC_AC_DEFINITION = p_FMS_FMGC->get_FMGC_AC_DEFINITION();
	
	p_fob 				= &fob;
	
	p_fpl_mode			= &fpl_mode;
	p_fpl_name			= &fpl_name;
	p_fpl_lat			= &fpl_lat;
	p_fpl_lon			= &fpl_lon;
	p_fpl_alt			= &fpl_alt;
	p_fpl_ias			= &fpl_ias;
	p_fpl_vs			= &fpl_vs;
	p_fpl_windhdg		= &fpl_windhdg;
	p_fpl_windspd		= &fpl_windspd;
	p_fpl_thrust_cond	= &fpl_thrust_cond;
	p_fpl_eng_n1		= &fpl_eng_n1;
	p_fpl_estGW			= &fpl_estGW;
	p_fpl_efob			= &fpl_efob;
	p_fpl_eta			= &fpl_eta;
	p_fpl_leg_dist		= &fpl_leg_dist;
	p_fpl_leg_dist_inv	= &fpl_leg_dist_inv;
	
	p_ac_ops_cond		= &ac_ops_cond;

	p_leg_dist			= &leg_dist;			
	p_leg_phi			= &leg_phi;				
	p_leg_hdg			= &leg_hdg;				
	p_leg_tas			= &leg_tas;				
	p_leg_gs			= &leg_gs;				
	p_leg_ete			= &leg_ete;				
	p_leg_fuel			= &leg_fuel;
	p_leg_windhdg		= &leg_windhdg;
	p_leg_windspd		= &leg_windspd;
	p_leg_ops_mode		= &leg_ops_mode;
	
	p_vnav_speed_seg	= &vnav_speed_seg;


	log_display = " ";
	
	FMS_FMGS_IDENT  = " Project ->  ";
	FMS_FMGS_IDENT +=  PROJECT;
	FMS_FMGS_IDENT += "  System -> ";
	FMS_FMGS_IDENT += OPS;
	FMS_FMGS_IDENT += "  ";
	FMS_FMGS_IDENT += VERSION;
	FMS_FMGS_IDENT += "  ";
	FMS_FMGS_IDENT += REV;
	FMS_FMGS_IDENT += " A/C Model -> ";
	FMS_FMGS_IDENT += acdb_oem[AC_MODEL];
	FMS_FMGS_IDENT += "  ";
	FMS_FMGS_IDENT += acdb_designator[AC_MODEL];
	FMS_FMGS_IDENT += "  Author ->";
    FMS_FMGS_IDENT += AUTHOR;

}

////////////////////////////////////////////////////////////////////////////

FlightManagementGuidanceSystem::~FlightManagementGuidanceSystem()
{
	delete p_FMS_FMGC;
}

////////////////////////////////////////////////////////////////////////////


void FlightManagementGuidanceSystem::initFMGS(const FlightRoute& active_route
												, const FlightRoute& alternate_route
												, const FlightStatus& flightstatus)
{
/*****************************************************************************************
 * author uwe, Class functional description: FMC input data to calculate the VNAV Plan ,
 * Fuel Predicition and  T/O - Approach speed condition for a tmpy FPL
 * Input -> const FlightRoute& active_route, const FlightRoute& alternate_route, const FlightStatus& flightstatus
 *  
 *****************************************************************************************/
	int index					= 0;
	int slat_flap_notches 		= 0;

	bool flag					= false;
	p_active_route				= &active_route;		
/*
 * Flight Plan initialisation with active route
 */
/*
 * lateral Navigation, calculates distance between fpl waypoints and inits all fpl vectors
 */
	fpl_vs.clear();
	fpl_thrust_cond.clear();		//leg thrust condition per leg;
	fpl_estGW.clear();
	fpl_eta.clear();
	fpl_leg_dist.clear();			//leg distance from starting point;
	fpl_leg_dist_inv.clear();		//inverse leg distance from starting point;
	fpl_eng_n1.clear();
	
	for (index = 0; index < active_route.count() - 1; index++)
	{
		
		p_waypoint_const	= active_route.waypoint(index);
		fpl_lat.push_back(p_waypoint_const->lat());
		fpl_lon.push_back(p_waypoint_const->lon());
		fpl_alt.push_back(0.);
		fpl_vs.push_back(0.);
		fpl_eng_n1.push_back(0.);
		fpl_thrust_cond.push_back("");
		fpl_estGW.push_back(0.);
		fpl_efob.push_back(0.);
		fpl_eta.push_back(0.);
		fpl_mode.push_back(0);
		
		if(((p_waypoint_const->isStar() == true) || (p_waypoint_const->isAppTransition() == true)) && (flag == false))
		{
			IAF 	= index;
			flag 	= true;
		}
/*
 * Altitude Restriction assignment
 */		
		waypoint_restrictions = p_waypoint_const->restrictions();
		
		if (   (waypoint_restrictions.altitudeEqualRestriction() == false)
			|| (waypoint_restrictions.altitudeGreaterRestriction() == false)
			|| (waypoint_restrictions.altitudeSmallerRestriction() == false))
		{
			cstr_alt.push_back(0.);
		}
		else
		{
			cstr_alt.push_back(static_cast<double>(waypoint_restrictions.altitudeRestrictionFt()));
		}
		
/*
 * speed Restriction assignment
 */		

		cstr_spd.push_back(static_cast<double>(waypoint_restrictions.speedRestrictionKts()));
		
	}

/*
 * Airport elevation assignment to active flight plan
 */
	
	p_airport_const = active_route.departureAirport();
	if (p_airport_const != 0)
	{
		cstr_alt.front()	= p_airport_const->elevationFt();
		fpl_alt.front()		= p_airport_const->elevationFt(); 
	}
	
	p_airport_const = alternate_route.destinationAirport();
	if (p_airport_const != 0)
	{
		cstr_alt.back()		= p_airport_const->elevationFt();
		fpl_alt.back()		= p_airport_const->elevationFt();
	}
	
	
/*
 * Flight Plan initialisation alternate route
 */
	
	index = alternate_route.count() - 1;
	p_waypoint_const	= alternate_route.waypoint(index);
	
	if (p_waypoint_const->isAdes() == true)
	{
		alternate_airport.insert( pair<string, double> ("LAT", p_waypoint_const->lat()));
		alternate_airport.insert( pair<string, double> ("LON", p_waypoint_const->lon()));
	}
	else
	{
		index = active_route.count() - 1;
		p_waypoint_const	= active_route.waypoint(index);
		
		if (p_waypoint_const->isAdes() == true)
		{
			alternate_airport.insert( pair<string, double> ("LAT", p_waypoint_const->lat()));
			alternate_airport.insert( pair<string, double> ("LON", p_waypoint_const->lon()));
		}
	}
	
/*
 * Parameter set up
 */
	
	cost_index 					= active_route.costIndex();
	SAT							= active_route.cruiseTemp();
	crz_alt						= active_route.cruiseFl() * 100.;
	
	Thrust_alt					= static_cast<double>(active_route.thrustReductionAltitudeFt());
	Accl_alt					= static_cast<double>(active_route.accelerationAltitudeFt());
	
	mach						= flightstatus.mach;
	ZFW							= flightstatus.zero_fuel_weight_kg;
	OAT							= flightstatus.oat;
	
	//DERATED_ENGINE =
	
	slat_flap_notches			= flightstatus.current_flap_lever_notch;
	
/* 
 * Data consistency check and FMGS set up
 */
	
	slatsFlapsConfig(slat_flap_notches);

// Cost Index -> Profile calculation	
	
	double gross_weight = p_FMS_FMGC->get_AC_DEF("MTOW");
	
	if ((cost_index <= 0.) | (cost_index > 999.))
	{
		mach = p_FMS_FMGC->get_SPEED("MMO") * 0.9;
	}
	else
	{
		
		p_FMS_FMGC->FMGC_GD_SPEED_POLARE(crz_alt, gross_weight);
		p_FMS_FMGC->get_SPEED("GD");
		
		mach =  LINEAR_EQUATION(cost_index, 0., p_FMS_FMGC->get_SPEED("GD_MACH"), 999., p_FMS_FMGC->get_SPEED("MMO"));
	}

// Zero Fuel Weight Check
	
	if (ZFW > p_FMS_FMGC->get_AC_DEF("MLW"))
	{
		FMGS_MESSAGE.insert( pair<string, string> ("FMGS DATA CHECK", "ZFW > MLW"));
		ZFW = p_FMS_FMGC->get_AC_DEF("MLW");
	}
	else if (ZFW < p_FMS_FMGC->get_AC_DEF("DOW") )
	{
		FMGS_MESSAGE.insert( pair<string, string> ("FMGS DATA CHECK", "ZFW < OWE"));
		ZFW = p_FMS_FMGC->get_AC_DEF("DOW");
	}
	
//	FMGS_ENGINE_DERATION(DERATED_ENGINE);

}

////////////////////////////////////////////////////////////////////////////

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 
 * FMGS -> FMGC Aircraft Configuration
 * 
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */

bool FlightManagementGuidanceSystem::aircraftDefinition()
{
	string ac_type 		= acdb_type.find(AC_MODEL)->second;
	string ac_wtclass	= acdb_wt_class.find(AC_MODEL)->second;

	if (ac_type[0] == 'H' || ac_type[0] == 'h')
	{
		FMGS_MESSAGE.insert( pair<string, string> ("FMGS UDEF", "Helicopter specified"));
		return false;
	}
	else
	{
		if (ac_type[0] == 'L' || ac_type[0] == 'L')			// L = Lower Wing
		{
			if (ac_type[2] == 'J' || ac_type[2] == 'j')		// J = Jet
			{
				if (ac_type[1] == '1')						// X = No of Engines
				{
					
				}
				else if (ac_type[1] == '2')					// X = No of Engines
				{
					
					if 		(AC_MODEL == "A30B")
						p_FMS_FMGC			= new A306_FMGC(AC_MODEL);
					else if (AC_MODEL == "A306")
						p_FMS_FMGC			= new A306_FMGC(AC_MODEL);					
					else if (AC_MODEL == "A310")
						p_FMS_FMGC			= new A310_FMGC(AC_MODEL);
					else if (AC_MODEL == "A318")
						p_FMS_FMGC			= new A320_FMGC(AC_MODEL); 
					else if (AC_MODEL == "A319")
						p_FMS_FMGC			= new A320_FMGC(AC_MODEL);
					else if (AC_MODEL == "A320")
						p_FMS_FMGC			= new A320_FMGC(AC_MODEL);
					else if (AC_MODEL == "A321")
						p_FMS_FMGC			= new A320_FMGC(AC_MODEL);
					else if (AC_MODEL == "A332")
						p_FMS_FMGC			= new A332_FMGC(AC_MODEL);
					else if (AC_MODEL == "A333")
						p_FMS_FMGC			= new A332_FMGC(AC_MODEL);
					else if (AC_MODEL == "B731")
						p_FMS_FMGC			= new B73X_FMGC(AC_MODEL);					
					else if (AC_MODEL == "B732")
						p_FMS_FMGC			= new B73X_FMGC(AC_MODEL);
					else if (AC_MODEL == "B733")
						p_FMS_FMGC			= new B73X_FMGC(AC_MODEL);
					else if (AC_MODEL == "B734")
						p_FMS_FMGC			= new B73X_FMGC(AC_MODEL);
					else if (AC_MODEL == "B735")
						p_FMS_FMGC			= new B73X_FMGC(AC_MODEL);
					else if (AC_MODEL == "B736")
						p_FMS_FMGC			= new B73X_FMGC(AC_MODEL);
					else if (AC_MODEL == "B737")
						p_FMS_FMGC			= new B73X_FMGC(AC_MODEL);
					else if (AC_MODEL == "B738")
						p_FMS_FMGC			= new B73X_FMGC(AC_MODEL);
					else if (AC_MODEL == "B739")
						p_FMS_FMGC			= new B73X_FMGC(AC_MODEL);
/*					else if (AC_MODEL == "B752")
						p_FMS_FMGC			= new B75X_FMGC;
					else if (AC_MODEL == "B753")
						p_FMS_FMGC			= new B75X_FMGC;
					else if (AC_MODEL == "B762")
						p_FMS_FMGC			= new B76X_FMGC;
					else if (AC_MODEL == "B763")
						p_FMS_FMGC			= new B76X_FMGC;
					else if (AC_MODEL == "B764")
						p_FMS_FMGC			= new B76X_FMGC;
					else if (AC_MODEL == "B772")
						p_FMS_FMGC			= new B77X_FMGC;
					else if (AC_MODEL == "B773")
						p_FMS_FMGC			= new B77X_FMGC; */
					else if(ac_type == "L2J" && ac_wtclass == "M")
						p_FMS_FMGC			= new A320_FMGC("A319");
					else if(ac_type == "L2J" && ac_wtclass == "H")
						p_FMS_FMGC			= new A332_FMGC("A332");
				}
				else if (ac_type[1] == '3')					// X = No of Engines
				{
/*					if (AC_MODEL == "DC10")
						p_FMS_FMGC			= new DC10_FMGC;*/
				}
				else if (ac_type[1] == '4')					// X = No of Engines
				{
					if (AC_MODEL == "A342")
						p_FMS_FMGC			= new A342_FMGC(AC_MODEL);
					else if (AC_MODEL == "A343")
						p_FMS_FMGC			= new A342_FMGC(AC_MODEL);
					else if (AC_MODEL == "A345")
						p_FMS_FMGC			= new A345_FMGC(AC_MODEL);
					else if (AC_MODEL == "A346")
						p_FMS_FMGC			= new A345_FMGC(AC_MODEL);
					else if (AC_MODEL == "A380")
						p_FMS_FMGC			= new A380_FMGC(AC_MODEL);
					else if (AC_MODEL == "A388")
						p_FMS_FMGC			= new A380_FMGC(AC_MODEL);
					else if (AC_MODEL == "B747")
						p_FMS_FMGC			= new B74X_FMGC(AC_MODEL);
					else if (AC_MODEL == "B741")
						p_FMS_FMGC			= new B74X_FMGC(AC_MODEL);
					else if (AC_MODEL == "B742")
						p_FMS_FMGC			= new B74X_FMGC(AC_MODEL);
					else if (AC_MODEL == "B743")
						p_FMS_FMGC			= new B74X_FMGC(AC_MODEL);
					else if (AC_MODEL == "B744")
						p_FMS_FMGC			= new B74X_FMGC(AC_MODEL);
					else if (AC_MODEL == "B74D")
						p_FMS_FMGC			= new B74X_FMGC(AC_MODEL);
					else if (AC_MODEL == "B74R")
						p_FMS_FMGC			= new B74X_FMGC(AC_MODEL);
					else if (AC_MODEL == "B74S")
						p_FMS_FMGC			= new B74X_FMGC(AC_MODEL);
//					else if(ac_type == "L4J" && ac_wtclass == "M")
//					p_FMS_FMGC			= new B461_FMGC;					
					else if(ac_type == "L4J" && ac_wtclass == "H")
						p_FMS_FMGC			= new B74X_FMGC("B743");
					
				}
				else if (ac_type[1] == '6')					// X = No of Engines
				{
					
				}
				else if (ac_type[1] == '8')					// X = No of Engines
				{
					
				}
				else
				{
					if (ac_wtclass == "H")
					{
						p_FMS_FMGC			= new A332_FMGC("A332");
					}
					else if (ac_wtclass == "M")
					{
						p_FMS_FMGC			= new A320_FMGC("A319");
					}
					else
					{
						// p_FMS_FMGC			= new Lear_Jet_FMGC;
					}

				}
			}
			else if (ac_type[2] == 'T' || ac_type[2] == 't')
			{
				if (ac_type[1] == '1')
				{
					
				}
				else if (ac_type[1] == '2')
				{
/*					if 	(AC_MODEL == "B350")
						p_FMS_FMGC			= new B350_FMGC;
					else if(ac_type == "L2T" && ac_wtclass == "L")
						p_FMS_FMGC			= new B350_FMGC;					
					else if(ac_type == "L2J" && ac_wtclass == "M")
						p_FMS_FMGC			= new C160_FMGC;    */
				}
				else if (ac_type[1] == '3')
				{
					
				}
				else if (ac_type[1] == '4')
				{
					
				}
				else
				{
					if (ac_wtclass == "H")
					{
						// p_FMS_FMGC			= new A400M_FMGC;
					}
					else if (ac_wtclass == "M")
					{
						// p_FMS_FMGC			= new C160_FMGC;
					}
					else
					{
						// p_FMS_FMGC			= new B350_FMGC;
					}
				}
			}
			else
			{
				if (ac_type[1] == '1')
				{
					
				}
				else if (ac_type[1] == '2')
				{
					
				}
				else if (ac_type[1] == '3')
				{
					
				}
				else if (ac_type[1] == '4')
				{
					
				}
				else
				{
					FMGS_MESSAGE.insert( pair<string, string> ("FMGS UDEF", "unspecified aircraft, A332 chosen"));
					p_FMS_FMGC			= new A332_FMGC("A332");
				}
			}
		}
		else
		{
			FMGS_MESSAGE.insert( pair<string, string> ("FMGS UDEF", "unspecified aircraft, A332 chosen"));
			p_FMS_FMGC			= new A332_FMGC("A332");
			return true;
		}
	}
	
	return true;

}

////////////////////////////////////////////////////////////////////////////

void FlightManagementGuidanceSystem::aircraftDataBase()
{
	p_acdb_oem				= &acdb_oem;
	p_acdb_designator		= &acdb_designator;
	p_acdb_type				= &acdb_type;
	p_acdb_wt_class			= &acdb_wt_class; 

/*
 * Open aircraft database file
 */
	file = "./aircraft_data/fmgs_database/aircraft.dat";
	
	if (file.empty())
	{
		cerr << " Aircraft database not found " << endl;
		FMGS_MESSAGE.insert( pair<string, string> ("FMGS AC_DB", "Aircraft Database not found"));
		return;
	}
			
	ifstream acdb_file (file.c_str(),ios::in);
			
	if (!acdb_file)
	{
		cerr << "FILE : " << file  << " could not be opened" << endl;
		FMGS_MESSAGE.insert( pair<string, string> ("FMGS AC_DB", "Cannot open Aircraft Database file"));
		acdb_file.close();
	}
	else
	{
		do
		{
			acdb_file >> read_buffer;
		
			aircraftDataBaseConvert(read_buffer);
		}
		while (acdb_file.eof() != true);
	}
}

////////////////////////////////////////////////////////////////////////////


void FlightManagementGuidanceSystem::aircraftDataBaseConvert(string& r_buffer)
{
	int pos1 	= 0;
	int pos2 	= 0;
	int	len		= 0;
	int dif		= 0;
	
	s1 = "";

	len = r_buffer.length();
	
// ac id	
	pos1 = r_buffer.find_first_of(":", 0);
	
	s1 = r_buffer.substr(0 , pos1);

// ac oem	
	pos2 = r_buffer.find_first_of(":", pos1 + 1);
	
	dif = pos2 - 1 - pos1;
	
	set_MAPVALUES(p_acdb_oem, s1, r_buffer.substr(pos1 + 1, dif));

// ac_designator	
	pos1 = pos2;
	
	pos2 = r_buffer.find_first_of(":", pos1 + 1);
	
	dif = pos2 - 1 - pos1;
	
	set_MAPVALUES(p_acdb_designator, s1, r_buffer.substr(pos1 + 1, dif));
	
// ac_type	
	pos1 = pos2;
	
	pos2 = r_buffer.find_first_of(":", pos1 + 1);
	
	dif = pos2 - 1 - pos1;
	
	set_MAPVALUES(p_acdb_type, s1, r_buffer.substr(pos1 + 1, dif));

	
// ac_designator
	pos1 = pos2;

	set_MAPVALUES(p_acdb_wt_class, s1, r_buffer.substr(pos1 + 1, len -1));

	if(log_display == "ON")
	{
		cout << s1 << " "<< acdb_oem[s1]<< " "<< acdb_designator[s1]<< " "<< acdb_type[s1]<< " "<< acdb_wt_class[s1] << endl;
	}	
}

////////////////////////////////////////////////////////////////////////////

bool FlightManagementGuidanceSystem::aircraftDataBaseScan(string& ac_model_id)
{
	map<string,string>::iterator acdb_it;
	
	acdb_it = acdb_oem.find(ac_model_id);
	
	if (acdb_it == acdb_oem.end())
	{
		FMGS_MESSAGE.insert( pair<string, string> ("FMGS AC_DB", "Aircraft not in database"));
		return false;
	}
	else
		return true;
	
}

////////////////////////////////////////////////////////////////////////////

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 
 * FMGS Control 
 * 
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */

void FlightManagementGuidanceSystem::fuelPrediction()
{
	double TOW_prev 	= 0.;
	int icount  		= 0;
/*
 *  Calculation of the initial lateral flight plan in LNAV mode
 */
	initLateralNavigation();

/*
 * Calculation of the first Aircraft weight assumed as rectangular flight profile
 */ 
	estimatedFuelOnBoard();
/*
 * Calculate trip wind forecast
 */	
	windPrediction();
/*
 * Calculate VNAV Profile
 */	
	envelopeVerticalNavigation();
	profileVerticalNavigation();
	speedProfileVerticalNavigation();
	speedSegmentsVerticalNavigation();
	
/*
 * Calculate Fuel on Board and reserve quantities
 */	
	do
	{
		icount++;
		
		TOW_prev = TOW;
		
		managedVerticalNavigation();
		
		fuelCalculation();
		
		fuelAnalysis(icount);
		
		est_fob  = fob["BLOCK"];
		
		prel_gw = TOW;
	}
	while ((abs(TOW - TOW_prev) > 0.01 * p_FMS_FMGC->get_AC_COND("MTOW")) && (icount < 3));
	
	messageCheck();

}

////////////////////////////////////////////////////////////////////////////

void FlightManagementGuidanceSystem::temporaryFlightPlan(	const double& LAT, const double& LON, const double& ALT, const double& N1,
									const double& SPEED, const double& VS, const double& GW, const double& FOB,
									const double& WINDHDG, const double& WINDSPD, const double& OATEMP)
{
	int mode		= 0;
	string th_cond	= " ";
	
	OAT 			= OATEMP;

	
	if (SPEED > 1. && SPEED < 20.)
	{
		fuelPrediction();
	}
	else if (SPEED > p_FMS_FMGC->get_SPEED("VR"))
	{
/*
 * Calculation of the initial lateral flight plan in LNAV mode
 */
		initLateralNavigation();
		
/*
 * Add current Leg dist to the flight plan if airborne
 */	
		greatCircleCalculation(LAT, LON, fpl_lat[0], fpl_lon[0]);
			
		total_leg_dist += gc_nav["DISTANCE"];

/*
 * Preset the initial fpl vectors
 */
		vector_Dinsert(p_leg_dist, 			gc_nav["DISTANCE"]);
		vector_Dinsert(p_leg_phi, 			gc_nav["ALPHA"]);
		vector_Dinsert(p_leg_hdg, 			0.);
		vector_Dinsert(p_leg_tas, 			0.);
		vector_Dinsert(p_leg_gs, 			0.);
		vector_Dinsert(p_leg_ete, 			0.);
		vector_Dinsert(p_leg_fuel, 			0.);
		vector_Dinsert(p_leg_windhdg, 		0.);
		vector_Dinsert(p_leg_windspd, 		0.);
		vector_Sinsert(p_leg_ops_mode, 		"CLB");

		if (VS > 20.)
		{
			mode = 1;
			th_cond = "CLB";
		}
		else if (VS < -20.)
		{
			mode = 3;
			th_cond = "DES";
		}
		else
		{
			mode = 2;
			th_cond = "CRZ";
		}
		vector_Iinsert(p_fpl_mode,			mode);
		vector_Sinsert(p_fpl_name, 			"POS");
		vector_Dinsert(p_fpl_lat, 			LAT);
		vector_Dinsert(p_fpl_lon, 			LON);
		vector_Dinsert(p_fpl_alt, 			ALT);
		vector_Dinsert(p_fpl_ias, 			SPEED);
		vector_Dinsert(p_fpl_vs, 			VS);
		vector_Dinsert(p_fpl_windhdg, 		WINDHDG);
		vector_Dinsert(p_fpl_windspd, 		WINDSPD);
		vector_Sinsert(p_fpl_thrust_cond,	th_cond);
		vector_Dinsert(p_fpl_eng_n1, 		N1);
		vector_Dinsert(p_fpl_estGW, 		GW);
		vector_Dinsert(p_fpl_efob, 			FOB);
		vector_Dinsert(p_fpl_eta, 			0.);
		vector_Dinsert(p_fpl_leg_dist,		0.);
		vector_Dinsert(p_fpl_leg_dist_inv, 	total_leg_dist);
		
/*
 * Calculate trip wind forecast
 */	
		windPrediction();		
		
/*
 * Calculate profile
 */
		
		managedVerticalNavigation();
		
	}
	else
	{
		FMGS_MESSAGE.insert( pair<string, string> ("FMGS DATA CHECK", "Active flight plan missing"));
	}

}

////////////////////////////////////////////////////////////////////////////


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 
 * Lateral Navigation (LNAV) and Wind Calculation
 * 
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */

void FlightManagementGuidanceSystem::initLateralNavigation()
{
	leg_dist.clear();				//leg distance
	leg_phi.clear();				//leg course in 360°
	leg_hdg.clear();				//leg ac heading after wind correction;
	leg_tas.clear();				//leg true air speed
	leg_gs.clear();					//leg avg. ground speed
	leg_ete.clear();				//leg estimated time enroute
	leg_fuel.clear();				//leg fuel con from starting point;

/*
 * Determine number of legs -> Number of waypoints - 1
 */
	int max = p_active_route->count();
	
	total_leg_dist 		= 0.;
	
	fpl_leg_dist.push_back(0.);
	
	fpl_eta.push_back(0.);
	
/*
 * determine leg properties
 */
	for (int i = 0; i < max - 1; i++)
	{
/*
 * determine leg distance and course from waypoint i bis i + 1, first waypoint is with index 0.
 * Determine the first leg properties like track and leg distance, first leg starts with index 0.
 */

		leg_dist.push_back(p_active_route->distanceNMToNextWaypoint(i));

		leg_phi.push_back(p_active_route->distanceNMToNextWaypoint(i));
		
		total_leg_dist += p_active_route->distanceNMToNextWaypoint(i);
		
		fpl_leg_dist.push_back(total_leg_dist);

		leg_hdg.push_back(0.);	
		leg_tas.push_back(0.);	
		leg_gs.push_back(0.);	
		leg_ete.push_back(0.);
		leg_fuel.push_back(0.);
		leg_windhdg.push_back(0.);
		leg_windspd.push_back(0.);
		leg_ops_mode.push_back("");


		if(LOG == 1)
		{
			cout << i <<" -----------------------------------------------------------------------------------------------" << endl;
			cout << fpl_name[i] << " " << fpl_lat[i] << " "<< fpl_name[i + 1] << " " << fpl_lat[i + 1] << endl;
			cout << "leg dist " << leg_dist[i]<< " Total dist " << fpl_leg_dist[i + 1] << endl;

		}
		

	}
	
/*
 * determine the inverse leg-sum per waypoint
 */
	double inv_total_leg_dist = fpl_leg_dist.back();
	
	fpl_leg_dist_inv.push_back(inv_total_leg_dist);
	
	for (int i = 0; i < max - 1; i++)
	{
			inv_total_leg_dist -= leg_dist[i];
			
			fpl_leg_dist_inv.push_back(inv_total_leg_dist);
		
	}

	
/*
 * Determine Alternate Airport lateral distance
 */

	greatCircleCalculation(fpl_lat.back(), fpl_lon.back(), alternate_airport["LAT"], alternate_airport["LON"]);
	
	altn_dist 	= gc_nav["DISTANCE"];
	
}

////////////////////////////////////////////////////////////////////////////

void FlightManagementGuidanceSystem::speedLateralNavigation(const double& leg_course, const double& ac_spd, const double& wind_hdg, const double& wind_spd)
{
/*
 * Difference between course and wind_hdg
 */
	double leg_course_rad 	= deg_to_rad(leg_course);
	double wind_hdg_rad		= deg_to_rad(wind_hdg);
	
	double aoa = leg_course_rad - asin(sin(wind_hdg_rad - leg_course_rad) * wind_spd / ac_spd); // angle of attack of aircraft with wind impact
	
	true_leg_spd = cos(aoa - leg_course_rad) * ac_spd + cos(wind_hdg_rad - leg_course_rad) * wind_spd; // represents the Ground speed of the aircraft
	
	aoa_true = rad_to_deg(aoa);

}

////////////////////////////////////////////////////////////////////////////

void FlightManagementGuidanceSystem::windLateralNavigation(const double& wind_hdg1, const double& wind_spd1, const double& wind_hdg2, const double& wind_spd2)
{
/*
 * Calculation of average wind conditions on a defined leg
 */
	double wind_hdg1_rad 	= deg_to_rad(wind_hdg1);	
	double wind_spd1_y		= wind_spd1 * cos(wind_hdg1_rad);
	double wind_spd1_x		= wind_spd1 * sin(wind_hdg1_rad);

	double wind_hdg2_rad 	= deg_to_rad(wind_hdg2);
	double wind_spd2_y		= wind_spd2 * cos(wind_hdg2_rad);
	double wind_spd2_x		= wind_spd2 * sin(wind_hdg2_rad);	
	
	double avg_y = (wind_spd1_y + wind_spd2_y) / 2.;
	double avg_x = (wind_spd1_x + wind_spd2_x) / 2.;
	
	avg_wind_spd = sqrt(pow(avg_y,2) + pow(avg_x,2));
	
	if (avg_x != 0)
	{
		avg_wind_hdg = rad_to_deg( atan2(avg_y, avg_x));
	}
	else
	{
		if ( avg_y > 0)
			avg_wind_hdg = 0.;
		else
			avg_wind_hdg = 180.;
	}	
}

////////////////////////////////////////////////////////////////////////////

void FlightManagementGuidanceSystem::windPrediction()
{
	double wind_spd;
	double wind_hdg;
	
	int max1 	= static_cast<int> (fpl_name.size());
	int max 	= static_cast<int> (fpl_windspd.size());

	if (max == 0)
	{
		for( int i = 0; i <= max1; i++)
		{
			fpl_windhdg.push_back(0.);
			fpl_windspd.push_back(0.);
		}
		return;
	}
	else
	{
		wind_spd = fpl_windspd[0];
		wind_hdg = fpl_windhdg[0];
	}
	
;
	
/*
 * determine leg properties
 */
	for (int i = 1; i <= max; i++)
	{
		if (fpl_windspd[i] != 0.)
		{
			windLateralNavigation(wind_hdg, wind_spd, fpl_windhdg[i], fpl_windspd[i]);
			leg_windspd[i - 1] = avg_wind_hdg;
			leg_windspd[i - 1] = avg_wind_spd;
		}
		wind_spd = fpl_windspd[i];
		wind_hdg = fpl_windhdg[i];
	}
}

////////////////////////////////////////////////////////////////////////////

void FlightManagementGuidanceSystem::greatCircleCalculation(const double& Lat1, const double& Lon1, const double& Lat2, const double& Lon2)
{
/*
 * arc conversion from deg to rad
 */
	double lat1 = deg_to_rad(Lat1);
	double lon1 = -1. * deg_to_rad(Lon1);
	double lat2 = deg_to_rad(Lat2);
	double lon2 = -1 * deg_to_rad(Lon2);


	std::MASCHD COP(1.);
	double dcop = COP.COP_PRECISION();
	eps = 4. * dcop;
	
	if (cos(lat1) < eps)
	{
		if (lat1 > 0.)
			alpha_init = PI;
		else
			alpha_init = 2. * PI;
	}
	else
	{
		double alpha_rad 	= 2. * asin(sqrt(pow(sin((lat1 - lat2) / 2.) , 2) + cos(lat1) * cos(lat2) * pow(sin((lon1 - lon2) / 2.),2)));
		distance			= alpha_rad * 180. * 60. / PI;
		alpha_init			= acos((sin(lat2)-sin(lat1) * cos(alpha_rad)) / (sin(alpha_rad) * cos(lat1)));
		
		if (sin(lon2 - lon1) < 0.)
			alpha_init = alpha_init * 180 / PI;
		else
			alpha_init = (2. * PI - alpha_init) * 180. / PI;
	}
	
	gc_nav.clear();
	gc_nav.insert( pair<string, double> ("ALPHA", alpha_init));
	gc_nav.insert( pair<string, double> ("DISTANCE", distance));
}

////////////////////////////////////////////////////////////////////////////

void FlightManagementGuidanceSystem::greatCircleCalcInterimWaypoint(const double& Lat1, const double& Lon1, const double& Lat2, const double& Lon2, const double& fraction)
{
/*
 * arc conversion from deg to rad
 */
	double A	= 0.;
	double B	= 0.;
	double x	= 0.;
	double y	= 0.;
	double z 	= 0.;
	double lat	= 0.;
	double lon	= 0.;
	double dist	= 0.;
	
	double lat1 = deg_to_rad(Lat1);
	double lon1 = -1. * deg_to_rad(Lon1);
	double lat2 = deg_to_rad(Lat2);
	double lon2 = -1 * deg_to_rad(Lon2);
	
	if ((lat1 + lat2 == 0) && (lon1 - lon2 == PI))
	{
		lat = lat1;
		lon = lon1;
	}
	else
	{
		greatCircleCalculation(Lat1, Lon1, Lat2, Lon2);
		
		dist = gc_nav["DISTANCE"] / 60.; // distance in radians
		
		A = sin(( 1. - fraction) * dist / sin(dist));
		B = sin(fraction * dist) / sin(dist);
		
		x = A * cos(lat1) * cos(lon1) + B * cos(lat2) * cos(lon2);
		y = B * cos(lat1) * sin(lon1) + B + cos(lat2) * sin(lon2);
		z = A * sin(lat1);
		
		lat = atan2(z, sqrt(pow(x,2) + pow(y,2)));
		lon = atan2(y, x);
	}
	
	lat = rad_to_deg(lat);
	lon = rad_to_deg(lon);
	
	gc_nav.clear();
	gc_nav.insert( pair<string, double> ("LAT", lat));
	gc_nav.insert( pair<string, double> ("LON", lon));
		
}

////////////////////////////////////////////////////////////////////////////

void FlightManagementGuidanceSystem::positionRadialDistance(const double& lat, const double& lon, const double& radial, const double& distance)
{
/*
 * arc conversion from deg to rad
 */
	double lambda 	= 0.;
	double phi		= 0.;
//	double longc		= 0.;
	
	double lat1 = deg_to_rad(lat);
	double lon1 = deg_to_rad(lon);
	double rad 	= deg_to_rad(radial);
	double dist = deg_to_rad(distance) / 60.;
/*	
	double latgc = asin(sin(lat1) * cos(dist) + cos(lat1) * sin(dist) * cos(rad));
	
	if (cos(lat1) == 0.)
	{
		longc = lon1;
	}
	else
	{
		longc = -1. * (mod(-lon1 - asin(sin(rad) * sin(dist) / cos(latgc)) + PI, 2. * PI) - PI);
	}
	
*/	
	phi 	= lat1 + dist * cos(rad);
	lambda	= lon1 + dist * sin(rad) / cos((lat1 + phi)/2.);
	
	if (lambda > 180.)
		lambda = 360 - lambda;
	
	phi_b		= rad_to_deg(phi);
	lambda_b	= rad_to_deg(lambda);

}

////////////////////////////////////////////////////////////////////////////

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 
 * Vertical Navigation VNAV
 * 
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */

void FlightManagementGuidanceSystem::profileVerticalNavigation()
{
	cstr_dist.clear();
	cstr_alt_mode.clear();

	
	double d_alt;
	double dist;
	double cstr_angle;
	
	double	x1 = 0.;
	double y1 = 0.;
	double x2 = 0.;
	double y2 = 0.;
	
	int		cstr_alt_count;

	
/*
 * Retrieve the flight plan copy
 */
	cstr_dist		= fpl_leg_dist_inv;

	
/*
 * Check if vector contains a speed constraint != 0
 */
	cstr_alt_count = (int) count(cstr_alt.begin(), cstr_alt.end(), 0.);
	
	if (cstr_alt_count == static_cast<int>(cstr_alt.size()) - 2)
	{
		cstr_alt_mode.assign(cstr_alt.size(), "MANAGED");
		return;
	}
	
/*
 * Analyse altitude profile
 * Get departure altitude as first reference
 */
	double alt_ref 			= cstr_alt.back();
	double dist_ref			= cstr_dist.back();

	int max = static_cast<int>(cstr_alt.size());
	
	for (int i = max - 2; i >= 0; i--)
	{
		if (fpl_leg_dist_inv[i] < envelope_des_dist)
		{
			if (cstr_alt[i] > alt_ref)
			{
				alt_ref 			= cstr_alt[i];
				dist_ref			= cstr_dist[i];
			}
			else
			{
				cstr_alt[i] 		= alt_ref;
				cstr_dist[i]		= dist_ref;
			}
		}
		else if (fpl_leg_dist[i] < envelope_clb_dist)
		{
			if (cstr_alt[i] != 0.)
			{
				alt_ref 			= cstr_alt[i];
				dist_ref			= cstr_dist[i];
			}
			else
			{
				cstr_alt[i] 		= alt_ref;
				cstr_dist[i]		= dist_ref;
			}
		}
		else
		{
			alt_ref 	= crz_alt;
			cstr_alt[i]	= crz_alt;
			dist_ref	= fpl_leg_dist_inv[i];
			cstr_dist[i]= dist_ref;
		}
		
		if (LOG == 1)
		{
			cout << i << " <##"<< endl;
			cout << fpl_name[i] << " ALT-> "<< cstr_alt[i] << " CSTR-Dist-> "<< cstr_dist[i]<<" LEG-Dist-> "<< fpl_leg_dist_inv[i]<< endl;

		}

	}
	
	

/*
 * Constraint profile analysis
 * position 0 init is for take off as managed.
 */
	cstr_alt_mode.push_back("SELECTED");
	
	for (int i = 1; i <= max; i++)
	{
		if (LOG == 1)
		{
			cout << i << "-> "<< i << " " << cstr_alt[i] << " cstr_dist " << cstr_dist[i]<< i-1<< " "<< cstr_alt[i-1] << " cstr_dist " << cstr_dist[i - 1] << endl;
			cout << i-1 << " " << fpl_alt[i-1] << endl;
			cout << i-1 << " " << cstr_alt_mode[i-1] << endl;
		}
		
		if (fpl_leg_dist[i] < envelope_clb_dist)
		{
			// Climb leg
			if ((cstr_alt[i] > cstr_alt[i - 1]) && (cstr_alt[i] != crz_alt))
			{
				d_alt		= (cstr_alt[i] - cstr_alt[i - 1]) * FT_TO_METER;
				dist		= abs (cstr_dist[i] - cstr_dist[i - 1]) * M_TO_NM;
				cstr_angle	= (atan (d_alt / dist));
				
				if (cstr_angle > envelope_clb_ang)
				{
					cstr_alt_mode.push_back("MANAGED");

				}
				else
				{
					cstr_alt_mode.push_back("SELECTED");
					x1 = cstr_dist[i - 1];
					y1 = cstr_alt[i - 1];
					x2 = cstr_dist[i];
					y2 = cstr_alt[i]; 
					fpl_alt[i] = LINEAR_EQUATION(fpl_leg_dist_inv[i], x1, y1, x2, y2);
				}
				
			}
			else if (fpl_leg_dist_inv[i] >= x2)
			{
				fpl_alt[i] = LINEAR_EQUATION(fpl_leg_dist_inv[i], x1, y1, x2, y2);
				cstr_alt_mode.push_back("SELECTED");
			}
			else
			{
				cstr_alt_mode.push_back("MANAGED");
			}
		}
		else if (fpl_leg_dist_inv[i] < envelope_des_dist)
		{
			// Climb Leg
			
			if (cstr_alt[i] != cstr_alt[i - 1])
			{
				d_alt		= (cstr_alt[i - 1] - cstr_alt[i]) * FT_TO_METER;
				dist		= abs (cstr_dist[i] - cstr_dist[i - 1]) * M_TO_NM;
				cstr_angle	= (atan (d_alt / dist));
				
				if (cstr_angle > envelope_des_ang)
				{
					cstr_alt_mode.push_back("MANAGED");

				}
				else
				{
					cstr_alt_mode.push_back("SELECTED");
					x1 = cstr_dist[i - 1];
					y1 = cstr_alt[i - 1];
					x2 = cstr_dist[i];
					y2 = cstr_alt[i]; 
					fpl_alt[i] = LINEAR_EQUATION(fpl_leg_dist_inv[i], x1, y1, x2, y2);
				}
				
			}
			else if (fpl_leg_dist_inv[i] >= x2)
			{
				fpl_alt[i] = LINEAR_EQUATION(fpl_leg_dist_inv[i], x1, y1, x2, y2);
				cstr_alt_mode.push_back("SELECTED");
			}
			else
			{
				cstr_alt_mode.push_back("MANAGED");
			}
		}
		else
		{
			// Cruise leg
			
			fpl_alt[i] 		= crz_alt;
			alt_ref 		= crz_alt;
			cstr_alt_mode.push_back("MANAGED");
		}

	}

}

////////////////////////////////////////////////////////////////////////////

void FlightManagementGuidanceSystem::speedProfileVerticalNavigation()
{
	cstr_dist.clear();
	cstr_spd_mode.clear();
	
	double	x1 = 0.;
	double y1 = 0.;
	double x2 = 0.;
	double y2 = 0.;
	int		cstr_spd_count;

/*
 * Retrieve the flight plan copy
 */
	cstr_dist		= fpl_leg_dist_inv;
	
	p_FMS_FMGC->FMGC_MACH_SPEED(28000., -56., mach);
	
	double mach_ref= p_FMS_FMGC->get_AC_COND("KIAS");

/*
 * Check if vector contains a speed constraint != 0
 */
	cstr_spd_count = (int) count(cstr_spd.begin(), cstr_spd.end(), 0.);
	
	if (cstr_spd_count == static_cast<int>(cstr_spd.size()))
	{
		cstr_spd_mode.assign(cstr_spd.size(), "MANAGED");
		return;
	}
	
/*
 * Analyse altitude profile
 * Get departure altitude as first reference
 */
	double spd_ref 			= cstr_spd.back();
	double dist_ref			= cstr_dist.back();

	int max = static_cast<int>(cstr_spd.size());
	
	for (int i = max - 2; i >= 0; i--)
	{
		if (fpl_leg_dist_inv[i] < envelope_des_dist)
		{
			if (cstr_spd[i] > spd_ref)
			{
				spd_ref 			= cstr_spd[i];
				dist_ref			= cstr_dist[i];
			}
			else
			{
				if ( fpl_alt[i] > 28000.)
				{
					spd_ref = mach_ref;
				}
				cstr_spd[i] 		= spd_ref;
				cstr_dist[i]		= dist_ref;
			}
		}
		else if (fpl_leg_dist[i] < envelope_clb_dist)
		{
			if (cstr_spd[i] != 0.)
			{
				spd_ref 			= cstr_spd[i];
				dist_ref			= cstr_dist[i];
			}
			else
			{
				if ( fpl_alt[i] > 28000.)
				{
					spd_ref = mach_ref;
				}
				else if(i == 0)
				{
					cstr_spd[i] 	= 0.;
					cstr_dist[i]	= fpl_leg_dist_inv.front();
				}
				else
				{
					cstr_spd[i] 		= spd_ref;
					cstr_dist[i]		= dist_ref;
				}

			}
		}
		else
		{
			if ( fpl_alt[i] > 28000.)
			{
				spd_ref = mach_ref;
			}
			spd_ref 	= spd_ref;
			cstr_spd[i]	= spd_ref;
			dist_ref	= fpl_leg_dist_inv[i];
			cstr_dist[i]= dist_ref;
		}
		
		if (LOG == 1)
		{
			cout << i << " <##"<< endl;
			cout << fpl_name[i] << " IAS-> "<< cstr_spd[i] << " CSTR-Dist-> "<< cstr_dist[i]<<" LEG-Dist-> "<< fpl_leg_dist_inv[i]<< endl;

		}

	}
	
	
	
/*
 * Constraint profile analysis
 */

	
	for (int i = 1; i < max; i++)
	{
		
		if (fpl_leg_dist[i] < envelope_clb_dist)
		{
			// Climb leg
			if (cstr_spd[i] != cstr_spd[i - 1])
			{
				x1 = cstr_dist[i - 1];
				y1 = p_FMS_FMGC->get_AC_DEF("stall_spd_clean") * 1.23;
				x2 = cstr_dist[i];
				
				if (cstr_spd[i] < y1)
				{
					y2 = y1 + 10.;
				}
				else
				{
					y2 = cstr_spd[i];
				}
					
				fpl_ias[i] = LINEAR_EQUATION(fpl_leg_dist_inv[i], x1, y1, x2, y2);
				cstr_spd_mode.push_back("SELECTED");
			}
			else if (fpl_leg_dist_inv[i] >= x2)
			{
				cstr_spd_mode.push_back("SELECTED");
				fpl_ias[i] = LINEAR_EQUATION(fpl_leg_dist_inv[i], x1, y1, x2, y2);
			}
			else
			{
				cstr_spd_mode.push_back("MANAGED");
			}

		}
		else if (fpl_leg_dist_inv[i] < envelope_des_dist)
		{
			// Descent  Leg
			
			if (cstr_spd[i] != cstr_spd[i - 1])
			{
				x1 = cstr_dist[i - 1];
				y1 = cstr_spd[i - 1];
				x2 = cstr_dist[i];
				
				if (cstr_spd[i] == 0)
				{
					y2 = p_FMS_FMGC->get_AC_DEF("stall_spd_clean") * 1.23;
				}
				else
				{
					if ((y1 < cstr_spd[i]) && (y1 > p_FMS_FMGC->get_AC_DEF("stall_spd_clean") * 1.23))
					{
						y2 = p_FMS_FMGC->get_AC_DEF("stall_spd_clean") * 1.23;
					}
					else if((y1 < cstr_spd[i]) && (y1 < p_FMS_FMGC->get_AC_DEF("stall_spd_clean") * 1.23))
					{
						y2 = p_FMS_FMGC->get_AC_DEF("stall_spd_f3") * 1.18;
					}
					else
					{
						y2 = cstr_spd[i];
					}
				}
				
				fpl_ias[i] = LINEAR_EQUATION(fpl_leg_dist_inv[i], x1, y1, x2, y2);
				cstr_spd_mode.push_back("SELECTED");
			}
			else if (fpl_leg_dist_inv[i] >= x2)
			{
				cstr_spd_mode.push_back("SELECTED");
				fpl_ias[i] = LINEAR_EQUATION(fpl_leg_dist_inv[i], x1, y1, x2, y2);
			}
			else
			{
				cstr_spd_mode.push_back("MANAGED");
			}
		}
		else
		{
			// Cruise leg
			cstr_spd_mode.push_back("MANAGED");
			fpl_ias[i] 		= mach;
			spd_ref 		= mach;
		}
		
		if (LOG == 1)
		{
			cout << i << "-> "<< i << " " << cstr_spd[i] << " cstr_dist " << cstr_dist[i]<< i-1<< " "<< cstr_spd[i-1] << " cstr_dist " << cstr_dist[i - 1] << endl;
			cout << i-1 << "-> "<< fpl_ias[i-1] << " MODE -> " << cstr_spd_mode[i-1]<< endl;
			cout << i << "-> "<< fpl_ias[i] << " MODE -> " << cstr_spd_mode[i]<< endl;
		}
	}
}


////////////////////////////////////////////////////////////////////////////

void FlightManagementGuidanceSystem::envelopeVerticalNavigation()
{
	double OAT 			= 15.;
	double apt_alt 		= fpl_alt.front();
	double crz_alt		= p_FMS_FMGC->get_AC_DEF("max_altitude");
	double GW			= prel_gw;
	double dest_alt 	= fpl_alt.back();
	double LW			= p_FMS_FMGC->get_AC_DEF("MLW");
/*
 * clb angle operation envelope
 */	
	p_FMS_FMGC->FMGC_CLIMB_PRED(OAT, apt_alt, crz_alt, GW);
	
	double vs_m = p_FMS_FMGC->get_AC_COND("VS_HIGH") * FT_TO_METER / 60.;
	
	envelope_clb_ang 		= asin (vs_m / p_FMS_FMGC->get_AC_COND("TAS_CLB_HIGH_M"));
	envelope_clb_dist 		= p_FMS_FMGC->get_AC_COND("CLB_DIST_LAT");
	
/*
 * des angle operation envelope
 */	
	p_FMS_FMGC->FMGC_DESCEND_PRED(300., OAT, crz_alt, dest_alt, LW);
	
	envelope_des_ang 		= asin(abs(p_FMS_FMGC->get_AC_COND("DESC_VS_MS")) / p_FMS_FMGC->get_AC_COND("DESC_SPEED_M"));
	envelope_des_dist 		= p_FMS_FMGC->get_AC_COND("DESC_DIST_LAT");
}

////////////////////////////////////////////////////////////////////////////

double FlightManagementGuidanceSystem::heightPredictionVerticalNavigation(double spd_clb, double w_clb, double leg_dist)
{
	double clb_ang = asin(w_clb / spd_clb);
	
	return tan(clb_ang) * leg_dist * M_TO_NM / FT_TO_METER;
}

////////////////////////////////////////////////////////////////////////////

double FlightManagementGuidanceSystem::heightDistancePredictionVerticalNavigation(double spd_clb, double w_clb, double height)
{
	double clb_ang = sin(w_clb / spd_clb);
	
	return height * FT_TO_METER / (tan(clb_ang) * M_TO_NM);
}

////////////////////////////////////////////////////////////////////////////

void FlightManagementGuidanceSystem::speedSegmentsVerticalNavigation()
{
	double l_220 = fpl_alt.back() + 6000.;
	if (l_220 < crz_alt)
	{
		set_MAPVALUES(p_vnav_speed_seg,	"alt_220kts", 	l_220);
	}
	else
	{
		set_MAPVALUES(p_vnav_speed_seg,	"alt_220kts", 	crz_alt);
	}
	
	
	double l_250 = fpl_alt.back() + 10000.;
	if (l_250 < crz_alt)
	{
		set_MAPVALUES(p_vnav_speed_seg,	"alt_250kts", 	l_250);
	}
	else
	{
		set_MAPVALUES(p_vnav_speed_seg,	"alt_250kts", 	crz_alt);
	}
	
	set_MAPVALUES(p_vnav_speed_seg,	"alt_300kts", 	25000.);
}

////////////////////////////////////////////////////////////////////////////

void FlightManagementGuidanceSystem::managedVerticalNavigation()
{
	int max = static_cast<int> (fpl_name.size());

	for (int i = 0; i < max - 1; i++)
	{
		if (i == 0 && fpl_mode[i] == 0)
		{
			managedVerticalNavigationTO();
			managedVerticalNavigationCLB(i);
		}
		else if (fpl_mode[i] == 1)
		{
			managedVerticalNavigationCLB(i);
		}
		else if (fpl_mode[i] == 2)
		{
			managedVerticalNavigationCRZ(i);
		}
		else if (fpl_mode[i] == 3)
		{
			managedVerticalNavigationDES(i);
		}
		else if (fpl_mode[i] == 4)
		{
			managedVerticalNavigationAPP(i);
		}
		else
		{
			break;
		}		
	} // end of for loop
	   						
}

////////////////////////////////////////////////////////////////////////////

void FlightManagementGuidanceSystem::managedVerticalNavigationTO()
{
	int 	i;
	string 	mode;
	string 	flap;
	double 	gw;
	double 	sel_speed;
	double 	act_speed;
	double 	eng_n1;
	double 	accl_alt;
	double 	thrust_red_height;
	double 	alt;
	double 	target_alt;
	double 	sat;
	double 	fuel_flow;
	double 	lat_dist;
	double 	prel_fuel;

	
	string 	F_Conf;
	string 	AP_flag;
	
/*
 * Profile Segment 0
 *
 * Calculate TAXI fuel consumption
 */
	p_FMS_FMGC->FMGC_ENGINE_GND_TAXI_COND(fpl_alt.front(), OAT);
	
	prel_fuel = p_FMS_FMGC->get_ENGINE_COND("TAXI_FF");
	
	set_MAPVALUES(p_fob, "TAXI",  prel_fuel);
/*
 * Flight Plan init position
 */
	i = 0;
	
	
/*
 * Calculate Take off conditions
 */
	p_FMS_FMGC->FMGC_ENGINE_TO_COND(fpl_alt.front(), Flx_temp, OAT);
	p_FMS_FMGC->FMGC_TO_SPEED(OAT, fpl_alt.front(), F_conf, prel_gw);

	fpl_ias[i]				= p_FMS_FMGC->get_SPEED("SRS");
	fpl_vs[i]				= 0.;
	fpl_eng_n1[i]			= p_FMS_FMGC->get_ENGINE_COND("FLEX_N1");
	fpl_thrust_cond[i]		= "TAKE OFF";
	fpl_estGW[i]			= prel_gw;
	fpl_eta[i]				= 0.;
	total_flight_time		= 0.;
	fpl_efob[i]				= est_fob;
	total_efob				= fpl_efob[i];
/*
 * A/C Mode
 */	
	mode					= "MANAGED";
	gw						= fpl_estGW[i];
	lat_dist				= leg_dist[i];
	eng_n1 					= fpl_eng_n1[i];				
	accl_alt				= Accl_alt;
	thrust_red_height		= Thrust_alt;
	alt						= fpl_alt[i];		// Take off + initial climb within 2 nm rwy distance
	target_alt				= abs((fpl_alt[i] - fpl_alt[i + 1]) / 2.) + fpl_alt[i];
	
	sel_speed				= p_FMS_FMGC->get_SPEED("SRS");
	act_speed				= sel_speed;
	
	if ((sel_speed < p_FMS_FMGC->get_SPEED("F")) && (fpl_alt[i] < Thrust_alt))
	{
		flap			= "F2";
	}
	else
	{
		flap			= "F1";
	}
			
	p_FMS_FMGC->FMGC_CLIMB_CONTROL(	  mode				, flap			
									, gw				, lat_dist
									, sel_speed			, act_speed
									, cost_index			, accl_alt
			   						, thrust_red_height	, alt
			   						, target_alt		, sat);
	
/*
 * First Leg -> Take off and initial climb
 */

	speedLateralNavigation(leg_phi.front(), p_FMS_FMGC->get_AC_COND("TAS_CLB"), leg_windhdg.front(), leg_windspd.front());
	
	leg_tas[i]				= p_FMS_FMGC->get_AC_COND("TAS_CLB");
	leg_gs[i]				= true_leg_spd;
	leg_hdg[i]				= aoa_true;
	leg_ete[i]				= leg_dist[i] / leg_gs[i];
	leg_ops_mode[i] 		= "TAKE_OFF";
	
	if (LOG == 1)
	{
		cout << i << " <################################################################################################################"<< endl;
		cout << " <###################################################################################################################"<< endl;
		cout << fpl_name[0] << " ALT-> "<< fpl_alt[0] << " Tot-Dist-> "<< fpl_leg_dist[0]<< endl;
		cout << "KIAS-> "<< fpl_ias[0] << " N1-> " << fpl_eng_n1[0] << " Thrust-> " << fpl_thrust_cond[0] << endl;
		cout << "GW-> "<< fpl_estGW[0] << " ETA-> " << fpl_eta[0] << " EFOB-> " << fpl_efob[0] << endl;

	}

	fpl_mode[i]					= 0;
	fpl_leg_dist[i+1]			-= p_FMS_FMGC->get_AC_COND("CLB_DIST_LAT");
	
	sat = p_FMS_FMGC->SAT(OAT, fpl_alt[i]);
	fpl_ias[i] 					= p_FMS_FMGC->get_SPEED("SRS");
	fpl_vs[i]					= p_FMS_FMGC->get_AC_COND("VS_CLB");
	
	fpl_thrust_cond[i]			= "TAKE OFF";
	fpl_eng_n1[i]				= p_FMS_FMGC->get_ENGINE_COND("N1_CONTROL");
	
	p_FMS_FMGC->FMGC_ENGINE_THRUST_CONTROL("FLEX_N1", alt, sat);
	p_FMS_FMGC->FMGC_ENGINE_FF("CLB_THRUST_SFC",  p_FMS_FMGC->get_ENGINE_COND("ENGINE_THRUST"));
	fuel_flow 					= p_FMS_FMGC->get_ENGINE_COND("CLB_THRUST_SFC") * p_FMS_FMGC->get_AC_COND("CLB_TIME") / 60.;
	
	fpl_estGW[i]				= fpl_estGW[i] - prel_fuel - fuel_flow;
	
	leg_fuel[i]					= fuel_flow + prel_fuel;
	total_efob 					-= leg_fuel[i];
	fpl_efob[i]					= total_efob;
	
	total_flight_time			+= leg_ete[i];
	fpl_eta[i]					= total_flight_time;
	
	if(LOG == 1)
	{
		cout << i << " <-----------------------------------------------------------------------------------------------------------------"<< endl;
		cout << fpl_name[i] << " ALT-> "<< fpl_alt[i] << " Tot-Dist-> "<< fpl_leg_dist[i+1]<< endl;
		cout << "KIAS-> "<< fpl_ias[i] << " V/S-> "<< fpl_vs[i] << " N1-> " << fpl_eng_n1[i] << " Thrust-> " << fpl_thrust_cond[i] << endl;
		cout << "GW-> "<< fpl_estGW[i] << " ETA-> " << fpl_eta[i] << " EFOB-> " << fpl_efob[i] << endl;
		cout << "TAS-> "<< leg_tas[i] << " GS-> " << leg_gs[i] << " HDG-> " << leg_hdg[i] << " Dist-> " << leg_dist[i] << endl;
		cout << "Leg Fuel-> "<< leg_fuel[i] << " ETE-> " << leg_ete[i] << " Leg ops-> " << leg_ops_mode[i] << endl;

	}
	
}

////////////////////////////////////////////////////////////////////////////
		
void FlightManagementGuidanceSystem::managedVerticalNavigationCLB(int i)
{
	string 	mode;
	string 	flap;
	double 	gw 					= 0.;
	double 	act_speed 			= 0.;
	double	gd_speed 			= 0.;
	double	speed 				= 0.;
	double	initial_clb_speed 	= 0.;
	double 	accl_alt 			= 0.;
	double 	thrust_red_height 	= 0.;
	double 	alt 				= 0.;
	double 	target_alt 			= 0.;
	double 	sat 				= 0.;
	double 	fuel_flow 			= 0.;
	double 	lat_dist 			= 0.;
	double	dist 				= 0.;
	double	time 				= 0.;
	
	if (clb_flag == false)
	{
		p_FMS_FMGC->FMGC_GD_SPEED_POLARE(Accl_alt, fpl_estGW[i]);
		gd_speed				= p_FMS_FMGC->get_SPEED("GD");	
		initial_clb_speed		= p_FMS_FMGC->get_SPEED("SRS");
		
		clb_flag				= true;
	}
	
/*
 * ALT Mode constraints
 */
	
	if ((cstr_alt_mode[i] == "SELECTED") && (i == 0))
	{
		mode						= "SELECTED";
		target_alt					= abs((fpl_alt[i] - fpl_alt[i + 1]) / 2.) + fpl_alt[i];
	}
	else if (cstr_alt_mode[i] == "SELECTED")
	{
		mode						= cstr_alt_mode[i + 1];
		target_alt					= fpl_alt[i + 1];
	}
	else
	{
		mode 						= "MANAGED";
	}

/*
 * Initial Calculation to get active flight plan
 */
	if (fpl_alt[i] < Accl_alt)
	{
		if ((cstr_spd_mode[i] == "MANAGED") && (mode == "SELECTED"))
		{			
			speed 	= LINEAR_EQUATION(fpl_alt[i], fpl_alt.front(), initial_clb_speed, Accl_alt, gd_speed);
			
			if (speed < initial_clb_speed)
			{
				speed = initial_clb_speed;
			}
		}
		else if ((cstr_spd_mode[i] == "MANAGED") && (mode == "MANAGED"))
		{
			act_speed 	= 250.; // set to 300 kts to get best climb velocity!
			speed		= act_speed;
		}
		else
		{
			speed 	= cstr_spd[i];
		}
		
		if (speed < p_FMS_FMGC->get_SPEED("F"))
		{
			flap			= "F2";
		}
								
		else if (speed < p_FMS_FMGC->get_SPEED("S"))
		{
			flap			= "S";
		}
								
		else
		{
			flap			= "CLEAN";
		}
	}
	else if(fpl_alt[i] > Accl_alt && fpl_alt[i] < 25000.)
	{
		if ((cstr_spd_mode[i] == "MANAGED") && (mode == "SELECTED"))
		{
			
			act_speed		= 300.;			
			speed 			= LINEAR_EQUATION(fpl_alt[i], Thrust_alt, initial_clb_speed, 27000., act_speed);

		}
		else if ((cstr_spd_mode[i] == "MANAGED") && (mode == "MANAGED"))
		{
			act_speed 	= 300.; // set to 300 kts to get best climb velocity!
			speed		= act_speed;
		}
		else
		{
			speed 	= cstr_spd[i];
		}
		
		
		sat = p_FMS_FMGC -> SAT(OAT, Accl_alt);
		
		if (speed < p_FMS_FMGC->get_SPEED("S"))
		{
			flap			= "S";
		}
				
		else
		{
			flap			= "CLEAN";
		}
				
	}
	else if (fpl_alt[i] >= 25000.)
	{
		if ((cstr_spd_mode[i] == "MANAGED") && (mode == "SELECTED"))
		{
			sat 					= p_FMS_FMGC->SAT(OAT, fpl_alt[i]);
			p_FMS_FMGC->FMGC_MACH_SPEED(fpl_alt[i], sat, 0.78);
			
			speed 					= p_FMS_FMGC->get_AC_COND("TAS");
			flap					= "CLEAN";
		}
		else if ((cstr_spd_mode[i] == "MANAGED") && (mode == "MANAGED"))
		{
			act_speed 	= 300.; // set to 300 kts to get best climb velocity!
			speed		= act_speed;
		}

	}
	

/*
 * A/C Mode
 */	
	alt							= fpl_alt[i];	
	gw							= fpl_estGW[i];
	
	if (i  == 0)
	{
		lat_dist				= leg_dist[i]- p_FMS_FMGC->get_AC_COND("CLB_DIST_LAT");
	}
	else
	{
		lat_dist				= leg_dist[i - 1];
	}
				
	accl_alt					= Accl_alt;
	thrust_red_height			= Thrust_alt;
	sat 						= p_FMS_FMGC->SAT(OAT, fpl_alt[i]);
	
	p_FMS_FMGC->FMGC_CLIMB_CONTROL(	  mode				, flap			
									, gw				, lat_dist
									, speed				, act_speed
									, cost_index			, accl_alt
			   						, thrust_red_height	, alt
			   						, target_alt		, sat);
	

	// leg properties
	
	speedLateralNavigation(leg_phi[i], p_FMS_FMGC->get_AC_COND("TAS_CLB"), leg_windhdg[i], leg_windspd[i]);
	leg_tas[i]		= p_FMS_FMGC->get_AC_COND("TAS_CLB");
	leg_gs[i]		= true_leg_spd;
	leg_hdg[i]		= aoa_true;
	leg_ete[i]		= leg_dist[i] / leg_gs[i];
	leg_ops_mode[i] = "CLIMB";

	
	// fix properties
	if (mode == "MANAGED")
	{
		target_alt = fpl_alt[i] + heightPredictionVerticalNavigation(p_FMS_FMGC->get_AC_COND("TAS_CLB_M"), p_FMS_FMGC->get_AC_COND("VS_CLB_M"), leg_dist[i]);
	}
	else
	{
		target_alt = fpl_alt[i + 1];
	}
	
	
	if (target_alt <= crz_alt)
	{
		fpl_alt[i + 1] 			= target_alt;
	}
	else
	{
		fpl_alt[i + 1] 			= crz_alt;
	}
	
	fpl_mode[i + 1]			= 1;	
	
	if (target_alt < crz_alt)
	{
		sat 					= p_FMS_FMGC->SAT(OAT, fpl_alt[i + 1]);
		speed 					= p_FMS_FMGC->get_AC_COND("TAS_CLB_M");
		p_FMS_FMGC->FMGC_IAS(fpl_alt[i + 1], sat, speed);
		
		fpl_ias[i + 1] 			= p_FMS_FMGC->get_AC_COND("KIAS");
		
		if(LOG == 1)
		{
			cout << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" << endl;
			cout << "KIAS " << i+1 << " = " << fpl_ias[i + 1] << " KIAS " << i << " = " << fpl_ias[i] << endl;	
		}
		
		if (fpl_ias[i + 1] < fpl_ias[i])
			fpl_ias[i + 1] = fpl_ias[i];
		
		fpl_vs[i + 1]			= p_FMS_FMGC->get_AC_COND("VS_CLB");
		
		fpl_thrust_cond[i + 1]	= "CLIMB";
		fpl_eng_n1[i + 1]		= p_FMS_FMGC->get_ENGINE_COND("N1_CONTROL");
		
		p_FMS_FMGC->FMGC_ENGINE_THRUST_CONTROL("N1_CONTROL", alt, sat);
		p_FMS_FMGC->FMGC_ENGINE_FF("CLB_THRUST_SFC",  p_FMS_FMGC->get_ENGINE_COND("ENGINE_THRUST"));
		fuel_flow = p_FMS_FMGC->get_ENGINE_COND("CLB_THRUST_SFC") * leg_ete[i];
	}
	else 
	{
		// climb part on leg
		
		dist = heightDistancePredictionVerticalNavigation(p_FMS_FMGC->get_AC_COND("TAS_CLB_M"), p_FMS_FMGC->get_AC_COND("VS_CLB_M"), crz_alt - fpl_alt[i]);
		time = dist / p_FMS_FMGC->get_AC_COND("TAS_CLB_M");
		
		fpl_mode[i + 1] 		= 2;
		fpl_alt[i + 1] 			= crz_alt;
		
		sat 					= p_FMS_FMGC->SAT(OAT, fpl_alt[i + 1]);		
		speed 					= p_FMS_FMGC->get_AC_COND("TAS_CLB_M");		
		p_FMS_FMGC->FMGC_IAS(fpl_alt[i + 1], sat, speed);
		
		fpl_ias[i + 1] 			= p_FMS_FMGC->get_AC_COND("KIAS");
		
		if(LOG == 1)
		{
			cout << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" << endl;
			cout << "KIAS " << i+1 << " = " << fpl_ias[i + 1] << " KIAS " << i << " = " << fpl_ias[i] << endl;	
		}

		if (fpl_ias[i + 1] < fpl_ias[i])
			fpl_ias[i + 1] = fpl_ias[i];
		
		fpl_vs[i + 1]			= 0.;
		
		fpl_thrust_cond[i + 1]	= "CLIMB";
		fpl_eng_n1[i + 1]		= p_FMS_FMGC->get_ENGINE_COND("CONTROL");
		
		p_FMS_FMGC->FMGC_ENGINE_THRUST_CONTROL("N1_CONTROL", alt, sat);
		p_FMS_FMGC->FMGC_ENGINE_FF("CLB_THRUST_SFC",  p_FMS_FMGC->get_ENGINE_COND("ENGINE_THRUST"));
		fuel_flow 				= p_FMS_FMGC->get_ENGINE_COND("CLB_THRUST_SFC") * leg_ete[i];
		
		// crz part on last climb leg, TOC is between the current fix on calculation
		
		dist 					= leg_dist[i] - dist;
		
		sat 					= p_FMS_FMGC->SAT(OAT, fpl_alt[i + 1]);
		p_FMS_FMGC->FMGC_MACH_SPEED(fpl_alt[i + 1], sat, mach);
		
		fpl_ias[i + 1]			= p_FMS_FMGC->get_AC_COND("KIAS");
		
		time 					= dist / p_FMS_FMGC->get_AC_COND("TAS");
		
		fpl_thrust_cond[i + 1]	= "CRZ";
		
		p_FMS_FMGC->FMGC_ENGINE_FF("CRZ", p_FMS_FMGC->get_AC_COND("TAS_M"), alt, sat, fpl_estGW[i]);
		fpl_eng_n1[i + 1]		= p_FMS_FMGC->get_ENGINE_COND("N1_CONTROL");
		
		fuel_flow 				+= p_FMS_FMGC->get_ENGINE_COND("CRZ") * time;
	}
	
	fpl_estGW[i + 1]	= fpl_estGW[i] - fuel_flow;
	
	leg_fuel[i]			= fuel_flow;
	total_efob 			-= leg_fuel[i];
	fpl_efob[i + 1]		= total_efob; 
	
	total_flight_time	+= leg_ete[i];
	fpl_eta[i + 1]		= total_flight_time;
	
	if(LOG == 1)
	{
		cout << i+1 << " <-----------------------------------------------------------------------------------------------------------------"<< endl;
		cout << fpl_name[i+1] << " ALT-> "<< fpl_alt[i+1] << " Tot-Dist-> "<< fpl_leg_dist[i+1]<< endl;
		cout << "KIAS-> "<< fpl_ias[i+1]  << " V/S-> "<< fpl_vs[i+1] << " N1-> " << fpl_eng_n1[i+1] << " Thrust-> " << fpl_thrust_cond[i+1] << endl;
		cout << "GW-> "<< fpl_estGW[i+1] << " ETA-> " << fpl_eta[i+1] << " EFOB-> " << fpl_efob[i+1] << endl;
		cout << "TAS-> "<< leg_tas[i] << " GS-> " << leg_gs[i] << " HDG-> " << leg_hdg[i] << " Dist-> " << leg_dist[i] << endl;
		cout << "Leg Fuel-> "<< leg_fuel[i] << " ETE-> " << leg_ete[i] << " Leg ops-> " << leg_ops_mode[i] << endl;

	}
}

////////////////////////////////////////////////////////////////////////////

void FlightManagementGuidanceSystem::managedVerticalNavigationCRZ(int i)
{
	double 	sat;
	double 	fuel_flow;
	double	des_dist;
	double mach_H;
	double mach_L;

	
	if (crz_flag == false)
	{
		mach_H					= p_FMS_FMGC->get_SPEED("MMO");
		
		OAT 					= 15;
		sat 					= p_FMS_FMGC->SAT(OAT, fpl_alt[i]);
		p_FMS_FMGC->FMGC_FOIL_POLAR_OPT_PERF(fpl_alt[i], sat, fpl_estGW[i]);
		mach_L					= p_FMS_FMGC->get_SPEED("V_MAX_RANGE");
		
		p_FMS_FMGC->FMGC_IAS(fpl_alt[i], sat, mach_L);
		mach_L					= p_FMS_FMGC->get_AC_COND("MACH");
		mach 					= cost_index / 999. * (mach_H - mach_L) + mach_L;
		
		crz_flag				= true;
	}	
	
	OAT = 15;
	// leg properties
	sat 					= p_FMS_FMGC->SAT(OAT, fpl_alt[i]);
	p_FMS_FMGC->FMGC_MACH_SPEED(fpl_alt[i], sat, mach);
	fpl_ias[i]		 		= p_FMS_FMGC->get_AC_COND("KIAS");
	
	speedLateralNavigation(leg_phi[i], p_FMS_FMGC->get_AC_COND("TAS"), leg_windhdg[i], leg_windspd[i]);
	leg_tas[i]				= p_FMS_FMGC->get_AC_COND("TAS");
	leg_gs[i]				= true_leg_spd;
	leg_hdg[i]				= aoa_true;
	leg_ete[i]				= leg_dist[i] / leg_gs[i];
	leg_ops_mode[i] 		= "CRZ";
	
	// fix properties

	fpl_alt[i + 1] 			= fpl_alt[i];
	fpl_ias[i + 1] 			= fpl_ias[i];
	fpl_vs[i + 1]			= 0.;
	fpl_thrust_cond[i + 1]	= "CRZ";
	
	sat 					= p_FMS_FMGC->SAT(OAT, fpl_alt[i + 1]);
	p_FMS_FMGC->FMGC_FOIL_POLAR(p_FMS_FMGC->get_AC_COND("TAS_M"), fpl_alt[i], sat, fpl_estGW[i]);
	p_FMS_FMGC->FMGC_ENGINE_FF("CRZ", p_FMS_FMGC->get_AC_COND("TAS_M"), fpl_alt[i + 1], sat, fpl_estGW[i]);
	fpl_eng_n1[i + 1]		= p_FMS_FMGC->get_ENGINE_COND("N1_CONTROL");
	
	fuel_flow 				= p_FMS_FMGC->get_ENGINE_COND("CRZ") * leg_ete[i];
	
	fpl_estGW[i + 1]		= fpl_estGW[i] - fuel_flow;
	
	leg_fuel[i]				= fuel_flow;
	total_efob 				-= leg_fuel[i];
	fpl_efob[i + 1]			= total_efob;
	
	total_flight_time		+= leg_ete[i];
	fpl_eta[i + 1]			= total_flight_time;
	
/*
 *  Check of Descent distance with current gross weight
 */
	p_FMS_FMGC->FMGC_DESCEND_PRED(fpl_ias[i + 1], sat, crz_alt, fpl_alt[IAF], fpl_estGW[i + 1]);
	des_dist				= p_FMS_FMGC->get_AC_COND("DESC_DIST_LAT");
	
	if (fpl_leg_dist_inv[i + 1] <= des_dist)
	{
		fpl_mode[i + 1] = 3;
	}
		
	else
	{
		fpl_mode[i + 1] = 2;
	}
		
	
	
	if(LOG == 1)
	{
		cout << i << " <-----------------------------------------------------------------------------------------------------------------"<< endl;
		cout << fpl_name[i+1] << " ALT-> "<< fpl_alt[i+1] << " Tot-Dist-> "<< fpl_leg_dist[i+1]<< endl;
		cout << "KIAS-> "<< fpl_ias[i+1]  << " V/S-> "<< fpl_vs[i+1] << " N1-> " << fpl_eng_n1[i+1] << " Thrust-> " << fpl_thrust_cond[i+1] << endl;
		cout << "GW-> "<< fpl_estGW[i+1] << " ETA-> " << fpl_eta[i+1] << " EFOB-> " << fpl_efob[i+1] << endl;
		cout << "TAS-> "<< leg_tas[i] << " GS-> " << leg_gs[i] << " HDG-> " << leg_hdg[i] << " Dist-> " << leg_dist[i] << endl;
		cout << "Leg Fuel-> "<< leg_fuel[i] << " ETE-> " << leg_ete[i] << " Leg ops-> " << leg_ops_mode[i] << endl;

	}

}

////////////////////////////////////////////////////////////////////////////

void FlightManagementGuidanceSystem::managedVerticalNavigationDES(int i)
{

	double 	sel_speed;
	double 	act_speed;
	double	speed;
	double 	alt;
	double 	target_alt;
	double 	sat;
	double 	fuel_flow;

	if ((cstr_alt_mode[i] == "MANAGED") && (cstr_spd_mode[i] == "MANAGED"))
	{
		target_alt			= fpl_alt[IAF];
		
		sat 				= p_FMS_FMGC->SAT(OAT, fpl_alt[i]);

		if (fpl_alt[i] < target_alt)
		{
			speed			= 220.;		
		}
		else
		{
			sel_speed		= 250.;
			
			if (fpl_alt[i] <= vnav_speed_seg["alt_220kts"])
			{
				speed			= 220.;

			}
			else if ((fpl_alt[i + 1] > vnav_speed_seg["alt_220kts"]) && (fpl_alt[i] < vnav_speed_seg["alt_250kts"]))
			{
				act_speed		= 300.;
				
				speed = LINEAR_EQUATION(fpl_alt[i], vnav_speed_seg["alt_300kts"], act_speed, vnav_speed_seg["alt_250kts"], sel_speed);
			}
			else
			{
				sat 			= p_FMS_FMGC->SAT(OAT, fpl_alt[i]);
				p_FMS_FMGC->FMGC_MACH_SPEED(fpl_alt[i], sat, 0.78);
				
				if (p_FMS_FMGC->get_AC_COND("KIAS") > 300.)
				{
					speed			= 300.;
				}
				else
				{
					speed 			= p_FMS_FMGC->get_AC_COND("KIAS");
				}
							
			}
		
		}
		
		if (fpl_alt[i] > target_alt)
		{
			p_FMS_FMGC->FMGC_DESCEND_PRED(speed, sat, fpl_alt[i], target_alt, fpl_estGW[i]);
		}
	}
	else if ((cstr_alt_mode[i] == "SELECTED") && (cstr_spd_mode[i] == "MANAGED"))
	{
		
		target_alt			= fpl_alt[i + 1];
		
		sat 				= p_FMS_FMGC->SAT(OAT, fpl_alt[i]);
			
		if (fpl_alt[i] <= vnav_speed_seg["alt_220kts"])
		{
			speed			= 220.;

		}
		else if ((fpl_alt[i + 1] > vnav_speed_seg["alt_220kts"]) && (fpl_alt[i] < vnav_speed_seg["alt_250kts"]))
		{
			act_speed		= 300.;
			sel_speed 		= 250.;
			
			speed = LINEAR_EQUATION(fpl_alt[i], vnav_speed_seg["alt_300kts"], act_speed, vnav_speed_seg["alt_250kts"], sel_speed);
		}
		else if ((fpl_alt[i + 1] < vnav_speed_seg["alt_220kts"]) && (fpl_alt[i] < vnav_speed_seg["alt_250kts"]))
		{
			act_speed		= 250.;
			sel_speed		= 220.;
			
			speed = LINEAR_EQUATION(fpl_alt[i], vnav_speed_seg["alt_250kts"], act_speed, vnav_speed_seg["alt_220kts"], sel_speed);
		}
		else
		{
			sat 			= p_FMS_FMGC->SAT(OAT, fpl_alt[i]);
			p_FMS_FMGC->FMGC_MACH_SPEED(fpl_alt[i], sat, 0.78);
			
			if (p_FMS_FMGC->get_AC_COND("KIAS") > 300.)
			{
				speed			= 300.;
			}
			else
			{
				speed 			= p_FMS_FMGC->get_AC_COND("KIAS");
			}
						
		}
		
		if (fpl_alt[i] > target_alt)
		{
			p_FMS_FMGC->FMGC_DESCEND_PRED(speed, sat, fpl_alt[i], target_alt);
		}
	}
	else
	{
		target_alt 			= cstr_alt[i + 1];
		speed				= cstr_spd[i];
		sat 				= p_FMS_FMGC->SAT(OAT, fpl_alt[i]);
		
		if (fpl_alt[i] > target_alt)
		{
			p_FMS_FMGC->FMGC_DESCEND_PRED(speed, sat, fpl_alt[i], target_alt);
		}
	}
		
	// leg properties
	
	speedLateralNavigation(leg_phi[i], p_FMS_FMGC->get_AC_COND("DESC_SPEED"), leg_windhdg[i], leg_windspd[i]);
	leg_tas[i]		= p_FMS_FMGC->get_AC_COND("DESC_SPEED");
	leg_gs[i]		= true_leg_spd;
	leg_hdg[i]		= aoa_true;
	leg_ete[i]		= leg_dist[i] / leg_gs[i];
	leg_ops_mode[i] = "DESCENT";

	// fix properties
	
/*
 * VNAV Constraints
 */	

	if(cstr_alt_mode[i] == "MANAGED")
	{
		target_alt = fpl_alt[i] - heightPredictionVerticalNavigation(p_FMS_FMGC->get_AC_COND("DESC_SPEED_M"), p_FMS_FMGC->get_AC_COND("DESC_VS_MS"), leg_dist[i]);
	}

	fpl_alt[i + 1] 			= target_alt;
	fpl_ias[i + 1] 			= speed;
	fpl_vs[i + 1]			= -1.* p_FMS_FMGC->get_AC_COND("DESC_VS");
	fpl_thrust_cond[i + 1]	= "DESCEND";
	fpl_eng_n1[i + 1]		= p_FMS_FMGC->get_ENGINE_COND("IDLE_N1");
	
	p_FMS_FMGC->FMGC_ENGINE_THRUST_CONTROL("IDLE_N1", alt, sat);
	p_FMS_FMGC->FMGC_ENGINE_FF("DESC_THRUST_SFC",  p_FMS_FMGC->get_ENGINE_COND("ENGINE_THRUST"));
	fuel_flow 				= p_FMS_FMGC->get_ENGINE_COND("DESC_THRUST_SFC") * leg_ete[i];

	
	fpl_estGW[i + 1]		= fpl_estGW[i] - fuel_flow;
	
	leg_fuel[i]				= fuel_flow;
	total_efob 				-= leg_fuel[i];
	fpl_efob[i + 1]			= total_efob;
	
	total_flight_time		+= leg_ete[i];
	fpl_eta[i + 1]			= total_flight_time;
	
	if (fpl_alt[i + 1] <= fpl_alt.back() + 3000.)
		fpl_mode[i + 1] = 4;
	else
		fpl_mode[i + 1] = 3;

	if(LOG == 1)
	{
		cout << i+1 << " <-----------------------------------------------------------------------------------------------------------------"<< endl;
		cout << fpl_name[i+1] << " ALT-> "<< fpl_alt[i+1] << " Tot-Dist-> "<< fpl_leg_dist[i+1]<< endl;
		cout << "KIAS-> "<< fpl_ias[i+1]  << " V/S-> "<< fpl_vs[i+1] << " N1-> " << fpl_eng_n1[i+1] << " Thrust-> " << fpl_thrust_cond[i+1] << endl;
		cout << "GW-> "<< fpl_estGW[i+1] << " ETA-> " << fpl_eta[i+1] << " EFOB-> " << fpl_efob[i+1] << endl;
		cout << "TAS-> "<< leg_tas[i] << " GS-> " << leg_gs[i] << " HDG-> " << leg_hdg[i] << " Dist-> " << leg_dist[i] << endl;
		cout << "Leg Fuel-> "<< leg_fuel[i] << " ETE-> " << leg_ete[i] << " Leg ops-> " << leg_ops_mode[i] << endl;

	}
}

////////////////////////////////////////////////////////////////////////////

void FlightManagementGuidanceSystem::managedVerticalNavigationAPP(int i)
{

	string 	mode;
	string 	flap;

	double 	speed;
	double 	alt;
	double 	sat;
	double 	fuel_flow;


	
	string F_Conf;
	string AP_flag;
	
	sat = p_FMS_FMGC->SAT(OAT, fpl_alt[i]);
	
	if (fpl_alt[i] - fpl_alt.back()  > 6000.)
	{
		F_Conf = "S";
	}
	else if (fpl_alt[i] - fpl_alt.back()  > 4000.)
	{
		F_Conf = "F2";
	}
	else
	{
		F_Conf = "F3";
	}

	
	p_FMS_FMGC->FMGC_APPR_SPEED(sat, fpl_alt[i], F_Conf, fpl_estGW[i], 0.);
	p_FMS_FMGC->FMGC_ENGINE_APPR_COND(sat, fpl_alt[i], F_Conf, fpl_estGW[i]);
	
	if (F_Conf == "S")
	{
		speed 				= getSpeedS(); //kts
		if (speed > p_FMS_FMGC->get_SPEED("GD"))
			speed = p_FMS_FMGC->get_SPEED("GD");
	}
	else 
	{
		speed 				= p_FMS_FMGC->get_SPEED("VAPP"); //kts
	}
	
	speedLateralNavigation(leg_phi[i], speed, leg_windhdg[i], leg_windspd[i]);
	leg_tas[i]				= speed;
	leg_gs[i]				= true_leg_spd;
	leg_hdg[i]				= aoa_true;
	leg_ete[i]				= leg_dist[i] / leg_gs[i];
	leg_ops_mode[i] 		= "APPROACH";
	
	// fix properties
	
	fpl_mode[i + 1]			 = 4;

	fpl_alt[i + 1] 			= LINEAR_EQUATION(fpl_leg_dist_inv[i + 1], fpl_leg_dist_inv[IAF], fpl_alt[IAF], fpl_leg_dist_inv.back(), fpl_alt.back());
	
	fpl_ias[i + 1] 			= speed;
	fpl_vs[i + 1]			= (fpl_alt[i + 1] - fpl_alt[i]) / (leg_ete[i] * 60.);

	fpl_thrust_cond[i + 1]	= "APPROACH";
	fpl_eng_n1[i + 1]		= p_FMS_FMGC->get_ENGINE_COND("N1_CONTROL");
	
	p_FMS_FMGC->FMGC_ENGINE_THRUST_CONTROL("N1_CONTROL", alt, sat);
	p_FMS_FMGC->FMGC_ENGINE_FF("APPROACH_SFC",  p_FMS_FMGC->get_ENGINE_COND("ENGINE_THRUST"));
	fuel_flow 				= p_FMS_FMGC->get_ENGINE_COND("APPROACH_SFC") * leg_ete[i];

	
	fpl_estGW[i + 1]		= fpl_estGW[i] - fuel_flow;
	
	leg_fuel[i]				= fuel_flow;
	total_efob 				-= leg_fuel[i];
	fpl_efob[i + 1]			= total_efob;
	
	total_flight_time		+= leg_ete[i];
	fpl_eta[i + 1]			= total_flight_time;
	
	if(LOG == 1)
	{
		cout << i+1 << " <-----------------------------------------------------------------------------------------------------------------"<< endl;
		cout << fpl_name[i+1] << " ALT-> "<< fpl_alt[i+1] << " Tot-Dist-> "<< fpl_leg_dist[i+1]<< endl;
		cout << "KIAS-> "<< fpl_ias[i+1]  << " V/S-> "<< fpl_vs[i+1] << " N1-> " << fpl_eng_n1[i+1] << " Thrust-> " << fpl_thrust_cond[i+1] << endl;
		cout << "GW-> "<< fpl_estGW[i+1] << " ETA-> " << fpl_eta[i+1] << " EFOB-> " << fpl_efob[i+1] << endl;
		cout << "TAS-> "<< leg_tas[i] << " GS-> " << leg_gs[i] << " HDG-> " << leg_hdg[i] << " Dist-> " << leg_dist[i] << endl;
		cout << "Leg Fuel-> "<< leg_fuel[i] << " ETE-> " << leg_ete[i] << " Leg ops-> " << leg_ops_mode[i] << endl;

	}
}

////////////////////////////////////////////////////////////////////////////


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 
 * Fuel Prediction ( INIT B )
 * 
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */

void FlightManagementGuidanceSystem::estimatedFuelOnBoard()
{
	
	double fpl_dist		= fpl_leg_dist.back() + altn_dist;
/*
 * estimate a first gw 
 */	
	prel_gw		= p_FMS_FMGC->FMGC_ESTIMATED_GW(fpl_dist, ZFW); 
/*
 * Opt FL
 */
	
	opt_alt 	= p_FMS_FMGC->FMGC_OPT_FL(mach, OAT, prel_gw);
/*
 * Calculate TAXI fuel consumption
 */
	p_FMS_FMGC->FMGC_ENGINE_GND_TAXI_COND(fpl_alt.front(), OAT);
	
/*
 * Calculate T/o condition with OAT as Flex_temperature
 */
	p_FMS_FMGC->FMGC_ENGINE_TO_COND(fpl_alt.front(), OAT, OAT);
	
/*
 * Calculate the climb profile fuel consumption
 */
	
	p_FMS_FMGC->FMGC_ENGINE_CLIMB_COND(OAT, fpl_alt.front(), opt_alt, prel_gw);
	
/*
 * Calculate the descend profile fuel consumption
 */
	double sat = p_FMS_FMGC -> SAT(OAT, opt_alt);
	
	p_FMS_FMGC->FMGC_ENGINE_DESC_COND(sat, opt_alt, prel_gw);
	
	
/*
 * Calculate Cruise fuel consumption
 */
	
	double mission_distance = fpl_dist - p_FMS_FMGC->get_AC_COND("CLB_DIST_LAT") - p_FMS_FMGC->get_AC_COND("DESC_DIST_LAT");
	
	double crz_ff 	= p_FMS_FMGC->FMGC_TRIP_FUEL_PREDICTION(mission_distance, mach, opt_alt, sat, prel_gw);
	
	est_fob 		= crz_ff + p_FMS_FMGC->get_ENGINE_COND("TAXI")
	                         + p_FMS_FMGC->get_ENGINE_COND("TO_FF")
	                         + p_FMS_FMGC->get_ENGINE_COND("CLB_FF")
	                         + p_FMS_FMGC->get_ENGINE_COND("DESC_FF");
	                        
	prel_gw = est_fob + ZFW;
	
}

////////////////////////////////////////////////////////////////////////////

void FlightManagementGuidanceSystem::fuelCalculation()
{	

	double mach_no;
	double altn_alt;
	double OAT;
	double altn_fuel;
	double final_fuel;
	double trip_fuel;
	double perc;
	double rsv;
	
	LW = fpl_estGW.back();
	
	
// Alternate Airport Flight in FL220 for dist < 200nm, else FL310
	
	if (altn_dist <= 220.)
	{
		mach_no 		= 0.78;
		altn_alt 		= 22000.;
		OAT 			= 15.; 

		altn_fuel = p_FMS_FMGC->FMGC_TRIP_FUEL_PREDICTION(altn_dist, mach_no, altn_alt, p_FMS_FMGC->SAT(OAT, altn_alt), LW);
	}
	else
	{
		mach_no 		= 0.78;
		altn_alt 		= 31000.;
		OAT 			= 15.; 

		altn_fuel = p_FMS_FMGC->FMGC_TRIP_FUEL_PREDICTION(altn_dist, mach_no, altn_alt, p_FMS_FMGC->SAT(OAT, altn_alt), LW);
	}
	
	set_MAPVALUES(p_fob, "ALTN",  altn_fuel);
	
	p_FMS_FMGC->FMGC_MACH_SPEED(altn_alt, p_FMS_FMGC->SAT(OAT, altn_alt), mach_no);
	
	double ete_altn = altn_dist / p_FMS_FMGC->get_AC_COND("TAS");
	
	set_MAPVALUES(p_fob, "ALTN_TIME",  ete_altn);

	
// Approach and Hold	30 min hold and approach at Alternate airport

	double alt = 1500.;
	
	string cond		="HOLDING";
	
	OAT = 15.;
	
	p_FMS_FMGC->FMGC_GD_SPEED_POLARE(alt, LW);
	
	p_FMS_FMGC->FMGC_ENGINE_FF(cond, p_FMS_FMGC->get_SPEED("GD") * KTS_TO_MS, alt, p_FMS_FMGC->SAT(OAT, alt), LW);
	
	final_fuel = p_FMS_FMGC->get_ENGINE_COND(cond) * 30. / 60.; // 30 minutes hold
	
	set_MAPVALUES(p_fob, "FINAL",  final_fuel);
	
	set_MAPVALUES(p_fob, "FINAL_TIME",  30.);
	
// Extra fuel
	
	it_fob1= fob.find("EXTRA");
	
	if(it_fob1 == fob.end())
	{
		set_MAPVALUES(p_fob, "EXTRA",  0.);
	}
	
	
// Cruise condition	

	int max = leg_fuel.size();
	trip_fuel = 0.;
	
	for (int i = 1; i <= max - 1; i++)
	{
		trip_fuel += leg_fuel[i];
	}
		
// FUEL Condition defintion
	
	set_MAPVALUES(p_fob, "TRIP",  trip_fuel);
	
	set_MAPVALUES(p_fob, "TRIP_TIME", fpl_eta.back());
	
	it_fob1 = fob.find("BLOCK");
	
	if (it_fob1 == fob.end())
	{
		if (trip_fuel == 0.)
			set_MAPVALUES(p_fob, "BLOCK",  ( fob["TAXI"] + fob["ALTN"] + fob["FINAL"] + fob["TRIP"] + fob["EXTRA"]) * 1.05);
		else
			set_MAPVALUES(p_fob, "BLOCK",  ( fob["TAXI"] + fob["ALTN"] + fob["FINAL"] + fob["EXTRA"] + trip_fuel ) * 1.05);
	}
	else
	{
		trip_fuel = fob["BLOCK"] - (fob["TAXI"] + fob["ALTN"] + fob["FINAL"] + fob["EXTRA"] );
		
		set_MAPVALUES(p_fob, "TRIP",  trip_fuel);
	}
		
	it_fob1 = fob.find("RSV");
	it_fob2 = fob.find("RSV_PERC");
	
	if (it_fob1 == fob.end())
	{
		set_MAPVALUES(p_fob, "RSV",  fob["BLOCK"] * 0.05);
		
		set_MAPVALUES(p_fob, "RSV_PERC",  100. * 0.05);
	}
	else
	{
		rsv = fob["RSV"];
		
		perc = rsv / (fob["BLOCK"] + rsv);
		
		set_MAPVALUES(p_fob, "RSV_PERC",  perc);
	}
		
	
	
	if (it_fob2 == fob.end())
	{
		perc = fob["RSV_PERC"];
		
		set_MAPVALUES(p_fob, "RSV",  fob["BLOCK"] * perc / 100.);
	}
	
	set_MAPVALUES(p_fob, "EXTRA",  fob["TAXI"] + fob["ALTN"] + fob["FINAL"]);
}

////////////////////////////////////////////////////////////////////////////

void FlightManagementGuidanceSystem::fuelAnalysis(int& i)
{
	double prel_fuel 		= fob["BLOCK"];
	
	if (prel_fuel > p_FMS_FMGC->get_AC_DEF("FUEL_capacity"))
	{
		FMGS_MESSAGE.insert( pair<string, string> ("FMGC trip_fuel", "Check calculated Fuel Contingency"));
		TOW = ZFW + p_FMS_FMGC->get_AC_DEF("FUEL_capacity");
		set_MAPVALUES(p_fob, "BLOCK",  prel_fuel);
		fuelCalculation();
		LW = p_FMS_FMGC->get_AC_DEF("MTOW") - fob["TRIP"];
	}
	else
	{
		TOW = ZFW + prel_fuel;
	}
	
	if (TOW > p_FMS_FMGC->get_AC_DEF("MTOW"))
	{
		if (i >= 2)
		{
			FMGS_MESSAGE.insert( pair<string, string> ("FMGC trip_fuel", "Check Range / Alternate Airport Distance / calculated Fuel Contigency "));
		}

		prel_fuel = p_FMS_FMGC->get_AC_DEF("MTOW") - ZFW;
		set_MAPVALUES(p_fob, "BLOCK",  prel_fuel);
		fuelCalculation();
		LW = p_FMS_FMGC->get_AC_DEF("MTOW") - fob["TRIP"];
		TOW = p_FMS_FMGC->get_AC_DEF("MTOW");
	}
	
	set_MAPVALUES(p_fob, "FQ_DEP",  prel_fuel);
	set_MAPVALUES(p_fob, "FQ_ARR",  prel_fuel - fob["TRIP"]);

}


////////////////////////////////////////////////////////////////////////////

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 
 * Method & Tools 
 * 
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
void FlightManagementGuidanceSystem::engineDeration(string& DERATED_ENG)
{
	string str1;
	double n1_derate;
	
	if (DERATED_ENG.length() == 0)
	{
		Flx_temp = OAT;
		n1_derate = 0.;
		p_FMS_FMGC->set_DERATED_N1(n1_derate);
	}
	else
	{
		str1 = DERATED_ENG.substr(1, 2);
		if (DERATED_ENG[0] == 'D')
		{
			n1_derate 	= string_to_double(str1);
			
			if ((n1_derate < 0.) || (n1_derate > 24.))
			{
				n1_derate = 0.;
			}
			else
			{
				n1_derate /= 100.;
			}
			
			p_FMS_FMGC->set_DERATED_N1(n1_derate);
			Flx_temp = OAT;
		}
		else if (DERATED_ENG[0] == 'F')
		{
			Flx_temp 	= string_to_double(str1);
		}
		else
		{
			Flx_temp = OAT;
			n1_derate = 0.;
			p_FMS_FMGC->set_DERATED_N1(n1_derate);
		}
	}
	
}

////////////////////////////////////////////////////////////////////////////

void FlightManagementGuidanceSystem::slatsFlapsConfig(const int& slat_flap_notches)
{
	string airbus_hl[] = {"CLEAN", "S", "F1", "F1 + 1", "F2", "F3", "FULL"};
	string boeing_hl[] = {"CLEAN", "S", "F1", "F2", "F3", "F4", "F5", "F6" , "FULL"};
	
/*
 * Investigate used Aircraft
 */
	if (!AC_MODEL.length())
	{
		F_conf		= "F2";
		F_CONF		= 5;
	}
	else
	{
		if (AC_MODEL.find("A3") == 0)
		{
			F_conf = airbus_hl[slat_flap_notches];
		}
		else if (AC_MODEL.find("B7") == 0)
		{
			F_conf = boeing_hl[slat_flap_notches];
		}
		else
		{
			F_conf = "F2";
		}
	}
}

////////////////////////////////////////////////////////////////////////////

double FlightManagementGuidanceSystem::optimumFlightLevel(const FlightStatus& flightstatus)
{
	SmoothedValueWithDelay<double> smoothedIas	= flightstatus.smoothed_ias;
	double IAS 									= smoothedIas.lastValue();
	double SAT									= flightstatus.sat;
	double GW									= static_cast<double>(flightstatus.total_weight_kg);
	
	return p_FMS_FMGC->FMGC_OPT_FL(IAS, SAT, GW);
}

////////////////////////////////////////////////////////////////////////////

double FlightManagementGuidanceSystem::maxFlightLevel(const FlightStatus& flightstatus)
{
	SmoothedValueWithDelay<double> smoothedIas	= flightstatus.smoothed_ias;
	double IAS 									= smoothedIas.lastValue();
	double SAT									= flightstatus.sat;
	double GW									= static_cast<double>(flightstatus.total_weight_kg);
	
	return p_FMS_FMGC->FMGC_MAX_FL(IAS, SAT, GW);
}

////////////////////////////////////////////////////////////////////////////

void FlightManagementGuidanceSystem::speed(const FlightStatus& flightstatus)
{
	string	mode						="";
	
	double slat_flap_notches			= flightstatus.current_flap_lever_notch;
	double SAT							= flightstatus.sat;
	double ALT							= flightstatus.alt_ft;
	double GW							= static_cast<double>(flightstatus.total_weight_kg);
	double HeadWind						= 0.;
	
	SmoothedValueWithDelay<double> smoothedIas	= flightstatus.smoothed_ias;
	double IAS 						= smoothedIas.lastValue();
	
	slatsFlapsConfig(slat_flap_notches);
	
	if (mode == "APP")
	{
		p_FMS_FMGC->FMGC_APPR_SPEED(SAT, ALT, F_conf, GW, HeadWind);
	}
	else if (mode == "TO")
	{
		p_FMS_FMGC->FMGC_TO_SPEED(SAT, ALT, F_conf, GW);
	}
	else
	{
		p_FMS_FMGC->FMGC_CONTROL_SPEED(IAS, SAT, ALT, F_conf, GW);
	}

}

////////////////////////////////////////////////////////////////////////////

void FlightManagementGuidanceSystem::fadec(const string& mode, const double& OAT, const double& ALT, const int& slat_flap_notches, const double& GW)
{
	slatsFlapsConfig(slat_flap_notches);
	
	if (mode == "CRZ")
	{
		p_FMS_FMGC->FMGC_THRUST_MAX_CONT_N1(1.,ALT, OAT);
	}
	else if (mode == "APPR")
	{
		p_FMS_FMGC->FMGC_THRUST_MAX_CONT_N1(1.,ALT, OAT);
	}
	else if (mode == "CLB")
	{
		p_FMS_FMGC->FMGC_THRUST_MAX_CLIMB_N1(1., ALT, OAT);
	}
	else if(mode == "DES")
	{
		p_FMS_FMGC->FMGC_THRUST_MAX_CONT_N1(1.,ALT, OAT);
	}
	else if(mode == "TOGA")
	{
		p_FMS_FMGC->FMGC_FLEX_THRUST(ALT, 55., OAT);
		
		p_FMS_FMGC->FMGC_TO_SPEED(OAT, ALT, F_conf, GW);
		
	}

}

////////////////////////////////////////////////////////////////////////////

void FlightManagementGuidanceSystem::estimatedFuelOnBoard(const FlightStatus& flightstatus, const FlightRoute& active_route)
{
	double ground_speed 	= flightstatus.ground_speed_kts;
	double tas 				= flightstatus.tas;
	double MACH				= flightstatus.mach;
	double CRZ_ALT			= flightstatus.alt_ft;
	double SAT				= flightstatus.sat;
	double GW				= static_cast<double>(flightstatus.total_weight_kg);
	double	CFOB			= flightstatus.total_fuel_capacity_kg;
	
	
	double dist				= active_route.distanceActiveWptToDestination();
	
	double crz_distance 	= dist * tas / ground_speed;
	
	double trip_fuel 		= p_FMS_FMGC->FMGC_TRIP_FUEL_PREDICTION(crz_distance, MACH, CRZ_ALT, SAT,  GW);
	
	set_MAPVALUES(p_fob, "EFOB", CFOB - trip_fuel);
}

////////////////////////////////////////////////////////////////////////////

bool FlightManagementGuidanceSystem::isExtraFuelValid()
{
	it_fob1 = fob.find("EXTRA");
	
	if (it_fob1 == fob.end())
		return false;
	else
		return true;			
}

////////////////////////////////////////////////////////////////////////////

bool FlightManagementGuidanceSystem::isAlternateFuelValid()
{
	it_fob1 = fob.find("ALTN");
	
	if (it_fob1 == fob.end())
		return false;
	else
		return true;			
}

////////////////////////////////////////////////////////////////////////////

bool FlightManagementGuidanceSystem::isFinalFuelValid()
{
	it_fob1 = fob.find("TINAL");
	
	if (it_fob1 == fob.end())
		return false;
	else
		return true;			
}

////////////////////////////////////////////////////////////////////////////

bool FlightManagementGuidanceSystem::isTripFuelValid()
{
	it_fob1 = fob.find("TRIP");
	
	if (it_fob1 == fob.end())
		return false;
	else
		return true;			
}

////////////////////////////////////////////////////////////////////////////

bool FlightManagementGuidanceSystem::isBlockFuelValid()
{
	it_fob1 = fob.find("BLOCK");
	
	if (it_fob1 == fob.end())
		return false;
	else
		return true;			
}

////////////////////////////////////////////////////////////////////////////

bool FlightManagementGuidanceSystem::isReserveFuelValid()
{
	it_fob1 = fob.find("RSV");
	
	if (it_fob1 == fob.end())
		return false;
	else
		return true;			
}

////////////////////////////////////////////////////////////////////////////

void FlightManagementGuidanceSystem::messageCheck()
{
	multimap<string,string> 			fmgc_message;
	
	fmgc_message = p_FMS_FMGC->get_FMGC_MESSAGE();
	
	FMGS_MESSAGE.insert(fmgc_message.begin(), fmgc_message.end());
}

////////////////////////////////////////////////////////////////////////////


}
