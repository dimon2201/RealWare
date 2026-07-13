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

		XHandleAllocator<SSlot, THandle, TAllocator, TObject>* _objects = nullptr;

	protected:
		inline THandle Create()
		{
			return _objects->Create();
		}

	public:
		virtual void Init() = 0;
		virtual void Free() = 0;
		virtual void Update() = 0;

		explicit ISubsystem(cContext* context) : iObject(context)
		{
			_objects = _context->Create<XHandleAllocator<SSlot, THandle, TAllocator, TObject>>(_context);
			_objects->Initialize();
		}

		virtual ~ISubsystem()
		{
			_objects->Free();
			_context->Destroy<XHandleAllocator<SSlot, THandle, TAllocator, TObject>>(_objects);
		}

		inline TObject& Get(const THandle& handle)
		{
			return *_objects->Get(handle);
		}

		inline void Destroy(const THandle& handle)
		{
			_objects->Destroy(handle);
		}

		inline types::usize GetBufferIndex(const THandle& handle) const
		{
			return handle._indexInArray;
		}
	};

	#define TRITON_SUBSYSTEM using ISubsystem::ISubsystem;
}