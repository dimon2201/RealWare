// application.hpp

#pragma once

#include <windows.h>
#include <chrono>
#include "object.hpp"
#include "capabilities.hpp"
#include "math.hpp"
#include "types.hpp"

namespace triton
{
    class cEngine;

    class IApplication : public iObject
    {
        TRITON_OBJECT(IApplication)

    protected:
        const sCapabilities* _caps = nullptr;
        cEngine* _engine = nullptr;

    public:
        explicit IApplication(cContext* context, const sCapabilities* caps);
        virtual ~IApplication() override;

        virtual void Setup() = 0;
        virtual void Stop() = 0;
        void Run();

        inline const sCapabilities* GetCapabilities() const
        {
            return _caps;
        }

        inline cEngine* GetEngine() const
        {
            return _engine;
        }
    };
}