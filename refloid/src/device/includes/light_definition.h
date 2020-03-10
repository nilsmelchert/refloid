#ifndef NSLAIFT_LIGHT_DEFINITION_H
#define NSLAIFT_LIGHT_DEFINITION_H

#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>

enum Lighttype
{
    LIGHT_POINTLIGHT = 0, //Constant PointLight
    LIGHT_PARALLELOGRAM = 1, //Rectangle Light
    LIGHT_PROJECTOR = 2
};

struct LightDefinition
{
    Lighttype type;// Point, rectangle (parallelogram), projector
    // Rectangle lights are defined in world coordinates as footpoint and two vectors spanning a parallelogram.
    // All in world coordinates with no scaling.
#if defined(__cplusplus)
    typedef optix::float3 float3;
#endif

    // Only for parallelogram
    float3 vecU;
    float3 vecV;
    float3 normal;
    float area;


    float3 emission;
    float3 wi; // Direction from point to light
    float solid_angle;
    float distance;
};

#endif //NSLAIFT_LIGHT_DEFINITION_H
