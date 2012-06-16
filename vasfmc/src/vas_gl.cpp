// vas_gl.cpp

#include "vas_gl.h"

#include "vas_gl_backend_agg.h"
#include "vas_gl_backend_qt.h"

#include <QBitmap>
#include <QPainter>
#include <QTransform>
#include <QVector>

#include <cmath>
#include <inttypes.h>

#include "logger.h"

namespace
{
    class ListCommand
    {
    public:
        virtual ~ListCommand()
        {
        }

        virtual void execute() = 0;
    };

    class ListCmd0 : public ListCommand
    {
    public:
        ListCmd0(void (*f)())
            : m_f(f)
        {
        }

        virtual void execute()
        {
            m_f();
        }

    private:
        void (*m_f)();
    };

    template <class T1>
    class ListCmd1 : public ListCommand
    {
    public:
        ListCmd1(void (*f)(T1), T1 arg1)
            : m_f(f), m_arg1(arg1)
        {
        }

        virtual void execute()
        {
            m_f(m_arg1);
        }

    private:
        void (*m_f)(T1);
        T1 m_arg1;
    };

    template <class T1, class T2>
    class ListCmd2 : public ListCommand
    {
    public:
        ListCmd2(void (*f)(T1, T2), T1 arg1, T2 arg2)
            : m_f(f), m_arg1(arg1), m_arg2(arg2)
        {
        }

        virtual void execute()
        {
            m_f(m_arg1, m_arg2);
        }

    private:
        void (*m_f)(T1, T2);
        T1 m_arg1;
        T2 m_arg2;
    };

    template <class T1, class T2, class T3>
    class ListCmd3 : public ListCommand
    {
    public:
        ListCmd3(void (*f)(T1, T2, T3), T1 arg1, T2 arg2, T3 arg3)
            : m_f(f), m_arg1(arg1), m_arg2(arg2), m_arg3(arg3)
        {
        }

        virtual void execute()
        {
            m_f(m_arg1, m_arg2, m_arg3);
        }

    private:
        void (*m_f)(T1, T2, T3);
        T1 m_arg1;
        T2 m_arg2;
        T3 m_arg3;
    };

    template <class T1, class T2, class T3, class T4>
    class ListCmd4 : public ListCommand
    {
    public:
        ListCmd4(void (*f)(T1, T2, T3, T4), T1 arg1, T2 arg2, T3 arg3,
            T4 arg4)
            : m_f(f), m_arg1(arg1), m_arg2(arg2), m_arg3(arg3), m_arg4(arg4)
        {
        }

        virtual void execute()
        {
            m_f(m_arg1, m_arg2, m_arg3, m_arg4);
        }

    private:
        void (*m_f)(T1, T2, T3, T4);
        T1 m_arg1;
        T2 m_arg2;
        T3 m_arg3;
        T4 m_arg4;
    };

    template <class T1, class T2, class T3, class T4, class T5>
    class ListCmd5 : public ListCommand
    {
    public:
        ListCmd5(void (*f)(T1, T2, T3, T4, T5), T1 arg1, T2 arg2, T3 arg3,
            T4 arg4, T5 arg5)
            : m_f(f), m_arg1(arg1), m_arg2(arg2), m_arg3(arg3), m_arg4(arg4),
              m_arg5(arg5)
        {
        }

        virtual void execute()
        {
            m_f(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5);
        }

    private:
        void (*m_f)(T1, T2, T3, T4, T5);
        T1 m_arg1;
        T2 m_arg2;
        T3 m_arg3;
        T4 m_arg4;
        T5 m_arg5;
    };

    template <class T1, class T2, class T3, class T4, class T5, class T6>
    class ListCmd6 : public ListCommand
    {
    public:
        ListCmd6(void (*f)(T1, T2, T3, T4, T5, T6), T1 arg1, T2 arg2, T3 arg3,
            T4 arg4, T5 arg5, T6 arg6)
            : m_f(f), m_arg1(arg1), m_arg2(arg2), m_arg3(arg3), m_arg4(arg4),
              m_arg5(arg5), m_arg6(arg6)
        {
        }

