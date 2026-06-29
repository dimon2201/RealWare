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

		template <typename... Args>
		inline THandle Create(Args&&... args)
		{
			return _objects->Create(std::forward<Args>(args)...);
		}

		inline THandle Create(TObject&& object)
		{
			return _objects->Create(std::move(object));
		}

		inline TObject& Get(const THandle& handle)
		{
			return *_objects->Get(handle);
		}

		inline void Destroy(const THandle& handle)
		{
			_objects->Destroy(handle);
		}
	};

	#define TRITON_SUBSYSTEM using ISubsystem::ISubsystem;
}