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

/*! \file    aircraft_data.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include <QRegExp>

#include "config.h"
#include "flightstatus.h"

#include "aircraft_data.h"

/////////////////////////////////////////////////////////////////////////////

AircraftData::AircraftData(const FlightStatus* flightstatus) : m_flightstatus(flightstatus)
{
    MYASSERT(m_flightstatus != 0);

    // load defaults

    m_name = "A320";

    m_max_zero_fuel_weight_kg = 61000;
    m_max_takeoff_weight_kg = 77000;
    m_max_landing_weight_kg = 64500;

    m_max_speed_mach = 0.82;
    m_max_speed_kts = 350;

    m_nr_flap_notches = 6;
    m_flap_max_speed_list_kts.append(350);
    m_flap_max_speed_list_kts.append(230);
    m_flap_max_speed_list_kts.append(215);
    m_flap_max_speed_list_kts.append(200);
    m_flap_max_speed_list_kts.append(185);
    m_flap_max_speed_list_kts.append(177);

    m_flap_min_selectable_speed_factor_list.append(1.28);
    m_flap_min_selectable_speed_factor_list.append(1.23);
    m_flap_min_selectable_speed_factor_list.append(1.13);
    m_flap_min_selectable_speed_factor_list.append(1.13);
    m_flap_min_selectable_speed_factor_list.append(1.13);
    m_flap_min_selectable_speed_factor_list.append(1.13);

    m_max_gear_extend_altitude = 25000;
    m_max_gear_extend_speed = 250;
    m_max_gear_retract_speed = 220;
    m_max_gear_extended_speed = 280;
    m_max_gear_extend_mach = 0.67;

    m_flap_stall_speed_map[0] = MinMaxWeightToSpeedTupleList(WeightToSpeedTuple(35000, 115), WeightToSpeedTuple(80000, 174));
    m_flap_stall_speed_map[1] = MinMaxWeightToSpeedTupleList(WeightToSpeedTuple(35000, 95), WeightToSpeedTuple(80000, 145));
    m_flap_stall_speed_map[2] = MinMaxWeightToSpeedTupleList(WeightToSpeedTuple(35000, 90), WeightToSpeedTuple(80000, 132));
    m_flap_stall_speed_map[3] = MinMaxWeightToSpeedTupleList(WeightToSpeedTuple(35000, 85), WeightToSpeedTuple(80000, 128));
    m_flap_stall_speed_map[4] = MinMaxWeightToSpeedTupleList(WeightToSpeedTuple(35000, 84), WeightToSpeedTuple(80000, 126));
    m_flap_stall_speed_map[5] = MinMaxWeightToSpeedTupleList(WeightToSpeedTuple(35000, 82), WeightToSpeedTuple(80000, 124));

    m_max_egt_temp_degrees = 950;
    m_max_n1_percent = 104.0;

    m_vref_factor = 1.23;
    m_v2_factor = 1.13;
    m_astyle_min_flaps_retraction_factor = 1.25;
    m_astyle_min_slats_retraction_factor = 1.25;

    m_idle_thrust = 10.0;

    m_throttle_controller_max_rate = 7.0;
    m_throttle_controller_trend_boost_factor = 4.0;
    m_throttle_controller_rate_factor = 13.0;

    m_speed_controller_max_trend = 2.0;
    m_speed_controller_trend_boost_factor = 12.0;
    m_speed_controller_wrong_direction_trend_boost_factor = 1.5;
    m_speed_controller_rate_factor = 14.0;

    m_flch_controller_ias_trend_factor = 10.0;
    m_flch_controller_n1_trend_factor = 5.5;
    m_flch_controller_ias_trend_target_limit = 15.0;
    m_flch_controller_vs_change_factor = 100.0;
    m_flch_controller_vs_change_limit = 250.0;
    m_flch_controller_vs_cmd_limit = 5000.0;
    m_flch_controller_vs_cmd_diff_limit = 1250.0;
}

/////////////////////////////////////////////////////////////////////////////

bool AircraftData::writeToFile(const QString& filename) const
{
    Config cfg(filename);

    cfg.setValue("name", m_name);

    cfg.setValue("max_zero_fuel_weight_kg", m_max_zero_fuel_weight_kg);
    cfg.setValue("max_takeoff_weight_kg", m_max_takeoff_weight_kg);
    cfg.setValue("max_landing_weight_kg", m_max_landing_weight_kg);

    cfg.setValue("max_speed_mach", m_max_speed_mach);
    cfg.setValue("max_speed_kts", m_max_speed_kts);

    cfg.setValue("nr_flap_notches", m_nr_flap_notches);

    cfg.setValue("flap_max_speed_list_kts", 
                 convertNumberToStringList<uint>(m_flap_max_speed_list_kts).join(","));
    cfg.setValue("flap_min_selectable_speed_factor_list", 
                 convertNumberToStringList<double>(m_flap_min_selectable_speed_factor_list).join(","));

    cfg.setValue("max_gear_extend_altitude", m_max_gear_extend_altitude);
    cfg.setValue("max_gear_extend_speed", m_max_gear_extend_speed);
    cfg.setValue("max_gear_retract_speed", m_max_gear_retract_speed);
    cfg.setValue("max_gear_extended_speed", m_max_gear_extended_speed);
    cfg.setValue("max_gear_extend_mach", m_max_gear_extend_mach);
    
    cfg.setValue("flap_stall_speed_map", convertStallSpeedMapToString());

    cfg.setValue("max_egt_temp_degrees", m_max_egt_temp_degrees);
    cfg.setValue("max_n1_percent", m_max_n1_percent);

    cfg.setValue("vref_factor", m_vref_factor);
    cfg.setValue("v2_factor", m_v2_factor);
    cfg.setValue("astyle_min_flaps_retraction_factor", m_astyle_min_flaps_retraction_factor);
    cfg.setValue("astyle_min_slats_retraction_factor", m_astyle_min_slats_retraction_factor);

    cfg.setValue("idle_thrust", m_idle_thrust);
    cfg.setValue("checklist_filename", "");

    cfg.setValue("throttle_controller_max_trend", m_throttle_controller_max_rate);
    cfg.setValue("throttle_controller_trend_boost_factor", m_throttle_controller_trend_boost_factor);
    cfg.setValue("throttle_controller_rate_factor", m_throttle_controller_rate_factor);

    cfg.setValue("speed_controller_max_trend", m_speed_controller_max_trend);
    cfg.setValue("speed_controller_trend_boost_factor", m_speed_controller_trend_boost_factor);
    cfg.setValue("speed_controller_wrong_direction_trend_boost_factor", m_speed_controller_wrong_direction_trend_boost_factor);
    cfg.setValue("speed_controller_rate_factor", m_speed_controller_rate_factor);

    cfg.setValue("flch_controller_ias_trend_factor", m_flch_controller_ias_trend_factor);
    cfg.setValue("flch_controller_n1_trend_factor", m_flch_controller_n1_trend_factor);
    cfg.setValue("flch_controller_ias_trend_target_limit", m_flch_controller_ias_trend_target_limit);
    cfg.setValue("flch_controller_vs_change_factor", m_flch_controller_vs_change_factor);
    cfg.setValue("flch_controller_vs_change_limit", m_flch_controller_vs_change_limit);
    cfg.setValue("flch_controller_vs_cmd_limit", m_flch_controller_vs_cmd_limit);
    cfg.setValue("flch_controller_vs_cmd_diff_limit", m_flch_controller_vs_cmd_diff_limit);

    return cfg.saveToFile();
}

/////////////////////////////////////////////////////////////////////////////

bool AircraftData::readFromFile(const QString& filename)
{
    Config cfg(filename);
    if (!cfg.loadfromFile()) 
    {
        Logger::log(QString("AircraftData:readFromFile: could not read (%1)").arg(cfg.filename()));
        return false;
    }
 
    clear();
   
    m_name = cfg.getValue("name");

    m_max_zero_fuel_weight_kg = cfg.getIntValue("max_zero_fuel_weight_kg");
    m_max_takeoff_weight_kg = cfg.getIntValue("max_takeoff_weight_kg");
    m_max_landing_weight_kg = cfg.getIntValue("max_landing_weight_kg");
    
    m_max_speed_mach = cfg.getDoubleValue("max_speed_mach");
    m_max_speed_kts = cfg.getIntValue("max_speed_kts");
    
    m_nr_flap_notches = cfg.getIntValue("nr_flap_notches");
    m_flap_max_speed_list_kts = 
        convertStringToUIntList(cfg.getValue("flap_max_speed_list_kts"));
    m_flap_min_selectable_speed_factor_list = 
        convertStringToDoubleList(cfg.getValue("flap_min_selectable_speed_factor_list"));

    m_max_gear_extend_altitude = cfg.getIntValue("max_gear_extend_altitude");
    m_max_gear_extend_speed = cfg.getIntValue("max_gear_extend_speed");
    m_max_gear_retract_speed = cfg.getIntValue("max_gear_retract_speed");
    m_max_gear_extended_speed = cfg.getIntValue("max_gear_extended_speed");
    m_max_gear_extend_mach = cfg.getDoubleValue("max_gear_extend_mach");

    MYASSERT(convertStringToStallSpeedMap(cfg.getValue("flap_stall_speed_map")));

    m_max_egt_temp_degrees = cfg.getIntValue("max_egt_temp_degrees");
    m_max_n1_percent = cfg.getDoubleValue("max_n1_percent");

    m_vref_factor = cfg.getDoubleValue("vref_factor");
    m_v2_factor = cfg.getDoubleValue("v2_factor");

    m_astyle_min_slats_retraction_factor = cfg.getDoubleValue("astyle_min_slats_retraction_factor");
    m_astyle_min_flaps_retraction_factor = cfg.getDoubleValue("astyle_min_flaps_retraction_factor");

    MYASSERT(m_flap_max_speed_list_kts.count() == (int)m_nr_flap_notches);
    MYASSERT(m_flap_stall_speed_map.count() == (int)m_nr_flap_notches);

    m_idle_thrust = qMax(0.0, cfg.getDoubleValue("idle_thrust"));

    if (cfg.contains("checklist_filename"))
        m_checklist_filename = cfg.getValue("checklist_filename");

    m_throttle_controller_max_rate = cfg.getDoubleValue("throttle_controller_max_trend");
    m_throttle_controller_trend_boost_factor = cfg.getDoubleValue("throttle_controller_trend_boost_factor");
    m_throttle_controller_rate_factor = cfg.getDoubleValue("throttle_controller_rate_factor");

    m_speed_controller_max_trend = cfg.getDoubleValue("speed_controller_max_trend");
    m_speed_controller_trend_boost_factor = cfg.getDoubleValue("speed_controller_trend_boost_factor");
    m_speed_controller_wrong_direction_trend_boost_factor = cfg.getDoubleValue("speed_controller_wrong_direction_trend_boost_factor");
    m_speed_controller_rate_factor = cfg.getDoubleValue("speed_controller_rate_factor");

    m_flch_controller_ias_trend_factor = cfg.getDoubleValue("flch_controller_ias_trend_factor");
    m_flch_controller_n1_trend_factor = cfg.getDoubleValue("flch_controller_n1_trend_factor");
    m_flch_controller_ias_trend_target_limit = cfg.getDoubleValue("flch_controller_ias_trend_target_limit");
    m_flch_controller_vs_change_factor = cfg.getDoubleValue("flch_controller_vs_change_factor");
    m_flch_controller_vs_change_limit = cfg.getDoubleValue("flch_controller_vs_change_limit");
    m_flch_controller_vs_cmd_limit = cfg.getDoubleValue("flch_controller_vs_cmd_limit");
    m_flch_controller_vs_cmd_diff_limit = cfg.getDoubleValue("flch_controller_vs_cmd_diff_limit");

    emit signalChanged();

    return true;
}

/////////////////////////////////////////////////////////////////////////////

void AircraftData::clear()
{
    m_max_zero_fuel_weight_kg = 0;
    m_max_takeoff_weight_kg = 0;
    m_max_landing_weight_kg = 0;

    m_max_speed_mach = 0;
    m_max_speed_kts = 0;

    m_nr_flap_notches = 0;
    m_flap_max_speed_list_kts.clear();
    m_flap_min_selectable_speed_factor_list.clear();
    
    m_max_gear_extend_altitude = 0;
    m_max_gear_extend_speed = 0;
    m_max_gear_retract_speed = 0;
    m_max_gear_extended_speed = 0;
    m_max_gear_extend_mach = 0;

    m_flap_stall_speed_map.clear();

    m_max_egt_temp_degrees = 0;
    m_max_n1_percent = 0;

    m_vref_factor = 0.0;
    m_v2_factor = 0.0;

    m_flaps_lever_to_stall_speed_weight_cache_map.clear();
    m_flaps_lever_to_stall_speed_cache_map.clear();

    m_idle_thrust = 10.0;

    m_checklist_filename.clear();

    m_throttle_controller_max_rate = 7.0;
    m_throttle_controller_trend_boost_factor = 4.0;
    m_throttle_controller_rate_factor = 13.0;

    m_speed_controller_max_trend = 2.0;
    m_speed_controller_trend_boost_factor = 12.0;
    m_speed_controller_wrong_direction_trend_boost_factor = 1.5;
    m_speed_controller_rate_factor = 14.0;

    m_flch_controller_ias_trend_factor = 10.0;
    m_flch_controller_n1_trend_factor = 5.5;
    m_flch_controller_ias_trend_target_limit = 15.0;
    m_flch_controller_vs_change_factor = 100.0;
    m_flch_controller_vs_change_limit = 250.0;
    m_flch_controller_vs_cmd_limit = 5000.0;
    m_flch_controller_vs_cmd_diff_limit = 1250.0;
}

/////////////////////////////////////////////////////////////////////////////

QString AircraftData::convertStallSpeedMapToString() const
{
    QString result;

    FlapNotchToMinMaxWeightToStallSpeedMap::const_iterator iter = m_flap_stall_speed_map.constBegin();
    for(; iter != m_flap_stall_speed_map.constEnd(); ++iter)
    {
        result += QString("%1:min(%2,%3)max(%4,%5);").
                  arg(iter.key()).
                  arg(iter.value().first.first).
                  arg(iter.value().first.second).
                  arg(iter.value().second.first).
                  arg(iter.value().second.second);
    }

    return result;
}

/////////////////////////////////////////////////////////////////////////////

bool AircraftData::convertStringToStallSpeedMap(const QString& string)
{
    QStringList flaps_list = string.split(";", QString::SkipEmptyParts);
    if (flaps_list.isEmpty()) return false;

    QRegExp regexp("(\\d+):min\\((\\d+),(\\d+)\\)max\\((\\d+),(\\d+)\\)");

    QStringList::const_iterator iter = flaps_list.constBegin();
    for(; iter != flaps_list.constEnd(); ++iter)
    {
        int pos = regexp.indexIn(*iter);

        if (pos < 0)
        {
            Logger::log(QString("AircraftData:convertStringToStallSpeedMap: "
                                "could not parse entry (%1) - aborting (%2)").arg(*iter).arg(pos));
            return false;
        }

//         Logger::log(QString("AircraftData:convertStringToStallSpeedMap: "
//                             "got min=%1/%2 max=%3/%4 for flap notch %5").
//                     arg(regexp.cap(2)).arg(regexp.cap(3)).
//                     arg(regexp.cap(4)).arg(regexp.cap(5)).
//                     arg(regexp.cap(1)));

        bool convok1 = false;
        bool convok2 = false;
        bool convok3 = false;
        bool convok4 = false;
        bool convok5 = false;

        m_flap_stall_speed_map[regexp.cap(1).toUInt(&convok1)] = 
            MinMaxWeightToSpeedTupleList(
                WeightToSpeedTuple(regexp.cap(2).toUInt(&convok2), regexp.cap(3).toUInt(&convok3)),
                WeightToSpeedTuple(regexp.cap(4).toUInt(&convok4), regexp.cap(5).toUInt(&convok5)));

        if (!convok1 || !convok2 || !convok3 || !convok4 || !convok5)
        {
            Logger::log(QString("AircraftData:convertStringToStallSpeedMap: "
                                "could not parse entry (%1) - encountered a non-integer value - aborting").arg(*iter));
            return false;
        }
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

double AircraftData::getStallSpeed(uint flap_lever_notch) const
{
    if (flap_lever_notch >= m_nr_flap_notches) return 0.0;

    // return the cached value if possible
    if (m_flaps_lever_to_stall_speed_cache_map.contains(flap_lever_notch) &&
        qAbs(m_flightstatus->total_weight_kg - m_flaps_lever_to_stall_speed_weight_cache_map[flap_lever_notch]) < 250)
    {
        return m_flaps_lever_to_stall_speed_cache_map[flap_lever_notch];
    }

    const WeightToSpeedTuple& min_speed_tuple = m_flap_stall_speed_map[flap_lever_notch].first;
    const WeightToSpeedTuple& max_speed_tuple = m_flap_stall_speed_map[flap_lever_notch].second;

    double k = ((double)max_speed_tuple.second - min_speed_tuple.second) /
               (max_speed_tuple.first - min_speed_tuple.first);

    double d = max_speed_tuple.second - (k * max_speed_tuple.first);

    double stall_speed = (k * m_flightstatus->total_weight_kg) + d;

    m_flaps_lever_to_stall_speed_cache_map[flap_lever_notch] = stall_speed;
    m_flaps_lever_to_stall_speed_weight_cache_map[flap_lever_notch] = m_flightstatus->total_weight_kg;
    return stall_speed;
}

/////////////////////////////////////////////////////////////////////////////

double AircraftData::getStallSpeed() const
{
    return getStallSpeed(m_flightstatus->current_flap_lever_notch);
}

/////////////////////////////////////////////////////////////////////////////

double AircraftData::getMaxSpeedKts() const
{
    double speed = m_max_speed_kts;

    // init with max. speed
    // take barber pole speed from fsaccess only when it's a sane value
    // otherwise take speed from config file
    if (m_flightstatus->barber_pole_speed > 0)
        speed = qMin(m_flightstatus->barber_pole_speed, (double)m_max_speed_kts);

    //TODO take max. mach of aircraft data into account

    // check max. flaps speed
    if (m_flightstatus->current_flap_lever_notch < m_nr_flap_notches)
        speed = qMin(speed, getMaxFlapsSpeed(m_flightstatus->current_flap_lever_notch));

    //  check max. gear speed
    if (m_flightstatus->isGearDown()) 
        speed = qMin(speed, (double)m_max_gear_extended_speed);

    return speed;
}

/////////////////////////////////////////////////////////////////////////////

double AircraftData::getMaxFlapsSpeed(uint flap_level_notch) const
{
    if (flap_level_notch >= m_nr_flap_notches) return 0.0;
    return m_flap_max_speed_list_kts[flap_level_notch];
}

/////////////////////////////////////////////////////////////////////////////

double AircraftData::getMinimumSelectableSpeed() const
{
    if (m_flightstatus->current_flap_lever_notch >= m_nr_flap_notches) return 0.0;    
    return m_flap_min_selectable_speed_factor_list[m_flightstatus->current_flap_lever_notch]  * getStallSpeed();
}

/////////////////////////////////////////////////////////////////////////////

double AircraftData::getMinimumSelectableSpeed(uint flap_lever_notch) const
{
    if (flap_lever_notch >= m_nr_flap_notches) return 0.0;
    return m_flap_min_selectable_speed_factor_list[flap_lever_notch]  * getStallSpeed(flap_lever_notch);
}

/////////////////////////////////////////////////////////////////////////////

double AircraftData::getMinimumAStyleFlapsRetractionSpeed() const
{
    return m_astyle_min_flaps_retraction_factor * getStallSpeed(2);
}

/////////////////////////////////////////////////////////////////////////////

double AircraftData::getMinimumAStyleSlatsRetractionSpeed() const
{
    return m_astyle_min_slats_retraction_factor * getStallSpeed(0);
}

/////////////////////////////////////////////////////////////////////////////

double AircraftData::getGreenDotSpeed() const
{
    return 85.0 + (2.0 * m_flightstatus->total_weight_kg / 1000.0) +
        qMax(0.0, (m_flightstatus->alt_ft - 20000.0) / 1000.0);
}

/////////////////////////////////////////////////////////////////////////////

double AircraftData::getVref(uint flap_lever_notch) const
{
    return m_vref_factor * getStallSpeed(flap_lever_notch);
}

/////////////////////////////////////////////////////////////////////////////

double AircraftData::getV2(uint flap_lever_notch) const
{
    return m_v2_factor * getStallSpeed(flap_lever_notch);
}

/////////////////////////////////////////////////////////////////////////////

// End of file
