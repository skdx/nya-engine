//https://code.google.com/p/nya-engine/

#ifndef texture_h
#define texture_h

namespace nya_render
{

class texture
{
public:
    enum color_format
    {
        color_rgb,
        color_bgr,
        color_rgba,
        color_bgra
    };

    void build_texture(void *data,unsigned int width,unsigned int height,color_format format);

public:
    void bind();
    void unbind();

public:
    void release();

public:
    texture(): m_tex_id(0) {}

private:
    unsigned int m_tex_id;
};

}

#endif