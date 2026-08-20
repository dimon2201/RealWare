// model3d_pool.hpp

#pragma once

#include "object_pool.hpp"
#include "model3d.hpp"

namespace triton
{
    class CModel3DPool : public CObjectPool<XModel3D>
    {
        TRITON_CLASS_NAME(CModel3DPool)

    public:
        using CObjectPool<XModel3D>::CObjectPool;
        ~CModel3DPool() override = default;

        XModel3D::TGPULayout ConvertToGpuLayout(const XModel3D& object) override
        {
            XModel3D::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}