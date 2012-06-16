// vas_gl_backend_agg.cpp

#include "vas_gl_backend_agg.h"

#include "logger.h"

#include <agg_conv_stroke.h>

#include <cstdlib>
#include <cstring>

static QVector<agg::rendering_buffer *> s_textures;

static void agg_error_callback(const char *msg)
{
    Logger::log(msg);
}

namespace
{
    struct StaticInitExit
    {
        ~StaticInitExit()
        {
            for(int i=0; i<s_textures.size(); i++)
                if(s_textures[i]!=0)
                {
                    // Delete memory buffer for texture
                    delete [] s_textures[i]->buf();

                    // Delete rendering buffer
                    delete s_textures[i];
                }
        }
    } s_staticInitExit;
}

VasGLBackendAGG::VasGLBackendAGG()
    : m_pixfmt_with_mask(m_pixfmt, m_alpha_mask),
      m_buffer_mask(NULL), m_pixfmt_mask(m_rbuf_mask),
      m_alpha_mask(m_rbuf_mask),
      m_conv_transform(m_path_storage, m_trans),
      m_conv_curve(m_conv_transform), m_conv_dash(m_conv_curve),
      m_max_scale(1), m_clip_box(0, 0, 0, 0), m_lineWidth(1),
      m_stippleLines(false), m_definingClip(false), m_haveClip(false)
{
}

VasGLBackendAGG::~VasGLBackendAGG()
{
    delete [] m_buffer_mask;
}

void VasGLBackendAGG::init(int width, int height)
{
    m_lineWidth=1.0;
    m_textureIdx=0;

    delete [] m_buffer_mask;
    m_buffer_mask=new uint8_t[width*height*BYTES_PER_PIXEL_MASK];

    m_pixfmt.attach(m_rbuf);
    m_pipeline.attach(m_pixfmt);
    m_pipeline_with_mask.attach(m_pixfmt_with_mask);

    m_rbuf_mask.attach(m_buffer_mask, width, height,
        width*BYTES_PER_PIXEL_MASK);
    m_pixfmt_mask.attach(m_rbuf_mask);
    m_pipeline_mask.attach(m_pixfmt_mask);

    m_clip_box=agg::rect_d(0, 0, width-1, height-1);
    m_pipeline_with_mask.clip_box(m_clip_box);
    m_pipeline_mask.clip_box(m_clip_box);
    m_pipeline.clip_box(m_clip_box);

    agg::curve4_div::set_error_callback(agg_error_callback);
}

void VasGLBackendAGG::end()
{
}

void VasGLBackendAGG::attach(QImage *pimg)
{
    // Image must have correct size and format
    MYASSERT(pimg->width()==int(m_rbuf_mask.width()) &&
        pimg->height()==int(m_rbuf_mask.height()) &&
        pimg->format()==QImage::Format_ARGB32);

    // Attach image to render buffer
    m_rbuf.attach(pimg->bits(), pimg->width(), pimg->height(),
        pimg->bytesPerLine());

    // Apparently, we have to attach the whole pipeline to the render buffer
    // again... don't know why we do, but that's the way it is.
    m_pixfmt.attach(m_rbuf);
    m_pipeline.attach(m_pixfmt);
    m_pipeline_with_mask.attach(m_pixfmt_with_mask);
}

void VasGLBackendAGG::detach()
{
    m_rbuf.attach(NULL, 0, 0, 0);
}

void VasGLBackendAGG::clear(QColor color)
{
    if(m_haveClip)
        m_pipeline_with_mask.clear(aggColor(color));
    else if(!m_definingClip)
    {
        // Do it by hand, because this is faster than m_pipeline.clear()
        pixfmt_type::pixel_type pix, *ppix;
        int y, num_pix;

        m_pixfmt.make_pix((agg::int8u *)&pix, aggColor(color));

        for(y=0; y<(int)m_pixfmt.height(); y++)
        {
            ppix=(pixfmt_type::pixel_type *)m_pixfmt.row_ptr(y);
            for(num_pix=m_pixfmt.width(); num_pix>0; num_pix--)
                *ppix++=pix;
        }
    }

    // Don't do anything for m_definingClip
}

