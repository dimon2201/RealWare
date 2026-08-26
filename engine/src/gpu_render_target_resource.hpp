// gpu_render_target_resource.hpp

#pragma once

#include <vector>
#include "gpu_resource.hpp"
#include "gpu_texture_resource.hpp"
#include "graphics_image_layout_enum.hpp"
#include "types.hpp"

namespace triton
{
    class CGPURenderTargetResource : public CGPUResource
    {
        std::vector<CGPUTextureResource> _colorAttachments;
        CGPUTextureResource _depthAttachment = CGPUTextureResource::Invalid();

    public:
        explicit CGPURenderTargetResource(
            types::qword instance,
            types::qword view,
            const std::vector<CGPUTextureResource>& colorAttachments,
            const CGPUTextureResource& depthAttachment
        ) : CGPUResource(instance, view),
            _depthAttachment(depthAttachment)
        {
            for (types::usize i = 0; i < colorAttachments.size(); i++)
                _colorAttachments.push_back(colorAttachments[i]);
        }
        ~CGPURenderTargetResource() override = default;

        static CGPURenderTargetResource Invalid()
        {
            return CGPURenderTargetResource(
                0, 0, std::vector<CGPUTextureResource>(), CGPUTextureResource::Invalid()
            );
        }

        inline types::usize GetColorAttachmentCount() const { return _colorAttachments.size(); }

        inline std::vector<CGPUTextureResource>& GetColorAttachments() { return _colorAttachments; }

        inline CGPUTextureResource& GetDepthAttachment() { return _depthAttachment; }

        inline const cVector2& GetSize() const
        {
            return cVector2(_colorAttachments[0].GetWidth(), _colorAttachments[0].GetHeight());
        }

        inline void SetDepthAttachment(const CGPUTextureResource& depthAttachment) { _depthAttachment = depthAttachment; }
    };
}