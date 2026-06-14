// ecs_subsystem.hpp

#pragma once

#include "object.hpp"
#include "handle.hpp"

namespace triton::ecs
{
	class cContext;
	class cScene;
	template <typename T>
	class cStack;

	class XECSSubsystem : public iObject
	{
		TRITON_OBJECT(XECSSubsystem)

		cStack<SSlot>* _sceneGenerationSlots = nullptr;
		cStack<cScene>* _scenes = nullptr;

	public:
		explicit XECSSubsystem(cContext* context) : iObject(context) {}
		virtual ~XECSSubsystem() override = default;

		void Initialize();
		void Shutdown();
		SHandle CreateScene();
		void DestroyScene(const SHandle& scene);
	};
}