void VasGLBackendAGG::setTransform(const QTransform &transform)
{
    QMatrix mtx;

    mtx=transform.toAffine();

    m_trans=agg::trans_affine(mtx.m11(), mtx.m12(), mtx.m21(), mtx.m22(),
        mtx.dx(), mtx.dy());
    m_conv_transform.transformer(m_trans);

    // Compute the maximum scaling that the transform carries out (scaling may
    // vary according to the direction). This is equivalent to the Euclidian
    // matrix norm of the 2x2 submatrix, which we approximate using the
    // Frobenius norm.
    m_max_scale=sqrt(m_trans.sx*m_trans.sx + m_trans.sy*m_trans.sy +
        m_trans.shx*m_trans.shx + m_trans.shy*m_trans.shy);
}

void VasGLBackendAGG::setLineWidth(double pixels)
{
    m_lineWidth=pixels;
}

void VasGLBackendAGG::setLineStipple(int length, unsigned pattern)
{
    int i, dashLength, gapLength;

    m_conv_dash.remove_all_dashes();

    i=0;
    while(i<16)
    {
        dashLength=0;
        while(i<16 && (pattern & (1<<i)))
        {
            dashLength++;
            i++;
        }
        gapLength=0;
        while(i<16 && !(pattern & (1<<i)))
        {
            gapLength++;
            i++;
        }

        m_conv_dash.add_dash(dashLength*length, gapLength*length);
    }

    m_conv_dash.dash_start(0);
}

void VasGLBackendAGG::enableLineStipple(bool stipple)
{
    m_stippleLines=stipple;
}

