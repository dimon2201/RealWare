// mesh_backend_resource.hpp

#pragma once

#include "types.hpp"

namespace triton
{
    struct SVertex;

    struct SMeshBackendResource
    {
        const SVertex* vertexData = nullptr;
        const types::u32* indexData = nullptr;
        types::usize vertexCount = 0;
        types::usize indexCount = 0;
    };
}