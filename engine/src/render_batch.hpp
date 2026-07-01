// render_batch.hpp

#pragma once

#include <optional>
#include "object.hpp"
#include "geometry_view.hpp"
#include "render_instance.hpp"
#include "handle.hpp"
#include "handle_allocator.hpp"
#include "handles.hpp"
#include "batch_instance.hpp"
#include "material.hpp"
#include "types.hpp"

namespace triton
{
	class cContext;
	class XInstanceBuffer;
	class SInstanceBufferSlot;
	template <typename TValue>
	class XLinearArray;

	struct SRenderBatchSlot final : public SSlot {};

	class XRenderBatch final : public iObject
	{
		TRITON_OBJECT(XRenderBatch)

		SGeometryView _geometry = {};
		XHandleAllocator<SInstanceBufferSlot, HRenderInstance, XLinearArray<SRenderInstance>, SRenderInstance>* _staticInstances = nullptr; // TODO: rename HRenderInstance to SRenderInstanceHandle
		XHandleAllocator<SInstanceBufferSlot, HRenderInstance, XLinearArray<SRenderInstance>, SRenderInstance>* _dynamicInstances = nullptr;
		types::usize _staticOffset = 0;
		types::usize _dynamicOffset = 0;

	public:
		explicit XRenderBatch(cContext* context, const SGeometryView& geometry);
		~XRenderBatch();

		std::optional<triton::SBatchInstance> Add(const HBatch& batch, SRenderInstance::EUsage usage, SRenderInstance& instance);
		SRenderInstance* Get(const SBatchInstance& handle);
		void Set(const SBatchInstance& handle, const SRenderInstance& instance);
		void Remove(const SBatchInstance& handle);
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