void VasGLBackendAGG::drawPrimitives(GLenum mode,
    const QVector<QPointF> &vertices, const QVector<QColor> &colors,
    const QVector<QPointF> &texCoords)
{
    int i;

    m_path_storage.remove_all();

    switch(mode)
    {
        // TODO: Use render_all_paths for performance?

        case GL_LINES:
            for(i=0; i<vertices.size()-1; i+=2)
            {
                m_path_storage.move_to(vertices[i].x(), vertices[i].y());
                m_path_storage.line_to(vertices[i+1].x(), vertices[i+1].y());
            }
            
            renderLines(colors[0]);
            break;

        case GL_LINE_LOOP:
        case GL_LINE_STRIP:
            m_path_storage.move_to(vertices[0].x(), vertices[0].y());
            for(i=1; i<vertices.size(); i++)
                m_path_storage.line_to(vertices[i].x(), vertices[i].y());

            if(mode==GL_LINE_LOOP && !vertices.empty())
                m_path_storage.line_to(vertices[0].x(), vertices[0].y());

            renderLines(colors[0]);
            break;

        case GL_TRIANGLES:
            for(i=0; i<vertices.size()-2; i+=3)
            {
                m_path_storage.remove_all();
                m_path_storage.move_to(vertices[i].x(), vertices[i].y());
                m_path_storage.line_to(vertices[i+1].x(), vertices[i+1].y());
                m_path_storage.line_to(vertices[i+2].x(), vertices[i+2].y());
                m_path_storage.close_polygon();

                renderPolygons(colors[i], m_conv_transform);
            }
            break;

        case GL_TRIANGLE_STRIP:
            // Test whether we have the special case for the fontrenderer
            if(texCoords.size()==4 &&
               vertices.size()==4 &&
               m_textureIdx!=0)
            {
                agg::pixfmt_gray8 pixfmt_img(*s_textures[m_textureIdx]);

                m_path_storage.remove_all();
                m_path_storage.move_to(vertices[0].x(), vertices[0].y());
                m_path_storage.line_to(vertices[1].x(), vertices[1].y());
                m_path_storage.line_to(vertices[3].x(), vertices[3].y());
                m_path_storage.line_to(vertices[2].x(), vertices[2].y());
                m_path_storage.close_polygon();

                agg::trans_affine trans;
                int width=s_textures[m_textureIdx]->width();
                int height=s_textures[m_textureIdx]->height();
                double src[6], dest[6];
                src[0]=vertices[0].x();
                src[1]=vertices[0].y();
                src[2]=vertices[1].x();
                src[3]=vertices[1].y();
                src[4]=vertices[2].x();
                src[5]=vertices[2].y();
                dest[0]=texCoords[0].x()*width;
                dest[1]=texCoords[0].y()*height;
                dest[2]=texCoords[1].x()*width;
                dest[3]=texCoords[1].y()*height;
                dest[4]=texCoords[2].x()*width;
                dest[5]=texCoords[2].y()*height;
                trans.parl_to_parl(src, dest);
                trans.premultiply_inv(m_trans);

                renderTexture(pixfmt_img, aggColor(colors[0]),
                    m_conv_transform, trans);
                break;
            }

            for(i=0; i<vertices.size()-2; i++)
            {
                m_path_storage.remove_all();
                if(i%2==0)
                {
                    m_path_storage.move_to(vertices[i].x(), vertices[i].y());
                    m_path_storage.line_to(vertices[i+1].x(),
                        vertices[i+1].y());
                }
                else
                {
                    m_path_storage.move_to(vertices[i+1].x(),
                        vertices[i+1].y());
                    m_path_storage.line_to(vertices[i].x(),
                        vertices[i].y());
                }
                m_path_storage.line_to(vertices[i+2].x(), vertices[i+2].y());
                m_path_storage.close_polygon();

                renderPolygons(colors[i], m_conv_transform);
            }
            break;

        case GL_TRIANGLE_FAN:
            for(i=1; i<vertices.size()-1; i++)
            {
                m_path_storage.move_to(vertices[0].x(), vertices[0].y());
                m_path_storage.line_to(vertices[i].x(), vertices[i].y());
                m_path_storage.line_to(vertices[i+1].x(), vertices[i+1].y());
                m_path_storage.close_polygon();
            }

            renderPolygons(colors[0], m_conv_transform);
            break;

        case GL_QUADS:
            for(i=0; i<vertices.size()-3; i+=4)
            {
                m_path_storage.remove_all();
                m_path_storage.move_to(vertices[i].x(), vertices[i].y());
                m_path_storage.line_to(vertices[i+1].x(), vertices[i+1].y());
                m_path_storage.line_to(vertices[i+2].x(), vertices[i+2].y());
                m_path_storage.line_to(vertices[i+3].x(), vertices[i+3].y());
                m_path_storage.close_polygon();

                renderPolygons(colors[i], m_conv_transform);
            }
            break;

        case GL_POLYGON:
            m_path_storage.move_to(vertices[0].x(), vertices[0].y());
            for(i=1; i<vertices.size(); i++)
                m_path_storage.line_to(vertices[i].x(), vertices[i].y());
            m_path_storage.close_polygon();

            renderPolygons(colors[0], m_conv_transform);
            break;
    }
}

