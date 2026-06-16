// handle_table.hpp

#pragma once

#include "object.hpp"

namespace triton
{
	class cContext;
	template <typename T>
	class cStack;

	template <typename TSlot, typename THandle, typename TObject>
	class XHandleTable : public iObject
	{
		TRITON_OBJECT(XHandleTable)

		cStack<TSlot>* _objectGenerationSlots = nullptr;
		cStack<TObject>* _objects = nullptr;

	public:
		explicit XHandleTable(cContext* context) : iObject(context) {}
		virtual ~XHandleTable() override = default;

		void Initialize()
		{
			const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();
			sChunkAllocatorDescriptor cad = {};
			cad.chunkByteSize = caps->hashTableChunkByteSize;
			cad.maxChunkCount = caps->hashTableMaxChunkCount;
			cad.hashTableSize = caps->hashTableSize;
			if (!_objectGenerationSlots)
				_objectGenerationSlots = _context->Create<cStack<TSlot>>(_context, cad);
			if (!_objects)
				_objects = _context->Create<cStack<TObject>>(_context, cad);
		}

		void Free()
		{
			if (_objects)
				_context->Destroy<cStack<TObject>>(_objects);
			if (_objectGenerationSlots)
				_context->Destroy<cStack<TSlot>>(_objectGenerationSlots);
		}

		THandle Create(const std::string& name)
		{
			types::usize arrayIndex = _objects->GetSize();
			types::usize slotIndex = _objectGenerationSlots->GetSize();
			types::usize generation = 0;

			TObject* object = _objects->Push(_context, name);
			TSlot slot = {};
			slot._arrayIndex = arrayIndex;
			slot._generation = generation;
			_objectGenerationSlots->Push(std::move(slot));

			THandle handle = {};
			handle._slotIndex = slotIndex;
			handle._generation = generation;

			return handle;
		}

		TObject* Get(const THandle& handle)
		{
			TSlot* slot = _objectGenerationSlots->At(handle._slotIndex);
			if (handle._generation != slot->_generation)
				return nullptr;

			return _objects->At(slot->_arrayIndex);
		}

		void Destroy(const THandle& handle)
		{
			TSlot* slot = _objectGenerationSlots->At(handle._slotIndex);
			_objects->Erase(slot->_arrayIndex);
			slot->_generation += 1;
		}
	};
}