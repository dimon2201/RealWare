// subsystem.hpp

#pragma once

#include "object.hpp"
#include "handle_allocator.hpp"
#include "handle.hpp"
#include "types.hpp"

namespace triton
{
	template <typename THandle, typename TObject, typename TAllocator>
	class ISubsystem : public iObject
	{
		TRITON_OBJECT(ISubsystem)

		TObject _nullObject = {};
		CHandleAllocator<SSlot, TObject, THandle, TAllocator>* _objects = nullptr;

	public:
		virtual void Init() = 0;
		virtual void Free() = 0;
		virtual void Update() = 0;

		explicit ISubsystem(cContext* context) : iObject(context)
		{
			_objects = _context->Create<CHandleAllocator<SSlot, TObject, THandle, TAllocator>>(
				_context,
				4096,
				4096,
				65536 * 64,
				1024
			);
		}

		virtual ~ISubsystem()
		{
			_context->Destroy<CHandleAllocator<SSlot, TObject, THandle, TAllocator>>(_objects);
		}

		inline THandle Create()
		{
			return _objects->Create();
		}

		inline TObject& Get(const THandle& handle)
		{
			TObject& p = _objects->Get(handle);

			return p;
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

	#define TRITON_SUBSYSTEM using ISubsystem::ISubsystem;
}