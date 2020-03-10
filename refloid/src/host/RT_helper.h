/**
  @file     RThelper.h
  @author   Nils Melchert   (nils.melchert@imr.uni-hannover.de)
  @since    2019 / 04 / 19
  @brief    helper functions for raytracer lib


  (c) 2010, Copyright Nils Melchert IMR Hannover
**/

#ifndef NSLAIFT_RT_HELPER_H
#define NSLAIFT_RT_HELPER_H

#include <QString>
#include <QStringList>
#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_stream_namespace.h>
#include <optixu_math_namespace.h>
#include "sutil.h"
#include "spdlog.h"

// Defines for getting the cmake environment variables in c++
#ifdef OPTIX_BIN_PATH
#define BIN_DIR OPTIX_BIN_PATH
#else
#define BIN_DIR NULL
#endif
#ifdef OPTIX_BASE_PATH
#define BASE_DIR OPTIX_BASE_PATH
#else
#define BASE_DIR NULL
#endif

// Defines for ray types considering optix convention
#define RADIANCE_RAY_TYPE 0
#define SHADOW_RAY_TYPE 1

extern const char *const SAMPLE_NAME;

namespace rthelpers{
    int RT_parse_float3(const QString &str, float *x, float *y, float *z, const QString& delimiter = QString(","));
    int RT_parse_matrix(const QString &str, optix::Matrix4x4 *matconst, const QString& delimiter = QString(","));
    std::string ptxPath(const std::string &cuda_file);
    std::string printMat4x4(optix::Matrix4x4 &mat);
    std::vector<unsigned char> writeBufferToPipe(optix::Buffer buffer);
    std::vector<unsigned char> writeBufferToPipe(RTbuffer buffer);
    int RT_parse2double(const QString &str, double *x, double *y, const QString &delimiter /*= QString(",")*/);
    int RT_parse2int(const QString &str, int *x, int *y, const QString &delimiter /*= QString(",")*/);
}

#endif //NSLAIFT_RT_HELPER_H
