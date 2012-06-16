/***************************************************************************
 *   Copyright (C) 2008 by Philipp Münzel                                  *
 *   philipp_muenzel@web.de                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef XPAPIFACE_MSG_H
#define XPAPIFACE_MSG_H

#define XPAPIFACE_MSG_READY             0x08000001
#define XPAPIFACE_MSG_NOT_READY         0x08000002
#define XPAPIFACE_MSG_UNABLE_THAT_MODE  0x08000003
#define XPAPIFACE_COMM_AP_AVAIL_TRUE    0x01000001
#define XPAPIFACE_COMM_AP_AVAIL_FALSE   0x01000002
#define XPAPIFACE_COMM_AP_ENABLED_TRUE  0x01000003
#define XPAPIFACE_COMM_AP_ENABLED_FALSE 0x01000004
#define XPAPIFACE_COMM_HDG_HOLD_ON      0x01000005
#define XPAPIFACE_COMM_HDG_HOLD_OFF     0x01000006
#define XPAPIFACE_COMM_ALT_HOLD_ON      0x01000007
#define XPAPIFACE_COMM_ALT_HOLD_OFF     0x01000008
#define XPAPIFACE_COMM_SPEED_HOLD_ON    0x01000009
#define XPAPIFACE_COMM_SPEED_HOLD_OFF   0x0100000A
#define XPAPIFACE_COMM_MACH_HOLD_ON     0x0100000B
#define XPAPIFACE_COMM_MACH_HOLD_OFF    0x0100000C
#define XPAPIFACE_COMM_VS_HOLD_ON       0x0100000D
#define XPAPIFACE_COMM_VS_HOLD_OFF      0x0100000E
#define XPAPIFACE_COMM_NAV1_HOLD_ON     0x0100000F
#define XPAPIFACE_COMM_NAV1_HOLD_OFF    0x01000010
#define XPAPIFACE_COMM_GS_HOLD_ON       0x01000011
#define XPAPIFACE_COMM_GS_HOLD_OFF      0x01000012
#define XPAPIFACE_COMM_APP_HOLD_ON      0x01000013
#define XPAPIFACE_COMM_APP_HOLD_OFF     0x01000014
#define XPAPIFACE_COMM_APP_BC_HOLD_ON   0x01000015
#define XPAPIFACE_COMM_APP_BC_HOLD_OFF  0x01000016
#define XPAPIFACE_COMM_AT_TOGA_ON       0x01000017
#define XPAPIFACE_COMM_AT_TOGA_OFF      0x01000018
#define XPAPIFACE_COMM_AT_ARM_ON        0x01000019
#define XPAPIFACE_COMM_AT_ARM_OFF       0x0100001A
#define XPAPIFACE_COMM_NAVMODE_GPS      0x0100001B
#define XPAPIFACE_COMM_NAVMODE_NAV      0x0100001C


#endif
