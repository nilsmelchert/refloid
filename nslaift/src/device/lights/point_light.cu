
#include "includes/app_config.h"
#include "includes/light_definition.h"
#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optix_cuda.h>
#include "optix_math.h"

rtDeclareVariable(optix::Matrix4x4, Rt, ,);
rtDeclareVariable(optix::Matrix4x4, Rt_inv, ,);
rtDeclareVariable(optix::float3, color, ,);

RT_CALLABLE_PROGRAM void light(float3 const& point, const float2 sample, LightDefinition& lightDef)
{
    float3 light_position = make_float3(Rt.getCol(3));
    lightDef.distance = length(light_position - point);
    lightDef.wi = normalize(light_position - point); //light_definition direction from surface to light_definition
    lightDef.area = 1.0f ;
    lightDef.solid_angle = 4.0f * M_PIf* lightDef.area / (powf(fmaxf(lightDef.distance , DENOMINATOR_EPSILON ), 2.0f));

    float4 point_4f = make_float4(point);
    point_4f.w = 1.0f;
    lightDef.emission = color;
}
