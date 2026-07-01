// mesh_backend_assimp.hpp

#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include "mesh_backend.hpp"

namespace triton
{
    class CMeshBackendAssimp final : public IMeshBackend
    {
        TRITON_OBJECT(CMeshBackendAssimp)

    public:
        explicit CMeshBackendAssimp(cContext* context) : IMeshBackend(context) {}
        ~CMeshBackendAssimp() override = default;

        SMeshBackendResource CreateMesh(EMeshFormat& format, const std::string& filePath) override final;
        void DestroyMesh(SMeshBackendResource& sound) override final;
    };
}