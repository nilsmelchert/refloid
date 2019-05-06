#include "RT_scene.h"
#include "RT_camera.h"

RT_scene::RT_scene()
{
    // Setting up the node graph following the optix conventions
    setupContext();
    setBackgroundColor(optix::make_float3(1.0f, 0.0f, 0.0f));
    initPrograms();
    initOutputBuffers();

    auto cam = new RT_camera(m_context);
    cam->setName("cam1");
    cam->setProjectionType(1);
    cam->updateCache();
    addCamera(cam);


    updateCaches();
    render();
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
    spdlog::debug("Setting rgb color: r:{0}, g:{1}, b:{2} as background color", m_colBackground.x, m_colBackground.y, m_colBackground.z);
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

RT_object *RT_scene::createObject(const QString &name, const QString &objType, const QString &objParams)
{
    return nullptr;
}

int RT_scene::manipulateObject(const QString &name, const QString &action, const QString &parameters)
{
    return 0;
}

int RT_scene::manipulateObject(RT_object *object, const QString &action, const QString &parameters)
{
    return 0;
}

int RT_scene::updateCaches(bool force)
{
    m_context->validate();
    spdlog::info("Context was successfully validated");
}

void RT_scene::setBackgroundColor(const optix::float3 &col)
{
    m_colBackground = col;
}

void RT_scene::render(int iterations)
{
    spdlog::info("Starting to render the entire scene");
    for (int cam_idx=0; cam_idx < m_cameras.size(); cam_idx++)
    {
        // Adjusting the size of the output buffer for the currently activated camera
        m_outputBuffer->setSize(m_cameras[cam_idx]->m_iWidth, m_cameras[cam_idx]->m_iHeight);
        m_accumBuffer->setSize(m_cameras[cam_idx]->m_iWidth, m_cameras[cam_idx]->m_iHeight);
        for (int iter=0; iter<iterations; iter++)
        {
            m_context->launch(cam_idx, m_cameras[cam_idx]->m_iWidth, m_cameras[cam_idx]->m_iHeight);
        }
    }
    spdlog::info("Rendering is done");
    std::vector<float> img_data;
    optix::Buffer output_buffer = m_context["sysOutputBuffer"]->getBuffer();
//    img_data = rthelpers::writeBufferToPipe(output_buffer);
}

optix::float3 RT_scene::backgroundColor()
{
    return m_colBackground;
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
  @return   pointer to the Camera or NULL if idx out of range
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


//    m_top_group = m_context->createGroup();
//    // Setting acceleration structure fot the top group
//    m_top_group->setAcceleration(m_context->createAcceleration("Trbvh"));
//    // TODO: ChildCount should not be hard coded. For now only one geometry is possible in the scene.
//    m_top_group->setChildCount(1);
//    // Setting Acceleration for geometry group
//    optix::GeometryGroup geom_group = m_context->createGeometryGroup();
//    geom_group->setAcceleration(m_context->createAcceleration("Trbvh"));
//    geom_group->setChildCount(1);
//    optix::GeometryInstance geom_inst = m_context->createGeometryInstance();
//    OptiXMesh mesh;
//    std::string file_name = "/home/melchert/Desktop/raytracing/refloid/res/data/turbineblade_sample.ply";
//    mesh.context = m_context;
//    std::string ptx_path_ipg(rthelpers::ptxPath("triangle_mesh.cu")); // ptx path ray generation program
//    mesh.intersection = m_context->createProgramFromPTXFile(ptx_path_ipg, "mesh_intersect_refine");
//    mesh.bounds = m_context->createProgramFromPTXFile(ptx_path_ipg, "mesh_bounds");
//    loadMesh(file_name, mesh);
//    geom_inst = mesh.geom_instance;
//    geom_inst->setMaterial(0, );
