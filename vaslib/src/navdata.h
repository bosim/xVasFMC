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

#ifndef NAVDATA_H
#define NAVDATA_H

#include <QFile>
#include <QObject>
#include <QStringList>
#include <QString>
#include <QDomDocument>

#include "config.h"
#include "assert.h"
#include "airway.h"
#include "waypoint.h"
#include "airport.h"
#include "runway.h"
#include "intersection.h"
#include "waypoint_hdg_to_alt.h"
#include "waypoint_hdg_to_intercept.h"
#include "ils.h"
#include "sid.h"
#include "star.h"
#include "transition.h"
#include "approach.h"

typedef QMap<QString, long> IndexMap;

typedef QList<Airport> AirportList;
typedef QMap<QString, AirportList> AirportListCoordinateIndexMap;

typedef QList<Vor> VorList;
typedef QMap<QString, VorList> VorListCoordinateIndexMap;

typedef QList<Ndb> NdbList;
typedef QMap<QString, NdbList> NdbListCoordinateIndexMap;

class QDomElement;

/////////////////////////////////////////////////////////////////////////////

class Navdata : public QObject
{
    Q_OBJECT

public:

    Navdata(const QString& navdata_config_filename, const QString& navdata_index_config_filename);
    virtual ~Navdata();

    void setupDefaultConfig();

    inline Config* config() { return m_navdata_config; }

    inline bool isValid() const { return m_valid; }

    inline const QString& getAiracCycleTitle() const { return m_airac_cycle_title; }
    inline const QString& getAiracCycleDates() const { return m_airac_cycle_dates; }

    //! the given list will be cleared first
    uint getAirways(const QString& airway_name, AirwayPtrList& airways) const;

    //! the given list will be cleared on error
    uint getAirports(const QString& name, WaypointPtrList& airports) const;

    //! the given list will *not* be cleared
    uint getIntersections(const QString& name, WaypointPtrList& waypoints) const;

    //! the given list will *not* be cleared
    uint getNavaids(const QString& id,
                    WaypointPtrList& waypoints,
                    const QString& wanted_country_code = QString::null,
                    const QString& wanted_type = Waypoint::TYPE_ALL) const;

    //! Searches for a waypoints of all kinds (intersections, navaids, airports, etc.)
    //! with the given name and returns the number of found entries.
    uint getWaypoints(const QString& wpt_name,
                      WaypointPtrList& result_list,
                      const QRegExp& lat_lon_regexp) const;

    //! Searches for an airway from "from_waypoint" to "to_waypoint" and adds all waypoints to
    //! "result_wpt_list". Returns true on success, false otherwise.
    bool getWaypointsByAirway(const Waypoint& from_waypoint,
                              const QString& airway,
                              const QString& to_waypoint,
                              WaypointPtrList& result_wpt_list,
                              QString& error_text) const;

    //! Will parse the given LAT/LON string and will return an intersection with the given data.
    //! Will return NULL on error.
    //! ATTENTION: The calling method will be responsible to delete the returned pointer.
    Waypoint* getIntersectionFromLatLonString(const QString& id,
                                              const QString& latlon_string,
                                              const QRegExp& regexp) const;


    //! The caller is responsible to the delete the returned pointer!
    Waypoint* getElementsWithSignal(const QString& id,
                                    const QString& wanted_country_code = QString::null,
                                    const QString& wanted_type = Waypoint::TYPE_ALL);

    uint getSids(const QString& airport, ProcedurePtrList& procedures) const;
    uint getStars(const QString& airport, ProcedurePtrList& procedures) const;
    uint getApproaches(const QString& airport, ProcedurePtrList& procedures) const;

    //----- direct access

    //! The caller is responsible to the delete the returned pointer!
    Vor* getNavaid(const QString& id, const QString& type, const QString& country_code);

    //! The caller is responsible to the delete the returned pointer!
    Vor* getNdb(const QString& id, const QString& country_code);

    //! The caller is responsible to the delete the returned pointer!
    Vor* getVor(const QString& id, const QString& country_code);

    //! The caller is responsible to the delete the returned pointer!
    Airport* getAirport(const QString& id);

    //! The caller is responsible to the delete the returned pointer!
    Intersection* getIntersection(const QString& id, const QString& country_code);

    //----- coordinate access

    //! Adds airport matching the given LAT/LON values to the given list.
    //! "variation" is used to span a search rectangle based on the given LAT/LON values.
    //! ATTENTION: The given list will *not* be cleared.
    uint getAirportListByCoordinates(const Waypoint& current_position,
                                     int variation,
                                     uint max_distance_nm,
                                     WaypointPtrList& airports);

