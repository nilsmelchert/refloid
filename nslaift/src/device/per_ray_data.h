/*
   @file    per_ray_data.h
  @author   Pascal Kern   (kern@imr.uni-hannover.de)
  @since    2018 / 28 / 6
  @brief    Ray data used in most shading files
*/

#pragma once

#include <optixu/optixu_vector_types.h>
#include <optix_math.h>

// Note that the fields are ordered by CUDA alignment restrictions.

struct PerRayData_radiance {
#if defined(__cplusplus)
    typedef optix::float3 float3;
#endif

    float intensity; // For Calculating the Overall Intensity of Multireflections
    int depth; //Bounces
    unsigned int seed;

    // shading state
    float3 reflectance; //attenuation
    float3 radiance; // Radiance along the current path segment.
    float3 origin; //Current surface hit point, in world space
    float3 direction;

    bool is_miss;
    bool visible;
};

struct PerRayData_shadow{
    bool visible;
};

