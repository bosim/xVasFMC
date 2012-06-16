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

/*! \file    vroute.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include <QHttp>
#include <QByteArray>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNodeList>

#include "logger.h"
#include "assert.h"

#include "vroute.h"

/////////////////////////////////////////////////////////////////////////////

VRoute::VRoute(const QString& vroute_host, const QString& vroute_dir) : 
    m_vroute_host(vroute_host), m_vroute_dir(vroute_dir), m_http(new QHttp), m_last_id(0)
{
    MYASSERT(m_http != 0);
    m_http->setHost(m_vroute_host);    
    MYASSERT(connect(m_http, SIGNAL(requestFinished(int, bool)), this, SLOT(slotCmdFin(int, bool))));
}

/////////////////////////////////////////////////////////////////////////////

VRoute::~VRoute()
{
    delete m_http;
}

/////////////////////////////////////////////////////////////////////////////

void VRoute::requestFP(const QString& adep, const QString& ades)
{
    QString path = QString(m_vroute_dir).arg(adep.trimmed().left(4)).arg(ades.trimmed().left(4));

    if (!m_airac_restriction_string.isEmpty())
        path += QString("&cycle=%1").arg(m_airac_restriction_string);

    m_last_id = m_http->get(path);
}

/////////////////////////////////////////////////////////////////////////////

void VRoute::slotCmdFin(int id, bool error)
{
    if (id != m_last_id) return;

    if (error)
    {
        emit signalError(m_http->errorString());
        Logger::log(QString("VRoute:slotCmdFin: error occured (%s)").arg(m_http->errorString()));
        return;
    }

    QDomDocument xml_fp;
    if (!xml_fp.setContent(m_http->readAll()))
    {
        emit signalError("Could not parse XML FP");
        Logger::log("VRoute:slotCmdFin: coult not parse XML");
        return;
    }

    //Logger::log(QString("got FP:\n%1").arg(xml_fp.toString()));

    QDomElement root_element = xml_fp.documentElement();
    if (root_element.isNull())
    {
        emit signalError("FP XML root error");
        return;
    }

    QDomNodeList result_code_element_list = root_element.elementsByTagName("result_code");
    if (result_code_element_list.count() != 1)
    {
        emit signalError(QString("XML error %1 result codes").arg(result_code_element_list.count()));
        return;
    }

    if (result_code_element_list.at(0).toElement().text() != "200")
    {
        emit signalError(QString("XML result code (%1)").arg(result_code_element_list.at(0).toElement().text()));
        return;
    }

// <?xml version='1.0'?>
// <flightplans>
//  <result>
//   <version>1</version>
//   <num_objects>1</num_objects>
//   <result_code>200</result_code>
//  </result>
//  <flightplan source="EUroute" id="316ee2d909c8b8c21db2effcc6339313" >
//   <short_name>EUroute</short_name>
//   <full_name>EUroute Flight Plan Database</full_name>
//   <logo>http://www.euroutepro.com/euroute_logo.png</logo>
//   <link>http://www.euroutepro.com/</link>
//   <contact>support@euroutepro.com</contact>
//   <dep>LOWG</dep>
//   <arr>LOWW</arr>
//   <trans_alt_dep>0</trans_alt_dep>
//   <trans_alt_arr>0</trans_alt_arr>
//   <min_fl>70</min_fl>
//   <max_fl>150</max_fl>
//   <dep_proc/>
//   <arr_proc/>
//   <route>XANUT XANUT1W</route>
//   <full_route>XANUT XANUT1W</full_route>
//   <last_change>20080211135358</last_change>
//   <flags/>
//   <comments/>
//   <distance type="route" >86</distance>
//   <class>0</class>
//  </flightplan>
// </flightplans>

    CompactRouteList route_list;

    // extract received routes

    QDomNodeList fp_element_list = root_element.elementsByTagName("flightplan");
    if (fp_element_list.count() < 1)
    {
        emit signalError("No flightplan found");
        return;
    }

    for(int index=0; index < fp_element_list.count(); ++index)
    {
        QDomElement fp_element = fp_element_list.at(index).toElement();
        MYASSERT(!fp_element.isNull());

        CompactRoute route = CompactRoute();
        
        // traverse through the FP nodes

        QDomNode fpnode = fp_element.firstChild();
        while(!fpnode.isNull())
        {
            QDomElement fpelement = fpnode.toElement();
            if (fpelement.isNull()) continue;

            if (fpelement.tagName() == "dep") route.m_adep = fpelement.text();
            else if (fpelement.tagName() == "arr") route.m_ades = fpelement.text();
            else if (fpelement.tagName() == "route") route.m_route = fpelement.text();
            else if (fpelement.tagName() == "min_fl") route.m_min_fl = fpelement.text();
            else if (fpelement.tagName() == "max_fl") route.m_max_fl = fpelement.text();
            else if (fpelement.tagName() == "distance" &&
                     fpelement.hasAttribute("type") &&
                     fpelement.attribute("type") == "route") route.m_distance = fpelement.text();

            fpnode = fpnode.nextSibling();
        }

        if (route.isValid()) route_list.append(route);
    }

    emit signalGotRoute(route_list);
}

// End of file
