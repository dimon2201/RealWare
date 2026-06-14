// ecs_subsystem.hpp

#pragma once

#include "object.hpp"
#include "entity.hpp"

namespace triton::ecs
{
	class cContext;
	class cScene;

	class XECSSubsystem : public iObject
	{
		TRITON_OBJECT(XECSSubsystem)

	public:
		explicit XECSSubsystem(cContext* context) : iObject(context) {}
		virtual ~XECSSubsystem() override = default;

		void Initialize();
		void Shutdown();
		cScene* CreateScene();
		void DestroyScene(cScene* scene);
	};
}