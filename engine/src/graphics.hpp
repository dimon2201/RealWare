// graphics.hpp

#pragma once

#include <optional>
#include <unordered_map>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "category.hpp"
#include "types.hpp"
#include "graphics_resource_backend.hpp"
#include "graphics_pipeline_backend.hpp"
#include "graphics_drawcall_backend.hpp"
#include "graphics_buffer_formats.hpp"
#include "geometry_view.hpp"
#include "render_pass.hpp"

namespace triton
{
	class cInputWindow;
	class iGraphicsBackend;
    class iRenderContext;
    class cApplication;
    class cGameObject;
    class cTextureAtlasTexture;
    class cContext;
    class cShader;
    class cVertexArray;
    class cRenderTarget;
    class cRenderPass;
    class cRenderPassGPU;
    class cBuffer;
    struct sBuffer;
    struct sVertexArray;
    struct sRenderTarget;
    struct sRenderPass;
    struct sShader;
    class XGeometryStorage;

    using index = types::u32;

    struct sVertex
    {
        glm::vec3 _position = glm::vec3(0.0f);
        glm::vec2 _texcoord = glm::vec2(0.0f);
        glm::vec3 _normal = glm::vec3(0.0f);
    };

    struct sVertexBufferGeometry : public iObject
    {
        TRITON_OBJECT(sVertexBufferGeometry)

        explicit sVertexBufferGeometry(cContext* context) : iObject(context) {} // TODO: Temporary solution

        types::usize _vertexCount = 0;
        types::usize _indexCount = 0;
        void* _vertexPtr = nullptr;
        void* _indexPtr = nullptr;
        types::usize _offsetVertex = 0;
        types::usize _offsetIndex = 0;
        eCategory _format = eCategory::VERTEX_BUFFER_FORMAT_NONE;
    };

    struct sPrimitive : public iObject
    {
        TRITON_OBJECT(sPrimitive)

        explicit sPrimitive(cContext* context) : iObject(context) {} // TODO: Temporary solution

        sVertex* _vertices = nullptr;
        index* _indices = nullptr;
        types::usize _vertexCount = 0;
        types::usize _indexCount = 0;
        types::usize _verticesByteSize = 0;
        types::usize _indicesByteSize = 0;
        eCategory _format = eCategory::VERTEX_BUFFER_FORMAT_NONE;
    };

    struct sModel : sPrimitive
    {
    };

    struct sLight
    {
        glm::vec3 _color = glm::vec3(0.0f);
        glm::vec3 _direction = glm::vec3(0.0f);
        types::f32 _scale = 0.0f;
        types::f32 _attenuationConstant = 0.0f;
        types::f32 _attenuationLinear = 0.0f;
        types::f32 _attenuationQuadratic = 0.0f;
    };

    class cMaterial : public iObject
    {
        TRITON_OBJECT(cMaterial)

        cShader* _customShader = nullptr;
        cTextureAtlasTexture* _diffuseTexture = nullptr;
        glm::vec4 _diffuseColor = glm::vec4(1.0f);
        glm::vec4 _highlightColor = glm::vec4(1.0f);

    public:
        explicit cMaterial(cContext* context, cTextureAtlasTexture* diffuseTexture, const glm::vec4& diffuseColor, const glm::vec4& highlightColor, cShader* customShader) : iObject(context), _diffuseTexture(diffuseTexture), _diffuseColor(diffuseColor), _highlightColor(highlightColor), _customShader(customShader) {}
        ~cMaterial() = default;

        inline cShader* GetCustomShader() const { return _customShader; }
        inline cTextureAtlasTexture* GetDiffuseTexture() const { return _diffuseTexture; }
        inline const glm::vec4& GetDiffuseColor() const { return _diffuseColor; }
        inline const glm::vec4& GetHighlightColor() const { return _highlightColor; }
    };

    struct sRenderInstance
    {
        sRenderInstance(types::s32 materialIndex, const cTransform& transform);

        types::f32 _use2D = 0.0f;
        types::s32 _materialIndex = -1;
        types::dword _pad[2] = {};
        cMatrix4 _world = cMatrix4(1.0f);
    };

    struct sTextInstance
    {
        glm::vec4 _info = glm::vec4(0.0f);
        glm::vec4 _atlasInfo = glm::vec4(0.0f);
    };

