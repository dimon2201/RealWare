#include <iostream>
#include "context.hpp"
#include "editor.hpp"

using namespace triton;
using namespace triton::editor;

int main()
{
	cContext* context = new cContext();
	XEditor* editor = new XEditor(context);
	editor->Initialize();
	editor->Run();
	editor->Shutdown();
	delete editor;
	delete context;

	return 0;
}