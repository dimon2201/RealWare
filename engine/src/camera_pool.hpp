// camera_pool.hpp

#pragma once

#include "object_pool_base.hpp"
#include "camera.hpp"

namespace triton
{
    class XCameraPool : public XObjectPoolBase<XCamera>
    {
    public:
        using XObjectPoolBase<XCamera>::XObjectPoolBase;
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