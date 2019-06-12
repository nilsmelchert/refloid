//
// Created by melchert on 11.06.19.
//

#ifndef NSLAIFT_RT_CUBOID_H
#define NSLAIFT_RT_CUBOID_H

#include "RT_object.h"
#include "includes/vertex_attributes.h"

#include <optix.h>
#include <sutil.h>

class RT_cuboid : virtual public RT_object {
public:
    RT_cuboid(optix::Context &context, optix::Group &root_group, RT_object *parent = nullptr);
    ~RT_cuboid();

public:
    int updateCache() override;
    int parseActions(const QString &action, const QString &parameters) override;

    void create_verticies();
    void update_vertices();

    void setMinMax(optix::float3 min, optix::float3 max);
    void setMinMax(float xmin, float ymin, float zmin, float xmax, float ymax, float zmax);

    optix::Group &m_rootGroup;
    optix::Program m_intersection_program;
    optix::Program m_bounding_box_program;
    optix::Geometry m_cuboid;
    optix::GeometryInstance m_geom_inst;
    optix::GeometryGroup m_geom_group;
    optix::Transform m_transform_optix;

//    optix::float3 m_cuboid_min = optix::make_float3(-2.0f, -2.0f, 8.0f);
//    optix::float3 m_cuboid_max = optix::make_float3(2.0f, 2.0f, 12.0f);

    optix::Buffer m_attributesBuffer;
    optix::Buffer m_indicesBuffer;

    float m_left = -1.0f;
    float m_right = 1.0f;
    float m_bottom = -1.0f;
    float m_top = 1.0f;
    float m_back = -1.0f;
    float m_front = 1.0f;

    std::vector<VertexAttributes> m_attributes;
    std::vector<unsigned int> m_indices;

};


#endif //NSLAIFT_RT_CUBOID_H
