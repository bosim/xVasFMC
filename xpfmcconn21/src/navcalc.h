#ifndef NAVCALC_H
#define NAVCALC_H

#ifndef M_PI
#define M_PI 3.1415926535897932385
#endif


class Navcalc
{
public:
    Navcalc() {}

    ~Navcalc() {}

    inline static double getInchesFromHpa(const double& hpa) { return hpa * HPA_TO_INHG; }

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

    inline static int round(double value)
    {
        if (value >= 0.0)
            return (int)(value+0.5);
        else
            return (int)(value-0.5);
    }

    static double getSignedHeadingDiff(double current_hdg, double target_hdg);

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
};

#endif // NAVCALC_H
