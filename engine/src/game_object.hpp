// game_object.hpp

#pragma once

#include <string>
#include "object.hpp"
#include "handle.hpp"
#include "material_data.hpp"
#include "skeleton_data.hpp"
#include "render_instance_data.hpp"
#include "math.hpp"

namespace triton
{
    class XGameObject : public iObject
    {
        TRITON_OBJECT(XGameObject)

        std::string _name = {};
        ERenderInstanceMotionType _motionType = ERenderInstanceMotionType::Static;
        SStaticRenderInstanceData::THandle _staticRenderInstance;
        SDynamicRenderInstanceData::THandle _dynamicRenderInstance;
        SSkeletonData::THandle _skeleton;

    public:
        struct THandle : public SHandle {};

        struct TGPULayout {};

        explicit XGameObject(cContext* context) : iObject(context) {}
        ~XGameObject() override = default;
    };
}