#ifndef NSLAIFT_RT_SCENE_H
#define NSLAIFT_RT_SCENE_H

#include "RT_object.h"
#include "RT_camera.h"
#include "RT_sphere.h"

#include <zmq.hpp>
#include <tiff.h>
#include <tiffio.h>

#include <sutil.h>
#include <OptiXMesh.h>
#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_stream_namespace.h>
#include <optixu_math_namespace.h>
#include <QString>
#include <QVector>
#include <spdlog/spdlog.h>

class RT_scene
{
public:
    RT_scene();
    ~RT_scene();

public:
    RT_camera*                     m_activeCamera;          ///<   camera through which it is rendered (=active camera)
    // delete all content in the scene
    int clear(bool destroy = true);
    RT_object* createObject(const QString &name, const QString &objType, const QString &objParams);
    RT_object* createObject(const QString &name, const QString &objType);
    int deleteObject(const QString &name);

    ///< for dynamic interaction! enable the expression manipulate("sphere1", "translate", "43,2,-5");
    int manipulateObject(const QString &name,const QString &action,const QString &parameters);
    int manipulateObject(RT_object *object, const QString &action, const QString &parameters);

    int addCamera(RT_camera *cam);
    int countCameras() const;
    RT_camera* camera(int idx) const;
    RT_camera* activeCamera() const;
    int       setActiveCamera(int idx);
    int       cameraIndex(const RT_camera* pCamera) const;
    int       cameraIndex(const QString& name) const;
    QString   cameraName(int idx) const;

    int         addObject(RT_object *obj);     // return id
    int         removeObject(int idx);
    int         countObjects() const;
    RT_object*   object(int idx) const;
    int         objectIndex(const QString& name) const;
    int         objectIndex(const RT_object* pObject) const;
    QString     objectName(int idx) const;


//    int         addLightSource(RT_lightSource *l);
//    int         countLightSources() const;
//    int         removeLightSource(int idx);
//    RT_lightSource* lightSource(int idx) const;
//    int         lightSourceIndex(const QString& name) const;
//    int         lightSourceIndex(const RT_lightSource* pSource) const;
//    QString     lightSourceName(int idx) const;
//
    RT_object*   findObject(const QString& name) const;
//    int         deleteObject(const QString& name);
//
    void setBackgroundColor(const optix::float3 &col);
    void setBackgroundColor(float x, float y, float z);
    optix::float3 backgroundColor();
//
//    bool    checkScene();
    int     updateCaches(bool force = false);
//
//public:
    bool                          m_bSceneOk;        ///<   is scene ok, set up properly? can we render???

//    RT_camera*                     m_activeCamera;          ///<   camera through which it is rendered (=active camera)
    optix::float3                       m_colBackground = {0.0f, 0.0f, 0.0f};   ///<   specify default background color for scene

    QVector< RT_camera* >            m_cameras;         ///<   list of all scene cameras
    QVector< RT_object* >            m_objects;         ///<   list of all scene objects may also have RT_objectGroup as entries
//    QVector< RT_lightSource* >       m_lights;          ///<   list of all light sources within scene
public:
    void render(int iterations=1);
    optix::Group m_rootGroup;

private:
    optix::Context m_context;
    optix::Group m_top_group;
    void setupContext();
    void initPrograms();
    void initOutputBuffers();

    optix::Program m_miss_program;
    optix::Buffer m_outputBuffer;
    optix::Buffer m_accumBuffer;

    unsigned int m_render_counter=0;
};

#endif //NSLAIFT_RT_SCENE_H
