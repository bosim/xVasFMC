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

#include "assert.h"
#include "logger.h"
#include "waypoint.h"
#include "declination.h"

#include "navcalc.h"

/////////////////////////////////////////////////////////////////////////////

const double Navcalc::STD_ALTIMETER_HPA = 1013.2;
const double Navcalc::STD_ALTIMETER_INCHES = Navcalc::getInchesFromHpa(Navcalc::STD_ALTIMETER_HPA);
const double Navcalc::EARTH_CIRCUMFERENCE_KM = 40076.59;
const double Navcalc::METER_TO_FEET = 3.2808;
const double Navcalc::FEET_TO_METER = 0.3048;
const double Navcalc::FEET_TO_NM = 0.000164466;
const double Navcalc::METER_TO_NM = 0.000539956803455724;
const double Navcalc::METER_PER_SECOND_TO_KNOTS = 1.94384;
const double Navcalc::T0ISA = 15.0;
const double Navcalc::POUNDS_TO_KG = 0.45;
const double Navcalc::USGALLONS_TO_LITERS = 3.785411784;
const double Navcalc::EARTH_ACCELERATION_NM_PER_HOUR = 68641.6;
const double Navcalc::JETA1_LITRES_TO_KILOS = (1/1.25);
const double Navcalc::HPA_TO_INHG = 0.02953;

/////////////////////////////////////////////////////////////////////////////

double Navcalc::getIasFromMach(const double& mach,
                               const double& oat,
                               const double& current_ias,
                               const double& current_tas,
                               const double& current_mach)
{
    if (current_tas < 1.0) return getTasFromMach(mach, oat);
    if (current_mach < 0.3) return getTasFromMach(mach, oat) * current_ias / current_tas;
    return (current_ias / current_mach) * mach;
}

/////////////////////////////////////////////////////////////////////////////

double Navcalc::getMachFromIas(const double& ias,
                               const double& oat,
                               const double& current_ias,
                               const double& current_tas,
                               const double& current_mach)
{
    if (current_tas < 1.0) return getMachFromTas(ias, oat);
    if (current_mach < 0.3) return getMachFromTas(ias * (current_tas / current_ias), oat);
    return (current_mach / current_ias) * ias;
}

/////////////////////////////////////////////////////////////////////////////

double Navcalc::getMSLTemp(double oat, double altitude_ft)
{
    double msl_temp = oat + (6.5*altitude_ft*FEET_TO_METER/1000.0);
    return msl_temp;
}

/////////////////////////////////////////////////////////////////////////////

double Navcalc::getAltTemp(double current_oat, double current_alt, double wanted_altitude_ft)
{
    double tropo_ft = getTropopauseFt(current_oat, current_alt);
    if (wanted_altitude_ft > tropo_ft) return 216.65 - 273.15;
    return current_oat - (6.5*(wanted_altitude_ft-current_oat)*FEET_TO_METER/1000.0);
}

/////////////////////////////////////////////////////////////////////////////

double Navcalc::getTropopauseFt(double oat, double altitude_ft)
{
    return (11000.0 + ((1000 / 6.5) * (getMSLTemp(oat, altitude_ft) - T0ISA))) * METER_TO_FEET;
}

/////////////////////////////////////////////////////////////////////////////

double Navcalc::getDistanceToAltitude(double alt_diff_ft,
                                      const double& ground_speed_kts,
                                      const double& vertical_speed_fpm)
{
    if (((vertical_speed_fpm > 0.0 && alt_diff_ft > 0.0) || 
	(vertical_speed_fpm < 0.0 && alt_diff_ft < 0.0 &&
        fabs(vertical_speed_fpm) > 100.0 && fabs(alt_diff_ft) > 200.0)))
    {
        double abs_vertical_speed_fpm = (fabs(vertical_speed_fpm) / 10) * 10;
        alt_diff_ft = (alt_diff_ft / 10) * 10;
        return qAbs(alt_diff_ft * ground_speed_kts) / (abs_vertical_speed_fpm * 60.0);
    }

    return 0.0;
}

/////////////////////////////////////////////////////////////////////////////

