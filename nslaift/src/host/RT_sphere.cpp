#include "RT_sphere.h"
#include <spdlog.h>

RT_sphere::RT_sphere(optix::Context &context, optix::Group &root_group, RT_object *parent) :
        RT_object(context, parent),
        m_rootGroup(root_group){

    m_ObjType = "geometry";

    m_geom_inst = m_context->createGeometryInstance();
    m_sphere = m_context->createGeometry();

    spdlog::debug("Assigning itersection and bounding box programs to sphere object");
    std::string ptx_path_ipg(rthelpers::ptxPath("sphere_intersect.cu"));
    m_intersection_program = m_context->createProgramFromPTXFile(ptx_path_ipg, "intersect");
    m_bounding_box_program = m_context->createProgramFromPTXFile(ptx_path_ipg, "bounds");
    m_sphere->setBoundingBoxProgram(m_bounding_box_program);
    m_sphere->setIntersectionProgram(m_intersection_program);
    m_sphere->setPrimitiveCount(1u);

    m_geom_inst->setGeometry(m_sphere);
    m_geom_inst->addMaterial(m_material->m_material_optix);

    spdlog::debug("Creating geometry group for sphere object");
    m_geom_group = m_context->createGeometryGroup();
    m_geom_group->setAcceleration(m_context->createAcceleration("Trbvh"));
    m_geom_group->addChild(m_geom_inst);

    spdlog::debug("Creating transform for sphere object");
    m_transform_optix = m_context->createTransform();
    m_transform_optix->setChild(m_geom_group);
    m_transform_optix->setMatrix(false, m_transform.getData(), m_transform.inverse().getData());

    spdlog::debug("Assigning transform of sphere object to top group");
    m_rootGroup->addChild(m_transform_optix);
}

RT_sphere::~RT_sphere() {
    spdlog::debug("Deleting sphere object: {}", m_strName.toUtf8().constData());
    int idx = m_rootGroup->getChildIndex(m_transform_optix);
    m_sphere->destroy();
    m_geom_inst->destroy();
    m_geom_group->destroy();
    m_rootGroup->removeChild(idx);
}

void RT_sphere::setRadius(float r) {
    m_radius = r;
}

int RT_sphere::updateCache() {
    spdlog::debug("Updating caches of sphere object {}", m_strName.toUtf8().constData());
    m_rootGroup->getAcceleration()->markDirty();
    m_geom_group->getAcceleration()->markDirty();
    m_geom_inst->setMaterial(0, m_material->m_material_optix);
    m_intersection_program["radius"]->setFloat(m_radius);
    m_bounding_box_program["radius"]->setFloat(m_radius);
    m_intersection_program["Rt"]->setMatrix4x4fv(false, m_transform.getData());
    m_bounding_box_program["Rt"]->setMatrix4x4fv(false, m_transform.getData());
}

/**
  @brief    parse parameters
  @param    action  string describing action to perform
  @param    params  action parameters
  @return   0 on success, negative on error, positive if action not found (use child class action)

  first all "local" actions are looked up, if none found then base class RTobject::parseActions() is called
  **/
int RT_sphere::parseActions(const QString &action, const QString &parameters) {
    spdlog::debug("Parsing parameter {0} for action {1} on camera object {2}", parameters.toUtf8().constData(),
                  action.toUtf8().constData(), m_strName.toUtf8().constData());
    if((0 == action.compare("setRadius", Qt::CaseInsensitive)) || (0 == action.compare("radius", Qt::CaseInsensitive)))
    {
        bool ok = false;
        float radius = parameters.toFloat(&ok);
        if (ok)
        {
            setRadius(radius);
        } else {
            spdlog::error("Could not convert the entered radius to float for sphere object");
        }
    } else {
        int ret = RT_object::parseActions(action, parameters);
        return ret;
    }
    return 0;
}