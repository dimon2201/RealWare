// material_subsystem.cpp

#include "material_subsystem.hpp"
#include "material_uploader.hpp"
#include "graphics.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "capabilities.hpp"
#include "texture_subsystem.hpp"

using namespace types;

triton::XMaterialSubsystem::XMaterialSubsystem(cContext* context)
    : ISubsystem(context),
      XMaterialStorage(
          context,
          (cGPUResource**)&_materialGPUBuffer,
          context->GetSubsystem<cEngine>()->GetCapabilities()->maxRenderMaterialCount,
          K_FALSE
      )
{
    auto p = this;
    const sCapabilities* caps = context->GetSubsystem<cEngine>()->GetCapabilities();

    XRenderSubsystem* renderSubsystem = context->GetSubsystem<XRenderSubsystem>();
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_BUFFER,
        (cpuword)cBuffer::eType::STORAGE,
        (cpuword)nullptr,
        caps->maxRenderMaterialCount * sizeof(SGPUMaterialLayout),
        2
    ));
    _materialGPUBuffer = renderSubsystem->FetchResult<cBuffer*>();
}

triton::HMaterial triton::XMaterialSubsystem::CreateMaterial(
    const cVector4& diffuseColor,
    const HTexture& diffuseTexture,
    const HTexture& normalTexture,
    const HTexture& roughnessTexture,
    const HTexture& metallicTexture
)
{
    HMaterial material = XMaterialStorage::CreateMaterial(
        diffuseColor,
        diffuseTexture,
        normalTexture,
        roughnessTexture,
        metallicTexture
    );

    SGPUMaterialLayout gml = ConvertToGPULayout(material);
    WriteToStaging(
        GetHandleBufferIndex(material),
        &gml,
        1
    );

    return material;
}

void triton::XMaterialSubsystem::Init()
{
}

void triton::XMaterialSubsystem::Free()
{
}

void triton::XMaterialSubsystem::Update()
{
    UploadStagingToGpuIfDirty(_context->GetSubsystem<XRenderSubsystem>());
}

triton::SGPUMaterialLayout triton::XMaterialSubsystem::ConvertToGPULayout(const HMaterial& material)
{
    SMaterial& m = XMaterialStorage::Get(material);

    STexture& dif = _context->GetSubsystem<XTextureSubsystem>()->Get(m.diffuseTexture);
    STexture& nor = _context->GetSubsystem<XTextureSubsystem>()->Get(m.normalTexture);
    STexture& rgh = _context->GetSubsystem<XTextureSubsystem>()->Get(m.roughnessTexture);
    STexture& met = _context->GetSubsystem<XTextureSubsystem>()->Get(m.metallicTexture);

    SGPUMaterialLayout gml;
    gml.diffuseColor = m.diffuseColor;
    gml.diffuseTextureLayout.layer = dif.layer;
    gml.diffuseTextureLayout.normOffset = dif.normOffset;
    gml.diffuseTextureLayout.normSize = dif.normSize;
    gml.normalTextureLayout.layer = nor.layer;
    gml.normalTextureLayout.normOffset = nor.normOffset;
    gml.normalTextureLayout.normSize = nor.normSize;
    gml.roughnessTextureLayout.layer = rgh.layer;
    gml.roughnessTextureLayout.normOffset = rgh.normOffset;
    gml.roughnessTextureLayout.normSize = rgh.normSize;
    gml.metallicTextureLayout.layer = met.layer;
    gml.metallicTextureLayout.normOffset = met.normOffset;
    gml.metallicTextureLayout.normSize = met.normSize;

    return gml;
}