#include <optix.h>
#include <optix_cuda.h>
#include <optix_world.h>
#include <optixu/optixu_math_namespace.h>
#include <internal/optix_defines.h>

#include "includes/per_ray_data_gpu.h"
#include "includes/helpers_gpu.h"
#include "includes/app_config.h"

using namespace optix;

// TODO: need to be implemented
//rtDeclareVariable(int,               max_depth, , );
//rtBuffer<BasicLight>                 lights;
//rtDeclareVariable(float3,            ambient_light_color, , );
//rtDeclareVariable(float,             scene_epsilon, , );
//rtDeclareVariable(rtObject,          top_object, , );

// Attributes
rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, );
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );

// Semantics
rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );
rtDeclareVariable(PerRayData_shadow,   prd_shadow,   rtPayload, );
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, intersectionDist, rtIntersectionDistance, );

rtDeclareVariable(float, scene_epsilon, ,);
rtDeclareVariable(rtObject, sysTopObject, ,);

// BRDF specific variables (phong)
rtDeclareVariable(optix::float3, Kd, ,);
rtDeclareVariable(optix::float3, Ks, ,);
rtDeclareVariable(float, specular_exponent, ,);

RT_PROGRAM void any_hit()
{
    // this material is opaque, so it fully attenuates all shadow rays
    prd_shadow.visible = false;
    rtTerminateRay();
}

RT_PROGRAM void closest_hit()
{
    // Calculate front hit point in object coordinates
    float3 fhp = ray.origin + intersectionDist * ray.direction;

    // Transform normals to world coordinates
    const float3 world_shading_normal = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, shading_normal) );
    const float3 world_geometric_normal = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, geometric_normal ) );

    //Calculating the face forward normal N in world coordinates
    const float3 N = faceforward( world_shading_normal, -ray.direction, world_geometric_normal );

    // Vector to camera
    const float3 wo = -ray.direction;

    // Transform front hit point to world coordinates
    const float3 fhp_world = rtTransformPoint(RT_OBJECT_TO_WORLD, fhp);

    prd_radiance.origin = fhp_world;

/////////////////////////////////////////////////////
//    for (int i; i<num_lights; i++) {
//      TODO: LICHTER HIER. FOR NOW DUMMY:
        optix::float3 light_pos = make_float3(0.0f) - fhp_world;
        float light_distance = length(light_pos - fhp_world);
        optix::float3 light_wi = normalize(light_pos - fhp_world);
        float light_area = 1.0f;
//    }
////////////////////////////////////////////////////////////

    PerRayData_shadow prdShadow;
    prdShadow.visible = true; // Initialize for miss.
    // Note that the sysSceneEpsilon is applied on both sides of the shadow ray [t_min, t_max] interval
    // to prevent self intersections with the actual light_definition geometry in the scene!
    optix::Ray shadow_ray = optix::make_Ray(prd_radiance.origin, light_wi, SHADOW_RAY_TYPE,
                                            scene_epsilon,
                                            light_distance - scene_epsilon);
    rtTrace(sysTopObject, shadow_ray, prdShadow); // Trace Shadow Ray

    if (prdShadow.visible) {
        const float cosAngIncidence = optix::clamp(optix::dot(N, light_wi), 0.0f, 1.0f);
        const float3 R = optix::normalize(2 * cosAngIncidence * N - light_wi);
        float phong_term = fmaxf(optix::dot(R, wo), 0.0f);
        phong_term = cosAngIncidence > DENOMINATOR_EPSILON ? phong_term : 0.0f;
        phong_term = powf(phong_term, specular_exponent);
        float3 f_phong_specular = make_float3(0.0f);
        if (cosAngIncidence > DENOMINATOR_EPSILON) { //Catch 0 division error
            f_phong_specular = optix::clamp(
                    (Ks * phong_term * M_PIf) / cosAngIncidence, make_float3(0.0f),
                    make_float3(1.0f)); //specular phong coefficient ks=material specific
        }
        const float3 f_phong_diffuse = Kd; //diffuse phong coefficient kd=material specific
        const float3 f_phong = f_phong_diffuse + f_phong_specular;// Do the visibility check of the light_definition sample.

        prd_radiance.radiance += f_phong * 1.0f /*light emission*/ * optix::dot(N, light_wi) * 1.0f /*solid angle*/;
    }
}
