// vas_gl_backend_qt.cpp

#include "vas_gl_backend_qt.h"

#include <cmath>

static void flipHorizontal(QRectF *pRect)
{
    double x1, y1, x2, y2;

    pRect->getCoords(&x1, &y1, &x2, &y2);
    pRect->setCoords(x2, y1, x1, y2);
}

static void flipVertical(QRectF *pRect)
{
    double x1, y1, x2, y2;

    pRect->getCoords(&x1, &y1, &x2, &y2);
    pRect->setCoords(x1, y2, x2, y1);
}

VasGLBackendQt::VasGLBackendQt()
{
}

VasGLBackendQt::~VasGLBackendQt()
{
    // TODO: Order in which QPixmap and QPainter appear in the class seems to
    // play a big role... want to make this more robust

    for(int i=0; i<m_textures.size(); i++)
        delete m_textures[i];
}

void VasGLBackendQt::init(int width, int height)
{
    m_lineWidth=1.0;
    m_textureIdx=0;
    m_definingClip=false;

    m_pixmap=QPixmap(width, height);
    m_mask=QBitmap(width, height);

    m_painter.begin(&m_pixmap);
    m_painter.setRenderHint(QPainter::Antialiasing);
}

void VasGLBackendQt::end()
{
    m_painter.end();
}

QImage VasGLBackendQt::toImage()
{
    QImage img;

    m_painter.end();

    // img=m_pixmap.toImage();

    m_painter.begin(&m_pixmap);
    m_painter.setRenderHint(QPainter::Antialiasing);

    img=m_pixmap.toImage();

    return img;
}

QPixmap VasGLBackendQt::toPixmap()
{
    m_painter.end();
    m_painter.begin(&m_pixmap);
    m_painter.setRenderHint(QPainter::Antialiasing);

    return m_pixmap;
}

void VasGLBackendQt::clear(QColor color)
{
    m_pixmap.fill(color);
}

void VasGLBackendQt::setTransform(const QTransform &transform)
{
    m_painter.setWorldTransform(transform);
    m_painter.setWorldMatrixEnabled(true);
}

void VasGLBackendQt::setLineWidth(double pixels)
{
    m_lineWidth=pixels;
}

void VasGLBackendQt::drawPrimitives(GLenum mode,
    const QVector<QPointF> &vertices, const QVector<QColor> &colors,
    const QVector<QPointF> &texCoords)
{
    QPointF points[3];
    int     i;

    switch(mode)
    {
        case GL_LINES:
            setPen(colors[0]);
            m_painter.drawLines(vertices);
            break;
        case GL_LINE_LOOP:
        case GL_LINE_STRIP:
            setPen(colors[0]);
            m_painter.drawPolyline(vertices.data(), vertices.size());

            if(mode==GL_LINE_LOOP)
                if(!vertices.empty())
                    m_painter.drawLine(vertices.back(), vertices.front());
            break;

        case GL_TRIANGLES:
            m_painter.setPen(Qt::NoPen);
            for(i=0; i<vertices.size()-2; i+=3)
            {
                setBrush(colors[i]);
                m_painter.drawConvexPolygon(vertices.data()+i, 3);
            }
            break;

        case GL_TRIANGLE_STRIP:
            // Test whether we have the special case for the fontrenderer
            if(texCoords.size()==4 &&
               vertices.size()==4 &&
               m_textureIdx!=0)
            {
                QRectF source, target;
                int width=m_textures[m_textureIdx]->width();
                int height=m_textures[m_textureIdx]->height();
                target.setCoords(vertices[0].x(), vertices[0].y(),
                    vertices[3].x(), vertices[3].y());
                source.setCoords(
                    texCoords[0].x()*width, texCoords[0].y()*height,
                    texCoords[3].x()*width, texCoords[3].y()*height);

                // Adjust target rectangle with negative width or height by
                // flipping source rectangle
                if(target.width()<0)
                {
                    flipHorizontal(&target);
                    flipHorizontal(&source);
                }
                if(target.height()<0)
                {
                    flipVertical(&target);
                    flipVertical(&source);
                }

                QPixmap pixmapTemp;
                QTransform transform;
                if(source.width()<0)
                {
                    flipHorizontal(&source);
                    transform=transform.scale(-1, 1);
                }
                if(source.height()<0)
                {
                    flipVertical(&source);
                    transform=transform.scale(1, -1);
                }

                pixmapTemp=m_textures[m_textureIdx]->copy(source.toRect());
                pixmapTemp=pixmapTemp.transformed(transform);
                m_painter.drawPixmap(target.toRect(), pixmapTemp);
                break;
            }

            m_painter.setPen(Qt::NoPen);
            for(i=0; i<vertices.size()-2; i++)
            {
                setBrush(colors[i]);
                if(i%2==0)
                    m_painter.drawConvexPolygon(vertices.data()+i, 3);
                else
                {
                    points[0]=vertices[i+1];
                    points[1]=vertices[i];
                    points[2]=vertices[i+2];
                    m_painter.drawConvexPolygon(points, 3);
                }
            }
            break;

        case GL_TRIANGLE_FAN:
            m_painter.setPen(Qt::NoPen);
            setBrush(colors[0]);
            points[0]=vertices[0];
            for(i=1; i<vertices.size()-1; i++)
            {
                points[1]=vertices[i];
                points[2]=vertices[i+1];
                m_painter.drawConvexPolygon(points, 3);
            }
            break;

        case GL_QUADS:
            m_painter.setPen(Qt::NoPen);
            for(i=0; i<vertices.size()-3; i+=4)
            {
                setBrush(colors[i]);
                m_painter.drawPolygon(vertices.data()+i, 4);
            }
            break;

        case GL_POLYGON:
            m_painter.setPen(Qt::NoPen);
            setBrush(colors[0]);
            m_painter.drawPolygon(vertices.data(), vertices.size());
            break;
    }
}

