#include "RT_mesh.h"
#include <spdlog.h>

RT_mesh::RT_mesh(optix::Context &context, optix::Group &root_group, RT_object *parent) :
        RT_object(context, parent),
        m_rootGroup(root_group){

    m_ObjType = "geometry";
    m_optix_mesh.use_tri_api = false;

    spdlog::debug("Assigning itersection and bounding box programs to mesh object");
    std::string ptx_path_ipg(rthelpers::ptxPath("mesh_intersect.cu"));
    m_bounding_box_program = m_context->createProgramFromPTXFile(ptx_path_ipg, "bounds");
    m_intersection_program = m_context->createProgramFromPTXFile(ptx_path_ipg, "intersect");

    m_optix_mesh.context = m_context;
    m_optix_mesh.bounds = m_bounding_box_program;
    m_optix_mesh.intersection = m_intersection_program;
    m_optix_mesh.material = m_material->m_material_optix;
    this->loadMeshPly("/home/melchert/Desktop/Projects/raytracing/refloid/res/data/turbineblade_axis_translated.ply");

    spdlog::debug("Creating geometry group for mesh object");
    m_geom_group = m_context->createGeometryGroup();
    m_geom_group->setAcceleration(m_context->createAcceleration("Trbvh"));
    m_geom_group->addChild(m_optix_mesh.geom_instance);

    spdlog::debug("Creating transform for mesh object");
    m_transform_optix = m_context->createTransform();
    m_transform_optix->setChild(m_geom_group);
    m_transform_optix->setMatrix(false, m_transform.getData(), m_transform.inverse().getData());

    spdlog::debug("Assigning transform of mesh object to top group");
    m_rootGroup->addChild(m_transform_optix);
}

RT_mesh::~RT_mesh() {
    spdlog::debug("Deleting mesh object: \"{}\"", m_strName.toUtf8().constData());
    int idx = m_rootGroup->getChildIndex(m_transform_optix);
    m_mesh->destroy();
    m_optix_mesh.geom_instance->destroy();
    m_geom_group->destroy();
    m_rootGroup->removeChild(idx);
}

void RT_mesh::loadMeshPly(const QString &file_name) {
    spdlog::debug("Loading Mesh PLY-file \"{}\"", file_name.toStdString());
    loadMesh(file_name.toStdString(), m_optix_mesh);
}

int RT_mesh::updateCache() {
    spdlog::debug("Updating caches of mesh object {}", m_strName.toUtf8().constData());
    m_transform_optix->setMatrix(false, m_transform.getData(), m_transform.inverse().getData());
    m_rootGroup->getAcceleration()->markDirty();
    m_geom_group->getAcceleration()->markDirty();
    m_optix_mesh.geom_instance->setMaterial(0, m_material->m_material_optix); //TODO
}

/**
  @brief    parse parameters
  @param    action  string describing action to perform
  @param    params  action parameters
  @return   0 on success, negative on error, positive if action not found (use child class action)

  first all "local" actions are looked up, if none found then base class RTobject::parseActions() is called
  **/
int RT_mesh::parseActions(const QString &action, const QString &parameters) {
    spdlog::debug("Parsing parameter {0} for action {1} on mesh object {2}", parameters.toUtf8().constData(),
                  action.toUtf8().constData(), m_strName.toUtf8().constData());
    if((0 == action.compare("load_mesh", Qt::CaseInsensitive)) || (0 == action.compare("set_mesh", Qt::CaseInsensitive)))
    {
        this->loadMeshPly(parameters);
    } else {
        int ret = RT_object::parseActions(action, parameters);
        return ret;
    }
    return 0;
}
