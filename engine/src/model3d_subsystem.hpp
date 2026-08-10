// model3d_subsystem.hpp

#pragma once

#include <optional>
#include <filesystem>
#include "DELETE_THIS_FILE_ASAP.hpp"
#include "model3d_data.hpp"

namespace triton
{
    class XModel3DPool;

    class XModel3DSubsystem : public ISubsys
    {
        TRITON_OBJECT(XModel3DSubsystem)

        XModel3DPool* _pool = nullptr;

    public:
        explicit XModel3DSubsystem(cContext* context);
        ~XModel3DSubsystem() override;

        std::optional<SModel3DData::THandle> CreateFromRaw(const std::filesystem::path& modelFilePath);
        std::optional<SModel3DData::THandle> CreateFromAsset(const std::filesystem::path& assetFilePath);
        void Destroy(const SModel3DData::THandle& model);

        void Init() override {}
        void Free() override {}
        void Update() override {}

        inline XModel3DPool* GetPool() const
        {
            return _pool;
        }
    };
}