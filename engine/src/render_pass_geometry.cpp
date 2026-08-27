// render_pass_geometry.cpp

#include "render_pass_geometry.hpp"
#include "thread_guard.hpp"
#include "shader_pool.hpp"
#include "render_instance_static_pool.hpp"
#include "render_instance_dynamic_pool.hpp"
#include "material_pool.hpp"
#include "input_layout_pool.hpp"
#include "camera_pool.hpp"
#include "render_target_pool.hpp"
#include "render_instance_pack_pool.hpp"
#include "gpu_buffer_pool.hpp"
#include "input_backend.hpp"
#include "geometry_storage.hpp"
#include "gpu_buffer.hpp"

using namespace types;

triton::XRenderPassGeometry::XRenderPassGeometry(
    cContext* context,
    s32 poolIndex,
    const XRenderTarget::THandle& renderTargetHandle,
    boolean bClearRenderTarget,
    const cVector4& clearColor,
    f32 clearDepth,
    const std::vector<EResourceUsage>& srcAttachmentsUsage,
    const std::vector<EResourceUsage>& dstAttachmentsUsage,
    const XShader::THandle& shaderHandle,
    EPrimitiveTopology primitiveTopology,
    const SViewport& viewport
) :
    IRenderPass(context, poolIndex, ERenderPassDispatch::Geometry),
    _bClearRenderTarget(bClearRenderTarget),
    _renderTarget(renderTargetHandle),
    _shader(shaderHandle),
    _primitiveTopology(primitiveTopology),
    _viewport(viewport)
{
    XRenderTarget& renderTarget = *_context->GetPool<CRenderTargetPool>()->Get(_renderTarget);
    
    const CGPURenderTargetResource& gpuRenderTarget = renderTarget.GetGPUResource();

    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_RENDER_PASS,
        (cpuword)&gpuRenderTarget,
        (cpuword)bClearRenderTarget,
        (cpuword)&clearColor,
        (cpuword)&clearDepth,
        (cpuword)&srcAttachmentsUsage,
        (cpuword)&dstAttachmentsUsage
    ));

    _gpuRenderPass = _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<CGPURenderPassResource>();

    XShader& shader = *_context->GetPool<CShaderPool>()->Get(_shader);

    const usize texturesToBindCount = _inputTextures.size();
    std::vector<CGPUTextureResource> texturesToBind;
    for (usize i = 0; i < texturesToBindCount; i++)
        texturesToBind.push_back(_inputTextures[i].texture);

    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_PIPELINE,
        (cpuword)&shader.GetGPUResource(),
        (cpuword)&_viewport,
        (cpuword)&gpuRenderTarget,
        (cpuword)&_gpuRenderPass,
        (cpuword)texturesToBindCount,
        (cpuword)texturesToBind.data(),
        (cpuword)primitiveTopology,
        (cpuword)EVertexBufferFormat::Rigid_48
    ));

    _gpuPipeline = _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<CGPUPipelineResource>();
}

triton::XRenderPassGeometry::~XRenderPassGeometry()
{
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_PIPELINE,
        (cpuword)&_gpuPipeline
    ));

    _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<void*>();

    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_RENDER_PASS,
        (cpuword)&_gpuRenderPass
    ));

    _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<void*>();
}

void triton::XRenderPassGeometry::Render()
{
    Bind();
    Draw();
    Unbind();
}

void triton::XRenderPassGeometry::Bind()
{
    // TODO: [Vulkan backend] This must be done using render command queue on main thread
    // ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
    // VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
    /*IGraphicsBackend* gfxBackend = _context->GetBackend<IGraphicsBackend>();

    XShader& shader = *_context->GetPool<CShaderPool>()->Get(_shader);
    gfxBackend->BindShader(shader.GetGPUResource());

    gfxBackend->SetViewport(_viewport);
    gfxBackend->BindDepthState(_depthState);
    gfxBackend->BindBlendState(_blendState);

    for (auto& tex : _inputTextures)
        gfxBackend->BindTextureNamed(shader.GetGPUResource(), tex.texture, tex.name, -1);

    auto iaResult = _context->GetPool<CInputLayoutPool>()->Get(_inputLayout);
    if (iaResult)
        gfxBackend->BindInputLayout((*iaResult).get().GetGPUResource());

    auto rtResult = _context->GetPool<CRenderTargetPool>()->Get(_renderTarget);
    if (rtResult)
        gfxBackend->BindRenderTarget((*rtResult).get().GetGPUResource());
    else
        gfxBackend->UnbindRenderTarget();

    if (_clearState.has_value())
    {
        SClearState cs = *_clearState;
        gfxBackend->ClearColor(cs.color);
        gfxBackend->ClearDepth(cs.depth);
    }*/
}

