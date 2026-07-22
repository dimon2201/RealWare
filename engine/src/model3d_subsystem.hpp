// model3d_subsystem.hpp

#pragma once

#include <optional>
#include <filesystem>
#include "subsystem.hpp"
#include "handles.hpp"
#include "model3d_data.hpp"

namespace triton
{
    class XModel3DSubsystem : public ISubsystem<HModel3D, SModel3DData, XLinearArray<SModel3DData>>
    {
        TRITON_OBJECT(XModel3DSubsystem)

    public:
        explicit XModel3DSubsystem(cContext* context) : ISubsystem(context) {}
        ~XModel3DSubsystem() override = default;

        std::optional<HModel3D> CreateModelFromRaw(const std::filesystem::path& modelFilePath);
        std::optional<HModel3D> CreateModelFromAsset(const std::filesystem::path& assetFilePath);
        void DestroyModel(const HModel3D& model);

        void Init() override {}
        void Free() override {}
        void Update() override {}
    };
}