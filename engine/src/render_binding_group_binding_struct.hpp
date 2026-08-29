// render_binding_group_binding_struct.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	enum class EBindingGroupBindingType : types::dword
	{
		Unknown,
		StorageBuffer,
		TextureSampler
	};

	struct SBindingGroupBinding final
	{
		types::usize index = 0;
		EBindingGroupBindingType type = EBindingGroupBindingType::Unknown;
		types::dword shaderStageMask = 0;
	};
}