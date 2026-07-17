// handle_allocator.hpp

#pragma once

#include "application.hpp"
#include "object.hpp"
#include "buffer_view.hpp"
#include "linear_array.hpp"
#include "object_allocator.hpp"

namespace triton
{
	class cContext;
	template <typename T>
	class XDynamicArray;

	// Concept to check if CHandleAllocator member data structure is derived from stack class XDynamicArray<TValue>
	/*template<typename T>
	concept IsDerivedFromIStack =
		requires
	{
		typename T::ValueType;
	} &&
	std::derived_from<T, XDynamicArray<typename T::ValueType>>;*/

	template <typename TSlot, typename TCPUObject, typename TCPUObjectHandle, typename TCPUObjectAllocator>
	class CHandleAllocator
	{
		//static_assert(IsDerivedFromIStack<TDataStructure>, "TDataStructure must inherit from XDynamicArray<TValue>");

		XDynamicArray<TSlot>* _slots = nullptr;
		XDynamicArray<types::usize>* _freeSlots = nullptr;
		XDynamicArray<types::usize>* _objectIndexToSlotIndex = nullptr;
		TCPUObjectAllocator* _objects = nullptr;
		TCPUObject _nullObject = {};

	public:
		CHandleAllocator() = delete;

		explicit CHandleAllocator(
			cContext* context,
			types::usize maxHandleElementCount,
			types::usize hashTableElementSize,
			types::usize maxChunkCount,
			types::usize chunkByteSize
		)
		{
			sChunkAllocatorDescriptor cad;
			cad.chunkByteSize = chunkByteSize;
			cad.hashTableSize = hashTableElementSize;
			cad.maxChunkCount = maxChunkCount;
			_slots = CObjectAllocator::Create<XDynamicArray<TSlot>>(64, context, cad);
			_freeSlots = CObjectAllocator::Create<XDynamicArray<types::usize>>(64, context, cad);
			_objectIndexToSlotIndex = CObjectAllocator::Create<XDynamicArray<types::usize>>(64, context, cad);
			sChunkAllocatorDescriptor cadObjects = {};
			cadObjects.chunkByteSize = maxHandleElementCount * sizeof(TCPUObject);
			_objects = CObjectAllocator::Create<TCPUObjectAllocator>(64, context, cadObjects);
		}

		~CHandleAllocator()
		{
			CObjectAllocator::Destroy<TCPUObjectAllocator>(_objects);
			CObjectAllocator::Destroy<XDynamicArray<types::usize>>(_objectIndexToSlotIndex);
			CObjectAllocator::Destroy<XDynamicArray<types::usize>>(_freeSlots);
			CObjectAllocator::Destroy<XDynamicArray<TSlot>>(_slots);
		}

		template <typename... Args>
		TCPUObjectHandle Create(Args&&... args)
		{
			TCPUObjectHandle handle;
			types::usize slotIndex;
			types::usize arrayIndex;
			types::usize generation;

			if (_freeSlots->IsEmpty())
			{
				slotIndex = _slots->GetSize();
				arrayIndex = _objects->GetSize();
				_objects->Push(std::forward<Args>(args)...);
				_objectIndexToSlotIndex->Push(slotIndex);

				generation = 0;
				
				TSlot slot = {};
				slot._arrayIndex = arrayIndex;
				slot._generation = generation;
				_slots->Push(std::move(slot));

				handle._slotIndex = slotIndex;
				handle._indexInArray = arrayIndex;
				handle._generation = generation;
			}
			else
			{
				slotIndex = *_freeSlots->Top().data;
				arrayIndex = _objects->GetSize();
				_objects->Push(std::forward<Args>(args)...);
				_objectIndexToSlotIndex->Push(slotIndex);
			
				TSlot& slot = *_slots->At(slotIndex).data;
				slot._arrayIndex = arrayIndex;
				generation = slot._generation;

				handle._slotIndex = slotIndex;
				handle._indexInArray = arrayIndex;
				handle._generation = generation;

				_freeSlots->Pop();
			}
			
			return handle;
		}

