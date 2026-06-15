// event_manager.cpp

#pragma once

#include "application.hpp"
#include "context.hpp"
#include "gameobject_manager.hpp"
#include "engine.hpp"
#include "event_manager.hpp"
#include "data_buffer.hpp"

using namespace types;

triton::cEventHandler::cEventHandler(cContext* context, iObject* receiver, eEventType type, EventFunction&& function)
    : iObject(context), _receiver(receiver), _type(type), _function(std::make_shared<EventFunction>(std::move(function))) {}

void triton::cEventHandler::Invoke(iObject* self, XDataBuffer* data)
{
    _function->operator()(self, _context, data);
}

triton::cEventDispatcher::cEventDispatcher(cContext* context) : iObject(context)
{
    const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();

    sChunkAllocatorDescriptor cad = {};
    cad.chunkByteSize = caps->hashTableChunkByteSize;
    cad.maxChunkCount = caps->hashTableMaxChunkCount;
    cad.hashTableSize = caps->hashTableSize;

    _listeners = _context->Create<cHashTable<eEventType, cStack<cEventHandler>>>(_context, cad);
}

triton::cEventDispatcher::~cEventDispatcher()
{
    for (usize i = 0; i < _listeners->GetSize(); i++)
        _context->Destroy<cStack<cEventHandler>>(_listeners->Find(i));

    _context->Destroy<cHashTable<eEventType, cStack<cEventHandler>>>(_listeners);
}

void triton::cEventDispatcher::Subscribe(iObject* receiver, eEventType type, EventFunction&& function)
{
    cStack<cEventHandler>* listener = _listeners->Find(type);
    if (listener == nullptr)
    {
        const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();
            
        sChunkAllocatorDescriptor cad = {};
        cad.chunkByteSize = caps->hashTableChunkByteSize;
        cad.maxChunkCount = caps->hashTableMaxChunkCount;
        cad.hashTableSize = caps->hashTableSize;

        cStack<cEventHandler> listener(_context, cad);
        _listeners->Insert(type, std::move(listener));
    }

    listener->Push(_context, receiver, type, std::move(function));
}

void triton::cEventDispatcher::Unsubscribe(iObject* receiver, eEventType type)
{
    cStack<cEventHandler>* listener = _listeners->Find(type);
    if (listener == nullptr)
        return;

    for (usize i = 0; i < listener->GetSize(); i++)
    {
        const iObject* listenerReceiver = listener->At(i);
        if (listenerReceiver == receiver)
        {
            listener->Erase(i);

            return;
        }
    }
}

void triton::cEventDispatcher::Send(eEventType type)
{
    XDataBuffer data(_context, 64); // TODO: decide byte size for XDataBuffer

    Send(type, &data);
}

void triton::cEventDispatcher::Send(eEventType type, XDataBuffer* data)
{
    cStack<cEventHandler>* listener = _listeners->Find(type);
    if (listener == nullptr)
        return;

    for (usize i = 0; i < listener->GetSize(); i++)
    {
        cEventHandler* eventHandler = listener->At(i);
        eventHandler->Invoke(eventHandler->GetReceiver(), data);
    }
}