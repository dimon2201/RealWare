// camera_pool.hpp

#pragma once

#include "object_pool.hpp"
#include "camera.hpp"

namespace triton
{
    class XCameraPool : public XObjectPool<XCamera>
    {
        TRITON_OBJECT(XCameraPool)

    public:
        using XObjectPool<XCamera>::XObjectPool;
        ~XCameraPool() override = default;

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