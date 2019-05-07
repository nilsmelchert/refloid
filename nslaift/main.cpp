#include <iostream>
#include <QCoreApplication>
#include "spdlog/spdlog.h"

#include "src/host/RT_object.h"
#include "src/host/RT_scene.h"
#include "src/host/RT_camera.h"

#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_stream_namespace.h>
#include <optixu_math_namespace.h>


const char *const SAMPLE_NAME = "nslaift";

int main(){
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Starting raytracing test application");
    auto Scene = new RT_scene();

    Scene->setBackgroundColor(0.7f, 0.7f, 0.7f);

//    Scene->createObject("cam1", "camera");
//    Scene->manipulateObject("cam1", "resolution", "400x400");


    Scene->updateCaches();
    Scene->render();
    return 0;
}
