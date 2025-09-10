#pragma once

#include <vector>
#include <unordered_map>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "texture_manager.hpp"
#include "types.hpp"

namespace realware
{
    namespace core
    {
        class cApplication;
        class cGameObject;
    }

    namespace render
    {
        struct sBuffer;
        struct sVertexArray;
        struct sRenderTarget;
        struct sRenderPass;
        class cRenderContext;

        using index = core::u32;

        struct sVertex
        {
            glm::vec3 Position = glm::vec3(0.0f);
            glm::vec2 Texcoord = glm::vec2(0.0f);
            glm::vec3 Normal = glm::vec3(0.0f);
        };

        struct sVertexBufferGeometry
        {
            enum class eFormat
            {
                NONE = 1,
                POSITION_TEXCOORD_NORMAL_VEC3_VEC2_VEC3 = 32
            };

            core::usize VertexCount = 0;
            core::usize IndexCount = 0;
            void* VertexPtr = nullptr;
            void* IndexPtr = nullptr;
            core::usize VertexOffset = 0;
            core::usize IndexOffset = 0;
            eFormat Format = eFormat::NONE;
        };

        struct sPrimitive
        {
            sVertexBufferGeometry::eFormat Format = render::sVertexBufferGeometry::eFormat::NONE;
            sVertex* Vertices = nullptr;
            index* Indices = nullptr;
            core::usize VertexCount = 0;
            core::usize IndexCount = 0;
            core::usize VerticesByteSize = 0;
            core::usize IndicesByteSize = 0;
        };

        struct sModel : sPrimitive
        {
        };

        struct sLight
        {
            glm::vec3 Color = glm::vec3(0.0f);
            glm::vec3 Direction = glm::vec3(0.0f);
            core::f32 Scale = 0.0f;
            core::f32 AttenuationConstant = 0.0f;
            core::f32 AttenuationLinear = 0.0f;
            core::f32 AttenuationQuadratic = 0.0f;
        };

        struct sTransform
        {
            sTransform() = default;
            sTransform(const core::cGameObject* const gameObject);

            void Transform();

            core::boolean Use2D = core::K_FALSE;
            glm::vec3 Position = glm::vec3(0.0f);
            glm::vec3 Rotation = glm::vec3(0.0f);
            glm::vec3 Scale = glm::vec3(1.0f);
            glm::mat4 World = glm::mat4(1.0f);
        };

        struct cMaterial
        {
        public:
            cMaterial() = default;
            explicit cMaterial(const std::string& id, const sTextureAtlasTexture* const diffuseTexture, const glm::vec4& diffuseColor, const glm::vec4& highlightColor)
                : _id(id), _diffuseTexture((sTextureAtlasTexture*)diffuseTexture), _diffuseColor(diffuseColor), _highlightColor(highlightColor) {}
            ~cMaterial() = default;

            const std::string& GetID() const { return _id; }
            sTextureAtlasTexture* GetDiffuseTexture() const { return _diffuseTexture; }
            glm::vec4 GetDiffuseColor() const { return _diffuseColor; }
            glm::vec4 GetHighlightColor() const { return _highlightColor; }

        private:
            std::string _id = "";
            sTextureAtlasTexture* _diffuseTexture = nullptr;
            glm::vec4 _diffuseColor = glm::vec4(1.0f);
            glm::vec4 _highlightColor = glm::vec4(1.0f);
        };

        struct sRenderInstance
        {
            sRenderInstance(core::s32 materialIndex, const sTransform& transform);

            core::f32 Use2D = 0.0f;
            core::s32 MaterialIndex = -1;
            unsigned _pad[2] = {};
            glm::mat4 World = {};
        };

        struct sTextInstance
        {
            glm::vec4 Info = glm::vec4(0.0f);
            glm::vec4 AtlasInfo = glm::vec4(0.0f);
        };

        struct sMaterialInstance
        {
            sMaterialInstance(core::s32 materialIndex, const cMaterial* const material);

            void SetDiffuseTexture(const sTextureAtlasTexture& area);

            core::s32 BufferIndex = -1;
            core::f32 DiffuseTextureLayerInfo = 0.0f;
            core::f32 MetallicTextureLayerInfo = 0.0f;
            core::f32 RoughnessTextureLayerInfo = 0.0f;
            core::f32 UserData[4] = {};
            glm::vec4 DiffuseTextureInfo = glm::vec4(0.0f);
            glm::vec4 DiffuseColor = glm::vec4(0.0f);
            glm::vec4 HighlightColor = glm::vec4(0.0f);
        };

