/**
  @file     RT_object.cpp
  @author   Nils Melchert (nils.melchert@imr.uni-hannover.de)
  @since    2019 / 04 / 01
  @brief    generic scene object(s)

  All light sources or scene objects (lenses, etc...) should be derived from this class

  (c) 2010, Copyright Nils Melchert IMR Hannover
**/

#include "RT_object.h"

/**
  @brief    basic constructor
  @param    parent  parental object, or null

  Initializes members
  **/
RT_object::RT_object(optix::Context &context, RT_object *parent):
m_context(context)
{
    spdlog::info("Creating RT_object with memory address: {}", 1);
    m_parent = parent;

    m_transform = optix::Matrix4x4::identity();
    m_bTransformCacheUpToDate = false;

    m_strName.setNum(reinterpret_cast<size_t> (this), 16);
    m_bVisible = true;
}

/**
  @param    destructor

  Handles object detaching
  **/
RT_object::~RT_object() {
    if (m_parent) {
        //detach from parent, notify parent
    }
}

/**
  @brief    notify the object that it now belongs to an object group
  @param    group   pointer to the objectGroup that the object shall belong to
                    NULL to detach object
  @return   true if object is newly attached/detached
            false if object got reattached from one to another group
  **/
bool RT_object::setParent(RT_object *object) {
    spdlog::debug("Setting parent object for RT_object {0}", m_strName.toUtf8().constData());
    bool ret = (m_parent != NULL);
    m_parent = object;
    return ret;
}

/**
  @brief    return pointer to associated group
  @return   pointer
  **/
RT_object *RT_object::parent()
{
    return m_parent;
}

/**
  @brief    set object's material
  @param    name    another material to have the contents copied

  **/
void RT_object::setMaterial(const optix::Material &material) {
    spdlog::debug("Setting material for RT_object {0}", m_strName.toUtf8().constData());
    // TODO: Implement me. Should the Material created inside here or before and then be passen to the RT_object instance?
    // m_context->createMaterial();
}

/**
  @brief    get object's material
  @return   return object's material
  **/
const optix::Material *RT_object::material() const {
    return &m_material;
}

/**
  @brief    set object's name
  @param    name    object's desired name

  It could be useful to have a tellin-name for camera, e.g. "Pinhole f=100mm", or for manipulation
  **/
void RT_object::setName(const QString &str) {
    m_strName = QString(str);
}

/**
  @brief    get object's name
  @return   string contining the object name
  **/
QString RT_object::name() const {
    return m_strName;
}

/**
  @brief    hide or show object
  @param    vis -true: object is visible
                -false: object is hidden
  **/
void RT_object::setVisible(bool vis) {
    m_bVisible = vis;
}

/**
  @brief    state if object is hidden or showed
  @return   -true: object is visible
            -false: object is hidden
  **/
bool RT_object::isVisible() const {
    return m_bVisible;
}


/**
  @brief    determine if object's caches are up to date
  @return   content of m_bTransformCacheUpToDate
  **/
bool RT_object::upToDate() const {
    return m_bTransformCacheUpToDate;
}

/**
  @brief    reset object unrotated at center position

  Sets transformation matrix m_transform to identity matrix
  **/
void RT_object::reset()                             //move relative to current location
{
    m_transform = optix::Matrix4x4::identity();
    m_bTransformCacheUpToDate = false;
}

/**
  @brief    move object relative to current position in World coordinate system
  @param    v   translation vector

  world system: left multiply transformation matrix

  **/
void RT_object::translate(optix::float3 &v) {
    optix::Matrix4x4 trans = optix::Matrix4x4::translate(v);
    m_transform = trans * m_transform;
    m_bTransformCacheUpToDate = false;

}

/**
  @brief    move object relative to current position in world coordinate system
  @param    x   translation in x-direction
  @param    y   translation in y-direction
  @param    z   translation in z-direction

  **/
void RT_object::translate(float x, float y, float z) {
    optix::float3 translation = optix::make_float3(x, y, z);
    translate(translation);
}


/**
  @brief    move object relative to local object coordinate system
  @param    v   translation vector

  local system: right multiply transformation matrix

  **/
