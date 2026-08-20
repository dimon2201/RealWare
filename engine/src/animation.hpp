// animation.hpp

#pragma once

#include <optional>
#include "object.hpp"
#include "handle.hpp"
#include "math.hpp"
#include "animation_key.hpp"
#include "animation_frame.hpp"
#include "skeleton.hpp"

namespace triton
{
    class cMatrix4;

    class XAnimation : public iObject
    {
        TRITON_OBJECT(XAnimation)

        std::string _name = "";
        types::f32 _duration = 0.0f;
        types::f32 _ticksPerSecond = 0.0f;
        std::vector<SAnimationKey> _keys = {};

    public:
        explicit XAnimation(cContext* context, types::s32 poolIndex) : iObject(context, poolIndex) {}

        explicit XAnimation(
            cContext* context,
            types::s32 poolIndex,
            const std::string& name,
            types::f32 duration,
            types::f32 ticksPerSecond,
            const std::vector<SAnimationKey> keys
        );
        
        ~XAnimation() override = default;

        std::optional<triton::SAnimationFrame> EvaluateFrame(
            const XSkeleton::THandle& skeleton,
            types::f32 time
        );

        struct THandle : public SHandle {};

        struct TGPULayout {};
    };
}