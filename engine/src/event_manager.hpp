// event_manager.hpp

#pragma once

#include <memory>
#include <functional>
#include "types.hpp"

namespace triton
{
    class XDataBuffer;
    class cContext;
    
    class cEventHandler
    {
        friend class mEvent;

        cContext* _context = nullptr;
        eEventType _type = eEventType::NONE;
        iObject* _receiver = nullptr;
        mutable std::shared_ptr<EventFunction> _function;

    public:
        explicit cEventHandler(iObject* receiver, eEventType type, EventFunction&& function);
        ~cEventHandler() = default;

        void Invoke(iObject* self, XDataBuffer* data);
        inline iObject* GetReceiver() const { return _receiver; }
        inline eEventType GetEventType() const { return _type; }
        inline std::shared_ptr<EventFunction> GetFunction() const { return _function; }
    };

    class cEventDispatcher
    {
        TRITON_CLASS_NAME(cEventDispatcher)

    public:
        explicit cEventDispatcher();
        ~cEventDispatcher();

        void Subscribe(iObject* receiver, eEventType type, EventFunction&& function);
        void Unsubscribe(iObject* receiver, eEventType type);
        void Send(eEventType type);
        void Send(eEventType type, XDataBuffer* data);
    };
}