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
    m_parent = parent;
    m_ObjType = "";
    m_material = new RT_material(m_context);

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
  @brief    set object's name
  @param    name    object's desired name

  It could be useful to have a tellin-name for camera, e.g. "Pinhole f=100mm", or for manipulation
  **/
void RT_object::setName(const QString &str) {
    spdlog::debug("Setting name of object to {0}", str.toUtf8().constData());
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
    spdlog::debug("Setting visibility of RT_object {0} to {1}.", m_strName.toUtf8().constData(), vis ? "True":"False");
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
    spdlog::info("Resetting poision of RT_object {0}", m_strName.toUtf8().constData());
    m_transform = optix::Matrix4x4::identity();
    m_bTransformCacheUpToDate = false;
}

/**
  @brief    move object relative to current position in World coordinate system
  @param    v   translation vector

  world system: left multiply transformation matrix

  **/
void RT_object::translate(optix::float3 &v) {
    spdlog::debug("Translating RT_object {0} in its global world coordinate system by {1}, {2}, {3}", m_strName.toUtf8().constData(), v.x, v.y, v.z);
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
    spdlog::debug("Moving RT_object {0} in its local coordinate system by {1}, {2}, {3}", m_strName.toUtf8().constData(), v.x, v.y, v.z);
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
    spdlog::debug("Setting RT_object {0} to {1}, {2}, {3} in world coordinates", m_strName.toUtf8().constData(), x, y, z);
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
    spdlog::debug("Spinning RT_object {0} in its local coordinate system to {1}, {2}, {3}", m_strName.toUtf8().constData(), rX, rY, rZ);
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
    spdlog::debug("Spinning RT_object {0} in its global world coordinate system to {1}, {2}, {3}", m_strName.toUtf8().constData(), rX, rY, rZ);
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
    spdlog::debug("Performing transformation on RT_object {0}", m_strName.toUtf8().constData());

    m_transform = mat * m_transform;
    m_bTransformCacheUpToDate = false;
}

/**
  @brief    replace objects transformation matrix with mat
  @param    mat new transformation matrix
  **/
void RT_object::setTransformationMatrix(const optix::Matrix4x4 &mat) {
    spdlog::debug("Setting new tranformation matrix for RT_object {0}", m_strName.toUtf8().constData());
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

/*//////////////////////////////////
 Settings for material
*//////////////////////////////////

/**
  @brief    parse parameters
  @param    action  string describing action to perform
  @param    params  action parameters
  @return   0 on success, negative on error, positive if action not found (use child class action)
  **/
int RT_object::parseActions(const QString &action, const QString &parameters) {
    spdlog::debug("Parsing the following action to RT_object {0}: action={1}; parameters={2}", m_strName.toUtf8().constData(), action.toUtf8().constData(), parameters.toUtf8().constData());

    if(0 == action.compare("reset", Qt::CaseInsensitive)) {
        reset();
    } else  if(0 == action.compare("move", Qt::CaseInsensitive)) {
        float x,y,z;
        if (0 == rthelpers::RT_parse_float3(parameters,&x,&y,&z)) {
            move(x,y,z);
        } else {
            spdlog::debug("Error manipulating RT_object {0}, canot parse parameters {1} for action {2}", m_strName.toUtf8().constData(), parameters.toUtf8().constData(), action.toUtf8().constData());
            return -1;
        }
    } else if(0 == action.compare("translate", Qt::CaseInsensitive)) {
        float x,y,z;
        if (0 == rthelpers::RT_parse_float3(parameters,&x,&y,&z)) {
            translate(x,y,z);
        } else {
            spdlog::debug("Error manipulating RT_object {0}, canot parse parameters {1} for action {2}", m_strName.toUtf8().constData(), parameters.toUtf8().constData(), action.toUtf8().constData());
            return -1;
        }
    } else if(0 == action.compare("setPosition", Qt::CaseInsensitive)) {
        float x,y,z;
        if (0 == rthelpers::RT_parse_float3(parameters,&x,&y,&z)) {
            setPosition(x,y,z);
        } else {
            spdlog::debug("Error manipulating RT_object {0}, canot parse parameters {1} for action {2}", m_strName.toUtf8().constData(), parameters.toUtf8().constData(), action.toUtf8().constData());
            return -1;
        }
    } else if(0 == action.compare("spin", Qt::CaseInsensitive)) {
        float x,y,z;
        if (0 == rthelpers::RT_parse_float3(parameters,&x,&y,&z)) {
            spin(x,y,z);
        } else {
            spdlog::debug("Error manipulating RT_object {0}, canot parse parameters {1} for action {2}", m_strName.toUtf8().constData(), parameters.toUtf8().constData(), action.toUtf8().constData());
            return -1;
        }
    } else if(0 == action.compare("rotate", Qt::CaseInsensitive)) {
        float x, y, z;
        if (0 == rthelpers::RT_parse_float3(parameters, &x, &y, &z)) {
            rotate(x, y, z);
        } else {
            spdlog::debug("Error manipulating RT_object {0}, canot parse parameters {1} for action {2}", m_strName.toUtf8().constData(), parameters.toUtf8().constData(), action.toUtf8().constData());
            return -1;
        }
    } else if(0 == action.compare("setName", Qt::CaseInsensitive)) {
        if (!parameters.isEmpty()) {
            setName(parameters);
        } else {
            spdlog::debug("Error manipulating RT_object {0}, canot parse parameters {1} for action {2}", m_strName.toUtf8().constData(), parameters.toUtf8().constData(), action.toUtf8().constData());
            return -1;
        }
    } else if((0 == action.compare("setVisible", Qt::CaseInsensitive)) || (0 == action.compare("visible", Qt::CaseInsensitive))) {
        bool ok;
        bool visible = parameters.toInt(&ok);
        if (ok) {
            setVisible(visible);
        } else {
            spdlog::debug("Error manipulating RT_object {0}, canot parse parameters {1} for action {2}", m_strName.toUtf8().constData(), parameters.toUtf8().constData(), action.toUtf8().constData());
            return -1;
        }
    } else if(0 == action.compare("transform", Qt::CaseInsensitive)) {
        optix::Matrix4x4 mat;
        rthelpers::RT_parse_matrix(parameters, &mat);
        transform(mat);  //matrix dimension check is performed by this fn
    } else if(0 == action.compare("setTransformationMatrix", Qt::CaseInsensitive)) {
        optix::Matrix4x4 mat = optix::Matrix4x4::identity();
        rthelpers::RT_parse_matrix(parameters, &mat);
        setTransformationMatrix(mat);  //matrix dimension check is performed by this fn
    } else if (0 == action.compare("setMaterialType", Qt::CaseInsensitive) || 0 == action.compare("setBRDF", Qt::CaseInsensitive) || 0 == action.compare("materialType", Qt::CaseInsensitive) || 0 == action.compare("brdf", Qt::CaseInsensitive) || 0 == action.compare("setMaterial", Qt::CaseInsensitive) | 0 == action.compare("Material", Qt::CaseInsensitive)) {
        return m_material->parseActions(action, parameters);
    } else if (0 == action.compare("setMaterialParameter", Qt::CaseInsensitive) || 0 == action.compare("materialParameter", Qt::CaseInsensitive)) {
        // TODO
    }
    return 0;
}