double Navcalc::getTrackBetweenWaypoints(const Waypoint& fix1, const Waypoint& fix2)
{
    if (fix1.lat() == fix2.lat() && fix1.lon() == fix2.lon()) return 0.0;

    double rad_lat1 = toRad(fix1.lat());
    double rad_lon1 = toRad(fix1.lon());
    double rad_lat2 = toRad(fix2.lat());
    double rad_lon2 = toRad(fix2.lon());

    double sin_lat1 = sin(rad_lat1);
    double sin_lat2 = sin(rad_lat2);
    double cos_lat1 = cos(rad_lat1);
    double cos_lat2 = cos(rad_lat2);
    double lon1_min_lon2 = rad_lon1-rad_lon2;

    // source: http://williams.best.vwh.net/avform.htm

    double track_rad =
        fmod(atan2(sin(lon1_min_lon2)*cos_lat2,
                   cos_lat1*sin_lat2-sin_lat1*cos_lat2*cos(lon1_min_lon2)), 2*M_PI);

    return trimHeading(toDeg(-track_rad));
}

/////////////////////////////////////////////////////////////////////////////

double Navcalc::getDistBetweenWaypoints(const Waypoint& fix1,
										const Waypoint& fix2)
{
	if (fix1.lat() == fix2.lat() && fix1.lon() == fix2.lon()) return 0.0;

    double rad_lat1 = toRad(fix1.lat());
    double rad_lon1 = toRad(fix1.lon());
    double rad_lat2 = toRad(fix2.lat());
    double rad_lon2 = toRad(fix2.lon());

    double sin_lat1 = sin(rad_lat1);
    double sin_lat2 = sin(rad_lat2);
    double cos_lat1 = cos(rad_lat1);
    double cos_lat2 = cos(rad_lat2);
    double lon1_min_lon2 = rad_lon1-rad_lon2;

    return toDeg(acos(sin_lat1*sin_lat2 + cos_lat1*cos_lat2*cos(lon1_min_lon2))) * 60.0;
}

/////////////////////////////////////////////////////////////////////////////

bool Navcalc::getDistAndTrackBetweenWaypoints(const Waypoint& fix1,
                                              const Waypoint& fix2,
                                              double& distance_nm,
                                              double& track_degrees)
{
    distance_nm = 0.0;
    track_degrees = 0.0;

    if (fix1.lat() == fix2.lat() && fix1.lon() == fix2.lon()) return true;

    double rad_lat1 = toRad(fix1.lat());
    double rad_lon1 = toRad(fix1.lon());
    double rad_lat2 = toRad(fix2.lat());
    double rad_lon2 = toRad(fix2.lon());

    double sin_lat1 = sin(rad_lat1);
    double sin_lat2 = sin(rad_lat2);
    double cos_lat1 = cos(rad_lat1);
    double cos_lat2 = cos(rad_lat2);
    double lon1_min_lon2 = rad_lon1-rad_lon2;

    // source: http://williams.best.vwh.net/avform.htm

    double distance_rad =
        acos(sin_lat1*sin_lat2 + cos_lat1*cos_lat2*cos(lon1_min_lon2));

    double track_rad =
        fmod(atan2(sin(lon1_min_lon2)*cos_lat2,
                   cos_lat1*sin_lat2-sin_lat1*cos_lat2*cos(lon1_min_lon2)), 2*M_PI);

    distance_nm = toDeg(distance_rad) * 60.0;
    track_degrees = trimHeading(toDeg(-track_rad));

//     printf("%s (%lf/%lf) -> %s (%lf/%lf): dist %lfnm, track %lf�\n",
//            fix1.id().latin1(), fix1.lat(), fix1.lon(),
//            fix2.id().latin1(), fix2.lat(), fix2.lon(),
//            distance_nm, track_degrees);

    return true;
}

/////////////////////////////////////////////////////////////////////////////

double Navcalc::getCrossTrackDistance(const Waypoint& from_wpt,
                                      const Waypoint& to_wpt,
                                      const Waypoint& current_pos,
                                      double& crs_from_to,
                                      double& crs_from_current,
                                      double& dist_from_current)
{
    MYASSERT(getDistAndTrackBetweenWaypoints(from_wpt, current_pos, dist_from_current, crs_from_current));

    double dist_from_to = 0.0;
    getDistAndTrackBetweenWaypoints(from_wpt, to_wpt, dist_from_to, crs_from_to);

    double alpha = getSignedHeadingDiff(crs_from_to, crs_from_current);
    return sin(toRad(alpha)) * dist_from_current;
}

/////////////////////////////////////////////////////////////////////////////

