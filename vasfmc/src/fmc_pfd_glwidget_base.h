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

#ifndef FMCPFD_GLWIDGET_STYLE_BASE_H
#define FMCPFD_GLWIDGET_STYLE_BASE_H

#include <QDateTime>
#include <QSizeF>
#include <QPointF>

#include "opengltext.h"

#include "vas_gl_widget.h"

class Config;
class ConfigWidgetProvider;
class FlightStatus;
class ProjectionBase;
class OpenGLText;
class FMCControl;
class FMCData;
class VasWidget;

/////////////////////////////////////////////////////////////////////////////

class GLPFDWidgetBase : public VasGLWidget
{
    Q_OBJECT

public:

    GLPFDWidgetBase(ConfigWidgetProvider* config_widget_provider,
                    Config* main_config,
                    Config* pfd_config,
                    FMCControl* fmc_control,
                    VasWidget *parent,
                    bool left_side);
    
    virtual ~GLPFDWidgetBase() {}

 	virtual void refreshPFD() = 0;

    virtual inline void reset() { updateGL(); }

protected:

    virtual void initializeGL() = 0;
    virtual void paintGL() = 0;
    virtual void resizeGL(int width, int height) = 0;

protected:
    
    ConfigWidgetProvider* m_config_widget_provider;

    Config* m_main_config;
	Config* m_pfd_config;

    FMCControl* m_fmc_control;
    FMCData& m_fmc_data;
    FlightStatus* m_flightstatus;

    bool m_left_side;
};

#endif // FMCPFD_GLWIDGET_STYLE_BASE_H
