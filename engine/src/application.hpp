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

    class IApplication
    {
    protected:
        cContext* _context = nullptr;
        const sCapabilities* _caps = nullptr;
        CEngine* _engine = nullptr;

    public:
        explicit IApplication(cContext* context, const sCapabilities* caps);
        virtual ~IApplication();

        virtual void Setup() = 0;
        virtual void Update() = 0;
        virtual void Stop() = 0;
        void Run();

        inline const sCapabilities* GetCapabilities() const
        {
            return _caps;
        }

        inline CEngine* GetEngine() const
        {
            return _engine;
        }
    };
}