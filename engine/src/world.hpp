// world.hpp

#pragma once

#include "subsystem.hpp"

namespace triton
{
	class cContext;

	class UWorld : public CSubsystem
	{
		TRITON_CLASS_NAME(UWorld)

	public:
		explicit UWorld(cContext* context) : CSubsystem(context) {}
		
		virtual ~UWorld() = default;
	};
}