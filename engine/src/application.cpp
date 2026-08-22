// application.cpp

#include <iostream>
#include "application.hpp"
#include "engine.hpp"
#include "graphics.hpp"
#include "context.hpp"
#include "font_manager.hpp"
#include "filesystem_manager.hpp"
#include "physics_manager.hpp"
#include "memory_pool.hpp"
#include "event_manager.hpp"
#include "thread_subsystem.hpp"
#include "window.hpp"
#include "time.hpp"
#include "log.hpp"

using namespace types;

triton::IApplication::IApplication(cContext* context, const sCapabilities& caps) : _context(context), _caps(caps)
{
    _engine = new CEngine(_context, this);
}

triton::IApplication::~IApplication()
{
    delete _window;
    delete _engine;
}

void triton::IApplication::CreateWindow()
{
    _window = new CWindow(
        _context,
        _caps.window.title,
        _caps.window.size,
        _caps.window.fullscreen
    );
}

void triton::IApplication::Run()
{
    _engine->Run();
}