        virtual void execute()
        {
            m_f(m_arg1, m_arg2, m_arg3, m_arg4, m_arg5, m_arg6);
        }

    private:
        void (*m_f)(T1, T2, T3, T4, T5, T6);
        T1 m_arg1;
        T2 m_arg2;
        T3 m_arg3;
        T4 m_arg4;
        T5 m_arg5;
        T6 m_arg6;
    };

    typedef ListCmd1<GLenum> ListCmd1e;
    typedef ListCmd1<GLfloat> ListCmd1f;
    typedef ListCmd4<GLfloat, GLfloat, GLfloat, GLfloat> ListCmd4f;
    typedef ListCmd2<GLdouble, GLdouble> ListCmd2d;
    typedef ListCmd3<GLdouble, GLdouble, GLdouble> ListCmd3d;
    typedef ListCmd4<GLdouble, GLdouble, GLdouble, GLdouble> ListCmd4d;
    typedef ListCmd1<GLuint> ListCmd1u;

    class DisplayList
    {
    public:
        DisplayList()
        {
        }

        ~DisplayList()
        {
            for(int i=0; i<m_commands.size(); i++)
                delete m_commands[i];
        }

        void addCommand(ListCommand *pCommand)
        {
            m_commands.push_back(pCommand);
        }

        void execute()
        {
            for(int i=0; i<m_commands.size(); i++)
                m_commands[i]->execute();
        }

    private:
        // Disallow copy construction and assignment
        DisplayList(const DisplayList &);
        DisplayList &operator=(const DisplayList &);

        QVector<ListCommand *> m_commands;
    };

    struct RenderContext
    {
        RenderContext()
            : m_pList(0), m_matrixMode(GL_MODELVIEW), m_color(Qt::white),
              m_clearColor(Qt::black)
        {
            Logger::log("Creating RenderContext");

            m_modelview.push_back(QTransform());
        }

        ~RenderContext()
        {
            Logger::log("Destroying RenderContext");
        }

        void TransformChanged()
        {
            m_backend.setTransform(m_modelview.back());
        }

        VasGLBackendAGG        m_backend;

        // Lists
        DisplayList            *m_pList;
        GLuint                 m_listIndex;
        GLenum                 m_listMode;

        QVector<QPointF>       m_vertices;
        QVector<QColor>        m_vertexColors;
        QVector<QPointF>       m_texCoords;
        QVector<QTransform>    m_modelview;
        GLenum                 m_mode;
        GLenum                 m_matrixMode;
        QColor                 m_color, m_clearColor;

    private:
        RenderContext(const RenderContext &);
        RenderContext &operator=(const RenderContext &);
    };

    struct ListRange
    {
        GLuint  begin;
        GLsizei size;
    };

    static RenderContext *s_pCtx=0;

    QVector<DisplayList *> s_lists;
    QVector<bool>          s_listAllocated;
    QList<ListRange>       s_availableLists;
    QVector<GLuint>        s_availableTextures;


    struct StaticInitExit
    {
        ~StaticInitExit()
        {
            for(int i=0; i<s_lists.size(); i++)
                delete s_lists[i];
        }
    } s_staticInitExit;
}

VasGLRenderContext vasglCreateContext(int width, int height)
{
    RenderContext *pCtx=new RenderContext();

    pCtx->m_backend.init(width, height);

    return pCtx;
}

void vasglFreeContext(VasGLRenderContext ctx)
{
    RenderContext *pCtx=(RenderContext *)ctx;

    if(s_pCtx==pCtx)
        s_pCtx=0;

    pCtx->m_backend.end();

    delete pCtx;
}

void vasglMakeCurrent(VasGLRenderContext ctx, QImage *pimg)
{
    // Detach departing context from its rendering surface
    if(s_pCtx)
        s_pCtx->m_backend.detach();

    s_pCtx=(RenderContext *)ctx;

    // Attach new context to its rendering surface
    if(s_pCtx)
        s_pCtx->m_backend.attach(pimg);
}

