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

/*! \file    ge_ndb.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __GE_NDB_H__
#define __GE_NDB_H__

#include "ndb.h"
#include "ge_projection.h"
#include "ge_label.h"

class Ndb;

/////////////////////////////////////////////////////////////////////////////

//! graphic NDB representation
class GENdb : public GraphicElementProjectionBase, public AnchorLatLon
{
    Q_OBJECT

public:
    //! Standard Constructor
    GENdb(const ProjectionBase* projection, const Ndb* ndb);

    //! Destructor
    virtual ~GENdb();

    void draw(QPainter& painter);

    const Ndb* getNdb() const { return m_ndb; }
    GELabel* getLabel() const { return m_label; }
    QPointF getAnchor() const { return m_ndb->getLatLon(); }

protected:

    void recalcBoundingAndSelectorElements();
    
    Ndb* m_ndb;
    GELabel* m_label;

private:
    //! Hidden copy-constructor
    GENdb(const GENdb&);
    //! Hidden assignment operator
    const GENdb& operator = (const GENdb&);
};

#endif /* __GE_NDB_H__ */

// End of file
