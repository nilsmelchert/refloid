#include "RT_object.h"
#include "RT_lightSource.h"

/**
  @brief    ambient light source constructor
  @param    parent parental object

  implicitely sets power to 1
  **/
RT_lightSource::RT_lightSource(optix::Context &context, RT_object *parent /*NULL*/) : RT_object(context, parent)
{
    m_ObjType = "light";
    m_baseColor = optix::make_float3(1.0f);     ///< multiplicator of color
    m_power = 1.0f;                         ///< base color
    if (!m_isBufferInitialized)
    {
        m_context["sysLightBuffer"]->setBuffer(m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_PROGRAM_ID, 1000));
        m_isBufferInitialized = true;
    }
    RT_lightSource::m_light_count++;
    m_light_idx = RT_lightSource::m_light_count - 1;
}

/**
  @brief    lazy destructor
  **/
RT_lightSource::~RT_lightSource() {
    RT_lightSource::m_light_count--;
}

bool RT_lightSource::m_isBufferInitialized = false;

unsigned int RT_lightSource::m_light_count = 0;

/**
  @brief    set light source power
  @param    pow power to set
  **/
void RT_lightSource::setPower(float pow) {
    m_power = pow;
    m_baseColor *= m_power;
}

/**
  @brief    get light source power
  @return   light source power

  Careful this is not content of m_dPower but
  its combination with the gray level equivalent of the color
  **/
float RT_lightSource::power() {
    return m_power * (m_baseColor.x + m_baseColor.y + m_baseColor.z) / 3.0f;
}

/**
  @brief    set light source color
  @param    r   red color part
  @param    g   green color part
  @param    b   blue color part
  **/
void RT_lightSource::setColor(float r, float g, float b) {
    setColor(optix::make_float3(r, g, b));
}

/**
  @brief    set light source color
  @param    col desired color
  **/
void RT_lightSource::setColor(optix::float3 color) {
    m_baseColor = color;
}

/**
  @brief    get light source base color
  @return   content of m_colBase
  **/
optix::float3 RT_lightSource::color() const {
    return m_baseColor;
}

/**
  @brief    parse parameters
  @param    action  string describing action to perform
  @param    params  action parameters
  @return   0 on success, negative on error, positive if action not found (use child class action)

  first all "local" actions are looked up, if none found then base class RTobject::parseActions() is called
  **/
int RT_lightSource::parseActions(const QString &action, const QString &parameters) {
    if((0 == action.compare("setColor", Qt::CaseInsensitive)) || (0 == action.compare("color", Qt::CaseInsensitive))) {
        float r,g,b;
        if(0 == rthelpers::RT_parse_float3(parameters, &r, &g, &b)) {
            setColor(r, g, b);
            return 0;
        }
        return -1;
    } else if((0 == action.compare("setPower", Qt::CaseInsensitive)) || (0 == action.compare("power", Qt::CaseInsensitive))) {
        bool ok = true;
        float power = parameters.toFloat(&ok);
        if (ok) {
            setPower(power);
            return 0;
        }
        return -1;
    } else {
        return RT_object::parseActions(action, parameters);
    }
}

/**
  @brief    update all transformation cache variables
  @return   returns 0 on success; non-zero on errors
  **/
int RT_lightSource::updateCache() {
    // Dummy implementation
    return 0; ///> This function should be overloaded by classes which inherit from this class e.g. RT_lightPoint
}