Waypoint Navcalc::getLatOnGreatCircleByLon(const Waypoint& from_wpt,
                                           const Waypoint& to_wpt,
                                           const Waypoint& current_pos)
{
    if (from_wpt.lon() - to_wpt.lon() < 0.00001)
    {
        return Waypoint("inter", "inter", current_pos.lat(), from_wpt.lon());
    }

    const double lat1 = toRad(from_wpt.lat());
    const double lon1 = toRad(from_wpt.lon());
    const double lat2 = toRad(to_wpt.lat());
    const double lon2 = toRad(to_wpt.lon());
    const double& lon = toRad(current_pos.lon());

    return Waypoint("inter", "inter",
                    toDeg(atan((sin(lat1)*cos(lat2)*sin(lon-lon2)
                                -sin(lat2)*cos(lat1)*sin(lon-lon1))/
                               (cos(lat1)*cos(lat2)*sin(lon1-lon2)))),
                    current_pos.lon());
}

/////////////////////////////////////////////////////////////////////////////

Waypoint Navcalc::getIntermediateWaypoint(const Waypoint& from_wpt,
                                          const Waypoint& to_wpt,
                                          const double& dist_from_inter)
{
    double dist_from_to = getDistBetweenWaypoints(from_wpt, to_wpt);
    double dist_frac = dist_from_inter / dist_from_to;

    if (dist_frac > 1.0)       dist_frac = 1.0;
    else if (dist_frac < 0.0)  dist_frac = 0.0;

    double f = dist_frac;
    double d = toRad(dist_from_to) / 60.0;

    double lat1 = toRad(from_wpt.lat());
    double lon1 = toRad(from_wpt.lon());
    double lat2 = toRad(to_wpt.lat());
    double lon2 = toRad(to_wpt.lon());

    // A=sin((1-f)*d)/sin(d)
    double a = sin((1-f)*d)/sin(d);

    // B=sin(f*d)/sin(d)
    double b = sin(f*d)/sin(d);

    // x = A*cos(lat1)*cos(lon1)  +  B*cos(lat2)*cos(lon2)
    double x = a * cos(lat1)*cos(lon1) +  b*cos(lat2)*cos(lon2);

    // y = A*cos(lat1)*sin(lon1)  +  B*cos(lat2)*sin(lon2)
    double y = a*cos(lat1)*sin(lon1) +  b*cos(lat2)*sin(lon2);

    // z = A*sin(lat1)  +  B*sin(lat2)
    double z = a*sin(lat1)  +  b*sin(lat2);

    // lat=atan2(z,sqrt(x^2+y^2))
    // lon=atan2(y,x)
    return Waypoint("inter", "inter", toDeg(atan2(z,sqrt(x*x+y*y))), toDeg(atan2(y,x)));
}

/////////////////////////////////////////////////////////////////////////////

double Navcalc::getWindCorrAngle(double tas, double true_hdg,
                                 double wind_speed, double wind_true_hdg)
{
    double vpx = sin(toRad(true_hdg)) * tas;
    double vpy = cos(toRad(true_hdg)) * tas;

    double vwx = sin(toRad(wind_true_hdg+180)) * wind_speed;
    double vwy = cos(toRad(wind_true_hdg+180)) * wind_speed;

    double vx = vpx + vwx;
    double vy = vpy + vwy;

    double gnd_track_calc = trimHeading(toDeg(atan2(vx, vy)));

    double wind_corr_angle = -(gnd_track_calc - true_hdg);
    if (wind_corr_angle > 180) wind_corr_angle -= 360;
    if (tas < 10) wind_corr_angle = 0.0;

    return wind_corr_angle;

//         printf("WindCorr: Vp=(%lf/%lf) Vw=(%lf/%lf) -> V=(%lf/%lf) | windcorr=%lf, gs=%lf\n",
}

/////////////////////////////////////////////////////////////////////////////

void Navcalc::getWindCorrAngleAndGS(double tas, double true_hdg,
                                    double wind_speed, double wind_true_hdg,
                                    double& wind_corr_angle, double& gs_kts)
{
    double vpx = sin(toRad(true_hdg)) * tas;
    double vpy = cos(toRad(true_hdg)) * tas;

    double vwx = sin(toRad(wind_true_hdg+180)) * wind_speed;
    double vwy = cos(toRad(wind_true_hdg+180)) * wind_speed;

    double vx = vpx + vwx;
    double vy = vpy + vwy;

    double gnd_track_calc = trimHeading(toDeg(atan2(vx, vy)));

    wind_corr_angle = -(gnd_track_calc - true_hdg);
    if (wind_corr_angle > 180) wind_corr_angle -= 360;
    if (tas < 10) wind_corr_angle = 0.0;

    gs_kts = sqrt(vx*vx + vy*vy);

//         printf("WindCorr: Vp=(%lf/%lf) Vw=(%lf/%lf) -> V=(%lf/%lf) | windcorr=%lf, gs=%lf\n",
//                vpx, vpy, vwx, vwy, vx, vy, wind_corr_angle, gs_kts);
}