void vasglBeginClipRegion(const QSize &size)
{
    if(!s_pCtx)
        return;

    glPushMatrix();
    glLoadIdentity();

    s_pCtx->m_backend.beginClipRegion();
}

void vasglEndClipRegion()
{
    if(!s_pCtx)
        return;

    glPopMatrix();

    s_pCtx->m_backend.endClipRegion();
}

void vasglDisableClipping()
{
    if(!s_pCtx)
        return;

    s_pCtx->m_backend.disableClipping();
}

void vasglCircle(double cx, double cy, double radius, double start_angle,
    double stop_angle, double angle_inc)
{
    if(!s_pCtx)
        return;

    if(s_pCtx->m_pList)
    {
        s_pCtx->m_pList->addCommand(
            new ListCmd6<double, double, double, double, double, double>
            (vasglCircle, cx, cy, radius, start_angle, stop_angle, angle_inc));
        return;
    }

    s_pCtx->m_backend.drawCircle(cx, cy, radius, start_angle, stop_angle,
        s_pCtx->m_color);
}

void vasglFilledCircle(double cx, double cy, double radius, double start_angle,
    double stop_angle, double angle_inc)
{
    if(!s_pCtx)
        return;

    if(s_pCtx->m_pList)
    {
        s_pCtx->m_pList->addCommand(
            new ListCmd6<double, double, double, double, double, double>
            (vasglFilledCircle, cx, cy, radius, start_angle, stop_angle,
            angle_inc));
        return;
    }

    s_pCtx->m_backend.drawFilledCircle(cx, cy, radius, start_angle,
        stop_angle, s_pCtx->m_color);
}

static int GLColorToInt(GLclampf col)
{
    int rval=int(round(col*255));

    if(rval<0)
        rval=0;
    if(rval>255)
        rval=255;

    return rval;
}

void glBegin(GLenum mode)
{
    if(!s_pCtx)
        return;

    if(s_pCtx->m_pList)
    {
        s_pCtx->m_pList->addCommand(new ListCmd1e(glBegin, mode));
        return;
    }

    s_pCtx->m_vertices.clear();
    s_pCtx->m_vertexColors.clear();
    s_pCtx->m_texCoords.clear();
    s_pCtx->m_mode=mode;
}

void glEnd()
{
    if(!s_pCtx)
        return;

    if(s_pCtx->m_pList)
    {
        s_pCtx->m_pList->addCommand(new ListCmd0(glEnd));
        return;
    }

    if(s_pCtx->m_vertices.size()==0)
        return;

    s_pCtx->m_backend.drawPrimitives(s_pCtx->m_mode, s_pCtx->m_vertices,
        s_pCtx->m_vertexColors, s_pCtx->m_texCoords);
}

void glVertex2d(GLdouble x, GLdouble y)
{
    if(!s_pCtx)
        return;

    if(s_pCtx->m_pList)
    {
        s_pCtx->m_pList->addCommand(new ListCmd2d(glVertex2d, x, y));
        return;
    }

    s_pCtx->m_vertices.push_back(QPointF(x, y));
    s_pCtx->m_vertexColors.push_back(s_pCtx->m_color);
}

void glVertex2i(GLint x, GLint y)
{
    glVertex2d(x, y);
}

void glVertex2f(GLfloat x, GLfloat y)
{
    glVertex2d(x, y);
}

void glTexCoord2f(GLfloat s, GLfloat t)
{
    if(!s_pCtx)
        return;

    if(s_pCtx->m_pList)
    {
        s_pCtx->m_pList->addCommand(new ListCmd2<GLfloat, GLfloat>
            (glTexCoord2f, s, t));
        return;
    }

    s_pCtx->m_texCoords.push_back(QPointF(s, t));
}

void glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
    if(!s_pCtx)
        return;

    if(s_pCtx->m_pList)
    {
        s_pCtx->m_pList->addCommand(new ListCmd4d(glRotated, angle, x, y, z));
        return;
    }

    if(s_pCtx->m_matrixMode==GL_MODELVIEW)
    {
        // TODO: Not general...
        if(x!=0)
            s_pCtx->m_modelview.back().rotate(angle, Qt::XAxis);
        else if(y!=0)
            s_pCtx->m_modelview.back().rotate(angle, Qt::YAxis);
        else
            s_pCtx->m_modelview.back().rotate(angle, Qt::ZAxis);

        s_pCtx->TransformChanged();
    }
}

void glTranslated(GLdouble x, GLdouble y, GLdouble z)
{
    if(!s_pCtx)
        return;

    if(s_pCtx->m_pList)
    {
        s_pCtx->m_pList->addCommand(new ListCmd3d(glTranslated, x, y, z));
        return;
    }

    if(s_pCtx->m_matrixMode==GL_MODELVIEW)
    {
        s_pCtx->m_modelview.back().translate(x, y);

        s_pCtx->TransformChanged();
    }
}

void glShadeModel(GLenum mode)
{
}

void glEnable(GLenum cap)
{
    if(!s_pCtx)
        return;

    if(s_pCtx->m_pList)
    {
        s_pCtx->m_pList->addCommand(new ListCmd1e(glEnable, cap));
        return;
    }

    if(cap == GL_LINE_STIPPLE)
        s_pCtx->m_backend.enableLineStipple(true);
}

void glDisable(GLenum cap)
{
    if(!s_pCtx)
        return;

    if(s_pCtx->m_pList)
    {
        s_pCtx->m_pList->addCommand(new ListCmd1e(glEnable, cap));
        return;
    }

    if(cap == GL_LINE_STIPPLE)
        s_pCtx->m_backend.enableLineStipple(false);
}

void glBlendFunc(GLenum sfactor, GLenum dfactor)
{
}

void glHint(GLenum target, GLenum mode)
{
}

// Matrices
void glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
}

void glMatrixMode(GLenum mode)
{
    if(!s_pCtx)
        return;

    if(s_pCtx->m_pList)
    {
        s_pCtx->m_pList->addCommand(new ListCmd1e(glMatrixMode, mode));
        return;
    }

    s_pCtx->m_matrixMode=mode;
}

void glLoadIdentity()
{
    if(!s_pCtx)
        return;

    if(s_pCtx->m_pList)
    {
        s_pCtx->m_pList->addCommand(new ListCmd0(glLoadIdentity));
        return;
    }

    if(s_pCtx->m_matrixMode==GL_MODELVIEW)
    {
        s_pCtx->m_modelview.back()=QTransform();

        s_pCtx->TransformChanged();
    }
}

void glOrtho(GLdouble left, GLdouble right, GLdouble bottom,
    GLdouble top, GLdouble near_val, GLdouble far_val)
{
}

void glPushMatrix()
{
    if(!s_pCtx)
        return;

    if(s_pCtx->m_pList)
    {
        s_pCtx->m_pList->addCommand(new ListCmd0(glPushMatrix));
        return;
    }

    if(s_pCtx->m_matrixMode==GL_MODELVIEW)
    {
        s_pCtx->m_modelview.push_back(s_pCtx->m_modelview.back());

        s_pCtx->TransformChanged();
    }
}

void glPopMatrix()
{
    if(!s_pCtx)
        return;

    if(s_pCtx->m_pList)
    {
        s_pCtx->m_pList->addCommand(new ListCmd0(glPopMatrix));
        return;
    }

    if(s_pCtx->m_matrixMode==GL_MODELVIEW)
        if(s_pCtx->m_modelview.size()>1)
        {
            s_pCtx->m_modelview.pop_back();

            s_pCtx->TransformChanged();
        }
}

void glClear(GLbitfield mask)
{
    if(!s_pCtx)
        return;

    if(s_pCtx->m_pList)
    {
        s_pCtx->m_pList->addCommand(new ListCmd1<GLbitfield>(glClear, mask));
        return;
    }

    s_pCtx->m_backend.clear(s_pCtx->m_clearColor);
}

void glFlush()
{
    if(!s_pCtx)
        return;
}

GLenum glGetError()
{
    return GL_NO_ERROR;
}

