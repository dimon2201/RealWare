// render_pass.cpp

#include "render_pass.hpp"
#include "context.hpp"
#include "graphics_backend.hpp"
#include "input_layout_pool.hpp"
#include "render_target_pool.hpp"
#include "camera_pool.hpp"
#include "input.hpp"
#include "batcher.hpp"
#include "shader_pool.hpp"

using namespace types;

triton::XRenderPass::XRenderPass(cContext* context) : iObject(context) {}

void triton::XRenderPass::Bind()
{
    CThreadGuard::AssertRender();

    XShader& shader = *_context->GetPool<XShaderPool>()->Get(_shader);

    IGraphicsBackend* gfxBackend = _context->GetBackend<IGraphicsBackend>();
    gfxBackend->BindShader(shader.GetGPUResource());
    gfxBackend->SetViewport(_viewport);
    gfxBackend->BindDepthState(_depthState);
    gfxBackend->BindBlendState(_blendState);

    gfxBackend->BindBuffer(_context->GetSubsystem<XBatchSubsystem>()->GetStaticRenderInstancePool()->GetGPUBuffer());
    gfxBackend->BindBuffer(_context->GetSubsystem<XMaterialSubsystem>()->GetPool()->GetGPUBuffer());

    for (auto& tex : _inputTextures)
        gfxBackend->BindTextureNamed(shader.GetGPUResource(), tex.texture, tex.name, -1);
    
    auto iaResult = _context->GetPool<XInputLayoutPool>()->Get(_inputLayout);
    if (iaResult)
        gfxBackend->BindInputLayout((*iaResult).get().GetGPUResource());

    auto rtResult = _context->GetPool<XRenderTargetPool>()->Get(_renderTarget);
    if (rtResult)
        gfxBackend->BindRenderTarget((*rtResult).get()._renderTarget);
}

void triton::XRenderPass::Unbind()
{
    CThreadGuard::AssertRender();

    IGraphicsBackend* gfxBackend = _context->GetBackend<IGraphicsBackend>();
    gfxBackend->UnbindShader();
    // TODO: uncomment this after debug
    //for (auto& tex : _inputTextures)
    //    gfxResourceBackend->UnbindTexture(tex._texture);
    
    auto iaResult = _context->GetPool<XInputLayoutPool>()->Get(_inputLayout);
    if (iaResult)
        gfxBackend->UnbindInputLayout();

    auto rtResult = _context->GetPool<XRenderTargetPool>()->Get(_renderTarget);
    if (rtResult)
        gfxBackend->UnbindRenderTarget();
}

void triton::XRenderPass::Render()
{
    CThreadGuard::AssertRender();

    IGraphicsBackend* gfxBackend = _context->GetBackend<IGraphicsBackend>();

    switch (_dispatch)
    {
        case ERenderPassDispatch::Geometry:
        {
            XCamera& camera = *_context->GetPool<XCameraPool>()->Get(_camera);
            XShader& shader = *_context->GetPool<XShaderPool>()->Get(_shader);

            // TODO: remove this line
            cInputWindow& ibw = _context->GetSubsystem<cInput>()->GetWindows()->at(0);
            cVector2 cursorPos = _context->GetSubsystem<cInput>()->GetCursorPosition(&ibw);
            camera.Update(cursorPos, 800, 600, 65.0f, 0.01f, 10000.0f, 0.1f);
            cVector4 cameraWorldPos = cVector4(camera._worldPosition.GetX(), camera._worldPosition.GetY(), camera._worldPosition.GetZ(), 0.0f);
            ///////////////////////////////////////////////////////////////////////////

            camera.Bind(this);

            static u32 time = 0;
            time += 1;

            gfxBackend->ClearColor(cVector4(0.45f, 0.0f, 0.8f, 1.0f));
            gfxBackend->ClearDepth(1.0f);

            gfxBackend->SetShaderUniform(shader.GetGPUResource(), "UniformTime", (u32)time);
            gfxBackend->SetShaderUniform(shader.GetGPUResource(), "CameraPosWorldSpace", 1, (f32*)&cameraWorldPos);
            
            for (usize i = 0; i < _batches.size(); i++)
            {
                // Static
                SBatchData::THandle batchHandle = _batches.at(i);
                const SBatchData& batch = _context->GetSubsystem<XBatchSubsystem>()->Get(batchHandle);
                const SGeometryView geometry = batch.sharedGeometry;

                gfxBackend->SetShaderUniform(shader.GetGPUResource(), "InstanceBatchType", (u32)batch.motionType);
                gfxBackend->SetShaderUniform(shader.GetGPUResource(), "InstanceOffset", (u32)batch.bufferOffset);
                gfxBackend->Draw(
                    geometry._indexCount,
                    geometry._vertexElementOffset,
                    geometry._indexElementOffset,
                    batch.instanceCount
                );

                // Dynamic
                /*gfxPipelineBackend->SetShaderUniform(&shader, "InstanceBatchType", 1);
                gfxPipelineBackend->SetShaderUniform(&shader, "InstanceOffset", (u32)batch.GetInstanceOffset(ERenderInstanceMotionType::Dynamic));
                gfxDrawcallBackend->Draw(
                    geometry._indexCount,
                    geometry._vertexElementOffset,
                    geometry._indexElementOffset,
                    batch.GetInstanceCount(ERenderInstanceMotionType::Dynamic)
                );*/
            }
            break;
        }
        case ERenderPassDispatch::Processing:
        {
            gfxBackend->DrawQuad();
            break;
        }
    }
}

void triton::XRenderPass::Execute()
{
    CThreadGuard::AssertRender();

    Bind();
    Render();
    Unbind();
}

void triton::XRenderPass::ResizeViewport(const cVector2& size)
{
    _viewport.rect.SetZ(size.GetX());
    _viewport.rect.SetW(size.GetY());
}

void triton::XRenderPass::ResizeColorAttachments(const cVector2& size)
{
    XRenderTarget& rt = *_context->GetPool<XRenderTargetPool>()->Get(_renderTarget);

    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::RESIZE_RENDER_TARGET_COLORS,
        (cpuword)&rt._renderTarget,
        size.GetX(),
        size.GetY()
    ));
}

void triton::XRenderPass::ResizeDepthAttachment(const cVector2& size)
{
    XRenderTarget& rt = *_context->GetPool<XRenderTargetPool>()->Get(_renderTarget);

    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::RESIZE_RENDER_TARGET_DEPTH,
        (cpuword)&rt._renderTarget,
        size.GetX(),
        size.GetY()
    ));
}