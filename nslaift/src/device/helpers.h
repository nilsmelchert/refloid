/*
   @file    helpers.h
  @author   Pascal Kern   (kern@imr.uni-hannover.de)
  @since    2018 / 28 / 6
  @brief    Useful functions for shading
*/

#ifndef RAYTRACING_PASCAL_HELPERS_H
#define RAYTRACING_PASCAL_HELPERS_H

#pragma once

#include "rt_function.h"
#include <optix_math.h>

RT_FUNCTION optix::uchar4 make_color(const optix::float3& c) {
    return optix::make_uchar4(static_cast<unsigned char>(__saturatef(c.z) * 255.99f),  /* B */ //saturatef cuda function
                              static_cast<unsigned char>(__saturatef(c.y) * 255.99f),  /* G */ //see cuda api as reference
                              static_cast<unsigned char>(__saturatef(c.x) * 255.99f),  /* R */
                              255u);                                                 /* A */
}

RT_FUNCTION float3 schlick(float nDi, const float3 &rgb) {
    float r = fresnel_schlick(nDi, 5, rgb.x, 1);
    float g = fresnel_schlick(nDi, 5, rgb.y, 1);
    float b = fresnel_schlick(nDi, 5, rgb.z, 1);
    return make_float3(r, g, b);
}

RT_FUNCTION optix::float3 logf(const optix::float3& v)
{
    return optix::make_float3(::logf(v.x), ::logf(v.y), ::logf(v.z));
}

RT_FUNCTION optix::float2 floorf(const optix::float2& v)
{
    return optix::make_float2(::floorf(v.x), ::floorf(v.y));
}

RT_FUNCTION optix::float3 floorf(const optix::float3& v)
{
    return optix::make_float3(::floorf(v.x), ::floorf(v.y), ::floorf(v.z));
}

RT_FUNCTION optix::float3 ceilf(const optix::float3& v)
{
    return optix::make_float3(::ceilf(v.x), ::ceilf(v.y), ::ceilf(v.z));
}

RT_FUNCTION optix::float3 powf(const optix::float3& v, const float e)
{
    return optix::make_float3(::powf(v.x, e), ::powf(v.y, e), ::powf(v.z, e));
}

RT_FUNCTION optix::float4 powf(const optix::float4& v, const float e)
{
    return optix::make_float4(::powf(v.x, e), ::powf(v.y, e), ::powf(v.z, e), ::powf(v.w, e));
}


RT_FUNCTION optix::float2 fminf(const optix::float2& v, const float m)
{
    return optix::make_float2(::fminf(v.x, m), ::fminf(v.y, m));
}
RT_FUNCTION optix::float3 fminf(const optix::float3& v, const float m)
{
    return optix::make_float3(::fminf(v.x, m), ::fminf(v.y, m), ::fminf(v.z, m));
}
RT_FUNCTION optix::float4 fminf(const optix::float4& v, const float m)
{
    return optix::make_float4(::fminf(v.x, m), ::fminf(v.y, m), ::fminf(v.z, m), ::fminf(v.w, m));
}

RT_FUNCTION optix::float2 fminf(const float m, const optix::float2& v)
{
    return optix::make_float2(::fminf(m, v.x), ::fminf(m, v.y));
}
RT_FUNCTION optix::float3 fminf(const float m, const optix::float3& v)
{
    return optix::make_float3(::fminf(m, v.x), ::fminf(m, v.y), ::fminf(m, v.z));
}
RT_FUNCTION optix::float4 fminf(const float m, const optix::float4& v)
{
    return optix::make_float4(::fminf(m, v.x), ::fminf(m, v.y), ::fminf(m, v.z), ::fminf(m, v.w));
}


RT_FUNCTION optix::float2 fmaxf(const optix::float2& v, const float m)
{
    return optix::make_float2(::fmaxf(v.x, m), ::fmaxf(v.y, m));
}
RT_FUNCTION optix::float3 fmaxf(const optix::float3& v, const float m)
{
    return optix::make_float3(::fmaxf(v.x, m), ::fmaxf(v.y, m), ::fmaxf(v.z, m));
}
RT_FUNCTION optix::float4 fmaxf(const optix::float4& v, const float m)
{
    return optix::make_float4(::fmaxf(v.x, m), ::fmaxf(v.y, m), ::fmaxf(v.z, m), ::fmaxf(v.w, m));
}

RT_FUNCTION optix::float2 fmaxf(const float m, const optix::float2& v)
{
    return optix::make_float2(::fmaxf(m, v.x), ::fmaxf(m, v.y));
}
RT_FUNCTION optix::float3 fmaxf(const float m, const optix::float3& v)
{
    return optix::make_float3(::fmaxf(m, v.x), ::fmaxf(m, v.y), ::fmaxf(m, v.z));
}
RT_FUNCTION optix::float4 fmaxf(const float m, const optix::float4& v)
{
    return optix::make_float4(::fmaxf(m, v.x), ::fmaxf(m, v.y), ::fmaxf(m, v.z), ::fmaxf(m, v.w));
}

RT_FUNCTION bool isNull(const optix::float3& v)
{
    return (v.x == 0.0f && v.y == 0.0f && v.z == 0.0f);
}

RT_FUNCTION bool isNotNull(const optix::float3& v)
{
    return (v.x != 0.0f || v.y != 0.0f || v.z != 0.0f);
}

RT_FUNCTION float3 UniformSampleSphere(float2 const& seed){
    float z = 1.0f - 2.0f * seed.x;
    float r = sqrtf(fmaxf(0.0f, 1.0f - z * z));
    float phi = 2.0f * M_PIf * seed.y;

    return make_float3(r * cosf(phi), r * sinf(phi), z);
}

#endif //RAYTRACING_PASCAL_HELPERS_H
