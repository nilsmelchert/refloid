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

namespace rthelpers{
    int RT_parse_float3(const QString &str, float *x, float *y, float *z, const QString& delimiter = QString(","));
    int RT_parse_matrix(const QString &str, optix::Matrix4x4 *matconst, const QString& delimiter = QString(","));
}

#endif //NSLAIFT_RT_HELPER_H
