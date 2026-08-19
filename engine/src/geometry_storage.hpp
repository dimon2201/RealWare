// geometry_storage.hpp

#pragma once

#include <optional>
#include "object.hpp"
#include "geometry_view.hpp"
#include "vertex_buffer_format.hpp"
#include "gpu_buffer.hpp"
#include "types.hpp"

namespace triton
{
    class cEngine;
    class XDataBuffer;

    class XGeometryStorage : public iObject
    {
        TRITON_OBJECT(XGeometryStorage)

        XGPUBuffer::THandle _staticVertexBuffer;
        XGPUBuffer::THandle _skinnedVertexBuffer;
        XGPUBuffer::THandle _staticIndexBuffer;
        XGPUBuffer::THandle _skinnedIndexBuffer;
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
            types::usize inputVertexCount,
            const types::u8* indices,
            types::usize inputIndexCount
        );

        inline XGPUBuffer::THandle GetStaticVertexBuffer() const
        {
            return _staticVertexBuffer;
        }

        inline XGPUBuffer::THandle GetSkinnedVertexBuffer() const
        {
            return _skinnedVertexBuffer;
        }

        inline XGPUBuffer::THandle GetStaticIndexBuffer() const
        {
            return _staticIndexBuffer;
        }

        inline XGPUBuffer::THandle GetSkinnedIndexBuffer() const
        {
            return _skinnedIndexBuffer;
        }
    };
}