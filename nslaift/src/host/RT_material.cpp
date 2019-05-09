#include "RT_material.h"

RT_material::RT_material(optix::Context &context) :
m_context(context)
{
    m_material = m_context->createMaterial();
}

RT_material::~RT_material() {

}

void RT_material::setMaterialType(QString &mat_type) {
    if (0 == mat_type.compare("normal", Qt::CaseInsensitive)){
        // TODO: Make intelligent parsing
    }
}

void RT_material::setBRDF(QString &mat_type) {
    setMaterialType(mat_type);
}