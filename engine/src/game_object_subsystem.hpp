// game_object_subsystem.hpp

#pragma once

#include "object.hpp"
#include "storage.hpp"
#include "game_object.hpp"

namespace triton
{
    template <typename TValue>
    class XLinearArray;

    class XGameObjectSubsystem : public XStorage<SSlot, SGameObject, XLinearArray<SGameObject>>
    {
        TRITON_OBJECT(XGameObjectSubsystem)
        TRITON_STORAGE
    };
}