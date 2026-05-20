// thread_guard.cpp

#include "thread_guard.hpp"

std::atomic<std::thread::id> triton::CThreadGuard::_mainThreadId;
std::atomic<std::thread::id> triton::CThreadGuard::_renderThreadId;