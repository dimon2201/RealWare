// scene_handle.hpp

#pragma once

#include "handle.hpp"

namespace triton
{
	class SSceneSlot final : public SSlot, public cStackValue {};

	class SSceneHandle final : public SHandle {};
}