/////////////////////////////////////////////////////////////////////////////

Waypoint Navcalc::getPBDWaypoint(const Waypoint& ref_waypoint,
                                 double bearing_deg,
                                 double distance_nm,
                                 const Declination* declination)
{
    if (declination != 0)
    {
        double magvar = declination->declination(ref_waypoint);
        Logger::log(QString("Navcalc::getPBDWaypoint: magvar=%1 for wpt %2").arg(magvar).arg(ref_waypoint.toString()));
        bearing_deg -= magvar;
    }

    Waypoint new_waypoint("PBD", "", 0.0, 0.0);

    double track_rad = toRad(trimHeading(-bearing_deg));
    double distance_rad = toRad(distance_nm) / 60.0;

    double ref_lat = toRad(ref_waypoint.lat());
    double ref_lon = toRad(ref_waypoint.lon());

//     printf("Navcalc:getBDWaypoint: ref: (%lf/%lf) (%lf/%lf), track: %d/%lf, dist: %lf/%lf\n",
//            ref_waypoint.lat(), ref_waypoint.lon(), ref_lat, ref_lon,
//            bearing_deg, track_rad, distance_nm, distance_rad);

    //----- calc BD waypoint

    new_waypoint.setLat(toDeg(asin(sin(ref_lat)*cos(distance_rad)+
                                   cos(ref_lat)*sin(distance_rad)*cos(track_rad))));

    double dlon = atan2(sin(track_rad)*sin(distance_rad)*cos(ref_lat),
                        cos(distance_rad)-sin(ref_lat)*
                        sin(toRad(new_waypoint.lat())));

    new_waypoint.setLon(toDeg(fmod(ref_lon - dlon + M_PI, 2*M_PI ) - M_PI));

    //-----

//     printf("Navcalc:getBDWaypoint: result: (%lf/%lf)\n",
//            new_waypoint.lat(), new_waypoint.lon());

    return new_waypoint;
}

/////////////////////////////////////////////////////////////////////////////

