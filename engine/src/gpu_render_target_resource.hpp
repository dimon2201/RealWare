// gpu_render_target_resource.hpp

#pragma once

#include "gpu_resource.hpp"
#include "gpu_texture_resource.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;

    class CGPURenderTargetResource : public cGPUResource
    {
        TRITON_OBJECT(CGPURenderTargetResource)

        types::usize _colorAttachmentCount = 0;
        CGPUTextureResource _colorAttachments[8];
        CGPUTextureResource _depthAttachment = CGPUTextureResource(
            nullptr,
            0,
            0,
            cVector3(0.0f),
            ETextureDimension::Unknown,
            ETextureFormat::Unknown,
            -1
        );

    public:
        explicit CGPURenderTargetResource() = default;
        explicit CGPURenderTargetResource(
            cContext* context,
            types::qword instance,
            types::qword viewInstance,
            types::usize colorAttachmentCount,
            const CGPUTextureResource* colorAttachments,
            const CGPUTextureResource& depthAttachment
        ) : cGPUResource(context, instance, viewInstance),
            _depthAttachment(depthAttachment)
        {
            SetColorAttachments(colorAttachmentCount, colorAttachments);
        }
        ~CGPURenderTargetResource() override = default;

        inline types::usize GetColorAttachmentCount() const { return _colorAttachmentCount; }

        inline const CGPUTextureResource* GetColorAttachments() const { return _colorAttachments; }

        inline const CGPUTextureResource& GetDepthAttachment() const { return _depthAttachment; }

        inline void SetColorAttachments(
            types::usize colorAttachmentCount,
            const CGPUTextureResource* colorAttachments
        )
        {
            _colorAttachmentCount = colorAttachmentCount;
            for (types::usize i = 0; i < _colorAttachmentCount; i++)
                _colorAttachments[i] = colorAttachments[i];
        }

        inline void SetDepthAttachment(const CGPUTextureResource& depthAttachment) { _depthAttachment = depthAttachment; }
    };
}