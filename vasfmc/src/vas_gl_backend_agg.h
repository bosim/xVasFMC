// vas_gl_backend_agg.h

#ifndef VASGLBACKENDAGG_H
#define VASGLBACKENDAGG_H

#include "vas_gl.h"

#include <agg_alpha_mask_u8.h>
#include <agg_conv_curve.h>
#include <agg_conv_dash.h>
#include <agg_conv_transform.h>
#include <agg_image_accessors.h>
#include <agg_path_storage.h>
#include <agg_pixfmt_amask_adaptor.h>
#include <agg_pixfmt_gray.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rasterizer_outline_aa.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_renderer_base.h>
#include <agg_renderer_outline_aa.h>
#include <agg_renderer_scanline.h>
#include <agg_rendering_buffer.h>
#include <agg_scanline_p.h>
#include <agg_span_allocator.h>
#include <agg_span_image_filter_gray.h>
#include <agg_trans_affine.h>

#include <inttypes.h>

template<class Source, class Interpolator>
class span_image_filter_gray_to_rgba
{
public:
    typedef Source source_type;
    typedef Interpolator interpolator_type;
    typedef agg::rgba8 color_type;

    span_image_filter_gray_to_rgba(source_type &src, interpolator_type &inter)
        : m_filter(src, inter)
    {
    }

    void prepare()
    {
        m_filter.prepare();
    }

    void set_color(color_type color)
    {
        m_color=color;
    }

    void __attribute__((__noinline__)) generate(color_type *span, int x, int y, unsigned len)
    {
        typename source_type::color_type *source_span;
        int                              i;
        unsigned                         val;

        source_span=(typename source_type::color_type *)span;

        // First, generate a span in the source type
        m_filter.generate(source_span, x, y, len);

        //TODOmemset(span, 0, sizeof(color_type)*len);
        // Now, convert to rgba8
        for(i=len-1; i>=0; i--)
        {
            val=source_span[i].v;
            if(val==0)
                span[i].clear();
            else if(val==255)
                span[i]=m_color;
            else
            {
                span[i]=m_color;
                span[i].premultiply(val);
            }
        }
    }

private:
    agg::span_image_filter_gray_bilinear<source_type, interpolator_type>
        m_filter;
    color_type m_color;
};

class LineProfileCache
{
public:
    LineProfileCache();

    ~LineProfileCache();

    agg::line_profile_aa *getProfile(double width);

private:
    static const int MAX_CACHE_SIZE=10;

    QVector<agg::line_profile_aa *> m_line_profiles;
    QVector<double>                 m_widths;
};

template <class pixfmt>
class AGGRenderPipeline
{
public:
    AGGRenderPipeline()
        : m_ren_outline_aa(m_renbase, *m_line_profile_cache.getProfile(1)),
          m_ras_outline_aa(m_ren_outline_aa),
          m_ren_scanline_aa(m_renbase)
    {
    }

    void attach(pixfmt &pf)
    {
        m_renbase.attach(pf);
    }

    void clip_box(const agg::rect_d &box)
    {
        m_ren_outline_aa.clip_box(box.x1, box.y1, box.x2, box.y2);
        m_ras_scanline_aa.clip_box(box.x1, box.y1, box.x2, box.y2);
    }

    void clear(typename pixfmt::color_type color)
    {
        m_renbase.clear(color);
    }

    template <class vertex_source>
    void renderLines(double width, typename pixfmt::color_type color,
        vertex_source &vs)
    {
        m_ren_outline_aa.profile(*m_line_profile_cache.getProfile(width));

        m_ren_outline_aa.color(color);
        m_ras_outline_aa.add_path(vs, 0);
    } 

    template <class vertex_source>
    void renderPolygons(typename pixfmt::color_type color, vertex_source &vs)
    {
        m_ren_scanline_aa.color(color);
        m_ras_scanline_aa.add_path(vs);

        agg::render_scanlines(m_ras_scanline_aa, m_scanline,
            m_ren_scanline_aa);
    }

    template <class pixfmt_img, class vertex_source>
    void renderTexture(const pixfmt_img &the_pixfmt,
        typename pixfmt::color_type color,
        vertex_source &vs, const agg::trans_affine &trans)
    {
        typedef agg::image_accessor_clone<pixfmt_img> source_type;
        source_type accessor(the_pixfmt);
        
        typedef agg::span_interpolator_linear<agg::trans_affine>
            interpolator_type;
        interpolator_type interpolator(trans);

        agg::span_allocator<typename pixfmt::color_type> span_allocator;

        span_image_filter_gray_to_rgba<source_type, interpolator_type>
            filter(accessor, interpolator);
        filter.set_color(color);

        // Prepare destination polygon in rasterizer
        m_ras_scanline_aa.add_path(vs);

        agg::render_scanlines_aa(m_ras_scanline_aa, m_scanline, m_renbase,
            span_allocator, filter);
    }

