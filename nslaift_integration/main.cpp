#include <iostream>
#include <QCoreApplication>
#include "spdlog/spdlog.h"

#include "src/host/RT_object.h"

#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_stream_namespace.h>
#include <optixu_math_namespace.h>
int main(){
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Starting raytracing test application");
//    optix::Matrix4x4 trans = optix::Matrix4x4::identity();
//    trans = trans.translate(optix::make_float3(1,2,3));
    optix::Matrix4x4 trans = optix::Matrix4x4::translate(optix::make_float3(1,2,3));
    trans *= optix::Matrix4x4::translate(optix::make_float3(1,2,3));
    std::cout << trans[3] << std::endl;
    std::cout << trans[7] << std::endl;
    std::cout << trans[11] << std::endl;

    return 0;
}
