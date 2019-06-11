//
// Created by melchert on 11.06.19.
//

#include "RT_cuboid.h"

RT_cuboid::RT_cuboid(optix::Context &context, optix::Group &root_group, RT_object *parent):
    RT_object(context, parent),
    m_rootGroup(root_group){

    m_ObjType = "geometry";

    m_geom_inst = m_context->createGeometryInstance();
    m_cuboid = m_context->createGeometry();

    spdlog::debug("Assigning itersection and bounding box programs to cuboid object");
    std::string ptx_path_ipg(rthelpers::ptxPath("cuboid_intersect.cu"));
    m_intersection_program = m_context->createProgramFromPTXFile(ptx_path_ipg, "intersect");
    m_bounding_box_program = m_context->createProgramFromPTXFile(ptx_path_ipg, "bounds");
    m_cuboid->setBoundingBoxProgram(m_bounding_box_program);
    m_cuboid->setIntersectionProgram(m_intersection_program);
    m_cuboid->setPrimitiveCount(1u);

    m_geom_inst->setGeometry(m_cuboid);
    m_geom_inst->addMaterial(m_material->m_material_optix);

    spdlog::debug("Creating geometry group for cuboid object");
    m_geom_group = m_context->createGeometryGroup();
    m_geom_group->setAcceleration(m_context->createAcceleration("Trbvh"));
    m_geom_group->addChild(m_geom_inst);

    spdlog::debug("Creating transform for cuboid object");
    m_transform_optix = m_context->createTransform();
    m_transform_optix->setChild(m_geom_group);
    m_transform_optix->setMatrix(false, m_transform.getData(), m_transform.inverse().getData());

    spdlog::debug("Assigning transform of cuboid object to top group");
    m_rootGroup->addChild(m_transform_optix);
}

RT_cuboid::~RT_cuboid() {
    spdlog::debug("Deleting cuboid object: \"{}\"", m_strName.toUtf8().constData());
    int idx = m_rootGroup->getChildIndex(m_transform_optix);
    m_cuboid->destroy();
    m_geom_inst->destroy();
    m_geom_group->destroy();
    m_rootGroup->removeChild(idx);
}

int RT_cuboid::updateCache() {
    spdlog::debug("Updating caches of cuboid object {}", m_strName.toUtf8().constData());
    // TODO: Seems like the intersection program needs some modification in order to be able to handle rotations
    m_transform_optix->setMatrix(false, m_transform.getData(), m_transform.inverse().getData());
    m_rootGroup->getAcceleration()->markDirty();
    m_geom_group->getAcceleration()->markDirty();

    m_geom_inst->setMaterial(0, m_material->m_material_optix);
    m_geom_inst["boxmin"]->setFloat(m_cuboid_min);
    m_geom_inst["boxmax"]->setFloat(m_cuboid_max);
    m_geom_inst["Rt"]->setMatrix4x4fv(false, m_transform.getData());
}

int RT_cuboid::parseActions(const QString &action, const QString &parameters) {
    // TODO parsing minmax for cuboid object
    return RT_object::parseActions(action, parameters);
}

void RT_cuboid::setMinMax(optix::float3 min, optix::float3 max) {
    spdlog::debug("Setting min and max coordinates for cuboid object \"{}\" to min: {}, {}, {} and max: {}, {}, {}", m_strName.toStdString(), min.x, min.y, min.z, max.x, max.y, max.z);
    m_cuboid_min = min;
    m_cuboid_max = max;
}

void RT_cuboid::setMinMax(float xmin, float ymin, float zmin, float xmax, float ymax, float zmax)
{
    setMinMax(optix::make_float3(xmin, ymin, zmin), optix::make_float3(xmax, ymax, zmax));
}
