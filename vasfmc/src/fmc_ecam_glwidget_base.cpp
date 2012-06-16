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

/*! \file    fmc_ecam_glwidget_base.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "fmc_control.h"
#include "vas_widget.h"

#include "fmc_ecam_glwidget_base.h"

/////////////////////////////////////////////////////////////////////////////

GLECAMWidgetBase::GLECAMWidgetBase(bool upper_ecam, 
                                   ConfigWidgetProvider* config_widget_provider,
                                   Config* main_config,
                                   Config* ecam_config,
                                   FMCControl* fmc_control,
                                   VasWidget *parent) :
    VasGLWidget(QGLFormat(QGL::SampleBuffers), parent),
    m_upper_ecam(upper_ecam),
    m_config_widget_provider(config_widget_provider),
    m_main_config(main_config),
    m_ecam_config(ecam_config),
    m_fmc_control(fmc_control),
    m_flightstatus(fmc_control->flightStatus())
{
    MYASSERT(m_config_widget_provider != 0);
    MYASSERT(m_main_config != 0);
    MYASSERT(m_ecam_config != 0);
    MYASSERT(m_fmc_control != 0);
    MYASSERT(m_flightstatus != 0);
}

/////////////////////////////////////////////////////////////////////////////

// End of file