bool Navcalc::getPBPBWaypoint(const Waypoint& ref_waypoint1, const double& bearing1_deg,
                              const Waypoint& ref_waypoint2, const double& bearing2_deg,
                              const Declination* declination,
                              Waypoint& new_waypoint)
{
    new_waypoint = Waypoint();

    Logger::log(QString("Navcalc::getPBPBWaypoint: %1/%2/%3 <-> %4/%5/%6").
                arg(ref_waypoint1.lat()).arg(ref_waypoint1.lon()).arg(bearing1_deg).
                arg(ref_waypoint2.lat()).arg(ref_waypoint2.lon()).arg(bearing2_deg));

    double magvar = 0.0;

    if (declination != 0)
    {
        double magvar = declination->declination(ref_waypoint1);
        Logger::log(QString("Navcalc::getPBPBWaypoint: magvar=%1 for wpt %2").arg(magvar).arg(ref_waypoint1.toString()));
    }

    double crs13 = toRad(trimHeading(bearing1_deg - magvar));
    double crs23 = toRad(trimHeading(bearing2_deg - magvar));

    double lat1 = toRad(ref_waypoint1.lat());
    double lon1 = toRad(-ref_waypoint1.lon());

    double lat2 = toRad(ref_waypoint2.lat());
    double lon2 = toRad(-ref_waypoint2.lon());

    //----- calc BB waypoint

    double dst12 = 2*asin(sqrt( pow((sin((lat1-lat2)/2)), 2) + cos(lat1)*cos(lat2)*(pow(sin((lon1-lon2)/2),2))));
    double crs12 = 0.0;
    double crs21 = 0.0;

    if (sin(lon2-lon1)<0)
    {
        crs12=acos((sin(lat2)-sin(lat1)*cos(dst12))/(sin(dst12)*cos(lat1)));
        crs21=2.*M_PI-acos((sin(lat1)-sin(lat2)*cos(dst12))/(sin(dst12)*cos(lat2)));
    }
    else
    {
        crs12=2.*M_PI-acos((sin(lat2)-sin(lat1)*cos(dst12))/(sin(dst12)*cos(lat1)));
        crs21=acos((sin(lat1)-sin(lat2)*cos(dst12))/(sin(dst12)*cos(lat2)));
    }

    double ang1 = fmod(crs13-crs12+M_PI,2.*M_PI)-M_PI;
    double ang2 = fmod(crs21-crs23+M_PI,2.*M_PI)-M_PI;

    if ((sin(ang1)==0 && sin(ang2)==0))
    {
        Logger::log(QString("Navcalc::getPBPBWaypoint: infinity of intersections - %1/%2 - %3/%4").
                    arg(ref_waypoint1.toString()).arg(bearing1_deg).
                    arg(ref_waypoint2.toString()).arg(bearing2_deg));
        return false;
    }
    else if (sin(ang1)*sin(ang2)<0)
    {
        Logger::log(QString("Navcalc::getPBPBWaypoint: ambigous intersection - %1/%2 - %3/%4").
                    arg(ref_waypoint1.toString()).arg(bearing1_deg).
                    arg(ref_waypoint2.toString()).arg(bearing2_deg));
        return false;
    }

    ang1=qAbs(ang1);
    ang2=qAbs(ang2);
    double ang3=acos(-cos(ang1)*cos(ang2)+sin(ang1)*sin(ang2)*cos(dst12));
    double dst13=atan2(sin(dst12)*sin(ang1)*sin(ang2),cos(ang2)+cos(ang1)*cos(ang3));
    double lat3=asin(sin(lat1)*cos(dst13)+cos(lat1)*sin(dst13)*cos(crs13));
    double dlon=atan2(sin(crs13)*sin(dst13)*cos(lat1),cos(dst13)-sin(lat1)*sin(lat3));
    double lon3=fmod(lon1-dlon+M_PI,2*M_PI)-M_PI;

    new_waypoint.setLat(toDeg(lat3));
    new_waypoint.setLon(toDeg(-lon3));
    new_waypoint.setId(QString("%1%2/%3%4").
                       arg(ref_waypoint1.id()).arg((int)bearing1_deg).
                       arg(ref_waypoint2.id()).arg((int)bearing2_deg));
    
    Logger::log(QString("Navcalc:getPBPBWaypoint: result: rad:(%1/%2) deg:(%3/%4)").
                arg(lat3).arg(-lon3).arg(new_waypoint.lat()).arg(new_waypoint.lon()));
    return true;
}

/////////////////////////////////////////////////////////////////////////////

double Navcalc::getPreTurnDistance(const double& overfly_turn_dist,
								   const double& turn_radius,
                                   const int& ground_speed,
                                   const double& curr_hdg,
                                   const double& next_hdg)
{
    double hdg_diff = fabs(curr_hdg - next_hdg);
    return getPreTurnDistance(overfly_turn_dist, turn_radius, ground_speed, hdg_diff);
}

/////////////////////////////////////////////////////////////////////////////

double Navcalc::getPreTurnDistance(const double& overfly_turn_dist,
								   const double& turn_radius,
                                   const int& ground_speed,
                                   double diff_hdg_to_turn)
{
    MYASSERT(diff_hdg_to_turn >= 0.0);
    MYASSERT(turn_radius >= 0.0);

    //TODO enhance for >90 deg turns

    // add 20% for long turns
    if (diff_hdg_to_turn > 120.0) return turn_radius * 1.2;

    diff_hdg_to_turn = qMax(15.0, diff_hdg_to_turn);

    // we add 1/900 the times of the ground speed which equals the distace of 6 seconds of flight
    // and multiply the result procentual by the ground speed
    return qMax(overfly_turn_dist,
                (diff_hdg_to_turn*turn_radius/90.0 + (ground_speed/600.0)) * (1.0+ground_speed/5000.0));
}

/////////////////////////////////////////////////////////////////////////////

bool Navcalc::isAfterAbeamFix(double track_to_wpt,
                              double abeam_track_to_wpt,
                              const double& abeam_angle)
{
    track_to_wpt = trimHeading(track_to_wpt);
    abeam_track_to_wpt = trimHeading(abeam_track_to_wpt);
    double abeam_track_to_wpt_upper_border = trimHeading(abeam_track_to_wpt + abeam_angle);

    bool is_after_abeam = false;

    if (abeam_track_to_wpt_upper_border >= abeam_track_to_wpt)
    {
        is_after_abeam = (track_to_wpt >= abeam_track_to_wpt &&
                          track_to_wpt <= abeam_track_to_wpt_upper_border);
    }
    else
    {
        is_after_abeam = (track_to_wpt >= abeam_track_to_wpt ||
                          track_to_wpt <= abeam_track_to_wpt_upper_border);
    }

//     printf("isAfterAbeamFix: ttw:%.02lf� [%.02lf�|%.02lf�] afterabeam:%d\n",
//            track_to_wpt, abeam_track_to_wpt, abeam_track_to_wpt_upper_border, is_after_abeam);

    return is_after_abeam;
}

