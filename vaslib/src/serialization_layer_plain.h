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

/*! \file    serialization_layer_plain.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __SERIALIZATION_LAYER_PLAIN_H__
#define __SERIALIZATION_LAYER_PLAIN_H__

#include "serialization_layer_iface.h"

//! plain serialization
/*! more details ...
 */
class SerializationLayerPlain : public SerializationLayerIface
{
public:
    //! Standard Constructor
    SerializationLayerPlain(TransportLayerIface* transport_layer = 0);

    //! Destructor
    virtual ~SerializationLayerPlain();

    //! encodes the given container to the given array.
    //! returns true on success, false otherwise.
    virtual bool encode(const ContainerBaseList* container, QDataStream& stream);

    //! decodes the given array and returns a pointer to the decoded container. 
    //! ATTENTION: the caller is responsible to delete the returned container!!
    //! will return a null pointer on error.
    virtual ContainerBaseList* decode(QDataStream& stream);

protected:

private:
    //! Hidden copy-constructor
    SerializationLayerPlain(const SerializationLayerPlain&);
    //! Hidden assignment operator
    const SerializationLayerPlain& operator = (const SerializationLayerPlain&);
};



#endif /* __SERIALIZATION_LAYER_PLAIN_H__ */

// End of file

