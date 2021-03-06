#include <optix_world.h>

#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optix_cuda.h>
#include <valarray>
#include <optix_world.h>
#include "includes/helpers_gpu.h"
#include "includes/per_ray_data_gpu.h"
#include "includes/random_number_generators_gpu.h"

using namespace optix;

rtDeclareVariable(float,  radius, , );
rtDeclareVariable(optix::Matrix4x4, Rt, , );

rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, ); 
rtDeclareVariable(float3, shading_normal, attribute shading_normal, ); 
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );

template<bool use_robust_method>
static __device__
void intersect_sphere(void)
{
  float3 center = make_float3(Rt[3], Rt[7], Rt[11]);
  float3 O = ray.origin - center;
  float  l = 1 / length(ray.direction);
  float3 D = ray.direction * l;

  float b = dot(O, D);
  float c = dot(O, O)-radius*radius;
  float disc = b*b-c;
  if(disc > 0.0f){
    float sdisc = sqrtf(disc);
    float root1 = (-b - sdisc);

    bool do_refine = false;

    float root11 = 0.0f;

    if(use_robust_method && fabsf(root1) > 10.f * radius) {
      do_refine = true;
    }

    if(do_refine) {
      // refine root1
      float3 O1 = O + root1 * D;
      b = dot(O1, D);
      c = dot(O1, O1) - radius*radius;
      disc = b*b - c;

      if(disc > 0.0f) {
        sdisc = sqrtf(disc);
        root11 = (-b - sdisc);
      }
    }

    bool check_second = true;
    if( rtPotentialIntersection( (root1 + root11) * l ) ) {
      shading_normal = geometric_normal = (O + (root1 + root11)*D)/radius;
      if(rtReportIntersection(0))
        check_second = false;
    } 
    if(check_second) {
      float root2 = (-b + sdisc) + (do_refine ? root1 : 0);
      if( rtPotentialIntersection( root2 * l ) ) {
        shading_normal = geometric_normal = (O + root2*D)/radius;
        rtReportIntersection(0);
      }
    }
  }
}


RT_PROGRAM void intersect(int primIdx)
{
  intersect_sphere<false>();
}


RT_PROGRAM void robust_intersect(int primIdx)
{
  intersect_sphere<true>();
}


RT_PROGRAM void bounds (int, float result[6])
{
  const float3 cen = make_float3( Rt[3], Rt[7], Rt[11] );
  const float3 rad = make_float3( radius );

  optix::Aabb* aabb = (optix::Aabb*)result;
  
  if( rad.x > 0.0f  && !isinf(rad.x) ) {
    aabb->m_min = cen - rad;
    aabb->m_max = cen + rad;
  } else {
    aabb->invalidate();
  }
}
