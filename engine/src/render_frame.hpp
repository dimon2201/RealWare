// render_frame.hpp

#pragma once

#include <queue>
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

	class SRenderCommand
	{
	public:
		ERenderCommand _command = ERenderCommand::NONE;
		SRenderCommandArgs _args = {};
	};

	class CRenderFrame final
	{
		std::queue<cInputWindow*> _windows;
		std::queue<SRenderCommand> _commands;

	public:
		void Reset();
		void PushWindow(cInputWindow* window);
		void PushCommand(const SRenderCommand& command);
	};
}