#include "RT_scene.h"
#include "RT_camera.h"
#include "RT_sphere.h"
#include "zmq.hpp"

#include <cv.h>
#include <highgui.h>
#include <opencv2/imgproc/imgproc.hpp>

RT_scene::RT_scene()
{
    // Setting up the node graph following the optix conventions
    setupContext();
    initPrograms();
    initOutputBuffers();
}

RT_scene::~RT_scene()
{

}

void RT_scene::setupContext()
{
    spdlog::debug("Setting up the optix context");
    // Set up context
    m_context = optix::Context::create();
    // Primary ray and shadow ray are needed (0 is the promary ray and 1 the shadow ray)
    m_context->setRayTypeCount( 2 );

    long unsigned int stack_size = 4640;
    m_context->setStackSize( stack_size );
    spdlog::debug("Using a stack size of {}", stack_size );

    // Note: high max depth for reflection and refraction through glass
    m_context["max_depth"]->setInt( 100 );
    m_context["scene_epsilon"]->setFloat( 1.e-4f );
    m_context["radiance_ray_type"]->setUint(0u);
    m_context["shadow_ray_type"]->setUint(1u);

    // Creating a top level group - this is the scenes root group
    m_rootGroup = m_context->createGroup();
    m_rootGroup->setAcceleration(m_context->createAcceleration("Trbvh"));
    m_context["sysTopObject"]->set(m_rootGroup);
}

void RT_scene::initPrograms()
{
    spdlog::debug("Initializing miss program");
    std::string ptx_path_mpg(rthelpers::ptxPath("miss.cu")); // ptx path to miss program
    m_miss_program = m_context->createProgramFromPTXFile(ptx_path_mpg, "miss_environment_constant");
    m_miss_program["miss_color"]->setFloat(m_colBackground);
    m_context->setMissProgram(0, m_miss_program);
    // TODO: Define exception program here
}

void RT_scene::initOutputBuffers()
{
    spdlog::debug("Initializing global output buffer");
    // Initializing buffer to size of 100x100 pixels. This will be overridden when a camera is initialized
    m_outputBuffer = m_context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_UNSIGNED_BYTE4, 100, 100);
    m_context["sysOutputBuffer"]->set(m_outputBuffer);
    m_accumBuffer = m_context->createBuffer( RT_BUFFER_INPUT_OUTPUT | RT_BUFFER_GPU_LOCAL, RT_FORMAT_FLOAT4, 100, 100);
    m_context["sysAccumBuffer"]->set(m_accumBuffer);
}

int RT_scene::clear(bool destroy)
{
    return 0;
}

/**
  @brief  create a new object and add it to the scene
  @param  objName    unique object name
  @param  objType    type of object to create
  @param  objParams  creation-essential parameters; other parameters are to be set via "manipulateObject"
  @return  reference to the object, if creation was successfull; Null if there was some error preventing object creation.
  **/
RT_object *RT_scene::createObject(const QString &name, const QString &objType, const QString &objParams)
{
    if (name.isEmpty()){
        spdlog::error("Object not named");
        return nullptr;
    }
    if (findObject(name) != nullptr) {
        spdlog::error("Object with that name {0} already exists! Not rendering for this camera.", name.toUtf8().constData());
        return nullptr;
    }
    // camera objects
    if(0 == objType.compare("camera", Qt::CaseInsensitive)) {
        auto* cam = new RT_camera(m_context);
        if (!objParams.isEmpty()) {
            //TODO: implement parsing intrinsics and extrinsics
        } else {
            spdlog::debug("No object parameters were given for camera object: {}", cam->m_strName.toUtf8().constData());
        }
        cam->setName(name);
        addCamera(cam);
    } else if (0 == objType.compare("sphere", Qt::CaseInsensitive)) {
        auto* sphere = new RT_sphere(m_context, m_rootGroup);
        if (!objParams.isEmpty()) {
            // TODO: implement setting radius
        } else {
            spdlog::debug("No object parameters were given for sphere object: {}", sphere->m_strName.toUtf8().constData());
        }
        sphere->setName(name);
        addObject(sphere);
    }
    return nullptr;
}

