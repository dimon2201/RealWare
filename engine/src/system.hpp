// system.hpp

#pragma once

#include "object.hpp"
#include "scene.hpp"

namespace triton
{
	class cContext;
	template <typename TValue>
	class cStack;
}

namespace triton::ecs
{
	class cSystem : public triton::iObject
	{
		TRITON_OBJECT(cSystem)

		triton::cStack<ecs::cScene>* _scenes;

	public:
		explicit cSystem(triton::cContext* context);
		virtual ~cSystem() override = default;

		virtual void OnFrameUpdate() = 0;
	};
}