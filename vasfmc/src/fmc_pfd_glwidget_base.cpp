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

/*! \file    fmc_pfd_glwidget_base.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "vas_widget.h"
#include "fmc_control.h"

#include "fmc_pfd_glwidget_base.h"

/////////////////////////////////////////////////////////////////////////////

GLPFDWidgetBase::GLPFDWidgetBase(ConfigWidgetProvider* config_widget_provider,
                                 Config* main_config,
                                 Config* pfd_config,
                                 FMCControl* fmc_control,
                                 VasWidget *parent,
                                 bool left_side) :
        VasGLWidget(QGLFormat(QGL::SampleBuffers), parent),
        m_config_widget_provider(config_widget_provider),
        m_main_config(main_config),
        m_pfd_config(pfd_config),
        m_fmc_control(fmc_control),
        m_fmc_data(fmc_control->fmcData()),
        m_flightstatus(fmc_control->flightStatus()),
        m_left_side(left_side)
{
    MYASSERT(m_config_widget_provider != 0);
    MYASSERT(m_main_config != 0);
    MYASSERT(m_pfd_config != 0);
    MYASSERT(m_fmc_control != 0);
    MYASSERT(m_flightstatus != 0);
}

/////////////////////////////////////////////////////////////////////////////

// End of file
