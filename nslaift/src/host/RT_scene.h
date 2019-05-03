//
// Created by melchert on 30.04.19.
//

#ifndef NSLAIFT_RT_SCENE_H
#define NSLAIFT_RT_SCENE_H

#include "RT_object.h"

#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_stream_namespace.h>
#include <optixu_math_namespace.h>
#include <QString>
#include "spdlog/spdlog.h"

class RT_scene
{
public:
    RT_scene();
    ~RT_scene();

public:
    // delete all content in the scene
    int clear(bool destroy = true);
    RT_object* createObject(const QString &name, const QString &objType, const QString &objParams);

    ///< for dynamic interaction! enable the expression manipulate("sphere1", "translate", "43,2,-5");
    int manipulateObject(const QString &name,const QString &action,const QString &parameters);
    int manipulateObject(RT_object *object, const QString &action, const QString &parameters);

//    int addCamera(RT_camera *cam);
//    int countCameras() const;
//    int removeCamera(int idx);
//    RT_camera* camera(int idx) const;
//    RT_camera* activeCamera() const;
//    int       setActiveCamera(int idx);
//    int       cameraIndex(const RT_camera* pCamera) const;
//    int       cameraIndex(const QString& name) const;
//    QString   cameraName(int idx) const;
//
//    int         addObject(RT_object *obj);     // return id
//    int         countObjects() const;
//    int         removeObject(int idx);
//    RT_object*   object(int idx) const;
//    int         objectIndex(const QString& name) const;
//    int         objectIndex(const RT_object* pObject) const;
//    QString     objectName(int idx) const;
//
//
//    int         addLightSource(RT_lightSource *l);
//    int         countLightSources() const;
//    int         removeLightSource(int idx);
//    RT_lightSource* lightSource(int idx) const;
//    int         lightSourceIndex(const QString& name) const;
//    int         lightSourceIndex(const RT_lightSource* pSource) const;
//    QString     lightSourceName(int idx) const;
//
//    RT_object*   findObject(const QString& name) const;
//    int         deleteObject(const QString& name);
//
//    void setBackgroundColor(const RT_color &col);
//    RT_color backgroundColor();
//
//    bool    checkScene();
//    int     updateCaches(bool force = false);
//
//public:
    bool                          m_bSceneOk;        ///<   is scene ok, set up properly? can we render???

//    RT_camera*                     m_activeCamera;          ///<   camera through which it is rendered (=active camera)
//    RT_color                       m_colBackground;   ///<   specify background color for scene
//
//    bool                          m_bGlobalShadows;  ///<   globally enable / disable shadowing
//    int                           m_iMirrorDepth;   ///<   recursion depth of mirroring, defaults to 0 (no reflections at all)
//
//    QVector< RT_camera* >            m_cameras;         ///<   list of all scene cameras
//    QVector< RT_object* >            m_objects;         ///<   list of all scene objects may also have RT_objectGroup as entries
//    QVector< RT_lightSource* >       m_lights;          ///<   list of all light sources within scene
private:
    optix::Context m_context;
    void setupContext(optix::Context &context);
};

#endif //NSLAIFT_RT_SCENE_H
