//
// Created by melchert on 30.04.19.
//

#include "RT_camera.h"

/**
  @brief    constructor initializes member variables
  **/
RT_camera::RT_camera(optix::Context &context, RT_object *parent) :
        RT_object(context, parent) {

    m_iCameraIdx = m_context->getEntryPointCount();
    m_context->setEntryPointCount(m_iCameraIdx + 1);
    spdlog::debug("Creating camera object with index {0}", m_iCameraIdx);

    m_iType = TypeUnConfig;

    m_iWidth = 1024;
    m_iHeight = 768;

    m_K = optix::Matrix4x4::identity();
    m_K_inv = optix::Matrix4x4::identity();

    m_transform = optix::Matrix4x4::identity();
}

/**
  @brief    destructor
  **/
RT_camera::~RT_camera() {
}

/**
  @brief    modify m_iType
  @param    type    camera type (see class constants)

  initially type is invalid (TypeUnConfig) and some type must be set before camera can be used
  **/
void RT_camera::setProjectionType(int type) {
    spdlog::debug("Setting projection type for camera {0}", m_strName.toUtf8().constData());
    m_iType = type;
    m_bTransformCacheUpToDate = false;
}

/**
  @brief    get camera type setting
  @return   m_iType
  **/
int RT_camera::projectionType() {
    return m_iType;
}

/**
  @brief    describe metric image sensor size
  @param    iWidth      sensor width in pixel count
  @param    iHeight     sensor height in pixel count
  @param    dPixelSize  metric pixel size (in millimeters) of (square) pixel
  @return   returns zero on success, non-zero on error

  **/
int RT_camera::setSensorResolution(int iWidth, int iHeight) {
    spdlog::debug("Setting resolution for camera {0}", m_strName.toUtf8().constData());
    if (iWidth <= 0 || iHeight <= 0) {
        spdlog::error("Invalid (non-positive) parameters given");
        return -1;
    }
    m_iWidth = iWidth;
    m_iHeight = iHeight;
    m_bTransformCacheUpToDate = false;
    return 0;
}

/**
  @brief    set camera intrinsic matrix
  @param    mat     new intrinsic matrix
  @return   0 on success non-zero on invalid mat
  **/
int RT_camera::setIntrinsics(const optix::Matrix4x4 &mat) {
    spdlog::debug("Setting intrinsics for camera {0}", m_strName.toUtf8().constData());
    m_K = mat;
    m_bTransformCacheUpToDate = false;
    return 0;
}

/**
  @brief    set camera extrinsic matrix
  @param    Rt     new extrinsic matrix (3x4 or 4x4)
  @return   0 on success non-zero on invalid mat
  **/
int RT_camera::setExtrinsics(const optix::Matrix4x4 &Rt) {
    spdlog::debug("Setting extrinsics for camera {0}", m_strName.toUtf8().constData());
    setTransformationMatrix(Rt);
    return 0;
}

/**
  @brief    set camera distortion coefficients
  @param    dist     coefficients (5-vector) k1,k2,p1,p2,k3 (as in OpenCV)
  @return   returns 0 on success
  **/
int RT_camera::setDistortion(const float dist[5]) {
    spdlog::debug("Setting distortion coefficients for camera {0}", m_strName.toUtf8().constData());
    memcpy(m_distortion, dist, sizeof(float) * 5);
    m_bTransformCacheUpToDate = false;
    return 0;
}

/**
  @brief    calculate the undistortion coefficients from the distortion coefficients
  @return   returns 0 on success
  **/
int RT_camera::setUndistortion() {
    spdlog::debug("Setting undistortion coefficients for camera {0}", m_strName.toUtf8().constData());
    for (int i = 0; i < 5; i++) {
        m_undistortion[i] = -m_distortion[i];
    }
    m_bTransformCacheUpToDate = false;
    return 0;
}

/**
  @brief    set camera undistortion coefficients
  @param    dunist     coefficients (5-vector) k1,k2,p1,p2,k3 (as in OpenCV)
  @return   returns 0 on success
  **/
int RT_camera::setUndistortion(const float undist[5]) {
    spdlog::debug("Setting undistortion coefficients for camera {0}", m_strName.toUtf8().constData());
    memcpy(m_undistortion, undist, sizeof(float) * 5);
    m_bTransformCacheUpToDate = false;
    return 0;
}

/**
  @brief    update cached matrices that are used for raytracing
  @return   0 on success, non-zero on error
  @attention    do never forget to call this function after changing any camera parameter!
  **/
int RT_camera::updateCache() {
    spdlog::debug("Updating the cache for camera object {0}", m_strName.toUtf8().constData());
    if (m_bTransformCacheUpToDate) {
        return 0;
    }
    if (m_iType == TypePinhole)
    {
        std::string ptx_path_rgp(rthelpers::ptxPath("pinhole_cam.cu")); // ptx path ray generation program
        std::cout << ptx_path_rgp << std::endl;
        m_ray_gen_pgrm = m_context->createProgramFromPTXFile(ptx_path_rgp, "camera");
        m_ray_gen_pgrm["Rt"]->setMatrix4x4fv(false, m_transform.getData());
        m_ray_gen_pgrm["Rt_inv"]->setMatrix4x4fv(false, m_transform.inverse().getData());
        m_ray_gen_pgrm["K"]->setMatrix4x4fv(false, m_K.getData());
        m_ray_gen_pgrm["K_inv"]->setMatrix4x4fv(false, m_K.inverse().getData());
        m_ray_gen_pgrm["dist"]->setUserData(sizeof(float)*5, m_distortion);
        m_ray_gen_pgrm["undist"]->setUserData(sizeof(float)*5, m_undistortion);

        m_context->setRayGenerationProgram(m_iCameraIdx, m_ray_gen_pgrm);
    } else {
        spdlog::error("The camera {0} is currently not supported.", m_iType);
    }
}

/**
  @param    get camera center position
  @return   camera centre in world frame

  origin of the principalAxis() -ray is returned (cached variable)
  **/
optix::float3 RT_camera::centerPosition() {
    return optix::make_float3(m_transform[3], m_transform[7], m_transform[11]);
}

/**
  @brief    get camera's principal axis
  @return   line starting at centre through principal axis

 Principle axis is actually the z-axis
  **/
optix::float3 RT_camera::principalAxis() {
    return optix::make_float3(m_transform[2], m_transform[6], m_transform[10]);
}

int RT_camera::parseActions(const QString &action, const QString &parameters) {
    spdlog::debug("Parsing parameter {0} for action {1} on camera object {2}", parameters.toUtf8().constData(),
                  action.toUtf8().constData(), m_strName.toUtf8().constData());
    return 0;
}
