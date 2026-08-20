// material.cpp

#include "material.hpp"
#include "material_pool.hpp"

triton::XMaterial::XMaterial(
    cContext* context,
    types::s32 poolIndex,
    const cVector4& diffuseColor,
    const XAtlasTexture::THandle& diffuseTexture,
    const XAtlasTexture::THandle& normalTexture,
    const XAtlasTexture::THandle& roughnessTexture,
    const XAtlasTexture::THandle& metallicTexture
) :
    iObject(context, poolIndex),
    _diffuseColor(diffuseColor),
    _diffuseTexture(diffuseTexture),
    _normalTexture(normalTexture),
    _roughnessTexture(roughnessTexture),
    _metallicTexture(metallicTexture)
{
    CMaterialPool* pool = _context->GetPool<CMaterialPool>();
    pool->WriteToStaging(
        pool->GetPackedIndex(poolIndex),
        *this
    );
}