// pipeline_buffers.hpp

#pragma once

#include "object.hpp"
#include "geometry.hpp"
#include "geometry_view.hpp"
#include "graphics_buffer_formats.hpp"
#include "types.hpp"

namespace triton
{
    class cEngine;
    class cBuffer;
    class XDataBuffer;

    class XGeometryStorage : public iObject
    {
        TRITON_OBJECT(XGeometryStorage)

        cBuffer* _vertexBuffer = nullptr;
        cBuffer* _indexBuffer = nullptr;
        cBuffer* _opaqueInstanceBuffer = nullptr;
        cBuffer* _transparentInstanceBuffer = nullptr;
        cBuffer* _textInstanceBuffer = nullptr;
        cBuffer* _opaqueMaterialBuffer = nullptr;
        cBuffer* _transparentMaterialBuffer = nullptr;
        cBuffer* _textMaterialBuffer = nullptr;
        cBuffer* _lightBuffer = nullptr;
        cBuffer* _opaqueTextureAtlasTexturesBuffer = nullptr;
        cBuffer* _transparentTextureAtlasTexturesBuffer = nullptr;
        cBuffer* _textTextureAtlasTexturesBuffer = nullptr;
        XDataBuffer* _vertexBufferCPU = nullptr;
        XDataBuffer* _indexBufferCPU = nullptr;

    public:
        explicit XGeometryStorage(cContext* context);
        virtual ~XGeometryStorage() override = default;

        void Initialize();
        void Release();
        SGeometryView CreateGeometry(EGraphicsBufferFormats format, const types::u8* vertices, types::usize verticesByteSize, const types::u8* indices, types::usize indicesByteSize);
        void RemoveGeometry(const SGeometryView& view);
    };
}