// storage.hpp

#pragma once

#include "object.hpp"
#include "handle_allocator.hpp"
#include "handle.hpp"
#include "types.hpp"

namespace triton
{
	template <typename THandle, typename TObject, typename TAllocator>
	class IStorage : public iObject
	{
		TRITON_OBJECT(IStorage)

		TObject _nullObject = {};
		XHandleAllocator<SSlot, THandle, TAllocator, TObject>* _objects = nullptr;

	public:
		explicit IStorage(cContext* context) : iObject(context)
		{
			_objects = _context->Create<XHandleAllocator<SSlot, THandle, TAllocator, TObject>>(_context);
			_objects->Initialize();
		}

		virtual ~IStorage()
		{
			_objects->Free();
			_context->Destroy<XHandleAllocator<SSlot, THandle, TAllocator, TObject>>(_objects);
		}

		virtual void Update() = 0;

		inline THandle Create()
		{
			return _objects->Create();
		}

		inline TObject& Get(const THandle& handle)
		{
			TObject* p = _objects->Get(handle);
			if (p)
				return *p;
			else
				return _nullObject;
		}

		inline void Destroy(const THandle& handle)
		{
			_objects->Destroy(handle);
		}

		inline types::usize GetBufferIndex(const THandle& handle) const
		{
			return handle._indexInArray;
		}

		inline types::usize GetBufferSize() const
		{
			return _objects->GetSize();
		}
	};

	#define TRITON_STORAGE using IStorage::IStorage;
}