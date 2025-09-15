#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "id_vec.hpp"
#include "types.hpp"

namespace realware
{
    namespace app
    {
        class cApplication;
    }

    namespace game
    {
        class cGameObject;
    }

    namespace render
    {
        struct cTextureAtlasTexture;
    }

    namespace render
    {
        struct sBuffer;
        struct sVertexArray;
        struct sRenderTarget;
        struct sRenderPass;
        class cRenderContext;

        using index = types::u32;

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

            types::usize VertexCount = 0;
            types::usize IndexCount = 0;
            void* VertexPtr = nullptr;
            void* IndexPtr = nullptr;
            types::usize VertexOffset = 0;
            types::usize IndexOffset = 0;
            eFormat Format = eFormat::NONE;
        };

        struct sPrimitive
        {
            sVertexBufferGeometry::eFormat Format = render::sVertexBufferGeometry::eFormat::NONE;
            sVertex* Vertices = nullptr;
            index* Indices = nullptr;
            types::usize VertexCount = 0;
            types::usize IndexCount = 0;
            types::usize VerticesByteSize = 0;
            types::usize IndicesByteSize = 0;
        };

        struct sModel : sPrimitive
        {
        };

        struct sLight
        {
            glm::vec3 Color = glm::vec3(0.0f);
            glm::vec3 Direction = glm::vec3(0.0f);
            types::f32 Scale = 0.0f;
            types::f32 AttenuationConstant = 0.0f;
            types::f32 AttenuationLinear = 0.0f;
            types::f32 AttenuationQuadratic = 0.0f;
        };

        struct sTransform
        {
            sTransform() = default;
            sTransform(const game::cGameObject* const gameObject);

            void Transform();

            types::boolean Use2D = types::K_FALSE;
            glm::vec3 Position = glm::vec3(0.0f);
            glm::vec3 Rotation = glm::vec3(0.0f);
            glm::vec3 Scale = glm::vec3(1.0f);
            glm::mat4 World = glm::mat4(1.0f);
        };

        struct cMaterial : public utils::cIdVecObject
        {
        public:
            cMaterial() = default;
            explicit cMaterial(const render::cTextureAtlasTexture* const diffuseTexture, const glm::vec4& diffuseColor, const glm::vec4& highlightColor)
                : _diffuseTexture((render::cTextureAtlasTexture*)diffuseTexture), _diffuseColor(diffuseColor), _highlightColor(highlightColor) {}
            ~cMaterial() = default;

            render::cTextureAtlasTexture* GetDiffuseTexture() const { return _diffuseTexture; }
            glm::vec4 GetDiffuseColor() const { return _diffuseColor; }
            glm::vec4 GetHighlightColor() const { return _highlightColor; }

        private:
            render::cTextureAtlasTexture* _diffuseTexture = nullptr;
            glm::vec4 _diffuseColor = glm::vec4(1.0f);
            glm::vec4 _highlightColor = glm::vec4(1.0f);
        };

        struct sRenderInstance
        {
            sRenderInstance(types::s32 materialIndex, const sTransform& transform);

            types::f32 Use2D = 0.0f;
            types::s32 MaterialIndex = -1;
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
            sMaterialInstance(types::s32 materialIndex, const cMaterial* const material);

            void SetDiffuseTexture(const render::cTextureAtlasTexture& area);

            types::s32 BufferIndex = -1;
            types::f32 DiffuseTextureLayerInfo = 0.0f;
            types::f32 MetallicTextureLayerInfo = 0.0f;
            types::f32 RoughnessTextureLayerInfo = 0.0f;
            types::f32 UserData[4] = {};
            glm::vec4 DiffuseTextureInfo = glm::vec4(0.0f);
            glm::vec4 DiffuseColor = glm::vec4(0.0f);
            glm::vec4 HighlightColor = glm::vec4(0.0f);
        };

        struct sLightInstance
        {
            sLightInstance(const game::cGameObject* const object);

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

            explicit mRender(const app::cApplication* const app, const cRenderContext* const context);
            ~mRender();

            cMaterial* AddMaterial(const std::string& id, const render::cTextureAtlasTexture* const diffuseTexture, const glm::vec4& diffuseColor, const glm::vec4& highlightColor);
            cMaterial* FindMaterial(const std::string& id);
            sVertexArray* CreateDefaultVertexArray();
            sVertexBufferGeometry* CreateGeometry(const sVertexBufferGeometry::eFormat& format, const types::usize verticesByteSize, const void* const vertices, const types::usize indicesByteSize, const void* const indices);
            void DeleteMaterial(const std::string& id);
            inline void DestroyGeometry(sVertexBufferGeometry* geometry) { delete geometry; }
            void ClearGeometryBuffer();

            void ClearRenderPass(const sRenderPass* const renderPass, const types::boolean clearColor, const types::usize bufferIndex, const glm::vec4& color, const types::boolean clearDepth, const types::f32 depth);
            void ClearRenderPasses(const glm::vec4& clearColor, const types::f32 clearDepth);
            
            void UpdateLights();

            void DrawGeometryOpaque(const sVertexBufferGeometry* const geometry, const std::vector<game::cGameObject>& objects, const game::cGameObject* const cameraObject);
            void DrawGeometryTransparent(const sVertexBufferGeometry* const geometry, const std::vector<game::cGameObject>& objects, const game::cGameObject* const cameraObject);
            void DrawTexts(const std::vector<game::cGameObject>& objects);
            
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
            app::cApplication* _app = nullptr;
            cRenderContext* _context = nullptr;
            sBuffer* _vertexBuffer = nullptr;
            sBuffer* _indexBuffer = nullptr;
            sBuffer* _instanceBuffer = nullptr;
            sBuffer* _materialBuffer = nullptr;
            sBuffer* _lightBuffer = nullptr;
            void* _vertices = nullptr;
            types::usize _verticesByteSize = 0;
            void* _indices = nullptr;
            types::usize _indicesByteSize = 0;
            void* _instances = nullptr;
            types::usize _instancesByteSize = 0;
            void* _materials = nullptr;
            types::usize _materialsByteSize = 0;
            void* _lights = nullptr;
            types::usize _lightsByteSize = 0;
            std::unordered_map<render::cMaterial*, types::s32>* _materialsMap = {};
            sRenderPass* _opaque = nullptr;
            sRenderPass* _transparent = nullptr;
            sRenderPass* _text = nullptr;
            sRenderPass* _compositeTransparent = nullptr;
            sRenderPass* _compositeFinal = nullptr;
            types::usize _materialCountCPU = 0;
            utils::cIdVec<cMaterial> _materialsCPU;
        };
    }
}