    //! Adds Vor matching the given LAT/LON values to the given list.
    //! "variation" is used to span a search rectangle based on the given LAT/LON values.
    //! ATTENTION: The given list will *not* be cleared.
    uint getVorListByCoordinates(const Waypoint& current_position,
                                 int variation,
                                 uint max_distance_nm,
                                 WaypointPtrList& vors);

    //! Adds Ndb matching the given LAT/LON values to the given list.
    //! "variation" is used to span a search rectangle based on the given LAT/LON values.
    //! ATTENTION: The given list will *not* be cleared.
    uint getNdbListByCoordinates(const Waypoint& current_position,
                                 int variation,
                                 uint max_distance_nm,
                                 WaypointPtrList& ndbs);

signals:

    void signalWaypointChoose(const WaypointPtrList& waypoint_list, Waypoint** waypoint_to_insert);

protected:

    bool setupFiles();
    bool extractAiracCycle();
    //! returns true when successfull, false otherwise
    bool setupIndexes();

    bool isOnCaseSensitiveFilesystem() const;
    void renameNavdataFilenamesToLower(const QString& relative_path) const;

    QString serializeIndexMap(const IndexMap& map);
    QString serializeAirportCoordinateIndexMap(const AirportListCoordinateIndexMap& map);
    QString serializeVorCoordinateIndexMap(const VorListCoordinateIndexMap& map);
    QString serializeNdbCoordinateIndexMap(const NdbListCoordinateIndexMap& map);

    IndexMap deSerializeIndexMap(const QString& line);
    AirportListCoordinateIndexMap deSerializeAirportCoordinateIndexMap(const QString& line);
    VorListCoordinateIndexMap deSerializeVorCoordinateIndexMap(const QString& line);
    NdbListCoordinateIndexMap deSerializeNdbCoordinateIndexMap(const QString& line);

    bool generateIndex(QFile* file_to_read, IndexMap& index_map, bool generate_navaid_coordinate_index = false);
    bool indexWaypoints();
    bool indexAirways();
    bool indexAirports();
    bool indexNavaids();

    inline QString getCoordinateIndex(const Waypoint& waypoint)
    { return QString("%1/%2").arg(waypoint.lat(), 0, 'f', 0).arg(waypoint.lon(), 0, 'f', 0); }
    inline QString getCoordinateIndex(const double& lat, const double& lon)
    { return QString("%1/%2").arg(lat, 0, 'f', 0).arg(lon, 0, 'f', 0); }

    bool parseAirwayRoute(const QString& line, QString& airway_name, int& segment_count) const;

    bool parseAirport(const QString& line, const QStringList& runway_lines, Airport* airport) const;

    //! ATTENTION: the caller is responsible to delete the returned navaid
    Waypoint* parseNavaid(const QString& line,
                          const QString& wanted_id = QString::null,
                          const QString& wanted_country_code = QString::null,
                          const QString& wanted_type = Waypoint::TYPE_ALL) const;

    bool parseAirwayRouteSegment(const QString& line,
                                 Waypoint& waypoint1,
                                 Waypoint& waypoint2,
                                 int& inbound_course,
                                 int& outbound_course,
                                 double& distance) const;

    void normalizeID(QString& id) const;

    //----- F1 procedures

    uint getProcedures(const QString& airport,
                       const QString& wanted_type,
                       ProcedurePtrList& procedures) const;

    Procedure* parseProcedure(const QString& line, const QString& wanted_type) const;

    bool parseProcedureWaypoint(const QString& line, Waypoint& parsed_wpt) const;

    //----- Level-D procedures

    uint getLevelDProcedures(const QString& airport,
                             const QString& wanted_type,
                             ProcedurePtrList& procedures) const;


    Procedure* parseLevelDProcedure(const QDomElement& element, const QString& wanted_type) const;

    Waypoint* parseLevelDProcedureWaypoint(const QDomElement& element,
                                           Procedure& procedure) const;

protected:

    bool m_valid;

    Config *m_navdata_config;
    Config *m_navdata_index_config;

    QString m_airac_cycle_title;
    QString m_airac_cycle_dates;

    QFile* m_waypoint_file;
    IndexMap m_waypoint_index_map;

    QFile* m_airway_file;
    IndexMap m_airway_index_map;

    QFile* m_airport_file;
    IndexMap m_airport_index_map;
    AirportListCoordinateIndexMap m_airport_list_coordinate_index_map;

    QFile* m_navaid_file;
    IndexMap m_navaid_index_map;
    VorListCoordinateIndexMap m_vor_list_coordinate_index_map;
    NdbListCoordinateIndexMap m_ndb_list_coordinate_index_map;

    //! airport ICAO to level-d DOM object map
    mutable QMap<QString, QDomDocument> m_airport_to_leveld_procedure_chache_map;

private:

};

#endif
