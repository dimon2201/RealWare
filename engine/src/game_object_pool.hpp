// game_object_pool.hpp

#pragma once

#include "object_pool_base.hpp"

namespace triton
{
    class XGameObjectPool : public XObjectPoolBase<XGameObject>
    {
    public:
        using XObjectPoolBase<XGameObject>::XObjectPoolBase;
        ~XGameObjectPool() override = default;

        XGameObject::TGPULayout ConvertToGpuLayout(const XGameObject& object) override
        {
            XGameObject::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}