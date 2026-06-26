// time.cpp

#include "time.hpp"

using namespace types;

triton::cTime::cTime(cContext* context) : iObject(context) {}

void triton::cTime::BeginFrame()
{
	_timepointLast = std::chrono::steady_clock::now();;
}

void triton::cTime::Update()
{
	const auto currentTime = std::chrono::steady_clock::now();;
	std::chrono::duration<f32> elapsed = currentTime - _timepointLast;
	_deltaTime = elapsed.count();
	_timepointLast = currentTime;
}

void triton::cTime::EndFrame()
{
}