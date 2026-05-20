// render_frame.cpp

#include "render_frame.hpp"

void triton::CRenderFrame::Reset(cInputWindow* window)
{
	if (window != nullptr)
		_window = window;
	while (!_commands.empty())
		_commands.pop();
}

void triton::CRenderFrame::PushCommand(const SRenderCommand& command)
{
	_commands.push(command);
}

std::optional<triton::SRenderCommand> triton::CRenderFrame::Pop()
{
	if (_commands.empty())
		return std::nullopt;

	SRenderCommand command = _commands.front();
	_commands.pop();

	return command;
}