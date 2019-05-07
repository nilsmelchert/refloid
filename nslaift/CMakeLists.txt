# See top level CMakeLists.txt file for documentation of OPTIX_add_sample_executable.
OPTIX_add_sample_executable( nslaift
        main.cpp

        src/device/cameras/pinhole_cam.cu
        src/device/shaders/miss.cu
        src/device/helpers_gpu.h
        src/device/rt_function.h
        src/device/per_ray_data_gpu.h
        src/device/random_number_generators_gpu.h
        src/device/app_config.h

        src/host/RT_matrixHelpers.h
        src/host/RT_matrixHelpers.cpp
        src/host/RT_helper.h
        src/host/RT_helper.cpp
        src/host/RT_camera.h
        src/host/RT_camera.cpp
        src/host/RT_scene.h
        src/host/RT_scene.cpp
        src/host/RT_object.h
        src/host/RT_object.cpp
        src/host/RT_sphere.h
        src/host/RT_sphere.cpp
  )


