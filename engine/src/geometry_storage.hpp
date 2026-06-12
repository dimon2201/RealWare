// geometry_storage.hpp

#pragma once

#include <optional>
#include "object.hpp"
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
        XDataBuffer* _vertexBufferCPU = nullptr;
        XDataBuffer* _indexBufferCPU = nullptr;
        types::usize _vertexBufferPointer = 0;
        types::usize _indexBufferPointer = 0;

        /*cBuffer* _opaqueInstanceBuffer = nullptr;
        cBuffer* _transparentInstanceBuffer = nullptr;
        cBuffer* _textInstanceBuffer = nullptr;
        cBuffer* _opaqueMaterialBuffer = nullptr;
        cBuffer* _transparentMaterialBuffer = nullptr;
        cBuffer* _textMaterialBuffer = nullptr;
        cBuffer* _lightBuffer = nullptr;
        cBuffer* _opaqueTextureAtlasTexturesBuffer = nullptr;
        cBuffer* _transparentTextureAtlasTexturesBuffer = nullptr;
        cBuffer* _textTextureAtlasTexturesBuffer = nullptr;
        XDataBuffer* _opaquePassInstance = nullptr;
        XDataBuffer* _transparentPassInstances = nullptr;
        XDataBuffer* _textPassInstance = nullptr;*/

    public:
        explicit XGeometryStorage(cContext* context) : iObject(context) {}
        virtual ~XGeometryStorage() override = default;

        void Initialize();
        void Free();
        std::optional<triton::SGeometryView> CreateGeometry(EGraphicsBufferFormat format, const types::u8* vertices, types::usize verticesByteSize, const types::u8* indices, types::usize indicesByteSize);
    };
}