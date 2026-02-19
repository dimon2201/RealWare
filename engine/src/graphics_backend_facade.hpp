// graphics_backend_facade.hpp

#pragma once

#include <vector>
#include <string>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "category.hpp"
#include "object.hpp"
#include "backend.hpp"
#include "math.hpp"
#include "gpu_resource.hpp"
#include "graphics_resource_backend.hpp"
#include "graphics_pipeline_backend.hpp"
#include "graphics_drawcall_backend.hpp"
#include "types.hpp"

namespace triton
{
    struct sInputBackendWindow;
    class cApplication;
    class cTextureAtlasTexture;

    class cGraphicsBackendFacade : public iObject
    {
        TRITON_OBJECT(cGraphicsBackendFacade)

        iGraphicsResourceBackend* _resource = nullptr;

    public:
        explicit cGraphicsBackendFacade(cContext* context, iGraphicsResourceBackend* resourceBackend);
        virtual ~cGraphicsBackendFacade() override = default;

        inline iGraphicsResourceBackend* GetResourceBackend() const { return _resource; }
    };
}