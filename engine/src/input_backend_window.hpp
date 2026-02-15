#pragma once

#include "math.hpp"
#include "types.hpp"

namespace triton
{
    struct sInputBackendWindow
    {
        types::qword instance = 0;
        ::std::string title = "";
        cVector2 size = cVector2(0.0f);
        types::boolean fullscreen = types::K_FALSE;
    };
}