void RT_object::move(optix::float3 &v) {
    optix::Matrix4x4 trans = optix::Matrix4x4::translate(v);
    m_transform *= trans;
    m_bTransformCacheUpToDate = false;
}

/**
  @brief    move object relative to current position in local object coordinate system
  @param    x   translation in x-direction
  @param    y   translation in y-direction
  @param    z   translation in z-direction

  **/
void RT_object::move(float x, float y, float z) {
    optix::float3 translation = optix::make_float3(x, y, z);
    move(translation);
}


/**
  @brief    move object to absolute position in world coordinate system
  @param    x   translation in x-direction
  @param    y   translation in y-direction
  @param    z   translation in z-direction

  @todo verify function

  Replace the part of the matrix that matters ;)
  **/
void RT_object::setPosition(float x, float y, float z) {
    m_transform[3] = x;
    m_transform[7] = y;
    m_transform[11] = z;
    m_bTransformCacheUpToDate = false;
}

/**
  @brief    move object to absolute position
  @param    v   translation vector
  **/
void RT_object::setPosition(optix::float3 &p) {
    setPosition(p.x, p.y, p.z);
}

/**
  @brief    rotate object in local coordinate system relatively to its current coordinate system
  @param    rX    x-rotation
  @param    rY    y-rotation
  @param    rZ    z-rotation

  right multiply rotation matrix: means spin locally

  All angles are in degrees
  **/
void RT_object::spin(float rX, float rY, float rZ) {

    float yaw = rX * M_PIf / 180.0f;
    float pitch = rY * M_PIf / 180.0f;
    float roll = rZ * M_PIf / 180.0f;

    m_transform = m_transform * mhelpers::rotation(yaw, pitch, roll);
    m_bTransformCacheUpToDate = false;
}

/**
  @brief    rotate object in local coordinate system relatively to its current rotation
  @param    r   relative rotation vector

  All angles are in degrees
  **/
void RT_object::spin(optix::float3 &r) {
    spin(r.x, r.y, r.z);
}

/**
  @brief    rotate object in world coordinate system
  @param    rX    x-rotation
  @param    rY    y-rotation
  @param    rZ    z-rotation

  left multiply matrix;  rotate around world center
  All angles are in degrees
  **/
void RT_object::rotate(float rX, float rY, float rZ) {

    float yaw = rX * M_PIf / 180.0f;
    float pitch = rY * M_PIf / 180.0f;
    float roll = rZ * M_PIf / 180.0f;

    m_transform = mhelpers::rotation(yaw, pitch, roll) * m_transform;
    m_bTransformCacheUpToDate = false;
}

/**
  @brief    rotate object in world coordinate system
  @param    r    rotation vector

  All angles are in degrees
  **/
void RT_object::rotate(optix::float3 &r) {
    rotate(r.x, r.y, r.z);
}

/**
  @brief    transform object
  @param    mat     transformation (matrix) to apply

  Transformation matrix is left multiplied   X' = T X
  **/
void RT_object::transform(optix::Matrix4x4 &mat) {

    m_transform = mat * m_transform;
    m_bTransformCacheUpToDate = false;
}

/**
  @brief    replace objects transformation matrix with mat
  @param    mat new transformation matrix
  **/
void RT_object::setTransformationMatrix(const optix::Matrix4x4 &mat) {
    m_transform = mat;
    m_bTransformCacheUpToDate = false;
}

/**
  @brief    get a copy of the current transofrmation matrix
  @return   transfromation matrix with current values
  **/
optix::Matrix4x4 RT_object::transformationMatrix() {
    return optix::Matrix4x4();
}

/**
  @brief    read position
  @return   the t-part of the R|t - transformation matrix

  @bug bad idea, not reliable? verify!
  **/
const optix::float3 RT_object::position() const {
    return optix::make_float3(m_transform[3], m_transform[7], m_transform[11]);
}

/**
  @brief    parse parameters
  @param    action  string describing action to perform
  @param    params  action parameters
  @return   0 on success, negative on error, positive if action not found (use child class action)
  **/
int RT_object::parseActions(const QString &action, const QString &parameters) {

    return 0;
}
