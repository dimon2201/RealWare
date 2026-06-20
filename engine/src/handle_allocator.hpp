// handle_allocator.hpp

#pragma once

#include "object.hpp"
#include "buffer_view.hpp"

namespace triton
{
	class cContext;
	template <typename T>
	class cStack;

	class SValue : public cStackValue
	{
	public:
		SValue() = default;
		SValue(types::usize value) : _value(value) {}

		types::usize _value = 0;
	};

	// Concept to check if XHandleAllocator is derived from base stack interface IStack
	template<typename T>
	concept IsDerivedFromIStack =
		requires
	{
		typename T::ValueType;
	} &&
	std::derived_from<T, IStack<typename T::ValueType>>;

	template <typename TSlot, typename THandle, typename TDataStructure, typename TObject>
	class XHandleAllocator : public iObject
	{
		TRITON_OBJECT(XHandleAllocator)

		static_assert(IsDerivedFromIStack<TDataStructure>, "TDataStructure must inherit from IStack<TValue>");

		cStack<TSlot>* _slots = nullptr;
		cStack<SValue>* _freeSlots = nullptr;
		cStack<SValue>* _reverseMap = nullptr;
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
				_freeSlots = _context->Create<cStack<SValue>>(_context, cad);
			if (!_reverseMap)
				_reverseMap = _context->Create<cStack<SValue>>(_context, cad);
			if (!_objects)
				_objects = _context->Create<cStack<TObject>>(_context, cad);
		}

		void Free()
		{
			if (_objects)
				_context->Destroy<cStack<TObject>>(_objects);
			if (_reverseMap)
				_context->Destroy<cStack<SValue>>(_reverseMap);
			if (_freeSlots)
				_context->Destroy<cStack<SValue>>(_freeSlots);
			if (_slots)
				_context->Destroy<cStack<TSlot>>(_slots);
		}

		template <typename... Args>
		THandle Create(Args&&... args)
		{
			types::usize arrayIndex;
			types::usize slotIndex;
			types::usize generation;
			
			arrayIndex = _objects->GetSize();
			THandle handle = {};

			if (_freeSlots->IsEmpty())
			{
				slotIndex = _slots->GetSize();
			
				_objects->Push(_context, std::forward<Args>(args)...);
				_reverseMap->Push(slotIndex);

				TSlot slot = {};
				slot._arrayIndex = arrayIndex;
				slot._generation = 0;

				_slots->Push(std::move(slot));

				handle._indexInArray = arrayIndex;
				handle._generation = 0;
			}
			else
			{
				slotIndex = _freeSlots->Pop()._value;

				_objects->Push(_context, std::forward<Args>(args)...);
				_reverseMap->Push(slotIndex);

				_slots->At(slotIndex)->_arrayIndex = arrayIndex;

				handle._indexInArray = arrayIndex;
				handle._generation = _slots->At(slotIndex)->_generation;
			}
			
			handle._slotIndex = slotIndex;

			return handle;
		}

		THandle Create(TObject&& object)
		{
			types::usize arrayIndex;
			types::usize slotIndex;
			types::usize generation;

			arrayIndex = _objects->GetSize();
			THandle handle = {};

			if (_freeSlots->IsEmpty())
			{
				slotIndex = _slots->GetSize();

				_objects->Push(std::move(object));
				_reverseMap->Push(slotIndex);

				TSlot slot = {};
				slot._arrayIndex = arrayIndex;
				slot._generation = 0;

				_slots->Push(std::move(slot));

				handle._generation = 0;
			}
			else
			{
				slotIndex = _freeSlots->Pop()._value;

				_objects->Push(std::move(object));
				_reverseMap->Push(slotIndex);

				_slots->At(slotIndex)->_arrayIndex = arrayIndex;

				handle._generation = _slots->At(slotIndex)->_generation;
			}

			handle._slotIndex = slotIndex;

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
			if (_objects->IsEmpty())
				return;

			TSlot* slot = _slots->At(handle._slotIndex);
			if (slot->_generation != handle._generation)
				return;

			types::usize removeIndex = slot._arrayIndex;
			types::usize lastIndex = _objects->GetSize() - 1;

			if (removeIndex != lastIndex)
			{
				*_objects->At(removeIndex) = std::move(*_objects->At(lastIndex));

				types::usize movedSlotIndex = _reverseMap->At(lastIndex)->_value;
				_reverseMap->At(removeIndex)->_value = movedSlotIndex;

				_slots->At(movedSlotIndex)->_arrayIndex = removeIndex;
			}

			_objects->Pop();
			_reverseMap->Pop();
			_freeSlots->Push(handle._slotIndex);

			slot->_generation += 1;
		}

		inline SBufferView GetData() const
		{
			return _objects->GetData();
		}
	};
}