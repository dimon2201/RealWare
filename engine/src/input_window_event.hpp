// input_window_event.hpp

#pragma once

#include "input_window_event_enum.hpp"

namespace triton
{
    struct SWindowEvent
    {
        EWindowEvent type = EWindowEvent::None;
        types::s32 argA = 0;
        types::s32 argB = 0;
        types::s32 argC = 0;
        types::s32 argD = 0;
    };
}