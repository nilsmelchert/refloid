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

private:
    QString m_mat_type = "phong"; ///< Default material is set to rendering phong
    optix::float3 m_color = optix::make_float3(0.6f);
    optix::float3 m_Kd = optix::make_float3(0.4f, 0.4f, 0.4f);
    optix::float3 m_Ks = optix::make_float3(0.2f, 0.2f, 0.2f);
    float m_spec_exp = 2;
};


#endif //NSLAIFT_RT_MATERIAL_H
