// application.hpp

#pragma once

#include <windows.h>
#include <chrono>
#include "object.hpp"
#include "capabilities.hpp"
#include "input_window.hpp"
#include "math.hpp"
#include "types.hpp"

struct GLFWwindow;

namespace triton
{
    class cPlatform;
    class cEngine;
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
        inline cPlatform* GetPlatform() const { return _platform; }
        inline cEngine* GetEngine() const { return _engine; }
        inline cStack<cInputWindow>* GetWindows() const { return _windows; }

    protected:
        const sCapabilities* _caps = nullptr;
        cPlatform* _platform = nullptr;
        cEngine* _engine = nullptr;
        cStack<cInputWindow>* _windows = nullptr;
    };
}