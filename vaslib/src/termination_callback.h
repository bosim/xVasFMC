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

/*! \file    termination_callback.h
    \author  Alexander Wemmer, alex@wemmer.at
*/


#ifndef __TERMINATION_CALLBACK_H__
#define __TERMINATION_CALLBACK_H__

//! callback to let terminated objects inform others about their death, emitter side
class TermiationCallbackEmitter
{
};

/////////////////////////////////////////////////////////////////////////////

//! callback to let terminated objects inform others about their death, receiver side
class TermiationCallbackReceiver
{
public:
    virtual void terminated(TermiationCallbackEmitter* emitter) = 0;
};

#endif /* __TERMINATION_CALLBACK_H__ */

// End of file

