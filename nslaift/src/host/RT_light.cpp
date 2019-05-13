//
// Created by melchert on 13.05.19.
//

#include "RT_light.h"

RT_light::RT_light(optix::Context &context, RT_object *parent) : RT_object(context, parent)
{
    if (!m_isLightInitialized)
    {
        setUpLightTypes();
    }
    initDefaultLightDefinition();
    m_light_definitions.append(m_light_definition);

    // Create buffer to store the vector of light definitions in and send them to device side
    m_buffer_light_parameters = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_USER);
    m_buffer_light_parameters->setElementSize(sizeof(LightDefinition));

// TODO: Make a new class called projector that inherits from this class and the camera class as "parent" so that it can use the camera matrix
}

RT_light::~RT_light() {

}

void RT_light::initDefaultLightDefinition()
{
    // Initialize light as pointlight which is located in origin in world coordinates
    m_light_definition.type = LIGHT_POINTLIGHT;
    m_light_definition.Rt = m_light_definition.Rt_inv = optix::Matrix4x4::identity();
    m_light_definition.K = m_light_definition.K_inv = optix::Matrix4x4::identity(); // This parameter is only needed for projectors
    m_light_definition.emission = optix::make_float3(130000.0f, 130000.0f, 13.0000f);
    m_light_definition.normal = optix::make_float3(0.0f, 0.0f, 1.0f);
    m_light_definition.area = 1.0f;
}

void RT_light::setUpLightTypes()
{
    optix::Buffer bufferLights = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_PROGRAM_ID, 1000); // 1000 lights should be enough ;-)
    int* sampleLight = (int*) bufferLights->map(0, RT_BUFFER_MAP_WRITE_DISCARD);
    std::string ptx_path_lights(rthelpers::ptxPath("lights.cu")); // ptx path ray generation program
    optix::Program point_light_prgm = m_context->createProgramFromPTXFile(ptx_path_lights, "point_light");
    optix::Program parallelogram_light_prgm = m_context->createProgramFromPTXFile(ptx_path_lights, "parallelogram_light");
    optix::Program projector_light_prgm = m_context->createProgramFromPTXFile(ptx_path_lights, "projector_light");

    sampleLight[LIGHT_POINTLIGHT] = point_light_prgm->getId();
    sampleLight[LIGHT_PARALLELOGRAM] = parallelogram_light_prgm->getId();
    sampleLight[LIGHT_PROJECTOR] = projector_light_prgm->getId();
    bufferLights->unmap();

    // sysSampleLight will be called in closest hit program
    m_context["sysLightTypes"]->setBuffer(bufferLights);
    m_isLightInitialized = true;
}

bool RT_light::m_isLightInitialized = false;

QVector<LightDefinition> RT_light::m_light_definitions;

void RT_light::setPower(float pow) {

}

float RT_light::power() {
    return 0;
}

void RT_light::setColor(optix::float3 rgb) {

}

void RT_light::setColor(float r, float g, float b) {

}

optix::float3 RT_light::color() const {
    return optix::float3();
}

int RT_light::parseActions(const QString &action, const QString &parameters) {
    // TODO: Parsing parameters
    return RT_object::parseActions(action, parameters);
}

int RT_light::updateCache() {
    // TODO: check if size can be updated without mapping the buffer
    m_buffer_light_parameters->setSize(m_light_definitions.size());
    void *dst = static_cast<LightDefinition *>(m_buffer_light_parameters->map(0, RT_BUFFER_MAP_WRITE_DISCARD));
    memcpy(dst, m_light_definitions.data(), sizeof(LightDefinition) * m_light_definitions.size());
    m_buffer_light_parameters->unmap();
    m_context["sysLightParameters"]->setBuffer(m_buffer_light_parameters);
    return 0;
}
