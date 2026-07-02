// mesh_backend.hpp

#pragma once

#include <string>
#include <optional>
#include "backend.hpp"
#include "mesh_formats.hpp"

namespace triton
{
    struct SVertex;
    struct SMeshBackendResource;

    class IMeshBackend : public iBackend
    {
        TRITON_OBJECT(IMeshBackend)

    public:
        explicit IMeshBackend(cContext* context) : iBackend(context) {}
        ~IMeshBackend() override = default;

        virtual std::optional<SMeshBackendResource> CreateMesh(const std::string& filePath) = 0;
        virtual void DestroyMesh(SMeshBackendResource& mesh) = 0;
    };
}