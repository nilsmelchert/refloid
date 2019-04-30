#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optix_cuda.h>
#include <valarray>
#include <optix_world.h>
//#include <zconf.h>
#include "helpers.h"
#include "per_ray_data.h"
#include "random_number_generators.h"
#include "app_config.h"

using namespace optix;

rtDeclareVariable(unsigned int, entry_point_id, ,);
rtDeclareVariable(int, parCameraIndex,,);

rtDeclareVariable(float, scene_epsilon, ,);
rtDeclareVariable(float3, cutoff_color, ,);
rtDeclareVariable(int, max_depth, ,);


rtBuffer<rtBufferId<uchar4, 2>, 1> output_id_buffer;
rtBuffer<rtBufferId<float4, 2>, 1> output_id_accum_buffer;
rtBuffer<rtBufferId<float, 2>, 1> output_id_intensity_buffer;

rtDeclareVariable(rtObject, top_object, ,);
rtDeclareVariable(unsigned int, frame, ,);
rtDeclareVariable(uint2, launch_index, rtLaunchIndex,);

rtDeclareVariable(unsigned int, radiance_ray_type, ,);

RT_FUNCTION void distort_pixels(float2 &uv, camera_information &camera_parameters)
{
    float X, Y, Z, r2, u_, v_;
    X = uv.x * camera_parameters.K_inv[0] + uv.y * 0.0f + camera_parameters.K_inv[2];
    Y = uv.x * 0.0f + uv.y * camera_parameters.K_inv[5] + camera_parameters.K_inv[6];
    Z = uv.x * 0.0f + uv.y * 0.0f + 1.0f;
    X /= Z; Y /= Z;
    //radial^2
    r2 = X*X + Y*Y;
    u_ = X * (1.0f + camera_parameters.distortion[0]*r2 + camera_parameters.distortion[1]*(r2*r2) + camera_parameters.distortion[4]*(r2*r2*r2)) + 2.0f*camera_parameters.distortion[2]*(X*Y) + camera_parameters.distortion[3]*(r2 + 2.0f*(X*X));
    v_ = Y * (1.0f + camera_parameters.distortion[0]*r2 + camera_parameters.distortion[1]*(r2*r2) + camera_parameters.distortion[4]*(r2*r2*r2)) + camera_parameters.distortion[2]*(r2 + 2.0f*(Y*Y)) + 2.0f*camera_parameters.distortion[3]*(X*Y);

    X = u_ * camera_parameters.K[0] + v_ * 0.0f + camera_parameters.K[2];
    Y = u_ * 0.0f + v_ * camera_parameters.K[5] + camera_parameters.K[6];
    Z = u_ * 0.0f + v_ * 0.0f + 1.0f;
    //de-homogenize
    uv.x = X / Z;
    uv.y = Y / Z;
}

