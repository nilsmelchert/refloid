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
    QString test = "bla";
    std::cout << test.compare("bla") << std::endl;
    return 0;
}
