// world.hpp

#pragma once

#include "subsystem.hpp"
#include "world_object.hpp"

namespace triton
{
	class UWorld : public CSubsystem
	{
		TRITON_CLASS_NAME(UWorld)

	public:
		explicit UWorld(cContext* context) : CSubsystem(context) {}
		
		~UWorld() override = default;
	};
}