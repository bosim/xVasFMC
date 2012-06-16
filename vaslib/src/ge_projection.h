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

/*! \file    ge_projection.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __GE_PROJECTION_H__
#define __GE_PROJECTION_H__

#include "ge.h"
#include "projection.h"

#define FULL_ARC 5760  // = 16*360

/////////////////////////////////////////////////////////////////////////////

//! graphic element base class
class GraphicElementProjectionBase : public GraphicElementBase
{
    Q_OBJECT

public:
    //! Standard Constructor
  GraphicElementProjectionBase(const ProjectionBase* projection, 
                               bool is_moveable = false);

    //! Destructor
    virtual ~GraphicElementProjectionBase() {};

protected slots:

    virtual void slotProjectionChanged() { m_projection_changed = true; }

protected:

    //! projection used to convert lat/lon to x/y
    const ProjectionBase* m_projection;

    bool m_projection_changed;

private:
    //! Hidden copy-constructor
    GraphicElementProjectionBase(const GraphicElementProjectionBase&);
    //! Hidden assignment operator
    const GraphicElementProjectionBase& operator = (const GraphicElementProjectionBase&);
};

#endif /* __GE_PROJECTION_H__ */

// End of file

