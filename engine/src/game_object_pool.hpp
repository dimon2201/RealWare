// game_object_pool.hpp

#pragma once

#include "object_pool.hpp"
#include "game_object.hpp"

namespace triton
{
    class CGameObjectPool : public CObjectPool<XGameObject>
    {
        TRITON_CLASS_NAME(CGameObjectPool)

    public:
        using CObjectPool<XGameObject>::CObjectPool;
        ~CGameObjectPool() override = default;

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