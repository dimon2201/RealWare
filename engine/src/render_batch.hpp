// render_batch.hpp

#pragma once

#include <optional>
#include "object.hpp"
#include "geometry_view.hpp"
#include "render_instance.hpp"
#include "handle.hpp"
#include "handle_allocator.hpp"
#include "types.hpp"

namespace triton
{
	class cContext;
	class XInstanceBuffer;
	class SInstanceBufferSlot;
	class SInstanceBufferHandle;
	template <typename TValue>
	class XLinearArray;

	struct SRenderBatchSlot final : public SSlot {};

	struct SRenderBatchHandle final : public SHandle {};

	class XRenderBatch final : public iObject
	{
		TRITON_OBJECT(XRenderBatch)

		SGeometryView _geometry = {};
		XHandleAllocator<SInstanceBufferSlot, SInstanceBufferHandle, XLinearArray<SRenderInstance>, SRenderInstance>* _staticInstances = nullptr; // TODO: rename SInstanceBufferHandle to SRenderInstanceHandle
		XHandleAllocator<SInstanceBufferSlot, SInstanceBufferHandle, XLinearArray<SRenderInstance>, SRenderInstance>* _dynamicInstances = nullptr;
		types::usize _staticOffset = 0;
		types::usize _dynamicOffset = 0;

	public:
		explicit XRenderBatch(cContext* context, const SGeometryView& geometry);
		~XRenderBatch();

		std::optional<triton::SInstanceBufferHandle> Add(const SRenderBatchHandle& batch, SRenderInstance::EUsage usage, SRenderInstance& instance);
		SRenderInstance* Get(const SInstanceBufferHandle& handle);
		void Remove(const SInstanceBufferHandle& handle);
		types::usize Write(SRenderInstance::EUsage usage, types::usize offset, types::u8* destination);

		inline SGeometryView GetGeometry() const
		{
			return _geometry;
		}

		inline types::usize GetInstanceCount(SRenderInstance::EUsage usage) const
		{
			if (usage == SRenderInstance::EUsage::STATIC)
				return _staticInstances->GetSize();
			else if (usage == SRenderInstance::EUsage::DYNAMIC)
				return _dynamicInstances->GetSize();
		}

		inline types::usize GetInstanceOffset(SRenderInstance::EUsage usage) const
		{
			if (usage == SRenderInstance::EUsage::STATIC)
				return _staticOffset;
			else if (usage == SRenderInstance::EUsage::DYNAMIC)
				return _dynamicOffset;
		}
	};
}