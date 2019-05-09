#ifndef NSLAIFT_RT_SPHERE_H
#define NSLAIFT_RT_SPHERE_H

#include "RT_object.h"

#include <optix.h>
#include <sutil.h>

class RT_sphere : virtual  public RT_object {
public:
    RT_sphere(optix::Context &context, optix::Group &root_group, RT_object *parent = nullptr);
    ~RT_sphere();

public:
    int updateCache() override;
    int parseActions(const QString &action, const QString &parameters) override;

    void setRadius(float r);

    optix::Group &m_rootGroup;
    optix::Program m_intersection_program;
    optix::Program m_bounding_box_program;
    optix::Geometry m_sphere;
    optix::GeometryInstance m_geom_inst;
    optix::GeometryGroup m_geom_group;
    optix::Material m_material;
    optix::Transform m_transform_optix;
    float m_radius = 0.1f;
};


#endif //NSLAIFT_RT_SPHERE_H
