// animation_subsystem.cpp

#include "animation_subsystem.hpp"
#include "render_subsystem.hpp"
#include "graphics_resource_backend.hpp"
#include "animation.hpp"

using namespace types;

triton::HAnimation triton::XAnimationSubsystem::CreateAnimation(
    const std::string& name,
    types::f32 duration,
    types::f32 ticksPerSecond,
    const std::vector<SBoneAnimation>& bones
)
{
    HAnimation animation = Create();
    SAnimation& a = Get(animation);
    a.name = name;
    a.duration = duration;
    a.ticksPerSecond = ticksPerSecond;
    a.bones = bones;
    
    return animation;
}

void triton::XAnimationSubsystem::DestroyAnimation(const HAnimation& animation)
{
    Destroy(animation);
}

void triton::XAnimationSubsystem::Init()
{
    XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
    IApplication* app = _context->GetSubsystem<cEngine>()->GetApplication();
    const sCapabilities* caps = app->GetCapabilities();
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_BUFFER,
        (cpuword)cBuffer::eType::STORAGE,
        (cpuword)nullptr,
        caps->boneBufferSize,
        3
    ));
    _boneBuffer = renderSubsystem->FetchResult<cBuffer*>();
}

void triton::XAnimationSubsystem::Free()
{
    XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_BUFFER,
        (cpuword)_boneBuffer,
        0,
        0,
        0
    ));
}