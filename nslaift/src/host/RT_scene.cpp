//
// Created by melchert on 30.04.19.
//

#include "RT_scene.h"
#include "RT_camera.h"

RT_scene::RT_scene() {
    m_context = optix::Context::create();
    setupContext(m_context);
    RT_camera* cam = new RT_camera(m_context);
    cam->setName("cam1");
    cam->setProjectionType(1);
    cam->updateCache();
//    m_context->validate();
}

RT_scene::~RT_scene() {

}

void RT_scene::setupContext(optix::Context &context) {
    // Set up context
    context = optix::Context::create();
    context->setRayTypeCount( 2 );
    context->setStackSize( 4640 );
    context->setMaxTraceDepth( 31 );

    // Note: high max depth for reflection and refraction through glass
    context["max_depth"]->setInt( 100 );
    context["scene_epsilon"]->setFloat( 1.e-4f );
    context["importance_cutoff"]->setFloat( 0.01f );
}

int RT_scene::clear(bool destroy) {
    return 0;
}

RT_object *RT_scene::createObject(const QString &name, const QString &objType, const QString &objParams) {
    return nullptr;
}

int RT_scene::manipulateObject(const QString &name, const QString &action, const QString &parameters) {
    return 0;
}

int RT_scene::manipulateObject(RT_object *object, const QString &action, const QString &parameters) {
    return 0;
}
