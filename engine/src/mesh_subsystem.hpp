// mesh_subsystem.hpp

#pragma once

#include <optional>
#include "subsystem.hpp"
#include "handles.hpp"
#include "mesh_data.hpp"

namespace triton
{
    class XMeshSubsystem : public ISubsystem<HMesh, SMeshData, XLinearArray<SMeshData>>
    {
        TRITON_OBJECT(XMeshSubsystem)

    public:
        explicit XMeshSubsystem(cContext* context) : ISubsystem(context) {}
        ~XMeshSubsystem() override = default;

        std::optional<HMesh> CreateMesh(const std::string& filePath);
        void DestroyMesh(const HMesh& mesh);

        void Init() override {}
        void Free() override {}
        void Update() override {}
    };
}