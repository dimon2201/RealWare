#pragma once

#include <vector>
#include <unordered_map>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "ecs.hpp"
#include "texture_manager.hpp"
#include "types.hpp"

namespace realware
{
    namespace core
    {
        struct sRenderComponent;
        class cApplication;
        class cUserInput;
        class cTextureAtlas;
        class cGameObject;
    }

    namespace font
    {
        struct sFont;
    }

    using namespace core;

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
            glm::vec3 Position;
            glm::vec2 Texcoord;
            glm::vec3 Normal;
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
            render::sVertexBufferGeometry::eFormat Format;
            render::sVertex* Vertices;
            render::index* Indices;
            core::usize VertexCount;
            core::usize IndexCount;
            core::usize VerticesByteSize;
            core::usize IndicesByteSize;
        };

        struct sModel : sPrimitive
        {
        };

        struct sLight
        {
            glm::vec3 Color;
            glm::vec3 Direction;
            f32 Scale;
            f32 AttenuationConstant;
            f32 AttenuationLinear;
            f32 AttenuationQuadratic;
        };

        struct sTransform
        {
            sTransform() = default;
            sTransform(cGameObject* gameObject);

            void Transform()
            {
                glm::quat quatX = glm::angleAxis(Rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
                glm::quat quatY = glm::angleAxis(Rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
                glm::quat quatZ = glm::angleAxis(Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

                World = glm::translate(glm::mat4(1.0f), Position) * glm::toMat4(quatZ * quatY * quatX) * glm::scale(glm::mat4(1.0f), Scale);
            }

            boolean Use2D = K_FALSE;
            glm::vec3 Position = glm::vec3(0.0f);
            glm::vec3 Rotation = glm::vec3(0.0f);
            glm::vec3 Scale = glm::vec3(1.0f);
            glm::mat4 World = glm::mat4(1.0f);
        };

        struct cMaterial
        {

        public:
            cMaterial() = default;
            cMaterial(
                const std::string& id,
                sArea* diffuseTexture,
                const glm::vec4& diffuseColor,
                const glm::vec4& highlightColor
            )
            {
                m_id = id;
                m_diffuseTexture = diffuseTexture;
                m_diffuseColor = diffuseColor;
                m_highlightColor = highlightColor;
            }
            ~cMaterial() = default;

            std::string& GetID() { return m_id; }
            sArea* GetDiffuseTexture() { return m_diffuseTexture; }
            glm::vec4 GetDiffuseColor() { return m_diffuseColor; }
            glm::vec4 GetHighlightColor() { return m_highlightColor; }

        private:
            std::string m_id = "";
            sArea* m_diffuseTexture = nullptr;
            glm::vec4 m_diffuseColor = glm::vec4(1.0f);
            glm::vec4 m_highlightColor = glm::vec4(1.0f);

        };

        struct sRenderInstance
        {
            sRenderInstance(core::s32 materialIndex, sTransform& transform)
            {
                Use2D = transform.Use2D;
                MaterialIndex = materialIndex;
                World = transform.World;
            }

            float Use2D;
            core::s32 MaterialIndex;
            unsigned _pad[2];
            glm::mat4 World;
        };

        struct sTextInstance
        {
            glm::vec4 Info;
            glm::vec4 AtlasInfo;
        };

        struct sMaterialInstance
        {
            sMaterialInstance(core::s32 materialIndex, cMaterial* material)
            {
                BufferIndex = materialIndex;
                DiffuseColor = material->GetDiffuseColor();
                HighlightColor = material->GetHighlightColor();
            }

            void SetDiffuseTexture(const core::sArea& area)
            {
                DiffuseTextureLayerInfo = area.Offset.z;
                DiffuseTextureInfo = glm::vec4(area.Offset.x, area.Offset.y, area.Size.x, area.Size.y);
            }

            core::s32 BufferIndex = -1;
            float DiffuseTextureLayerInfo;
            float MetallicTextureLayerInfo;
            float RoughnessTextureLayerInfo;
            float UserData[4];
            glm::vec4 DiffuseTextureInfo;
            glm::vec4 DiffuseColor;
            glm::vec4 HighlightColor;
        };

        struct sLightInstance
        {
            sLightInstance(cGameObject* object);

            glm::vec4 Position;
            glm::vec4 Color;
            glm::vec4 DirectionAndScale;
            glm::vec4 Attenuation;
        };

        struct sInstanceList
        {
            sInstanceList(sVertexBufferGeometry* geometry, core::cScene* scene, core::usize instanceCount)
                : Geometry(geometry), Scene(scene), InstanceCount(instanceCount)
            {
            }

            sVertexBufferGeometry* Geometry;
            core::cScene* Scene;
            core::usize InstanceCount;
        };

        class mRender
        {

        public:
            mRender(
                cApplication* app,
                const cRenderContext* context,
                core::usize vertexBufferSize,
                core::usize indexBufferSize,
                core::usize instanceBufferSize,
                core::usize materialBufferSize,
                core::usize lightBufferSize,
                core::usize maxMaterialCount,
                const glm::vec2& windowSize
            );
            ~mRender();

            cMaterial* CreateMaterial(
                const std::string& id,
                sArea* diffuseTexture,
                const glm::vec4& diffuseColor,
                const glm::vec4& highlightColor
            );
            void DeleteMaterial(const std::string& id);
            sVertexArray* CreateDefaultVertexArray();
            sVertexBufferGeometry* AddGeometry(const sVertexBufferGeometry::eFormat& format, core::usize verticesByteSize, const void* vertices, core::usize indicesByteSize, const void* indices);
            inline void DeleteGeometry(sVertexBufferGeometry* geometry) { delete geometry; }
            void ClearGeometryBuffer();
            inline void DeleteInstanceList(sInstanceList* list) { delete list; }
            void ClearRenderPass(sRenderPass* renderPass, core::boolean clearColor, core::usize bufferIndex, const glm::vec4& color, core::boolean clearDepth, float depth);
            void ClearRenderPasses(const glm::vec4& clearColor, const float clearDepth);
            void UpdateLights(core::cApplication* app);
            void DrawGeometryOpaque(
                core::cApplication* application,
                sVertexBufferGeometry* geometry,
                std::vector<cGameObject>& objects,
                const std::string& cameraObjectID
            );
            void DrawGeometryTransparent(
                core::cApplication* application,
                sVertexBufferGeometry* geometry,
                std::vector<cGameObject>& objects,
                const std::string& cameraObjectID
            );
            void DrawTexts(
                core::cApplication* application,
                std::vector<cGameObject>& objects
            );
            void DrawCaptions(core::cApplication* app, core::cScene* scene);
            void DrawButtons(core::cApplication* app, core::cScene* scene);
            void DrawPopupMenus(core::cApplication* app, core::cScene* scene);
            void DrawCheckboxes(core::cApplication* app, core::cScene* scene);
            void DrawWidgets(core::cApplication* app, core::cScene* scene);
            void CompositeTransparent();
            void CompositeFinal();
            sPrimitive* CreateTriangle();
            sPrimitive* CreateQuad();
            sModel* CreateModel(const std::string& filename);
            void FreePrimitive(sPrimitive* primitive);
            void ResizeWindow(const glm::vec2& size);

            sBuffer* GetVertexBuffer() { return m_vertexBuffer; }
            sBuffer* GetIndexBuffer() { return m_indexBuffer; }
            sBuffer* GetInstanceBuffer() { return m_instanceBuffer; }
            sBuffer* GetMaterialBuffer() { return m_materialBuffer; }
            sBuffer* GetLightBuffer() { return m_lightBuffer; }
            inline sRenderPass* GetOpaqueRenderPass() { return m_opaque; }
            inline sRenderPass* GetTransparentRenderPass() { return m_transparent; }
            inline sRenderPass* GetWidgetRenderPass() { return m_widget; }
            inline sRenderPass* GetTextRenderPass() { return m_text; }
            inline sRenderPass* GetCompositeTransparentRenderPass() { return m_compositeTransparent; }
            inline sRenderPass* GetCompositeFinalRenderPass() { return m_compositeFinal; }

        private:
            cApplication* m_app;
            cRenderContext* m_context;
            sBuffer* m_vertexBuffer;
            sBuffer* m_indexBuffer;
            sBuffer* m_instanceBuffer;
            sBuffer* m_materialBuffer;
            sBuffer* m_lightBuffer;
            void* m_vertices;
            core::usize m_verticesByteSize;
            void* m_indices;
            core::usize m_indicesByteSize;
            void* m_instances;
            core::usize m_instancesByteSize;
            void* m_materials;
            core::usize m_materialsByteSize;
            void* m_lights;
            core::usize m_lightsByteSize;
            std::unordered_map<render::cMaterial*, core::s32>* m_materialsMap;
            sRenderPass* m_opaque;
            sRenderPass* m_transparent;
            sRenderPass* m_widget;
            sRenderPass* m_text;
            sRenderPass* m_compositeTransparent;
            sRenderPass* m_compositeFinal;
            usize m_materialCountCPU = 0;
            std::vector<cMaterial> m_materialsCPU;

        };
    }
}