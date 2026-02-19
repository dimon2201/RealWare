// graphics_pipeline_backend.hpp

#pragma once

#include "graphics_pipeline_backend.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;

    class cGraphicsShaderBackendOGL final : public iGraphicsShaderBackend
    {
        TRITON_OBJECT(cGraphicsShaderBackendOGL)

    public:
        explicit cGraphicsShaderBackendOGL(cContext* context);
        virtual ~cGraphicsShaderBackendOGL() override final = default;

        virtual void BindShader(const cShader* shader) override final;
        virtual void UnbindShader() override final;
        virtual cShader* CreateShader(
            cGraphics::eRenderPath renderPath,
            const std::string& vertexPath,
            const std::string& fragmentPath,
            const std::vector<cShader::sDefinePair>& definePairs = {}
        ) override final;
        virtual cShader* CreateShader(
            const cShader* baseShader,
            const std::string& vertexFunc,
            const std::string& fragmentFunc,
            const std::vector<cShader::sDefinePair>& definePairs = {}
        ) override final;
        virtual void DestroyShader(cShader* shader) override final;
        virtual void SetShaderUniform(const cShader* shader, const std::string& name, const glm::mat4& matrix) override final;
        virtual void SetShaderUniform(
            const cShader* shader,
            const std::string& name,
            types::usize count,
            const types::f32* values
        ) override final;
    };
}