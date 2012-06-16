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

/*! \file    chartmodel.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __CHART_MODEL_H__
#define __CHART_MODEL_H__

#include <QDate>

#include "xml.model.h"
#include "waypoint.h"

class ProjectionBase;
class Navdata;
class GraphicElementLayerList;
class ChartElemRoot;
class ChartElemNavaids;
class ChartElemRoutes;
class ChartElemInfo;
class ChartElemTexts;
class Treebase;

/////////////////////////////////////////////////////////////////////////////

//! Data model for charts
class ChartModel : public XMLModel
{
    Q_OBJECT

public:
    //! Standard Constructor
    ChartModel(ProjectionBase* projection,
               Navdata* m_navdata,
               bool save_on_close = false);
    
    //! Destructor
    virtual ~ChartModel();

    //! clears all chart model content
    virtual void close();

    //! Loads the given file. If the file does not exist, a new file will be created.
    bool loadFromXMLFile(const QString& filename, QString& err_msg);

    //! parses the chart DOM and returns true on success
    virtual bool parseDOM(QString& err_msg);
    
    //-----

    ProjectionBase* getProjection() const { return m_projection; }
    Navdata* getNavdata() const { return m_navdata; }
    GraphicElementLayerList* getGELayerList() const { return m_ge_layer_list; }

    //-----

    ChartElemNavaids* getElemNavaids() const;
    ChartElemRoutes* getElemRoutes() const;
    ChartElemInfo* getElemInfo() const;
    ChartElemTexts* getElemTexts() const;

signals:

    void signalDoSaveQuestion(bool& m_save_on_close);
    void signalClosing();
    void signalClosed();
    void signalWaypointChoose(const WaypointList& waypoint_list, Waypoint** waypoint_to_insert);

protected:

    ChartElemRoot* m_chart_elem_root;
    GraphicElementLayerList* m_ge_layer_list;
    ProjectionBase* m_projection;
    Navdata* m_navdata;

private:
    //! Hidden copy-constructor
    ChartModel(const ChartModel&);
    //! Hidden assignment operator
    const ChartModel& operator = (const ChartModel&);
};

#endif /* __CHARTMODEL_H__ */

// End of file