void VasGLBackendQt::drawCircle(double cx, double cy, double radius,
    double start_angle, double stop_angle, const QColor &color)
{
    double span_angle;

    // Angles run clockwise in vasFMC, counter-clockwise in Qt
    // Starting point is 12 o'clock in vasFMC, 3 o'clock in Qt
    start_angle=M_PI/2-start_angle;
    stop_angle=M_PI/2-stop_angle;

    span_angle=stop_angle-start_angle;

    if(span_angle>0)
    {
        while(start_angle<0)
            start_angle+=2*M_PI;
    }
    else
    {
        while(start_angle>0)
            start_angle-=2*M_PI;
    }

    setPen(color);
    m_painter.drawArc(QRectF(cx-radius, cy-radius, 2*radius, 2*radius),
        (int)round(start_angle*16*180/M_PI),
        (int)round(span_angle*16*180/M_PI));
}

void VasGLBackendQt::drawFilledCircle(double cx, double cy, double radius,
    double start_angle, double stop_angle, const QColor &color)
{
    double span_angle;

    // Angles run clockwise in vasFMC, counter-clockwise in Qt
    // Starting point is 12 o'clock in vasFMC, 3 o'clock in Qt
    start_angle=M_PI/2-start_angle;
    stop_angle=M_PI/2-stop_angle;

    span_angle=stop_angle-start_angle;

    if(span_angle>0)
    {
        while(start_angle<0)
            start_angle+=2*M_PI;
    }
    else
    {
        while(start_angle>0)
            start_angle-=2*M_PI;
    }

    m_painter.setPen(Qt::NoPen);
    setBrush(color);
    m_painter.drawPie(QRectF(cx-radius, cy-radius, 2*radius, 2*radius),
        (int)round(start_angle*16*180/M_PI),
        (int)round(span_angle*16*180/M_PI));
}

void VasGLBackendQt::beginClipRegion()
{
    m_mask.clear();

    m_painter.end();
    m_painter.begin(&m_mask);

    m_definingClip=true;
    // TODO: How to remember that we're defining the clip?
    // m_color=Qt::color1;
}

void VasGLBackendQt::endClipRegion()
{
    m_painter.end();

    m_definingClip=false;

    // m_pixmap.setMask(m_mask);

    m_painter.begin(&m_pixmap);
    m_painter.setRenderHint(QPainter::Antialiasing);

    m_painter.setClipRegion(QRegion(m_mask));
    m_painter.setClipping(true);
}

void VasGLBackendQt::disableClipping()
{
    m_painter.end();

    // m_pixmap.setMask(QBitmap());

    m_painter.begin(&m_pixmap);
    m_painter.setRenderHint(QPainter::Antialiasing);

    m_painter.setClipping(false);
}

void VasGLBackendQt::reserveTexIndexes(int num, GLuint *indexes)
{
    // Since a texture index of 0 is not permissible, create a dummy texture
    // with an index of 0 if necessary
    if(m_textures.size()==0)
        m_textures.push_back(NULL);

    for(int i=0; i<num; i++)
    {
        indexes[i]=m_textures.size();
        m_textures.push_back(new QPixmap());
    }
}

void VasGLBackendQt::setTexture(QImage img)
{
    if(m_textureIdx!=0)
        *m_textures[m_textureIdx]=QPixmap::fromImage(img);
}

void VasGLBackendQt::selectTexture(int idx)
{
    m_textureIdx=idx;
}

// private:

void VasGLBackendQt::setPen(const QColor &color)
{
    if(m_definingClip)
        m_painter.setPen(QPen(Qt::color1, m_lineWidth));
    else
        m_painter.setPen(QPen(color, m_lineWidth));
}

void VasGLBackendQt::setBrush(const QColor &color)
{
    if(m_definingClip)
        m_painter.setBrush(QBrush(Qt::color1));
    else
        m_painter.setBrush(QBrush(color));
}
