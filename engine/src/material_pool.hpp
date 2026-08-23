// material_pool.hpp

#pragma once

#include "material.hpp"
#include "object_pool.hpp"
#include "texture_pool.hpp"

namespace triton
{
    class CMaterialPool : public CObjectPool<XMaterial>
    {
        TRITON_CLASS_NAME(CMaterialPool)

    public:
        using CObjectPool<XMaterial>::CObjectPool;
        ~CMaterialPool() override = default;

        XMaterial::TGPULayout ConvertToGpuLayout(const XMaterial& object) override
        {
            XMaterial::TGPULayout gpul;
            gpul.diffuseColor = object.GetDiffuseColor();
            gpul.specularColor = object.GetSpecularColor();
            gpul.shininess = object.GetShininess();

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}