void VasGLBackendAGG::drawCircle(double cx, double cy, double radius,
    double start_angle, double stop_angle, const QColor &color)
{
    double radiusTrans, cxTrans, cyTrans;

    // Do a quick rejection test to see if the circle is entirely outside the
    // clip box
    radiusTrans=m_max_scale*radius;
    cxTrans=cx;
    cyTrans=cy;
    m_trans.transform(&cxTrans, &cyTrans);
    if(cxTrans<m_clip_box.x1-radiusTrans ||
       cxTrans>m_clip_box.x2+radiusTrans ||
       cyTrans<m_clip_box.y1-radiusTrans ||
       cyTrans>m_clip_box.y2+radiusTrans)
        // Circle not visible
        return;

    // Angles run clockwise in vasFMC and AGG
    // Starting point is 12 o'clock in vasFMC, 3 o'clock in AGG
    start_angle=start_angle-M_PI/2;
    stop_angle=stop_angle-M_PI/2;

    m_path_storage.remove_all();

    agg::bezier_arc arc(cx, cy, radius, radius, start_angle,
        stop_angle-start_angle);

    m_path_storage.remove_all();
    m_path_storage.join_path(arc);

    if(m_stippleLines)
    {
        agg::conv_stroke<conv_dash> stroke(m_conv_dash);
        renderPolygons(color, stroke);
    }
    else
    {
        agg::conv_stroke<conv_curve> stroke(m_conv_curve);
        renderPolygons(color, stroke);
    }
}

void VasGLBackendAGG::drawFilledCircle(double cx, double cy, double radius,
    double start_angle, double stop_angle, const QColor &color)
{
    // Angles run clockwise in vasFMC and AGG
    // Starting point is 12 o'clock in vasFMC, 3 o'clock in AGG
    start_angle=start_angle-M_PI/2;
    stop_angle=stop_angle-M_PI/2;

    agg::bezier_arc arc(cx, cy, radius, radius, start_angle,
        stop_angle-start_angle);

    m_path_storage.remove_all();
    m_path_storage.move_to(cx, cy);
    m_path_storage.join_path(arc);
    m_path_storage.close_polygon();

    renderPolygons(color, m_conv_curve);
}

void VasGLBackendAGG::beginClipRegion()
{
    m_pipeline_mask.clear(agg::gray8(0));

    m_definingClip=true;
}

void VasGLBackendAGG::endClipRegion()
{
    int xMin, xMax, yMin, yMax, x, y;

    m_definingClip=false;
    m_haveClip=true;

    // Find bounding box of mask
    xMin=-1;
    for(x=0; x<(int)m_rbuf.width() && xMin==-1; x++)
        for(y=0; y<(int)m_rbuf.height(); y++)
            if(m_rbuf_mask.row_ptr(y)[x]!=0)
            {
                xMin=x;
                break;
            }
    xMax=-1;
    for(x=(int)m_rbuf.width()-1; x>=0 && xMax==-1; x--)
        for(y=0; y<(int)m_rbuf.height(); y++)
            if(m_rbuf_mask.row_ptr(y)[x]!=0)
            {
                xMax=x;
                break;
            }
    yMin=-1;
    for(y=0; y<(int)m_rbuf.height() && yMin==-1; y++)
        for(x=0; x<(int)m_rbuf.width(); x++)
            if(m_rbuf_mask.row_ptr(y)[x]!=0)
            {
                yMin=y;
                break;
            }
    yMax=-1;
    for(y=(int)m_rbuf.height()-1; y>=0 && yMax==-1; y--)
        for(x=0; x<(int)m_rbuf.width(); x++)
            if(m_rbuf_mask.row_ptr(y)[x]!=0)
            {
                yMax=y;
                break;
            }

    if(xMin==-1 || xMax==-1 || yMin==-1 || yMax==-1)
        m_clip_box=agg::rect_d(0, 0, 0, 0);
    else
        m_clip_box=agg::rect_d(xMin, yMin, xMax, yMax);

    m_pipeline_with_mask.clip_box(m_clip_box);
}

void VasGLBackendAGG::disableClipping()
{
    m_haveClip=false;
    m_clip_box=agg::rect_d(0, 0, m_rbuf.width()-1, m_rbuf.height()-1);
}

