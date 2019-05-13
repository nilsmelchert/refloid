#pragma once

#include <optixu/optixu_vector_types.h>
#include <optix_math.h>

// Note that the fields are ordered by CUDA alignment restrictions.
struct PerRayData_radiance {

    int depth; //Bounces
    unsigned int seed;

    // shading state
    optix::float3 reflectance; //attenuation
    optix::float3 radiance; // Radiance along the current path segment.
    optix::float3 origin; //Current surface hit point, in world space
    optix::float3 direction;

    bool is_miss;
    bool visible;
};

struct PerRayData_shadow{
    bool visible;
};

// Currently only containing some vertex attributes in world coordinates.
struct State
{
    optix::float3 geoNormal;
    optix::float3 normal;
    optix::float3 texcoord;
};

