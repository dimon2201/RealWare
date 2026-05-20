// render_frame.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	class cInputWindow;

	enum class ERenderCommand
	{
		NONE = 0,
		CLEAR
	};

	class SRenderCommandArgs
	{
	public:
		types::cpuword _argA = 0;
		types::cpuword _argB = 0;
		types::cpuword _argC = 0;
		types::cpuword _argD = 0;
	};

	class CRenderFrame final
	{
		types::usize _windowCount = 0;
		cInputWindow* _windows[8] = {};
		types::usize _commandCount = 0;
		ERenderCommand _commands[512] = {};
		SRenderCommandArgs _commandArgs[512] = {};

	public:
		void Reset();
		void PushWindow(cInputWindow* window);
		void PushCommand(ERenderCommand command, SRenderCommandArgs&& args);
	};
}