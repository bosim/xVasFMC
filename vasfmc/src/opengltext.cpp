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

/*! \file    opengltext.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "logger.h"

#include "opengltext.h"

/////////////////////////////////////////////////////////////////////////////

OpenGLText::OpenGLText(const QString& fontname, const unsigned int& fontsize) 
{

#ifdef USE_GLTT
    m_font_face = new FTFace();
    MYASSERT(m_font_face);
    MYASSERT(m_font_face->open(fontname.toLatin1().data()));
    m_font = new GLTTFont(m_font_face);
    MYASSERT(m_font);
    MYASSERT(m_font->create(fontsize));
#endif

#ifdef USE_OGLFT
    m_font = new OGLFT::TranslucentTexture(fontname.toLatin1().data(), fontsize);
    MYASSERT(m_font);

    const OGLFT::BBox& box = m_font->measure('X');
    m_height = fabs(box.y_max_ - box.y_min_) * 1.5;
#endif

#ifdef USE_FTGL
    m_font = new FTGLPolygonFont(fontname.toLatin1().data());
    MYASSERT(m_font);
    m_font->FaceSize(fontsize);
#endif

#ifdef USE_FONTRENDERER
    MYASSERT(m_font.LoadFont("FMCFont", fontname.toLatin1().data()) == 0);
    m_font.SetHeight(fontsize);
    m_font.SetActiveFont("FMCFont");
#endif

}

/////////////////////////////////////////////////////////////////////////////

OpenGLText::~OpenGLText()
{
#ifdef USE_GLTT
    delete m_font;
    delete m_font_face;
#endif

#ifdef USE_OGLFT
    delete m_font;
#endif

#ifdef USE_FTGL
    delete m_font;
#endif

}

/////////////////////////////////////////////////////////////////////////////

void OpenGLText::loadFont(const QString& filename, uint font_size)
{
#ifdef USE_FONTRENDERER
    m_font.Clear();
    MYASSERT(m_font.LoadFont("FMCFont", filename.toLatin1().data()) == 0);
    m_font.SetHeight(font_size);
    m_font.SetActiveFont("FMCFont");
#endif
}

/////////////////////////////////////////////////////////////////////////////

void OpenGLText::setHeight(uint height)
{
#ifdef USE_FONTRENDERER
    m_font.SetHeight(height);
#endif

    emit signalResized();
}

/////////////////////////////////////////////////////////////////////////////

double OpenGLText::getHeight() const
{
#ifdef USE_GLTT
    return m_font->getHeight(); 
#endif

#ifdef USE_OGLFT
    return m_height;
#endif

#ifdef USE_FTGL
    return m_font->LineHeight();
#endif

#ifdef USE_FONTRENDERER
    return m_font.GetHeight();
#endif

    MYASSERT(0);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////

double OpenGLText::getWidth(const QString& text) const
{
#ifdef USE_GLTT
    return m_font->getWidth(text.toLatin1().data()); 
#endif

#ifdef USE_OGLFT
    return m_font->measure(text.toLatin1().data()).advance_.dx_;
#endif

#ifdef USE_FTGL
    float llx, lly, llz;
    float urx, ury, urz;
    m_font->BBox(text.toLatin1().data(), llx, lly, llz, urx, ury, urz);
    return urx - llx;
#endif

#ifdef USE_FONTRENDERER
    return m_font.GetStringWidth(text.toLatin1().data());
#endif

    MYASSERT(0);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////

void OpenGLText::drawText(const double& x,
                          const double& y,
                          const QString& text)
{
#ifdef USE_GLTT
    glPushMatrix();
    glTranslated(x, y, 0.0);
    glRotated(180.0, 1.0, 0.0, 0.0);
    m_font->output(text.toLatin1().data());
    glPopMatrix();
#endif

#ifdef USE_OGLFT
    glPushMatrix();
    glTranslated(x, y, 0.0);
    glRotated(180.0, 1.0, 0.0, 0.0);
    glEnable( GL_TEXTURE_2D );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    GLfloat colors[4];
    glGetFloatv(GL_CURRENT_COLOR, colors);
    m_font->setForegroundColor(colors[0], colors[1], colors[2], colors[3]);
    m_font->draw(0.0, 0.0, text.toLatin1().data());

    glDisable( GL_TEXTURE_2D );
    glPopMatrix();
#endif
    
#ifdef USE_FTGL
    glPushMatrix();
    glTranslated(x, y, 0.0);
    glRotated(180.0, 1.0, 0.0, 0.0);
    m_font->Render(text.toLatin1().data());
    glPopMatrix();
#endif

#ifdef USE_FONTRENDERER
    GLfloat colors[4];
    glGetFloatv(GL_CURRENT_COLOR, colors);
    m_font.SetColor(colors[0], colors[1], colors[2]);
    m_font.SetWidthScale(1);
    m_font.StringOut(x, y, text.toLatin1().data());
    m_font.Draw();
#endif
    
}

// End of file
