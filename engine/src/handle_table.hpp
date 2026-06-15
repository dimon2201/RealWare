// handle_table.hpp

#pragma once

#include "object.hpp"

namespace triton
{
	template <typename T>
	class cStack;

	template <typename TSlot, typename THandle, typename TObject>
	class XHandleTable : public iObject
	{
		TRITON_OBJECT(XHandleTable)

		cStack<TSlot>* _sceneGenerationSlots = nullptr;
		cStack<TObject>* _scenes = nullptr;

	public:
		THandle Create(const std::string& name);
		TObject* Get(const THandle& handle);
		void Destroy(const THandle& handle);
	};
}