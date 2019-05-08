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

    Scene->createObject("cam1", "camera");

    Scene->createObject("cam2", "camera");
    Scene->manipulateObject("cam2", "translate", "-1.0, 0.0, 0.0");
    Scene->manipulateObject("cam2", "spin", "0.0, 45.0, 0.0");

    Scene->createObject("cam3", "camera");
    Scene->manipulateObject("cam3", "translate", "0.0, -20.0, 0.0");
    Scene->manipulateObject("cam3", "spin", "-90.0, 0.0, 0.0");

    Scene->createObject("sphere1", "sphere");
    Scene->manipulateObject("sphere1", "translate", "0.0, 0.0, 1.0");

    Scene->createObject("sphere2", "sphere");
    Scene->manipulateObject("sphere2", "translate", "0.0, 0.15, 1.0");

    Scene->createObject("sphere3", "sphere");
    Scene->manipulateObject("sphere3", "translate", "0.0, 1.5, 6.0");
    Scene->manipulateObject("sphere3", "radius", "4.0");

    Scene->updateCaches();
    Scene->render(100);
    return 0;
}
