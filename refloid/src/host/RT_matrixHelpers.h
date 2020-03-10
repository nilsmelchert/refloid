//
// Created by melchert on 30.04.19.
//

#ifndef NSLAIFT_RT_MATRIXHELPERS_H
#define NSLAIFT_RT_MATRIXHELPERS_H

#include <iostream>
#include <cmath>
#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_stream_namespace.h>
#include <optixu_math_namespace.h>

namespace mhelpers {
    //create 4x4 homogeneous rotation matrix with yaw pitch and roll set to parameters (in rad)
    optix::Matrix4x4 rotation(float yaw, float pitch, float roll);
}
#endif
