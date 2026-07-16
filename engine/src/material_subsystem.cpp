// material_subsystem.cpp

#include "material_subsystem.hpp"
#include "material_uploader.hpp"
#include "graphics.hpp"

using namespace types;

triton::XMaterialSubsystem::XMaterialSubsystem(cContext* context) : ISubsystem(context)
{
    _uploader = _context->Create<XMaterialUploader>(_context);
}

triton::XMaterialSubsystem::~XMaterialSubsystem()
{
    _context->Destroy<XMaterialUploader>(_uploader);
}

triton::HMaterial triton::XMaterialSubsystem::CreateMaterial(
    const cVector4& diffuseColor,
    const HTexture& diffuseTexture,
    const HTexture& normalTexture,
    const HTexture& roughnessTexture,
    const HTexture& metallicTexture
)
{
    HMaterial material = Create();
    SMaterial materialData;
    materialData.diffuseColor = diffuseColor;
    materialData.diffuseTexture = diffuseTexture;
    materialData.normalTexture = normalTexture;
    materialData.roughnessTexture = roughnessTexture;
    materialData.metallicTexture = metallicTexture;

    _objects->Get(material) = materialData;
    Set(material._indexInArray, materialData);

    return material;
}

void triton::XMaterialSubsystem::Set(types::usize materialIndex, const SMaterial& materialData)
{
    _uploader->Set(_context->GetSubsystem<XTextureSubsystem>(), materialIndex, materialData);
}

void triton::XMaterialSubsystem::Init()
{
}

void triton::XMaterialSubsystem::Free()
{
}

void triton::XMaterialSubsystem::Update()
{
    cGraphics* gfx = _context->GetSubsystem<cGraphics>();
    const usize byteSizeToUpload = _objects->GetSize() * sizeof(SMaterialLayout);
    _uploader->Upload(gfx->GetMaterialBuffer(), byteSizeToUpload);
}

void triton::XMaterialSubsystem::MarkDirty()
{
    _uploader->MarkDirty();
}