RT_object *RT_scene::createObject(const QString &name, const QString &objType)
{
    return createObject(name, objType, "");
}

/**
  @brief    manipulate some object called by name
  @param    name        name of object
  @param    action      action to perform, "translate", "rotate", "setVisible", etc...
  @param    parameters  parameter string, e.g. "0, -45,0"
  @return   0 on success, error-code on failure
  **/
int RT_scene::manipulateObject(const QString &name, const QString &action, const QString &parameters)
{
    RT_object *object = findObject(name);
    if(object) {
        return manipulateObject(object, action, parameters);
    } else {
        spdlog::error("Object {1} not found", name.toUtf8().constData());
        return -1;
    }
}

/**
  @brief    manipulate some object (light, camera, object)
  @param    obj         pointer to object
  @param    action      action to perform, "translate", "rotate", "setVisible", etc...
  @param    parameters  parameter string, e.g. "0, -45,0"
  @return   0 on success, error-code on failure

  manipulators (action, parameters):

  All objects implement their own manipulators via "parseActions"
  **/
int RT_scene::manipulateObject(RT_object *object, const QString &action, const QString &parameters)
{
    if (nullptr == dynamic_cast<RT_object*>(object)) {
        spdlog::warn("Object {0} is not derived from RT_object", QString::number(reinterpret_cast<size_t>(object)).toUtf8().constData());
        return -1;
    }
    if (action.isEmpty()) {
        spdlog::warn("No action given");
        return -2;
    }
    int ret = object->parseActions(action, parameters);
    if(ret > 0) { //action not found
        spdlog::error("action {0} erroneous/not known, cannot manipulate object {1} (retcode: {2})", action.toUtf8().constData(), object->m_strName.toUtf8().constData(), ret);
        return -3;
    }
    return 0;
}

int RT_scene::updateCaches(bool force)
{
    // Updating background color in the miss program
    m_miss_program["miss_color"]->setFloat(m_colBackground);

    for (int cam_idx=0; cam_idx<m_cameras.size(); cam_idx++){
        m_cameras[cam_idx]->updateCache();
    }
    for (int obj_idx=0; obj_idx<m_objects.size(); obj_idx++){
        m_objects[obj_idx]->updateCache();
    }

    // Checking if everything was configured correctly in the optix context
    m_context->validate();
    spdlog::info("Context was successfully validated");
}

void RT_scene::setBackgroundColor(const optix::float3 &col)
{
    spdlog::debug("Setting background color to r:{}, g:{}, b:{}", col.x, col.y, col.z);
    m_colBackground = col;
}

void RT_scene::setBackgroundColor(float x, float y, float z)
{
    optix::float3 background = optix::make_float3(x, y, z);
    setBackgroundColor(background);
}

