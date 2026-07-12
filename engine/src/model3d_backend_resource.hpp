// model3d_backend_resource.hpp

#pragma once

#include "handles.hpp"
#include "types.hpp"

namespace triton
{
    struct SVertex;

    struct SModel3DBackendResource
    {
        const SVertex* vertexData = nullptr;
        const types::u32* indexData = nullptr;
        types::usize vertexCount = 0;
        types::usize indexCount = 0;
        std::vector<HMaterial> materials = {};
        std::vector<HAnimation> animations = {};
    };
}