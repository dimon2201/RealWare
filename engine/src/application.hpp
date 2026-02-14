// application.hpp

#pragma once

#include <windows.h>
#include <chrono>
#include "object.hpp"
#include "capabilities.hpp"
#include "math.hpp"
#include "types.hpp"

struct GLFWwindow;

namespace triton
{
    class cEngine;
    class cInputWindow;
    template <typename TValue>
    class cStack;

    class iApplication : public iObject
    {
        TRITON_OBJECT(iApplication)

    public:
        enum class eMouseButton
        {
            LEFT,
            RIGHT,
            MIDDLE
        };

        explicit iApplication(cContext* context, const sCapabilities* caps);
        virtual ~iApplication() override;

        virtual void Setup() = 0;
        virtual void Stop() = 0;

        void Run();

        inline const sCapabilities* GetCapabilities() const { return _caps; }
        inline cEngine* GetEngine() const { return _engine; }
        inline cStack<cInputWindow>* GetWindows() const { return _windows; }

    protected:
        const sCapabilities* _caps = nullptr;
        cEngine* _engine = nullptr;
        cStack<cInputWindow>* _windows = nullptr;
    };
}