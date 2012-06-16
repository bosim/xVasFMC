// vas_gl_backend_qt.h

#ifndef VASGLBACKENDQT_H
#define VASGLBACKENDQT_H

#include "vas_gl.h"

#include <QBitmap>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QVector>

class VasGLBackendQt
{
public:
    VasGLBackendQt();

    ~VasGLBackendQt();

    void init(int width, int height);

    void end();

    QImage toImage();

    QPixmap toPixmap();

    void clear(QColor color);

    // Attributes
    void setTransform(const QTransform &transform);
    void setLineWidth(double pixels);

    // Drawing
    void drawPrimitives(GLenum mode, const QVector<QPointF> &vertices,
        const QVector<QColor> &colors, const QVector<QPointF> &texCoords);
    void drawCircle(double cx, double cy, double radius,
        double start_angle, double stop_angle, const QColor &color);
    void drawFilledCircle(double cx, double cy, double radius,
        double start_angle, double stop_angle, const QColor &color);

    // Clipping
    void beginClipRegion();
    void endClipRegion();
    void disableClipping();

    // Textures
    void reserveTexIndexes(int num, GLuint *indexes);
    void setTexture(QImage img);
    void selectTexture(int idx);

private:
    // Disallow copy construction and assignment (not defined)
    VasGLBackendQt(const VasGLBackendQt &);
    VasGLBackendQt &operator=(const VasGLBackendQt &);

    void setPen(const QColor &color);
    void setBrush(const QColor &color);

    QPixmap            m_pixmap;
    QBitmap            m_mask;
    QPainter           m_painter;
    QVector<QPixmap *> m_textures;
    double             m_lineWidth;
    int                m_textureIdx;
    bool               m_definingClip;
};

#endif // VASGLBACKENDQT_H
