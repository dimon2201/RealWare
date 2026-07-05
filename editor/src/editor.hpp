// editor.hpp

#pragma once

namespace triton
{
	class cContext;
}

namespace triton::editor
{
	void Init(cContext* context);
	void Free(cContext* context);
}