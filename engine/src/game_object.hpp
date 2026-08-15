// game_object.hpp

#pragma once

#include <string>
#include <optional>
#include "object.hpp"
#include "handle.hpp"
#include "material_data.hpp"
#include "skeleton_data.hpp"
#include "render_instance_data.hpp"
#include "batch_data.hpp"
#include "animation.hpp"
#include "skinning_data.hpp"
#include "model3d_data.hpp"
#include "math.hpp"

namespace triton
{
    class XGameObject : public iObject
    {
        TRITON_OBJECT(XGameObject)

        std::string _name = {};
        ERenderInstanceMotionType _motionType = ERenderInstanceMotionType::Static;
        SBatchData::THandle _batch;
        SStaticRenderInstanceData::THandle _staticRenderInstance;
        SDynamicRenderInstanceData::THandle _dynamicRenderInstance;
        SSkeletonData::THandle _skeleton;
        SSkinData::THandle _skin;
        std::vector<SAnimationData::THandle> _animations = {};

    public:
        struct THandle : public SHandle {};

        struct TGPULayout {};

        explicit XGameObject(cContext* context, const SBatchData::THandle& batchHandle);
        explicit XGameObject(cContext* context, const SModel3DData& model, const SBatchData::THandle& batchHandle);
        ~XGameObject() override;

        std::optional<SStaticRenderInstanceData::THandle> SetRenderableStatic(const SBatchData::THandle& batch);

        std::optional<SDynamicRenderInstanceData::THandle> SetRenderableDynamic(const SBatchData::THandle& batch);

        void RemoveRenderableStatic();

        void RemoveRenderableDynamic();

        void PlayAnimation(types::usize index);
    };
}