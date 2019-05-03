//
// Created by melchert on 30.04.19.
//

#ifndef NSLAIFT_RT_CAMERA_H
#define NSLAIFT_RT_CAMERA_H

#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_stream_namespace.h>
#include <optixu_math_namespace.h>

#include "RT_object.h"
#include "RT_helper.h"

class RT_camera : virtual public RT_object {
public:
    static const int TypeGeneric = 0;            ///<    General projective camera, using A, Rt, D
    static const int TypePinhole = 1;            ///<    projective camera at origin (Rt = 0) looking into Z-direction, only using m_K; note that m_transform (from ancestor RTobject) remains!

    static const int TypeInfinite = 10;           ///<    telecentric camera at origin looking into Z-direction, A is reinterpreted to hold metric pixel width instead of f, image centre at last column remains
    static const int TypeUnConfig = -1;          ///<     initial state: not yet configured

    int m_iType;                    ///< camera type

    unsigned int m_iCameraIdx;

    optix::Program m_ray_gen_pgrm;

/* intrinsics */
    //optix::Matrix4x4 m_transform;   ///< inherited from RTobject: the external transformation matrix (is Rt)
    optix::Matrix4x4 m_K;                ///<    <3 x 3> camera intrinsic-Matrix /f K = \begin{array}{ccc} f_x & 0 & c_x \\ 0 & f_y & cy \\ 0 & 0 & 1 \end{array}  /f which represents the intrinsics is called a camera matrix, or a matrix of intrinsic parameters. (cx, cy) is a principal point (that is usually at the image center), and fx, fy are the focal lengths expressed in pixel-related units.
    float m_distortion[5] = {0.0f, 0.0f, 0.0f, 0.0f,
                             0.0f};       ///<    distortion coefficients (k1,k2,p1,p2,k3) defined as in OpenCV: ideal coords --> observed coords
    float m_undistortion[5] = {0.0f, 0.0f, 0.0f, 0.0f,
                               0.0f};       ///<    distortion coefficients (k1,k2,p1,p2,k3) defined as in OpenCV: ideal coords --> observed coords

/* caching some parameters for speed */
    optix::Matrix4x4 m_K_inv;               ///<    inverse of m_K

    unsigned int m_iWidth;      ///< pixel count for rendering the exactly same image area
    unsigned int m_iHeight;     ///< pixel count for rendering the exactly same image area

public:
    RT_camera(optix::Context &context, RT_object *parent = nullptr);

    virtual ~RT_camera();

    virtual void setProjectionType(int type);

    virtual int projectionType();

    virtual int setSensorResolution(unsigned int iWidth, unsigned int iHeight);

    virtual int setIntrinsics(const optix::Matrix4x4 &mat);

    virtual int setExtrinsics(const optix::Matrix4x4 &Rt);

    virtual int setDistortion(const float dist[5]);

    virtual int setUndistortion();

    virtual int setUndistortion(const float undist[5]);

    virtual int parseActions(const QString &action, const QString &parameters);

    virtual int updateCache();

    virtual optix::float3 centerPosition();

    virtual optix::float3 principalAxis();
};


#endif //NSLAIFT_RT_CAMERA_H