		TCPUObjectHandle Create(TCPUObject&& object)
		{
			TCPUObjectHandle handle;
			types::usize slotIndex;
			types::usize arrayIndex;
			types::usize generation;

			if (_freeSlots->IsEmpty())
			{
				slotIndex = _slots->GetSize();
				arrayIndex = _objects->GetSize();
				_objects->Push(std::move(object));
				_objectIndexToSlotIndex->Push(slotIndex);

				generation = 0;

				TSlot slot = {};
				slot._arrayIndex = arrayIndex;
				slot._generation = generation;
				_slots->Push(std::move(slot));

				handle._slotIndex = slotIndex;
				handle._indexInArray = arrayIndex;
				handle._generation = generation;
			}
			else
			{
				slotIndex = *_freeSlots->Top().data;
				arrayIndex = _objects->GetSize();
				_objects->Push(std::move(object));
				_objectIndexToSlotIndex->Push(slotIndex);

				TSlot& slot = *_slots->At(slotIndex).data;
				slot._arrayIndex = arrayIndex;
				generation = slot._generation;

				handle._slotIndex = slotIndex;
				handle._indexInArray = arrayIndex;
				handle._generation = generation;

				_freeSlots->Pop();
			}

			return handle;
		}

		TCPUObject& Get(const TCPUObjectHandle& handle)
		{
			TSlot* slot = _slots->At(handle._slotIndex).data;
			if (!slot)
				return _nullObject;
			if (handle._generation != slot->_generation)
				return _nullObject;

			return *_objects->At(slot->_arrayIndex).data;
		}

		TCPUObjectHandle GetHandle(types::usize elementIndex)
		{
			const types::usize idx = *_objectIndexToSlotIndex->At(elementIndex).data;
			const TSlot& slot = *_slots->At(idx).data;

			TCPUObjectHandle handle;
			handle._slotIndex = idx;
			handle._indexInArray = slot._arrayIndex;
			handle._generation = slot._generation;

			return handle;
		}

		void Destroy(const TCPUObjectHandle& handle)
		{
			if (_objects->IsEmpty())
				return;

			TSlot& slot = *_slots->At(handle._slotIndex).data;
			if (slot._generation != handle._generation)
				return;
			slot._generation += 1;

			types::usize removeObjectIndex = slot._arrayIndex;
			types::usize lastObjectIndex = _objects->GetSize() - 1;
			types::usize freeSlotIndex = handle._slotIndex;

			if (removeObjectIndex != lastObjectIndex)
			{
				_objects->At(removeObjectIndex).data->~TCPUObject();
				new (_objects->At(removeObjectIndex).data)
					TCPUObject(std::move(*(_objects->At(lastObjectIndex).data)));

				types::usize movedSlotIndex = *_objectIndexToSlotIndex->At(lastObjectIndex).data;
				*_objectIndexToSlotIndex->At(removeObjectIndex).data = movedSlotIndex;

				_slots->At(movedSlotIndex).data->_arrayIndex = removeObjectIndex;
			}

			_objects->Pop();
			_objectIndexToSlotIndex->Pop();
			_freeSlots->Push(freeSlotIndex);
		}

		types::boolean Exists(const TCPUObjectHandle& handle) const
		{
			SSlot* slotData = _slots->At(handle._slotIndex).data;
			if (!slotData)
				return types::K_FALSE;

			TSlot& slot = *slotData;
			if (slot._generation != handle._generation)
				return types::K_FALSE;

			return types::K_TRUE;
		}

		inline const SBufferView<TCPUObject> GetData() const
		{
			return _objects->GetData();
		}

		inline types::usize GetSize() const
		{
			return _objects->GetSize();
		}

		inline types::usize GetByteSize() const
		{
			return _objects->GetByteSize();
		}

		inline types::s32 GetHandleBufferIndex(const TCPUObjectHandle& handle) const
		{
			if (handle.IsInvalid())
				return -1;
			else
				return handle._indexInArray;
		}
	};
}