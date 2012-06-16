///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2008 Martin Boehme
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

#include "code_timer.h"

static void MyGetThreadTimes(LARGE_INTEGER *pKernel, LARGE_INTEGER *pUser)
{
    FILETIME timeCreation, timeExit, timeKernel, timeUser;

    GetThreadTimes(GetCurrentThread(), &timeCreation, &timeExit, &timeKernel,
        &timeUser);

    if(pKernel)
    {
        pKernel->u.LowPart=timeKernel.dwLowDateTime;
        pKernel->u.HighPart=timeKernel.dwHighDateTime;
    }

    if(pUser)
    {
        pUser->u.LowPart=timeUser.dwLowDateTime;
        pUser->u.HighPart=timeUser.dwHighDateTime;
    }
}

void CodeTimer::Tick()
{
    QueryPerformanceCounter(&m_perfCounterStart);
    MyGetThreadTimes(&m_kernelStart, &m_userStart);
}

QString CodeTimer::Tock()
{
    double        wallTime, kernelTime, userTime;
    LARGE_INTEGER perfCounterStop, perfFrequency;
    LARGE_INTEGER kernelStop, userStop;
    QString       str;

    QueryPerformanceCounter(&perfCounterStop);
    QueryPerformanceFrequency(&perfFrequency);

    wallTime=double(perfCounterStop.QuadPart-m_perfCounterStart.QuadPart) /
        double(perfFrequency.QuadPart);

    MyGetThreadTimes(&kernelStop, &userStop);
    kernelTime=double(kernelStop.QuadPart-m_kernelStart.QuadPart)/1e7;
    userTime=double(userStop.QuadPart-m_userStart.QuadPart)/1e7;

    //str.sprintf("wall %.2f kernel %.2f user %.2f ms", wallTime*1000,
    //    kernelTime*1000, userTime*1000);
    str.sprintf("wall %.3f ms", wallTime*1000);

    return str;
}
