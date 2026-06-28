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
#include "buffer_view.hpp"
#include "batch_storage.hpp"
#include "handle.hpp"

namespace triton
{
	class cInputWindow;
	class iGraphicsBackend;
    class iRenderContext;
    class cApplication;
    class cGameObject;
    class cTextureAtlasTexture;
    class cContext;
    class XShader;
    class CVertexArray;
    class XRenderTarget;
    class XRenderPass;
    class XRenderPassGPU;
    class cBuffer;
    struct sBuffer;
    struct sVertexArray;
    struct sRenderTarget;
    struct sRenderPass;
    struct sShader;
    class XGeometryStorage;
    class XRenderPassExecutor;
    class XInstanceBuffer;
    template <typename TSlot, typename THandle, typename TDataStructure, typename TObject>
    class XHandleAllocator;
    template <typename TValue>
    class XLinearArray;
    class XCamera;

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

        XShader* _customShader = nullptr;
        cTextureAtlasTexture* _diffuseTexture = nullptr;
        glm::vec4 _diffuseColor = glm::vec4(1.0f);
        glm::vec4 _highlightColor = glm::vec4(1.0f);

    public:
        explicit cMaterial(cContext* context, cTextureAtlasTexture* diffuseTexture, const glm::vec4& diffuseColor, const glm::vec4& highlightColor, XShader* customShader) : iObject(context), _diffuseTexture(diffuseTexture), _diffuseColor(diffuseColor), _highlightColor(highlightColor), _customShader(customShader) {}
        ~cMaterial() = default;

        inline XShader* GetCustomShader() const { return _customShader; }
        inline cTextureAtlasTexture* GetDiffuseTexture() const { return _diffuseTexture; }
        inline const glm::vec4& GetDiffuseColor() const { return _diffuseColor; }
        inline const glm::vec4& GetHighlightColor() const { return _highlightColor; }
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

    class SRenderPasses final
    {
        XDataBuffer* _opaquePassInstance = nullptr;
        XDataBuffer* _transparentPassInstances = nullptr;
        XDataBuffer* _textPassInstance = nullptr;
    };

    class SShaderDefine final
    {
    public:
        SShaderDefine(const std::string& name_, types::usize index_) : name(name_), index(index_) {}

        std::string name = "";
        types::usize index = 0;
    };

    struct SCameraSlot : public SSlot {};
    struct SCameraHandle : public SHandle {};

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
        types::usize _materialCountCPU = 0;

        XGeometryStorage* _geometryStorage = nullptr;
        XBatchStorage* _batchStorage = nullptr;
        XRenderPassExecutor* _renderPassExecutor = nullptr;
        XInstanceBuffer* _instanceBufferStatic = nullptr;
        XInstanceBuffer* _instanceBufferDynamic = nullptr;
        types::boolean _isStaticBufferDirty = types::K_FALSE;
        XRenderPass* _opaque = nullptr;
        XRenderPass* _transparent = nullptr;
        XRenderPass* _text = nullptr;
        XRenderPass* _compositeTransparent = nullptr;
        XRenderPass* _compositeFinal = nullptr;
        XRenderTarget* _opaqueRenderTarget = nullptr;
        XRenderTarget* _transparentRenderTarget = nullptr;
        XHandleAllocator<SCameraSlot, SCameraHandle, XLinearArray<XCamera>, XCamera>* _cameras = nullptr;

        void CreateGeometryStorage();
        void CreateBatchStorage();
        void CreateInstanceBuffers();
        void CreateDefaultRenderTargets();
        void CreateDefaultRenderPasses();
        void CreateCameraAllocator();
        void DestroyGeometryStorage();
        void DestroyBatchStorage();
        void DestroyInstanceBuffers();
        void DestroyDefaultRenderTargets();
        void DestroyDefaultRenderPasses();
        void DestroyCameraAllocator();
        void MarkStaticBufferDirty();
        void WriteBatchInstances(SRenderInstance::EUsage usage);
        void WriteDirtyStaticInstances();
        void WriteDynamicInstances();

	public:
        enum class eAPI
        {
            NONE = 0,
            OGL,
            D3D11
        };

		explicit cGraphics(cContext* context);
		~cGraphics() override;

