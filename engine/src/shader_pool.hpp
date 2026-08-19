// shader_pool.hpp

#pragma once

#include "object_pool_base.hpp"

namespace triton
{
    class XShaderPool : public XObjectPoolBase<XShader>
    {
        TRITON_OBJECT(XShaderPool)

    public:
        using XObjectPoolBase<XShader>::XObjectPoolBase;
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