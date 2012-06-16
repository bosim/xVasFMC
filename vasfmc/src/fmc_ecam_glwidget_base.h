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

#ifndef FMCECAM_GLWIDGET_STYLE_BASE_H
#define FMCECAM_GLWIDGET_STYLE_BASE_H

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
class VasWidget;

/////////////////////////////////////////////////////////////////////////////

class GLECAMWidgetBase : public VasGLWidget
{
    Q_OBJECT

public:

    GLECAMWidgetBase(bool upper_ecam, 
                     ConfigWidgetProvider* config_widget_provider,
                     Config* main_config,
                     Config* ecam_config,
                     FMCControl* fmc_control,
                     VasWidget *parent = 0);

    virtual ~GLECAMWidgetBase() {}

 	virtual void refreshECAM() = 0;

    virtual inline void reset() { updateGL(); }

protected:

    virtual void initializeGL() = 0;
    virtual void paintGL() = 0;
    virtual void resizeGL(int width, int height) = 0;

protected:
    
    bool m_upper_ecam;

    ConfigWidgetProvider* m_config_widget_provider;

    Config* m_main_config;
	Config* m_ecam_config;

    FMCControl* m_fmc_control;
    FlightStatus* m_flightstatus;
};

#endif // FMCECAM_GLWIDGET_STYLE_BASE_H