void triton::XRenderPassGeometry::Draw()
{
    // TODO: [Vulkan backend] This must be done using render command queue on main thread
    // ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
    // VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
    /*IGraphicsBackend* gfxBackend = _context->GetBackend<IGraphicsBackend>();

    auto cameraResult = _context->GetPool<CCameraPool>()->Get(_camera);
    if (!cameraResult.has_value())
        return;

    XCamera& camera = *cameraResult;
    XShader& shader = *_context->GetPool<CShaderPool>()->Get(_shader);

    // TODO: remove this line
    const cVector2 mouseDelta = _context->GetBackend<IInputBackend>()->GetMouseDelta();
    camera.Update(mouseDelta, 800, 600, 65.0f, 0.01f, 10000.0f, 0.1f);
    cVector4 cameraWorldPos = cVector4(camera._worldPosition.GetX(), camera._worldPosition.GetY(), camera._worldPosition.GetZ(), 0.0f);
    ///////////////////////////////////////////////////////////////////////////

    camera.Bind(this);

    static u32 time = 0;
    time += 1;

    gfxBackend->SetShaderUniform(shader.GetGPUResource(), "time", (u32)time);
    gfxBackend->SetShaderUniform(shader.GetGPUResource(), "cameraPositionWorldSpace", 1, (f32*)&cameraWorldPos);

    for (usize i = 0; i < _renderInstancePacks.size(); i++)
    {
        XRenderInstancePack& instancePack =
            *_context->
            GetPool<CRenderInstancePackPool>()->
            Get(_renderInstancePacks.at(i));

        const SGeometryView sharedGeometry = instancePack.GetSharedGeometry();

        gfxBackend->SetShaderUniform(shader.GetGPUResource(), "instanceBatchType", (u32)instancePack.GetMotionType());
        gfxBackend->SetShaderUniform(shader.GetGPUResource(), "instanceOffset", (u32)instancePack.GetBufferOffset());
        
        const XMaterial::THandle& sharedMaterialHandle = instancePack.GetSharedMaterial();
        const XMaterial& sharedMaterial = *_context->GetPool<CMaterialPool>()->Get(sharedMaterialHandle);
        
        if (_shadingModel == EShadingModel::PBR)
        {
            const XTexture& sharedMaterialDiffuseTex = *_context->GetPool<PTexturePool>()->Get(sharedMaterial.GetDiffuseTexture());
            const XTexture& sharedMaterialNormalTex = *_context->GetPool<PTexturePool>()->Get(sharedMaterial.GetNormalTexture());
            const XTexture& sharedMaterialRoughnessTex = *_context->GetPool<PTexturePool>()->Get(sharedMaterial.GetRoughnessTexture());
            const XTexture& sharedMaterialMetallicTex = *_context->GetPool<PTexturePool>()->Get(sharedMaterial.GetMetallicTexture());

            gfxBackend->BindTextureNamed(
                shader.GetGPUResource(),
                sharedMaterialDiffuseTex.GetGPUResource(),
                "inDiffuseTexture",
                0
            );
            gfxBackend->BindTextureNamed(
                shader.GetGPUResource(),
                sharedMaterialNormalTex.GetGPUResource(),
                "inNormalTexture",
                1
            );
            gfxBackend->BindTextureNamed(
                shader.GetGPUResource(),
                sharedMaterialRoughnessTex.GetGPUResource(),
                "inRoughnessTexture",
                2
            );
            gfxBackend->BindTextureNamed(
                shader.GetGPUResource(),
                sharedMaterialMetallicTex.GetGPUResource(),
                "inMetallicTexture",
                3
            );
        }
        else if (_shadingModel == EShadingModel::PBR)
        {
            const XTexture& sharedMaterialDiffuseTex = *_context->GetPool<PTexturePool>()->Get(sharedMaterial.GetDiffuseTexture());
            const XTexture& sharedMaterialNormalTex = *_context->GetPool<PTexturePool>()->Get(sharedMaterial.GetNormalTexture());

            gfxBackend->BindTextureNamed(
                shader.GetGPUResource(),
                sharedMaterialDiffuseTex.GetGPUResource(),
                "inDiffuseTexture",
                0
            );
            gfxBackend->BindTextureNamed(
                shader.GetGPUResource(),
                sharedMaterialNormalTex.GetGPUResource(),
                "inNormalTexture",
                1
            );
        }

        gfxBackend->Draw(
            sharedGeometry._indexCount,
            sharedGeometry._vertexElementOffset,
            sharedGeometry._indexElementOffset,
            instancePack.GetInstanceCount()
        );
    }*/

    XRenderTarget& renderTarget = *_context->GetPool<CRenderTargetPool>()->Get(_renderTarget);

    _nativeCommandDrawInfo.vertexCount = 3;
    _nativeCommandDrawInfo.instanceCount = 1;
    _nativeCommandDrawInfo.firstVertex = 0;
    _nativeCommandDrawInfo.firstInstance = 0;

    XGPUBuffer::THandle vertexBufferHandle = _context->GetSubsystem<CGeometryStorage>()->GetRigidVertexBuffer();
    XGPUBuffer::THandle indexBufferHandle = _context->GetSubsystem<CGeometryStorage>()->GetRigidIndexBuffer();
    XGPUBuffer& vertexBuffer = *_context->GetPool<CGPUBufferPool>()->Get(vertexBufferHandle);
    XGPUBuffer& indexBuffer = *_context->GetPool<CGPUBufferPool>()->Get(indexBufferHandle);
    const CGPUBufferResource& gpuVertexBuffer = vertexBuffer.GetGPUResource();
    const CGPUBufferResource& gpuIndexBuffer = indexBuffer.GetGPUResource();

    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::EXECUTE_RENDER_PASS,
        (cpuword)&renderTarget.GetGPUResource(),
        (cpuword)&_gpuRenderPass,
        (cpuword)&_gpuPipeline,
        (cpuword)&gpuVertexBuffer,
        (cpuword)&gpuIndexBuffer,
        (cpuword)&_nativeCommandDrawInfo
    ));
}

void triton::XRenderPassGeometry::Unbind()
{
    // TODO: [Vulkan backend] This must be done using render command queue on main thread
    // ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
    // VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
    /*IGraphicsBackend* gfxBackend = _context->GetBackend<IGraphicsBackend>();

    gfxBackend->UnbindInputLayout();

    for (auto& tex : _inputTextures)
        gfxBackend->UnbindTexture(tex.texture);

    gfxBackend->UnbindRenderTarget();*/
}