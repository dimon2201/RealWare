// mesh_backend.hpp

#pragma once

#include <string>
#include "backend.hpp"
#include "mesh_formats.hpp"

namespace triton
{
    struct SMeshBackendResource
    {
    };

    class IMeshBackend : public iBackend
    {
        TRITON_OBJECT(IMeshBackend)

    public:
        explicit IMeshBackend(cContext* context) : iBackend(context) {}
        ~IMeshBackend() override = default;

        virtual SMeshBackendResource CreateMesh(EMeshFormat& format, const std::string& filePath) = 0;
        virtual void DestroyMesh(SMeshBackendResource& mesh) = 0;
    };
}