void RT_scene::render(int iterations)
{
    spdlog::info("Starting to render the entire scene");
    for (int cam_idx=0; cam_idx < m_cameras.size(); cam_idx++)
    {
        // Adjusting the size of the output buffer for the currently activated camera
        m_outputBuffer->setSize(m_cameras[cam_idx]->m_iWidth, m_cameras[cam_idx]->m_iHeight);
        m_accumBuffer->setSize(m_cameras[cam_idx]->m_iWidth, m_cameras[cam_idx]->m_iHeight);
        spdlog::debug("Rendering with {0} with a resolution of {1}x{2}", m_cameras[cam_idx]->m_strName.toUtf8().constData(), m_cameras[cam_idx]->m_iWidth,m_cameras[cam_idx]->m_iHeight);
        for (int iter=0; iter<iterations; iter++)
        {
            m_context->launch(cam_idx, m_cameras[cam_idx]->m_iWidth, m_cameras[cam_idx]->m_iHeight);
        }
        spdlog::info("Rendering with {0} with a resolution of {1}x{2} is DONE!", m_cameras[cam_idx]->m_strName.toUtf8().constData(), m_cameras[cam_idx]->m_iWidth,m_cameras[cam_idx]->m_iHeight);
        std::vector<unsigned char> img_data;
        optix::Buffer output_buffer = m_context["sysOutputBuffer"]->getBuffer();
        img_data = rthelpers::writeBufferToPipe(output_buffer);
        cv::Mat cv_img = cv::Mat(m_cameras[cam_idx]->m_iHeight, m_cameras[cam_idx]->m_iWidth, CV_8UC3);
        cv_img.data = img_data.data();
//        cv::imshow("rendered_image", cv_img);
//        cv::waitKey(1);
        QString img_path = "/tmp/render";
        img_path.append(QString::number(m_render_counter)).append(".png");
        cv::imwrite(img_path.toStdString().c_str(), cv_img);
        m_render_counter++;
    }

}

optix::float3 RT_scene::backgroundColor()
{
    return m_colBackground;
}

/**
  @return   find object (of any kind) within scene and return pointer to it
  @param    name    object name
  @return   pointer to object, or nullptr if object not found within scene
  **/
RT_object*   RT_scene::findObject(const QString& name) const
{
    int idx = 0;
    if ( (idx = cameraIndex(name)) >= 0) {
        return camera(idx);
    }
    if ( (idx = objectIndex(name)) >= 0) {
        return object(idx);
    }
    //TODO: IMPLEMENT
//    if ( (idx = lightSourceIndex(name)) >= 0) {
//        return lightSource(idx);
//    }
    return nullptr;
}

///////////////// begin: camera handlers ////////////////

/**
  @brief    add a camera to the scene
  @param    obj pointer to camera to add to scene
  @return   >= 0: object id (index in vector)
            < 0: error

   -cameras cannot be added twice (same pointer not allowed more than once)
   -two objcameraects cannot share the exactly same name(!); empty names will be auto-filled by address-to-textstring
 **/
int RT_scene::addCamera(RT_camera *cam)
{
    if (nullptr != dynamic_cast<RT_camera*> (cam)) {
        if (cam->m_strName.isEmpty()) {                     //no object name, use pointer address as an object name
            cam->m_strName = QString::number( reinterpret_cast<size_t> (cam), 16);
        }
        if (cameraIndex(cam->name()) >= 0) {           //if object already known by name
            return -1;
        }
        if (cameraIndex(cam) >= 0) {             //object already added, return its index
            return cameraIndex(cam);
        }

        m_cameras.push_back(cam);                   //it's really a new one; add its
        if (m_cameras.size() == 1)                  //the first added camera will automatically be the active camera
            m_activeCamera = cam;

        return m_cameras.size() -1 ;
    } else {
        return -1;
    }
}



/**
  @brief    determine camera count
  @return   number of camera in vector m_cameras
  **/
int RT_scene::countCameras() const
{
    return m_cameras.size();
}

/**
  @brief    get pointer to Camera from scene
  @param    idx index of Camera
  @return   pointer to the Camera or nullptr if idx out of range
  **/
RT_camera*   RT_scene::camera(int idx) const
{
    if (idx < 0)
        return nullptr;
    if (idx >= m_cameras.size())
        return nullptr;
    return m_cameras[idx];
}

/**
  @brief    get pointer to active camera
  @return   pointer to active camera
  **/
RT_camera* RT_scene::activeCamera() const
{
    return m_activeCamera;
}

/**
  @brief    set some existant camera to be the active camera
  @param    idx index of the camera we wish to set the active one
  @return   >= 0: index of the new active camera
            < 0: error

  if idx is invalid then the m_activeCamera remains unchanged
  **/
int       RT_scene::setActiveCamera(int idx)
{
    RT_camera* cam = camera(idx);
    if (cam) {
        m_activeCamera = cam;
        return idx;
    } else {
        return -1;
    }
}