    typedef agg::renderer_base<pixfmt>                 renbase;
    typedef agg::renderer_outline_aa<renbase>          ren_outline_aa;
    typedef agg::rasterizer_outline_aa<ren_outline_aa> ras_outline_aa;
    typedef agg::renderer_scanline_aa_solid<renbase>   ren_scanline_aa;

private:
    LineProfileCache              m_line_profile_cache;
    renbase                       m_renbase;
    ren_outline_aa                m_ren_outline_aa;
    ras_outline_aa                m_ras_outline_aa;
    ren_scanline_aa               m_ren_scanline_aa;
    agg::rasterizer_scanline_aa<> m_ras_scanline_aa;
    agg::scanline_p8              m_scanline;
};

class VasGLBackendAGG
{
public:
    VasGLBackendAGG();

    ~VasGLBackendAGG();

    void init(int width, int height);

    void end();

    void attach(QImage *pimg);

    void detach();

    void clear(QColor color);

    // Attributes
    void setTransform(const QTransform &transform);
    void setLineWidth(double pixels);
    void setLineStipple(int length, unsigned pattern);
    void enableLineStipple(bool stipple);

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
    static void reserveTexIndexes(int num, GLuint *indexes);
    void setTexture(QImage img);
    void selectTexture(int idx);

private:
    // Disallow copy construction and assignment (not defined)
    VasGLBackendAGG(const VasGLBackendAGG &);
    VasGLBackendAGG &operator=(const VasGLBackendAGG &);

    // TODO
    // - Use bgra32_pre for performance?
    // - Don't set clip box every time, have ability to save it
    // - Make sure we always reset the rasterizer

    // typedef agg::pixfmt_bgra32                            pixfmt_type;
    typedef agg::pixfmt_bgra32_pre                        pixfmt_type;
    typedef agg::conv_transform<agg::path_storage>        conv_transform;
    typedef agg::conv_curve<conv_transform>               conv_curve;
    typedef agg::conv_dash<conv_curve>                    conv_dash;
    static const int BYTES_PER_PIXEL=4;

    typedef agg::pixfmt_gray8                             pixfmt_mask;
    typedef agg::renderer_base<pixfmt_mask>               renbase_mask;
    typedef agg::renderer_scanline_aa_solid<renbase_mask> ren_mask_aa;
    static const int BYTES_PER_PIXEL_MASK=1;

    typedef agg::pixfmt_amask_adaptor<pixfmt_type, agg::alpha_mask_gray8>
        pixfmt_with_mask;

    void renderLines(const QColor &color);

    template <class vertex_source>
    void renderPolygons(const QColor &color, vertex_source &vs)
    {
        if(m_definingClip)
            m_pipeline_mask.renderPolygons(agg::gray8(255), vs);
        else if(m_haveClip)
            m_pipeline_with_mask.renderPolygons(aggColor(color), vs);
        else
            m_pipeline.renderPolygons(aggColor(color), vs);
    }

    template <class pixfmt_img, class vertex_source>
    void renderTexture(const pixfmt_img &the_pixfmt,
        typename pixfmt_type::color_type color,
        vertex_source &vs, const agg::trans_affine &trans)
    {
        if(m_haveClip)
            m_pipeline_with_mask.renderTexture(the_pixfmt, color, vs, trans);
        else
            m_pipeline.renderTexture(the_pixfmt, color, vs, trans);
    }

    pixfmt_type::color_type aggColor(const QColor &color)
    {
        return agg::rgba8(color.red(), color.green(), color.blue(),
            color.alpha());
    }

    // Buffer, renderers, rasterizers for main bitmap
    agg::rendering_buffer m_rbuf;
    pixfmt_type           m_pixfmt;
    pixfmt_with_mask      m_pixfmt_with_mask;
    AGGRenderPipeline<pixfmt_type> m_pipeline;
    AGGRenderPipeline<pixfmt_with_mask> m_pipeline_with_mask;

    // Buffer, renderers for mask
    uint8_t               *m_buffer_mask;
    agg::rendering_buffer m_rbuf_mask;
    pixfmt_mask           m_pixfmt_mask;
    agg::alpha_mask_gray8 m_alpha_mask;
    AGGRenderPipeline<pixfmt_mask> m_pipeline_mask;

    // Paths and transformations
    agg::path_storage     m_path_storage;
    agg::trans_affine     m_trans;
    conv_transform        m_conv_transform;
    conv_curve            m_conv_curve;
    conv_dash             m_conv_dash;
    double                m_max_scale;
    agg::rect_d           m_clip_box;

    double                m_lineWidth;
    bool                  m_stippleLines;
    int                   m_textureIdx;
    bool                  m_definingClip, m_haveClip;
};

#endif // VASGLBACKENDAGG_H