        // TODO: Remove material creation from cGraphics
        //cCacheObject<cMaterial> CreateMaterial(const std::string& id, cTextureAtlasTexture* diffuseTexture, const glm::vec4& diffuseColor, const glm::vec4& highlightColor, eCategory customShaderRenderPath = eCategory::RENDER_PATH_OPAQUE, const std::string& customVertexFuncPath = "", const std::string& customFragmentFuncPath = "");
        void ExecuteDefaultRenderPasses();
        CVertexArray* CreateDefaultVertexArray();
        std::optional<triton::SGeometryView> StoreGeometry(EGraphicsBufferFormat format, const types::u8* vertices, types::usize verticesByteSize, const types::u8* indices, types::usize indicesByteSize);
        std::optional<SRenderBatchHandle> CreateBatch(const SGeometryView& geometry);
        void RemoveBatch(const SRenderBatchHandle& handle);
        std::optional<SInstanceBufferHandle> CreateInstance(SRenderInstance::EUsage usage, const SRenderBatchHandle& batch);
        void DestroyInstance(const SInstanceBufferHandle& instance);
        std::optional<SCameraHandle> CreateCamera();
        XCamera* GetCamera(const SCameraHandle& camera);
        void DestroyCamera(const SCameraHandle& camera);
        sPrimitive* CreatePrimitive(eCategory primitive);
        sModel* CreateModel(const std::string& filename);

        // TODO: Remove material finding from cGraphics
        //cCacheObject<cMaterial> FindMaterial(const cTag& id);
        
        // TODO: Remove material destroying from cGraphics
        void DestroyMaterial(const cTag& id);
        void DestroyVertexArray(CVertexArray* vertexArray);
        void DestroyGeometry(sVertexBufferGeometry* geometry);
        void DestroyRenderPass(XRenderPass* renderPass);
        void DestroyPrimitive(sPrimitive* primitiveObject);
        void DestroyModel(sModel* model);
        
        void ClearRenderTarget(const XRenderPass* renderPass, types::boolean clearColor, types::usize bufferIndex, const glm::vec4& color, types::boolean clearDepth, types::f32 depth);
        void ClearRenderTargets(const glm::vec4& clearColor, types::f32 clearDepth);
        void ResizeRenderTargets(const cVector2& size);
        void LoadShaderFiles(const std::string& vertexFuncPath, const std::string& fragmentFuncPath, std::string& vertexFunc, std::string& fragmentFunc);
        void UpdateLights();
        
        // TODO: Implement new CPU->GPU geometry buffer communication
        //void WriteObjectsToOpaqueBuffers(cIdVector<cGameObject>& objects, XRenderPass* renderPass);
        //void WriteObjectsToTransparentBuffers(cIdVector<cGameObject>& objects, XRenderPass* renderPass);
        
        void DrawGeometryOpaque(const sVertexBufferGeometry* geometry, const cGameObject* cameraObject, XRenderPass* renderPass);
        void DrawGeometryOpaque(const sVertexBufferGeometry* geometry, const cGameObject* cameraObject, XShader* singleShader = nullptr);
        void DrawGeometryTransparent(const sVertexBufferGeometry* geometry, const std::vector<cGameObject>& objects, const cGameObject* cameraObject, XRenderPass* renderPass);
        void DrawGeometryTransparent(const sVertexBufferGeometry* geometry, const cGameObject* cameraObject, XShader* singleShader = nullptr);
        // TODO: Implement new text drawing approach
        //void DrawTexts(const std::vector<cGameObject>& objects);
        
        void CompositeTransparent();
        void CompositeFinal();

        cBuffer* GetVertexBuffer() const;
        cBuffer* GetIndexBuffer() const;

        inline SBufferView<XRenderBatch> GetBatches() const
        {
            return _batchStorage->GetBatches();
        }

        inline XRenderPass* GetOpaqueRenderPass() const
        {
            return _opaque;
        }

        inline XRenderPass* GetTransparentRenderPass() const
        {
            return _transparent;
        }

        inline XRenderPass* GetTextRenderPass() const
        {
            return _text;
        }

        inline XRenderPass* GetCompositeTransparentRenderPass() const
        {
            return _compositeTransparent;
        }

        inline XRenderPass* GetCompositeFinalRenderPass() const
        {
            return _compositeFinal;
        }
	};
}