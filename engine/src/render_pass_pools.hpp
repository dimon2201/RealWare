// render_pass_pools.hpp

#pragma once

#include "object_pool.hpp"
#include "render_pass_geometry.hpp"
#include "render_pass_processing.hpp"

namespace triton
{
    class CRenderPassGeometryPool : public CObjectPool<XRenderPassGeometry>
    {
        TRITON_CLASS_NAME(CRenderPassGeometryPool)

    public:
        using CObjectPool<XRenderPassGeometry>::CObjectPool;
        ~CRenderPassGeometryPool() override = default;

        XRenderPassGeometry::TGPULayout ConvertToGpuLayout(const XRenderPassGeometry& object) override
        {
            XRenderPassGeometry::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };

    class CRenderPassProcessingPool : public CObjectPool<XRenderPassProcessing>
    {
        TRITON_CLASS_NAME(CRenderPassProcessingPool)

    public:
        using CObjectPool<XRenderPassProcessing>::CObjectPool;
        ~CRenderPassProcessingPool() override = default;

        XRenderPassProcessing::TGPULayout ConvertToGpuLayout(const XRenderPassProcessing& object) override
        {
            XRenderPassProcessing::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}