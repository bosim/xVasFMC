//
// C++ Header: fsaccess_xplane_refids.h
//
// Description: This enum holds the IDs of flightstatus data to be READ FROM XPLANE,
// so vaslib and xpfmcconn share a common database.
//
//
// Author: Philipp Muenzel <philipp_muenzel@web.de>, (C) 2009
// Organization: Technische Universitaet Darmstadt, Department of Mathematics
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef FSACCESS_XPLANE_REFIDS_H
#define FSACCESS_XPLANE_REFIDS_H

#define PLUGIN_NODE_ID 1
#define VASFMC_NODE_ID 2
#define PLUGIN_USES_ID29 1
// software revision in this compile (build) to make sure vaslib and xpfmcconn are compatible
#define PLUGIN_SOFTWARE_REVISION 86 // revision 1086 on v2, TODO: better format ? We have only 0-255 since it is uint8_t
#define PLUGIN_HARDWARE_REVISION 1  // just a number, might be used for cfg-file control ?


enum FlightPanelsDistribution
{
    // 0 - 127 emergency event data as in CANaerospace standard
    //
    NSH_CH0_REQ = 128, // High priority node service request channel 0 request id (for IDS, STS)
    NSH_CH0_RES = 129, // High priority node service request channel 0 response id
    // 130-199 other NSH channels
    // 200 - 299 UDH
    // 300 - 1799 NOD, at present just straightly numbered, to be read from cfg file

    RSRVD = 300, // note that according to C++ standard, the enum now continues from 301
    THDG, LAT, LON, IAS, VS, PITCH, BANK, TALT, GS, TAS, APALT, APHDG, MAGVAR, WINDSPEED, WINDDIR,
    AVIONICS, BATTERY, ONGROUND, BEACON, STROBE, LDGLT, NAVLT, TAXILT, PITOTHT, APSPD, APSPDMACH, FDON,
    NAV1, NAV2, ADF1, ADF2, OBS1, OBS2, NOENGINES, ENGN1, ENGN2, ENGEGT, ENGFF, GEAR, FDROLL, FDPITCH,
    TOTWT, FUELWT, PAYLOAD, FUELCAP, TAI, ENGREV, PAUSE, PRKBRK, QNH, OAT, DEW, TAT, ALTSET, APMODE, APVS,
    YAGL, INDALT, MACH, V_S0, V_S, V_FE, V_NO, M_MO, SOS, N1FROMTO, N2FROMTO, N1HASDME, N2HASDME, NAVMODE, ADF1BRG, ADF2BRG,
    N1DME, N2DME, ZTIME, ZDATE, EFIS1SELCPT, EFIS2SELCPT, VOR1HDEF, VOR2HDEF, ILS1VDEF,
    APSTATE, ENGTHRO, FLAPS, FLAPRQST, THRAXIS, THROVRD, THROVRDPOS, PITCHINPUT,
    ROLLINPUT, PITCHOVRD, ROLLOVRD, PITCHOVRDPOS, ROLLOVRDPOS, FLAPDET, FLAPDETPOS, SPDBRK,
    NDB1TND, NDB1ID, NDB1LAT, NDB1LON, NDB2TND, NDB2ID, NDB2LAT, NDB2LON,
    VOR1TND, VOR1ID, VOR1LOC, VOR1LAT, VOR1LON, VOR1LOCCRS,
    VOR2TND, VOR2ID, VOR2LOC, VOR2LAT, VOR2LON, VOR2LOCCRS,
    TOTALNUM

    // 1800 - 1899 UDL
    // 1900 - 1999 DSD
    // 2000 - 2031 NSL
};

#endif
