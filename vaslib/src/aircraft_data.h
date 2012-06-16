///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2008 Alexander Wemmer 
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

/*! \file    aircraft_data.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __AIRCRAFT_DATA_H__
#define __AIRCRAFT_DATA_H__

#include <QMap>
#include <QList>
#include <QString>
#include <QFile>
#include <QPair>

#include "assert.h"
#include "logger.h"

typedef QPair<uint, uint> WeightToSpeedTuple;
typedef QPair<WeightToSpeedTuple, WeightToSpeedTuple> MinMaxWeightToSpeedTupleList;
typedef QMap<uint, MinMaxWeightToSpeedTupleList> FlapNotchToMinMaxWeightToStallSpeedMap;

class FlightStatus;

/////////////////////////////////////////////////////////////////////////////

//! This class stores aircraft type specific data
class AircraftData : public QObject
{
    Q_OBJECT

public:
    //! Standard Constructor
    AircraftData(const FlightStatus* flightstatus);

    //! Destructor
    virtual ~AircraftData() {};

    void clear();

    bool writeToFile(const QString& filename) const;
    bool readFromFile(const QString& filename);

    //-----

    inline const QString& name() const { return m_name; }

    double getStallSpeed() const;
    double getStallSpeed(uint flap_lever_notch) const;
    double getMaxSpeedKts() const;
    double getMaxFlapsSpeed(uint flap_level_notch) const;

    inline double maxN1() const { return m_max_n1_percent; }
    inline double maxEGTTemp() const { return m_max_egt_temp_degrees; }

    double getMinimumSelectableSpeed() const;
    double getMinimumSelectableSpeed(uint flap_lever_notch) const;
    double getMinimumAStyleFlapsRetractionSpeed() const;
    double getMinimumAStyleSlatsRetractionSpeed() const;
    double getGreenDotSpeed() const;

    double getVref(uint flap_lever_notch) const;
    double getV2(uint flap_lever_notch) const;

    inline double idleThrust() const { return m_idle_thrust; }

    inline QString checklistFilename() const { return m_checklist_filename; }

    inline double throttleControllerMaxRate() const { return m_throttle_controller_max_rate; }
    inline double throttleControllerTrendBoostFactor() const { return m_throttle_controller_trend_boost_factor; }
    inline double throttleControllerRateFactor() const { return m_throttle_controller_rate_factor; }

    inline double speedControllerMaxTrend() const { return m_speed_controller_max_trend; }
    inline double speedControllerTrendBoostFactor() const { return m_speed_controller_trend_boost_factor; }
    inline double speedControllerWrongDirectionTrendBoostFactor() const { return m_speed_controller_wrong_direction_trend_boost_factor; }
    inline double speedControllerRateFactor() const { return m_speed_controller_rate_factor; }

    inline double flchControllerIASTrendFactor() const { return m_flch_controller_ias_trend_factor; }
    inline double flchControllerN1TrendFactor() const { return m_flch_controller_n1_trend_factor; }
    inline double flchControllerIASTrendTargetLimit() const { return m_flch_controller_ias_trend_target_limit; }
    inline double flchControllerVSChangeFactor() const { return m_flch_controller_vs_change_factor; }
    inline double flchControllerVSChangeLimit() const { return m_flch_controller_vs_change_limit; }
    inline double flchControllerVSCmdDiffLimit() const { return m_flch_controller_vs_cmd_diff_limit; }
    inline double flchControllerVSCmdLimit() const { return m_flch_controller_vs_cmd_limit; }

signals:

    void signalChanged();

protected:

    template <class TYPE> QStringList convertNumberToStringList(const QList<TYPE> list) const
    {
        QStringList string_list;
        for(int index=0; index < list.count(); ++index) string_list.append(QString::number(list.at(index)));
        return string_list;
    }

    QList<uint> convertStringToUIntList(const QString text)
    {
        QList<uint> result_list;
        QStringList string_list= text.split(",");
        bool convok = false;

        for(int index=0; index < string_list.count(); ++index)
        {
            result_list.append(string_list[index].toUInt(&convok));
            if (!convok)
                Logger::log(QString("AircraftData:convertStringToUIntList: invalid value (%s)").arg(string_list[index]));
        }
        
        return result_list;
    }

    QList<double> convertStringToDoubleList(const QString text)
    {
        QList<double> result_list;
        QStringList string_list= text.split(",");
        bool convok = false;

        for(int index=0; index < string_list.count(); ++index)
        {
            result_list.append(string_list[index].toDouble(&convok));
            if (!convok)
                Logger::log(QString("AircraftData:convertStringToDoubleList: invalid value (%s)").arg(string_list[index]));
        }
        
        return result_list;
    }

    QString convertStallSpeedMapToString() const;
    bool convertStringToStallSpeedMap(const QString& string);

protected:

    const FlightStatus* m_flightstatus;

    QString m_name;
    
    uint m_max_zero_fuel_weight_kg;
    uint m_max_takeoff_weight_kg;
    uint m_max_landing_weight_kg;

    double m_max_speed_mach;
    uint m_max_speed_kts;

    uint m_nr_flap_notches;
    QList<uint> m_flap_max_speed_list_kts;
    QList<double> m_flap_min_selectable_speed_factor_list;

    uint m_max_gear_extend_altitude;
    uint m_max_gear_extend_speed;
    uint m_max_gear_retract_speed;
    uint m_max_gear_extended_speed;
    double m_max_gear_extend_mach;

    FlapNotchToMinMaxWeightToStallSpeedMap m_flap_stall_speed_map;

    uint m_max_egt_temp_degrees;
    double m_max_n1_percent;

    double m_vref_factor;
    double m_v2_factor;

    double m_astyle_min_flaps_retraction_factor;
    double m_astyle_min_slats_retraction_factor;

    mutable QMap<uint, double> m_flaps_lever_to_stall_speed_weight_cache_map;
    mutable QMap<uint, double> m_flaps_lever_to_stall_speed_cache_map;

    double m_idle_thrust;

    QString m_checklist_filename;

    double m_throttle_controller_max_rate;
    double m_throttle_controller_trend_boost_factor;
    double m_throttle_controller_rate_factor;

    double m_speed_controller_max_trend;
    double m_speed_controller_trend_boost_factor;
    double m_speed_controller_wrong_direction_trend_boost_factor;
    double m_speed_controller_rate_factor;

    double m_flch_controller_ias_trend_factor;
    double m_flch_controller_n1_trend_factor;
    double m_flch_controller_ias_trend_target_limit;
    double m_flch_controller_vs_change_factor;
    double m_flch_controller_vs_change_limit;
    double m_flch_controller_vs_cmd_limit;
    double m_flch_controller_vs_cmd_diff_limit;

private:
    //! Hidden copy-constructor
    AircraftData(const AircraftData&);
    //! Hidden assignment operator
    const AircraftData& operator = (const AircraftData&);
};

#endif /* __AIRCRAFT_DATA_H__ */

// End of file

