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

using namespace types;

void triton::XRenderPassGeometry::Render()
{
    CThreadGuard::AssertRender();

    Bind();
    Draw();
    Unbind();
}

void triton::XRenderPassGeometry::Bind()
{
    IGraphicsBackend* gfxBackend = _context->GetBackend<IGraphicsBackend>();

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
    }
}

void triton::XRenderPassGeometry::Draw()
{
    IGraphicsBackend* gfxBackend = _context->GetBackend<IGraphicsBackend>();

    XCamera& camera = *_context->GetPool<CCameraPool>()->Get(_camera);
    XShader& shader = *_context->GetPool<CShaderPool>()->Get(_shader);

    // TODO: remove this line
    cInputWindow& ibw = _context->GetSubsystem<CInput>()->GetWindows()->at(0);
    cVector2 cursorPos = _context->GetSubsystem<CInput>()->GetCursorPosition(&ibw);
    camera.Update(cursorPos, 800, 600, 65.0f, 0.01f, 100.0f, 0.1f);
    cVector4 cameraWorldPos = cVector4(camera._worldPosition.GetX(), camera._worldPosition.GetY(), camera._worldPosition.GetZ(), 0.0f);
    ///////////////////////////////////////////////////////////////////////////

    camera.Bind(this);

    static u32 time = 0;
    time += 1;

    gfxBackend->SetShaderUniform(shader.GetGPUResource(), "UniformTime", (u32)time);
    gfxBackend->SetShaderUniform(shader.GetGPUResource(), "CameraPosWorldSpace", 1, (f32*)&cameraWorldPos);

    for (usize i = 0; i < _renderInstancePacks.size(); i++)
    {
        XRenderInstancePack& instancePack =
            *_context->
            GetPool<CRenderInstancePackPool>()->
            Get(_renderInstancePacks.at(i));

        const SGeometryView sharedGeometry = instancePack.GetSharedGeometry();

        gfxBackend->SetShaderUniform(shader.GetGPUResource(), "InstanceBatchType", (u32)instancePack.GetMotionType());
        gfxBackend->SetShaderUniform(shader.GetGPUResource(), "InstanceOffset", (u32)instancePack.GetBufferOffset());
        gfxBackend->Draw(
            sharedGeometry._indexCount,
            sharedGeometry._vertexElementOffset,
            sharedGeometry._indexElementOffset,
            instancePack.GetInstanceCount()
        );
    }
}

void triton::XRenderPassGeometry::Unbind()
{
    IGraphicsBackend* gfxBackend = _context->GetBackend<IGraphicsBackend>();

    gfxBackend->UnbindInputLayout();

    for (auto& tex : _inputTextures)
        gfxBackend->UnbindTexture(tex.texture);

    gfxBackend->UnbindRenderTarget();
}