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

/*! \file    chart.elem.info.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __CHART_ELEM_INFO_H__
#define __CHART_ELEM_INFO_H__

#include <QDate>
#include <QDomElement>

#include "treebase_xml_chart.h"

class Airport;
class ChartModel;

/////////////////////////////////////////////////////////////////////////////

//! info element for charts
class ChartElemInfo : public TreeBaseXMLChart
{
    Q_OBJECT

public:
    //! Standard Constructor
    ChartElemInfo(TreeBaseXMLChart* parent, ChartModel* chart_model);

    //! Destructor
    virtual ~ChartElemInfo();

    virtual bool loadFromDomElement(QDomElement& dom_element, 
                                    QDomDocument& dom_doc,
                                    QString& err_msg);

    virtual bool saveToDomElement();

    QString getAirportId() const;
    QString getCity() const;
    QString getCountry() const;
    QString getChartName() const;
    QDate getEffDate() const;
    double getMagneticVariation() const;

    void setAirportId(const QString& id);
    void setCity(const QString& city);
    void setCountry(const QString& ctry);
    void setChartName(const QString& name);
    void setEffDate(const QDate& date);
    void setMagneticVariation(const double& magvar);

signals:

    void signalNewMagneticVariation(const double& variation);

private:
    //! Hidden copy-constructor
    ChartElemInfo(const ChartElemInfo&);
    //! Hidden assignment operator
    const ChartElemInfo& operator = (const ChartElemInfo&);
};

#endif /* __CHART_ELEM_INFO_H__ */

// End of file

