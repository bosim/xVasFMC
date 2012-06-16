///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2008 Alexander Wemmer 
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

/*! \file    serialization_iface.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __SERIALIZATION_IFACE_H__
#define __SERIALIZATION_IFACE_H__

class QDataStream;

/////////////////////////////////////////////////////////////////////////////

//! interface for classes to serialize themselves
class SerializationIface
{
public:

    SerializationIface() {};
    virtual ~SerializationIface() {};
  
    virtual void operator<<(QDataStream& in) = 0;
    virtual void operator>>(QDataStream& out) const = 0;
};

#endif /* __SERIALIZATION_IFACE_H__ */

// End of file

