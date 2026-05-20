// render_frame.hpp

#pragma once

#include <queue>
#include <optional>
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

	class alignas(64) CRenderFrame final
	{
		cInputWindow* _window = nullptr;
		std::queue<SRenderCommand> _commands;

	public:
		explicit CRenderFrame(cInputWindow* window) : _window(window) {}

		void Reset(cInputWindow* window = nullptr);
		void PushCommand(const SRenderCommand& command);
		std::optional<SRenderCommand> Pop();

		inline cInputWindow* GetWindow() const
		{
			return _window;
		}
	};
}