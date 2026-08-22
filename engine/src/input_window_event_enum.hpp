// input_window_event_enum.hpp

#pragma once

namespace triton
{
    enum class EWindowEvent
    {
        None,
        KeyDown,
        KeyUp,
        FocusGained,
        FocusLost,
        Resized,
        MouseMotion,
        MouseButtonDown,
        MouseButtonUp,
        Quit
    };
}