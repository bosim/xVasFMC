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

/*! \file    holding.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef HOLDING_H
#define HOLDING_H

#include <QString>
#include <QDateTime>

#include "serialization_iface.h"
#include "assert.h"

//! Holding definitions
class Holding 
{
public:

    enum EntryType { ENTRY_INVALID = 0,
                     ENTRY_PARALLEL,
                     ENTRY_DIRECT_DCT,
                     ENTRY_DIRECT_INTERCEPT,
                     ENTRY_TEARDROP
    };

    enum Status { STATUS_INACTIVE = 0,
                  STATUS_ENTRY_TO_FIX,
                  STATUS_ENTRY_LEG_1,
                  STATUS_ENTRY_TURN_1,
                  STATUS_ENTRY_LEG_2,
                  STATUS_INSIDE_TURN_1, // the turn after overflying the fix
                  STATUS_INSIDE_LEG_1, // the leg to/from the fix
                  STATUS_INSIDE_TURN_2, // the turn to overfly the fix
                  STATUS_INSIDE_LEG_2 // the leg "on the other side" of the fix
    };
    
    //! Standard Constructor
    Holding();

    //! Destructor
    virtual ~Holding();

    //-----

    bool isValid() const { return m_is_valid; }

    virtual QString toString() const;

    QString entryTypeText(EntryType type) const;
    QString statusText(Status status) const;

    //-----

    void setHoldingTrack(int holding_track)
    {
        m_holding_track = holding_track;
        m_holding_out_track = m_holding_track + 180;
        while(m_holding_out_track >= 360) m_holding_out_track -= 360;
        m_is_valid = true;
    }
    const int& holdingTrack() const { return m_holding_track; }
    const int& holdingOutTrack() const { return m_holding_out_track; }
    
    void setIsInboundFixTrack(bool is_inbound_fix_track)
    {
        m_is_inbound_fix_track = is_inbound_fix_track;
        m_is_valid = true;
    }
    const bool& isInboundFixTrack() const { return m_is_inbound_fix_track; }
    
    void setIsLeftHolding(bool is_left_holding)
    {
        m_is_left_holding = is_left_holding;
        m_is_valid = true;
    }
    const bool& isLeftHolding() const { return m_is_left_holding; }

    QString holdTurnDirText() const { return (m_is_left_holding ? "L" : "R"); }
    
    void setHoldLegLengthNm(const double& hold_leg_length_nm)
    {
        m_hold_leg_length_nm = hold_leg_length_nm;
        m_hold_leg_length_min = 0.0;
        m_is_valid = true;
    }
    const double& holdLegLengthNm() const { return m_hold_leg_length_nm; }
    
    void setHoldLegLengthMin(const double& hold_leg_length_min)
    {
        m_hold_leg_length_min = hold_leg_length_min;
        m_hold_leg_length_nm = 0.0;
        m_is_valid = true;
    }
    const double& holdLegLengthMin() const { return m_hold_leg_length_min; }

    //-----

    void setTrueTargetTrack(const double true_target_track)
    {
        m_true_target_track = (int)(true_target_track+0.5);
    }

    void setTrueTargetTrack(const int true_target_track)
    {
        m_true_target_track = true_target_track;
    }
    int trueTargetTrack() const { return m_true_target_track; }

    void setTurnToTargetTrackWithLeftTurn(const bool turn_to_target_track_with_left_turn)
    {
        m_turn_to_target_track_with_left_turn = turn_to_target_track_with_left_turn;
    }
    bool turnToTargetTrackWithLeftTurn() const 
    { return m_turn_to_target_track_with_left_turn; }

    
    void setStatus(const Status status)
    {
        m_status = status;
    }
    Status status() const { return m_status; }

    bool isActive() const { return m_status != STATUS_INACTIVE; }

    void resetTimer() { m_timer.restart(); }
    int timerSeconds() const { return m_timer.elapsed() / 1000; }
    
    void setEntryType(const EntryType& entry_type)
    {
        m_entry_type = entry_type;
    }
    const EntryType& entryType() const { return m_entry_type; }

    void setInbdTrackLatLon(const double& inbd_track_lat, const double& inbd_track_lon)
    {
        m_inbd_track_lat = inbd_track_lat;
        m_inbd_track_lon = inbd_track_lon;
    }
    const double& inbdTrackLat() const { return m_inbd_track_lat; }
    const double& inbdTrackLon() const { return m_inbd_track_lon; }

    void setExitHolding(const bool exit_holding)
    {
        m_exit_holding = exit_holding;
    }
    bool exitHolding() const { return m_exit_holding; }

    virtual void operator<<(QDataStream& in);
    virtual void operator>>(QDataStream& out) const;

protected:

    bool m_is_valid;
    int m_holding_track;
    int m_holding_out_track;
    bool m_is_inbound_fix_track;
    bool m_is_left_holding;
    double m_hold_leg_length_nm;
    double m_hold_leg_length_min;

    int m_true_target_track;
    bool m_turn_to_target_track_with_left_turn;
    Status m_status;
    QTime m_timer;
    EntryType m_entry_type;
    double m_inbd_track_lat;
    double m_inbd_track_lon;
    bool m_exit_holding;
};

#endif /* HOLDING_H */

// End of file

