// camera_pool.hpp

#pragma once

#include "object_pool.hpp"
#include "camera.hpp"

namespace triton
{
    class CCameraPool : public CObjectPool<XCamera>
    {
        TRITON_CLASS_NAME(CCameraPool)

    public:
        using CObjectPool<XCamera>::CObjectPool;
        ~CCameraPool() override = default;

        XCamera::TGPULayout ConvertToGpuLayout(const XCamera& object) override
        {
            XCamera::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}