/////////////////////////////////////////////////////////////////////////////

bool Navcalc::isWaypointBehind(double track_to_wpt, double true_heading)
{
    bool ret = isAfterAbeamFix(track_to_wpt, true_heading+90.0, 180.0);
//     printf("isWaypointBehind: ttw:%.02lf� thdg:%.02lf� isb:%d\n",
//            track_to_wpt, true_heading, ret);
    return ret;
}

/////////////////////////////////////////////////////////////////////////////

double Navcalc::getAbsHeadingDiff(double current_hdg, double target_hdg)
{
    double res = fabs(target_hdg - current_hdg);
    if (res > 180.0) res = 360.0 - res;
    return res;
}

/////////////////////////////////////////////////////////////////////////////

double Navcalc::getSignedHeadingDiff(double current_hdg, double target_hdg)
{
	double res = target_hdg - current_hdg;
	if (res < -180.0) return res + 360;
	if (res > +180.0) return res - 360;
	return res;
}

/////////////////////////////////////////////////////////////////////////////

double Navcalc::getTurnHeading(const double& current_true_heading, double target_true_heading, bool is_left_turn)
{
    double true_track_result = target_true_heading;
    double diff_angle = 0.0;

    if (is_left_turn)
    {
        while(target_true_heading > current_true_heading) target_true_heading -= 360.0;
        diff_angle = target_true_heading - current_true_heading;
        MYASSERT(diff_angle <= 0.0);
        if (diff_angle < -90.0) true_track_result = current_true_heading - 90.0;
    }
    else
    {
        while(target_true_heading < current_true_heading) target_true_heading += 360.0;
        diff_angle = target_true_heading - current_true_heading;
        MYASSERT(diff_angle >= 0.0);
        if (diff_angle > 90.0) true_track_result = current_true_heading + 90.0;
    }

    return Navcalc::trimHeading(true_track_result);

//     printf("getTurnHeading: %.2lf� -> %.2lf� (left:%d)(diff:%.2lf�) = %.2lf�\n",
//            current_true_heading, target_true_heading,
//            is_left_turn, diff_angle, true_track_result);
}

/////////////////////////////////////////////////////////////////////////////

double Navcalc::getTurnRadiusNm(const double& ground_speed_kts, const double& bank_angle_deg)
{
    return (ground_speed_kts * ground_speed_kts) / (EARTH_ACCELERATION_NM_PER_HOUR * tan(toRad(bank_angle_deg)));
}

/////////////////////////////////////////////////////////////////////////////

void Navcalc::getPointAfterTurn(const Waypoint& reference_wpt,
                                const double& initial_true_hdg,
                                const double& ground_speed_kts,
                                const double& bank_angle_deg,
                                const double& true_hdg_to_turn_to,
                                TURN_DIRECTION turn_direction,
                                Waypoint& point_after_turn)
{
    double radius = getTurnRadiusNm(ground_speed_kts, bank_angle_deg);
    double hdg_diff = getSignedHeadingDiff(initial_true_hdg, true_hdg_to_turn_to);
    
    if (turn_direction == TURN_LEFT && hdg_diff > 0)       hdg_diff -= 360.0;
    else if (turn_direction == TURN_RIGHT && hdg_diff < 0) hdg_diff += 360.0;

    double pbd_bearing = trimHeading(initial_true_hdg + hdg_diff*0.5);
    double pbd_distance = radius * sqrt(2.0 * (1.0 - cos(toRad(hdg_diff))));

    point_after_turn = getPBDWaypoint(reference_wpt, pbd_bearing, pbd_distance, 0);

    Logger::log(QString("Navcalc:getPointAfterTurn: R=%1, hdg_diff=%2 (%3 -> %4), pbd=%5deg@%6nm").
                arg(radius).arg(hdg_diff).arg(initial_true_hdg).arg(true_hdg_to_turn_to).
                arg(pbd_bearing).arg(pbd_distance));
}
