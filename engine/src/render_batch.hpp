// render_batch.hpp

#pragma once

#include <optional>
#include "object.hpp"
#include "geometry_view.hpp"
#include "render_instance.hpp"
#include "handle.hpp"
#include "types.hpp"

namespace triton
{
	class cContext;
	class XInstanceBuffer;
	class SInstanceBufferHandle;
	template <typename TSlot, typename THandle, typename TDataStructure, typename TObject>
	class XHandleAllocator;
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
		SRenderInstance* Get(SInstanceBufferHandle& handle);
		void Remove(SInstanceBufferHandle& handle);
	};
}