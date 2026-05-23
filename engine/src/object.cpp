// object.cpp

#include "object.hpp"
#include "event_manager.hpp"
#include "context.hpp"

using namespace types;

triton::cTag triton::cIdentifier::Generate(const std::string& seed)
{
    static usize counter = 0;

    cTag tag;

    const std::string& idStr = seed + std::to_string(counter++);
    tag.CopyChars((const u8*)idStr.c_str(), idStr.size());

    return tag;
}

void triton::iObject::Subscribe(eEventType type, EventFunction&& function)
{
    cEventDispatcher* dispatcher = _context->GetSubsystem<cEventDispatcher>();
    dispatcher->Subscribe(this, type, std::move(function));
}

void triton::iObject::Unsubscribe(eEventType type)
{
    cEventDispatcher* dispatcher = _context->GetSubsystem<cEventDispatcher>();
    dispatcher->Unsubscribe(this, type);
}

void triton::iObject::Send(eEventType type)
{
    cEventDispatcher* dispatcher = _context->GetSubsystem<cEventDispatcher>();
    dispatcher->Send(type);
}

void triton::iObject::Send(eEventType type, XDataBuffer* data)
{
    cEventDispatcher* dispatcher = _context->GetSubsystem<cEventDispatcher>();
    dispatcher->Send(type, data);
}