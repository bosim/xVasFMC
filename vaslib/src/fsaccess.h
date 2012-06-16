///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2007 Alexander Wemmer 
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

/*! \file    fsaccess.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef FSACCESS_H
#define FSACCESS_H

#include <QObject>
#include <QTime>
#include <QDate>
#include <QTimer>
#include <QHostAddress>
#include <QUdpSocket>

#include "config.h"
#include "logger.h"
#include "meanvalue.h"
#include "flightstatus.h"

/////////////////////////////////////////////////////////////////////////////

struct FMCStatusData
{
    FMCStatusData()
    {
        fd_engaged = false;
        athr_armed = false;
        athr_engaged = false;
        athr_speed_mode = false;
        athr_mach_mode = false;
        athr_n1_mode = false;
        ap_engaged = false;
        ap_both_app_mode = false;
        ap_horiz_hdg_mode = false;
        ap_horiz_lnav_mode = false;
        ap_horiz_loc_mode = false;
        ap_vert_vs_mode = false;
        ap_vert_flch_mode = false;
        ap_vert_vnav_mode = false;
        ap_vert_alt_hold = false;
        ap_vert_fpa_mode = false;
    }

    //-----

    bool fd_engaged;

    bool athr_armed;
    bool athr_engaged;
    bool athr_speed_mode;
    bool athr_mach_mode;
    bool athr_n1_mode;

    bool ap_engaged;
    bool ap_both_app_mode;

    bool ap_horiz_hdg_mode;
    bool ap_horiz_lnav_mode;
    bool ap_horiz_loc_mode;

    bool ap_vert_vs_mode;
    bool ap_vert_flch_mode;
    bool ap_vert_vnav_mode;
    bool ap_vert_alt_hold;
    bool ap_vert_fpa_mode;
};

/////////////////////////////////////////////////////////////////////////////

class FSAccess;

class FSAccessProvider
{
public:
    virtual FSAccess& fsAccess() = 0;
    virtual ~FSAccessProvider() {};
};

/////////////////////////////////////////////////////////////////////////////

//! Flightsim Access
class FSAccess : public QObject
{
    Q_OBJECT
    friend class FMCAutopilot;

public:

    enum MODE { MODE_MASTER = 0,
                MODE_SLAVE = 1
    };

    enum PushBack { PUSHBACK_STOP = 3,
                    PUSHBACK_STRAIGHT = 0,
                    PUSHBACK_LEFT = 1,
                    PUSHBACK_RIGHT = 2
    };

    //! Standard Constructor
    FSAccess(FlightStatus* flightstatus);

    //! Destructor
    virtual ~FSAccess() {};

    virtual Config* config() = 0;

    MODE mode() const { return m_mode; }
    void setMode(MODE mode) 
    {
        Logger::log(QString("FSAccess:setMode: (%1)").arg(mode));
        m_mode = mode; 
    }

    void setSeperateThrottleLeverMode(bool yes) { m_separate_throttle_lever_mode = yes; }

    //----- write access

    //! sets the NAV frequency, index 0 = NAV1, index 1 = NAV2
    virtual bool setNavFrequency(int freq, uint nav_index) = 0;
    //! sets the ADF frequency, index 0 = ADF1, index 1 = ADF2
    virtual bool setAdfFrequency(int freq, uint adf_index) = 0;
    //! sets the OBS angle, index 0 = NAV1, index 1 = NAV2
    virtual bool setNavOBS(int degrees, uint nav_index) = 0;

    virtual bool setAutothrustArm(bool armed) = 0;
    virtual bool setAutothrustSpeedHold(bool on) = 0;
    virtual bool setAutothrustMachHold(bool on) = 0;

    virtual bool setFDOnOff(bool on) = 0;
    virtual bool setAPOnOff(bool on) = 0;
    virtual bool setAPHeading(double heading) = 0;
    virtual bool setAPAlt(unsigned int alt) = 0;
    virtual bool setAPAirspeed(unsigned short speed_kts) = 0;
    virtual bool setAPMach(double mach) = 0;
    virtual bool setAPHeadingHold(bool on) = 0;
    virtual bool setAPAltHold(bool on) = 0;
    virtual bool setNAV1Arm(bool on) = 0;
    virtual bool setAPPArm(bool on) = 0;

	virtual bool setUTCTime(const QTime& utc_time) = 0;
	virtual bool setUTCDate(const QDate& utc_date) = 0;

    virtual bool freeThrottleAxes() = 0;
    virtual bool setThrottle(double percent) = 0;
    virtual bool setSBoxTransponder(bool on) = 0;
    virtual bool setSBoxIdent() = 0;

    virtual bool setFlaps(uint notch) = 0;

    virtual bool freeControlAxes() { return false; }
    virtual bool freeAileronAxis() { return false; }
    virtual bool freeElevatorAxis() { return false; }
    virtual bool setAileron(double percent) = 0;
    virtual bool setElevator(double percent) = 0;
    virtual bool setElevatorTrimPercent(double percent) = 0;
    virtual bool setElevatorTrimDegrees(double degrees) = 0;

    virtual bool freeSpoilerAxis() { return false; }
    virtual bool setSpoiler(double percent) = 0;

    virtual bool setAltimeterHpa(const double& hpa) = 0;

    virtual bool setPushback(PushBack) { return false; }

    virtual void writeFMCStatusToSim(const FMCStatusData&) {};

protected:

    //! this method shall only be called by FMCAutopilot
    virtual bool setAPVs(int vs_ft_min) = 0;

protected:

    inline int convertBCDToInt(int bcd)
    {
        return ((bcd & 0xF)) + (10  * ((bcd & 0xF0)  >> 4)) + 
            (100 * ((bcd & 0xF00) >> 8)) + (1000* ((bcd & 0xF000)>>12)); 
    }

    inline int convertIntToBCD(int value)
    {
        //QString cutted_freq = QString::number((value % 100000) / 10);
        QString value_string = QString::number(value);
        int bcd_value = 0;
        int hex_digit_inc = 1;

        for (int index = 0; index < value_string.length(); ++index)
        {
            bcd_value += hex_digit_inc * value_string.at(value_string.length()-1-index).digitValue();
            hex_digit_inc *= 16;
        }

        return bcd_value;
    }

protected:

    //! access to the flightstatus to write the gathered values to
    FlightStatus* m_flightstatus;

    //! whether we are in master or slave mode
    MODE m_mode;

    //! if enables, the engine data throttle level input percent values are set from separate throttle lever inputs
    bool m_separate_throttle_lever_mode;

private:
    //! Hidden copy-constructor
    FSAccess(const FSAccess&);
    //! Hidden assignment operator
    const FSAccess& operator = (const FSAccess&);
};


#endif /* __FSACCESS_H__ */

// End of file
