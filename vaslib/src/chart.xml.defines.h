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

/*! \file    chart.xml.defines.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __CHART_XML_DEFINES_H__
#define __CHART_XML_DEFINES_H__

#include <QDomElement>
#include <QString>
#include "assert.h"

/////////////////////////////////////////////////////////////////////////////

#define CHART_NODE_NAME_ROOT "vaschart"

#define CHART_NODE_NAME_PROJECTION "projection"
#define CHART_NODE_NAME_PROJECTION_CENTER "center"
#define CHART_NODE_NAME_PROJECTION_RANGE "range"

#define CHART_NODE_NAME_NAVAIDS "navaids"
#define CHART_NODE_NAME_NAVAIDS_VOR "vor"
#define CHART_NODE_NAME_NAVAIDS_NDB "ndb"
#define CHART_NODE_NAME_NAVAIDS_INTERSECTION "int"
#define CHART_NODE_NAME_NAVAIDS_AIRPORT "apt"

#define CHART_NODE_NAME_INFO "info"
#define CHART_NODE_NAME_INFO_CHARTNAME "chartname"
#define CHART_NODE_NAME_INFO_EFFDATE "effdate"
#define CHART_NODE_NAME_INFO_AIRPORT "airport"
#define CHART_NODE_NAME_INFO_VARIATION "variation"

#define CHART_NODE_NAME_ROUTES "routes"
#define CHART_NODE_NAME_ROUTES_ROUTE "route"
#define CHART_NODE_NAME_ROUTES_FIX "fix"

#define CHART_NODE_NAME_TEXTS "texts"
#define CHART_NODE_NAME_TEXTS_TEXT "text"

#define CHART_ATTR_TYPE "type"
#define CHART_ATTR_VALUE "value"
#define CHART_ATTR_LAT "lat"
#define CHART_ATTR_LON "lon"
#define CHART_ATTR_FREQ "freq"
#define CHART_ATTR_DME "dme"
#define CHART_ATTR_ID "id"
#define CHART_ATTR_NAME "name"
#define CHART_ATTR_TITLE "title"
#define CHART_ATTR_CTRY "ctry"
#define CHART_ATTR_CITY "city"
#define CHART_ATTR_EFF_DATE "effdate"
#define CHART_ATTR_ALT "alt"
#define CHART_ATTR_ELEV "elev"
#define CHART_ATTR_LABEL "label"
#define CHART_ATTR_LABEL_LAT_DIFF "label_lat_diff"
#define CHART_ATTR_LABEL_LON_DIFF "label_lon_diff"
#define CHART_ATTR_LABEL_X_DIFF "label_x_diff"
#define CHART_ATTR_LABEL_Y_DIFF "label_y_diff"
#define CHART_ATTR_LABEL_PARAM_ROUTE "#route"
#define CHART_ATTR_LABEL_PARAM_ARROW "#arrow"
#define CHART_ATTR_POINT_X "point_x"
#define CHART_ATTR_POINT_Y "point_y"
#define CHART_ATTR_BORDER "border"
#define CHART_ATTR_ISLATLON "islatlon"
#define CHART_ATTR_REFPOINT "refpoint"

#define CHART_DATE_FORMAT "d.M.yyyy"

#define CHART_GE_LAYER_LABELS 300
#define CHART_GE_LAYER_NAVAIDS 200
#define CHART_GE_LAYER_ROUTES 100
#define CHART_GE_LAYER_TEXTRECT 80

#define CHART_LABEL_SEPARATOR ","

/////////////////////////////////////////////////////////////////////////////

//! Creates a new child node with the given name.
//! Return the (new) child node.
QDomElement createNode(QDomElement element, const QString& name);

/////////////////////////////////////////////////////////////////////////////

//! checks if the given element has at least one child node with the given
//! name. If not a new node will be created.
//! Return the (new) child node.
QDomElement checkAndCreateNode(QDomElement element, const QString& name);

/////////////////////////////////////////////////////////////////////////////

void writeDOMDoubleAttribute(QDomElement& element, 
                             const QString& attribute,
                             const double& value);

/////////////////////////////////////////////////////////////////////////////

void writeDOMIntAttribute(QDomElement& element, 
                          const QString& attribute,
                          const int& value);

/////////////////////////////////////////////////////////////////////////////

#endif /* __CHART_XML_DEFINES_H__ */

// End of file

