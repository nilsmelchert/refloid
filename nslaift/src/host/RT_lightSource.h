
#ifndef NSLAIFT_RT_LIGHTSOURCE_H
#define NSLAIFT_RT_LIGHTSOURCE_H

#include "RT_object.h"
#include "RT_helper.h"


class RT_lightSource : virtual public RT_object
{
public:
    RT_lightSource(optix::Context &context, RT_object* parent = nullptr);
    virtual ~RT_lightSource();

    virtual void setPower(float pow);
    virtual float power();

    virtual void setColor(float r, float g, float b);
    virtual void setColor(optix::float3 color);
    virtual optix::float3 color() const;

    virtual int parseActions(const QString &action, const QString &parameters);
    virtual int updateCache();

public:
    optix::float3 m_baseColor;
    float m_power;

    static bool m_isBufferInitialized; ///< The buffer has to be initialized once only! So its is defined as static.
    static unsigned int m_light_count; ///< This is the counter for all light sources that are assigned to the scene
    unsigned int m_light_idx; ///< This variable holds the index of the light source. Classes which inherit from this class need to explicitly set this variable
};


#endif //NSLAIFT_RT_LIGHTSOURCE_H
