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

/*! \file    ge_airport.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __GE_AIRPORT_H__
#define __GE_AIRPORT_H__

#include "airport.h"
#include "ge_projection.h"
#include "ge_label.h"

class Airport;

/////////////////////////////////////////////////////////////////////////////

//! graphic AIRPORT representation
class GEAirport : public GraphicElementProjectionBase, public AnchorLatLon
{
    Q_OBJECT

public:
    //! Standard Constructor
    GEAirport(const ProjectionBase* projection, const Airport* airport);

    //! Destructor
    virtual ~GEAirport();

    void draw(QPainter& painter);

    const Airport* getAirport() const { return m_airport; }
    GELabel* getLabel() const { return m_label; }
    QPointF getAnchor() const { return m_airport->getLatLon(); }

protected:

    void recalcBoundingAndSelectorElements();
    
    Airport* m_airport;
    GELabel* m_label;    
    
private:
    //! Hidden copy-constructor
    GEAirport(const GEAirport&);
    //! Hidden assignment operator
    const GEAirport& operator = (const GEAirport&);
};

#endif /* __GE_AIRPORT_H__ */

// End of file

