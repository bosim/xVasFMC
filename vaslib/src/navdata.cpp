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

#include <QApplication>
#include <QByteArray>
#include <QStringList>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QChar>
#include <QDateTime>
#include <QMessageBox>

#include <QDomElement>

#include "logger.h"
#include "navcalc.h"
#include "vas_path.h"

#include "navdata.h"

/////////////////////////////////////////////////////////////////////////////

#define CFG_AIRAC_CYCLE_TITLE "airactitle"
#define CFG_AIRAC_CYCLE_DATES "airacdates"

#define CFG_WAYPOINT_FILENAME "waypointfile"
#define CFG_AIRWAY_FILENAME "airwayfile"
#define CFG_AIRPORT_FILENAME "airportfile"
#define CFG_NAVAID_FILENAME "navaidfile"

#define CFG_WAYPOINT_INDEX "waypointindex"
#define CFG_AIRWAY_INDEX "airwayindex"
#define CFG_AIRPORT_INDEX "airportindex"
#define CFG_NAVAID_INDEX "navaidindex"

#define CFG_AIRPORT_COORDINATE_INDEX "airportcoordinateindex"
#define CFG_VOR_COORDINATE_INDEX "vorcoordinateindex"
#define CFG_NDB_COORDINATE_INDEX "ndbcoordinateindex"

#define CFG_SID_SUBDIR "sidsubdir"
#define CFG_STAR_SUBDIR "starsubdir"
#define CFG_LEVELD_PROCEDURES_SUBDIR "level_procedures_subdir"

/////////////////////////////////////////////////////////////////////////////

#define AIRAC_WAYPOINT_FILENAME_DEFAULT "navdata/waypoints.txt"
#define AIRAC_AIRWAY_FILENAME_DEFAULT "navdata/ats.txt"
#define AIRAC_AIRPORT_FILENAME_DEFAULT "navdata/airports.txt"
#define AIRAC_NAVAID_FILENAME_DEFAULT "navdata/navaids.txt"
#define AIRAC_SID_SUBDIR_DEFAULT "navdata/sid"
#define AIRAC_STAR_SUBDIR_DEFAULT "navdata/star"
#define AIRAC_LEVELD_PROCEDURES_SUBDIR_DEFAULT "navdata/leveld_proc"

/////////////////////////////////////////////////////////////////////////////

#define AIRAC_CYCLE_RECORD_PREFIX 'X'
#define AIRPORT_RECORD_PREFIX 'A'
#define RUNWAY_RECORD_PREFIX 'R'
#define AIRWAY_ROUTE_PREFIX 'A'
#define AIRWAY_ROUTE_SEGMENT_PREFIX 'S'
#define PROCEDURE_RECORD_PREFIX 'P'
#define PROCEDURE_LEG_RECORD_PREFIX 'S'

/////////////////////////////////////////////////////////////////////////////

// waypoint item indices
#define WPT_ID_INDEX 0
#define WPT_LAT_INDEX 1
#define WPT_LON_INDEX 2
#define WPT_CCODE_INDEX 3

//navaid item indices
#define NAVAID_ID_INDEX 0
#define NAVAID_NAME_INDEX 1
#define NAVAID_FREQ_INDEX 2
#define NAVAID_VORFLAG_INDEX 3
#define NAVAID_DMEFLAG_INDEX 4
#define NAVAID_RANGE_INDEX 5
#define NAVAID_LAT_INDEX 6
#define NAVAID_LON_INDEX 7
#define NAVAID_ELEVATION_INDEX 8
#define NAVAID_CCODE_INDEX 9

//airway item indices
#define AIRWAY_PREFIX_INDEX 0
#define AIRWAY_NAME_INDEX 1
#define AIRWAY_SEGCOUNT_INDEX 2

//airway segement item indices
#define AIRWAY_SEGMENT_PREFIX_INDEX 0
#define AIRWAY_SEGMENT_NAME1_INDEX 1
#define AIRWAY_SEGMENT_LAT1_INDEX 2
#define AIRWAY_SEGMENT_LON1_INDEX 3
#define AIRWAY_SEGMENT_NAME2_INDEX 4
#define AIRWAY_SEGMENT_LAT2_INDEX 5
#define AIRWAY_SEGMENT_LON2_INDEX 6
#define AIRWAY_SEGMENT_INCOURSE_INDEX 7
#define AIRWAY_SEGMENT_OUTCOURSE_INDEX 8
#define AIRWAY_SEGMENT_DIST_INDEX 8

//airport item indices
#define AIRPORT_PREFIX_INDEX 0
#define AIRPORT_ID_INDEX 1
#define AIRPORT_NAME_INDEX 2
#define AIRPORT_LAT_INDEX 3
#define AIRPORT_LON_INDEX 4
#define AIRPORT_ELEV_INDEX 5

//runway item indices
#define RUNWAY_PREFIX_INDEX 0
#define RUNWAY_ID_INDEX 1
#define RUNWAY_HDG_INDEX 2
#define RUNWAY_LENGTH_INDEX 3
#define RUNWAY_ILSFLAG_INDEX 4
#define RUNWAY_ILSFREQ_INDEX 5
#define RUNWAY_ILSHDG_INDEX 6
#define RUNWAY_LAT_INDEX 7
#define RUNWAY_LON_INDEX 8
#define RUNWAY_ELEV_INDEX 9
#define RUNWAY_GSANGLE_INDEX 10
#define RUNWAY_THROFLYHEIGHT_INDEX 11

//procedure item indices
#define PROCEDURE_PREFIX_INDEX 0
#define PROCEDURE_ID_INDEX 1
#define PROCEDURE_RUNWAY_INDEX 2
#define PROCEDURE_TRANSITION_INDEX 3
#define PROCEDURE_DEFAULTFLAG_INDEX 4
#define PROCEDURE_LEGCOUNT_INDEX 5

//procedue leg item indices
#define PROCEDURE_LEG_PREFIX_INDEX 0
#define PROCEDURE_LEG_WPTID_INDEX 1
#define PROCEDURE_LEG_WPTLAT_INDEX 2
#define PROCEDURE_LEG_WPTLON_INDEX 3
#define PROCEDURE_LEG_WPTNAME_INDEX 4
#define PROCEDURE_LEG_PROCTYPE_INDEX 5
#define PROCEDURE_LEG_TURNDIR_INDEX 6
#define PROCEDURE_LEG_HDG_BEARING_INDEX 7
#define PROCEDURE_LEG_NAVAID_DIST_INDEX 8
#define PROCEDURE_LEG_NAVAID_BEARING_INDEX 9
#define PROCEDURE_LEG_DIST_INDEX 10
#define PROCEDURE_LEG_SPEED_RESTRICTION_INDEX 11
#define PROCEDURE_LEG_ALT_RESTRICTION_INDEX 12
#define PROCEDURE_LEG_ALT_A_INDEX 13
#define PROCEDURE_LEG_ALT_B_INDEX 14
#define PROCEDURE_LEG_OVFLYWPTFLAG_INDEX 15
#define PROCEDURE_LEG_IAFFIX_FLAG_INDEX 16
#define PROCEDURE_LEG_FAFFIX_FLAG_INDEX 17
#define PROCEDURE_LEG_MAPFIX_FLAG_INDEX 18

/////////////////////////////////////////////////////////////////////////////

#define NDSEP "|"
#define INDEX_ITEM_SEP '@'
#define INDEX_TUPLE_SEP ','
#define COORD_FACTOR 1000000.0
#define DIST_FACTOR 100.0
#define FT_TO_M 0.3048
#define PROCEDURE_FILE_EXT ".txt"
#define LEVELD_PROCEDURE_FILE_EXT ".xml"

/////////////////////////////////////////////////////////////////////////////

Navdata::Navdata(const QString& navdata_config_filename, const QString& navdata_index_config_filename) :
    m_valid(false), m_navdata_config(0), m_navdata_index_config(0),
    m_waypoint_file(0), m_airway_file(0), m_airport_file(0), m_navaid_file(0)
{
    Logger::log("Navdata: init");

    // init navdata config

    m_navdata_config = new Config(navdata_config_filename);
    MYASSERT(m_navdata_config);

    // init navdata index config

    m_navdata_index_config = new Config(navdata_index_config_filename);
    MYASSERT(m_navdata_index_config);

    // setup configs

    setupDefaultConfig();
    m_navdata_config->loadfromFile();
    m_navdata_index_config->loadfromFile();

    // setup database

    bool file_setup_done = false;

    // 1st try
    if (!setupFiles())
    {
        Logger::log("Navdata: no files found - renaming to lower case");
        renameNavdataFilenamesToLower(VasPath::prependPath("navdata/"));

        // 2nd try
        if (!setupFiles())
        {
            Logger::log("Navdata: no files found");
#if! VASFMC_GAUGE
            QMessageBox::critical(0, "Navdata init", "Cannot find navigational (AIRAC) data.");
#endif
        }
        else
        {
            file_setup_done = true;
        }
    }
    else
    {
        file_setup_done = true;
    }

    if (!file_setup_done)
    {
        Logger::log("Navdata: could not init navdata - giving up");
        return;
    }

    MYASSERT(extractAiracCycle());
    MYASSERT(setupIndexes());
    m_valid = true;
    m_navdata_config->saveToFile();
    m_navdata_index_config->saveToFile();
    Logger::log("Navdata: init complete");
};

/////////////////////////////////////////////////////////////////////////////

void Navdata::setupDefaultConfig()
{
    MYASSERT(m_navdata_config != 0);
    m_navdata_config->setValue(CFG_AIRPORT_FILENAME, AIRAC_AIRPORT_FILENAME_DEFAULT);
    m_navdata_config->setValue(CFG_WAYPOINT_FILENAME, AIRAC_WAYPOINT_FILENAME_DEFAULT);
    m_navdata_config->setValue(CFG_AIRWAY_FILENAME, AIRAC_AIRWAY_FILENAME_DEFAULT);
    m_navdata_config->setValue(CFG_NAVAID_FILENAME, AIRAC_NAVAID_FILENAME_DEFAULT);
    m_navdata_config->setValue(CFG_SID_SUBDIR, AIRAC_SID_SUBDIR_DEFAULT);
    m_navdata_config->setValue(CFG_STAR_SUBDIR, AIRAC_STAR_SUBDIR_DEFAULT);
    m_navdata_config->setValue(CFG_LEVELD_PROCEDURES_SUBDIR, AIRAC_LEVELD_PROCEDURES_SUBDIR_DEFAULT);

    MYASSERT(m_navdata_index_config != 0);
    m_navdata_index_config->setValue(CFG_AIRAC_CYCLE_TITLE, "");
    m_navdata_index_config->setValue(CFG_AIRAC_CYCLE_DATES, "");
    m_navdata_index_config->setValue(CFG_AIRPORT_INDEX, "");
    m_navdata_index_config->setValue(CFG_WAYPOINT_INDEX, "");
    m_navdata_index_config->setValue(CFG_AIRWAY_INDEX, "");
    m_navdata_index_config->setValue(CFG_NAVAID_INDEX, "");
}

/////////////////////////////////////////////////////////////////////////////

