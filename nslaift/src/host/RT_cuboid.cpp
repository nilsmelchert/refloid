#include "RT_cuboid.h"

RT_cuboid::RT_cuboid(optix::Context &context, optix::Group &root_group, RT_object *parent):
    RT_object(context, parent),
    m_rootGroup(root_group){

    m_ObjType = "geometry";

    m_geom_inst = m_context->createGeometryInstance();
    m_cuboid = m_context->createGeometry();

    create_verticies();

    m_attributesBuffer = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_USER);
    m_attributesBuffer->setElementSize(sizeof(VertexAttributes));
    m_attributesBuffer->setSize(m_attributes.size());

    m_indicesBuffer  = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_UNSIGNED_INT3, m_indices.size() / 3);

    update_vertices();

    spdlog::debug("Assigning itersection and bounding box programs to cuboid object");
    std::string ptx_path_ipg(rthelpers::ptxPath("triangle_intersect.cu"));
    m_intersection_program = m_context->createProgramFromPTXFile(ptx_path_ipg, "intersect");
    m_bounding_box_program = m_context->createProgramFromPTXFile(ptx_path_ipg, "bounds");
    m_cuboid->setBoundingBoxProgram(m_bounding_box_program);
    m_cuboid->setIntersectionProgram(m_intersection_program);
    m_cuboid->setPrimitiveCount((unsigned int)(m_indices.size() / 3));

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
    // Thinking about making averything triangle based
    m_transform_optix->setMatrix(false, m_transform.getData(), m_transform.inverse().getData());
    m_rootGroup->getAcceleration()->markDirty();
    m_geom_group->getAcceleration()->markDirty();

    m_geom_inst->setMaterial(0, m_material->m_material_optix);
    m_geom_inst["Rt"]->setMatrix4x4fv(false, m_transform.getData());

    create_verticies();
    update_vertices();
    m_cuboid["attributesBuffer"]->setBuffer(m_attributesBuffer);
    m_cuboid["indicesBuffer"]->setBuffer(m_indicesBuffer);
}

int RT_cuboid::parseActions(const QString &action, const QString &parameters) {
    // TODO parsing minmax for cuboid object
    return RT_object::parseActions(action, parameters);
}

void RT_cuboid::setMinMax(optix::float3 min, optix::float3 max) {
    // TODO: Setting parameters: front, back, top, etc.
    spdlog::debug("Setting min and max coordinates for cuboid object \"{}\" to min: {}, {}, {} and max: {}, {}, {}", m_strName.toStdString(), min.x, min.y, min.z, max.x, max.y, max.z);
//    m_cuboid_min = min;
//    m_cuboid_max = max;
}

void RT_cuboid::setMinMax(float xmin, float ymin, float zmin, float xmax, float ymax, float zmax)
{
    setMinMax(optix::make_float3(xmin, ymin, zmin), optix::make_float3(xmax, ymax, zmax));
}

