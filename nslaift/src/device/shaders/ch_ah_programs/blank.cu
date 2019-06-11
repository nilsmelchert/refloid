#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optix_world.h>
#include <optixu/optixu_math_namespace.h>
#include <internal/optix_defines.h>

#include "includes/per_ray_data_gpu.h"

using namespace optix;

rtDeclareVariable(float3, shading_normal, attribute shading_normal, );

rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );
rtDeclareVariable(PerRayData_shadow,   prd_shadow,   rtPayload, );

rtDeclareVariable(optix::float3, color, ,);

RT_PROGRAM void any_hit()
{
    // this material is opaque, so it fully attenuates all shadow rays
    prd_shadow.visible = false;
    rtTerminateRay();
}

RT_PROGRAM void closest_hit()
{
    prd_radiance.radiance = make_float3(1.0f);
}