bool Navdata::setupFiles()
{
    // airport file

    m_airport_file = new QFile(VasPath::prependPath(m_navdata_config->getValue(CFG_AIRPORT_FILENAME)));
    MYASSERT(m_airport_file);

    if (!m_airport_file->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Logger::log(QString("Navdata:setupFiles: Could not open airport file %1").
                    arg(m_airport_file->fileName()));

#if! VASFMC_GAUGE
        QMessageBox::critical(0, "Navdata init",
                              QString("Could not open airport file %1").
                              arg(m_airport_file->fileName()));
#endif
        return false;
    }

    // waypoint file

    m_waypoint_file = new QFile(VasPath::prependPath(m_navdata_config->getValue(CFG_WAYPOINT_FILENAME)));
    MYASSERT(m_waypoint_file);

    if (!m_waypoint_file->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Logger::log(QString("Navdata:setupFiles: Could not open waypoint file %1").
                    arg(m_waypoint_file->fileName()));

#if! VASFMC_GAUGE
        QMessageBox::critical(0, "Navdata init",
                              QString("Could not open waypoint file %1").
                              arg(m_waypoint_file->fileName()));
#endif
        return false;
    }

    // airway file

    m_airway_file = new QFile(VasPath::prependPath(m_navdata_config->getValue(CFG_AIRWAY_FILENAME)));
    MYASSERT(m_airway_file);

    if (!m_airway_file->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Logger::log(QString("Navdata:setupFiles: Could not open airway file %1").
                    arg(m_airway_file->fileName()));

#if! VASFMC_GAUGE
        QMessageBox::critical(0, "Navdata init",
                              QString("Could not open airway file %1").
                              arg(m_airway_file->fileName()));
#endif
        return false;
    }

    // navaid file

    m_navaid_file = new QFile(VasPath::prependPath(m_navdata_config->getValue(CFG_NAVAID_FILENAME)));
    MYASSERT(m_navaid_file);

    if (!m_navaid_file->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Logger::log(QString("Navdata:setupFiles: Could not open navaid file %1").
                    arg(m_navaid_file->fileName()));

#if! VASFMC_GAUGE
        QMessageBox::critical(0, "Navdata init",
                              QString("Could not open navaid file %1").
                              arg(m_navaid_file->fileName()));
#endif
        return false;
    }

    return true;
};

/////////////////////////////////////////////////////////////////////////////

