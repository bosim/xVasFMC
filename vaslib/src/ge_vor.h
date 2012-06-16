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

/*! \file    ge_vor.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __GE_VOR_H__
#define __GE_VOR_H__

#include "vor.h"
#include "ge_projection.h"
#include "ge_label.h"

/////////////////////////////////////////////////////////////////////////////

//! graphic VOR representation
class GEVor : public GraphicElementProjectionBase, public AnchorLatLon
{
    Q_OBJECT
    
public:
    //! Standard Constructor
    GEVor(const ProjectionBase* projection, const Vor* vor);

    //! Destructor
    virtual ~GEVor();

    void draw(QPainter& painter);

    const Vor* getVor() const { return m_vor; }

    GELabel* getLabel() const { return m_label; }

    QPointF getAnchor() const { return m_vor->getLatLon(); }
    
protected:

    void recalcBoundingAndSelectorElements();
    
    Vor* m_vor;
    GELabel *m_label;    

private:
    //! Hidden copy-constructor
    GEVor(const GEVor&);
    //! Hidden assignment operator
    const GEVor& operator = (const GEVor&);
};

#endif /* __GE_VOR_H__ */

// End of file
