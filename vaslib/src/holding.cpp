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

/*! \file    holding.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "logger.h"

#include "holding.h"

/////////////////////////////////////////////////////////////////////////////

Holding::Holding() :
    m_is_valid(0), m_holding_track(0), m_holding_out_track(0),
    m_is_inbound_fix_track(true), m_is_left_holding(true),
    m_hold_leg_length_nm(0.0), m_hold_leg_length_min(1.0),
    m_true_target_track(0), m_turn_to_target_track_with_left_turn(false),
    m_status(STATUS_INACTIVE), m_entry_type(ENTRY_INVALID),
    m_inbd_track_lat(0.0), m_inbd_track_lon(0.0), m_exit_holding(false)
{}

/////////////////////////////////////////////////////////////////////////////

Holding::~Holding() 
{}

/////////////////////////////////////////////////////////////////////////////

QString Holding::statusText(Status status) const
{
    switch(status)
    {
        case(STATUS_INACTIVE): return "INACTIVE"; break;
        case(STATUS_ENTRY_TO_FIX): return "ENTRY_TO_FIX"; break;
        case(STATUS_ENTRY_LEG_1): return "ENTRY_LEG1"; break;
        case(STATUS_ENTRY_TURN_1): return "ENTRY_TURN1"; break;
        case(STATUS_ENTRY_LEG_2): return "ENTRY_LEG2"; break;
        case(STATUS_INSIDE_TURN_1): return "INSIDE_TURN1"; break;
        case(STATUS_INSIDE_LEG_1): return "INSIDE_LEG1"; break;
        case(STATUS_INSIDE_TURN_2): return "INSIDE_TURN2"; break;
        case(STATUS_INSIDE_LEG_2): return "INSIDE_LEG2"; break;
        default: MYASSERT(0);
    }

    return "";
}

/////////////////////////////////////////////////////////////////////////////

QString Holding::entryTypeText(EntryType type) const
{
    switch(type)
    {
        case(ENTRY_INVALID): return "INVALID"; break;
        case(ENTRY_PARALLEL): return "PARA"; break;
        case(ENTRY_DIRECT_DCT): return "DCT"; break;
        case(ENTRY_DIRECT_INTERCEPT): return "DCT(I)"; break;
        case(ENTRY_TEARDROP): return "TEAR"; break;
        default: MYASSERT(0);
    }
    
    return "";
}

/////////////////////////////////////////////////////////////////////////////

QString Holding::toString() const
{
    return QString("Holding: T:%1 Min:%2/Nm:%3 "
                   "St:%4 Ent:%5 TTrack:%6deg/L:%7 ela:%8s").
        arg(m_holding_track).
        arg(m_hold_leg_length_min, 2, 'f', 1).
        arg(m_hold_leg_length_nm, 2, 'f', 1).
        arg(statusText(m_status)).
        arg(entryTypeText(m_entry_type)).
        arg(m_true_target_track).
        arg(m_turn_to_target_track_with_left_turn).
        arg(m_timer.elapsed()/1000);
}

/////////////////////////////////////////////////////////////////////////////

void Holding::operator<<(QDataStream& in)
{
    qint16 status, entry_type;

    in >> m_is_valid
       >> m_holding_track
       >> m_holding_out_track
       >> m_is_inbound_fix_track
       >> m_is_left_holding
       >> m_hold_leg_length_nm
       >> m_hold_leg_length_min
       >> m_true_target_track
       >> m_turn_to_target_track_with_left_turn
       >> status
       >> entry_type
       >> m_inbd_track_lat
       >> m_inbd_track_lon
       >> m_exit_holding;

    m_status = (Status)status;
    m_entry_type = (EntryType)entry_type;
}

/////////////////////////////////////////////////////////////////////////////

void Holding::operator>>(QDataStream& out) const
{
    out << m_is_valid
        << m_holding_track
        << m_holding_out_track
        << m_is_inbound_fix_track
        << m_is_left_holding
        << m_hold_leg_length_nm
        << m_hold_leg_length_min
        << m_true_target_track
        << m_turn_to_target_track_with_left_turn
        << (qint16)m_status
        << (qint16)m_entry_type
        << m_inbd_track_lat
        << m_inbd_track_lon
        << m_exit_holding;
}

// End of file