void RT_cuboid::create_verticies()
{

    VertexAttributes attrib;
    m_attributes.clear();
    m_indices.clear();

    // Left.
    attrib.tangent   = optix::make_float3(0.0f, 0.0f, 1.0f);
    attrib.normal    = optix::make_float3(-1.0f, 0.0f, 0.0f);

    attrib.vertex    = optix::make_float3(m_left, m_bottom, m_back);
    attrib.texcoord  = optix::make_float3(0.0f, 0.0f, 0.0f);
    m_attributes.push_back(attrib);

    attrib.vertex    = optix::make_float3(m_left, m_bottom, m_front);
    attrib.texcoord  = optix::make_float3(1.0f, 0.0f, 0.0f);
    m_attributes.push_back(attrib);

    attrib.vertex    = optix::make_float3(m_left, m_top, m_front);
    attrib.texcoord  = optix::make_float3(1.0f, 1.0f, 0.0f);
    m_attributes.push_back(attrib);

    attrib.vertex    = optix::make_float3(m_left, m_top, m_back);
    attrib.texcoord  = optix::make_float3(0.0f, 1.0f, 0.0f);
    m_attributes.push_back(attrib);

    // Right.
    attrib.tangent   = optix::make_float3(0.0f, 0.0f, -1.0f);
    attrib.normal    = optix::make_float3(1.0f, 0.0f,  0.0f);

    attrib.vertex    = optix::make_float3(m_right, m_bottom, m_front);
    attrib.texcoord  = optix::make_float3(0.0f, 0.0f, 0.0f);
    m_attributes.push_back(attrib);

    attrib.vertex    = optix::make_float3(m_right, m_bottom, m_back);
    attrib.texcoord  = optix::make_float3(1.0f, 0.0f, 0.0f);
    m_attributes.push_back(attrib);

    attrib.vertex    = optix::make_float3(m_right, m_top, m_back);
    attrib.texcoord  = optix::make_float3(1.0f, 1.0f, 0.0f);
    m_attributes.push_back(attrib);

    attrib.vertex    = optix::make_float3(m_right, m_top, m_front);
    attrib.texcoord  = optix::make_float3(0.0f, 1.0f, 0.0f);
    m_attributes.push_back(attrib);

    // Back.
    attrib.tangent   = optix::make_float3(-1.0f, 0.0f, 0.0f);
    attrib.normal    = optix::make_float3(0.0f, 0.0f, -1.0f);

    attrib.vertex    = optix::make_float3(m_right, m_bottom, m_back);
    attrib.texcoord  = optix::make_float3(0.0f, 0.0f, 0.0f);
    m_attributes.push_back(attrib);

    attrib.vertex    = optix::make_float3(m_left, m_bottom, m_back);
    attrib.texcoord  = optix::make_float3(1.0f, 0.0f, 0.0f);
    m_attributes.push_back(attrib);

    attrib.vertex    = optix::make_float3(m_left, m_top, m_back);
    attrib.texcoord  = optix::make_float3(1.0f, 1.0f, 0.0f);
    m_attributes.push_back(attrib);

    attrib.vertex    = optix::make_float3(m_right, m_top, m_back);
    attrib.texcoord  = optix::make_float3(0.0f, 1.0f, 0.0f);
    m_attributes.push_back(attrib);

    // Front.
    attrib.tangent   = optix::make_float3(1.0f, 0.0f,  0.0f);
    attrib.normal    = optix::make_float3(0.0f, 0.0f, 1.0f);

    attrib.vertex    = optix::make_float3(m_left, m_bottom, m_front);
    attrib.texcoord  = optix::make_float3(0.0f, 0.0f, 0.0f);
    m_attributes.push_back(attrib);

    attrib.vertex    = optix::make_float3(m_right, m_bottom, m_front);
    attrib.texcoord  = optix::make_float3(1.0f, 0.0f, 0.0f);
    m_attributes.push_back(attrib);

    attrib.vertex    = optix::make_float3(m_right, m_top, m_front);
    attrib.texcoord  = optix::make_float3(1.0f, 1.0f, 0.0f);
    m_attributes.push_back(attrib);

    attrib.vertex    = optix::make_float3(m_left, m_top, m_front);
    attrib.texcoord  = optix::make_float3(0.0f, 1.0f, 0.0f);
    m_attributes.push_back(attrib);

    // Bottom.
    attrib.tangent   = optix::make_float3(1.0f, 0.0f,  0.0f);
    attrib.normal    = optix::make_float3(0.0f, -1.0f, 0.0f);

    attrib.vertex    = optix::make_float3(m_left, m_bottom, m_back);
    attrib.texcoord  = optix::make_float3(0.0f, 0.0f, 0.0f);
    m_attributes.push_back(attrib);

    attrib.vertex    = optix::make_float3(m_right, m_bottom, m_back);
    attrib.texcoord  = optix::make_float3(1.0f, 0.0f, 0.0f);
    m_attributes.push_back(attrib);

    attrib.vertex    = optix::make_float3(m_right, m_bottom, m_front);
    attrib.texcoord  = optix::make_float3(1.0f, 1.0f, 0.0f);
    m_attributes.push_back(attrib);

    attrib.vertex    = optix::make_float3(m_left, m_bottom, m_front);
    attrib.texcoord  = optix::make_float3(0.0f, 1.0f, 0.0f);
    m_attributes.push_back(attrib);

    // Top.
    attrib.tangent   = optix::make_float3(1.0f, 0.0f,  0.0f);
    attrib.normal    = optix::make_float3( 0.0f, 1.0f, 0.0f);

    attrib.vertex    = optix::make_float3(m_left, m_top, m_front);
    attrib.texcoord  = optix::make_float3(0.0f, 0.0f, 0.0f);
    m_attributes.push_back(attrib);

    attrib.vertex    = optix::make_float3(m_right, m_top, m_front);
    attrib.texcoord  = optix::make_float3(1.0f, 0.0f, 0.0f);
    m_attributes.push_back(attrib);

    attrib.vertex    = optix::make_float3(m_right, m_top, m_back);
    attrib.texcoord  = optix::make_float3(1.0f, 1.0f, 0.0f);
    m_attributes.push_back(attrib);

    attrib.vertex    = optix::make_float3(m_left, m_top, m_back);
    attrib.texcoord  = optix::make_float3(0.0f, 1.0f, 0.0f);
    m_attributes.push_back(attrib);

    for (unsigned int i = 0; i < 6; ++i)
    {
        const unsigned int idx = i * 4; // Four attributes per box face.

        m_indices.push_back(idx    );
        m_indices.push_back(idx + 1);
        m_indices.push_back(idx + 2);

        m_indices.push_back(idx + 2);
        m_indices.push_back(idx + 3);
        m_indices.push_back(idx    );
    }

}

void RT_cuboid::update_vertices()
{
    void *dst = m_attributesBuffer->map(0, RT_BUFFER_MAP_WRITE_DISCARD);
    memcpy(dst, m_attributes.data(), sizeof(VertexAttributes) * m_attributes.size());
    m_attributesBuffer->unmap();

    dst = m_indicesBuffer->map(0, RT_BUFFER_MAP_WRITE_DISCARD);
    memcpy(dst, m_indices.data(), sizeof(optix::uint3) * m_indices.size() / 3);
    m_indicesBuffer->unmap();
}