void glClearColor(GLclampf red, GLclampf green, GLclampf blue,
    GLclampf alpha)
{
    if(!s_pCtx)
        return;

    if(s_pCtx->m_pList)
    {
        s_pCtx->m_pList->addCommand(
            new ListCmd4<GLclampf, GLclampf, GLclampf, GLclampf>
            (glClearColor, red, green, blue, alpha));
        return;
    }

    s_pCtx->m_clearColor=QColor(GLColorToInt(red), GLColorToInt(green),
        GLColorToInt(blue), GLColorToInt(alpha));
}

void glGetFloatv(GLenum pname, GLfloat *params)
{
    if(!s_pCtx)
        return;

    if(pname==GL_CURRENT_COLOR)
    {
        params[0]=double(s_pCtx->m_color.red())/255.0;
        params[1]=double(s_pCtx->m_color.green())/255.0;
        params[2]=double(s_pCtx->m_color.blue())/255.0;
        params[3]=double(s_pCtx->m_color.alpha())/255.0;
    }
}

// Lists
void glDeleteLists(GLuint list,  GLsizei range)
{
    ListRange listRange;
    int       i;

    if(!s_pCtx)
        return;

    if(list==0)
        return;

    MYASSERT(list>=1 && int(list+range)<=s_lists.size());

    // All of the lists that are being freed should be currently allocated,
    // and we're going to mark them as not allocated
    for(i=0; i<range; i++)
    {
        // To protect ourselves against double frees in a release build,
        // don't free the list if one of the list elements isn't allocated
        if(!s_listAllocated[list+i])
        {
            *((int *)0)=0;
            return;
        }

        MYASSERT(s_listAllocated[list+i]);

        s_listAllocated[list+i]=false;
    }

    listRange.begin=list;
    listRange.size=range;

    s_availableLists.push_back(listRange);
}

GLuint glGenLists(GLsizei range)
{
    int i, j, rval;

    // First of all, check if a list range of sufficient size is available
    for(i=0; i<s_availableLists.size(); i++)
        if(s_availableLists[i].size >= range)
        {
            // Remember beginning of list range
            rval=s_availableLists[i].begin;

            // If the size of this list range is exactly equal to the
            // requested size, delete it from the available list. Otherwise, 
            // reduce its size accordingly
            if(s_availableLists[i].size == range)
            {
                s_availableLists.erase(
                    s_availableLists.begin()+i);
            }
            else
            {
                s_availableLists[i].begin+=range;
                s_availableLists[i].size=-range;
            }

            // Remeber that these lists have been allocated
            for(j=0; j<range; j++)
            {
                MYASSERT(!s_listAllocated[rval+j]);
                s_listAllocated[rval+j]=true;
            }

            return rval;
        }

    // Since a list index of 0 is not permissible, create a dummy list with a
    // list index of 0 if necessary
    if(s_lists.size()==0)
    {
        s_lists.push_back(NULL);
        s_listAllocated.push_back(false);
    }

    rval=s_lists.size();
    
    for(int i=0; i<range; i++)
    {
        s_lists.push_back(NULL);
        s_listAllocated.push_back(true);
    }

    return rval;
}

void glNewList(GLuint list, GLenum mode)
{
    if(!s_pCtx)
        return;

    // If we're already compiling a list, return
    if(s_pCtx->m_pList)
        return;

    // Spec says we should replace any potential existing list only when
    // glEndList is called -- so for the moment, we allocate the new list but
    // do not place it in s_lists yet.

    s_pCtx->m_listIndex=list;
    s_pCtx->m_listMode=mode;
    s_pCtx->m_pList=new DisplayList();
}

void glEndList()
{
    if(!s_pCtx)
        return;

    // Delete old list if one exists and replace it with the new list
    delete s_lists[s_pCtx->m_listIndex];
    s_lists[s_pCtx->m_listIndex]=s_pCtx->m_pList;

    // Remember that we're no longer compiling a list
    s_pCtx->m_pList=0;
}

