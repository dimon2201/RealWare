#pragma once

#include "math.hpp"
#include "input_backend_window.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;

    class cInputWindow
    {
        friend class cInput;

        cContext* _context = nullptr;
        mutable sInputBackendWindow _backendWindow = {};

    public:
        enum class eRunState
        {
            OPENED = 1,
            CLOSED
        };

        explicit cInputWindow(cContext* context, const sInputBackendWindow& backendWindow);
        ~cInputWindow() = default;

        types::boolean IsWindowFocused() const;

        const cVector2& GetSize() const;
        inline sInputBackendWindow& GetBackendWindow() const { return _backendWindow; }
    };
}