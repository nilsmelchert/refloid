
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

};


#endif //NSLAIFT_RT_LIGHTSOURCE_H
