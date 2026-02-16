// event_manager.hpp

#pragma once

#include <memory>
#include <functional>
#include "object.hpp"
#include "hash_table.hpp"
#include "types.hpp"

namespace triton
{
    class cDataBuffer;
    class cContext;
    
    class cEventHandler : public iObject
    {
        TRITON_OBJECT(cEventHandler)

        friend class mEvent;

        eEventType _type = eEventType::NONE;
        iObject* _receiver = nullptr;
        mutable std::shared_ptr<EventFunction> _function;

    public:
        explicit cEventHandler(cContext* context, iObject* receiver, eEventType type, EventFunction&& function);
        virtual ~cEventHandler() override final = default;

        void Invoke(iObject* self, cDataBuffer* data);
        inline iObject* GetReceiver() const { return _receiver; }
        inline eEventType GetEventType() const { return _type; }
        inline std::shared_ptr<EventFunction> GetFunction() const { return _function; }
    };

    class cEventDispatcher : public iObject
    {
        TRITON_OBJECT(cEventDispatcher)

        cHashTable<eEventType, cStack<cEventHandler>>* _listeners = nullptr;

    public:
        explicit cEventDispatcher(cContext* context);
        virtual ~cEventDispatcher() override final;

        void Subscribe(iObject* receiver, eEventType type, EventFunction&& function);
        void Unsubscribe(iObject* receiver, eEventType type);
        void Send(eEventType type);
        void Send(eEventType type, cDataBuffer* data);
    };
}