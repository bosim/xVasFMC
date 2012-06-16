#include "apxplane9standard.h"
#include "simdata.h"
#include "owneddata.h"
#include <XPLMDataAccess.h>
#include <XPLMUtilities.h>
#include "xpapiface_msg.h"
#include "navcalc.h"
#include <math.h>

APXPlane9Standard::APXPlane9Standard(std::ostream& logfile):
        LogicHandler(logfile),
        m_simAPState(0),
        m_simFDMode(0),
        m_simVVI(0),
        m_simHDG(0),
        m_simAPSPDisMach(0),
        m_simBarAlt(0),
        m_simAltWindow(0),
        m_internalAPState(0),
        m_internalVVI(0),
        m_internalHDG(0),
        m_logfile(logfile)
{}

APXPlane9Standard::~APXPlane9Standard()
{
    delete m_simAPState;
    delete m_simFDMode;
    delete m_simAltWindow;
    delete m_simVVI;
    delete m_simBarAlt;
    delete m_simAPSPDisMach;
    delete m_simHDG;

    delete m_internalAPState;
    delete m_internalVVI;
    delete m_internalHDG;
}

bool APXPlane9Standard::registerDataRefs()
{
    m_simAPState = new SimData<int>("sim/cockpit/autopilot/autopilot_state","Autopilot bitfield", RWType::ReadWrite);
    m_simFDMode = new SimData<int>("sim/cockpit/autopilot/autopilot_mode", "FD : Off, On, Auto", RWType::ReadWrite);
    m_simAltWindow = new SimData<float>("sim/cockpit/autopilot/altitude","Autopilot Alt window", RWType::ReadWrite);
    m_simVVI = new SimData<float>("sim/cockpit/autopilot/vertical_velocity","Autopilot VVI",RWType::ReadWrite);
    m_simBarAlt = new SimData<float>("sim/flightmodel/misc/h_ind","indicated barometric alt", RWType::ReadOnly);
    m_simAPSPDisMach = new SimData<bool>("sim/cockpit/autopilot/airspeed_is_mach","AP Spd shows mach",RWType::ReadWrite);
    m_simHDG = new SimData<float>("sim/cockpit/autopilot/heading_mag","XP AP HDG",RWType::ReadWrite);
    return true;
}

bool APXPlane9Standard::initState()
{
    m_internalAPState->set(1);
    m_internalHDG->set(m_simHDG->data());
    m_internalVVI->set(m_simVVI->data());
    /*m_joyRollInput = new SimData<float>("plugins/org/vasproject/xpfmcconn/joystick/roll_input_percent","Joystick roll input from joystick handler",RWType::ReadOnly);
    m_joyPitchInput = new SimData<float>("plugins/org/vasproject/xpfmcconn/joystick/pitch_input_percent","Joystick roll input from joystick handler",RWType::ReadOnly);*/
    return true;
}

