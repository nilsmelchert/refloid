#ifndef NSLAIFT_RT_LIGHT_H
#define NSLAIFT_RT_LIGHT_H

#include "RT_object.h"
#include "includes/light_definition.h"

#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_stream_namespace.h>
#include <optixu_math_namespace.h>
#include <QVector>

class RT_light : virtual public RT_object
{
public:
    RT_light(optix::Context &context, RT_object *parent = nullptr);
    ~RT_light();

    void setPower(float pow);
    float power();


    void setColor(optix::float3 rgb);
    void setColor(float r, float g, float b);
    optix::float3 color() const;

    int parseActions(const QString &action, const QString &parameters);
    int updateCache();
    LightDefinition m_light_definition;
    static QVector<LightDefinition> m_light_definitions;

private:
    static bool m_isLightInitialized;
    void setUpLightTypes();
    void initDefaultLightDefinition();
    optix::Buffer m_buffer_light_parameters;
};


#endif //NSLAIFT_RT_LIGHT_H
