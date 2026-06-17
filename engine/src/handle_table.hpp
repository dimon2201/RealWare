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

		cStack<TSlot>* _slots = nullptr;
		cStack<types::usize>* _freeSlots = nullptr;
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
			if (!_slots)
				_slots = _context->Create<cStack<TSlot>>(_context, cad);
			if (!_freeSlots)
				_freeSlots = _context->Create<cStack<types::usize>>(_context, cad);
			if (!_objects)
				_objects = _context->Create<cStack<TObject>>(_context, cad);
		}

		void Free()
		{
			if (_objects)
				_context->Destroy<cStack<TObject>>(_objects);
			if (_freeSlots)
				_context->Destroy<cStack<types::usize>>(_freeSlots);
			if (_slots)
				_context->Destroy<cStack<TSlot>>(_slots);
		}

		template <typename... Args>
		THandle Create(Args&&... args)
		{
			types::usize arrayIndex;
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
				slotIndex = *_freeSlots->Top();
				_freeSlots->Pop();

				TSlot slot = _slots->At(slotIndex);
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
			_freeSlots->Push(handle._slotIndex);
		}
	};
}