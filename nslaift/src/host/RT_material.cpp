#include "RT_material.h"

RT_material::RT_material(optix::Context &context) :
m_context(context)
{
    m_material_optix = m_context->createMaterial();

    // Default material is set to rendering normals
    setMaterialType("normal");
}

RT_material::~RT_material() {
    m_material_optix->destroy();
}

void RT_material::setMaterialType(QString &mat_type, QString &parameters) {
    // TODO: Parse material parameters depending of brdf function

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
        // TODO: define variables e.g.: ch_pgrm["color"]->setFloat(constColor)
    } else if (0 == mat_type.compare("phong", Qt::CaseInsensitive)) {
        ptx_mat_path = rthelpers::ptxPath(mat_type.append(".cu").toStdString());
        ch_pgrm = m_context->createProgramFromPTXFile(ptx_mat_path, "closest_hit");
        ah_pgrm = m_context->createProgramFromPTXFile(ptx_mat_path, "any_hit");
        // TODO: define variables e.g.: ch_pgrm["Kd"]->setFloat(phongParam)
    } else {
        spdlog::error("Material type \"{}\" is not implemented. Please specify a valid material type", mat_type.toStdString());
    }
    m_material_optix->setClosestHitProgram(RADIANCE_RAY_TYPE, ch_pgrm);
    m_material_optix->setAnyHitProgram(SHADOW_RAY_TYPE, ah_pgrm);
    spdlog::debug("Setting material to {}", mat_type.toStdString());
}

void RT_material::setMaterialType(QString mat_type) {
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
    if (sList.size() == 1) {
        setMaterialType(sList.at(0));
    }
    return 0;
}