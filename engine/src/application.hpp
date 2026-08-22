// application.hpp

#pragma once

#include <chrono>
#include "object.hpp"
#include "capabilities.hpp"
#include "math.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;
    class CEngine;
    class CWindow;

    class IApplication
    {
        TRITON_CLASS_NAME(IApplication)

    protected:
        cContext* _context = nullptr;
        sCapabilities _caps;
        CWindow* _window = nullptr;
        CEngine* _engine = nullptr;

    public:
        explicit IApplication(cContext* context, const sCapabilities& caps);

        virtual ~IApplication();

        void CreateWindow();

        virtual void Setup() = 0;

        virtual void Update() = 0;

        virtual void Stop() = 0;

        void Run();

        inline const sCapabilities& GetCapabilities() const { return _caps; }

        inline CWindow* GetWindow() const { return _window; }

        inline CEngine* GetEngine() const { return _engine; }
    };
}