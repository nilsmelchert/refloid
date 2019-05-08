#include "RT_sphere.h"
#include <spdlog.h>

RT_sphere::RT_sphere(optix::Context &context, optix::Group &root_group, RT_object *parent) :
        RT_object(context, parent),
        m_rootGroup(root_group){

    m_geom_inst = m_context->createGeometryInstance();
    m_sphere = m_context->createGeometry();

    spdlog::debug("Assigning itersection and bounding box programs to sphere object");
    std::string ptx_path_ipg(rthelpers::ptxPath("sphere_intersect.cu"));
    m_intersection_program = m_context->createProgramFromPTXFile(ptx_path_ipg, "intersect");
    m_bounding_box_program = m_context->createProgramFromPTXFile(ptx_path_ipg, "bounds");
    m_sphere->setBoundingBoxProgram(m_bounding_box_program);
    m_sphere->setIntersectionProgram(m_intersection_program);
    m_sphere->setPrimitiveCount(1u);


    // TODO: This needs to be in a seperate Material class
    spdlog::debug("Creating material for sphere object. WARNING: This has to be moved to a seperate material class!");
    m_material = m_context->createMaterial();
    std::string ptx_path_ch(rthelpers::ptxPath("ch_normal.cu"));
    optix::Program ch_pgrm = m_context->createProgramFromPTXFile(ptx_path_ch, "closest_hit_radiance");
    m_material->setClosestHitProgram(0, ch_pgrm);
    // TODO: This needs to be in a seperate Material class


    m_geom_inst->setGeometry(m_sphere);
    m_geom_inst->setMaterialCount(1);
    m_geom_inst->setMaterial(0, m_material);

    spdlog::debug("Creating geometry group for sphere object");
    m_geom_group = m_context->createGeometryGroup();
    m_geom_group->setAcceleration(m_context->createAcceleration("Trbvh"));
    m_geom_group->setChildCount(1);
    m_geom_group->setChild(0, m_geom_inst);

    spdlog::debug("Creating transform for sphere object");
    m_transform_optix = m_context->createTransform();
    m_transform_optix->setChild(m_geom_group);
    m_transform_optix->setMatrix(false, m_transform.getData(), m_transform.inverse().getData());

    spdlog::debug("Assigning transform of sphere object to top group");
    m_rootGroup->setChildCount(m_rootGroup->getChildCount()+1);
    m_rootGroup->setChild(m_rootGroup->getChildCount()-1, m_transform_optix);
    spdlog::debug("Assigning transform of sphere object to top group");
}

void RT_sphere::setRadius(float r) {
    m_radius = r;
}

int RT_sphere::updateCache() {
    m_intersection_program["radius"]->setFloat(m_radius);
    m_bounding_box_program["radius"]->setFloat(m_radius);
    m_intersection_program["Rt"]->setMatrix4x4fv(false, m_transform.getData());
    m_bounding_box_program["Rt"]->setMatrix4x4fv(false, m_transform.getData());
}