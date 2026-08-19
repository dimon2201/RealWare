// material_subsystem.cpp

#include "material_subsystem.hpp"
#include "graphics.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "capabilities.hpp"
#include "material_pool.hpp"

using namespace types;

triton::XMaterialSubsystem::XMaterialSubsystem(cContext* context) : ISubsys(context)
{
    _pool = CObjectAllocator::Create<XMaterialPool>(
        64,
        _context,
        K_TRUE,
        (s32)2,
        EGPUBufferType::Storage
    );
}

triton::XMaterialSubsystem::~XMaterialSubsystem()
{
    CObjectAllocator::Destroy<XMaterialPool>(_pool);
}

std::optional<triton::SMaterialData::THandle> triton::XMaterialSubsystem::Create(
    const cVector4& diffuseColor,
    const XAtlasTexture::THandle& diffuseTexture,
    const XAtlasTexture::THandle& normalTexture,
    const XAtlasTexture::THandle& roughnessTexture,
    const XAtlasTexture::THandle& metallicTexture
)
{
    auto handleResult = _pool->Create();
    if (!handleResult.has_value())
        return std::nullopt;
    auto handle = *handleResult;

    SMaterialData& m = *_pool->Get(handle);
    m.diffuseColor = diffuseColor;
    m.diffuseTexture = diffuseTexture;
    m.normalTexture = normalTexture;
    m.roughnessTexture = roughnessTexture;
    m.metallicTexture = metallicTexture;

    _pool->WriteToStaging(
        _pool->GetPackedIndex(handle),
        m
    );

    return handle;
}

void triton::XMaterialSubsystem::Update()
{
    _pool->Update();
}