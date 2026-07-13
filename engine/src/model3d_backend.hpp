// model3d_backend.hpp

#pragma once

#include <string>
#include <optional>
#include "backend.hpp"
#include "model3d_formats.hpp"

namespace triton
{
    struct SVertex;
    struct SModel3DData;

    class IModel3DBackend : public iBackend
    {
        TRITON_OBJECT(IModel3DBackend)

    public:
        explicit IModel3DBackend(cContext* context) : iBackend(context) {}
        ~IModel3DBackend() override = default;

        virtual std::optional<SModel3DData> CreateModel(const std::string& modelFolderPath, const std::string& modelLocalPath) = 0;
        virtual void DestroyModel(SModel3DData& model) = 0;
    };
}