///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Martin Böhme
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

/*! \file    vas_gl.h
    \author  Martin Böhme
*/

#ifndef VAS_GL_H
#define VAS_GL_H

#include <QSize>

extern void vasglBeginClipRegion(const QSize &size);

extern void vasglEndClipRegion();

extern void vasglDisableClipping();

extern void vasglCircle(double cx, double cy, double radius,
    double start_angle, double stop_angle, double angle_inc);

extern void vasglFilledCircle(double cx, double cy, double radius,
    double start_angle, double stop_angle, double angle_inc);

#if VAS_GL_EMUL
#include <QPixmap>

namespace QGL
{
    enum FormatOption
    {
        SampleBuffers
    };
}

typedef void *VasGLRenderContext;

extern VasGLRenderContext vasglCreateContext(int width, int height);

extern void vasglFreeContext(VasGLRenderContext ctx);

extern void vasglMakeCurrent(VasGLRenderContext ctx, QImage *pimg);

// Types
typedef unsigned int GLuint;
typedef unsigned short GLushort;
typedef int GLint;
typedef int GLsizei;
typedef unsigned int GLenum;
typedef double GLdouble;
typedef unsigned int GLbitfield;
typedef unsigned char GLboolean;
typedef float GLfloat;
typedef float GLclampf;
typedef void GLvoid;

// Primitives
const GLenum GL_LINES=1;
const GLenum GL_LINE_LOOP=2;
const GLenum GL_LINE_STRIP=3;
const GLenum GL_TRIANGLES=4;
const GLenum GL_TRIANGLE_STRIP=5;
const GLenum GL_TRIANGLE_FAN=6;
const GLenum GL_QUADS=7;
const GLenum GL_POLYGON=9;

// Lighting
const GLenum GL_FLAT=0x1d00;
const GLenum GL_SMOOTH=0x1d01;

// Depth buffer
const GLenum GL_LESS=0x0201;
const GLenum GL_ALWAYS=0x0207;
const GLenum GL_DEPTH_TEST=0x0b71;

// Blending
const GLenum GL_BLEND=0x0be2;
const GLenum GL_SRC_ALPHA=0x0302;
const GLenum GL_ONE_MINUS_SRC_ALPHA=0x0303;

// Buffers, pixel formats
const GLenum GL_LUMINANCE_ALPHA=0x190a;

// Gets
const GLenum GL_CURRENT_COLOR=0xb00;

// Fog
const GLenum GL_LINEAR=0x2601;

// Hints
const GLenum GL_LINE_SMOOTH_HINT=0x0c52;
const GLenum GL_POINT_SMOOTH_HINT=0x0c51;
const GLenum GL_POLYGON_SMOOTH_HINT=0x0c53;
const GLenum GL_DONT_CARE=0x1100;
const GLenum GL_NICEST=0x1102;

// Alpha
const GLenum GL_ALPHA_TEST=0x0bc0;

// Points
const GLenum GL_POINT_SMOOTH=0x0b10;

// Lines
const GLenum GL_LINE_SMOOTH=0x0b20;
const GLenum GL_LINE_STIPPLE=0x0b24;

// Polygons
const GLenum GL_FILL=0x1b02;
const GLenum GL_FRONT=0x0404;
const GLenum GL_POLYGON_SMOOTH=0x0b41;

// Data types
const GLenum GL_UNSIGNED_BYTE=0x1401;

// Matrix Mode
const GLenum GL_MODELVIEW=0x1700;
const GLenum GL_PROJECTION=0x1701;
const GLenum GL_TEXTURE=0x1702;

// glPush/PopAttrib
const GLenum GL_DEPTH_BUFFER_BIT=0x00000100;
const GLenum GL_COLOR_BUFFER_BIT=0x00004000;

// Lists
const GLenum GL_COMPILE=0x1300;

// Errors
const GLenum GL_NO_ERROR=0;

// Textures
const GLenum GL_TEXTURE_ENV_MODE=0x2200;
const GLenum GL_TEXTURE_ENV=0x2300;
const GLenum GL_TEXTURE_2D=0x0de1;
const GLenum GL_TEXTURE_MAG_FILTER=0x2800;
const GLenum GL_TEXTURE_MIN_FILTER=0x2801;
const GLenum GL_TEXTURE_WRAP_S=0x2802;
const GLenum GL_TEXTURE_WRAP_T=0x2803;
const GLenum GL_TEXTURE_GEN_S=0x0c60;
const GLenum GL_TEXTURE_GEN_T=0x0c61;
const GLenum GL_MODULATE=0x2100;
const GLenum GL_CLAMP=0x2900;


// Drawing
extern void glBegin(GLenum mode);
extern void glEnd();
extern void glVertex2d(GLdouble x, GLdouble y);
extern void glVertex2i(GLint x, GLint y);
extern void glVertex2f(GLfloat x, GLfloat y);
extern void glTexCoord2f(GLfloat s, GLfloat t);
extern void glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
extern void glTranslated(GLdouble x, GLdouble y, GLdouble z);

extern void glShadeModel(GLenum mode);
extern void glEnable(GLenum cap);
extern void glDisable(GLenum cap);
extern void glBlendFunc(GLenum sfactor, GLenum dfactor);
extern void glHint(GLenum target, GLenum mode);

// Matrices
// http://jerome.jouvie.free.fr/OpenGl/Lessons/Lesson2.php
extern void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
extern void glMatrixMode(GLenum mode);
extern void glLoadIdentity();
extern void glOrtho(GLdouble left, GLdouble right, GLdouble bottom,
    GLdouble top, GLdouble near_val, GLdouble far_val);
extern void glPushMatrix();
extern void glPopMatrix();

extern void glClear(GLbitfield mask);
extern void glFlush();
extern GLenum glGetError();
extern void glClearColor(GLclampf red, GLclampf green, GLclampf blue,
    GLclampf alpha);
extern void glGetFloatv(GLenum pname, GLfloat *params);

// Lists
extern void glDeleteLists(GLuint list,  GLsizei range);
extern GLuint glGenLists(GLsizei range);
extern void glNewList(GLuint list, GLenum mode);
extern void glEndList();
extern void glCallList(GLuint list);

// Depth buffer
extern void glDepthFunc(GLenum func);
extern void glDepthMask(GLboolean flag);

extern void glColor4f(GLfloat red, GLfloat green, GLfloat blue,
    GLfloat alpha);
extern void glLineStipple(GLint factor, GLushort pattern);
extern void glLineWidth(GLfloat width);
extern void glPolygonMode(GLenum face, GLenum mode);
extern void glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);

// Textures
extern void glGenTextures(GLsizei n, GLuint *textures);
extern void glDeleteTextures(GLsizei n, GLuint *textures);
extern void glBindTexture(GLenum target, GLuint texture);
extern void glTexImage2D(GLenum target, GLint level, GLint internalFormat,
    GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type,
    const GLvoid *pixels);
extern void glTexParameteri(GLenum target, GLenum pname, GLint param);
extern void glTexEnvi(GLenum target, GLenum pname, GLint param);
#else
#include <QtOpenGL>
#endif

#endif // VAS_GL_H
