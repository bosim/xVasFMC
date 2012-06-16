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

/*! \file    chart.elem.ndb.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __CHART_ELEM_NDB_H__
#define __CHART_ELEM_NDB_H__

#include "chart.elem.navaid.h"
#include "ge_ndb.h"
#include "ndb.h"

/////////////////////////////////////////////////////////////////////////////

//! ndb element for charts
class ChartElemNdb : public ChartElemNavaid
{
    Q_OBJECT

public:
    //! Standard Constructor
    ChartElemNdb(TreeBaseXMLChart* parent, ChartModel* chart_model, const Ndb& ndb);

    //! Destructor
    virtual ~ChartElemNdb();

    virtual bool loadFromDomElement(QDomElement& dom_element, 
                                    QDomDocument& dom_doc,
                                    QString& err_msg);

    virtual bool saveToDomElement();
    
    virtual const Waypoint* getNavaid() const { return m_ge_ndb->getNdb(); }

protected:

    GENdb* m_ge_ndb;

private:
    //! Hidden copy-constructor
    ChartElemNdb(const ChartElemNdb&);
    //! Hidden assignment operator
    const ChartElemNdb& operator = (const ChartElemNdb&);
};

#endif /* __CHART_ELEM_NDB_H__ */

// End of file

