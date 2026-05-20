// render_frame.cpp

#include "render_frame.hpp"

void triton::CRenderFrame::Reset()
{
	_windowCount = 0;
	_commandCount = 0;
}

void triton::CRenderFrame::PushWindow(cInputWindow* window)
{
	_windows[_windowCount] = window;
	_windowCount += 1;
}

void triton::CRenderFrame::PushCommand(ERenderCommand command, SRenderCommandArgs&& args)
{
	_commands[_commandCount] = command;
	_commandArgs[_commandCount] = args;
	_commandCount += 1;
}