bool APXPlane9Standard::processState()
{
    m_simAPState->poll();
    m_simFDMode->poll();
    m_simAltWindow->poll();
    m_simBarAlt->poll();
    m_simAPSPDisMach->poll();
    m_simVVI->poll();
    m_simHDG->poll();
    /*m_joyPitchInput->poll();
    m_joyRollInput->poll();*/

    float heading = m_simHDG->data();
    float last_received_heading = m_internalHDG->data();
    int xp_fd_mode = m_simFDMode->data();
    int xp_ap_state = m_simAPState->data();
    bool ap_spd_is_mach = m_simAPSPDisMach->data();
    float vs_request = m_internalVVI->data();

    if ( fabs(last_received_heading-heading) > 1 && fabs(last_received_heading - heading) < 359)
    {
        double ap_hdg_diff = -Navcalc::getSignedHeadingDiff(last_received_heading, heading);
        if (ap_hdg_diff > 20.0)
            heading = Navcalc::trimHeading(heading + 10.0);
        else if (ap_hdg_diff < -20.0)
            heading = Navcalc::trimHeading(heading - 10.0);
        else
            heading = last_received_heading;
        m_simHDG->set(heading);
    } else
    {
        m_internalHDG->set(Navcalc::round(heading));
    }
    m_internalVVI->set( qBound(-9000.0f, vs_request, 9000.0f) );
    if ( x_isAct(x_alt_arm) && x_isAct(x_vs) ) {
        m_simVVI->set(vs_request);
    }


    // first, handle fd modes (active / inactive)
    if(xp_fd_mode == 2)
    {
        // engage autopilot internally
        i_set( i_enabled );
    } else {
        // disengage AP
        i_unset( i_enabled );
    }

    // check the various lateral modes (hdg, lvl, loc)
    if ( x_isAct(x_hdg) )
    {
        // HDG hold is active
        i_set( i_hdg );
    }
    else
    {
        // HDG hold is inactive
        i_unset( i_hdg );
    }
    if ( x_isAct(x_lvl) )
    {
        // winglever means that hdg hold is inactive
        i_unset( i_hdg );
    }

    // recognize alt hold and alt arm (vs) modes
    if (xp_ap_state & 16384)
    {
        // ALT HOLD engage means ALT hold is on, VS is off, app is off, gs might be armed
        i_set( i_alt );
        i_unset( i_vs );
        i_unset( i_app );
    }
    if((xp_ap_state & 32) && (xp_ap_state & 16))
    {
        // VS is active, alt hold is armed, means ALT and VS is active, gs is off, app is off
        i_set( i_alt );
        i_set( i_vs );
        i_unset( i_gs );
        i_unset( i_app );
        i_unset( i_appbc );
    }
    else if ((xp_ap_state & 32) && (xp_ap_state & 64) && (xp_ap_state & 8))
    {
        // LVL CHG (spd hold) active, must be supressed.and aliased to vs
        x_set( x_vs );
    } else if (!(xp_ap_state & 16))
    {
        // no vertical speed mode active
        i_unset( i_vs );
    }

    // recognize autothrottle and speed hold modes
    if ( x_isAct(x_atr) )
    {
        if (ap_spd_is_mach)
        {
            i_set( i_mach );
            i_set( i_at_arm );
            i_unset( i_spd );
        } else {
            i_set( i_spd );
            i_set( i_at_arm );
            i_unset( i_mach );
        }
    }
    else
    {
        // ATHR is inactive
        i_unset( i_spd );
        i_unset( i_mach );
    }

    // now analyze approach modes
    if ( x_isAct(x_hnav_arm) || x_isAct(x_hnav_engage) )
    {
        // hnav is armed or enagaged
        i_set( i_nav1 );
    }
    if ( x_isAct(x_hnav_arm) && x_isAct(x_gs_arm) )
    {
        //hnav is armed and gls is armed
        i_set( i_nav1 );
        i_set( i_gs );
        i_set( i_app );
    }
    if ( x_isAct(x_hnav_engage) )
    {
        // hnav is engaged
        i_set( i_nav1 );
        i_unset( i_hdg );
    }
    if( x_isAct(x_hnav_engage) && x_isAct( x_gs_arm))
    {
        //hnav engaged and glideslope armed - no Localizer, but ILS mode, unset NAV1
        i_unset( i_nav1 );
        i_set( i_gs );
        i_set( i_app );
    }
    if ( x_isAct(x_hnav_engage) && x_isAct(x_gs_engage))
    {
        //hnav is engaged and gls is engaged;
        i_set( i_app );
        i_unset( i_gs );
        i_unset( i_nav1 );
        i_unset( i_hdg );
        i_unset( i_alt );
        i_unset( i_vs );
    }
    if (!(xp_ap_state &1024)&&!(xp_ap_state&2048)&&!(xp_ap_state&512)&&!(xp_ap_state&256))
    {
        // no approach modes are either armed or engaged
        i_unset ( i_gs);
        i_unset ( i_app);
        i_unset ( i_nav1);
    }
    return true;
}

