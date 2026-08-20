// shader_pool.hpp

#pragma once

#include "object_pool.hpp"

namespace triton
{
    class XShaderPool : public XObjectPool<XShader>
    {
        TRITON_OBJECT(XShaderPool)

    public:
        using XObjectPool<XShader>::XObjectPool;
        ~XShaderPool() override = default;

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