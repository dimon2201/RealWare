#pragma once

#include "math.hpp"
#include "input_backend_window.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;

    class cInputWindow : public iObject
    {
        TRITON_OBJECT(cInputWindow)

        friend class cInput;

        mutable sInputBackendWindow _backendWindow = {};

    public:
        enum class eRunState
        {
            OPENED = 1,
            CLOSED
        };

        explicit cInputWindow(cContext* context, const sInputBackendWindow& backendWindow);
        virtual ~cInputWindow() override final = default;

        void SwapBuffers();
        types::boolean IsWindowFocused() const;
        eRunState GetRunState();

        const cVector2& GetSize() const;
        inline sInputBackendWindow& GetBackendWindow() const { return _backendWindow; }
    };
}