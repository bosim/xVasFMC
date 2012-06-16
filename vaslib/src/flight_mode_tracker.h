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

/*! \file    flight_mode_tracker.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __FLIGHT_MODE_TRACKER_H__
#define __FLIGHT_MODE_TRACKER_H__

#include <QObject>
#include <QString>
#include <QTime>

class FlightStatus;
class FMCDataProvider;

/////////////////////////////////////////////////////////////////////////////

//! tracks the current flight mode
class FlightModeTracker : public QObject
{
    Q_OBJECT

public:

    enum FLIGHTMODE { FLIGHT_MODE_UNKNOWN,
                      FLIGHT_MODE_PREFLIGHT,
                      FLIGHT_MODE_TAXIING,
                      FLIGHT_MODE_TAKEOFF,   // on the ground
                      FLIGHT_MODE_CLIMB,     // in the air
                      FLIGHT_MODE_CRUISE,
                      FLIGHT_MODE_DESCENT,
                      FLIGHT_MODE_APPROACH,  // in the air
                      FLIGHT_MODE_LANDING,   // on the ground
                      FLIGHT_MODE_GOAROUND
    };

    //! Standard Constructor
    FlightModeTracker(const FlightStatus* flightstatus, const FMCDataProvider* fmc_data_provider);

    //! Destructor
    virtual ~FlightModeTracker();

    //-----

    FLIGHTMODE currentFlightMode() const { return m_current_flight_mode; }
    FLIGHTMODE prevFlightMode() const { return m_prev_flight_mode; }

    QString flightModeText(FLIGHTMODE mode) const;
    QString currentFlightModeText() const { return flightModeText(m_current_flight_mode); }
    QString prevFlightModeText() const { return flightModeText(m_prev_flight_mode); }

    inline bool isPreflight() const { return m_current_flight_mode == FLIGHT_MODE_PREFLIGHT; }
    inline bool isTaxiing() const { return m_current_flight_mode == FLIGHT_MODE_TAXIING; }
    inline bool isTakeoff() const { return m_current_flight_mode == FLIGHT_MODE_TAKEOFF; }
    inline bool isClimb() const { return m_current_flight_mode == FLIGHT_MODE_CLIMB; }
    inline bool isCruise() const { return m_current_flight_mode == FLIGHT_MODE_CRUISE; }
    inline bool isDescent() const { return m_current_flight_mode == FLIGHT_MODE_DESCENT; }
    inline bool isApproach() const { return m_current_flight_mode == FLIGHT_MODE_APPROACH; }
    inline bool isLanding() const { return m_current_flight_mode == FLIGHT_MODE_LANDING; }
    inline bool isGoAround() const { return m_current_flight_mode == FLIGHT_MODE_GOAROUND; }

    inline bool wasPreflight() const { return m_prev_flight_mode == FLIGHT_MODE_PREFLIGHT; }
    inline bool wasTaxiing() const { return m_prev_flight_mode == FLIGHT_MODE_TAXIING; }
    inline bool wasTakeoff() const { return m_prev_flight_mode == FLIGHT_MODE_TAKEOFF; }
    inline bool wasClimb() const { return m_prev_flight_mode == FLIGHT_MODE_CLIMB; }
    inline bool wasCruise() const { return m_prev_flight_mode == FLIGHT_MODE_CRUISE; }
    inline bool wasDescent() const { return m_prev_flight_mode == FLIGHT_MODE_DESCENT; }
    inline bool wasApproach() const { return m_prev_flight_mode == FLIGHT_MODE_APPROACH; }
    inline bool wasLanding() const { return m_prev_flight_mode == FLIGHT_MODE_LANDING; }
    inline bool wasGoAround() const { return m_prev_flight_mode == FLIGHT_MODE_GOAROUND; }    

    const QTime& timeOut() const { return m_time_out; }
    const QTime& timeOff() const { return m_time_off; }
    const QTime& timeOn() const { return m_time_on; }
    const QTime& timeIn() const { return m_time_in; }

signals:

   void signalFlightModeChanged(FLIGHTMODE new_mode);

public slots:

    virtual void slotCheckAndSetFlightMode();

protected:

    const FlightStatus* m_flightstatus;
    const FMCDataProvider* m_fmc_data_provider;

    QTime m_check_timer;

    FLIGHTMODE m_current_flight_mode;
    FLIGHTMODE m_prev_flight_mode;

    // OOOI times
    QTime m_time_out;
    QTime m_time_off;
    QTime m_time_on;
    QTime m_time_in;

private:
    //! Hidden copy-constructor
    FlightModeTracker(const FlightModeTracker&);
    //! Hidden assignment operator
    const FlightModeTracker& operator = (const FlightModeTracker&);
};

#endif /* __FLIGHT_MODE_TRACKER_H__ */

// End of file