    class cMaterialInstance
    {
    public:
        cMaterialInstance(types::s32 materialIndex, const cMaterial* material);

    private:
        types::s32 _bufferIndex = -1;
        types::f32 _diffuseTextureLayerInfo = 0.0f;
        types::f32 _metallicTextureLayerInfo = 0.0f;
        types::f32 _roughnessTextureLayerInfo = 0.0f;
        types::f32 _userData[4] = {};
        glm::vec4 _diffuseTextureInfo = glm::vec4(0.0f);
        glm::vec4 _diffuseColor = glm::vec4(0.0f);
        glm::vec4 _highlightColor = glm::vec4(0.0f);
    };

    struct sLightInstance
    {
        sLightInstance(const cGameObject* object);

        cVector4 _position = cVector4(0.0f);
        cVector4 _color = cVector4(0.0f);
        cVector4 _directionAndScale = cVector4(0.0f);
        cVector4 _attenuation = cVector4(0.0f);
    };

    class SRenderPassBuffers final
    {
    public:
        cBuffer* _opaqueInstanceBuffer = nullptr;
        cBuffer* _transparentInstanceBuffer = nullptr;
        cBuffer* _textInstanceBuffer = nullptr;
        XDataBuffer* _opaqueInstanceCPU = nullptr;
        XDataBuffer* _transparentInstanceCPU = nullptr;
        XDataBuffer* _textInstanceCPU = nullptr;
        cBuffer* _opaqueMaterialBuffer = nullptr;
        cBuffer* _transparentMaterialBuffer = nullptr;
        cBuffer* _textMaterialBuffer = nullptr;
        cBuffer* _opaqueTextureAtlasTexturesBuffer = nullptr;
        cBuffer* _transparentTextureAtlasTexturesBuffer = nullptr;
        cBuffer* _textTextureAtlasTexturesBuffer = nullptr;
    };

    class SRenderPasses final
    {
        XDataBuffer* _opaquePassInstance = nullptr;
        XDataBuffer* _transparentPassInstances = nullptr;
        XDataBuffer* _textPassInstance = nullptr;
    };

	class cGraphics : public iObject
	{
        TRITON_OBJECT(cGraphics)

        void* _lights = nullptr;
        types::usize _lightsByteSize = 0;
        void* _opaqueTextureAtlasTextures = nullptr;
        types::usize _opaqueTextureAtlasTexturesByteSize = 0;
        void* _transparentTextureAtlasTextures = nullptr;
        types::usize _transparentTextureAtlasTexturesByteSize = 0;
        void* _textTextureAtlasTextures = nullptr;
        types::usize _textTextureAtlasTexturesByteSize = 0;
        std::unordered_map<cMaterial*, types::s32>* _materialsMap = {};
        cRenderPass* _opaque = nullptr;
        cRenderPass* _transparent = nullptr;
        cRenderPass* _text = nullptr;
        cRenderPass* _compositeTransparent = nullptr;
        cRenderPass* _compositeFinal = nullptr;
        cRenderTarget* _opaqueRenderTarget = nullptr;
        cRenderTarget* _transparentRenderTarget = nullptr;
        types::usize _materialCountCPU = 0;

        XGeometryStorage _geometryStorage;
        SRenderPassBuffers _renderPassBuffers = {};

	public:
        enum class eAPI
        {
            NONE = 0,
            OGL,
            D3D11
        };

		explicit cGraphics(cContext* context);
		virtual ~cGraphics() override final = default;

        void Initialize();
        void Shutdown();

        // TODO: Remove material creation from cGraphics
        //cCacheObject<cMaterial> CreateMaterial(const std::string& id, cTextureAtlasTexture* diffuseTexture, const glm::vec4& diffuseColor, const glm::vec4& highlightColor, eCategory customShaderRenderPath = eCategory::RENDER_PATH_OPAQUE, const std::string& customVertexFuncPath = "", const std::string& customFragmentFuncPath = "");
        cVertexArray* CreateDefaultVertexArray();
        std::optional<triton::SGeometryView> CreateGeometry(EGraphicsBufferFormat format, const types::u8* vertices, types::usize verticesByteSize, const types::u8* indices, types::usize indicesByteSize);
        XRenderPass* CreateRenderPass(const SRenderPassDescriptor& desc);
        sPrimitive* CreatePrimitive(eCategory primitive);
        sModel* CreateModel(const std::string& filename);

