// render_frame.cpp

#include "render_frame.hpp"

void triton::CRenderFrame::Reset()
{
	while (!_windows.empty())
		_windows.pop();
	while (!_commands.empty())
		_commands.pop();
}

void triton::CRenderFrame::PushWindow(cInputWindow* window)
{
	_windows.push(window);
}

void triton::CRenderFrame::PushCommand(const SRenderCommand& command)
{
	_commands.push(command);
}