        struct sLightInstance
        {
            sLightInstance(const core::cGameObject* const object);

            glm::vec4 Position = glm::vec4(0.0f);
            glm::vec4 Color = glm::vec4(0.0f);
            glm::vec4 DirectionAndScale = glm::vec4(0.0f);
            glm::vec4 Attenuation = glm::vec4(0.0f);
        };

        class mRender
        {
        public:
            enum class ePrimitive
            {
                TRIANGLE = 1,
                QUAD = 2
            };

            explicit mRender(const core::cApplication* const app, const cRenderContext* const context);
            ~mRender();

            cMaterial* AddMaterial(const std::string& id, const sTextureAtlasTexture* const diffuseTexture, const glm::vec4& diffuseColor, const glm::vec4& highlightColor);
            sVertexArray* CreateDefaultVertexArray();
            sVertexBufferGeometry* CreateGeometry(const sVertexBufferGeometry::eFormat& format, const core::usize verticesByteSize, const void* const vertices, const core::usize indicesByteSize, const void* const indices);
            
            void DeleteMaterial(const std::string& id);
            inline void DestroyGeometry(sVertexBufferGeometry* geometry) { delete geometry; }
            void ClearGeometryBuffer();

            void ClearRenderPass(const sRenderPass* const renderPass, const core::boolean clearColor, const core::usize bufferIndex, const glm::vec4& color, const core::boolean clearDepth, const core::f32 depth);
            void ClearRenderPasses(const glm::vec4& clearColor, const core::f32 clearDepth);
            
            void UpdateLights();

            void DrawGeometryOpaque(const sVertexBufferGeometry* const geometry, std::vector<core::cGameObject>& objects, const core::cGameObject* const cameraObject);
            void DrawGeometryTransparent(const sVertexBufferGeometry* const geometry, std::vector<core::cGameObject>& objects, const core::cGameObject* const cameraObject);
            void DrawTexts(std::vector<core::cGameObject>& objects);
            
            void CompositeTransparent();
            void CompositeFinal();
            
            sPrimitive* CreatePrimitive(const ePrimitive& primitive);
            sModel* CreateModel(const std::string& filename);
            void DestroyPrimitive(sPrimitive* primitiveObject);
            
            void ResizeWindow(const glm::vec2& size);
            
            sBuffer* GetVertexBuffer() const { return _vertexBuffer; }
            sBuffer* GetIndexBuffer() const { return _indexBuffer; }
            sBuffer* GetInstanceBuffer() const { return _instanceBuffer; }
            sBuffer* GetMaterialBuffer() const { return _materialBuffer; }
            sBuffer* GetLightBuffer() const { return _lightBuffer; }
            inline sRenderPass* GetOpaqueRenderPass() const { return _opaque; }
            inline sRenderPass* GetTransparentRenderPass() const { return _transparent; }
            inline sRenderPass* GetTextRenderPass() const { return _text; }
            inline sRenderPass* GetCompositeTransparentRenderPass() const { return _compositeTransparent; }
            inline sRenderPass* GetCompositeFinalRenderPass() const { return _compositeFinal; }

        private:
            core::cApplication* _app = nullptr;
            cRenderContext* _context = nullptr;
            sBuffer* _vertexBuffer = nullptr;
            sBuffer* _indexBuffer = nullptr;
            sBuffer* _instanceBuffer = nullptr;
            sBuffer* _materialBuffer = nullptr;
            sBuffer* _lightBuffer = nullptr;
            void* _vertices = nullptr;
            core::usize _verticesByteSize = 0;
            void* _indices = nullptr;
            core::usize _indicesByteSize = 0;
            void* _instances = nullptr;
            core::usize _instancesByteSize = 0;
            void* _materials = nullptr;
            core::usize _materialsByteSize = 0;
            void* _lights = nullptr;
            core::usize _lightsByteSize = 0;
            std::unordered_map<render::cMaterial*, core::s32>* _materialsMap = {};
            sRenderPass* _opaque = nullptr;
            sRenderPass* _transparent = nullptr;
            sRenderPass* _text = nullptr;
            sRenderPass* _compositeTransparent = nullptr;
            sRenderPass* _compositeFinal = nullptr;
            core::usize _materialCountCPU = 0;
            std::vector<cMaterial> _materialsCPU = {};
        };
    }
}