#include <optix.h>
#include <optixu/optixu_math_namespace.h>

#include "includes/per_ray_data_gpu.h"

using namespace optix;

rtDeclareVariable(float3, shading_normal, attribute shading_normal, );
rtDeclareVariable(optix::float3, front_hit_point, attribute front_hit_point,);

rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );
rtDeclareVariable(PerRayData_shadow,   prd_shadow,   rtPayload, );


RT_PROGRAM void any_hit()
{
    // this material is opaque, so it fully attenuates all shadow rays
    prd_shadow.visible = false;
    rtTerminateRay();
}

RT_PROGRAM void closest_hit()
{
    prd_radiance.radiance = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal))*0.5f + 0.5f;
}
