// ecs_subsystem.hpp

#pragma once

#include "object.hpp"
#include "scene_handle.hpp"
#include "scene.hpp"

namespace triton
{
	class cContext;
	template <typename T>
	class cStack;
	template <typename X, typename Y, typename Z>
	class XHandleTable;
}

namespace triton::ecs
{
	class XECSSubsystem : public iObject
	{
		TRITON_OBJECT(XECSSubsystem)

		XHandleTable<SSceneSlot, SSceneHandle, cScene>* _handleTable = nullptr;

	public:
		explicit XECSSubsystem(cContext* context) : iObject(context) {}
		virtual ~XECSSubsystem() override = default;

		void Initialize();
		void Shutdown();
		SSceneHandle CreateScene(const std::string& name);
		cScene* GetScene(const SSceneHandle& handle);
		void DestroyScene(const SSceneHandle& handle);
	};
}