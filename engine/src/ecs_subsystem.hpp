// ecs_subsystem.hpp

#pragma once

#include "object.hpp"
#include "scene_handle.hpp"
#include "scene.hpp"
#include "handles.hpp"

namespace triton
{
	class cContext;
	template <typename TValue>
	class XDynamicArray;
	template <typename TSlot, typename THandle, typename TDataStructure, typename TObject>
	class XHandleAllocator;
}

namespace triton::ecs
{
	class XECSSubsystem : public iObject
	{
		TRITON_OBJECT(XECSSubsystem)

		XHandleAllocator<SSceneSlot, cScene, HScene, XDynamicArray<cScene>>* _handleAllocator = nullptr;

	public:
		explicit XECSSubsystem(cContext* context) : iObject(context) {}
		virtual ~XECSSubsystem() override = default;

		void Initialize();
		void Shutdown();
		HScene CreateScene(const std::string& name);
		cScene& GetScene(const HScene& handle);
		void DestroyScene(const HScene& handle);
	};
}