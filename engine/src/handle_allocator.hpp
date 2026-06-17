// handle_allocator.hpp

#pragma once

#include "object.hpp"

namespace triton
{
	class cContext;
	template <typename T>
	class cStack;

	class SFreeSlotValue : public cStackValue
	{
	public:
		SFreeSlotValue() = default;
		SFreeSlotValue(types::usize value) : _value(value) {}

		types::usize _value = 0;
	};

	template <typename TSlot, typename THandle, typename TDataStructure>
	class XHandleAllocator : public iObject
	{
		TRITON_OBJECT(XHandleAllocator)

		cStack<TSlot>* _slots = nullptr;
		cStack<SFreeSlotValue>* _freeSlots = nullptr;
		TDataStructure* _objects = nullptr;

	public:
		explicit XHandleAllocator(cContext* context) : iObject(context) {}
		virtual ~XHandleAllocator() override = default;

		void Initialize()
		{
			const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();
			sChunkAllocatorDescriptor cad = {};
			cad.chunkByteSize = caps->hashTableChunkByteSize;
			cad.maxChunkCount = caps->hashTableMaxChunkCount;
			cad.hashTableSize = caps->hashTableSize;
			if (!_slots)
				_slots = _context->Create<cStack<TSlot>>(_context, cad);
			if (!_freeSlots)
				_freeSlots = _context->Create<cStack<SFreeSlotValue>>(_context, cad);
			if (!_objects)
				_objects = _context->Create<cStack<TObject>>(_context, cad);
		}

		void Free()
		{
			if (_objects)
				_context->Destroy<cStack<TObject>>(_objects);
			if (_freeSlots)
				_context->Destroy<cStack<SFreeSlotValue>>(_freeSlots);
			if (_slots)
				_context->Destroy<cStack<TSlot>>(_slots);
		}

		template <typename... Args>
		THandle Create(Args&&... args)
		{
			types::usize arrayIndex;
			SFreeSlotValue slotValue;
			types::usize slotIndex;
			types::usize generation;
			if (_freeSlots->IsEmpty())
			{
				arrayIndex = _objects->GetSize();
				slotIndex = _slots->GetSize();
				generation = 0;

				_objects->Push(_context, std::forward<Args>(args)...);
				TSlot slot = {};
				slot._arrayIndex = arrayIndex;
				slot._generation = generation;
				_slots->Push(std::move(slot));
			}
			else
			{
				slotValue = *_freeSlots->Top();
				_freeSlots->Pop();
				slotIndex = slotValue._value;

				TSlot slot = *_slots->At(slotIndex);
				arrayIndex = slot._arrayIndex;
				generation = slot._generation;

				_objects->Recreate(arrayIndex, _context, std::forward<Args>(args)...);
			}

			THandle handle = {};
			handle._slotIndex = slotIndex;
			handle._generation = generation;

			return handle;
		}

		TObject* Get(const THandle& handle)
		{
			TSlot* slot = _slots->At(handle._slotIndex);
			if (handle._generation != slot->_generation)
				return nullptr;

			return _objects->At(slot->_arrayIndex);
		}

		void Destroy(const THandle& handle)
		{
			TSlot* slot = _slots->At(handle._slotIndex);
			slot->_generation += 1;
			_freeSlots->Push(SFreeSlotValue(handle._slotIndex));
		}
	};
}