/* static */ void VasGLBackendAGG::reserveTexIndexes(int num, GLuint *indexes)
{
    // Since a texture index of 0 is not permissible, create a dummy texture
    // with an index of 0 if necessary
    if(s_textures.size()==0)
        s_textures.push_back(NULL);

    for(int i=0; i<num; i++)
    {
        indexes[i]=s_textures.size();
        s_textures.push_back(new agg::rendering_buffer());

        // Make sure the buffer pointer is initialized to NULL
        s_textures.back()->attach(NULL, 0, 0, 0);
    }
}

void VasGLBackendAGG::setTexture(QImage img)
{
    uint8_t *buffer;
    QRgb    *pScanline;
    int     x, y;

    // At present, we can only handle images with the ARGB32 format
    if(img.format()!=QImage::Format_ARGB32)
        return;

    if(m_textureIdx!=0)
    {
        // Allocate buffer
        buffer=new uint8_t[img.width()*img.height()];

        // Delete old buffer
        delete [] s_textures[m_textureIdx]->buf();

        // Attach buffer to rendering buffer
        s_textures[m_textureIdx]->attach(buffer, img.width(), img.height(),
            img.width());

        // Copy in image data
        for(y=0; y<img.height(); y++)
        {
            pScanline=(QRgb *)img.scanLine(y);
            for(x=0; x<img.width(); x++)
                s_textures[m_textureIdx]->row_ptr(y)[x]=qRed(pScanline[x]);
        }
    }
}

void VasGLBackendAGG::selectTexture(int idx)
{
    if(idx<0 || idx>=s_textures.size())
    {
        Logger::log("VasGLBackendAGG::selectTexture(): Warning, invalid "
            "texture index");
        return;
    }

    m_textureIdx=idx;
}

// private:

void VasGLBackendAGG::renderLines(const QColor &color)
{
    if(m_stippleLines)
    {
        if(m_haveClip)
            m_pipeline_with_mask.renderLines(m_lineWidth*m_trans.scale(),
                aggColor(color), m_conv_dash);
        else if(m_definingClip)
            m_pipeline_mask.renderLines(m_lineWidth*m_trans.scale(),
                agg::gray8(255), m_conv_dash);
        else
            m_pipeline.renderLines(m_lineWidth*m_trans.scale(), aggColor(color),
                m_conv_dash);
    }
    else
    {
        if(m_haveClip)
            m_pipeline_with_mask.renderLines(m_lineWidth*m_trans.scale(),
                aggColor(color), m_conv_transform);
        else if(m_definingClip)
            m_pipeline_mask.renderLines(m_lineWidth*m_trans.scale(),
                agg::gray8(255), m_conv_transform);
        else
            m_pipeline.renderLines(m_lineWidth*m_trans.scale(), aggColor(color),
                m_conv_transform);
    }
}

//////////////////////////////////////////////////////////////////////////////
// LineProfileCache

LineProfileCache::LineProfileCache()
{
}

LineProfileCache::~LineProfileCache()
{
    int i;

    for(i=0; i<m_line_profiles.size(); i++)
        delete m_line_profiles[i];
}

agg::line_profile_aa *LineProfileCache::getProfile(double width)
{
    agg::line_profile_aa *pProfile;
    int                  i;

    // Search for a suitable profile in the cache
    for(i=0; i<m_line_profiles.size(); i++)
        if(fabs(m_widths[i]-width)<1e-3)
            return m_line_profiles[i];

    // Didn't find a suitable profile -- create one
    pProfile=new agg::line_profile_aa();
    pProfile->width(width);

    // Have we got space left in the cache?
    if(m_line_profiles.size()<MAX_CACHE_SIZE)
    {
        // Append the line profile to the cache
        m_line_profiles.push_back(pProfile);
        m_widths.push_back(width);
    }
    else
    {
        // Choose a random cache entry to replace
        i=rand() % MAX_CACHE_SIZE;

        // Delete old profile
        delete m_line_profiles[i];

        // Store new profile
        m_line_profiles[i]=pProfile;
        m_widths[i]=width;
    }

    return pProfile;
}
