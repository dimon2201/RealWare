// shader_pool.hpp

#pragma once

#include "object_pool.hpp"

namespace triton
{
    class CShaderPool : public CObjectPool<XShader>
    {
        TRITON_CLASS_NAME(CShaderPool)

    public:
        using CObjectPool<XShader>::CObjectPool;
        ~CShaderPool() override = default;

        XShader::TGPULayout ConvertToGpuLayout(const XShader& object) override
        {
            XShader::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}