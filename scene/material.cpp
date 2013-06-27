//https://code.google.com/p/nya-engine/

#include "material.h"

namespace nya_scene
{

void material::set() const
{
    m_shader.set();

    for(int i=0;i<(int)m_params.size();++i)
    {
        const param_proxy &p=m_params[i].p;
        if(!p.is_valid())
        {
            const param_array_proxy &a=m_params[i].a;
            if(a.is_valid() && a->get_count()>0)
            {
                m_shader.set_uniform4_array(i,a->m_params[0].f,a->get_count());
                continue;
            }

            m_shader.set_uniform_value(i,0,0,0,0);
            continue;
        }

        const param_proxy &m=m_params[i].m;
        if(m.is_valid())
            m_shader.set_uniform_value(i,p->f[0]*m->f[0],p->f[1]*m->f[1],p->f[2]*m->f[2],p->f[3]*m->f[3]);
        else
            m_shader.set_uniform_value(i,p->f[0],p->f[1],p->f[2],p->f[3]);
    }

    if(m_blend)
        nya_render::blend::enable(m_blend_src,m_blend_dst);
    else
        nya_render::blend::disable();

    if(m_color_write)
        nya_render::color_write::enable();
    else
        nya_render::color_write::disable();

    if(m_zwrite)
        nya_render::zwrite::enable();
    else
        nya_render::zwrite::disable();

    if(m_cull_face)
        nya_render::cull_face::enable(m_cull_order);
    else
        nya_render::cull_face::disable();

    for(size_t i=0;i<m_textures.size();++i)
    {
        if(m_textures[i].slot<0)
            continue;

        if(!m_textures[i].proxy.is_valid())
        {
            nya_render::texture::select_multitex_slot(m_textures[i].slot);
            nya_render::texture::unbind();
            continue;
        }

        m_textures[i].proxy->set(m_textures[i].slot);
    }
}

void material::unset() const
{
    m_shader.unset();

    if(m_blend)
        nya_render::blend::disable();

    if(!m_zwrite)
        nya_render::zwrite::enable();

    if(!m_color_write)
        nya_render::color_write::enable();

    for(size_t i=0;i<m_textures.size();++i)
    {
        if(m_textures[i].slot<0 || !m_textures[i].proxy.is_valid())
            continue;

        m_textures[i].proxy->unset();
    }
}

void material::set_shader(const shader &shdr)
{
    m_shader.unload();

    m_shader=shdr;

    for(size_t i=0;i<m_textures.size();++i)
        m_textures[i].slot=m_shader.get_texture_slot(m_textures[i].semantics.c_str());

    m_params.clear();
    m_params.resize(m_shader.get_uniforms_count());
}

void material::set_texture(const char *semantics,const texture &tex)
{
    if(!semantics)
        return;

    set_texture(semantics,texture_proxy(tex));
}

void material::set_texture(const char *semantics,const texture_proxy &proxy)
{
    if(!semantics)
        return;

    for(size_t i=0;i<m_textures.size();++i)
    {
        material_texture &t=m_textures[i];
        if(t.semantics!=semantics)
            continue;

        t.proxy=proxy;
        t.slot=m_shader.get_texture_slot(semantics);
        return;
    }

    m_textures.resize(m_textures.size()+1);
    m_textures.back().proxy=proxy;
    m_textures.back().semantics.assign(semantics);
    m_textures.back().slot=m_shader.get_texture_slot(semantics);
}

void material::set_blend(bool enabled,blend_mode src,blend_mode dst)
{
    m_blend=enabled;
    m_blend_src=src;
    m_blend_dst=dst;
}

const texture_proxy &material::get_texture(int idx) const
{
    if(idx<0 || idx>=(int)m_textures.size() )
    {
        static texture_proxy invalid;
        return invalid;
    }

    return m_textures[idx].proxy;
}

const char *material::get_texture_semantics(int idx) const
{
    if(idx<0 || idx>=(int)m_textures.size())
        return 0;

    return m_textures[idx].semantics.c_str();
}

const char *material::get_param_name(int idx) const
{
    if(idx<0 || idx>=(int)m_params.size())
        return 0;

    return m_shader.get_uniform(idx).name.c_str();
}

int material::get_param_idx(const char *name) const
{
    if(!name)
        return -1;

    for(int i=0;i<m_shader.get_uniforms_count();++i)
    {
        if(m_shader.get_uniform(i).name.compare(name)==0)
            return i;
    }

    return -1;
}

void material::set_param(int idx,float f0,float f1,float f2,float f3)
{
    set_param(idx,param(f0,f1,f2,f3));
}

void material::set_param(int idx,const param &p)
{
    set_param(idx,param_proxy(p));
}

void material::set_param(int idx,const param_proxy &p)
{
    if(idx<0 || idx>=(int)m_params.size())
        return;

    m_params[idx].p=p;
    m_params[idx].m.free();
    m_params[idx].a.free();
}

void material::set_param(int idx,const param_proxy &p,const param &m)
{
    set_param(idx,p,param_proxy(m));
}

void material::set_param(int idx,const param_proxy &p,const param_proxy &m)
{
    if(idx<0 || idx>=(int)m_params.size())
        return;

    m_params[idx].p=p;
    m_params[idx].m=m;
    m_params[idx].a.free();
}

void material::set_param_array(int idx,const param_array & a)
{
    set_param_array(idx,param_array_proxy(a));
}

void material::set_param_array(int idx,const param_array_proxy & p)
{
    if(idx<0 || idx>=(int)m_params.size())
        return;

    m_params[idx].p.free();
    m_params[idx].m.free();
    m_params[idx].a=p;
}

const material::param_proxy &material::get_param(int idx) const
{
    if(idx<0 || idx>=(int)m_params.size())
    {
        static param_proxy invalid;
        return invalid;
    }

    return m_params[idx].p;
}

const material::param_proxy &material::get_param_multiplier(int idx) const
{
    if(idx<0 || idx>=(int)m_params.size())
    {
        static param_proxy invalid;
        return invalid;
    }

    return m_params[idx].m;
}

const material::param_array_proxy &material::get_param_array(int idx) const
{
    if(idx<0 || idx>=(int)m_params.size())
    {
        static param_array_proxy invalid;
        return invalid;
    }

    return m_params[idx].a;
}

void material::release()
{
    for(size_t i=0;i<m_textures.size();++i)
        m_textures[i].proxy.free();

    m_textures.clear();
    m_params.clear();
    m_shader.unload();
    m_name.clear();
}

}
