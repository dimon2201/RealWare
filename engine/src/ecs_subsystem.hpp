// ecs_subsystem.hpp

#pragma once

#include "object.hpp"
#include "scene_handle.hpp"

namespace triton
{
	class cContext;
	class cScene;
	template <typename T>
	class cStack;
}

namespace triton::ecs
{
	class XECSSubsystem : public iObject
	{
		TRITON_OBJECT(XECSSubsystem)

		cStack<SSceneSlot>* _sceneGenerationSlots = nullptr;
		cStack<cScene>* _scenes = nullptr;

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