RT_PROGRAM void camera() {

    optix::size_t2 screen = output_id_buffer[entry_point_id].size();

    uint2 launch_index_cv = launch_index;
    camera_information camera_parameters = sysCameraParameters[parCameraIndex];

    launch_index_cv.y = (camera_parameters.height - 1) - launch_index_cv.y;
    // Provides a random number between -veryhighnumber and +veryhighnumber
    unsigned int seed = tea<16>(screen.x * launch_index_cv.y + launch_index_cv.x, frame);
    //Subpixel jitter: send the ray through a different position inside the pixel each time,
    // to provide antialiasing.
    // Random number generator (the value is between 0 and 1
    float2 subpixel_jitter = frame == 0 ? make_float2(0.0f) : make_float2(rng(seed) - 0.5f, rng(seed) - 0.5f);

    // d is pixel for a casted ray
    float2 d = (make_float2(launch_index_cv) + subpixel_jitter);// / make_float2(screen) * 2.f - 1.f;

    distort_pixels(d, camera_parameters);

    float3 ray_direction = normalize(
            make_float3((1.0f / camera_parameters.K[0]) * (d.x - camera_parameters.K[2]), (1.0f / camera_parameters.K[5]) * (d.y - camera_parameters.K[6]),
                        1.0f));

    // Apply extrinsic transformation
    ray_direction = optix::make_matrix3x3(camera_parameters.Rt) * ray_direction;
    float3 ray_origin = make_float3(camera_parameters.Rt[3], camera_parameters.Rt[7], camera_parameters.Rt[11]);

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
    rtTrace(top_object, ray, prd);
#if USE_DEBUG_EXCEPTIONS
    // DAR DEBUG Highlight numerical errors.
    if (isnan(prd.radiance.x) || isnan(prd.radiance.y) || isnan(prd.radiance.z))
      {
        prd.radiance = make_float3(1000000.0f, 0.0f, 0.0f); // super red
      }
      else if (isinf(prd.radiance.x) || isinf(prd.radiance.y) || isinf(prd.radiance.z))
      {
        prd.radiance = make_float3(0.0f, 1000000.0f, 0.0f); // super green
      }
      else if (prd.radiance.x < 0.0f || prd.radiance.y < 0.0f || prd.radiance.z < 0.0f)
      {
        prd.radiance = make_float3(0.0f, 0.0f, 1000000.0f); // super blue
      }
#else
    // NaN values will never go away. Filter them out before they can arrive in the output buffer.
    // This only has an effect if the debug coloring above is off!
    if (!(isnan(prd.radiance.x) || isnan(prd.radiance.y) || isnan(prd.radiance.z)))
#endif
    {
        float4 acc_val = output_id_accum_buffer[entry_point_id][launch_index];
        if (frame > 0) {
#if Visualization_Transparency_OFF
            acc_val = lerp(acc_val, make_float4(prd.radiance, 0.f), 1.0f / static_cast<float>( frame + 1 ));
#endif
#if Multireflection_Visualization
        if (prd.depth > 0 && prd.visible && prd.radiance.x > 0.001f && prd.radiance.y > 0.001f && prd.radiance.z > 0.001f)
        {
        acc_val = lerp(acc_val, make_float4(1.0f, 0.0f, 0.0f, 0.f), 1.0f / static_cast<float>( frame + 1 ));
        }
        if (prd.depth > 1 && prd.visible && prd.radiance.x > 0.001f && prd.radiance.y > 0.001f && prd.radiance.z > 0.001f)
        {
        acc_val = lerp(acc_val, make_float4(0.7f, 0.5f, 0.0f, 0.f), 1.0f / static_cast<float>( frame + 1 ));
        }
        if (prd.depth > 2 && prd.visible && prd.radiance.x > 0.001f && prd.radiance.y > 0.001f && prd.radiance.z > 0.001f)
        {
        acc_val = lerp(acc_val, make_float4(0.0f, 1.0f, 0.0f, 0.f), 1.0f / static_cast<float>( frame + 1 ));
        }
        if (prd.depth > 3 && prd.visible && prd.radiance.x > 0.001f && prd.radiance.y > 0.001f && prd.radiance.z > 0.001f)
        {
            acc_val = lerp(acc_val, make_float4(0.0f, 0.0f, 1.0f, 0.f), 1.0f / static_cast<float>( frame + 1 ));
        }
#endif
        }else {
            acc_val = make_float4(prd.radiance, 0.f);
        }

        output_id_buffer[entry_point_id][launch_index] = make_color(make_float3(acc_val));
        output_id_accum_buffer[entry_point_id][launch_index] = acc_val;
        if (frame == 0)
        {
            output_id_intensity_buffer[entry_point_id][launch_index] = 0.0f;
        }
        else if(prd.visible && prd.radiance.x >0.001f && prd.radiance.y > 0.001f && prd.radiance.z > 0.001f){
            output_id_intensity_buffer[entry_point_id][launch_index] = prd.intensity;
        }
        // Change here for a gamma corrected RGB picture
//        output_id_buffer[entry_point_id][launch_index] = make_color(make_float3(powf(acc_val.x, 1.0f / 2.2f), powf(acc_val.y, 1.0f / 2.2f) , powf(acc_val.z, 1.0f / 2.2f)));
    }
}
// Entry point program
