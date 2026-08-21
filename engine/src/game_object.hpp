// game_object.hpp

#pragma once

#include <string>
#include <optional>
#include <string>
#include "object.hpp"
#include "handle.hpp"
#include "material.hpp"
#include "skeleton.hpp"
#include "animation.hpp"
#include "skin.hpp"
#include "model3d.hpp"
#include "math.hpp"
#include "render_instance_motion_type.hpp"
#include "render_instance_pack.hpp"
#include "render_instance.hpp"

namespace triton
{
    class cContext;

    class XGameObject : public iObject
    {
        TRITON_OBJECT(XGameObject)

        std::string                         _name = {};
        ERenderInstanceMotionType           _motionType = ERenderInstanceMotionType::Unknown;
        XRenderInstancePack::THandle        _renderInstancePack;
        XRenderInstance::THandle            _renderInstance;
        XSkeleton::THandle                  _skeleton;
        XSkin::THandle                      _skin;
        std::vector<XAnimation::THandle>    _animations = {};

    public:
        explicit XGameObject(cContext* context, types::s32 poolIndex) : iObject(context, poolIndex) {}

        explicit XGameObject(cContext* context, types::s32 poolIndex, const std::string& name);

        explicit XGameObject(
            cContext* context,
            types::s32 poolIndex,
            const std::string& name,
            const SModel3DData& model,
            const XRenderInstancePack::THandle& renderInstancePack
        );

        ~XGameObject() override = default;

        std::optional<XRenderInstance::THandle> SetRenderable(
            types::boolean bIsRenderable,
            const XRenderInstancePack::THandle& renderInstancePack = XRenderInstancePack::THandle()
        );

        void SetRenderable(
            const XRenderInstancePack::THandle& renderInstancePack,
            const XRenderInstance::THandle& renderInstance
        );

        void SetMaterial(const XMaterial::THandle& material);

        void SetWorldPosition(const cVector3& worldPosition);

        void SetRotation(const cVector3& rotation);

        void SetScale(const cVector3& scale);

        void PlayAnimation(types::usize index);

        struct THandle : public SHandle {};

        struct TGPULayout {};
    };
}