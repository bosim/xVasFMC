///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2009 Alexander Wemmer 
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

/*! \file    bithandling.h
    \author  Alexander Wemmer, alex@wemmer.at
*/


#ifndef __BITHANDLING_H__
#define __BITHANDLING_H__

//! bit handling
class BitHandling 
{
public:
    //! Standard Constructor
    BitHandling() {};

    //! Destructor
    virtual ~BitHandling() {};

    inline static unsigned char getByteFromBits(bool bit_0,
                                                bool bit_1,
                                                bool bit_2,
                                                bool bit_3,
                                                bool bit_4,
                                                bool bit_5,
                                                bool bit_6,
                                                bool bit_7)
    {
        return (bit_0 ? 1:0) +
            (bit_1 ? 2:0) +
            (bit_2 ? 4:0) +
            (bit_3 ? 8:0) +
            (bit_4 ? 16:0) +
            (bit_5 ? 32:0) +
            (bit_6 ? 64:0) +
            (bit_7 ? 128:0);
    }

protected:

private:
    //! Hidden copy-constructor
    BitHandling(const BitHandling&);
    //! Hidden assignment operator
    const BitHandling& operator = (const BitHandling&);
};

#endif /* __BITHANDLING_H__ */

// End of file

