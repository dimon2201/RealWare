// geometry_storage.hpp

#pragma once

#include <optional>
#include "object.hpp"
#include "geometry_view.hpp"
#include "vertex_buffer_format.hpp"
#include "types.hpp"

namespace triton
{
    class cEngine;
    class cBuffer;
    class XDataBuffer;

    class XGeometryStorage : public iObject
    {
        TRITON_OBJECT(XGeometryStorage)

        cBuffer* _staticVertexBuffer = nullptr;
        cBuffer* _skinnedVertexBuffer = nullptr;
        cBuffer* _staticIndexBuffer = nullptr;
        cBuffer* _skinnedIndexBuffer = nullptr;
        XDataBuffer* _staticVertexBufferCPU = nullptr;
        XDataBuffer* _skinnedVertexBufferCPU = nullptr;
        XDataBuffer* _staticIndexBufferCPU = nullptr;
        XDataBuffer* _skinnedIndexBufferCPU = nullptr;
        types::usize _staticVertexBufferPointer = 0;
        types::usize _skinnedVertexBufferPointer = 0;
        types::usize _staticIndexBufferPointer = 0;
        types::usize _skinnedIndexBufferPointer = 0;

    public:
        explicit XGeometryStorage(cContext* context);
        virtual ~XGeometryStorage() override;

        std::optional<triton::SGeometryView> Create(
            EVertexBufferFormat format,
            const types::u8* vertices,
            types::usize verticesByteSize,
            const types::u8* indices,
            types::usize indicesByteSize
        );

        inline cBuffer* GetStaticVertexBuffer() const
        {
            return _staticVertexBuffer;
        }

        inline cBuffer* GetSkinnedVertexBuffer() const
        {
            return _skinnedVertexBuffer;
        }

        inline cBuffer* GetStaticIndexBuffer() const
        {
            return _staticIndexBuffer;
        }

        inline cBuffer* GetSkinnedIndexBuffer() const
        {
            return _skinnedIndexBuffer;
        }
    };
}