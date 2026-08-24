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
        CGPUTextureResource _depthAttachment = CGPUTextureResource(
            0, 0,cVector3(0.0f), ETextureDimension::Unknown, ETextureFormat::Unknown, 0
        );
        EGraphicsImageLayout _colorAttachmentSrcLayouts[8];
        EGraphicsImageLayout _colorAttachmentDstLayouts[8];
        EGraphicsImageLayout _depthAttachmentSrcLayout;
        EGraphicsImageLayout _depthAttachmentDstLayout;

    public:
        explicit CGPURenderTargetResource(
            types::qword instance,
            types::qword view,
            const std::vector<CGPUTextureResource>& colorAttachments,
            const CGPUTextureResource& depthAttachment
        ) : CGPUResource(instance, view),
            _depthAttachment(depthAttachment)
        {
            SetColorAttachments(colorAttachments);
        }
        ~CGPURenderTargetResource() override = default;

        static CGPURenderTargetResource Invalid()
        {
            return CGPURenderTargetResource(
                0, 0, std::vector<CGPUTextureResource>(), CGPUTextureResource::Invalid()
            );
        }

        inline types::usize GetColorAttachmentCount() const { return _colorAttachments.size(); }

        inline const std::vector<CGPUTextureResource> GetColorAttachments() const { return _colorAttachments; }

        inline const CGPUTextureResource& GetDepthAttachment() const { return _depthAttachment; }

        inline const EGraphicsImageLayout* GetColorAttachmentSrcLayouts() const { return _colorAttachmentSrcLayouts; }

        inline const EGraphicsImageLayout* GetColorAttachmentDstLayouts() const { return _colorAttachmentDstLayouts; }

        inline const EGraphicsImageLayout GetDepthAttachmentSrcLayout() const { return _depthAttachmentSrcLayout; }

        inline const EGraphicsImageLayout GetDepthAttachmentDstLayout() const { return _depthAttachmentDstLayout; }

        inline const cVector2& GetSize() const
        {
            return cVector2(_colorAttachments[0].GetWidth(), _colorAttachments[0].GetHeight());
        }

        inline void SetColorAttachments(const std::vector<CGPUTextureResource>& colorAttachments)
        {
            const types::usize attachmentCount = colorAttachments.size();
            _colorAttachments.reserve(attachmentCount);
            for (types::usize i = 0; i < attachmentCount; i++)
                _colorAttachments.push_back(colorAttachments[i]);
        }

        inline void SetColorAttachmentLayout(
            types::usize attachmentIndex,
            EGraphicsImageLayout srcLayout,
            EGraphicsImageLayout dstLayout
        )
        {
            _colorAttachmentSrcLayouts[attachmentIndex] = srcLayout;
            _colorAttachmentDstLayouts[attachmentIndex] = dstLayout;
        }

        inline void SetDepthAttachmentLayout(
            EGraphicsImageLayout srcLayout,
            EGraphicsImageLayout dstLayout
        )
        {
            _depthAttachmentSrcLayout = srcLayout;
            _depthAttachmentSrcLayout = dstLayout;
        }

        inline void SetDepthAttachment(const CGPUTextureResource& depthAttachment) { _depthAttachment = depthAttachment; }
    };
}