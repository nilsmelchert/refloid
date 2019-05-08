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
    spdlog::debug("Creating camera object with entry point index {0}", m_iCameraIdx);

    std::string ptx_path_rgp(rthelpers::ptxPath("pinhole_cam.cu")); // ptx path ray generation program
    m_ray_gen_pgrm = m_context->createProgramFromPTXFile(ptx_path_rgp, "camera");
    spdlog::debug("Setting ray generation program for entry point index {}", m_iCameraIdx);
    //TODO: MAYBE THIS NEEDS TO BE UPDATED IN updateCache()
    m_context->setRayGenerationProgram(m_iCameraIdx, m_ray_gen_pgrm);

    m_iType = TypePinhole;

    // Setting default camera parameters
    spdlog::debug("Setting default camera parameters for cam with entry point index {0}", m_iCameraIdx);
    m_iWidth = 1280;
    m_iHeight = 960;
    const float K_default[16] = {1136.0f, 0.0f      , 640.0f    , 0.0f,
                                0.0f    , 1136.0f   , 480.0f    , 0.0f,
                                0.0f    , 0.0f      , 1.0f      , 0.0f,
                                0.0f    , 0.0f      , 0.0f      , 1.0f};
    m_K = optix::Matrix4x4(K_default);
    m_K_inv = m_K.inverse();

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
int RT_camera::setResolution(unsigned int iWidth, unsigned int iHeight) {
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
        m_ray_gen_pgrm["width"]->setUint(m_iWidth);
        m_ray_gen_pgrm["height"]->setUint(m_iHeight);
        m_ray_gen_pgrm["Rt"]->setMatrix4x4fv(false, m_transform.getData());
        m_ray_gen_pgrm["Rt_inv"]->setMatrix4x4fv(false, m_transform.inverse().getData());
        m_ray_gen_pgrm["K"]->setMatrix4x4fv(false, m_K.getData());
        m_ray_gen_pgrm["K_inv"]->setMatrix4x4fv(false, m_K.inverse().getData());
        m_ray_gen_pgrm["entry_point_id"]->setUint(m_iCameraIdx);

        optix::Buffer distBuff = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT, 5);
        // Copy data into OptiX buffer
        void* dist = distBuff->map(0, RT_BUFFER_MAP_WRITE_DISCARD);
        memcpy(dist, m_distortion, sizeof(float)*5);
        distBuff->unmap();
        m_ray_gen_pgrm["distBuff"]->setBuffer(distBuff);
        optix::Buffer undistBuff = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT, 5);
        // Copy data into OptiX buffer
        void* undist = undistBuff->map(0, RT_BUFFER_MAP_WRITE_DISCARD);
        memcpy(undist, m_undistortion, sizeof(float)*5);
        undistBuff->unmap();
        m_ray_gen_pgrm["undistBuff"]->setBuffer(undistBuff);
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
  @return   line starting at center through principal axis

 Principle axis is actually the z-axis
  **/
optix::float3 RT_camera::principalAxis() {
    return optix::make_float3(m_transform[2], m_transform[6], m_transform[10]);
}

/**
  @brief    parse parameters
  @param    action  string describing action to perform
  @param    params  action parameters
  @return   0 on success, negative on error, positive if action not found (use child class action)

  first all "local" actions are looked up, if none found then base class RTobject::parseActions() is called
  **/
int RT_camera::parseActions(const QString &action, const QString &parameters) {
    spdlog::debug("Parsing parameter {0} for action {1} on camera object {2}", parameters.toUtf8().constData(),
                  action.toUtf8().constData(), m_strName.toUtf8().constData());
    if((0 == action.compare("setResolution", Qt::CaseInsensitive)) || (0 == action.compare("resolution", Qt::CaseInsensitive)))
    {
        int width, height;
        if (0 == rthelpers::RT_parse2int(parameters, &width, &height, "x"))
        {
            setResolution(width, height);
        }
    } else {
        int ret = RT_object::parseActions(action, parameters);
        return ret;
    }
    return 0;
}
