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

/*! \file    opengltext.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __OPENGLTEXT_H__
#define __OPENGLTEXT_H__

#include "vas_gl.h"

#ifdef USE_GLTT
#include <FTFace.h>
#include <GLTTFont.h>
#endif

#ifdef USE_OGLFT
#include <OGLFT.h>
#endif

#ifdef USE_FTGL
#include <FTGL.h>
#include <FTGLPolygonFont.h>
#endif

#ifdef USE_FONTRENDERER
#include "lfontrenderer.h"
#endif

#include <QString>

/////////////////////////////////////////////////////////////////////////////

class OpenGLText : public QObject
{
    Q_OBJECT

public:

    OpenGLText(const QString& fontname, const unsigned int& fontsize);

    virtual ~OpenGLText();

    void loadFont(const QString& filename, uint font_size);

    void setHeight(uint height);
    
    double getHeight() const;
    
    double getWidth(const QString& text) const;

    void drawText(const double& x,
                  const double& y,
                  const QString& text);

signals:

    //! emitted when the font size is set
    void signalResized();

protected:
    
#ifdef USE_GLTT
    FTFace *m_font_face;
    GLTTFont *m_font;
#endif
    
#ifdef USE_OGLFT
    OGLFT::Face *m_font;
    double m_height;
#endif

#ifdef USE_FTGL
    FTGLPolygonFont *m_font;
#endif

#ifdef USE_FONTRENDERER
    LFontRenderer m_font;
#endif

};

#endif /* __OPENGLTEXT_H__ */

// End of file

