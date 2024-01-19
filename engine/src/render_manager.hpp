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
        class cUserInput;
        class cTextureAtlas;
    }

    namespace font
    {
        struct sFont;
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

        struct sRenderInstance
        {
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
            mRender() {}
            ~mRender() {}

            void Init(const cRenderContext* context, core::usize vertexBufferSize, core::usize indexBufferSize, core::usize instanceBufferSize, core::usize materialBufferSize, const glm::vec2& windowSize);
            void Free();
            sVertexArray* CreateDefaultVertexArray();
            sVertexBufferGeometry* AddGeometry(const sVertexBufferGeometry::eFormat& format, core::usize verticesByteSize, const void* vertices, core::usize indicesByteSize, const void* indices);
            inline void DeleteGeometry(sVertexBufferGeometry* geometry) { delete geometry; }
            void ClearGeometryBuffer();
            inline void DeleteInstanceList(sInstanceList* list) { delete list; }
            void ClearRenderPass(sRenderPass* renderPass, core::boolean clearColor, core::usize bufferIndex, const glm::vec4& color, core::boolean clearDepth, float depth);
            void ClearRenderPasses(const glm::vec4& clearColor, const float clearDepth);
            void DrawGeometryOpaque(core::cApplication* application, sVertexBufferGeometry* geometry, core::cScene* scene);
            void DrawGeometryTransparent(core::cApplication* application, sVertexBufferGeometry* geometry, core::cScene* scene);
            void DrawTexts(core::cApplication* app, core::cScene* scene);
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
            inline sRenderPass* GetOpaqueRenderPass() { return m_opaque; }
            inline sRenderPass* GetTransparentRenderPass() { return m_transparent; }
            inline sRenderPass* GetWidgetRenderPass() { return m_widget; }
            inline sRenderPass* GetTextRenderPass() { return m_text; }
            inline sRenderPass* GetCompositeTransparentRenderPass() { return m_compositeTransparent; }
            inline sRenderPass* GetCompositeFinalRenderPass() { return m_compositeFinal; }
            inline void SetCamera(core::entity camera) { m_camera = camera; }

        private:
            cRenderContext* m_context;
            sBuffer* m_vertexBuffer;
            sBuffer* m_indexBuffer;
            sBuffer* m_instanceBuffer;
            sBuffer* m_materialBuffer;
            void* m_vertices;
            core::usize m_verticesByteSize;
            void* m_indices;
            core::usize m_indicesByteSize;
            void* m_instances;
            core::usize m_instancesByteSize;
            void* m_materials;
            core::usize m_materialsByteSize;
            std::unordered_map<core::sCMaterial*, core::s32>* m_materialsMap;
            sRenderPass* m_opaque;
            sRenderPass* m_transparent;
            sRenderPass* m_widget;
            sRenderPass* m_text;
            sRenderPass* m_compositeTransparent;
            sRenderPass* m_compositeFinal;
            core::entity m_camera;

        };
    }
}