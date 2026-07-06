// editor.hpp

#pragma once

#include "object.hpp"

namespace triton
{
	class cContext;
}

namespace triton::editor
{
	class XEditor : public iObject
	{
		TRITON_OBJECT(XEditor)

	public:
		explicit XEditor(cContext* context) : iObject(context) {}
		~XEditor() override = default;

		void Initialize();
		void Shutdown();
		void Run();
	};
}