/**
  @brief    get camera index from camera name
  @param    name    name of camera to look for
  @return   >= 0: index of camera
            <0: named camera not found
  **/
int RT_scene::cameraIndex(const QString& name) const
{
    for(int i = 0; i < m_cameras.size(); i++) {
        if( 0 == name.compare( m_cameras.at(i)->m_strName, Qt::CaseInsensitive )) {
            return i;
        }
    }
    return -1;
}

/**
  @brief    get object index from camera memory location
  @param    pObject   pointer to camera
  @return   >= 0: index of camera
            <0:  camera not in this list
  **/
int RT_scene::cameraIndex(const RT_camera* pCam) const
{
    for(int i = 0; i < m_cameras.size(); i++) {
        if( (m_cameras.at(i)) == pCam ) {
            return i;
        }
    }
    return -1;
}

/**
  @brief    get name of camera
  @param    idx camera index
  @return   the camera's name
  **/
QString RT_scene::cameraName(int idx) const
{
    if (idx < 0)
        return QString();
    if (idx >= m_cameras.size())
        return QString();
    return  m_cameras.at(idx)->name();
}
///////////////// end: camera handlers ////////////////

///////////////// begin: object handlers ////////////////

/**
  @brief    add a RT_object to the scene, object can also be RT_objectGroup
  @param    obj pointer to object to add to scene
  @return   >= 0: object id (index in vector)
            < 0: error

   -objects cannot be added twice (same pointer not allowed more than once)
   -two objects cannot share the exactly same name(!); empty names will be auto-filled by address-to-textstring
 **/
int RT_scene::addObject(RT_object *obj)
{
    if (nullptr != dynamic_cast<RT_object*> (obj)) {
        if (obj->m_strName.isEmpty()) {               //no object name, use pointer address as an object name
            obj->m_strName = QString::number( reinterpret_cast<size_t> (obj), 16);
        }
        if (objectIndex(obj->name()) >= 0) {     //if object already known by name
            return -1;
        }
        if (objectIndex(obj) >= 0) {             //object already added, return its index
            return objectIndex(obj);
        }

        m_objects.push_back(obj);                   //it's really a new one; add its
        return m_objects.size() - 1;
    } else {
        return -1;
    }
}

/**
  @brief    determine object count
  @return   number of objects in vector m_objects

  RT_objectGroups contained in m_objects count as one element

  **/
int RT_scene::countObjects() const
{
    return m_objects.size();
}

/**
  @brief    get pointer to object from scene
  @param    idx index of object
  @return   pointer to the object or nullptr if idx out of range
  **/
RT_object*   RT_scene::object(int idx) const
{
    if (idx < 0)
        return nullptr;
    if (idx >= m_objects.size())
        return nullptr;
    return m_objects[idx];
}

/**
  @brief    get object index from object name
  @param    name    name of object to look for
  @return   >= 0: index of object
            <0: named object not found
  **/
int RT_scene::objectIndex(const QString& name) const
{
    for(int i = 0; i < m_objects.size(); i++) {
        if( 0 == name.compare( m_objects.at(i)->m_strName, Qt::CaseInsensitive )) {
            return i;
        }
    }
    return -1;
}

/**
  @brief    get object index from object memory location
  @param    pObject   pointer to object
  @return   >= 0: index of object
            <0:  object not in this list
  **/
int RT_scene::objectIndex(const RT_object* pObject) const
{
    for(int i = 0; i < m_objects.size(); i++) {
        if( (m_objects.at(i)) == pObject ) {
            return i;
        }
    }
    return -1;
}

/**
  @brief    get name of object
  @param    idx object index
  @return   the object's name
  **/
QString RT_scene::objectName(int idx) const
{
    if (idx < 0)
        return QString();
    if (idx >= m_objects.size())
        return QString();
    return  m_objects.at(idx)->name();
}