void glCallList(GLuint list)
{
    if(!s_pCtx)
        return;

    if(s_pCtx->m_pList)
        s_pCtx->m_pList->addCommand(new ListCmd1u(glCallList, list));
    else
        s_lists[list]->execute();
}

// Depth buffer
void glDepthFunc(GLenum func)
{
}

void glDepthMask(GLboolean flag)
{
}

void glColor4f(GLfloat red, GLfloat green, GLfloat blue,
    GLfloat alpha)
{
    if(!s_pCtx)
        return;

    if(s_pCtx->m_pList)
    {
        s_pCtx->m_pList->addCommand(new ListCmd4f(glColor4f, red, green, blue,
            alpha));
        return;
    }

    s_pCtx->m_color=QColor(GLColorToInt(red), GLColorToInt(green),
        GLColorToInt(blue), GLColorToInt(alpha));
}

void glLineStipple(GLint factor, GLushort pattern)
{
    if(!s_pCtx)
        return;

    if(s_pCtx->m_pList)
    {
        s_pCtx->m_pList->addCommand(
            new ListCmd2<GLint, GLushort>(glLineStipple, factor, pattern));
        return;
    }

    s_pCtx->m_backend.setLineStipple(factor, pattern);
}

void glLineWidth(GLfloat width)
{
    if(!s_pCtx)
        return;

    if(s_pCtx->m_pList)
    {
        s_pCtx->m_pList->addCommand(new ListCmd1f(glLineWidth, width));
        return;
    }

    s_pCtx->m_backend.setLineWidth(width);
}

void glPolygonMode(GLenum face, GLenum mode)
{
}

void glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
    glBegin(GL_POLYGON);
        glVertex2d(x1, y1);
        glVertex2d(x2, y1);
        glVertex2d(x2, y2);
        glVertex2d(x1, y2);
    glEnd();
}

void glGenTextures(GLsizei n, GLuint *textures)
{
    if(!s_pCtx)
        return;

    // First, try to fill request from the available deleted textures
    while(n>0 && s_availableTextures.size()>0)
    {
        *textures=s_availableTextures.back();
        s_availableTextures.pop_back();
        textures++;
        n--;
    }

    // If we still need additional textures, get them from the backend
    if(n>0)
        s_pCtx->m_backend.reserveTexIndexes(n, textures);
}

void glDeleteTextures(GLsizei n, GLuint *textures)
{
    int i;

    if(!s_pCtx)
        return;

    for(i=0; i<n; i++)
        s_availableTextures.push_back(textures[i]);
}

void glBindTexture(GLenum target, GLuint texture)
{
    if(!s_pCtx)
        return;

    if(s_pCtx->m_pList)
    {
        s_pCtx->m_pList->addCommand(new ListCmd2<GLenum, GLuint>
            (glBindTexture, target, texture));
        return;
    }

    if(target==GL_TEXTURE_2D)
        s_pCtx->m_backend.selectTexture(texture);
}

void glTexImage2D(GLenum target, GLint level, GLint internalFormat,
    GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type,
    const GLvoid *pixels)
{
    int x, y;
    const uint8_t *buf=(const uint8_t *)pixels;
    uint8_t lum, alpha;

    if(!s_pCtx)
        return;

    if(target==GL_TEXTURE_2D && level==0 &&
       GLenum(internalFormat)==GL_LUMINANCE_ALPHA &&
       format==GL_LUMINANCE_ALPHA &&
       type==GL_UNSIGNED_BYTE)
    {
        QImage img(width, height, QImage::Format_ARGB32);
        // Format_ARGB32_Premultiplied?
        
        for(y=0; y<height; y++)
            for(x=0; x<width; x++)
            {
                lum=buf[2*(y*width+x)];
                alpha=buf[2*(y*width+x)+1];
                img.setPixel(x, y, qRgba(lum, lum, lum, alpha));
            }

        s_pCtx->m_backend.setTexture(img);
    }
}

void glTexParameteri(GLenum target, GLenum pname, GLint param)
{
}

void glTexEnvi(GLenum target, GLenum pname, GLint param)
{
}
