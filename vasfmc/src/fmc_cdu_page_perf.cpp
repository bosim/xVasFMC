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

/*! \file    fmc_cdu_page_perf.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "aircraft_data.h"

#include "fmc_autopilot.h"
#include "fmc_autothrottle.h"

#include "fmc_data.h"
#include "fmc_control.h"

#include "fmc_cdu_defines.h"
#include "fmc_cdu_page_manager.h"
#include "fmc_cdu_page_perf.h"

/////////////////////////////////////////////////////////////////////////////

FMCCDUPageStyleAPerformance::FMCCDUPageStyleAPerformance(const QString& page_name, FMCCDUPageManager* page_manager) :
    FMCCDUPageBase(page_name, page_manager), m_current_perf_page(PERF_PAGE_TAKE_OFF),
    m_last_flight_mode(FlightModeTracker::FLIGHT_MODE_UNKNOWN)
{
    MYASSERT(connect(&m_check_timer, SIGNAL(timeout()), this, SLOT(slotCheckAndSetPerfPage())));
}

/////////////////////////////////////////////////////////////////////////////

FMCCDUPageStyleAPerformance::~FMCCDUPageStyleAPerformance()
{
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAPerformance::slotCheckAndSetPerfPage(bool force)
{
    PERF_PAGE min_requested_perf_page = m_current_perf_page;
    
    if (m_fmc_data.approachPhaseActive())
    {
        min_requested_perf_page = PERF_PAGE_APPROACH;
    }
    else
    {
        switch(fmcControl().flightModeTracker().currentFlightMode())
        {
            case(FlightModeTracker::FLIGHT_MODE_PREFLIGHT):
            case(FlightModeTracker::FLIGHT_MODE_TAXIING):
            case(FlightModeTracker::FLIGHT_MODE_TAKEOFF):
                min_requested_perf_page = PERF_PAGE_TAKE_OFF;
                break;
            case(FlightModeTracker::FLIGHT_MODE_CLIMB): {
                if (m_fmc_data.normalRoute().accelerationAltitudeFt() > 0 &&
                    m_flightstatus->smoothed_altimeter_readout.lastValue() < m_fmc_data.normalRoute().accelerationAltitudeFt())
                    min_requested_perf_page = PERF_PAGE_TAKE_OFF;
                else
                    min_requested_perf_page = PERF_PAGE_CLIMB;
                break;
            }
            case(FlightModeTracker::FLIGHT_MODE_CRUISE):
                min_requested_perf_page = PERF_PAGE_CRUISE;
                break;
            case(FlightModeTracker::FLIGHT_MODE_DESCENT):
                min_requested_perf_page = PERF_PAGE_DESCENT;
                break;
            case(FlightModeTracker::FLIGHT_MODE_APPROACH):
                case(FlightModeTracker::FLIGHT_MODE_LANDING):
                min_requested_perf_page = PERF_PAGE_APPROACH;
                break;
            default:
                break;
        }
    }

    if (force || m_current_perf_page < min_requested_perf_page)
        m_current_perf_page = min_requested_perf_page;

    m_last_flight_mode = fmcControl().flightModeTracker().currentFlightMode();
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAPerformance::drawPrevPhaseText(QPainter& painter) const
{
    setFont(painter, NORM_FONT);

    if (m_flightstatus->onground)
    {
        drawTextLeft(painter, 1, 13, "<PHASE");
        setFont(painter, SMALL_FONT);
        drawTextLeft(painter, 2, 12, "PREV");
        setFont(painter, NORM_FONT);
    }
    else
    {
        if (!m_approach_phase_trigger)
        {
            drawTextLeft(painter, 1, 13, "<-APPR PHASE", CYAN);
            setFont(painter, SMALL_FONT);
            drawTextLeft(painter, 2, 12, "ACTIVATE", CYAN);
            setFont(painter, NORM_FONT);
        }
        else
        {
            drawTextLeft(painter, 1, 13, "*APPR PHASE", ORANGE);
            setFont(painter, SMALL_FONT);
            drawTextLeft(painter, 2, 12, "CONFIRM", ORANGE);
            setFont(painter, NORM_FONT);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

bool FMCCDUPageStyleAPerformance::changesOnActivePerfPageAllowed() const
{
    FlightModeTracker::FLIGHTMODE current_mode = fmcControl().flightModeTracker().currentFlightMode();

    switch(m_current_perf_page)
    {
        case(PERF_PAGE_TAKE_OFF):
            return current_mode < FlightModeTracker::FLIGHT_MODE_TAKEOFF;
        default: 
            return true;

//TODO needed later?
//         case(PERF_PAGE_CLIMB):
//         case(PERF_PAGE_CRUISE):
//         case(PERF_PAGE_DESCENT):
//         case(PERF_PAGE_APPROACH):
//         case(PERF_PAGE_GOAROUND):
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAPerformance::paintPage(QPainter& painter) const
{
    painter.setBackground(QBrush(BLACK));
    painter.fillRect(painter.window(), painter.background());

    bool changes_allowed = changesOnActivePerfPageAllowed();

    switch(m_current_perf_page)
    {
        case(PERF_PAGE_TAKE_OFF): {

            setFont(painter, NORM_FONT, QFont::Bold);
            
            drawTextCenter(painter, 1, "TAKE OFF", fmcControl().flightModeTracker().isTakeoff() ? GREEN : WHITE);

            //-----

            setFont(painter, SMALL_FONT);
            drawTextLeft(painter, 2, 2, "V1");
            drawTextLeft(painter, 2, 4, "VR");
            drawTextLeft(painter, 2, 6, "V2");
            drawTextLeft(painter, 2, 8, "TRANS ALT");
            drawTextLeft(painter, 2, 10, "THR RED/ACC");

            drawTextLeft(painter, 7, 2, "FLP RETR");
            drawTextLeft(painter, 7, 4, "SLT RETR");
            drawTextLeft(painter, 9, 6, "CLEAN");

            drawTextRight(painter, 2, 2, "RWY");
//             drawTextRight(painter, 2, 4, "TO SHIFT");
            drawTextRight(painter, 2, 6, "FLAPS/THS");
//            drawTextRight(painter, 2, 8, "TO-FLX TO");
//TODO replace by FLEX
            drawTextRight(painter, 2, 8, "FLEX TAKEOFF N1"); 
//            drawTextRight(painter, 2, 10, "ENG OUT ACC");

            drawTextRight(painter, 2, 12, "NEXT");

            //-----

            setFont(painter, NORM_FONT);

            QColor color = changes_allowed ? CYAN : GREEN;

            (m_fmc_data.V1() == 0) ?
                drawTextLeft(painter, 1, 3, QString(SPACER)+SPACER+SPACER, ORANGE) :
                drawTextLeft(painter, 1, 3, QString::number(m_fmc_data.V1()), color);
            
            (m_fmc_data.Vr() == 0) ?
                drawTextLeft(painter, 1, 5, QString(SPACER)+SPACER+SPACER, ORANGE) :
                drawTextLeft(painter, 1, 5, QString::number(m_fmc_data.Vr()), color);

            (m_fmc_data.V2() == 0) ?
                drawTextLeft(painter, 1, 7, QString(SPACER)+SPACER+SPACER, ORANGE) :
                drawTextLeft(painter, 1, 7, QString::number(m_fmc_data.V2()), color);

            (m_fmc_data.transitionAltitudeAdep() == 0) ?
                drawTextLeft(painter, 1, 9, QString(SPACER)+SPACER+SPACER+SPACER, ORANGE) :
                drawTextLeft(painter, 1, 9, QString::number(m_fmc_data.transitionAltitudeAdep()), CYAN);

            (m_fmc_data.normalRoute().thrustReductionAltitudeFt() == 0) ?
                drawTextLeft(painter, 1, 11, QString(SPACER)+SPACER+SPACER+SPACER, ORANGE) :
                drawTextLeft(painter, 1, 11, QString::number(m_fmc_data.normalRoute().thrustReductionAltitudeFt()), color);
            
            drawTextLeft(painter, 6, 11, "/", color);

            (m_fmc_data.normalRoute().accelerationAltitudeFt() == 0) ?
                drawTextLeft(painter, 7, 11, QString(SPACER)+SPACER+SPACER+SPACER, ORANGE) :
                drawTextLeft(painter, 7, 11, QString::number(m_fmc_data.normalRoute().accelerationAltitudeFt()), color);
            
            drawTextLeft(painter, 8, 3, "F=");
            drawTextLeft(painter, 10, 3, QString("%1").
                         arg(fmcControl().aircraftData().getMinimumAStyleFlapsRetractionSpeed(), 0, 'f', 0), GREEN);
            
            drawTextLeft(painter, 8, 5, "S=");
            drawTextLeft(painter, 10, 5, QString("%1").
                         arg(fmcControl().aircraftData().getMinimumAStyleSlatsRetractionSpeed(), 0, 'f', 0), GREEN);

            drawTextLeft(painter, 8, 7, "O=");
            drawTextLeft(painter, 10, 7, QString("%1").
                         arg((int)fmcControl().aircraftData().getGreenDotSpeed()), GREEN);

            if (fmcControl().normalRoute().departureAirport() != 0 &&
                fmcControl().normalRoute().departureAirport()->hasActiveRunway())
            {
                drawTextRight(painter, 1, 3, fmcControl().normalRoute().departureAirport()->activeRunwayId(), GREEN);
            }

            drawTextRight(painter, 1, 7, QString("%1/%2%3").
                          arg(m_fmc_data.takeoffFlapsNotch()).
                          arg((m_fmc_data.takeoffTrim() < 0) ? "DN" : "UP").
                          arg(qAbs(m_fmc_data.takeoffTrim()), 0, 'f', 1), color);

            (m_fmc_data.fmcTakeoffThrustN1() == 0.0) ?
                drawTextRight(painter, 1, 9, QString(SPACER)+SPACER+SPACER, ORANGE) :
                drawTextRight(painter, 1, 9, QString::number(m_fmc_data.fmcTakeoffThrustN1(), 'f', 1), color);

            drawTextRight(painter, 2, 13, "PHASE>");

            break;
        }

        case(PERF_PAGE_CLIMB): {
            setFont(painter, NORM_FONT, QFont::Bold);

            drawTextCenter(painter, 1, "CLB", fmcControl().flightModeTracker().isClimb() ? GREEN : WHITE);
            
            setFont(painter, SMALL_FONT);

            //TODOdrawTextLeft(painter, 2, 2, "ACT MODE");

            drawTextLeft(painter, 2, 8, "PRESEL SPD");
            drawTextLeft(painter, 2, 10, "PRESEL MACH");

            //TODO replace by FLEX
            drawTextRight(painter, 2, 8, "INITIAL CLIMB N1"); 
            drawTextRight(painter, 2, 10, "MAXIMUM CLIMB N1"); 

            drawTextRight(painter, 2, 12, "NEXT");

            setFont(painter, NORM_FONT);

            //TODOdrawTextLeft(painter, 1, 3, "SELECTED", GREEN);

            (m_fmc_data.climbSpeedKts() == 0) ?
                drawTextLeft(painter, 1, 9, "[    ]", CYAN) : 
                drawTextLeft(painter, 1, 9, QString::number(m_fmc_data.climbSpeedKts()), CYAN);

            (m_fmc_data.climbMach() == 0.0) ?
                drawTextLeft(painter, 1, 11, "[    ]", CYAN) : 
                drawTextLeft(painter, 1, 11, QString::number(m_fmc_data.climbMach()), CYAN);

            (m_fmc_data.fmcInitialClimbThrustN1() == 0.0) ?
                drawTextRight(painter, 1, 9, QString(SPACER)+SPACER+SPACER, ORANGE) :
                drawTextRight(painter, 1, 9, QString::number(m_fmc_data.fmcInitialClimbThrustN1(), 'f', 1), CYAN);
            
            (m_fmc_data.fmcMaxClimbThrustN1() == 0.0) ?
                drawTextRight(painter, 1, 11, QString(SPACER)+SPACER+SPACER, ORANGE) :
                drawTextRight(painter, 1, 11, QString::number(m_fmc_data.fmcMaxClimbThrustN1(), 'f', 1), CYAN);

            drawPrevPhaseText(painter);
            drawTextRight(painter, 1, 13, "PHASE>");
            break;
        }

        case(PERF_PAGE_CRUISE): {
            setFont(painter, NORM_FONT, QFont::Bold);

            drawTextCenter(painter, 1, "CRZ", fmcControl().flightModeTracker().isCruise() ? GREEN : WHITE);

            setFont(painter, SMALL_FONT);

            //TODOdrawTextLeft(painter, 2, 2, "ACT MODE");

            drawTextLeft(painter, 2, 8, "PRESEL");            

            if (m_fmc_data.distanceToTODNm() > 0.0)
            {
                drawTextRight(painter, 7, 5, "TO");
                drawTextRight(painter, 1, 6, "DIST");
            }

            drawTextRight(painter, 2, 12, "NEXT");

            setFont(painter, NORM_FONT);

            //TODOdrawTextLeft(painter, 1, 3, "SELECTED", GREEN);

            (m_fmc_data.cruiseMach() == 0.0) ?
                drawTextLeft(painter, 1, 9, "[    ]", CYAN) : 
                drawTextLeft(painter, 1, 9, QString::number(m_fmc_data.cruiseMach(), 'f', 2), CYAN);

            if (m_fmc_data.distanceToTODNm() > 0.0)
            {
                drawTextRight(painter, 1, 5, "(T/D)", GREEN);
                drawTextRight(painter, 1, 7, QString::number((int)m_fmc_data.distanceToTODNm()), GREEN);
            }

            drawPrevPhaseText(painter);
            drawTextRight(painter, 1, 13, "PHASE>");
            break;
        }

        case(PERF_PAGE_DESCENT): {
            setFont(painter, NORM_FONT, QFont::Bold);

            drawTextCenter(painter, 1, "DES", fmcControl().flightModeTracker().isDescent() ? GREEN : WHITE);

            setFont(painter, SMALL_FONT);

            //TODO drawTextLeft(painter, 2, 2, "ACT MODE");

            drawTextLeft(painter, 2, 8, "PRESEL SPD");
            drawTextLeft(painter, 2, 10, "PRESEL MACH");

            drawTextRight(painter, 2, 12, "NEXT");

            setFont(painter, NORM_FONT);

            //TODOdrawTextLeft(painter, 1, 3, "SELECTED", GREEN);

            (m_fmc_data.descentSpeedKts() == 0) ?
                drawTextLeft(painter, 1, 9, "[    ]", CYAN) : 
                drawTextLeft(painter, 1, 9, QString::number(m_fmc_data.descentSpeedKts()), CYAN);

            (m_fmc_data.descentMach() == 0.0) ?
                drawTextLeft(painter, 1, 11, "[    ]", CYAN) : 
                drawTextLeft(painter, 1, 11, QString::number(m_fmc_data.descentMach()), CYAN);

            drawPrevPhaseText(painter);
            drawTextRight(painter, 1, 13, "PHASE>");
            break;
        }

        case(PERF_PAGE_APPROACH): {
            setFont(painter, NORM_FONT, QFont::Bold);

            drawTextCenter(painter, 1, "APPR", fmcControl().flightModeTracker().isApproach() ? GREEN : WHITE);

            setFont(painter, SMALL_FONT);

            drawTextLeft(painter, 2, 8, "TRANS LVL");
            drawTextLeft(painter, 2, 10, "VAPP");

            drawTextLeft(painter, 9, 2, "FLP RETR");
            drawTextLeft(painter, 9, 4, "SLT RETR");
            drawTextLeft(painter, 11, 6, "CLEAN");

            drawTextCenter(painter, 10, "VLS");

            drawTextRight(painter, 2, 4, "MDA");
            drawTextRight(painter, 2, 6, "DH");
            drawTextRight(painter, 2, 8, "LDG CONF");
            
            drawTextRight(painter, 2, 12, "NEXT");

            setFont(painter, NORM_FONT);

            double vls = fmcControl().aircraftData().getMinimumSelectableSpeed(m_fmc_data.landingFlapsNotch());

            (m_fmc_data.transitionLevelAdes() == 0) ?
                drawTextLeft(painter, 1, 9, QString(SPACER)+SPACER+SPACER+SPACER, ORANGE) :
                drawTextLeft(painter, 1, 9, QString::number(m_fmc_data.transitionLevelAdes()), CYAN);

            //TODO
//             (m_fmc_data.approachSpeedKts() == 0) ?
//                 drawTextLeft(painter, 1, 11, QString(SPACER)+SPACER+SPACER, ORANGE) :
//                 drawTextLeft(painter, 1, 11, QString::number(m_fmc_data.approachSpeedKts()), CYAN);
            
            //TODO take wind from CDU page
            double headwind = 0.0;

            if (fmcControl().normalRoute().destinationAirport() != 0 &&
                fmcControl().normalRoute().destinationAirport()->hasActiveRunway())
            {
                double head_wind_angle = qMin(
                    90.0, 
                    Navcalc::getAbsHeadingDiff(
                        m_flightstatus->wind_dir_deg_true + m_flightstatus->magvar,
                        fmcControl().normalRoute().destinationAirport()->activeRunway().hdg()));

                headwind = m_flightstatus->wind_speed_kts * cos(Navcalc::toRad(head_wind_angle));
            }
                                                        
            int vapp = qMax(Navcalc::round(vls+5.0), 
                            qMin(Navcalc::round(vls+15.0), Navcalc::round(vls + headwind / 3.0)));
            drawTextLeft(painter, 1, 11, QString::number(vapp), GREEN);

            drawTextLeft(painter, 10, 3, "F=");
            drawTextLeft(painter, 12, 3, QString("%1").
                         arg(fmcControl().aircraftData().getMinimumAStyleFlapsRetractionSpeed(), 0, 'f', 0), GREEN);
            
            drawTextLeft(painter, 10, 5, "S=");
            drawTextLeft(painter, 12, 5, QString("%1").
                         arg(fmcControl().aircraftData().getMinimumAStyleSlatsRetractionSpeed(), 0, 'f', 0), GREEN);

            drawTextLeft(painter, 10, 7, "O=");
            drawTextLeft(painter, 12, 7, QString("%1").
                         arg((int)fmcControl().aircraftData().getGreenDotSpeed()), GREEN);

            drawTextCenter(painter, 11, QString("%1").arg(vls, 0, 'f', 0), GREEN);

            (m_fmc_data.minDescentAltitudeFt() == 0) ?
                drawTextRight(painter, 1, 5, "[    ]", CYAN) :
                drawTextRight(painter, 1, 5, QString::number(m_fmc_data.minDescentAltitudeFt()), CYAN);

            (m_fmc_data.decisionHeightFt() == 0) ?
                drawTextRight(painter, 1, 7, "[    ]", CYAN) :
                drawTextRight(painter, 1, 7, QString::number(m_fmc_data.decisionHeightFt()), CYAN);

            if (m_fmc_data.landingFlapsNotch() < m_flightstatus->flaps_lever_notch_count-1)
            {
                drawTextRight(painter, 1, 9, "CONF3*", CYAN);
                setFont(painter, SMALL_FONT);
                drawTextRight(painter, 1, 11, "FULL", CYAN);
            }
            else
            {
                drawTextRight(painter, 1, 11, "FULL*", CYAN);
                setFont(painter, SMALL_FONT);
                drawTextRight(painter, 1, 9, "CONF3", CYAN);
            }

            setFont(painter, NORM_FONT);

            drawPrevPhaseText(painter);
            drawTextRight(painter, 1, 13, "PHASE>");
            
            break;
        }

        case(PERF_PAGE_GOAROUND): {
            setFont(painter, NORM_FONT, QFont::Bold);
            drawTextCenter(painter, 1, "GO AROUND");

            drawPrevPhaseText(painter);
            break;
        }

    }
}

/////////////////////////////////////////////////////////////////////////////

bool FMCCDUPageStyleAPerformance::handleAppPhaseAction()
{
    if (m_flightstatus->onground) return false;

    if (!m_approach_phase_trigger) 
    {
        m_approach_phase_trigger = true;
    }
    else
    {
        m_fmc_data.setApproachPhaseActive(true);
        m_approach_phase_trigger = false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAPerformance::processAction(const QString& action)
{
    //TODO if (processBasicActions(action)) return;
	
    QString text = m_page_manager->scratchpad().text();
    bool convok_uint = false;
    uint text_value = text.toUInt(&convok_uint);

    int llsk_index = -1;
    int rlsk_index = -1;
    if (!getLSKIndex(action, llsk_index, rlsk_index)) return;

    bool changes_allowed = changesOnActivePerfPageAllowed();

    //-----

    switch(m_current_perf_page)
    {
        case(PERF_PAGE_TAKE_OFF): {

            switch(llsk_index)
            {
                case(1): {

                    if (text.isEmpty()) return;

                    if (!changes_allowed)
                    {
                        m_page_manager->scratchpad().setOverrideText("NOT ALLOWED");
                        return;
                    }

                    if (!convok_uint)
                    {
                        m_page_manager->scratchpad().setOverrideText("INVALID VALUE");
                        return;
                    }

                    if (text_value < 50 || text_value > 180) 
                    {
                        m_page_manager->scratchpad().setOverrideText("OUT OF RANGE");
                        return;
                    }

                    m_fmc_data.setV1(text_value);
                    m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
                    break;
                }
                case(2): {

                    if (text.isEmpty()) return;

                    if (!changes_allowed)
                    {
                        m_page_manager->scratchpad().setOverrideText("NOT ALLOWED");
                        return;
                    }

                    if (!convok_uint)
                    {
                        m_page_manager->scratchpad().setOverrideText("INVALID VALUE");
                        return;
                    }

                    if (text_value < 50 || text_value > 180) 
                    {
                        m_page_manager->scratchpad().setOverrideText("OUT OF RANGE");
                        return;
                    }

                    m_fmc_data.setVr(text_value);
                    m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
                    break;
                }
                case(3): {

                    if (text.isEmpty()) return;

                    if (!changes_allowed)
                    {
                        m_page_manager->scratchpad().setOverrideText("NOT ALLOWED");
                        return;
                    }

                    if (!convok_uint)
                    {
                        m_page_manager->scratchpad().setOverrideText("INVALID VALUE");
                        return;
                    }

                    if (text_value < 50 || text_value > 180) 
                    {
                        m_page_manager->scratchpad().setOverrideText("OUT OF RANGE");
                        return;
                    }

                    m_fmc_data.setV2(text_value);
                    fmcControl().fmcAutothrottle().setAPThrottleModeSpeed(false);
                    fmcControl().fsAccess().setAPAirspeed(text_value);
                    m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
                    break;
                }
                case(4): {

                    if (text.isEmpty()) return;

                    if (!convok_uint)
                    {
                        m_page_manager->scratchpad().setOverrideText("INVALID VALUE");
                        return;
                    }

                    if (text_value < 500 || text_value > 30000) 
                    {
                        m_page_manager->scratchpad().setOverrideText("OUT OF RANGE");
                        return;
                    }

                    m_fmc_data.setTransitionAltitudeAdep(text_value);
                    m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
                    break;
                }

                case(5): {

                    if (text.isEmpty()) return;

                    if (!changes_allowed)
                    {
                        m_page_manager->scratchpad().setOverrideText("NOT ALLOWED");
                        return;
                    }

                    uint thr_red_alt = 0;
                    uint acc_alt = 0;

                    if (text.contains("/"))
                    {
                        if (text.section("/", 0, 0).isEmpty())
                            thr_red_alt = m_fmc_data.normalRoute().thrustReductionAltitudeFt();
                        else
                            thr_red_alt = text.section("/", 0, 0).toUInt();

                        acc_alt = text.section("/", 1).toUInt();
                    }
                    else
                    {
                        if (convok_uint) 
                        {
                            thr_red_alt = text_value;
                            acc_alt = m_fmc_data.normalRoute().accelerationAltitudeFt();
                        }
                    }

                    if (thr_red_alt == 0 || acc_alt == 0)
                    {
                        m_page_manager->scratchpad().setOverrideText("INVALID VALUE");
                        return;
                    }

                    if (thr_red_alt < 500 || thr_red_alt > 30000 || acc_alt < 500 || acc_alt > 30000) 
                    {
                        m_page_manager->scratchpad().setOverrideText("OUT OF RANGE");
                        return;
                    }

                    m_fmc_data.normalRoute().setThrustReductionAltitudeFt(thr_red_alt);
                    m_fmc_data.normalRoute().setAccelerationAltitudeFt(acc_alt);
                    m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
                    break;
                }
            }

            switch(rlsk_index)
            {
                case(3): {

                    if (text.isEmpty()) return;

                    if (!changes_allowed)
                    {
                        m_page_manager->scratchpad().setOverrideText("NOT ALLOWED");
                        return;
                    }

                    int flaps_notch = -1;
                    double trim = 0.0;
                    bool convok = false;

                    if (text.contains("/"))
                    {
                        if (text.section("/", 0, 0).isEmpty())
                        {
                            flaps_notch = m_fmc_data.takeoffFlapsNotch();
                        }
                        else
                        {
                            flaps_notch = text.section("/", 0, 0).toUInt(&convok);
                            if (!convok) flaps_notch = -1;
                        }

                        trim = text.section("/", 1).toDouble();
                    }
                    else
                    {
                        if (convok_uint) 
                        {
                            flaps_notch = text_value;
                            trim = m_fmc_data.takeoffTrim();
                        }
                    }

                    if (flaps_notch < 0)
                    {
                        m_page_manager->scratchpad().setOverrideText("INVALID VALUE");
                        return;
                    }

                    if (flaps_notch < 0 || flaps_notch >= (int)m_flightstatus->flaps_lever_notch_count || trim < -99 || trim > 99) 
                    {
                        m_page_manager->scratchpad().setOverrideText("OUT OF RANGE");
                        return;
                    }

                    m_fmc_data.setTakeoffFlapsNotch(flaps_notch);
                    m_fmc_data.setTakeoffTrim(trim);
                    fmcControl().fsAccess().setElevatorTrimDegrees(trim);
                    m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
                    break;
                }

                case(4): {

                    if (text.isEmpty()) return;

                    if (!changes_allowed)
                    {
                        m_page_manager->scratchpad().setOverrideText("NOT ALLOWED");
                        return;
                    }

                    bool convok = false;
                    double n1 = text.toDouble(&convok);
                    if (!convok) n1 = text.toInt(&convok);
                    if (!convok || n1 < 50.0 || n1 > 100.0)
                    {
                        m_page_manager->scratchpad().setOverrideText("OUT OF RANGE");
                        return;
                    }

                    m_fmc_data.setFMCTakeoffThrustN1(n1);
                    m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
                    break;
                }

                case(6): {

                    m_current_perf_page = PERF_PAGE_CLIMB;
                    break;
                }
            }

            break;
        }

            //-----

        case(PERF_PAGE_CLIMB): {

            switch(llsk_index)
            {
                case(4): {

                    if (text.isEmpty()) return;

                    if (!convok_uint)
                    {
                        m_page_manager->scratchpad().setOverrideText("INVALID VALUE");
                        return;
                    }

                    if (text_value < 100 || text_value > 399) 
                    {
                        m_page_manager->scratchpad().setOverrideText("OUT OF RANGE");
                        return;
                    }

                    m_fmc_data.setClimbSpeedKts(text_value);
                    m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
                    break;
                }

                case(5): {

                    if (text.isEmpty()) return;

                    double mach = parseMach(text);
                    if (mach <= 0.0)
                    {
                        m_page_manager->scratchpad().setOverrideText("OUT OF RANGE");
                        return;
                    }

                    m_fmc_data.setClimbMach(mach);
                    m_fmc_data.setCruiseMach(mach);
                    m_fmc_data.setDescentMach(mach);
                    m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
                    break;
                }

                case(6): {

                    if (!handleAppPhaseAction()) m_current_perf_page = PERF_PAGE_TAKE_OFF;
                    break;
                }
            }

            switch(rlsk_index)
            {
                case(4): {

                    if (text.isEmpty()) return;

                    bool convok = false;
                    double n1 = text.toDouble(&convok);
                    if (!convok) n1 = text.toInt(&convok);
                    if (!convok || n1 < 50.0 || n1 > 100.0)
                    {
                        m_page_manager->scratchpad().setOverrideText("OUT OF RANGE");
                        return;
                    }

                    m_fmc_data.setFMCInitialClimbThrustN1(n1);
                    m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
                    break;
                }

                case(5): {

                    if (text.isEmpty()) return;

                    bool convok = false;
                    double n1 = text.toDouble(&convok);
                    if (!convok) n1 = text.toInt(&convok);
                    if (!convok || n1 < 50.0 || n1 > 100.0)
                    {
                        m_page_manager->scratchpad().setOverrideText("OUT OF RANGE");
                        return;
                    }

                    m_fmc_data.setFMCMaxClimbThrustN1(n1);
                    m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
                    break;
                }

                case(6): {

                    m_current_perf_page = PERF_PAGE_CRUISE;
                    break;
                }
            }

            break;
        }

            //-----

        case(PERF_PAGE_CRUISE): {

            switch(llsk_index)
            {
                case(4): {

                    if (text.isEmpty()) return;

                    double mach = parseMach(text);
                    if (mach <= 0.0)
                    {
                        m_page_manager->scratchpad().setOverrideText("OUT OF RANGE");
                        return;
                    }

                    m_fmc_data.setCruiseMach(mach);
                    m_fmc_data.setDescentMach(mach);
                    m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
                    break;
                }

                case(6): {

                    if (!handleAppPhaseAction()) m_current_perf_page = PERF_PAGE_CLIMB;
                    break;
                }
            }

            switch(rlsk_index)
            {
                case(6): {

                    m_current_perf_page = PERF_PAGE_DESCENT;
                    break;
                }
            }
            break;
        }

            //-----

        case(PERF_PAGE_DESCENT): {

            switch(llsk_index)
            {
                case(4): {

                    if (text.isEmpty()) return;

                    if (!convok_uint)
                    {
                        m_page_manager->scratchpad().setOverrideText("INVALID VALUE");
                        return;
                    }

                    if (text_value < 100 || text_value > 399) 
                    {
                        m_page_manager->scratchpad().setOverrideText("OUT OF RANGE");
                        return;
                    }

                    m_fmc_data.setDescentSpeedKts(text_value);
                    m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
                    break;
                }

                case(5): {

                    if (text.isEmpty()) return;

                    double mach = parseMach(text);
                    if (mach <= 0.0)
                    {
                        m_page_manager->scratchpad().setOverrideText("OUT OF RANGE");
                        return;
                    }

                    m_fmc_data.setDescentMach(mach);
                    m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
                    break;
                }

                case(6): {

                    if (!handleAppPhaseAction()) m_current_perf_page = PERF_PAGE_CRUISE;
                    break;
                }
            }

            switch(rlsk_index)
            {
                case(6): {

                    m_current_perf_page = PERF_PAGE_APPROACH;
                    break;
                }
            }
            
            break;
        }
            
            //-----

        case(PERF_PAGE_APPROACH): {
            
            switch(llsk_index)
            {
                case(4): {

                    if (text.isEmpty()) return;

                    if (!convok_uint)
                    {
                        m_page_manager->scratchpad().setOverrideText("INVALID VALUE");
                        return;
                    }

                    if (text_value < 10 || text_value > 300)
                    {
                        m_page_manager->scratchpad().setOverrideText("OUT OF RANGE");
                        return;
                    }

                    m_fmc_data.setTransitionLevelAdes(text_value);
                    m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
                    break;
                }
                case(5): {

                    if (text.isEmpty()) return;

                    if (!convok_uint)
                    {
                        m_page_manager->scratchpad().setOverrideText("INVALID VALUE");
                        return;
                    }
                    
                    if (text_value < 50 || text_value > 180) 
                    {
                        m_page_manager->scratchpad().setOverrideText("OUT OF RANGE");
                        return;
                    }

                    m_fmc_data.setApproachSpeedKts(text_value);
                    m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
                    break;
                }

                case(6): {

                    if (!handleAppPhaseAction()) m_current_perf_page = PERF_PAGE_DESCENT;
                    break;
                }
            }

            switch(rlsk_index)
            {
                case(2): {

                    if (text.isEmpty()) return;

                    if (!convok_uint)
                    {
                        m_page_manager->scratchpad().setOverrideText("INVALID VALUE");
                        return;
                    }

                    if (text_value < 100 || text_value > 10000) 
                    {
                        m_page_manager->scratchpad().setOverrideText("OUT OF RANGE");
                        return;
                    }

                    m_fmc_data.setMinDescentAltitudeFt(text_value);
                    m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
                    break;
                }
                case(3): {
                    
                    if (text.isEmpty()) return;

                    if (!convok_uint)
                    {
                        m_page_manager->scratchpad().setOverrideText("INVALID VALUE");
                        return;
                    }
                    
                    if (text_value < 10 || text_value > 2000) 
                    {
                        m_page_manager->scratchpad().setOverrideText("OUT OF RANGE");
                        return;
                    }

                    m_fmc_data.setDecisionHeightFt(text_value);
                    m_page_manager->scratchpad().processAction(FMCCDUPageBase::ACTION_CLRALL);
                    break;
                }

                case(4): {
                    m_fmc_data.setLandingFlapsNotch(m_flightstatus->flaps_lever_notch_count-2);
                    break;
                }

                case(5): {
                    m_fmc_data.setLandingFlapsNotch(m_flightstatus->flaps_lever_notch_count-1);
                    break;
                }

                case(6): {

                    m_current_perf_page = PERF_PAGE_GOAROUND;
                    break;
                }
            }

            break;
        }
            
            //-----

        case(PERF_PAGE_GOAROUND): {
            
            switch(llsk_index)
            {
                case(6): {
                    
                    if (!handleAppPhaseAction()) m_current_perf_page = PERF_PAGE_APPROACH;
                    break;
                }
            }
            
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

double FMCCDUPageStyleAPerformance::parseMach(const QString& text)
{
    bool convok = false;
    double mach = text.toDouble(&convok);
    if (!convok) mach = text.toInt(&convok);
    if (!convok || mach < 0.2 || mach > 0.99) return 0.0;
    return mach;
}

/////////////////////////////////////////////////////////////////////////////

uint FMCCDUPageStyleAPerformance::maxHorizontalScrollOffset() const 
{
    return 0;
}

/////////////////////////////////////////////////////////////////////////////

void FMCCDUPageStyleAPerformance::setActive(bool active)
{
    if (active) m_check_timer.start(250); 
    else m_check_timer.stop();

    FMCCDUPageBase::setActive(active);
    m_approach_phase_trigger = false;
    slotCheckAndSetPerfPage(true);
}

// End of file
