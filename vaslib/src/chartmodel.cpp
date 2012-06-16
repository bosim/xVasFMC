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

/*! \file    chartmodel.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include <QFile>
#include <QMetaObject>

#include "assert.h"
#include "config.h"
#include "projection.h"
#include "navdata.h"
#include "ge_layers.h"
#include "treebase.h"

#include "ge_rect.h"
#include "ge_vor.h"
#include "ge_ndb.h"
#include "ge_airport.h"
#include "ge_intersection.h"

#include "chart.xml.defines.h"
#include "chart.elem.root.h"
#include "chart.elem.projection.h"
#include "chart.elem.navaids.h"
#include "chart.elem.routes.h"
#include "chart.elem.info.h"
#include "chart.elem.texts.h"

#include "chartmodel.h"

/////////////////////////////////////////////////////////////////////////////

ChartModel::ChartModel(ProjectionBase* projection,
                       Navdata* navdata,
                       bool save_on_close) : 
    XMLModel(save_on_close), m_chart_elem_root(0), m_ge_layer_list(new GraphicElementLayerList),
    m_projection(projection), m_navdata(navdata)
{
    MYASSERT(m_ge_layer_list != 0);
    MYASSERT(m_projection != 0);
    MYASSERT(m_navdata != 0);

    MYASSERT(connect(m_navdata, SIGNAL(signalWaypointChoose(const WaypointList&, Waypoint**)),
                     this, SIGNAL(signalWaypointChoose(const WaypointList&, Waypoint**))));
}

/////////////////////////////////////////////////////////////////////////////

ChartModel::~ChartModel()
{ 
    printf("~ChartModel\n");
    fflush(stdout);
    close();
    delete m_ge_layer_list;
}

/////////////////////////////////////////////////////////////////////////////

void ChartModel::close()
{
    printf("ChartModel:close: dirty=%d\n", isDirty());
    fflush(stdout);

    if (isDirty() && !m_xml_dom.isNull()) emit signalDoSaveQuestion(m_save_on_close);
    emit signalClosing();
    if (m_chart_elem_root) m_chart_elem_root->clear();
    XMLModel::close();
    delete m_chart_elem_root;
    m_chart_elem_root = 0;
    m_ge_layer_list->clear();
    emit signalClosed();
    emit signalChanged();
}

/////////////////////////////////////////////////////////////////////////////

bool ChartModel::loadFromXMLFile(const QString& filename, QString& err_msg)
{
    if (!QFile::exists(filename))
    {
        QFile newfile(filename);
        if (!newfile.open(QIODevice::WriteOnly))
        {
            err_msg = QString("Could not create new file (%1)").arg(filename);
            printf("ChartModel:loadFromXMLFile: ERROR: Could not create (%s)\n",
                   filename.toLatin1().data());
            fflush(stdout);
            return false;
        }

        QDomDocument newdoc;
        QDomElement root = newdoc.createElement(CHART_NODE_NAME_ROOT);
        newdoc.appendChild(root);

        newfile.write(newdoc.toByteArray());
        newfile.close();
    }

    return XMLModel::loadFromXMLFile(filename, err_msg);
}

/////////////////////////////////////////////////////////////////////////////

bool ChartModel::parseDOM(QString& err_msg)
{
    // root node

    delete m_chart_elem_root;
    m_chart_elem_root = new ChartElemRoot(this);
    MYASSERT(m_chart_elem_root != 0);
    
    QDomElement dom_elem_base = m_xml_dom.documentElement();
    if (dom_elem_base.isNull()) 
    {
        err_msg = "Could not parse ROOT node";
        return false;
    }

    if (!m_chart_elem_root->loadFromDomElement(dom_elem_base, m_xml_dom, err_msg)) 
    {
        delete m_chart_elem_root;
        m_chart_elem_root = 0;
        return false;
    }

    // projection node

    QDomElement element = checkAndCreateNode(dom_elem_base, CHART_NODE_NAME_PROJECTION);
    ChartElemProjection* chart_elem_proj = new ChartElemProjection(m_chart_elem_root, this);
    MYASSERT(chart_elem_proj!= 0);
    m_chart_elem_root->addLeaf(CHART_NODE_NAME_PROJECTION, chart_elem_proj);
    if (!chart_elem_proj->loadFromDomElement(element, m_xml_dom, err_msg)) return false;

    // navaids node

    element = checkAndCreateNode(dom_elem_base, CHART_NODE_NAME_NAVAIDS);
    ChartElemNavaids* chart_elem_navaids = new ChartElemNavaids(m_chart_elem_root, this);
    MYASSERT(chart_elem_navaids!= 0);
    m_chart_elem_root->addLeaf(CHART_NODE_NAME_NAVAIDS, chart_elem_navaids);
    if (!chart_elem_navaids->loadFromDomElement(element, m_xml_dom, err_msg)) return false;

    // info node

    element = checkAndCreateNode(dom_elem_base, CHART_NODE_NAME_INFO);
    ChartElemInfo *chart_elem_info = new ChartElemInfo(m_chart_elem_root, this);
    MYASSERT(chart_elem_info != 0);
    m_chart_elem_root->addLeaf(CHART_NODE_NAME_INFO, chart_elem_info);
    if (!chart_elem_info->loadFromDomElement(element, m_xml_dom, err_msg)) return false;

    // routes node

    element = checkAndCreateNode(dom_elem_base, CHART_NODE_NAME_ROUTES);
    ChartElemRoutes* chart_elem_routes = new ChartElemRoutes(m_chart_elem_root, this);
    MYASSERT(chart_elem_routes!= 0);
    m_chart_elem_root->addLeaf(CHART_NODE_NAME_ROUTES, chart_elem_routes);
    if (!chart_elem_routes->loadFromDomElement(element, m_xml_dom, err_msg)) return false;

    // texts node

    element = checkAndCreateNode(dom_elem_base, CHART_NODE_NAME_TEXTS);
    ChartElemTexts* chart_elem_texts = new ChartElemTexts(m_chart_elem_root, this);
    MYASSERT(chart_elem_texts!= 0);
    m_chart_elem_root->addLeaf(CHART_NODE_NAME_TEXTS, chart_elem_texts);
    if (!chart_elem_texts->loadFromDomElement(element, m_xml_dom, err_msg)) return false;

    // finished parsing

    emit signalChanged();
    return true;
}

/////////////////////////////////////////////////////////////////////////////

ChartElemNavaids* ChartModel::getElemNavaids() const
{
    if (!m_chart_elem_root) return 0;
    const TreeBase* leaf = m_chart_elem_root->getLeaf(CHART_NODE_NAME_NAVAIDS);
    if (!leaf) return 0;
    MYASSERT(leaf->inherits("ChartElemNavaids"));
    return (ChartElemNavaids*)leaf;
}

/////////////////////////////////////////////////////////////////////////////

ChartElemRoutes* ChartModel::getElemRoutes() const
{
    if (!m_chart_elem_root) return 0;
    const TreeBase* leaf = m_chart_elem_root->getLeaf(CHART_NODE_NAME_ROUTES);
    if (!leaf) return 0;
    MYASSERT(leaf->inherits("ChartElemRoutes"));
    return (ChartElemRoutes*)leaf;
}

/////////////////////////////////////////////////////////////////////////////

ChartElemInfo* ChartModel::getElemInfo() const
{
    if (!m_chart_elem_root) return 0;
    const TreeBase* leaf = m_chart_elem_root->getLeaf(CHART_NODE_NAME_INFO);
    if (!leaf) return 0;
    MYASSERT(leaf->inherits("ChartElemInfo"));
    return (ChartElemInfo*)leaf;
}

/////////////////////////////////////////////////////////////////////////////

ChartElemTexts* ChartModel::getElemTexts() const
{
    if (!m_chart_elem_root) return 0;
    const TreeBase* leaf = m_chart_elem_root->getLeaf(CHART_NODE_NAME_TEXTS);
    if (!leaf) return 0;
    MYASSERT(leaf->inherits("ChartElemTexts"));
    return (ChartElemTexts*)leaf;
}

// End of file