bool Navdata::extractAiracCycle()
{
    MYASSERT(m_airport_file->reset());
    MYASSERT(!m_airport_file->atEnd());

    QByteArray line_array = m_airport_file->readLine();
    QString line(line_array);
    line = line.trimmed().toUpper();

    Logger::log(QString("Navdata:extractAiracCycle: (%1)").arg(line.trimmed()));

    QStringList airac_items = line.split(NDSEP);
    MYASSERT(airac_items.count() >=5);
    MYASSERT(airac_items[0].at(0) == AIRAC_CYCLE_RECORD_PREFIX);

    m_airac_cycle_title = airac_items[1];
    m_airac_cycle_dates = airac_items[2];
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool Navdata::setupIndexes()
{
    bool config_airac_matches = false;

    // if the airac data in the config match the airac data files, we
    // will load the indexes from the config for faster startup

    if (m_navdata_index_config->getValue(CFG_AIRAC_CYCLE_TITLE) == m_airac_cycle_title &&
        m_navdata_index_config->getValue(CFG_AIRAC_CYCLE_DATES) == m_airac_cycle_dates)
    {
        Logger::log("Navdata:setupIndexes: config airac title and data matched");

        if (m_navdata_index_config->getValue(CFG_AIRPORT_INDEX).isEmpty() ||
            m_navdata_index_config->getValue(CFG_WAYPOINT_INDEX).isEmpty() ||
            m_navdata_index_config->getValue(CFG_AIRWAY_INDEX).isEmpty() ||
            m_navdata_index_config->getValue(CFG_NAVAID_INDEX).isEmpty())
        {
            Logger::log("Navdata:setupIndexes: config airac index empty");
            config_airac_matches = false;
        }
        else
        {
            Logger::log("Navdata:setupIndexes: config airac index ok");
            config_airac_matches = true;
        }
    }

    if (!config_airac_matches)
    {
        Logger::log("Navdata:setupIndexes: config airac not ok");

        m_navdata_index_config->setValue(CFG_AIRAC_CYCLE_TITLE, m_airac_cycle_title);
        m_navdata_index_config->setValue(CFG_AIRAC_CYCLE_DATES, m_airac_cycle_dates);

        MYASSERT(indexAirports());
        MYASSERT(indexWaypoints());
        MYASSERT(indexAirways());
        MYASSERT(indexNavaids());

        m_navdata_index_config->setValue(CFG_AIRPORT_INDEX, serializeIndexMap(m_airport_index_map));
        m_navdata_index_config->setValue(CFG_WAYPOINT_INDEX, serializeIndexMap(m_waypoint_index_map));
        m_navdata_index_config->setValue(CFG_AIRWAY_INDEX, serializeIndexMap(m_airway_index_map));
        m_navdata_index_config->setValue(CFG_NAVAID_INDEX, serializeIndexMap(m_navaid_index_map));

        m_navdata_index_config->setValue(
            CFG_AIRPORT_COORDINATE_INDEX, serializeAirportCoordinateIndexMap(m_airport_list_coordinate_index_map));
        m_navdata_index_config->setValue(
            CFG_VOR_COORDINATE_INDEX, serializeVorCoordinateIndexMap(m_vor_list_coordinate_index_map));
        m_navdata_index_config->setValue(
            CFG_NDB_COORDINATE_INDEX, serializeNdbCoordinateIndexMap(m_ndb_list_coordinate_index_map));
    }
    else
    {
        m_airport_index_map = deSerializeIndexMap(m_navdata_index_config->getValue(CFG_AIRPORT_INDEX));
        m_waypoint_index_map = deSerializeIndexMap(m_navdata_index_config->getValue(CFG_WAYPOINT_INDEX));
        m_airway_index_map = deSerializeIndexMap(m_navdata_index_config->getValue(CFG_AIRWAY_INDEX));
        m_navaid_index_map = deSerializeIndexMap(m_navdata_index_config->getValue(CFG_NAVAID_INDEX));

        m_airport_list_coordinate_index_map =
            deSerializeAirportCoordinateIndexMap(m_navdata_index_config->getValue(CFG_AIRPORT_COORDINATE_INDEX));
        m_vor_list_coordinate_index_map =
            deSerializeVorCoordinateIndexMap(m_navdata_index_config->getValue(CFG_VOR_COORDINATE_INDEX));
        m_ndb_list_coordinate_index_map =
            deSerializeNdbCoordinateIndexMap(m_navdata_index_config->getValue(CFG_NDB_COORDINATE_INDEX));
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////

QString Navdata::serializeIndexMap(const IndexMap& map)
{
    QString result_string;

    IndexMap::const_iterator iter = map.begin();
    for(; iter != map.end(); ++iter)
    {
        result_string += QString("%1%2%3%4").arg(iter.key()).arg(INDEX_ITEM_SEP).
                         arg(*iter).arg(INDEX_TUPLE_SEP);
    }

    return result_string;
};

/////////////////////////////////////////////////////////////////////////////

QString Navdata::serializeAirportCoordinateIndexMap(const AirportListCoordinateIndexMap& map)
{
    QString result_string;

    AirportListCoordinateIndexMap::const_iterator iter = map.begin();
    for(; iter != map.end(); ++iter)
    {
        const AirportList& airportlist = iter.value();

        AirportList::const_iterator list_iter = airportlist.begin();
        for(; list_iter != airportlist.end(); ++list_iter)
        {
            const Airport& airport = *list_iter;
            result_string += QString("%1%2%3%4%5%6").arg(airport.id()).arg(INDEX_ITEM_SEP).
                             arg(airport.lat()).arg(INDEX_ITEM_SEP).arg(airport.lon()).arg(INDEX_TUPLE_SEP);
        }
    }

    return result_string;
}

/////////////////////////////////////////////////////////////////////////////

QString Navdata::serializeVorCoordinateIndexMap(const VorListCoordinateIndexMap& map)
{
    QString result_string;

    VorListCoordinateIndexMap::const_iterator iter = map.begin();
    for(; iter != map.end(); ++iter)
    {
        const VorList& vorlist = iter.value();

        VorList::const_iterator list_iter = vorlist.begin();
        for(; list_iter != vorlist.end(); ++list_iter)
        {
            const Vor& vor = *list_iter;
            result_string += QString("%1%2%3%4%5%6%7%8").arg(vor.id()).arg(INDEX_ITEM_SEP).
                             arg(vor.lat()).arg(INDEX_ITEM_SEP).arg(vor.lon()).
                             arg(INDEX_ITEM_SEP).arg(vor.hasDME()).arg(INDEX_TUPLE_SEP);
        }
    }

    return result_string;
}

/////////////////////////////////////////////////////////////////////////////

QString Navdata::serializeNdbCoordinateIndexMap(const NdbListCoordinateIndexMap& map)
{
    QString result_string;

    NdbListCoordinateIndexMap::const_iterator iter = map.begin();
    for(; iter != map.end(); ++iter)
    {
        const NdbList& ndblist = iter.value();

        NdbList::const_iterator list_iter = ndblist.begin();
        for(; list_iter != ndblist.end(); ++list_iter)
        {
            const Ndb& ndb = *list_iter;
            result_string += QString("%1%2%3%4%5%6").arg(ndb.id()).arg(INDEX_ITEM_SEP).
                             arg(ndb.lat()).arg(INDEX_ITEM_SEP).arg(ndb.lon()).arg(INDEX_TUPLE_SEP);
        }
    }

    return result_string;
}

/////////////////////////////////////////////////////////////////////////////

IndexMap Navdata::deSerializeIndexMap(const QString& line)
{
    MYASSERT(!line.isEmpty());
    IndexMap result_map;

    QStringList tuple_list = line.split(INDEX_TUPLE_SEP, QString::SkipEmptyParts);
    MYASSERT(tuple_list.count() > 0);

    QStringList::const_iterator iter = tuple_list.begin();
    for(; iter != tuple_list.end(); ++iter)
    {
        QStringList item_list = (*iter).split(INDEX_ITEM_SEP);
        MYASSERT(item_list.count() == 2);

        bool convok = false;
        result_map.insert(item_list[0], item_list[1].toLong(&convok));
        MYASSERT(convok);
    }

    return result_map;
};

/////////////////////////////////////////////////////////////////////////////

AirportListCoordinateIndexMap Navdata::deSerializeAirportCoordinateIndexMap(const QString& line)
{
    MYASSERT(!line.isEmpty());
    AirportListCoordinateIndexMap result_map;

    QStringList tuple_list = line.split(INDEX_TUPLE_SEP, QString::SkipEmptyParts);
    MYASSERT(tuple_list.count() > 0);

    QStringList::const_iterator iter = tuple_list.begin();
    for(; iter != tuple_list.end(); ++iter)
    {
        QStringList item_list = (*iter).split(INDEX_ITEM_SEP);
        MYASSERT(item_list.count() == 3);

        bool convok = false;
        double lat = item_list[1].toDouble(&convok);
        MYASSERT(convok);
        double lon = item_list[2].toDouble(&convok);
        MYASSERT(convok);

        Airport airport(item_list[0], "", lat, lon, 0);
        result_map[getCoordinateIndex(airport)].append(airport);
    }

    return result_map;
}

/////////////////////////////////////////////////////////////////////////////

VorListCoordinateIndexMap Navdata::deSerializeVorCoordinateIndexMap(const QString& line)
{
    MYASSERT(!line.isEmpty());
    VorListCoordinateIndexMap result_map;

    QStringList tuple_list = line.split(INDEX_TUPLE_SEP, QString::SkipEmptyParts);
    MYASSERT(tuple_list.count() > 0);

    QStringList::const_iterator iter = tuple_list.begin();
    for(; iter != tuple_list.end(); ++iter)
    {
        QStringList item_list = (*iter).split(INDEX_ITEM_SEP);
        MYASSERT(item_list.count() == 4);

        bool convok = false;
        double lat = item_list[1].toDouble(&convok);
        MYASSERT(convok);
        double lon = item_list[2].toDouble(&convok);
        MYASSERT(convok);
        bool dme = item_list[3].toInt(&convok);
        MYASSERT(convok);

        Vor vor(item_list[0], "", lat, lon, 0, dme, 0, 0, QString::null);
        result_map[getCoordinateIndex(vor)].append(vor);
    }

    return result_map;
}

/////////////////////////////////////////////////////////////////////////////

NdbListCoordinateIndexMap Navdata::deSerializeNdbCoordinateIndexMap(const QString& line)
{
    MYASSERT(!line.isEmpty());
    NdbListCoordinateIndexMap result_map;

    QStringList tuple_list = line.split(INDEX_TUPLE_SEP, QString::SkipEmptyParts);
    MYASSERT(tuple_list.count() > 0);

    QStringList::const_iterator iter = tuple_list.begin();
    for(; iter != tuple_list.end(); ++iter)
    {
        QStringList item_list = (*iter).split(INDEX_ITEM_SEP);
        MYASSERT(item_list.count() == 3);

        bool convok = false;
        double lat = item_list[1].toDouble(&convok);
        MYASSERT(convok);
        double lon = item_list[2].toDouble(&convok);
        MYASSERT(convok);

        Ndb ndb(item_list[0], "", lat, lon, 0, 0, 0, QString::null);
        result_map[getCoordinateIndex(ndb)].append(ndb);
    }

    return result_map;
}

/////////////////////////////////////////////////////////////////////////////

Navdata::~Navdata()
{
    delete m_waypoint_file;
    delete m_airway_file;
    delete m_airport_file;
    delete m_navaid_file;
};

/////////////////////////////////////////////////////////////////////////////
////////////////////////// INDEX METHODS ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

bool Navdata::generateIndex(QFile* file_to_read, IndexMap& index_map, bool generate_navaid_coordinate_index)
{
    QList<QChar> current_index;
    current_index.append(QChar('*'));
    current_index.append(QChar('*'));

    unsigned long linecount = 0;

    while(!file_to_read->atEnd())
    {
        qint64 read_pos = file_to_read->pos();

        QByteArray line_array = file_to_read->readLine();
        ++linecount;
        QString line(line_array);
        line = line.trimmed();
        if (line.isEmpty()) continue;
        line = line.toUpper();

        if (line.at(0) != current_index[0])
        {
            current_index[0] = line.at(0);
            index_map.insert(current_index[0], read_pos);

            current_index[1] = '*';
            if (line.length() > 1)
            {
                current_index[1] = line.at(1);
                index_map.insert(QString(current_index[0])+current_index[1], read_pos);
            }
        }
        else if (line.length() > 1 && line.at(1) != current_index[1])
        {
            current_index[1] = line.at(1);
            index_map.insert(QString(current_index[0])+current_index[1], read_pos);
        }

        // create VOR&NDB coordinate index
        if (generate_navaid_coordinate_index)
        {
            Waypoint* navaid = parseNavaid(line, QString::null);

            if (navaid != 0)
            {
                QString coordinate_index = getCoordinateIndex(*navaid);

                if (navaid->asIls() != 0) {}
                else if (navaid->asVor() != 0)
                    m_vor_list_coordinate_index_map[coordinate_index].append(*navaid->asVor());
                else if (navaid->asNdb() != 0)
                    m_ndb_list_coordinate_index_map[coordinate_index].append(*navaid->asNdb());

                delete navaid;
                navaid = 0;
            }
        }
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool Navdata::indexWaypoints()
{
    m_valid = false;
    MYASSERT(m_waypoint_file->reset());
    m_waypoint_index_map.clear();

    bool ret = generateIndex(m_waypoint_file, m_waypoint_index_map);

    Logger::log(QString("Navdata:indexWaypoints: generated %1 index lines").arg(m_waypoint_index_map.count()));
    return ret;
}

/////////////////////////////////////////////////////////////////////////////

bool Navdata::indexNavaids()
{
    m_valid = false;
    MYASSERT(m_navaid_file->reset());
    m_navaid_index_map.clear();
    m_vor_list_coordinate_index_map.clear();
    m_ndb_list_coordinate_index_map.clear();

    bool ret = generateIndex(m_navaid_file, m_navaid_index_map, true);

    Logger::log(QString("Navdata:indexNavaids: generated %1 index lines").arg(m_navaid_index_map.count()));
    return ret;
}

/////////////////////////////////////////////////////////////////////////////

bool Navdata::indexAirways()
{
    m_valid = false;
    MYASSERT(m_airway_file->reset());
    m_airway_index_map.clear();

    QList<QChar> current_index;
    current_index.append(QChar('*'));
    current_index.append(QChar('*'));

    while(!m_airway_file->atEnd())
    {
        qint64 read_pos = m_airway_file->pos();

        QByteArray line_array = m_airway_file->readLine();
        QString line(line_array);
        line = line.trimmed();
        if (line.isEmpty()) continue;
        line = line.toUpper();

        if (line.at(0) != AIRWAY_ROUTE_PREFIX) continue;

        QString found_airway_name;
        int segment_count;

        if (!parseAirwayRoute(line, found_airway_name, segment_count))
        {
            Logger::log(QString("Navdata:indexAirways: ERROR: "
                                "Could not parse airway (%1)").arg(line));
            return false;
        }

        if (found_airway_name.at(0) != current_index[0])
        {
            current_index[0] = found_airway_name.at(0);
            m_airway_index_map.insert(current_index[0], read_pos);

            current_index[1] = '*';
            if (found_airway_name.length() > 1)
            {
                current_index[1] = found_airway_name.at(1);
                m_airway_index_map.insert(QString(current_index[0])+current_index[1], read_pos);
            }
        }
        else if (found_airway_name.length() > 1 && found_airway_name.at(1) != current_index[1])
        {
            current_index[1] = found_airway_name.at(1);
            m_airway_index_map.insert(QString(current_index[0])+current_index[1], read_pos);
        }
    }

    Logger::log(QString("Navdata:indexAirways: generated %1 index lines").arg(m_airway_index_map.count()));
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool Navdata::indexAirports()
{
    m_valid = false;
    MYASSERT(m_airport_file->reset());
    m_airport_index_map.clear();
    m_airport_list_coordinate_index_map.clear();

    QList<QChar> current_index;
    current_index.append(QChar('*'));
    current_index.append(QChar('*'));

    while(!m_airport_file->atEnd())
    {
        // read the airport line

        qint64 read_pos = m_airport_file->pos();
        QByteArray line_array = m_airport_file->readLine();
        QString line(line_array);
        line = line.trimmed();
        if (line.isEmpty()) continue;
        line = line.toUpper();

        if (line.at(0) != AIRPORT_RECORD_PREFIX) continue;

        // read the runway lines of the airport

        QStringList runway_lines;
        while(!m_airport_file->atEnd())
        {
            QByteArray line_array = m_airport_file->readLine();
            QString line(line_array);
            line = line.trimmed();
            if (line.isEmpty()) break;
            line = line.toUpper();
            if (line.at(0) != RUNWAY_RECORD_PREFIX) break;
            runway_lines.append(line);
        }

        // parse the data

        Airport found_airport;

        if (!parseAirport(line, runway_lines, &found_airport))
        {
            Logger::log(QString("Navdata:indexAirports: ERROR: "
                                "Could not parse airport (%1)").arg(line));
            return false;
        }

        // create ID index

        if (found_airport.id().at(0) != current_index[0])
        {
            current_index[0] = found_airport.id().at(0);
            m_airport_index_map.insert(current_index[0], read_pos);

            current_index[1] = '*';
            if (found_airport.id().length() > 1)
            {
                current_index[1] = found_airport.id().at(1);
                m_airport_index_map.insert(QString(current_index[0])+current_index[1], read_pos);
            }
        }
        else if (line.length() > 1 && found_airport.id().at(1) != current_index[1])
        {
            current_index[1] = found_airport.id().at(1);
            m_airport_index_map.insert(QString(current_index[0])+current_index[1], read_pos);
        }

        // create coordinate index for airport with at least one runway longer than 2000m

        RunwayMapIterator rwy_iter = found_airport.runwayMapIterator();
        while(rwy_iter.hasNext())
        {
            const Runway& rwy = rwy_iter.next().value();

            if (rwy.lengthM() >= 2000)
            {
                QString coordinate_index = getCoordinateIndex(found_airport);
                m_airport_list_coordinate_index_map[coordinate_index].append(found_airport);
                break;
            }
        }
    }

    Logger::log(QString("Navdata::indexAirports: generated %1 ID index lines").
                arg(m_airport_index_map.count()));
    Logger::log(QString("Navdata::indexAirports: generated %1 coordinate index lines").
                arg(m_airport_list_coordinate_index_map.count()));
    return true;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////// PARSER METHODS //////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

bool Navdata::parseAirwayRoute(const QString& line, QString& airway_name, int& segment_count) const
{
    airway_name = QString::null;
    segment_count = 0;

    if (line.isEmpty())
    {
        Logger::log("Navdata:parseAirwayRoute: line was empty");
        return false;
    }
    QStringList item_list = line.split(NDSEP);
    if (item_list.count() != 3)
    {
        Logger::log(QString("Navdata:parseAirwayRoute: item count != 3 (%1)").arg(line));
        return false;
    }
    if (item_list[AIRWAY_PREFIX_INDEX].at(0) != AIRWAY_ROUTE_PREFIX)
    {
        Logger::log(QString("Navdata:parseAirwayRoute: item[0] != PREFIX (%1)").arg(line));
        return false;
    }

    airway_name = item_list[AIRWAY_NAME_INDEX];

    bool conv_ok = false;
    segment_count = item_list[AIRWAY_SEGCOUNT_INDEX].toInt(&conv_ok);
    if (!conv_ok)
    {
        Logger::log(QString("Navdata:parseAirwayRoute: could not convert distance (%1)").arg(line));
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool Navdata::parseAirwayRouteSegment(const QString& line,
                                      Waypoint& waypoint1,
                                      Waypoint& waypoint2,
                                      int& inbound_course,
                                      int& outbound_course,
                                      double& distance) const
{
    waypoint1 = waypoint2 = Waypoint();

    if (line.isEmpty()) return false;
    QStringList item_list = line.split(NDSEP);
    if (item_list.count() != 10)
    {
        Logger::log(QString("Navdata:parseAirwayRouteSegment: ERROR: "
                            "item count (%1) != 10").arg(item_list.count()));
        return false;
    }
    if (item_list[AIRWAY_SEGMENT_PREFIX_INDEX].at(0) != AIRWAY_ROUTE_SEGMENT_PREFIX)
    {
        Logger::log("Navdata:parseAirwayRouteSegment: ERROR: "
                    "airway segment prefix not found");
        return false;
    }

    waypoint1 = Waypoint(item_list[AIRWAY_SEGMENT_NAME1_INDEX],
                         QString::null,
                         item_list[AIRWAY_SEGMENT_LAT1_INDEX].toDouble()/COORD_FACTOR,
                         item_list[AIRWAY_SEGMENT_LON1_INDEX].toDouble()/COORD_FACTOR);

    waypoint2 = Waypoint(item_list[AIRWAY_SEGMENT_NAME2_INDEX],
                         QString::null,
                         item_list[AIRWAY_SEGMENT_LAT2_INDEX].toDouble()/COORD_FACTOR,
                         item_list[AIRWAY_SEGMENT_LON2_INDEX].toDouble()/COORD_FACTOR);

    inbound_course = item_list[AIRWAY_SEGMENT_INCOURSE_INDEX].trimmed().toInt();
    outbound_course = item_list[AIRWAY_SEGMENT_OUTCOURSE_INDEX].trimmed().toInt();
    distance = item_list[AIRWAY_SEGMENT_DIST_INDEX].trimmed().toDouble() / DIST_FACTOR;

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool Navdata::parseAirport(const QString& line,
                           const QStringList& runway_lines,
                           Airport* airport) const
{
    MYASSERT(airport);
    *airport = Airport();

    QString airport_icao;
    QString airport_name;
    Waypoint airport_location;
    int airport_elevation = 0;

    if (line.isEmpty())
    {
        Logger::log("Navdata:parseAirport: line was empty");
        return false;
    }

    QStringList item_list = line.split(NDSEP);

    if (item_list.count() != 6)
    {
        Logger::log(QString("Navdata:parseAirport: item count != 6 (%1)").arg(line));
        return false;
    }

    if (item_list[AIRPORT_PREFIX_INDEX].at(0) != AIRPORT_RECORD_PREFIX)
    {
        Logger::log(QString("Navdata:parseAirport: item[0] != PREFIX (%1)").arg(line));
        return false;
    }

    // get airport ICAO and name

    airport_icao = item_list[AIRPORT_ID_INDEX];
    //NOTE: this normalization is done because of erronous character
    //inside the navdata waypoints file.
    normalizeID(airport_icao);

    airport_name = item_list[AIRPORT_NAME_INDEX];

    bool convok1 = false, convok2 = false;

    // get airport location

    airport_location =
        Waypoint(airport_icao, airport_name,
                 item_list[AIRPORT_LAT_INDEX].toDouble(&convok1)/COORD_FACTOR,
                 item_list[AIRPORT_LON_INDEX].toDouble(&convok2)/COORD_FACTOR);

    if (!convok1 || ! convok2)
    {
        Logger::log(QString("Navdata:parseAirport: ERROR: "
                            "Could not convert LAT/LON of (%1/%2/%3)").
                    arg(item_list[AIRPORT_ID_INDEX]).
                    arg(item_list[AIRPORT_LAT_INDEX]).
                    arg(item_list[AIRPORT_LON_INDEX]));
        return false;
    }

    // get airport elevation

    bool conv_ok = false;
    airport_elevation = item_list[AIRPORT_ELEV_INDEX].toInt(&conv_ok);
    if (!conv_ok)
    {
        Logger::log(QString("Navdata:parseAirport: could not convert elevation (%1)").arg(line));
        return false;
    }

    *airport = Airport(airport_icao, airport_name, airport_location.lat(),
                       airport_location.lon(), airport_elevation);

    // process runway lines

    QStringList::const_iterator iter = runway_lines.begin();
    for (; iter != runway_lines.end(); ++iter)
    {
        const QString& line = *iter;

        QStringList item_list = line.split(NDSEP);

        if (item_list.count() != 12)
        {
            Logger::log(QString("Navdata:parseAirport: rwy item count != 12 (%1)").arg(line));

            return false;
        }

        if (item_list[RUNWAY_PREFIX_INDEX].at(0) != RUNWAY_RECORD_PREFIX)
        {
            Logger::log(QString("Navdata:parseAirport: rwy item[0] != PREFIX (%1)").arg(line));

            return false;
        }

        Runway runway(item_list[RUNWAY_ID_INDEX],
                      item_list[RUNWAY_LAT_INDEX].toDouble()/COORD_FACTOR,
                      item_list[RUNWAY_LON_INDEX].toDouble()/COORD_FACTOR,
                      item_list[RUNWAY_HDG_INDEX].toInt(),
                      (int) ( (item_list[RUNWAY_LENGTH_INDEX].toInt() * FT_TO_M) + 0.5),
                      item_list[RUNWAY_ILSFLAG_INDEX].toInt(),
                      item_list[RUNWAY_ILSFREQ_INDEX].toInt(),
                      item_list[RUNWAY_ILSHDG_INDEX].toInt(),
                      item_list[RUNWAY_ELEV_INDEX].toInt(),
                      item_list[RUNWAY_GSANGLE_INDEX].toInt(),
                      item_list[RUNWAY_THROFLYHEIGHT_INDEX].toInt());

//       Logger::log("Navdata:parseAirport: inserted new runway: %1: %2").arg(
//              airport_icao, runway.toString());
//

        airport->addRunway(runway);
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////

Waypoint* Navdata::parseNavaid(const QString& line,
                               const QString& wanted_id,
                               const QString& wanted_country_code,
                               const QString& wanted_type) const
{
    QStringList item_list = line.split(NDSEP);

    if (item_list.count() != 10)
    {
        Logger::log(QString("Navdata:parseNavaidgetNavaids: line %1, #items %2 != 10 - skipping").
                    arg(line).arg(item_list.count()));
        return 0;
    }

    // check if we found the an entry

    QString item_id = item_list[NAVAID_ID_INDEX];
    //NOTE: this normalization is done because of erronous characters inside the navdata waypoints file.
    normalizeID(item_id);

    const QString& ctry = item_list[NAVAID_CCODE_INDEX];
    if ((wanted_id.isEmpty() || item_id == wanted_id) && (wanted_country_code.isEmpty() || wanted_country_code == ctry))
    {
        if (item_list[NAVAID_VORFLAG_INDEX] == "1")
        {
            if (wanted_type == Waypoint::TYPE_VOR || wanted_type == Waypoint::TYPE_ALL)
            {
                Vor* vor = new Vor(item_id,
                                   item_list[NAVAID_NAME_INDEX],
                                   item_list[NAVAID_LAT_INDEX].toDouble()/COORD_FACTOR,
                                   item_list[NAVAID_LON_INDEX].toDouble()/COORD_FACTOR,
                                   item_list[NAVAID_FREQ_INDEX].toInt(),
                                   item_list[NAVAID_DMEFLAG_INDEX].toInt(),
                                   item_list[NAVAID_RANGE_INDEX].toInt(),
                                   item_list[NAVAID_ELEVATION_INDEX].toInt(),
                                   item_list[NAVAID_CCODE_INDEX]);
                MYASSERT(vor);
                return vor;
            }
        }
        else if (item_list[NAVAID_VORFLAG_INDEX] == "0")
        {
            if (item_list[NAVAID_NAME_INDEX].contains("ILS") || item_list[NAVAID_DMEFLAG_INDEX].toInt() != 0)
            {
                if (wanted_type == Waypoint::TYPE_ILS || wanted_type == Waypoint::TYPE_ALL)
                {
                    Ils* ils = new Ils(item_id,
                                       item_list[NAVAID_NAME_INDEX],
                                       item_list[NAVAID_LAT_INDEX].toDouble()/COORD_FACTOR,
                                       item_list[NAVAID_LON_INDEX].toDouble()/COORD_FACTOR,
                                       item_list[NAVAID_FREQ_INDEX].toInt(),
                                       item_list[NAVAID_DMEFLAG_INDEX].toInt(),
                                       item_list[NAVAID_RANGE_INDEX].toInt(),
                                       item_list[NAVAID_ELEVATION_INDEX].toInt(),
                                       item_list[NAVAID_CCODE_INDEX], 0);
                    MYASSERT(ils);
                    return ils;
                }
            }
            else
            {
                if (wanted_type == Waypoint::TYPE_NDB || wanted_type == Waypoint::TYPE_ALL)
                {
                    Ndb* ndb = new Ndb(item_id,
                                       item_list[NAVAID_NAME_INDEX],
                                       item_list[NAVAID_LAT_INDEX].toDouble()/COORD_FACTOR,
                                       item_list[NAVAID_LON_INDEX].toDouble()/COORD_FACTOR,
                                       item_list[NAVAID_FREQ_INDEX].toInt(),
                                       item_list[NAVAID_RANGE_INDEX].toInt(),
                                       item_list[NAVAID_ELEVATION_INDEX].toInt(),
                                       item_list[NAVAID_CCODE_INDEX]);
                    MYASSERT(ndb);
                    return ndb;
                }
            }
        }
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
////////////////////////// GET METHODS //////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

uint Navdata::getNavaids(const QString& wanted_id,
                         WaypointPtrList& wpt_list,
                         const QString& wanted_country_code,
                         const QString& wanted_type) const
{
    QTime start_time;
    start_time.start();

    MYASSERT(!wanted_id.isEmpty());

    if (!m_navaid_index_map.contains(wanted_id.at(0)))
    {
        Logger::log(QString("Navdata:getNavaids: WARNING: "
                            "Could not find index for navaid (%1)").arg(wanted_id));

        return 0;
    }
    else
    {
        if (wanted_id.length() > 1)
        {
            if (!m_navaid_index_map.contains(QString(wanted_id.at(0))+wanted_id.at(1)))
            {
                Logger::log(QString("Navdata:getNavaids: no two-char-index for %1").arg(wanted_id));
                return 0;
            }
            m_navaid_file->seek(m_navaid_index_map[QString(wanted_id.at(0))+wanted_id.at(1)]);
        }
        else
        {
            m_navaid_file->seek(m_navaid_index_map[wanted_id.at(0)]);
        }
    }

    bool found_navaid = false;

    while (!m_navaid_file->atEnd())
    {
        QString line(m_navaid_file->readLine());
        line = line.trimmed();
        if (line.isEmpty()) continue;
        line = line.toUpper();

        Waypoint* navaid = parseNavaid(line, wanted_id, wanted_country_code, wanted_type);
        if (navaid != 0)
        {
            found_navaid = true;
            wpt_list.append(navaid);

            // check if we are already past the possible cases in the navaid file

            if (navaid->id().at(0) != wanted_id.at(0))
            {
                break;
            }
            else if (wanted_id.length() > 1 &&
                     navaid->id().length() > 1 &&
                     navaid->id().at(1) > wanted_id.at(1))
            {
                break;
            }
            else if (wanted_id.length() > 2 &&
                     navaid->id().length() > 2 &&
                     navaid->id().at(1) >= wanted_id.at(1) &&
                     navaid->id().at(2) > wanted_id.at(2))
            {
                break;
            }

            if (found_navaid && navaid->id() != wanted_id) break;
        }
    }

//     Logger::log(QString("Navdata:getNavaids: searching for [%1] fin, found %2 entries in %3ms").
//                 arg(wanted_id).arg(wpt_list.count()).arg(start_time.elapsed()));

    return wpt_list.count();
}

/////////////////////////////////////////////////////////////////////////////

Waypoint* Navdata::getElementsWithSignal(const QString& id,
                                         const QString& wanted_country_code,
                                         const QString& wanted_type)
{
    WaypointPtrList wpt_list;

    if (wanted_type == Waypoint::TYPE_VOR || wanted_type == Waypoint::TYPE_NDB)
    {
        if (getNavaids(id, wpt_list, wanted_country_code, wanted_type) <= 0) return 0;
    }
    else if (wanted_type == Waypoint::TYPE_AIRPORT)
    {
        if (getAirports(id, wpt_list) <= 0) return 0;
    }
    else if (wanted_type == Waypoint::TYPE_INTERSECTION)
    {
        if (getIntersections(id, wpt_list) <= 0) return 0;
    }
    else return 0;

    Waypoint *chosen_waypoint = wpt_list.at(0);

    if (wpt_list.count() > 1)
    {
        emit signalWaypointChoose(wpt_list, &chosen_waypoint);
        if (!chosen_waypoint) return 0;
    }

    return chosen_waypoint->deepCopy();
}

/////////////////////////////////////////////////////////////////////////////

uint Navdata::getIntersections(const QString& wanted_id, WaypointPtrList& wpt_list) const
{
    //Logger::log(QString("Navdata:getIntersections: searching for [%1]").arg(wanted_id));

    QTime start_time;
    start_time.start();

    MYASSERT(!wanted_id.isEmpty());

    if (!m_waypoint_index_map.contains(wanted_id.at(0)))
    {
        Logger::log(QString("Navdata:getIntersections: WARNING: "
                            "Could not find index for waypoint (%1)").arg(wanted_id));

        return 0;
    }
    else
    {
        if (wanted_id.length() > 1)
        {
            if (!m_waypoint_index_map.contains(QString(wanted_id.at(0))+wanted_id.at(1)))
            {
                Logger::log(QString("Navdata:getNavaids: no two-char-index for %1").arg(wanted_id));
                return 0;
            }

            m_waypoint_file->seek(m_waypoint_index_map[QString(wanted_id.at(0))+wanted_id.at(1)]);
        }
        else
        {
            m_waypoint_file->seek(m_waypoint_index_map[wanted_id.at(0)]);
        }
    }

    bool found_waypoint = false;

    while(!m_waypoint_file->atEnd())
    {
        QString line(m_waypoint_file->readLine());
        line = line.trimmed();
        if (line.isEmpty()) continue;
        line = line.toUpper();

        QStringList item_list = line.split(NDSEP);
        if (item_list.count() != 4)
        {
            Logger::log(QString("Navdata:getIntersections: ERROR: "
                                "could not find 4 items in line (%1)").arg(line));
            continue;
        }

        // check if we found an item

        QString item_id = item_list[WPT_ID_INDEX];
        //NOTE: this normalization is done because of erronous character
        //inside the navdata waypoints file.
        normalizeID(item_id);

        if (item_id == wanted_id)
        {
            found_waypoint = true;

            bool convok1 = false, convok2 = false;

            Intersection* intersection =
                new Intersection(item_id,
                                 item_id,
                                 item_list[WPT_LAT_INDEX].toDouble(&convok1)/COORD_FACTOR,
                                 item_list[WPT_LON_INDEX].toDouble(&convok2)/COORD_FACTOR,
                                 item_list[WPT_CCODE_INDEX]);
            MYASSERT(intersection);

            if (!convok1 || ! convok2)
            {
                Logger::log(QString("Navdata:getIntersections: ERROR: "
                                    "Could not convert LAT/LON of (%1/%2/%3)").
                            arg(item_id).
                            arg(item_list[WPT_LAT_INDEX]).
                            arg(item_list[WPT_LON_INDEX]));

                delete intersection;
            }
            else
            {
                wpt_list.append(intersection);
            }
        }

        // check if we are already past the possible cases in the navaid file

        if (item_id.at(0) != wanted_id.at(0))
        {
            break;
        }
        else if (wanted_id.length() > 1 &&
                 item_id.length() > 1 &&
                 item_id.at(1) > wanted_id.at(1))
        {
            break;
        }
        else if (wanted_id.length() > 2 &&
                 item_id.length() > 2 &&
                 item_id.at(1) >= wanted_id.at(1) &&
                 item_id.at(2) > wanted_id.at(2))
        {
            break;
        }

        if (found_waypoint && item_id != wanted_id) break;
    }

    Logger::log(QString("Navdata:getIntersections: searching for [%1] fin, found %2 entries in %3ms").
                arg(wanted_id).arg(wpt_list.count()).arg(start_time.elapsed()));
    return wpt_list.count();
}

/////////////////////////////////////////////////////////////////////////////

uint Navdata::getWaypoints(const QString& wpt_name,
                           WaypointPtrList& result_list,
                           const QRegExp& lat_lon_regexp) const
{
    //Logger::log(QString("FMCControl:getWaypoint: %1").arg(wpt_name));

    result_list.clear();

    //----- look for airport

    if (!wpt_name.contains(" ") &&
        (wpt_name.length() <= 4 || (wpt_name.length() >= 6 && wpt_name[4].isDigit() && wpt_name[5].isDigit())))
    {
        getAirports(wpt_name.left(4), result_list);

        if (result_list.count() == 1 && wpt_name.length() >= 6)
        {
            QString runway_id = wpt_name.mid(4);
            Airport* airport = result_list.at(0)->asAirport();
            MYASSERT(airport != 0);
            airport->setActiveRunwayId(runway_id);
            if (!airport->activeRunway().isValid()) result_list.clear();
        }
    }

    //----- look for LAT/LON waypoint

    Waypoint* wpt = getIntersectionFromLatLonString(wpt_name, wpt_name, lat_lon_regexp);
    if (wpt != 0)
    {
        wpt->setId(QString("%1/%2").arg(wpt->latString().left(3)).arg(wpt->lonString().left(3)));
        result_list.append(wpt);
    }

    //----- look for normal waypoint (vor/ndb/fix)

    if (!wpt_name.contains(" "))
    {
        // search navaids
        getNavaids(wpt_name, result_list);

        // search waypoints
        WaypointPtrList waypoint_list;
        getIntersections(wpt_name, waypoint_list);

        // merge lists

        WaypointPtrListIterator wpt_iter(waypoint_list);
        for(; wpt_iter.hasNext();)
        {
            const Waypoint* waypoint = wpt_iter.next();
            MYASSERT(waypoint);

            bool is_the_same = false;

            WaypointPtrListIterator navaid_iter(result_list);
            for(; navaid_iter.hasNext();)
            {
                const Waypoint* navaid = navaid_iter.next();
                MYASSERT(navaid);
                is_the_same = (*navaid == *waypoint);
                if (is_the_same) break;
            }

            if (!is_the_same) result_list.append(waypoint->deepCopy());
        }
    }

    return result_list.count();
}

/////////////////////////////////////////////////////////////////////////////

Waypoint* Navdata::getIntersectionFromLatLonString(const QString& id,
                                                   const QString& latlon_string,
                                                   const QRegExp& regexp) const
{
    if (latlon_string.isEmpty() || !regexp.isValid() || regexp.isEmpty()) return 0;

    QRegExp latlon_regexp = regexp;
    if (latlon_regexp.indexIn(latlon_string) < 0) return 0;

    // convert latitude

    QString lat_char = latlon_regexp.cap(1);
    QString lat_deg_string = latlon_regexp.cap(2);
    QString lat_min_string = latlon_regexp.cap(3);

    double lat = lat_deg_string.toDouble() + (lat_min_string.toDouble() * (1/60.0));
    if (lat_char == "S") lat *= -1.0;

    // convert longitude

    QString lon_char = latlon_regexp.cap(4);
    QString lon_deg_string = latlon_regexp.cap(5);
    QString lon_min_string = latlon_regexp.cap(6);

    double lon = lon_deg_string.toDouble() + (lon_min_string.toDouble() * (1/60.0));
    if (lon_char == "W") lon *= -1.0;

    Waypoint* wpt = new Waypoint(id, QString::null, lat, lon);
    MYASSERT(wpt != 0);
    return wpt;
}

/////////////////////////////////////////////////////////////////////////////

uint Navdata::getAirways(const QString& airway_name, AirwayPtrList& airways) const
{
    QTime start_time;
    start_time.start();

    MYASSERT(!airway_name.isEmpty());
    airways.clear();

    if (!m_airway_index_map.contains(airway_name.at(0)))
    {
        Logger::log(QString("Navdata:getAirways: WARNING: "
                            "Could not find index for airway (%1)").arg(airway_name));
        return 0;
    }
    else
    {
        if (airway_name.length() > 1)
        {
            if (!m_airway_index_map.contains(QString(airway_name.at(0))+airway_name.at(1)))
            {
                Logger::log(QString("Navdata:getAirways: no two-char-index for %1").arg(airway_name));
                return 0;
            }
            m_airway_file->seek(m_airway_index_map[QString(airway_name.at(0))+airway_name.at(1)]);
        }
        else
        {
            m_airway_file->seek(m_airway_index_map[airway_name.at(0)]);
        }
    }

    while(!m_airway_file->atEnd())
    {
        QByteArray line_array = m_airway_file->readLine();
        QString line(line_array);
        line = line.trimmed();
        if (line.isEmpty()) continue;
        line = line.toUpper();

        QStringList item_list = line.split(NDSEP);
        if (item_list[AIRWAY_PREFIX_INDEX].at(0) != AIRWAY_ROUTE_PREFIX) continue;

        QString found_airway_name;
        int segment_count = 0;

        if (!parseAirwayRoute(line, found_airway_name, segment_count))
        {
            Logger::log(QString("Navdata:getAirways: ERROR: "
                                "Could not read parse airway (%1) - aborting").arg(line));
            airways.clear();
            return 0;
        }

        // check if we should break

        if (found_airway_name.at(0) != airway_name.at(0))
        {
            break;
        }
        else if (found_airway_name.length() > 1 &&
                 airway_name.length() > 1 &&
                 found_airway_name.at(1) > airway_name.at(1))
        {
            break;
        }
        else if (found_airway_name.length() > 1 &&
                 airway_name.length() > 1 &&
                 found_airway_name.at(1) > airway_name.at(1))
        {
            break;
        }
        else if (found_airway_name.length() > 2 &&
                 airway_name.length() > 2 &&
                 found_airway_name.at(1) >= airway_name.at(1) &&
                 found_airway_name.at(2) > airway_name.at(2))
        {
            break;
        }

        if (found_airway_name != airway_name) continue;

        //Logger::log(QString("Navdata:getAirways: found (%1)").arg(found_airway_name));

        //----- loop through airway segments

        bool first_segment = true;
        Waypoint prev_waypoint2;
        Airway* airway = new Airway(airway_name);
        MYASSERT(airway);

        while(!m_airway_file->atEnd())
        {
            QByteArray line_array = m_airway_file->readLine();
            QString line(line_array);
            line = line.trimmed();
            if (line.isEmpty()) break;
            line = line.toUpper();

            QStringList item_list = line.split(NDSEP);
            if (item_list[AIRWAY_SEGMENT_PREFIX_INDEX].at(0) !=
                AIRWAY_ROUTE_SEGMENT_PREFIX) break;

            Waypoint waypoint1;
            Waypoint waypoint2;
            int inbound_course = 0;
            int outbound_course = 0;
            double distance = 0;

            if (!parseAirwayRouteSegment(line, waypoint1, waypoint2,
                                         inbound_course, outbound_course, distance))
            {
                Logger::log(QString("Navdata:getAirways: ERROR: "
                                    "Could not read parse airway segment (%1) - aborting").arg(line));

                airways.clear();
                delete airway;
                airway = 0;
                return 0;
            }

            waypoint1.setParent(airway->id());
            waypoint2.setParent(airway->id());

            if (first_segment)
            {
                airway->appendWaypoint(waypoint1);
                first_segment = false;
            }
            else
            {
                if (waypoint1.name() != prev_waypoint2.name())
                {
                    Logger::log(QString("Navdata:getAirways: ERROR: "
                                        "(AW: %1) 2nd waypoint of the last segment is different "
                                        "from the 1st waypoint of the current segment "
                                        "(%2 vs. %3) - aborting").
                                arg(airway_name).arg(prev_waypoint2.name()).arg(waypoint1.name()));

                    airways.clear();
                    delete airway;
                    airway = 0;
                    return 0;
                }
            }

            airway->appendWaypoint(waypoint2);
            prev_waypoint2 = waypoint2;
        }

// add airway to list

        if (airway != 0 && airway->count() > 0)
            airways.append(airway);
        else
            delete airway;
    }

    Logger::log(QString("Navdata:getAirways: searching for [%1] fin, found %2 entries in %3ms").
                arg(airway_name).arg(airways.count()).arg(start_time.elapsed()));

    return airways.count();
}

/////////////////////////////////////////////////////////////////////////////

uint Navdata::getAirports(const QString& name, WaypointPtrList& airports) const
{
    QTime start_time;
    start_time.start();

    MYASSERT(!name.isEmpty());

    if (!m_airport_index_map.contains(name.at(0)))
    {
        Logger::log(QString("Navdata:getAirports: WARNING: "
                            "Could not find index for airport (%1)").arg(name));

        return 0;
    }
    else
    {
        if (name.length() > 1)
        {
            if (!m_airport_index_map.contains(QString(name.at(0))+name.at(1)))
            {
                Logger::log(QString("Navdata:getAirports: no two-char-index for %1").arg(name));
                return 0;
            }
            m_airport_file->seek(m_airport_index_map[QString(name.at(0))+name.at(1)]);
        }
        else
        {
            m_airport_file->seek(m_airport_index_map[name.at(0)]);
        }
    }

    bool found_airport = false;

    while(!m_airport_file->atEnd())
    {
        // read the airport line

        QByteArray line_array = m_airport_file->readLine();
        QString line(line_array);
        line = line.trimmed();
        if (line.isEmpty()) continue;
        line = line.toUpper();

        if (line.at(0) != AIRPORT_RECORD_PREFIX) continue;

        // read the runway lines of the airport

        QStringList runway_lines;
        while(!m_airport_file->atEnd())
        {
            QByteArray line_array = m_airport_file->readLine();
            QString line(line_array);
            line = line.trimmed();
            if (line.isEmpty()) break;
            line = line.toUpper();
            if (line.at(0) != RUNWAY_RECORD_PREFIX) break;
            runway_lines.append(line);
        }

        // parse the data

        Airport* airport = new Airport();
        MYASSERT(airport);

        if (!parseAirport(line, runway_lines, airport))
        {
            Logger::log(QString("Navdata:getAirports: ERROR: "
                                "Could not parse airport from line (%1) - aborting").arg(line));

            airports.clear();
            delete airport;
            airport = 0;
            break;
        }

        if (airport->id().at(0) != name.at(0)) break;
        if (found_airport && airport->id() != name) break;

        if (airport->id() == name)
        {
            found_airport = true;
            airports.append(airport);
        }
    }

    Logger::log(QString("Navdata:getAirports: searching for [%1] fin, found %2 entries in %3ms").
                arg(name).arg(airports.count()).arg(start_time.elapsed()));

    return airports.count();
}

/////////////////////////////////////////////////////////////////////////////

uint Navdata::getSids(const QString& airport, ProcedurePtrList& procedures) const
{
    if (getLevelDProcedures(airport, Procedure::TYPE_SID, procedures) == 0)
        getProcedures(airport, Procedure::TYPE_SID, procedures);

    return procedures.count();
}

/////////////////////////////////////////////////////////////////////////////

uint Navdata::getStars(const QString& airport, ProcedurePtrList& procedures) const
{
    if (getLevelDProcedures(airport, Procedure::TYPE_STAR, procedures) == 0)
        getProcedures(airport, Procedure::TYPE_STAR, procedures);

    return procedures.count();
}

/////////////////////////////////////////////////////////////////////////////

uint Navdata::getApproaches(const QString& airport, ProcedurePtrList& procedures) const
{
    if (getLevelDProcedures(airport, Procedure::TYPE_APPROACH, procedures) == 0)
        getProcedures(airport, Procedure::TYPE_APPROACH, procedures);

    return procedures.count();
}

/////////////////////////////////////////////////////////////////////////////

uint Navdata::getProcedures(const QString& airport, const QString& wanted_type, ProcedurePtrList& procedures) const
{
    MYASSERT(!airport.isEmpty());
    procedures.clear();

    QFile* procfile = 0;

    if (wanted_type == Route::TYPE_SID)
        procfile = new QFile(VasPath::prependPath(m_navdata_config->getValue(CFG_SID_SUBDIR)+
                                                  "/"+airport.toLower()+PROCEDURE_FILE_EXT));
    else if (wanted_type == Route::TYPE_STAR)
        procfile = new QFile(VasPath::prependPath(m_navdata_config->getValue(CFG_STAR_SUBDIR)+
                                                  "/"+airport.toLower()+PROCEDURE_FILE_EXT));
    else
        return 0;

    MYASSERT(procfile);

    //-----

    if (!procfile->exists())
    {
#ifndef Q_OS_WIN32
        if (isOnCaseSensitiveFilesystem())
        {
            Logger::log(QString("Navdata::getProcedures: no procedures found for airport "
                                "%1 (%2). Your filesystem is case-sensitive, renaming all proc files to lowercase").arg(airport).arg(procfile->fileName()));

            renameNavdataFilenamesToLower(VasPath::prependPath(m_navdata_config->getValue(CFG_SID_SUBDIR)));
            renameNavdataFilenamesToLower(VasPath::prependPath(m_navdata_config->getValue(CFG_STAR_SUBDIR)));
        }
#else
        Logger::log(QString("Navdata:getProcedures: no procedures found for "
                            "(%1) (%2)").arg(airport).arg(procfile->fileName()));
#endif

        delete procfile;
        return 0;
    }

    if (!procfile->open(QIODevice::ReadOnly | QIODevice::Text))
    {
#if! VASFMC_GAUGE
        QMessageBox::critical(0, "Navdata init",
                              QString("Could not open airport file %1").
                              arg(procfile->fileName()));
#endif
        delete procfile;
        return 0;
    }

    //-----

    while(!procfile->atEnd())
    {
        QByteArray line_array = procfile->readLine();
        QString line(line_array);
        line = line.trimmed();
        if (line.isEmpty()) continue;
        line = line.toUpper();

        // parse the procedure definition line

        Procedure* procedure = parseProcedure(line, wanted_type);
        if (!procedure) continue;

        // fetch procedure leg lines

        while(!procfile->atEnd())
        {
            QByteArray line_array = procfile->readLine();
            QString line(line_array);
            line = line.trimmed();
            if (line.isEmpty()) break;
            line = line.toUpper();

            Waypoint proc_wpt;
            if (!parseProcedureWaypoint(line, proc_wpt)) continue;

            proc_wpt.setParent(procedure->id());
            if (wanted_type == Route::TYPE_SID) proc_wpt.setFlag(Waypoint::FLAG_SID);
            else if (wanted_type == Route::TYPE_STAR) proc_wpt.setFlag(Waypoint::FLAG_STAR);

            procedure->appendWaypoint(proc_wpt);

            //Logger::log(QString("Navdata:getProcedures: added wpt (%1)").arg(proc_wpt.toString()));
        }

        if (procedure->count() > 0)
        {
            // add the procedure to the result list
            procedures.append(procedure);
        }
        else
        {
            delete procedure;
            procedure = 0;
        }

        //Logger::log(QString("Navdata:getProcedures: added proc (%1)").arg(procedure->toString()));
    }

    //-----

    Logger::log(QString("Navdata:getProcedures: found %1 procedures of type %2 for %3").
                arg(procedures.count()).arg(wanted_type).arg(airport));

    delete procfile;
    return procedures.count();
}

/////////////////////////////////////////////////////////////////////////////

Procedure* Navdata::parseProcedure(const QString& line, const QString& wanted_type) const
{
    if (line.isEmpty())
    {
        Logger::log("Navdata:parseProcedure: line was empty");

        return 0;
    }

    QStringList item_list = line.split(NDSEP, QString::SkipEmptyParts);
    if (item_list.count() != 6)
    {
        Logger::log(QString("Navdata:parseProcedure: item count %1 != 6 (%2)").
                    arg(item_list.count()).arg(line));

        return 0;
    }
    if (item_list[PROCEDURE_PREFIX_INDEX].at(0) != PROCEDURE_RECORD_PREFIX)
    {
        Logger::log(QString("Navdata:parseProcedure: item[0] != PREFIX (%1)").arg(line));
        return 0;
    }

    //-----

    Procedure* parsed_proc = 0;

    if (wanted_type == Procedure::TYPE_PROCEDURE)
        parsed_proc = new Procedure(item_list[PROCEDURE_ID_INDEX].trimmed(),
                                    QStringList(item_list[PROCEDURE_RUNWAY_INDEX].trimmed()));
    else if (wanted_type == Procedure::TYPE_SID)
        parsed_proc = new Sid(item_list[PROCEDURE_ID_INDEX].trimmed(),
                              QStringList(item_list[PROCEDURE_RUNWAY_INDEX].trimmed()));
    else if (wanted_type == Procedure::TYPE_STAR)
        parsed_proc = new Star(item_list[PROCEDURE_ID_INDEX].trimmed(),
                               QStringList(item_list[PROCEDURE_RUNWAY_INDEX].trimmed()));

    MYASSERT(parsed_proc);

    return parsed_proc;
}

/////////////////////////////////////////////////////////////////////////////

//FIXXME: filter out unparseable waypoints (e.g. without id, etc.)

bool Navdata::parseProcedureWaypoint(const QString& line, Waypoint& parsed_wpt) const
{
    if (line.isEmpty())
    {
        Logger::log("Navdata:parseProcedureWaypoint: line was empty");
        return false;
    }

    QStringList item_list = line.split(NDSEP);
    if (item_list.count() != 19)
    {
        Logger::log(QString("Navdata:parseProcedureWaypoint: item count != 19 (%1)").arg(line));
        return false;
    }

    if (item_list[PROCEDURE_PREFIX_INDEX].at(0) != PROCEDURE_LEG_RECORD_PREFIX)
    {
        Logger::log(QString("Navdata:parseProcedureWaypoint: item[0] != PREFIX (%1)").arg(line));
        return false;
    }

    //----- filter out unsupported waypoints

    if (item_list[PROCEDURE_LEG_WPTID_INDEX].trimmed().isEmpty()) return false;

    //-----

    parsed_wpt = Waypoint(item_list[PROCEDURE_LEG_WPTID_INDEX].trimmed(),
                          item_list[PROCEDURE_LEG_WPTNAME_INDEX].trimmed(),
                          item_list[PROCEDURE_LEG_WPTLAT_INDEX].toDouble()/COORD_FACTOR,
                          item_list[PROCEDURE_LEG_WPTLON_INDEX].toDouble()/COORD_FACTOR);

    if (item_list[PROCEDURE_LEG_OVFLYWPTFLAG_INDEX].toInt() != 0)
        parsed_wpt.restrictions().setOverflyRestriction(true);

    return true;
}

/////////////////////////////////////////////////////////////////////////////

uint Navdata::getLevelDProcedures(const QString& airport,
                                  const QString& wanted_type,
                                  ProcedurePtrList& procedures) const
{
    MYASSERT(!airport.isEmpty());
    procedures.clear();

    const QDomDocument *proceduredb_dom = 0;

    if (!m_airport_to_leveld_procedure_chache_map.contains(airport))
    {
        QFile procfile(VasPath::prependPath(m_navdata_config->getValue(CFG_LEVELD_PROCEDURES_SUBDIR)+
                                            "/"+airport.toLower()+LEVELD_PROCEDURE_FILE_EXT));
        bool procedure_opened = true;

        if (!procfile.open(QIODevice::ReadOnly))
        {
            procedure_opened = false;
#ifndef Q_OS_WIN32
            if (isOnCaseSensitiveFilesystem())
            {
                Logger::log(QString("Navdata::getProcedures: no procedures found for airport "
                                    "%1 (%2). Your filesystem is case-sensitive, renaming all proc files to lowercase").arg(airport).arg(procfile.fileName()));
                renameNavdataFilenamesToLower(VasPath::prependPath(m_navdata_config->getValue(CFG_LEVELD_PROCEDURES_SUBDIR)));
            }
#else
            Logger::log(QString("Navdata:getLevelDProcedures: could not open procedure file (%1)").
                        arg(procfile.fileName()));
#endif
        }
        if (!m_airport_to_leveld_procedure_chache_map[airport].setContent(&procfile))
        {
            procedure_opened = false;
            Logger::log(QString("Navdata:getLevelDProcedures: could not parse procedure file (%1)").
                        arg(procfile.fileName()));
        }
        procfile.close();

//        Logger::log(QString("Navdata:getLevelDProcedures: found procedure file (%1)").
//                     arg(procfile.fileName()));
//-----
        return 0;
    }

    MYASSERT(m_airport_to_leveld_procedure_chache_map.contains(airport));
    proceduredb_dom = &m_airport_to_leveld_procedure_chache_map[airport];

    QDomNode proceduredb_node = proceduredb_dom->namedItem("ProceduresDB");
    if (proceduredb_node.isNull())
    {
        Logger::log("Navdata:getLevelDProcedures: DOM has no ProceduresDB node");
        return 0;
    }

    if (proceduredb_node.namedItem("Airport").isNull())
    {
        Logger::log("Navdata:getLevelDProcedures: DOM has no airport node");
        return 0;
    }

    if (proceduredb_node.namedItem("Airport").attributes().namedItem("ICAOcode").nodeValue().toUpper() != airport.toUpper())
    {
        Logger::log(QString("Navdata:getLevelDProcedures: airport node value (%1) did not match wanted airport (%2)").
                    arg(proceduredb_node.namedItem("Airport").attributes().namedItem("ICAOcode").nodeValue()).
                    arg(airport));
        return 0;
    }

    QDomNode procedure_node = proceduredb_node.namedItem("Airport").firstChild();
    for(;!procedure_node.isNull(); procedure_node = procedure_node.nextSibling())
    {
        if (!procedure_node.isElement()) continue;
        QDomElement procedure_element = procedure_node.toElement();

        // parse the procedure definition line

        Procedure* procedure = parseLevelDProcedure(procedure_element, wanted_type);
        if (procedure == 0) continue;

//         //TODO
//         Logger::log(QString("Navdata:getLevelDProcedures: ======================== %1 =====================").
//                     arg(procedure->id()));

        // fetch procedure waypoints

        QDomNode wpt_node = procedure_element.firstChild();
        for(;!wpt_node.isNull(); wpt_node = wpt_node.nextSibling())
        {
            if (!wpt_node.isElement()) continue;
            QDomElement wpt_element = wpt_node.toElement();

            Waypoint* proc_wpt = parseLevelDProcedureWaypoint(wpt_element, *procedure);
            if (proc_wpt == 0) continue;

            proc_wpt->setParent(procedure->id());
            if (wanted_type == Route::TYPE_SID) proc_wpt->setFlag(Waypoint::FLAG_SID);
            else if (wanted_type == Route::TYPE_STAR) proc_wpt->setFlag(Waypoint::FLAG_STAR);
            else if (wanted_type == Route::TYPE_APPROACH) proc_wpt->setFlag(Waypoint::FLAG_APPROACH);

            procedure->appendWaypoint(*proc_wpt);
            delete proc_wpt;
            proc_wpt = 0;
        }

        if (procedure->count() > 0)
        {
            // in add this procedure, but in case it is an approach, it has to have one runway
            if (procedure->asApproach() == 0 ||
                procedure->asApproach()->runwayList().count() == 1)
            {
                procedures.append(procedure);
            }

            //Logger::log(QString("Navdata:getProcedures: added proc (%1)").arg(procedure->toString()));
        }
        else
        {
            Logger::log(QString("Navdata:getLevelDProcedures: skipping empty procedure %1").
                        arg(procedure->id()));

            delete procedure;
            procedure = 0;
        }
    }

    //-----

    Logger::log(QString("Navdata:getLevelDProcedures: found %1 procedures of type %2 for %3").
                arg(procedures.count()).arg(wanted_type).arg(airport));

    return procedures.count();
}

/////////////////////////////////////////////////////////////////////////////

Procedure* Navdata::parseLevelDProcedure(const QDomElement& element, const QString& wanted_type) const
{
    if (wanted_type == Procedure::TYPE_SID)
    {
        if (element.tagName() != "Sid") return 0;
        return new Sid(element.attributes().namedItem("Name").nodeValue().trimmed(),
                       element.attributes().namedItem("Runways").nodeValue().split(","));
    }
    else if (wanted_type == Route::TYPE_STAR)
    {
        if (element.tagName() != "Star") return 0;
        return new Star(element.attributes().namedItem("Name").nodeValue().trimmed(),
                        element.attributes().namedItem("Runways").nodeValue().split(","));
    }
    else if (wanted_type == Route::TYPE_APPROACH)
    {
        if (element.tagName() != "Approach") return 0;
        return new Approach(element.attributes().namedItem("Name").nodeValue().trimmed(), QStringList());
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////

Waypoint* Navdata::parseLevelDProcedureWaypoint(const QDomElement& element,
                                                Procedure& procedure) const
{
    if (element.nodeName() == "App_Transition")
    {
        if (procedure.asApproach() == 0) return false;

        Transition* transition =
            new Transition(element.attributes().namedItem("Name").nodeValue().trimmed(),
                           procedure.asApproach()->runwayList());
        MYASSERT(transition != 0);
        transition->setParentProcedure(procedure.asApproach());
        MYASSERT(transition->parentProcedure()->asApproach() != 0);

        QDomNode trans_wpt_node = element.firstChild();
        for(;!trans_wpt_node.isNull(); trans_wpt_node = trans_wpt_node.nextSibling())
        {
            if (!trans_wpt_node.isElement()) continue;
            QDomElement wpt_element = trans_wpt_node.toElement();

            Waypoint* proc_wpt = parseLevelDProcedureWaypoint(wpt_element, *transition);
            if (proc_wpt == 0) continue;

            proc_wpt->setParent(transition->id());
            proc_wpt->setFlag(Waypoint::FLAG_APP_TRANS);
            transition->appendWaypoint(*proc_wpt);
            delete proc_wpt;
            proc_wpt = 0;
        }

        if (transition->count() > 0)
        {
            procedure.asApproach()->transitions().append(transition);
        }
        else
        {
            Logger::log(QString("Navdata:parseLevelDProcedureWaypoint: skipping empty transition %1 of APP %2").
                        arg(transition->id()).arg(procedure.id()));
            delete transition;
            transition = 0;
        }

        return 0;
    }
    else if (element.nodeName() == "Sid_Transition")
    {
        if (procedure.asSID() == 0) return false;

        Transition* transition =
            new Transition(element.attributes().namedItem("Name").nodeValue().trimmed(),
                           procedure.asSID()->runwayList());
        MYASSERT(transition != 0);
        transition->setParentProcedure(procedure.asSID());
        MYASSERT(transition->parentProcedure()->asSID() != 0);

        QDomNode trans_wpt_node = element.firstChild();
        for(;!trans_wpt_node.isNull(); trans_wpt_node = trans_wpt_node.nextSibling())
        {
            if (!trans_wpt_node.isElement()) continue;
            QDomElement wpt_element = trans_wpt_node.toElement();

            Waypoint* proc_wpt = parseLevelDProcedureWaypoint(wpt_element, *transition);
            if (proc_wpt == 0) continue;

            proc_wpt->setParent(transition->id());
            proc_wpt->setFlag(Waypoint::FLAG_SID_TRANS);
            transition->appendWaypoint(*proc_wpt);
            delete proc_wpt;
            proc_wpt = 0;
        }

        if (transition->count() > 0)
        {
            procedure.asSID()->transitions().append(transition);
        }
        else
        {
            Logger::log(QString("Navdata:parseLevelDProcedureWaypoint: skipping empty transition %1 of SID %2").
                        arg(transition->id()).arg(procedure.id()));

            delete transition;
            transition = 0;
        }

        return 0;
    }
    else if (element.nodeName() == "Sid_Waypoint" ||
             element.nodeName() == "Star_Waypoint" ||
             element.nodeName() == "App_Waypoint" ||
             element.nodeName() == "SidTr_Waypoint" ||
             element.nodeName() == "AppTr_Waypoint")
    {
        QString wpt_name = element.namedItem("Name").toElement().text().trimmed();
        QString wpt_type = element.namedItem("Type").toElement().text();
        double lat = element.namedItem("Latitude").toElement().text().toDouble();
        double lon = element.namedItem("Longitude").toElement().text().toDouble();

        Waypoint* parsed_wpt = 0;

        //TODO add more waypoint types (hdg2alt, etc.)
        if (wpt_type == "Runway")
        {
            if (wpt_name.startsWith("RW")) wpt_name = wpt_name.mid(2);

            parsed_wpt = new Runway(wpt_name, lat, lon,
                                    element.namedItem("Hdg_Crs_value").toElement().text().toUInt(),
                                    0, false, 0, 0,
                                    element.namedItem("Altitude").toElement().text().toUInt(),
                                    0, 0);

            // set the approach runway list when we got a runway waypoint
            if (procedure.asApproach() != 0)
                procedure.asApproach()->setRunwayList(QStringList(wpt_name));
        }
        else if (wpt_type == "Hold")
        {
            if (procedure.asApproach() != 0 && procedure.count() > 0)
            {
                Waypoint* last_procedure_wpt = procedure.waypoint(procedure.count()-1);
                MYASSERT(last_procedure_wpt != 0);

                Holding holding;
                holding.setHoldingTrack(element.namedItem("Hdg_Crs_value").toElement().text().toUInt());
                holding.setIsLeftHolding(element.namedItem("Hld_Turn").toElement().text() == "Left");

                if (element.namedItem("Hld_Time_or_Dist").toElement().text() == "Time")
                    holding.setHoldLegLengthMin(element.namedItem("Hld_td_value").toElement().text().toDouble());

                last_procedure_wpt->setHolding(holding);
            }

        }
        else if (wpt_type == "Normal")
        {
            parsed_wpt = new Waypoint(wpt_name, "", lat, lon);
        }
        else if (wpt_type == "ConstHdgtoAlt")
        {
            parsed_wpt = new WaypointHdgToAlt(
                wpt_name, element.namedItem("Hdg_Crs_value").toElement().text().toUInt());
        }
        else if (wpt_type == "Intc")
        {
            Navcalc::TURN_DIRECTION turndir = Navcalc::TURN_AUTO;

            if (element.namedItem("Sp_Turn").toElement().text() == "Right")     turndir = Navcalc::TURN_RIGHT;
            else if (element.namedItem("Sp_Turn").toElement().text() == "Left") turndir = Navcalc::TURN_LEFT;

            parsed_wpt = new WaypointHdgToIntercept(
                wpt_name, 0.0, 0.0, Waypoint("Fix2Intercept", "", lat, lon),
                element.namedItem("RadialtoIntercept").toElement().text().toUInt(),
                element.namedItem("Hdg_Crs_value").toElement().text().toUInt(),
                turndir);
            //TODO set if to hold the heading or the course!
        }
        else
        {
            Logger::log(QString("Navdata::parseLevelDProcedureWaypoint: skipped wpt %1 of %2 - unsupported type %3").
                        arg(wpt_name).arg(procedure.id()).arg(wpt_type));
        }

        if (parsed_wpt == 0) return 0;

        // parse the waypoint info

        parsed_wpt->restrictions().setOverflyRestriction(false);
        if (!element.namedItem("Flytype").isNull() && element.namedItem("Flytype").toElement().text() == "Fly-by")
            parsed_wpt->restrictions().setOverflyRestriction(false);
        if (!element.namedItem("Flytype").isNull() && element.namedItem("Flytype").toElement().text() == "Fly-over")
            parsed_wpt->restrictions().setOverflyRestriction(true);

        if (!element.namedItem("Speed").isNull())
            parsed_wpt->restrictions().setSpeedRestrictionKts(element.namedItem("Speed").toElement().text().toUInt());

        if (!element.namedItem("Altitude").isNull())
            parsed_wpt->restrictions().setAltitudeRestrictionFt(element.namedItem("Altitude").toElement().text().toUInt());

        if (!element.namedItem("AltitudeRestriction").isNull())
        {
            const QString& alt_res = element.namedItem("AltitudeRestriction").toElement().text();

            if (alt_res == "above")
            {
                parsed_wpt->restrictions().setAltitudeRestrictionType(WaypointRestrictions::RESTRICTION_ALT_GREATER);
                if (element.nodeName() == "Sid_Waypoint" && element.namedItem("Type").toElement().text() == "Normal")
                    parsed_wpt->restrictions().setOverflyRestriction(true);
            }

            if (alt_res == "below")
                parsed_wpt->restrictions().setAltitudeRestrictionType(WaypointRestrictions::RESTRICTION_ALT_SMALLER);
            if (alt_res == "at")
                parsed_wpt->restrictions().setAltitudeRestrictionType(WaypointRestrictions::RESTRICTION_ALT_EQUAL);
        }

        if (wpt_type == "ConstHdgtoAlt")
        {
            if (parsed_wpt->restrictions().altitudeRestrictionFt() <= 0)
            {
                Logger::log(QString("Navdata::parseLevelDProcedureWaypoint: "
                                    "skipped wpt %1 of %2 - hdg2alt and no alt restriction").
                            arg(wpt_name).arg(procedure.id()));
                delete parsed_wpt;
                parsed_wpt = 0;
            }
        }

        return parsed_wpt;
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////

void Navdata::normalizeID(QString& id) const
{
    id = id.toUpper();

    for (int index=0; index <id.length(); ++index)
    {
        const QChar& character = id[index];
        if ((character >= '0' && character <= '9') ||
            (character >= 'A' && character <= 'Z'))
        {
            // everything is right
        }
        else
        {
            id[index] = ' ';
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

Vor* Navdata::getNavaid(const QString& id, const QString& wanted_type, const QString& country_code)
{
    MYASSERT(!id.isEmpty());
    MYASSERT(!wanted_type.isEmpty());

    WaypointPtrList possible_navaid_list;
    if (getNavaids(id, possible_navaid_list) > 0)
    {
        WaypointPtrListIterator iter(possible_navaid_list);
        for(; iter.hasNext(); )
        {
            const Waypoint* navaid = iter.next();
            MYASSERT(navaid);
            if (navaid->type() == wanted_type &&
                (country_code.isEmpty() || ((Vor*)navaid)->countryCode() == country_code))
                return (Vor*)navaid->deepCopy();
        }
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////

Vor* Navdata::getNdb(const QString& id, const QString& country_code)
{
    return getNavaid(id, Waypoint::TYPE_NDB, country_code);
}

/////////////////////////////////////////////////////////////////////////////

Vor* Navdata::getVor(const QString& id, const QString& country_code)
{
    return getNavaid(id, Waypoint::TYPE_VOR, country_code);
}

/////////////////////////////////////////////////////////////////////////////

Airport* Navdata::getAirport(const QString& id)
{
    MYASSERT(!id.isEmpty());
    WaypointPtrList airport_list;
    if (getAirports(id, airport_list) <= 0) return 0;
    Waypoint* wpt = airport_list[0];
    MYASSERT(wpt != 0);
    MYASSERT(wpt->type() == Waypoint::TYPE_AIRPORT);
    Airport* airport = (Airport*)wpt;
    return (Airport*)airport->deepCopy();
}

/////////////////////////////////////////////////////////////////////////////

Intersection* Navdata::getIntersection(const QString& id, const QString& country_code)
{
    MYASSERT(!id.isEmpty());

    WaypointPtrList possible_navaid_list;
    if (getIntersections(id, possible_navaid_list) > 0)
    {
        WaypointPtrListIterator iter(possible_navaid_list);
        for(; iter.hasNext(); )
        {
            const Waypoint* navaid = iter.next();
            MYASSERT(navaid);
            if (navaid->type() == Waypoint::TYPE_INTERSECTION &&
                (country_code.isEmpty() ||
                 ((Intersection*)navaid)->countryCode() == country_code))
                return (Intersection*)navaid->deepCopy();
        }
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////

uint Navdata::getAirportListByCoordinates(const Waypoint& current_position,
                                          int variation,
                                          uint max_distance_nm,
                                          WaypointPtrList& airports)
{
    MYASSERT(variation >= 0);

    int lat_start = (int)current_position.lat() - variation;
    int lon_start = (int)current_position.lon() - variation;
    int lat_end   = (int)current_position.lat() + variation;
    int lon_end   = (int)current_position.lon() + variation;

    for(int lat_var=lat_start; lat_var <= lat_end; ++lat_var)
        for(int lon_var=lon_start; lon_var <= lon_end; ++lon_var)
        {
            QString index = getCoordinateIndex(lat_var, lon_var);

            if (!m_airport_list_coordinate_index_map.contains(index)) continue;

            const AirportList& airportlist = m_airport_list_coordinate_index_map[index];
            AirportList::const_iterator list_iter = airportlist.begin();
            for(; list_iter != airportlist.end(); ++list_iter)
            {
                const Airport& airport = *list_iter;
                if (Navcalc::getDistBetweenWaypoints(current_position, airport) > max_distance_nm) continue;
                airports.append(airport.deepCopy());
            }
        }

    return airports.count();
}

/////////////////////////////////////////////////////////////////////////////

uint Navdata::getVorListByCoordinates(const Waypoint& current_position,
                                      int variation,
                                      uint max_distance_nm,
                                      WaypointPtrList& vors)
{
    MYASSERT(variation >= 0);

    int lat_start = (int)current_position.lat() - variation;
    int lon_start = (int)current_position.lon() - variation;
    int lat_end   = (int)current_position.lat() + variation;
    int lon_end   = (int)current_position.lon() + variation;

    for(int lat_var=lat_start; lat_var <= lat_end; ++lat_var)
        for(int lon_var=lon_start; lon_var <= lon_end; ++lon_var)
        {
            QString index = getCoordinateIndex(lat_var, lon_var);

            if (!m_vor_list_coordinate_index_map.contains(index)) continue;

            const VorList& vorlist = m_vor_list_coordinate_index_map[index];
            VorList::const_iterator list_iter = vorlist.begin();
            for(; list_iter != vorlist.end(); ++list_iter)
            {
                const Vor& vor = *list_iter;

                if (Navcalc::getDistBetweenWaypoints(current_position, vor) > max_distance_nm) continue;
                vors.append(vor.deepCopy());
            }
        }

    return vors.count();
}

/////////////////////////////////////////////////////////////////////////////

uint Navdata::getNdbListByCoordinates(const Waypoint& current_position,
                                      int variation,
                                      uint max_distance_nm,
                                      WaypointPtrList& ndbs)
{
    MYASSERT(variation >= 0);

    int lat_start = (int)current_position.lat() - variation;
    int lon_start = (int)current_position.lon() - variation;
    int lat_end   = (int)current_position.lat() + variation;
    int lon_end   = (int)current_position.lon() + variation;

    for(int lat_var=lat_start; lat_var <= lat_end; ++lat_var)
        for(int lon_var=lon_start; lon_var <= lon_end; ++lon_var)
        {
            QString index = getCoordinateIndex(lat_var, lon_var);

            if (!m_ndb_list_coordinate_index_map.contains(index)) continue;

            const NdbList& ndblist = m_ndb_list_coordinate_index_map[index];
            NdbList::const_iterator list_iter = ndblist.begin();
            for(; list_iter != ndblist.end(); ++list_iter)
            {
                const Ndb& ndb = *list_iter;

                if (Navcalc::getDistBetweenWaypoints(current_position, ndb) > max_distance_nm) continue;
                ndbs.append(ndb.deepCopy());
            }
        }

    return ndbs.count();
}

/////////////////////////////////////////////////////////////////////////////

bool Navdata::getWaypointsByAirway(const Waypoint& from_waypoint,
                                   const QString& airway,
                                   const QString& to_waypoint,
                                   WaypointPtrList& result_wpt_list,
                                   QString& error_text) const
{
    Logger::log(QString("Navdata:getWaypointsByAirway: Searching for airway (%1) from (%2) to (%3)").
                arg(airway).arg(from_waypoint.id()).arg(to_waypoint));

    AirwayPtrList found_airways;
    if (!getAirways(airway, found_airways) || found_airways.count() <= 0)
    {
        error_text = QString("No Airway %1 from %2 to %3").arg(airway).arg(from_waypoint.id()).arg(to_waypoint);
        Logger::log(QString("Navdata:getWaypointsByAirway: %1").arg(error_text));
        return false;
    }

    // search for the right airway

    bool finished = false;

    AirwayPtrListIterator airway_iter(found_airways);
    for (; airway_iter.hasNext();)
    {
        const Airway* airway = airway_iter.next();

//         Logger::log(QString("Navdata:getWaypointsByAirway: loop airway (%1) with %2 waypoints").
//                     arg(airway->id()).arg(airway->count()));

        bool found_from_waypoint = false;
        result_wpt_list.clear();

        const WaypointPtrList& waypoint_list = airway->waypointList();
        WaypointPtrListIterator wpt_iter(waypoint_list);
        for(; wpt_iter.hasNext();)
        {
            Waypoint* waypoint = wpt_iter.next();
            waypoint->setParent(airway->id());

            //Logger::log(QString("Navdata:getWaypointsByAirway: loop waypoint (%1)").arg(waypoint->id()));

            if (*waypoint == from_waypoint)
            {
                found_from_waypoint = true;
                continue;
            }

            if (found_from_waypoint)
            {
                // search for navaid with the waypoint name to get more infos
                bool added_navaid = false;
                WaypointPtrList possible_navaid_list;
                if (getNavaids(waypoint->id(), possible_navaid_list) > 0)
                {
                    WaypointPtrListIterator navaid_iter(possible_navaid_list);
                    for(; navaid_iter.hasNext(); )
                    {
                        Waypoint* navaid = navaid_iter.next();

                        if (*navaid == *waypoint)
                        {
                            navaid->setParent(airway->id());
                            result_wpt_list.append(navaid->deepCopy());
                            added_navaid = true;
                            break;
                        }
                    }
                }

                if (!added_navaid) result_wpt_list.append(waypoint->deepCopy());

//                 Logger::log(QString("Navdata:getWaypointsByAirway: added waypoint %1").
//                             arg(waypoint->id()));

                if (waypoint->id() == to_waypoint)
                {
                    finished = true;
                    break;
                }
            }
        }

        if (finished) break;
    }

    if (!finished)
    {
        Logger::log("Navdata:getWaypointsByAirway: airway not found");
        error_text = "Airway not found";
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////

// determine case-sensitivity for loading the procedure-files
bool Navdata::isOnCaseSensitiveFilesystem() const
{
    QString path = VasPath::getPath()+"/docs";
    QString filename = "gpl.txt";
    QFileInfo upperFI(path+"/"+filename.toUpper());
    QFileInfo lowerFI(path+"/"+filename.toLower());

    return (upperFI != lowerFI);
}

/////////////////////////////////////////////////////////////////////////////

void Navdata::renameNavdataFilenamesToLower(const QString& relative_path) const
{
    Logger::log(QString("Navdata:renameNavdataFilenamesToLower: path=%1").
                arg(relative_path));

    QDir dir(relative_path);
    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i)
    {
        QFileInfo fileInfo = list.at(i);
        QString name(fileInfo.fileName());
        if (name.toLower() != name) dir.rename(name,name.toLower());
    }
}

/////////////////////////////////////////////////////////////////////////////
