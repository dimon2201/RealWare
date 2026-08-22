// window.hpp

#pragma once

#include <string>
#include "math.hpp"
#include "input_window_backend.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;

    class CWindow
    {
        cContext* _context = nullptr;
        SWindowBackend _backendWindow;

    public:
        explicit CWindow(
            cContext* context,
            const std::string& title,
            const cVector2& size,
            types::boolean fullscreen
        );
        ~CWindow();

        inline const cVector2& GetSize() const { return _backendWindow.size; }

        inline SWindowBackend& GetBackendWindow() { return _backendWindow; }
    };
}