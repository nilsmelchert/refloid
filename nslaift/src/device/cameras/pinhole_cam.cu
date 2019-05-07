#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optix_cuda.h>
#include <valarray>
#include <optix_world.h>

#include "includes/helpers_gpu.h"
#include "includes/per_ray_data_gpu.h"
#include "includes/random_number_generators_gpu.h"

rtDeclareVariable(unsigned int, entry_point_id, ,);
rtDeclareVariable(int, parCameraIndex,,);

// Camera Parameters
rtDeclareVariable(unsigned int, width, ,);
rtDeclareVariable(unsigned int, height, ,);
rtDeclareVariable(optix::Matrix4x4, K, ,);
rtDeclareVariable(optix::Matrix4x4, K_inv, ,);
rtDeclareVariable(optix::Matrix4x4, Rt, ,);
rtDeclareVariable(optix::Matrix4x4, Rt_inv, ,);
rtBuffer<float>distBuff;
rtBuffer<float>undistBuff;


rtDeclareVariable(float, scene_epsilon, ,);
rtDeclareVariable(float3, cutoff_color, ,);
rtDeclareVariable(int, max_depth, ,);


rtBuffer<uchar4, 2> sysOutputBuffer; // RGB32F
rtBuffer<float4, 2> sysAccumBuffer; // RGB32F

rtDeclareVariable(rtObject, sysTopObject, ,);
rtDeclareVariable(unsigned int, frame, ,);
rtDeclareVariable(uint2, launch_index, rtLaunchIndex,);

rtDeclareVariable(unsigned int, radiance_ray_type, ,);

RT_FUNCTION void distort_pixels(float2 &uv);

RT_FUNCTION optix::float3 calculate_ray_direction(float2 &uv);

RT_PROGRAM void camera() {
    optix::size_t2 screen = sysOutputBuffer.size();

    // Convert pixel so that it follows the OpenCV convention
    uint2 launch_index_cv = launch_index;
    launch_index_cv.y = (height - 1) - launch_index_cv.y;

    // Provides a random number between -veryhighnumber and +veryhighnumber
    unsigned int seed = tea<16>(screen.x * launch_index_cv.y + launch_index_cv.x, frame);
    //Subpixel jitter: send the ray through a different position inside the pixel each time,
    // to provide antialiasing.
    // Random number generator (the value is between 0 and 1
    float2 subpixel_jitter = frame == 0 ? make_float2(0.0f) : make_float2(rng(seed) - 0.5f, rng(seed) - 0.5f);

    // d is pixel for a casted ray
    float2 d = (make_float2(launch_index_cv) + subpixel_jitter);// / make_float2(screen) * 2.f - 1.f;

    distort_pixels(d);

    optix::float3 ray_direction = calculate_ray_direction(d);

    // Apply extrinsic transformation
    ray_direction = optix::make_matrix3x3(Rt) * ray_direction;
    float3 ray_origin = make_float3(Rt[3], Rt[7], Rt[11]);

    PerRayData_radiance prd;
    prd.depth = 0; // Initialize Bounces
    prd.seed = seed; // Random generated Number (Sample) for spatial antialiasing
    prd.intensity = 0.0f; //Initialize Multireflection overall Intensity for Calculations

    // These represent the current shading state and will be set by the closest-hit or miss program
    // attenuation (<= 1) from surface interaction.
    prd.reflectance = make_float3(1.0f);
    // light from a light source or miss program
    prd.radiance = make_float3(0.0f);
    // next ray to be traced
    prd.origin = make_float3(0.0f);
    prd.direction = make_float3(0.0f);
    optix::Ray ray(ray_origin, ray_direction, radiance_ray_type, scene_epsilon, RT_DEFAULT_MAX);
    rtTrace(sysTopObject, ray, prd);

    // NaN values will never go away. Filter them out before they can arrive in the output buffer.
    // This only has an effect if the debug coloring above is off!
    if (!(isnan(prd.radiance.x) || isnan(prd.radiance.y) || isnan(prd.radiance.z)))
    {
        float4 acc_val = sysAccumBuffer[launch_index];
        if (frame > 0) {
            acc_val = lerp( acc_val, make_float4( prd.radiance, 0.f ), 1.0f / static_cast<float>( frame+1 ) );
        }else {
            acc_val = optix::make_float4(prd.radiance, 1.0f);
        }
        sysOutputBuffer[launch_index] = make_color( optix::make_float3(acc_val));

        sysAccumBuffer[launch_index] = acc_val;
        // Change here for a gamma corrected RGB picture
    }
}

RT_FUNCTION void distort_pixels(float2 &uv)
{
    float X, Y, Z, r2, u_, v_;
    X = uv.x * K_inv[0] + uv.y * 0.0f + K_inv[2];
    Y = uv.x * 0.0f + uv.y * K_inv[5] + K_inv[6];
    Z = uv.x * 0.0f + uv.y * 0.0f + 1.0f;
    X /= Z; Y /= Z;
    //radial^2
    r2 = X*X + Y*Y;
    u_ = X * (1.0f + distBuff[0]*r2 + distBuff[1]*(r2*r2) + distBuff[4]*(r2*r2*r2)) + 2.0f*distBuff[2]*(X*Y) + distBuff[3]*(r2 + 2.0f*(X*X));
    v_ = Y * (1.0f + distBuff[0]*r2 + distBuff[1]*(r2*r2) + distBuff[4]*(r2*r2*r2)) + distBuff[2]*(r2 + 2.0f*(Y*Y)) + 2.0f*distBuff[3]*(X*Y);

    X = u_ * K[0] + v_ * 0.0f + K[2];
    Y = u_ * 0.0f + v_ * K[5] + K[6];
    Z = u_ * 0.0f + v_ * 0.0f + 1.0f;
    //de-homogenize
    uv.x = X / Z;
    uv.y = Y / Z;
}

RT_FUNCTION optix::float3 calculate_ray_direction(float2 &uv)
{
    return normalize(make_float3((1.0f / K[0]) * (uv.x - K[2]), (1.0f / K[5]) * (uv.y - K[6]), 1.0f));
}
