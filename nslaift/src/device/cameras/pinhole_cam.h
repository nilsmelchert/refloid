#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optix_cuda.h>
#include <valarray>
#include <optix_world.h>

#include "includes/helpers_gpu.h"
#include "includes/per_ray_data_gpu.h"
#include "includes/random_number_generators_gpu.h"


RT_FUNCTION void distort_pixels(float2 &uv);

RT_FUNCTION optix::float3 calculate_ray_direction(float2 &uv);

RT_FUNCTION PerRayData_radiance init_per_ray_data();
