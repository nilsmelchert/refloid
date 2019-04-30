/**
  @file     RT_object.h
  @author   Nils Melchert (nils.melchert@imr.uni-hannover.de)
  @since    2019 / 04 / 01
  @brief    generic scene object(s)

  All light sources or scene objects (lenses, etc...) should be derived from this class

  (c) 2010, Copyright Nils Melchert IMR Hannover
**/
#ifndef NSLAIFT_RTOBJECT_H
#define NSLAIFT_RTOBJECT_H

#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_stream_namespace.h>
#include <optixu_math_namespace.h>
#include <QString>
#include "spdlog/spdlog.h"
#include "RT_matrixHelpers.h"

/**
  @brief    abstract base class for scene object

  Scene objects can be lenses, geometric objects, etc... even object groups
**/
class RT_object {
public:
    ///< transformation matrix of object
    optix::Matrix4x4 m_transform;
    ///< state if m_polyDataTransformed must be recalculated
    bool m_bTransformCacheUpToDate;

    ///< readable name
    QString m_strName;
    ///< object material/color
    optix::Material m_material;

    ///< shall object be visible? (VTK and rendering)
    bool m_bVisible;
    ///< =NULL    pointer to the occupying objectGroup
    RT_object *m_parent;

public:
    // Object memory management
    RT_object(optix::Context &context, RT_object *parent = NULL);
    virtual ~RT_object();

    virtual bool setParent(RT_object *object);
    virtual RT_object *parent();


    virtual void    setMaterial(const optix::Material &material);
    virtual const optix::Material* material() const;

    virtual void      setName(const QString &str);
    virtual QString   name() const;

    virtual void setVisible(bool vis = true);
    virtual bool isVisible() const;

    virtual int updateCache() = 0;                              //pure virtual function --> prevent base class init
    virtual int parseActions(const QString& action, const QString& parameters);
    virtual bool upToDate() const;

    virtual void reset();      //reset transformations to initial state (non-rotated at center)

    virtual void translate(float x, float y, float z);            //move relative in world
    virtual void translate(optix::float3 &v);                             //move relative in world
    virtual void move(float x, float y, float z);                 //move relative to current location in local object system
    virtual void move(optix::float3 &v);                                  //move relative to current location in local object system
    virtual void setPosition(float x, float y, float z);          //move absolute to location in world
    virtual void setPosition(optix::float3 &p);                           //move absolute to location in world

    virtual void spin(float rX, float rY, float rZ);              //rotate in local coordinate system
    virtual void spin(optix::float3 &r);                                  //rotate in local coordinate system
    virtual void rotate(float rX, float rY, float rZ);            //rotate in global coordinate system
    virtual void rotate(optix::float3 &r);                                //rotate in global coordinate system

    virtual void transform(optix::Matrix4x4 &mat);                    //right multiply transformation matrix
    virtual void setTransformationMatrix(const optix::Matrix4x4 &mat);
    virtual optix::Matrix4x4 transformationMatrix();                  //get transformation matrix

    virtual const optix::float3 position() const;

private:
    optix::Context& m_context;
};

#endif //NSLAIFT_RTOBJECT_H
