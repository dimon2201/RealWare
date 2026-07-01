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

triton::HMaterial triton::XMaterialSubsystem::CreateMaterial(const cVector4& diffuseColor, const HTexture& diffuseTexture)
{
    HMaterial material = Create();
    SMaterial materialData;
    materialData.diffuseColor = diffuseColor;
    materialData.diffuseTexture = diffuseTexture;
    *_objects->Get(material) = materialData;

    MarkDirty();

    return material;
}

void triton::XMaterialSubsystem::Set(types::usize materialIndex, const SMaterial& materialData)
{
    _uploader->Set(_context->GetSubsystem<XTextureSubsystem>(), materialIndex, materialData);
    _uploader->MarkDirty();
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
    _uploader->Upload(gfx->GetMaterialBuffer(), _objects->GetByteSize());
}

void triton::XMaterialSubsystem::MarkDirty()
{
    _uploader->MarkDirty();
}