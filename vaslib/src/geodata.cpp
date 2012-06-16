///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2007 Alexander Wemmer 
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

/*! \file    geodata.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include <QApplication>
#include <QFile>
#include <QDateTime>
#include <QRectF>

#include "logger.h"
#include "gshhs.h"
#include "navcalc.h"
#include "vas_path.h"

#include "geodata.h"

/////////////////////////////////////////////////////////////////////////////

GeoData::GeoData()
{
}

/////////////////////////////////////////////////////////////////////////////

GeoData::~GeoData()
{
}

/////////////////////////////////////////////////////////////////////////////

bool GeoData::readData(uint filter_level)
{
    bool ret = true;
    m_route_list.clear();

    QTime readtimer;
    readtimer.start();

    long wpt_count = 0;

    QStringList::const_iterator iter = m_filename_list.begin();
    for(; iter != m_filename_list.end(); ++iter)
    {
        QFile file(VasPath::prependPath(*iter));
        if (!file.exists())
        {
            Logger::log(QString("GeoData:readData: file not found (%1)").arg(*iter));
            ret = false;
            continue;
        }

        if (!file.open(QIODevice::ReadOnly))
        {
            Logger::log(QString("GeoData:readData: could not open file for reading (%1)").arg(*iter));
            ret = false;
            continue;
        }

        Route* current_route = 0;

        while(!file.atEnd())
        {
            GSHHS header;
            if (file.read((char*)&header, sizeof(GSHHS)) != sizeof(GSHHS))
            {
                Logger::log(QString("GeoData:readData: could not read GSHHS header at pos (%1)").arg(file.pos()));
                ret = false;
                break;
            }

            if (QSysInfo::ByteOrder == QSysInfo::LittleEndian)
            {
                header.id = swabi4 ((unsigned int)header.id);
                header.n  = swabi4 ((unsigned int)header.n);
                header.level = swabi4 ((unsigned int)header.level);
                header.west  = swabi4 ((unsigned int)header.west);
                header.east  = swabi4 ((unsigned int)header.east);
                header.south = swabi4 ((unsigned int)header.south);
                header.north = swabi4 ((unsigned int)header.north);
                header.area  = swabi4 ((unsigned int)header.area);
                header.version  = swabi4 ((unsigned int)header.version);
                header.greenwich = swabi2 ((unsigned int)header.greenwich);
                header.source = swabi2 ((unsigned int)header.source);
            }

            current_route = new Route(QString::number(header.id));
            MYASSERT(current_route != 0);
            m_route_list.append(current_route);

            for(int wpt_index=0; wpt_index<header.n; ++wpt_index)
            {
                GSHHS_POINT point;
                if (file.read((char*)&point, sizeof(GSHHS_POINT)) != sizeof(GSHHS_POINT))
                {
                    Logger::log(QString("GeoData:readData: could not read GSHHS point at pos (%1)").arg(file.pos()));
                    ret = false;
                    break;
                }

                if (QSysInfo::ByteOrder == QSysInfo::LittleEndian)
                {
                    point.x = swabi4 ((unsigned int)point.x);
                    point.y = swabi4 ((unsigned int)point.y);
               }

                if (header.level > (int)filter_level) continue;
                
                current_route->appendWaypoint(Waypoint("p", QString::null, point.lat(), point.lon(header.greenwich)));
                ++wpt_count;
            }
        }
    }

    Logger::log(QString("GeoData:readData: added %1 waypoints in %2ms").arg(wpt_count).arg(readtimer.elapsed()));
    return ret;
}

/////////////////////////////////////////////////////////////////////////////

void GeoData::updateActiveRouteList(const Waypoint& center, int max_dist_nm)
{
    m_active_route_list.clear();

    QTime geotime;
    geotime.start();

    uint max = m_route_list.count();
    for(uint index=0; index<max; ++index)
    {
        Route* copy_route = new Route;
        MYASSERT(copy_route != 0);

        const Waypoint* prev_wpt = 0;
        bool out_of_range = false;

        WaypointPtrListIterator wpt_iter(m_route_list.at(index)->waypointList());
        while(wpt_iter.hasNext()) 
        {
            const Waypoint* wpt = wpt_iter.next();

            if (out_of_range)
            {
                if (Navcalc::getDistBetweenWaypoints(center, *wpt) > max_dist_nm)
                {
                    prev_wpt = wpt;

                    if (copy_route != 0)
                    {
                        if (copy_route->count() < 2)  delete copy_route;
                        else                          m_active_route_list.append(copy_route);
                        copy_route = 0;
                    }

                    continue;
                }

                if (copy_route == 0)
                {
                    copy_route = new Route();
                    MYASSERT(copy_route != 0);
                }

                if (prev_wpt != 0) copy_route->appendWaypoint(*prev_wpt);
                prev_wpt = 0;
                out_of_range = false;
            }

            copy_route->appendWaypoint(*wpt);
            out_of_range = Navcalc::getDistBetweenWaypoints(center, *wpt) > max_dist_nm;
        }

        if (copy_route != 0)
        {
            if (copy_route->count() < 2)  delete copy_route;
            else                          m_active_route_list.append(copy_route);
        }
    }

    //Logger::log(QString("GeoData:updateActiveRouteList: active route updated in %1ms").arg(geotime.elapsed()));

    emit signalActiveRouteChanged();
}

/////////////////////////////////////////////////////////////////////////////

void GeoData::calcProjection(const ProjectionBase& projection)
{
//     QTime geotime;
//     geotime.start();
    uint max = m_route_list.count();
    for(uint index=0; index<max; ++index) m_route_list.at(index)->calcProjection(projection);
    //Logger::log(QString("GeoData:calcProjection: processed projection in %1ms").arg(geotime.elapsed()));
}

/////////////////////////////////////////////////////////////////////////////

void GeoData::calcProjectionActiveRoute(const ProjectionBase& projection)
{
//     QTime geotime;
//     geotime.start();
    uint max = m_active_route_list.count();
    for(uint index=0; index<max; ++index) m_active_route_list.at(index)->calcProjection(projection);
    //Logger::log(QString("GeoData:calcProjectionActiveRoute: processed projection in %1ms").arg(geotime.elapsed()));
}

// End of file
