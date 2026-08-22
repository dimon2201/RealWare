// material.cpp

#include "material.hpp"
#include "material_pool.hpp"

triton::XMaterial::XMaterial(
    cContext* context,
    types::s32 poolIndex,
    const XAtlasTexture::THandle& diffuseTexture,
    const XAtlasTexture::THandle& normalTexture,
    const XAtlasTexture::THandle& roughnessTexture,
    const XAtlasTexture::THandle& metallicTexture,
    const cVector4& diffuseColor,
    const cVector4& specularColor,
    types::f32 shininess
) :
    iObject(context, poolIndex),
    _diffuseTexture(diffuseTexture),
    _normalTexture(normalTexture),
    _roughnessTexture(roughnessTexture),
    _metallicTexture(metallicTexture),
    _diffuseColor(diffuseColor),
    _specularColor(specularColor),
    _shininess(shininess)
{
    CMaterialPool* pool = _context->GetPool<CMaterialPool>();
    pool->WriteToStaging(
        pool->GetPackedIndex(poolIndex),
        *this
    );
}

void triton::XMaterial::SetShininess(types::f32 shininess)
{
    _shininess = shininess;

    CMaterialPool* pool = _context->GetPool<CMaterialPool>();
    pool->WriteToStaging(
        pool->GetPackedIndex(_poolIndex),
        *this
    );
}