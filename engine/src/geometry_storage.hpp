// geometry_storage.hpp

#pragma once

#include <optional>
#include "object.hpp"
#include "geometry_view.hpp"
#include "vertex_buffer_format.hpp"
#include "gpu_buffer.hpp"
#include "subsystem.hpp"
#include "types.hpp"

namespace triton
{
    class cEngine;
    class XDataBuffer;
    class cContext;

    class CGeometryStorage final : public CSubsystem
    {
        TRITON_CLASS_NAME(CGeometryStorage)

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
        explicit CGeometryStorage(cContext* context);
        ~CGeometryStorage();

        std::optional<triton::SGeometryView> Create(
            EVertexBufferFormat format,
            const types::u8* vertices,
            types::usize inputVertexCount,
            const types::u8* indices,
            types::usize inputIndexCount
        );

        inline const XGPUBuffer::THandle& GetStaticVertexBuffer() const
        {
            return _staticVertexBuffer;
        }

        inline const XGPUBuffer::THandle& GetSkinnedVertexBuffer() const
        {
            return _skinnedVertexBuffer;
        }

        inline const XGPUBuffer::THandle& GetStaticIndexBuffer() const
        {
            return _staticIndexBuffer;
        }

        inline const XGPUBuffer::THandle& GetSkinnedIndexBuffer() const
        {
            return _skinnedIndexBuffer;
        }
    };
}