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
#include "graphics_context_backend.hpp"
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
        iGraphicsPipelineBackend* _pipeline = nullptr;
        iGraphicsContextBackend* _context = nullptr;
        iGraphicsDrawcallBackend* _drawcall = nullptr;

    public:
        explicit cGraphicsBackendFacade(
            cContext* context,
            iGraphicsResourceBackend* resourceBackend,
            iGraphicsPipelineBackend* pipelineBackend,
            iGraphicsContextBackend* contextBackend,
            iGraphicsDrawcallBackend* drawcallBackend
        );
        virtual ~cGraphicsBackendFacade() override = default;

        inline iGraphicsResourceBackend* GetResourceBackend() const { return _resource; }
        inline iGraphicsPipelineBackend* GetPipelineBackend() const { return _pipeline; }
        inline iGraphicsContextBackend* GetContextBackend() const { return _context; }
        inline iGraphicsDrawcallBackend* GetDrawcallBackend() const { return _drawcall; }
    };
}