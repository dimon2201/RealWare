// platform.hpp

#pragma once

#include <unordered_map>
#include <memory>
#include "object.hpp"

namespace triton
{
	class cContext;
	class iBackend;
	class iInputBackend;
	class iGraphicsBackend;

	class cPlatform final : public iObject
	{
		TRITON_OBJECT(cPlatform)

		::std::unordered_map<ClassType, std::shared_ptr<iBackend>> _backends;

	public:
		enum class eInputBackend
		{
			NONE = 0,
			GLFW
		};

		enum class eGraphicsBackend
		{
			NONE = 0,
			OGL
		};

		explicit cPlatform(cContext* context, eInputBackend inputBackend, eGraphicsBackend graphicsBackend);
		virtual ~cPlatform() override final = default;

		template <typename T>
		inline void RegisterBackend(T* backend);

		template <typename T>
		inline T* GetBackend() const;
	};
}

template <typename T>
void triton::cPlatform::RegisterBackend(T* backend)
{
	// TODO: static_assert that T must inherit from iBackend
	const ClassType type = T::GetTypeStatic();
	const auto it = _backends.find(type);
	if (it == _backends.end())
		_backends.insert({ type, std::shared_ptr<iBackend>(backend) });
}

template <typename T>
T* triton::cPlatform::GetBackend() const
{
	const ClassType type = T::GetTypeStatic();
	const auto it = _backends.find(type);
	if (it != _backends.end())
		return (T*)it->second.get();
	else
		return nullptr;
}