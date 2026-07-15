// handle_allocator.hpp

#pragma once

#include "application.hpp"
#include "object.hpp"
#include "buffer_view.hpp"
#include "linear_array.hpp"

namespace triton
{
	class cContext;
	template <typename T>
	class XDynamicArray;

	// Concept to check if XHandleAllocator member data structure is derived from stack class XDynamicArray<TValue>
	/*template<typename T>
	concept IsDerivedFromIStack =
		requires
	{
		typename T::ValueType;
	} &&
	std::derived_from<T, XDynamicArray<typename T::ValueType>>;*/

	template <typename TSlot, typename THandle, typename TDataStructure, typename TObject>
	class XHandleAllocator : public iObject
	{
		TRITON_OBJECT(XHandleAllocator)

		//static_assert(IsDerivedFromIStack<TDataStructure>, "TDataStructure must inherit from XDynamicArray<TValue>");

		XDynamicArray<TSlot>* _slots = nullptr;
		XDynamicArray<types::usize>* _freeSlots = nullptr;
		XDynamicArray<types::usize>* _objectIndexToSlotIndex = nullptr;
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
				_slots = _context->Create<XDynamicArray<TSlot>>(_context, cad);
			if (!_freeSlots)
				_freeSlots = _context->Create<XDynamicArray<types::usize>>(_context, cad);
			if (!_objectIndexToSlotIndex)
				_objectIndexToSlotIndex = _context->Create<XDynamicArray<types::usize>>(_context, cad);
			sChunkAllocatorDescriptor cadObjects = {};
			cadObjects.chunkByteSize = caps->handleAllocatorObjectCount * sizeof(TObject);
			if (!_objects)
				_objects = _context->Create<TDataStructure>(_context, cadObjects);
		}

		void Free()
		{
			if (_objects)
				_context->Destroy<TDataStructure>(_objects);
			if (_objectIndexToSlotIndex)
				_context->Destroy<XDynamicArray<types::usize>>(_objectIndexToSlotIndex);
			if (_freeSlots)
				_context->Destroy<XDynamicArray<types::usize>>(_freeSlots);
			if (_slots)
				_context->Destroy<XDynamicArray<TSlot>>(_slots);
		}

		template <typename... Args>
		THandle Create(Args&&... args)
		{
			THandle handle;
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

		THandle Create(TObject&& object)
		{
			THandle handle;
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

		TObject* Get(const THandle& handle)
		{
			TSlot* slot = _slots->At(handle._slotIndex).data;
			if (!slot)
				return nullptr;
			if (handle._generation != slot->_generation)
				return nullptr;

			return _objects->At(slot->_arrayIndex).data;
		}

		void Destroy(const THandle& handle)
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
				_objects->At(removeObjectIndex).data->~TObject();
				new (_objects->At(removeObjectIndex).data)
					TObject(std::move(*(_objects->At(lastObjectIndex).data)));

				types::usize movedSlotIndex = *_objectIndexToSlotIndex->At(lastObjectIndex).data;
				*_objectIndexToSlotIndex->At(removeObjectIndex).data = movedSlotIndex;

				_slots->At(movedSlotIndex).data->_arrayIndex = removeObjectIndex;
			}

			_objects->Pop();
			_objectIndexToSlotIndex->Pop();
			_freeSlots->Push(freeSlotIndex);
		}

		inline SBufferView<TObject> GetData() const
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
	};
}