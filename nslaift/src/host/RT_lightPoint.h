#ifndef NSLAIFT_RT_LIGHTPOINT_H
#define NSLAIFT_RT_LIGHTPOINT_H

#include "RT_object.h"
#include "RT_lightSource.h"

#include "includes/light_definition.h"

class RT_lightPoint : public RT_lightSource
{
public:
    RT_lightPoint(optix::Context &context, RT_object *parent = nullptr);
    virtual ~RT_lightPoint();

    virtual void setDecayRadius(float radius);
    virtual float decayRadius();

    virtual int parseActions(const QString &action, const QString &parameters);
    virtual int updateCache();

public:
    float m_decayRadius;
    optix::Program m_point_light_prgm;
};


#endif //NSLAIFT_RT_LIGHTPOINT_H
