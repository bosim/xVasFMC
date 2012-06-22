#include "navcalc.h"

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

double Navcalc::getSignedHeadingDiff(double current_hdg, double target_hdg)
{
    double res = target_hdg - current_hdg;
    if (res < -180.0) return res + 360;
    if (res > +180.0) return res - 360;
    return res;
}
