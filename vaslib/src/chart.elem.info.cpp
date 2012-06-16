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

/*! \file    chart.elem.info.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "chart.xml.defines.h"
#include "airport.h"
#include "chartmodel.h"

#include "chart.elem.info.h"
#include "chart.elem.navaids.h"

/////////////////////////////////////////////////////////////////////////////

ChartElemInfo::ChartElemInfo(TreeBaseXMLChart* parent, ChartModel* chart_model) : 
    TreeBaseXMLChart(parent, chart_model)
{
}

/////////////////////////////////////////////////////////////////////////////

ChartElemInfo::~ChartElemInfo() 
{
    saveToDomElement();
}

/////////////////////////////////////////////////////////////////////////////

bool ChartElemInfo::loadFromDomElement(QDomElement& dom_element, 
                                       QDomDocument& dom_doc,
                                       QString& err_msg)
{
    MYASSERT(!dom_element.isNull());
    TreeBaseXML::loadFromDomElement(dom_element, dom_doc, err_msg);

    if (m_dom_element.tagName() != CHART_NODE_NAME_INFO)
    {
        err_msg = QString("Info: Wrong tag name (%1)").arg(m_dom_element.tagName());
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool ChartElemInfo::saveToDomElement()
{
    TreeBaseXML::saveToDomElement();
    return true;
}

/////////////////////////////////////////////////////////////////////////////

QString ChartElemInfo::getAirportId() const
{
    QDomElement element = m_dom_element.firstChildElement(CHART_NODE_NAME_INFO_AIRPORT);
    return element.attribute(CHART_ATTR_ID);
}

/////////////////////////////////////////////////////////////////////////////

QString ChartElemInfo::getCountry() const
{
    QDomElement element = m_dom_element.firstChildElement(CHART_NODE_NAME_INFO_AIRPORT);
    return element.attribute(CHART_ATTR_CTRY);
}

/////////////////////////////////////////////////////////////////////////////

QString ChartElemInfo::getCity() const
{
    QDomElement element = m_dom_element.firstChildElement(CHART_NODE_NAME_INFO_AIRPORT);
    return element.attribute(CHART_ATTR_CITY);
}

/////////////////////////////////////////////////////////////////////////////

QString ChartElemInfo::getChartName() const
{
    QDomElement element = m_dom_element.firstChildElement(CHART_NODE_NAME_INFO_CHARTNAME);
    return element.attribute(CHART_ATTR_NAME);
}

///////////////////////////////////////////////////////////////////////////////

QDate ChartElemInfo::getEffDate() const
{
    QDomElement element = m_dom_element.firstChildElement(CHART_NODE_NAME_INFO_EFFDATE);
    const QString& eff_dt_string = element.attribute(CHART_ATTR_VALUE);
    if (eff_dt_string.isEmpty()) return QDateTime::currentDateTime().date();
    return QDate::fromString(eff_dt_string, CHART_DATE_FORMAT);
}

/////////////////////////////////////////////////////////////////////////////

double ChartElemInfo::getMagneticVariation() const
{
    QDomElement element = m_dom_element.firstChildElement(CHART_NODE_NAME_INFO_VARIATION);
    return element.attribute(CHART_ATTR_VALUE).toDouble();
}

/////////////////////////////////////////////////////////////////////////////

void ChartElemInfo::setAirportId(const QString& id)
{
    QDomElement element = checkAndCreateNode(m_dom_element, CHART_NODE_NAME_INFO_AIRPORT);
    element.setAttribute(CHART_ATTR_ID, id);
}

/////////////////////////////////////////////////////////////////////////////

void ChartElemInfo::setCity(const QString& city)
{
    QDomElement element = checkAndCreateNode(m_dom_element, CHART_NODE_NAME_INFO_AIRPORT);
    element.setAttribute(CHART_ATTR_CITY, city);
}

/////////////////////////////////////////////////////////////////////////////

void ChartElemInfo::setCountry(const QString& ctry)
{
    QDomElement element = checkAndCreateNode(m_dom_element, CHART_NODE_NAME_INFO_AIRPORT);
    element.setAttribute(CHART_ATTR_CTRY, ctry);
}

/////////////////////////////////////////////////////////////////////////////

void ChartElemInfo::setChartName(const QString& name)
{
    QDomElement element = checkAndCreateNode(m_dom_element, CHART_NODE_NAME_INFO_CHARTNAME);
    element.setAttribute(CHART_ATTR_NAME, name);
}

/////////////////////////////////////////////////////////////////////////////

void ChartElemInfo::setEffDate(const QDate& date)
{
    QDomElement element = checkAndCreateNode(m_dom_element, CHART_NODE_NAME_INFO_EFFDATE);
    element.setAttribute(CHART_ATTR_VALUE, date.toString(CHART_DATE_FORMAT));
}

/////////////////////////////////////////////////////////////////////////////

void ChartElemInfo::setMagneticVariation(const double& magvar)
{
    QDomElement element = checkAndCreateNode(m_dom_element, CHART_NODE_NAME_INFO_VARIATION);
    writeDOMDoubleAttribute(element, CHART_ATTR_VALUE, magvar);
    emit signalNewMagneticVariation(magvar);
}

// End of file
