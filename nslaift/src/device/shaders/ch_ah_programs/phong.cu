#include <optix.h>
#include <optix_world.h>
#include <optixu/optixu_math_namespace.h>
#include <internal/optix_defines.h>

#include "includes/per_ray_data_gpu.h"

using namespace optix;

// TODO: need to be implemented
//rtDeclareVariable(int,               max_depth, , );
//rtBuffer<BasicLight>                 lights;
//rtDeclareVariable(float3,            ambient_light_color, , );
//rtDeclareVariable(float,             scene_epsilon, , );
//rtDeclareVariable(rtObject,          top_object, , );

rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, );
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );

rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );
rtDeclareVariable(PerRayData_shadow,   prd_shadow,   rtPayload, );

rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );

rtDeclareVariable(float, intersectionDist, rtIntersectionDistance, );

RT_PROGRAM void any_hit()
{
    // this material is opaque, so it fully attenuates all shadow rays
    rtTerminateRay();
    prd_shadow.visible = false;
}

RT_PROGRAM void closest_hit()
{
    float3 front_hit_point = ray.origin + intersectionDist * ray.direction;

    const float3 world_shading_normal = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, shading_normal) );
    const float3 world_geometric_normal = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, geometric_normal ) );
    //Calculating the face forward normal N
    const float3 N = faceforward( world_shading_normal, -ray.direction, world_geometric_normal );

    const float3 wo = -ray.direction; // Vector to camera

    const float3 fhp_world = rtTransformPoint(RT_OBJECT_TO_WORLD, front_hit_point);

    prd_radiance.origin = fhp_world;

//    for (int i; i<num_lights; i++) {
//        //Do stuff here
//      LICHTER HIER
//    }



    // Shading with Phong-BRDF using the parameters Kd, Ka, Ks, Kr, phong_exp and ffnormal

//    float3 hit_point = ray.origin + t_hit * ray.direction;

}
