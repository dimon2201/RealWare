// graphics_pipeline_state.hpp

#pragma once

#include "category.hpp"

namespace triton
{
    class cTexture;
    class cBuffer;
    class cTextureAtlasTexture;
    class cShader;
    class cRenderTarget;

    struct sDepthMode
    {
        types::boolean useDepthTest = types::K_TRUE;
        types::boolean useDepthWrite = types::K_TRUE;
    };

    struct sBlendMode
    {
        enum class eBlendFactor
        {
            ZERO = 0,
            ONE = 1,
            SRC_COLOR = 2,
            INV_SRC_COLOR = 3,
            SRC_ALPHA = 4,
            INV_SRC_ALPHA = 5
        };

        types::usize factorCount = 0;
        eBlendFactor srcFactors[8] = { eBlendFactor::ZERO };
        eBlendFactor dstFactors[8] = { eBlendFactor::ZERO };
    };

    struct sViewport
    {
        cVector4 rect = cVector4(0.0f);
    };

    struct sRenderPassDescriptor
    {
        enum class eRenderPath
        {
            NONE = 0,
            OPAQUE_PATH,
            TRANSPARENT_PATH,
            TEXT_PATH,
            TRANSPARENT_COMPOSITE_PATH,
            QUAD_PATH
        };

        eCategory inputVertexFormat = eCategory::VERTEX_BUFFER_FORMAT_NONE;
        std::vector<cBuffer*> inputBuffers = {};
        std::vector<cTexture*> inputTextures = {};
        std::vector<std::string> inputTextureNames = {};
        std::vector<cTextureAtlasTexture*> inputTextureAtlasTextures = {};
        std::vector<std::string> inputTextureAtlasTextureNames = {};
        eRenderPath shaderRenderPath = eRenderPath::NONE;
        std::string shaderVertexPath = "";
        std::string shaderFragmentPath = "";
        std::string shaderVertexFunc = "";
        std::string shaderFragmentFunc = "";
        cShader* shaderBase = nullptr;
        sDepthMode depthMode = {};
        sBlendMode blendMode = {};
        sViewport viewport = {};
        cRenderTarget* renderTarget = nullptr;
    };
}