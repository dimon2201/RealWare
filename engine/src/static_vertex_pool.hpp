// static_vertex_pool.hpp

#pragma once

#include "object_pool_base.hpp"
#include "vertex.hpp"

namespace triton
{
    class XStaticVertexPool : public XObjectPoolBase<SStaticVertex>
    {
    public:
        using XObjectPoolBase<SStaticVertex>::XObjectPoolBase;
        ~XStaticVertexPool() override = default;

        SStaticVertex::TGPULayout ConvertToGpuLayout(const SStaticVertex& object) override
        {
            SStaticVertex::TGPULayout gpul;
            gpul.position = object.position;
            gpul.texcoord = object.texcoord;
            gpul.normal = object.normal;
            gpul.materialIndex = object.materialIndex;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}