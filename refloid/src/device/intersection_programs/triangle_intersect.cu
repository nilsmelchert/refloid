#include "includes/app_config.h"
#include "includes/vertex_attributes.h"

#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_aabb_namespace.h>
#include <optix_cuda.h>

rtBuffer<VertexAttributes> attributesBuffer;
rtBuffer<uint3>            indicesBuffer;

// Attributes.
rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, );
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );
rtDeclareVariable(optix::float3, tangent,   attribute tangent, );
rtDeclareVariable(optix::float3, tex_coord,  attribute tex_coord, );
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );

// Intersection routine for indexed interleaved triangle data.
RT_PROGRAM void intersect(int primitiveIndex)
{
    const uint3 indices = indicesBuffer[primitiveIndex];

    VertexAttributes const& a0 = attributesBuffer[indices.x];
    VertexAttributes const& a1 = attributesBuffer[indices.y];
    VertexAttributes const& a2 = attributesBuffer[indices.z];

    const float3 v0 = a0.vertex;
    const float3 v1 = a1.vertex;
    const float3 v2 = a2.vertex;

    float3 n;
    float  t;
    float  beta;
    float  gamma;

    if (intersect_triangle(ray, v0, v1, v2, n, t, beta, gamma))
    {
        if (rtPotentialIntersection(t))
        {
            // Barycentric interpolation:
            const float alpha = 1.0f - beta - gamma;

            // Note: No normalization on the TBN attributes here for performance reasons.
            //       It's done after the transformation into world space anyway.
            geometric_normal      = n;
            tangent        = a0.tangent  * alpha + a1.tangent  * beta + a2.tangent  * gamma;
            shading_normal         = a0.normal   * alpha + a1.normal   * beta + a2.normal   * gamma;
            tex_coord       = a0.texcoord * alpha + a1.texcoord * beta + a2.texcoord * gamma;

            rtReportIntersection(0);
        }
    }
}
// Axis Aligned Bounding Box routine for indexed interleaved triangle data.
RT_PROGRAM void bounds(int primitiveIndex, float result[6])
{
    const uint3 indices = indicesBuffer[primitiveIndex];

    const float3 v0 = attributesBuffer[indices.x].vertex;
    const float3 v1 = attributesBuffer[indices.y].vertex;
    const float3 v2 = attributesBuffer[indices.z].vertex;

    const float area = optix::length(optix::cross(v1 - v0, v2 - v0));

    optix::Aabb *aabb = (optix::Aabb *) result;

    if (0.0f < area && !isinf(area))
    {
        aabb->m_min = fminf(fminf(v0, v1), v2);
        aabb->m_max = fmaxf(fmaxf(v0, v1), v2);
    }
    else
    {
        aabb->invalidate();
    }
}
