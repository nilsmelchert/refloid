#include "RT_object.h"
#include "RT_lightSource.h"
#include "RT_lightPoint.h"

/**
  @brief    simple constructor
  @param    parent parental object
**/
//RT_lightPoint::RT_lightPoint(optix::Context &context, RT_object *parent /*=NULL*/) : RT_lightSource(context, parent) // Stack overflow: https://stackoverflow.com/questions/56184941/how-to-fix-error-no-matching-function-for-call-to-when-inheriting-twice-from
RT_lightPoint::RT_lightPoint(optix::Context &context, RT_object *parent /*=NULL*/) : RT_object(context, parent), RT_lightSource(context, parent) ///-> Need to call RT_object explicitly since its is a virtual lightSource inherits from a virtual RT_object
{
    m_ObjType = "light";
    m_decayRadius = 1.0f;
    std::string ptx_path_lights(rthelpers::ptxPath("point_light.cu")); // ptx path ray generation program
    m_point_light_prgm = m_context->createProgramFromPTXFile(ptx_path_lights, "light");
    m_light_count++;
    m_light_idx = m_light_count - 1;
}

/**
  @brief    lazy destructor
**/
RT_lightPoint::~RT_lightPoint() {
}

/**
  @brief    set decay radius m_dRadiusDecay
  @param    radius  new decay radius

  negative radii are allowed in principal but "sqared away".
  Radius 0 is replaced by the value 1.0
**/
void RT_lightPoint::setDecayRadius(float radius)
{
    if (radius == 0.0f) {
        m_decayRadius = 1.0f;
    } else {
        m_decayRadius = radius;
    }
}

float RT_lightPoint::decayRadius()
{
    return m_decayRadius;
}

/**
  @brief    parse parameters
  @param    action  string describing action to perform
  @param    params  action parameters
  @return   0 on success, negative on error, positive if action not found (use child class action)

  first all "local" actions are looked up, if none found then base class RTlightSource::parseActions() is called
  **/
int RT_lightPoint::parseActions(const QString &action, const QString &parameters) {
    if((0 == action.compare("decayradius", Qt::CaseInsensitive)) || (0 == action.compare("setdecayradius", Qt::CaseInsensitive))) {
        bool ok;
        float r = parameters.toFloat(&ok);
        if (ok) {
            setDecayRadius(r);
            return 0;
        }
        return -1;
    } else {
        int ret = RT_lightSource::parseActions(action, parameters);
        return ret;
    }
}

/**
  @brief    update all transformation cache variables
  @return   returns 0 on success; non-zero on errors
**/
int RT_lightPoint::updateCache() {
    m_point_light_prgm["Rt"]->setMatrix4x4fv(false, m_transform.getData());
    m_point_light_prgm["Rt_inv"]->setMatrix4x4fv(false, m_transform.inverse().getData());
    m_point_light_prgm["color"]->setFloat(m_baseColor * m_power);

    optix::Buffer LightsBuffer = m_context["sysLightBuffer"]->getBuffer();

    int* sampleLight = (int*) LightsBuffer->map(0, RT_BUFFER_MAP_READ_WRITE);
    sampleLight[m_light_count-1] = m_point_light_prgm->getId();
    LightsBuffer->unmap();
    m_context["light_count"]->setUint(m_light_count);

    return 0;
}
