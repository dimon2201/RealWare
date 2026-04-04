// application.cpp

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <iostream>
#include "application.hpp"
#include "engine.hpp"
#include "graphics.hpp"
#include "context.hpp"
#include "font_manager.hpp"
#include "filesystem_manager.hpp"
#include "physics_manager.hpp"
#include "gameobject_manager.hpp"
#include "texture_manager.hpp"
#include "memory_pool.hpp"
#include "event_manager.hpp"
#include "thread_manager.hpp"
#include "input.hpp"
#include "time.hpp"
#include "log.hpp"

using namespace types;

triton::iApplication::iApplication(cContext* context, const sCapabilities* caps) : iObject(context), _caps(caps)
{
    _engine = new cEngine(_context, this);
}

triton::iApplication::~iApplication()
{
    delete _engine;
}

void triton::iApplication::Run()
{
    _engine->Run();
}