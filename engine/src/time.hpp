// time.hpp

#pragma once

#include <chrono>
#include <string>
#include <typeinfo>
#include "log.hpp"
#include "subsystem.hpp"
#include "types.hpp"

namespace triton
{
	class cContext;

	class cTime : public CSubsystem
	{
		TRITON_CLASS_NAME(cTime)

		std::chrono::steady_clock::time_point _timepointLast;
		types::f32 _deltaTime = 0.0f;

	public:
		explicit cTime(cContext* context);
		~cTime() = default;

		void BeginFrame();
		void Update();
		void EndFrame();

		inline types::f32 GetDeltaTime() const { return _deltaTime; }
	};
}