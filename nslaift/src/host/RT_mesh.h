#ifndef NSLAIFT_RT_MESH_H
#define NSLAIFT_RT_MESH_H

#include "RT_object.h"

#include <optix.h>
#include <sutil.h>
#include <OptiXMesh.h>


class RT_mesh : virtual  public RT_object {
public:
    RT_mesh(optix::Context &context, optix::Group &root_group, RT_object *parent = nullptr);
    ~RT_mesh();

public:
    int updateCache() override;
    int parseActions(const QString &action, const QString &parameters) override;
    void loadMeshPly(const QString &file_name);

private:
    optix::Geometry m_mesh;
    optix::Group &m_rootGroup;
    optix::Program m_intersection_program;
    optix::Program m_bounding_box_program;
//    optix::GeometryInstance m_geom_inst;
    optix::GeometryGroup m_geom_group;
    optix::Transform m_transform_optix;
    OptiXMesh m_optix_mesh;
};

#endif //NSLAIFT_RT_MESH_H
