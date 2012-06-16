///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2006 Alexander Wemmer
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

#ifndef NAVCALC_H
#define NAVCALC_H

#include <cmath>

#ifndef M_PI
#define M_PI 3.1415926535897932385
#endif

#include <QString>

class Waypoint;
class Declination;

/////////////////////////////////////////////////////////////////////////////

class Navcalc
{
public:

    enum TURN_DIRECTION { TURN_AUTO = 0,
                          TURN_LEFT = 1,
                          TURN_RIGHT = 2
    };

    Navcalc() {};

    virtual ~Navcalc() {};

    const static double STD_ALTIMETER_HPA;
    const static double STD_ALTIMETER_INCHES;
    const static double EARTH_CIRCUMFERENCE_KM;
    const static double METER_TO_FEET;
    const static double FEET_TO_METER;
    const static double FEET_TO_NM;
    const static double METER_TO_NM;
    const static double METER_PER_SECOND_TO_KNOTS;
    const static double T0ISA;
    const static double POUNDS_TO_KG;
    const static double USGALLONS_TO_LITERS;
    const static double EARTH_ACCELERATION_NM_PER_HOUR;
    const static double JETA1_LITRES_TO_KILOS;
    const static double HPA_TO_INHG;

    static double getMSLTemp(double oat, double altitude_ft);
    static double getAltTemp(double current_oat, double current_alt, double wanted_altitude_ft);

    static double getTropopauseFt(double oat, double altitude_ft);

    static double toRad(double degrees) { return (M_PI * degrees) / 180.0; }
    static double toDeg(double radiant) { return (radiant * 180.0) / M_PI; }

    inline static double getHpaFromInches(const double& inches) { return inches / HPA_TO_INHG; }
    inline static double getInchesFromHpa(const double& hpa) { return hpa * HPA_TO_INHG; }

    inline static double getSpeedOfSoundTas(const double& oat)
    { return (331.3 +(0.6 * oat))* METER_TO_NM * 3600.0; }

    inline static double getTasFromMach(const double& mach, const double& oat)
    { return mach * getSpeedOfSoundTas(oat); }

    inline static double getMachFromTas(const double& tas, const double& oat)
    { return tas / getSpeedOfSoundTas(oat); }

    static double getIasFromMach(const double& mach, const double& oat,
                                 const double& current_ias, const double& current_tas, const double& current_mach);

    static double getMachFromIas(const double& ias, const double& oat,
                                 const double& current_ias, const double& current_tas, const double& current_mach);

    static double getDistanceToAltitude(const double alt_diff_ft,
                                        const double& ground_speed_kts,
                                        const double& vertical_speed_fpm);

    static double getFlightPath(const double& vs_ft_min, const double& ground_speed_kts)
    { return (ground_speed_kts < 0.1) ? 0.0 : toDeg(atan2(vs_ft_min*FEET_TO_NM*60.0, ground_speed_kts)); }

    static double getVsByFlightPath(const double& ground_speed_kts, const double fpv)
    { return (ground_speed_kts * tan(toRad(fpv))) / (FEET_TO_NM*60.0); }

	inline static int round(double value)
	{
		if (value >= 0.0)
			return (int)(value+0.5);
		else
			return (int)(value-0.5);
	}

    inline static double trimHeading(double hdg)
    {
        while (hdg < 0.0) hdg += 360.0;
        while (hdg >= 360.0) hdg -= 360.0;
        return hdg;
    }

    inline static int trimHeading(int hdg)
    {
        while (hdg < 0) hdg += 360;
        while (hdg >= 360) hdg -= 360;
        return hdg;
    }

    static double getTrackBetweenWaypoints(const Waypoint& fix1,
                                           const Waypoint& fix2);

    static double getDistBetweenWaypoints(const Waypoint& fix1,
										  const Waypoint& fix2);

    static bool getDistAndTrackBetweenWaypoints(const Waypoint& fix1,
                                                const Waypoint& fix2,
                                                double& distance_nm,
                                                double& track_degrees);

    static double getCrossTrackDistance(const Waypoint& from_wpt,
                                        const Waypoint& to_wpt,
                                        const Waypoint& current_pos,
                                        double& crs_from_to,
                                        double& crs_from_current,
                                        double& dist_from_current);

    static Waypoint getLatOnGreatCircleByLon(const Waypoint& from_wpt,
                                             const Waypoint& to_wpt,
                                             const Waypoint& current_pos);

    static Waypoint getIntermediateWaypoint(const Waypoint& from_wpt,
                                            const Waypoint& to_wpt,
                                            const double& dist_from_inter);

    static void getWindCorrAngleAndGS(double tas,
                                      double true_hdg,
                                      double wind_speed,
                                      double wind_true_hdg,
                                      double& wind_corr_angle,
                                      double& gs_kts);

    static double getWindCorrAngle(double tas,
                                   double true_hdg,
                                   double wind_speed,
                                   double wind_true_hdg);

    // declination will only be used when != 0
    static Waypoint getPBDWaypoint(const Waypoint& ref_waypoint,
                                   double bearing_deg,
                                   double distance_nm,
                                   const Declination* declination);

    // not working :(
    static bool getPBPBWaypoint(const Waypoint& ref_waypoint1,
                                const double& bearing1_deg,
                                const Waypoint& ref_waypoint2,
                                const double& bearing2_deg,
                                const Declination* declination,
                                Waypoint& new_waypoint);

    //! Same as the other getPreTurnDistance() but with explicit heading difference.
    static double getPreTurnDistance(const double& overfly_turn_dist,
                                     const double& turn_radius,
                                     const int& ground_speed,
                                     const double& curr_hdg,
                                     const double& next_hdg);

    //! Returns the distance from the next fix to initiate the next turn at.
    //! "overfly_turn_dist" is the distance to turn if the next fix is an
    //! overfly fix, "turn_radius" is the radius a turn will take at the current
    //! speed, "diff_hdg_to_turn" is the difference between the current
    //! heading and the heading after the next turn. The smaller the heading
    //! difference is, the smaller the turn distance will be.
    static double getPreTurnDistance(const double& overfly_turn_dist,
                                     const double& turn_radius,
                                     const int& ground_speed,
                                     double diff_hdg_to_turn);

    static bool isAfterAbeamFix(double track_to_wpt,
                                double abeam_track_to_wpt,
                                const double& abeam_angle);

    //! Returns true if the waypoint the "track_to_wpt" points to is behind
    //! us, by comparing to our true heading. "track_to_wpt" is the true track
    //! from the current position to the wanted waypoint.
    static bool isWaypointBehind(double track_to_wpt, double true_heading);

    //! Returns the absolute heading difference between the given headings.
	static double getAbsHeadingDiff(double current_hdg, double target_hdg);

    //! Returns the signed heading difference between the given headings.
	static double getSignedHeadingDiff(double current_hdg, double target_hdg);

    //! Returns the true heading to turn to when "current_true_heading" is the
    //! current true heading, "target_true_heading" is the true heading to turn
    //! to" and "is_left_turn" determines whether we would like to turn left of right.
    static double getTurnHeading(const double& current_true_heading, double target_true_heading, bool is_left_turn);

    //! returns the turn radius in NM
    static double getTurnRadiusNm(const double& ground_speed_kts, const double& bank_angle_deg);

    static void getPointAfterTurn(const Waypoint& reference_wpt,
                                  const double& initial_true_hdg,
                                  const double& ground_speed_kts,
                                  const double& bank_angle_deg,
                                  const double& true_hdg_to_turn_to,
                                  TURN_DIRECTION turn_direction,
                                  Waypoint& point_after_turn);
                                  
};

#endif