bool APXPlane9Standard::processInput(long input)
{
    m_simAPState->poll();
    m_simFDMode->poll();
    m_simAltWindow->poll();
    m_simBarAlt->poll();

    float xp_ap_alt_window = m_simAltWindow->data();
    float xp_bar_alt = m_simBarAlt->poll();
    bool ap_spd_is_mach = m_simAPSPDisMach->data();

    if (input == XPAPIFACE_COMM_AP_AVAIL_TRUE)
    {
        i_set( i_avail );
        //internal_ap_state = internal_ap_state | 1;
    }

    else if (input == XPAPIFACE_COMM_AP_AVAIL_FALSE)
    {
        m_internalAPState->set(0);
        XPLMCommandKeyStroke(xplm_key_otto_dis);
        m_simFDMode->set(0);
    }

    else if (input == XPAPIFACE_COMM_AP_ENABLED_TRUE)
    {
        i_set( i_enabled );
        m_simFDMode->set(2);
    }

    else if (input == XPAPIFACE_COMM_AP_ENABLED_FALSE)
    {
        i_unset( i_enabled );
        m_simFDMode->set(1);
    }

    else if (input == XPAPIFACE_COMM_HDG_HOLD_ON)
    {
        if ( i_isAct(i_hdg) )
        {
            // HDG is already active, do nothing
        } else {
            i_set( i_hdg );
            x_set( x_hdg );
        }
    }

    else if (input == XPAPIFACE_COMM_HDG_HOLD_OFF)
    {
        if ( i_isAct(i_hdg) )
        {
            i_unset( i_hdg );
            x_set( x_lvl );
        } else {
            // HDG is already inactive, do nothing
        }
    }

    else if (input == XPAPIFACE_COMM_ALT_HOLD_ON)
    {
        i_set( i_alt );
        if ( fabs(xp_ap_alt_window - xp_bar_alt) < alt_hold_engage_thresh_ft )
        {
            // we are already at our desired altitude
            x_set( x_alt_engage );
        } else
        {
            // we need to climb or descent
            if ( i_isAct(i_vs) )
            {
                m_simVVI->set(m_last_received_vs);
            } else
            {
                i_set( i_vs );
                x_set( x_vs );
                m_simVVI->set(m_last_received_vs);
            }
        }
    }

    else if (input == XPAPIFACE_COMM_ALT_HOLD_OFF)
    {
        if ( i_isAct( i_alt) )
        {
            m_last_received_vs = 0;
            i_unset( i_alt );
            x_unset( x_vs );
            x_unset( x_alt_engage );
        }
    }

    else if (input == XPAPIFACE_COMM_AT_ARM_ON)
    {
        i_set( i_at_arm );
    }

    else if (input == XPAPIFACE_COMM_AT_ARM_OFF)
    {
        if ( i_isAct( i_at_arm ) )
        {
            i_unset( i_at_arm );
            i_unset( i_at_toga );
            i_unset( i_spd );
            i_unset( i_mach );
        }
        x_unset( x_atr );
    }

    else if (input == XPAPIFACE_COMM_SPEED_HOLD_ON)
    {
        if ( i_isAct(i_at_arm) && !i_isAct(i_spd) )
        {
            // if AT is armed, engage Speed hold now
            i_set( i_spd );
            //internal_ap_state = internal_ap_state | 16;
            ap_spd_is_mach = false;
            m_simAPSPDisMach->set(false);
            x_set( x_atr );
        }
    }

    else if (input == XPAPIFACE_COMM_SPEED_HOLD_OFF)
    {
        if ( i_isAct(i_spd) )
        {
            i_unset( i_spd );
            x_unset( x_atr );
        }
    }

    else if (input == XPAPIFACE_COMM_MACH_HOLD_ON)
    {
        if ( i_isAct( i_at_arm ) && !(i_isAct( i_mach )) )
        {
            // if AT is armed, engage Mach hold now
            i_set( i_mach );
            ap_spd_is_mach = true;
            x_set( x_atr );
        }
    }

    else if (input == XPAPIFACE_COMM_MACH_HOLD_OFF)
    {
        if ( i_isAct( i_mach ) )
        {
            i_unset( i_mach );
            x_unset( x_atr );
        }
    }

    else if (input == XPAPIFACE_COMM_NAV1_HOLD_ON)
    {
        if ( !(i_isAct( i_nav1 )) )
        {
            i_set( i_nav1 );
            x_set( x_hnav_arm );
        }
    }

    else if (input == XPAPIFACE_COMM_NAV1_HOLD_OFF)
    {
        if ( i_isAct(i_nav1) )
        {
            i_unset( i_nav1 );
            x_unset( x_hnav_arm );
        }
    }

    else if (input == XPAPIFACE_COMM_APP_HOLD_ON)
    {
        if ( !(i_isAct(i_app)) )
        {
            i_set( i_nav1 );
            i_set( i_app );
            i_set( i_gs );
            x_set( x_hnav_arm );
            x_set( x_gs_arm );
        }
    }

    else if (input == XPAPIFACE_COMM_APP_HOLD_OFF)
    {
        if ( i_isAct( i_app ) )
        {
            i_unset( i_nav1 );
            i_unset( i_app );
            i_unset( i_gs );
            x_unset( x_hnav_arm );
            x_unset( x_gs_arm );
        }
    }

    return true;
}

bool APXPlane9Standard::publishData()
{
    m_internalAPState = new OwnedData<int>("plugins/org/vasproject/xpfmcconn/autopilot/autopilot_state");
    m_internalVVI = new OwnedData<float>("plugins/org/vasproject/xpfmcconn/autopilot/vertical_velocity");
    m_internalHDG = new OwnedData<float>("plugins/org/vasproject/xpfmcconn/autopilot/heading");

    m_internalHDG->set(m_simHDG->data());
    m_internalVVI->set(m_simVVI->data());
    m_internalAPState->set(0);

    m_internalAPState->registerRead();
    m_internalVVI->registerReadWrite();
    m_internalHDG->registerReadWrite();
    return (m_internalAPState->isRegistered() && m_internalVVI->isRegistered() && m_internalHDG->isRegistered());
}

bool APXPlane9Standard::unpublishData()
{
    m_internalAPState->unregister();
    m_internalVVI->unregister();
    m_internalHDG->unregister();

    delete m_internalAPState;
    m_internalAPState = 0;
    delete m_internalVVI;
    m_internalVVI = 0;
    delete m_internalHDG;
    m_internalHDG = 0;

    return true;
}

float APXPlane9Standard::processingFrequency()
{
    return -7;
}

void APXPlane9Standard::i_set(int func)
{
    m_internalAPState->set(m_internalAPState->data() | func);
}

void APXPlane9Standard::i_unset(int func)
{
    m_internalAPState->set(m_internalAPState->data() & ~func);
}

void APXPlane9Standard::i_tgl(int func)
{
    if (i_isAct(func))
        i_unset(func);
    else
        i_set(func);
}

bool APXPlane9Standard::i_isAct(int func)
{
    return (m_internalAPState->data() & func);
}

void APXPlane9Standard::x_set(int func)
{
    if (!x_isAct(func))
        x_tgl(func);
}

void APXPlane9Standard::x_unset(int func)
{
    if (x_isAct(func))
        x_tgl(func);
}

void APXPlane9Standard::x_tgl(int func)
{
    m_simAPState->set(func);
}

bool APXPlane9Standard::x_isAct(int func)
{
    return (m_simAPState->data() & func);
}
