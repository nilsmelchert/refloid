#ifndef NSLAIFT_RT_MATERIAL_H
#define NSLAIFT_RT_MATERIAL_H

#include <optix.h>
#include <optix_world.h>
#include <spdlog.h>
#include <RT_helper.h>


#include <QString>

class RT_material {
public:
    RT_material(optix::Context &context);
    ~RT_material();

    void setBRDF(QString &mat_type, QString &parameters);
    void setBRDF(QString mat_type);
    void setMaterialType(QString &mat_type, QString& parameters);
    void setMaterialType(QString mat_type);
    int parseActions(const QString &action, const QString &parameters, const QString &delimiter=";");

public:
    optix::Context& m_context;
    optix::Material m_material_optix;
    optix::float3 m_material_color = optix::make_float3(0.6, 0.6, 0.6); // Set default object color to gray
};


#endif //NSLAIFT_RT_MATERIAL_H
