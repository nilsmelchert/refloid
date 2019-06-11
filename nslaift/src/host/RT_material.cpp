#include "RT_material.h"

RT_material::RT_material(optix::Context &context) :
m_context(context)
{
    m_material_optix = m_context->createMaterial();

    setMaterialType(m_mat_type);
}

RT_material::~RT_material() {
    m_material_optix->destroy();
}

void RT_material::setMaterialType(QString &mat_type, QString &parameters) {
    m_mat_type = mat_type;
    std::string ptx_mat_path;
    optix::Program ch_pgrm;
    optix::Program ah_pgrm;

    if (0 == mat_type.compare("normal", Qt::CaseInsensitive)) {
        ptx_mat_path = rthelpers::ptxPath(mat_type.append(".cu").toStdString());
        ch_pgrm = m_context->createProgramFromPTXFile(ptx_mat_path, "closest_hit");
        ah_pgrm = m_context->createProgramFromPTXFile(ptx_mat_path, "any_hit");
    } else if (0 == mat_type.compare("blank", Qt::CaseInsensitive)) {
        ptx_mat_path = rthelpers::ptxPath(mat_type.append(".cu").toStdString());
        ch_pgrm = m_context->createProgramFromPTXFile(ptx_mat_path, "closest_hit");
        ah_pgrm = m_context->createProgramFromPTXFile(ptx_mat_path, "any_hit");
    } else if (0 == mat_type.compare("phong", Qt::CaseInsensitive)) {
        ptx_mat_path = rthelpers::ptxPath(mat_type.append(".cu").toStdString());
        ch_pgrm = m_context->createProgramFromPTXFile(ptx_mat_path, "closest_hit");
        ah_pgrm = m_context->createProgramFromPTXFile(ptx_mat_path, "any_hit");
        // Setting default parameters
        ch_pgrm["Kd"]->setFloat(m_Kd);
        ch_pgrm["Ks"]->setFloat(m_Ks);
        ch_pgrm["specular_exponent"]->setFloat(m_spec_exp);
    } else {
        spdlog::error("Material type \"{}\" is not implemented. Please specify a valid material type", mat_type.toStdString());
    }
    m_material_optix->setClosestHitProgram(RADIANCE_RAY_TYPE, ch_pgrm);
    m_material_optix->setAnyHitProgram(SHADOW_RAY_TYPE, ah_pgrm);
    spdlog::debug("Setting material to {}", mat_type.toStdString());
}

void RT_material::setMaterialType(QString mat_type) {
    spdlog::debug("No material parameters were passed.");
    QString dummy_str = "";
    setMaterialType(mat_type, dummy_str);
}

void RT_material::setBRDF(QString &mat_type, QString &parameters) {
    setMaterialType(mat_type, parameters);
}

void RT_material::setBRDF(QString mat_type) {
    QString dummy_str = "";
    setBRDF(mat_type, dummy_str);
}

int RT_material::parseActions(const QString &action, const QString &parameters, const QString &delimiter/*=QString(";"*/) {
    QStringList sList = parameters.split(delimiter);
    if (sList.size() == 1)
    {
        if (0 == action.compare("setMaterialType", Qt::CaseInsensitive) ||
            0 == action.compare("setBRDF", Qt::CaseInsensitive) ||
            0 == action.compare("materialType", Qt::CaseInsensitive) ||
            0 == action.compare("brdf", Qt::CaseInsensitive) ||
            0 == action.compare("setMaterial", Qt::CaseInsensitive) |
            0 == action.compare("Material", Qt::CaseInsensitive))
        {
            setMaterialType(sList.at(0));
        }
    } else {
        if (0 == action.compare("setMaterialParameter", Qt::CaseInsensitive) ||
            0 == action.compare("materialParameter", Qt::CaseInsensitive))
        {
            if (0 == sList.at(0).compare("color", Qt::CaseInsensitive))
            {
                float x, y, z;
                rthelpers::RT_parse_float3(sList.at(1), &x, &y, &z);
                m_color = optix::make_float3(x, y, z);
                spdlog::debug("Setting material parameter color to: {}, {}, {}", m_color.x, m_color.y, m_color.z);
            } else if (0 == sList.at(0).compare("Kd", Qt::CaseInsensitive))
            {
                float x, y, z;
                rthelpers::RT_parse_float3(sList.at(1), &x, &y, &z);
                m_Kd = optix::make_float3(x, y, z);
                spdlog::debug("Setting material parameter Kd to: {}, {}, {}", m_Kd.x, m_Kd.y, m_Kd.z);
            } else if (0 == sList.at(0).compare("Ks", Qt::CaseInsensitive))
            {
                float x, y, z;
                rthelpers::RT_parse_float3(sList.at(1), &x, &y, &z);
                m_Ks = optix::make_float3(x, y, z);
                spdlog::debug("Setting material parameter Ks to: {}, {}, {}", m_Ks.x, m_Ks.y, m_Ks.z);
            } else if (0 == sList.at(0).compare("spec_exp", Qt::CaseInsensitive)) {
                bool ok = false;
                m_spec_exp = sList.at(1).toFloat(&ok);
                spdlog::debug("Setting material parameter specular exponent to {}", m_spec_exp);
            } else {
                spdlog::error("Was not able to set material parameters");
            }
            setMaterialType(m_mat_type);
        }
    }
    return 0;
}