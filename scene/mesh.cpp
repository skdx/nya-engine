//https://code.google.com/p/nya-engine/

#include "mesh.h"
#include "scene.h"
#include "render/render.h"
#include "memory/tmp_buffer.h"
#include "memory/memory_reader.h"

namespace nya_scene
{

bool mesh::load_pmd(shared_mesh &res,resource_data &data,const char* name)
{
    if(!data.get_size())
        return false;

    nya_memory::memory_reader reader(data.get_data(),data.get_size());
    if(!reader.test("Pmd",3))
        return false;

    if(reader.read<float>()!=1.0f)
        return false;

    reader.skip(20+256); //name and comment

    typedef unsigned int uint;
    typedef unsigned short ushort;
    typedef unsigned char uchar;

    uint vert_count=reader.read<uint>();
    const size_t pmd_vert_size=sizeof(float)*8+sizeof(ushort)*2+sizeof(uchar)*2;
    if(!reader.check_remained(pmd_vert_size*vert_count))
        return false;

    std::vector<float> vertices;
    vertices.resize(vert_count*11);
    for(size_t i=0;i<vertices.size();i+=11)
    {
        for(int j=0;j<8;++j)
            vertices[i+j]=reader.read<float>();

        vertices[i+8]=reader.read<ushort>();
        vertices[i+9]=reader.read<ushort>();
        vertices[i+10]=reader.read<uchar>()/255.0f;
        reader.skip(1);
    }

    res.vbo.set_vertex_data(&vertices[0],sizeof(float)*11,vert_count);
    res.vbo.set_normals(3*sizeof(float));
    res.vbo.set_tc(0,6*sizeof(float),2);
    res.vbo.set_tc(1,8*sizeof(float),3); //skin info

    vertices.clear();

    uint ind_count=reader.read<uint>();
    if(!reader.check_remained(sizeof(ushort)*ind_count))
        return false;

    res.vbo.set_index_data(data.get_data(reader.get_offset()),nya_render::vbo::index2b,ind_count);

    return true;
}

void mesh::unload()
{
    scene_shared::unload();

    for(size_t i=0;i<m_replaced_materials.size();++i)
        m_replaced_materials[i].release();

    m_replaced_materials.clear();
    m_replaced_materials_idx.clear();
}

void mesh::draw(int idx) const
{
    if(!m_shared.is_valid() || idx>=(int)m_shared->groups.size())
        return;

    nya_scene_internal::transform::set(m_transform);

    m_shared->vbo.bind();

    if(m_replaced_materials_idx.empty())
    {
        if(idx>=0)
        {
            const shared_mesh::group &g=m_shared->groups[idx];
            g.mat.set();
            m_shared->vbo.draw(g.offset,g.count);
            g.mat.unset();
        }
        else
        {
            for(size_t i=0;i<m_shared->groups.size();++i)
            {
                const shared_mesh::group &g=m_shared->groups[i];
                g.mat.set();
                m_shared->vbo.draw(g.offset,g.count);
                g.mat.unset();
            }
        }
    }
    else
    {
        if(idx>=0)
        {
            const shared_mesh::group &g=m_shared->groups[idx];
            const int rep_idx=m_replaced_materials_idx[idx];
            if(rep_idx>=0)
            {
                m_replaced_materials[rep_idx].set();
                m_shared->vbo.draw(g.offset,g.count);
                m_replaced_materials[rep_idx].unset();
            }
            else
            {
                g.mat.set();
                m_shared->vbo.draw(g.offset,g.count);
                g.mat.unset();
            }
        }
        else
        {
            for(size_t i=0;i<m_shared->groups.size();++i)
            {
                const shared_mesh::group &g=m_shared->groups[i];
                const int rep_idx=m_replaced_materials_idx[i];
                if(rep_idx>=0)
                {
                    m_replaced_materials[rep_idx].set();
                    m_shared->vbo.draw(g.offset,g.count);
                    m_replaced_materials[rep_idx].unset();
                }
                else
                {
                    g.mat.set();
                    m_shared->vbo.draw(g.offset,g.count);
                    g.mat.unset();
                }
            }
        }
    }

    m_shared->vbo.unbind();
}

int mesh::get_materials_count() const
{
    if(!m_shared.is_valid())
        return 0;

    return (int)m_shared->groups.size();
}

const material &mesh::get_material(int idx) const
{
    if(!m_shared.is_valid() || idx<0 || idx>=(int)m_shared->groups.size())
    {
        const static material invalid;
        return invalid;
    }

    if(!m_replaced_materials.empty())
    {
        const int replace_idx=m_replaced_materials_idx[idx];
        if(replace_idx>=0)
            return m_replaced_materials[replace_idx];
    }

    return m_shared->groups[idx].mat;
}

material &mesh::modify_material(int idx)
{
    if(!m_shared.is_valid() || idx<0 || idx>=(int)m_shared->groups.size())
    {
        static material invalid;
        invalid=material();
        return invalid;
    }

    if(m_replaced_materials.empty())
        m_replaced_materials_idx.resize(m_shared->groups.size(),-1);

    int &replace_idx=m_replaced_materials_idx[idx];
    if(replace_idx<0)
    {
        replace_idx=(int)m_replaced_materials.size();
        m_replaced_materials.resize(m_replaced_materials.size()+1);
        m_replaced_materials.back()=m_shared->groups[idx].mat;
    }

    return m_replaced_materials[replace_idx];
}

void mesh::set_material(int idx,const material &mat)
{
    if(!m_shared.is_valid() || idx<0 || idx>=(int)m_shared->groups.size())
        return;

    if(m_replaced_materials_idx.empty())
        m_replaced_materials_idx.resize(m_shared->groups.size(),-1);

    if(m_replaced_materials_idx[idx]>=0)
    {
        m_replaced_materials[m_replaced_materials_idx[idx]]=mat;
        return;
    }

    m_replaced_materials_idx[idx]=(int)m_replaced_materials.size();
    m_replaced_materials.resize(m_replaced_materials.size()+1);
    m_replaced_materials.back()=mat;
}

const nya_render::skeleton &mesh::get_skeleton() const
{
    if(!m_shared.is_valid())
    {
        static nya_render::skeleton invalid;
        return invalid;
    }

    return m_shared->skeleton;
}

}
