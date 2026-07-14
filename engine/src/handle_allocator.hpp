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
		XDynamicArray<types::usize>* _reverseMap = nullptr;
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
			if (!_reverseMap)
				_reverseMap = _context->Create<XDynamicArray<types::usize>>(_context, cad);
			sChunkAllocatorDescriptor cadObjects = {};
			cadObjects.chunkByteSize = caps->handleAllocatorObjectCount * sizeof(TObject);
			if (!_objects)
				_objects = _context->Create<TDataStructure>(_context, cadObjects);
		}

		void Free()
		{
			if (_objects)
				_context->Destroy<TDataStructure>(_objects);
			if (_reverseMap)
				_context->Destroy<XDynamicArray<types::usize>>(_reverseMap);
			if (_freeSlots)
				_context->Destroy<XDynamicArray<types::usize>>(_freeSlots);
			if (_slots)
				_context->Destroy<XDynamicArray<TSlot>>(_slots);
		}

		template <typename... Args>
		THandle Create(Args&&... args)
		{
			types::usize arrayIndex;
			types::usize slotIndex;
			types::usize generation;
			
			arrayIndex = _objects->GetSize();
			THandle handle;

			if (_freeSlots->IsEmpty())
			{
				slotIndex = _slots->GetSize();
			
				_objects->Push(std::forward<Args>(args)...);
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
				slotIndex = *_freeSlots->Pop().data;

				_objects->Push(std::forward<Args>(args)...);
				_reverseMap->Push(slotIndex);

				_slots->At(slotIndex).data->_arrayIndex = arrayIndex;

				handle._indexInArray = arrayIndex;
				handle._generation = _slots->At(slotIndex).data->_generation;
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
			THandle handle;

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
				slotIndex = *_freeSlots->Pop().data;

				_objects->Push(std::move(object));
				_reverseMap->Push(slotIndex);

				_slots->At(slotIndex).data->_arrayIndex = arrayIndex;

				handle._generation = _slots->At(slotIndex).data->_generation;
			}

			handle._slotIndex = slotIndex;

			return handle;
		}

		TObject* Get(const THandle& handle)
		{
			TSlot* slot = _slots->At(handle._slotIndex).data;
			if (handle._generation != slot->_generation)
				return nullptr;

			return _objects->At(slot->_arrayIndex).data;
		}

		void Destroy(const THandle& handle)
		{
			if (_objects->IsEmpty())
				return;

			TSlot* slot = _slots->At(handle._slotIndex).data;
			if (slot->_generation != handle._generation)
				return;

			types::usize removeIndex = slot->_arrayIndex;
			types::usize lastIndex = _objects->GetSize() - 1;

			if (removeIndex != lastIndex)
			{
				// TODO: figure out what to do here
				//*(_objects->At(removeIndex).data) = std::move(*(_objects->At(lastIndex).data));
				// Call "_objects->At(removeIndex).data" destructor here
				new (_objects->At(removeIndex).data) TObject(std::move(*(_objects->At(lastIndex).data)));

				types::usize movedSlotIndex = *_reverseMap->At(lastIndex).data;
				*_reverseMap->At(removeIndex).data = movedSlotIndex;

				_slots->At(movedSlotIndex).data->_arrayIndex = removeIndex;
			}

			_objects->Pop();
			_reverseMap->Pop();
			_freeSlots->Push(handle._slotIndex);

			slot->_generation += 1;
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