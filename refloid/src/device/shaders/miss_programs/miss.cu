/*
   @file    miss.cu
  @author   Pascal Kern   (kern@imr.uni-hannover.de)
  @since    2018 / 1 / 8
  @brief    Coloring Background with Miss Color set in main
*/

#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optix_cuda.h>

#include "includes/per_ray_data_gpu.h"

rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );
rtDeclareVariable(float3, miss_color, , );
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );

RT_PROGRAM void miss_environment_constant()
{
    prd_radiance.is_miss = true;
    prd_radiance.radiance = miss_color; // Background Color set in Host (black)
}
