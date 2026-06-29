// game_object_subsystem.hpp

#pragma once

#include "object.hpp"
#include "storage.hpp"
#include "game_object.hpp"

namespace triton
{
    template <typename TValue>
    class XLinearArray;

    using HGameObject = SHandle;

    class XGameObjectSubsystem : public XStorage<HGameObject, SGameObject, XLinearArray<SGameObject>>
    {
        TRITON_OBJECT(XGameObjectSubsystem)
        TRITON_STORAGE
    };
}