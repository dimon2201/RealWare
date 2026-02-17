// graphics_shader_backend.hpp

#pragma once

#include "gpu_resource.hpp"
#include "backend.hpp"
#include "math.hpp"
#include "graphics.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;

    class cShader : public cGPUResource
    {
        TRITON_OBJECT(cShader)

        std::string _vertex = "";
        std::string _fragment = "";

    public:
        struct sDefinePair
        {
            sDefinePair(const std::string& name, types::usize index);
            ~sDefinePair() = default;

            std::string name = "";
            types::usize index = 0;
        };

        explicit cShader(
            cContext* context,
            types::qword instance,
            const std::string& vertexShaderStr,
            const std::string& fragmentShaderStr
        );
        virtual ~cShader() override = default;

        inline const std::string& GetVertexStr() const { return _vertex; }
        inline const std::string& GetFragmentStr() const { return _fragment; }
    };

    class iGraphicsShaderBackend : public iBackend
    {
        TRITON_OBJECT(iGraphicsShaderBackend)

    public:
        explicit iGraphicsShaderBackend(cContext* context);
        virtual ~iGraphicsShaderBackend() override = default;

        virtual void BindShader(const cShader* shader) = 0;
        virtual void UnbindShader() = 0;
        virtual cShader* CreateShader(
            cGraphics::eRenderPath renderPath,
            const std::string& vertexPath,
            const std::string& fragmentPath,
            const std::vector<cShader::sDefinePair>& definePairs = {}
        ) = 0;
        virtual cShader* CreateShader(
            const cShader* baseShader,
            const std::string& vertexFunc,
            const std::string& fragmentFunc,
            const std::vector<cShader::sDefinePair>& definePairs = {}
        ) = 0;
        virtual void DestroyShader(cShader* shader) = 0;
        virtual void SetShaderUniform(const cShader* shader, const std::string& name, const glm::mat4& matrix) = 0;
        virtual void SetShaderUniform(
            const cShader* shader,
            const std::string& name,
            types::usize count,
            const types::f32* values
        ) = 0;
    };
}