        // TODO: Remove material finding from cGraphics
        //cCacheObject<cMaterial> FindMaterial(const cTag& id);
        
        // TODO: Remove material destroying from cGraphics
        void DestroyMaterial(const cTag& id);
        void DestroyVertexArray(cVertexArray* vertexArray);
        void DestroyGeometry(sVertexBufferGeometry* geometry);
        void DestroyRenderPass(cRenderPass* renderPass);
        void DestroyPrimitive(sPrimitive* primitiveObject);
        void DestroyModel(sModel* model);
        
        void ClearGeometryBuffer();
        void ClearRenderTarget(const cRenderPass* renderPass, types::boolean clearColor, types::usize bufferIndex, const glm::vec4& color, types::boolean clearDepth, types::f32 depth);
        void ClearRenderTargets(const glm::vec4& clearColor, types::f32 clearDepth);
        void ResizeRenderTargets(const glm::vec2& size);
        void LoadShaderFiles(const std::string& vertexFuncPath, const std::string& fragmentFuncPath, std::string& vertexFunc, std::string& fragmentFunc);
        void UpdateLights();
        
        // TODO: Implement new CPU->GPU geometry buffer communication
        //void WriteObjectsToOpaqueBuffers(cIdVector<cGameObject>& objects, cRenderPass* renderPass);
        //void WriteObjectsToTransparentBuffers(cIdVector<cGameObject>& objects, cRenderPass* renderPass);
        
        void DrawGeometryOpaque(const sVertexBufferGeometry* geometry, const cGameObject* cameraObject, cRenderPass* renderPass);
        void DrawGeometryOpaque(const sVertexBufferGeometry* geometry, const cGameObject* cameraObject, cShader* singleShader = nullptr);
        void DrawGeometryTransparent(const sVertexBufferGeometry* geometry, const std::vector<cGameObject>& objects, const cGameObject* cameraObject, cRenderPass* renderPass);
        void DrawGeometryTransparent(const sVertexBufferGeometry* geometry, const cGameObject* cameraObject, cShader* singleShader = nullptr);
        // TODO: Implement new text drawing approach
        //void DrawTexts(const std::vector<cGameObject>& objects);
        
        void CompositeTransparent();
        void CompositeFinal();

        inline cBuffer* GetVertexBuffer() const { return _vertexBuffer; }
        inline cBuffer* GetIndexBuffer() const { return _indexBuffer; }
        inline cBuffer* GetOpaqueInstanceBuffer() const { return _opaqueInstanceBuffer; }
        inline cBuffer* GetTextInstanceBuffer() const { return _textInstanceBuffer; }
        inline cBuffer* GetOpaqueMaterialBuffer() const { return _opaqueMaterialBuffer; }
        inline cBuffer* GetTransparentInstanceBuffer() const { return _transparentInstanceBuffer; }
        inline cBuffer* GetTransparentMaterialBuffer() const { return _transparentMaterialBuffer; }
        inline cBuffer* GetTextMaterialBuffer() const { return _textMaterialBuffer; }
        inline cBuffer* GetLightBuffer() const { return _lightBuffer; }
        inline cBuffer* GetOpaqueTextureAtlasTexturesBuffer() const { return _opaqueTextureAtlasTexturesBuffer; }
        inline cBuffer* GetTransparentTextureAtlasTexturesBuffer() const { return _transparentTextureAtlasTexturesBuffer; }
        inline cRenderPass* GetOpaqueRenderPass() const { return _opaque; }
        inline cRenderPass* GetTransparentRenderPass() const { return _transparent; }
        inline cRenderPass* GetTextRenderPass() const { return _text; }
        inline cRenderPass* GetCompositeTransparentRenderPass() const { return _compositeTransparent; }
        inline cRenderPass* GetCompositeFinalRenderPass() const { return _compositeFinal; }
        inline cRenderTarget* GetOpaqueRenderTarget() const { return _opaqueRenderTarget; }
        inline cRenderTarget* GetTransparentRenderTarget() const { return _transparentRenderTarget; }
	};
}