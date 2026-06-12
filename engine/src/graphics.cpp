// graphics.cpp

#include <GL/glew.h>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include "../../thirdparty/glm/glm/gtc/quaternion.hpp"
#include "../../thirdparty/glm/glm/gtx/quaternion.hpp"
#include "context.hpp"
#include "graphics.hpp"
#include "engine.hpp"
#include "texture_manager.hpp"
#include "gameobject_manager.hpp"
#include "font_manager.hpp"
#include "filesystem_manager.hpp"
#include "application.hpp"
#include "memory_pool.hpp"
#include "log.hpp"
#include "graphics.hpp"
#include "input.hpp"
#include "geometry_storage.hpp"
#include "log.hpp"

using namespace types;

triton::sRenderInstance::sRenderInstance(s32 materialIndex, const cTransform& transform)
{
    // TODO: Implement 2D/3D render instances
    //_use2D = transform._use2D;
    _materialIndex = materialIndex;
    _world = transform.GetWorld();
}

triton::cMaterialInstance::cMaterialInstance(s32 materialIndex, const cMaterial* material)
{
    _bufferIndex = materialIndex;
    _diffuseColor = material->GetDiffuseColor();
    _highlightColor = material->GetHighlightColor();

    const cTextureAtlasTexture* diffuse = material->GetDiffuseTexture();
    if (diffuse)
    {
        _diffuseTextureLayerInfo = diffuse->GetOffset().z;
        _diffuseTextureInfo = glm::vec4(diffuse->GetOffset().x, diffuse->GetOffset().y, diffuse->GetSize().x, diffuse->GetSize().y);
    }
    else
    {
        _diffuseTextureLayerInfo = -1.0f;
    }
}

triton::sLightInstance::sLightInstance(const cGameObject* object)
{
    const sLight* light = object->GetLight();
    // TODO: Overload constructors for cVector4
    //_position = cVector4(object->GetTransform()->_position, 0.0f);
    //_color = cVector4(light->_color, 0.0f);
    //_directionAndScale = cVector4(light->_direction, light->_scale);
    _attenuation = cVector4(
        light->_attenuationConstant,
        light->_attenuationLinear,
        light->_attenuationQuadratic,
        0.0f
    );
}
    
triton::cGraphics::cGraphics(cContext* context) : iObject(context) {}

void triton::cGraphics::Initialize()
{
    _geometryStorage.Initialize();
    CreateDefaultRenderTargets();

    /*_vertices = memoryAllocator->Allocate(caps->vertexBufferSize, caps->memoryAlignment);
    _verticesByteSize = 0;
    _indices = memoryAllocator->Allocate(caps->indexBufferSize, caps->memoryAlignment);
    _indicesByteSize = 0;
    _opaqueInstances = memoryAllocator->Allocate(_maxOpaqueInstanceBufferByteSize, caps->memoryAlignment);
    _opaqueInstancesByteSize = 0;
    _transparentInstances = memoryAllocator->Allocate(_maxTransparentInstanceBufferByteSize, caps->memoryAlignment);
    _transparentInstancesByteSize = 0;
    _textInstances = memoryAllocator->Allocate(_maxTextInstanceBufferByteSize, caps->memoryAlignment);
    _textInstancesByteSize = 0;
    _opaqueMaterials = memoryAllocator->Allocate(_maxMaterialBufferByteSize, caps->memoryAlignment);
    _opaqueMaterialsByteSize = 0;
    _transparentMaterials = memoryAllocator->Allocate(_maxMaterialBufferByteSize, caps->memoryAlignment);
    _transparentMaterialsByteSize = 0;
    _textMaterials = memoryAllocator->Allocate(_maxMaterialBufferByteSize, caps->memoryAlignment);
    _textMaterialsByteSize = 0;
    _lights = memoryAllocator->Allocate(_maxLightBufferByteSize, caps->memoryAlignment);
    _lightsByteSize = 0;
    _opaqueTextureAtlasTextures = memoryAllocator->Allocate(_maxTextureAtlasTexturesBufferByteSize, caps->memoryAlignment);
    _opaqueTextureAtlasTexturesByteSize = 0;
    _transparentTextureAtlasTextures = memoryAllocator->Allocate(_maxTextureAtlasTexturesBufferByteSize, caps->memoryAlignment);
    _transparentTextureAtlasTexturesByteSize = 0;
    _materialsMap = new std::unordered_map<cMaterial*, s32>(); // TODO: replace std::unordered_map with cHashTable

    cTextureAtlas* textureAtlas = _context->GetSubsystem<cTextureAtlas>();

    sViewport viewport;
    viewport.rect = cVector4(0.0f, 0.0f, windowSize.GetX(), windowSize.GetY());

    sRenderPassDescriptor opaqueRenderPassDesc;
    opaqueRenderPassDesc.inputVertexFormat = eCategory::VERTEX_BUFFER_FORMAT_POS_TEX_NRM_VEC3_VEC2_VEC3;
    opaqueRenderPassDesc.inputBuffers.emplace_back(cGraphics::GetVertexBuffer());
    opaqueRenderPassDesc.inputBuffers.emplace_back(cGraphics::GetIndexBuffer());
    opaqueRenderPassDesc.inputBuffers.emplace_back(cGraphics::GetOpaqueInstanceBuffer());
    opaqueRenderPassDesc.inputBuffers.emplace_back(cGraphics::GetOpaqueMaterialBuffer());
    opaqueRenderPassDesc.inputBuffers.emplace_back(cGraphics::GetLightBuffer());
    opaqueRenderPassDesc.inputBuffers.emplace_back(cGraphics::GetOpaqueTextureAtlasTexturesBuffer());
    opaqueRenderPassDesc.inputTextures.emplace_back(textureAtlas->GetAtlas());
    opaqueRenderPassDesc.inputTextureNames.emplace_back("TextureAtlas");
    opaqueRenderPassDesc.shaderBase = nullptr;
    opaqueRenderPassDesc.shaderRenderPath = sRenderPassDescriptor::eRenderPath::OPAQUE_PATH;
    opaqueRenderPassDesc.shaderVertexPath = "C:/My/My Projects Programming/TritonEngine/runtime/data/shaders/main_vertex.shader";
    opaqueRenderPassDesc.shaderFragmentPath = "C:/My/My Projects Programming/TritonEngine/runtime/data/shaders/main_fragment.shader";
    opaqueRenderPassDesc.viewport = viewport;
    opaqueRenderPassDesc.depthMode.useDepthTest = K_TRUE;
    opaqueRenderPassDesc.depthMode.useDepthWrite = K_TRUE;
    opaqueRenderPassDesc.blendMode.factorCount = 1;
    opaqueRenderPassDesc.blendMode.srcFactors[0] = sBlendMode::eBlendFactor::ONE;
    opaqueRenderPassDesc.blendMode.dstFactors[0] = sBlendMode::eBlendFactor::ZERO;
    opaqueRenderPassDesc.renderTarget = _opaqueRenderTarget;
    _opaque = CreateRenderPass(opaqueRenderPassDesc);

    sRenderPassDescriptor transparentRenderPassDesc;
    transparentRenderPassDesc.inputVertexFormat = eCategory::VERTEX_BUFFER_FORMAT_POS_TEX_NRM_VEC3_VEC2_VEC3;
    transparentRenderPassDesc.inputBuffers.emplace_back(cGraphics::GetVertexBuffer());
    transparentRenderPassDesc.inputBuffers.emplace_back(cGraphics::GetIndexBuffer());
    transparentRenderPassDesc.inputBuffers.emplace_back(cGraphics::GetTransparentInstanceBuffer());
    transparentRenderPassDesc.inputBuffers.emplace_back(cGraphics::GetTransparentMaterialBuffer());
    transparentRenderPassDesc.inputTextures.emplace_back(textureAtlas->GetAtlas());
    transparentRenderPassDesc.inputTextureNames.emplace_back("TextureAtlas");
    transparentRenderPassDesc.shaderBase = nullptr;
    transparentRenderPassDesc.shaderRenderPath = sRenderPassDescriptor::eRenderPath::TRANSPARENT_PATH;
    transparentRenderPassDesc.shaderVertexPath = "C:/My/My Projects Programming/TritonEngine/runtime/data/shaders/main_vertex.shader";
    transparentRenderPassDesc.shaderFragmentPath = "C:/My/My Projects Programming/TritonEngine/runtime/data/shaders/main_fragment.shader";
    transparentRenderPassDesc.viewport = viewport;
    transparentRenderPassDesc.depthMode.useDepthTest = K_TRUE;
    transparentRenderPassDesc.depthMode.useDepthWrite = K_FALSE;
    transparentRenderPassDesc.blendMode.factorCount = 2;
    transparentRenderPassDesc.blendMode.srcFactors[0] = sBlendMode::eBlendFactor::ONE;
    transparentRenderPassDesc.blendMode.dstFactors[0] = sBlendMode::eBlendFactor::ONE;
    transparentRenderPassDesc.blendMode.srcFactors[1] = sBlendMode::eBlendFactor::ZERO;
    transparentRenderPassDesc.blendMode.dstFactors[1] = sBlendMode::eBlendFactor::INV_SRC_COLOR;
    transparentRenderPassDesc.renderTarget = _transparentRenderTarget;
    _transparent = CreateRenderPass(transparentRenderPassDesc);

    sRenderPassDescriptor textRenderPassDesc;
    textRenderPassDesc.inputVertexFormat = eCategory::VERTEX_BUFFER_FORMAT_NONE;
    textRenderPassDesc.inputBuffers.emplace_back(cGraphics::GetTextInstanceBuffer());
    textRenderPassDesc.inputBuffers.emplace_back(cGraphics::GetTextMaterialBuffer());
    textRenderPassDesc.shaderBase = nullptr;
    textRenderPassDesc.shaderRenderPath = sRenderPassDescriptor::eRenderPath::TEXT_PATH;
    textRenderPassDesc.shaderVertexPath = "C:/My/My Projects Programming/TritonEngine/runtime/data/shaders/main_vertex.shader";
    textRenderPassDesc.shaderFragmentPath = "C:/My/My Projects Programming/TritonEngine/runtime/data/shaders/main_fragment.shader";
    textRenderPassDesc.viewport = viewport;
    textRenderPassDesc.depthMode.useDepthTest = K_FALSE;
    textRenderPassDesc.depthMode.useDepthWrite = K_FALSE;
    textRenderPassDesc.renderTarget = _opaqueRenderTarget;
    _text = CreateRenderPass(textRenderPassDesc);

    sRenderPassDescriptor compositeTransparentRenderPassDesc;
    auto& transparentColorAttachments = _transparentRenderTarget->GetColorAttachments();
    compositeTransparentRenderPassDesc.inputVertexFormat = eCategory::VERTEX_BUFFER_FORMAT_NONE;
    compositeTransparentRenderPassDesc.inputTextures.emplace_back(transparentColorAttachments[0]);
    compositeTransparentRenderPassDesc.inputTextureNames.emplace_back("AccumulationTexture");
    compositeTransparentRenderPassDesc.inputTextures.emplace_back(transparentColorAttachments[1]);
    compositeTransparentRenderPassDesc.inputTextureNames.emplace_back("RevealageTexture");
    compositeTransparentRenderPassDesc.shaderBase = nullptr;
    compositeTransparentRenderPassDesc.shaderRenderPath = sRenderPassDescriptor::eRenderPath::TRANSPARENT_COMPOSITE_PATH;
    compositeTransparentRenderPassDesc.shaderVertexPath = "C:/My/My Projects Programming/TritonEngine/runtime/data/shaders/main_vertex.shader";
    compositeTransparentRenderPassDesc.shaderFragmentPath = "C:/My/My Projects Programming/TritonEngine/runtime/data/shaders/main_fragment.shader";
    compositeTransparentRenderPassDesc.viewport = viewport;
    compositeTransparentRenderPassDesc.depthMode.useDepthTest = K_FALSE;
    compositeTransparentRenderPassDesc.depthMode.useDepthWrite = K_FALSE;
    compositeTransparentRenderPassDesc.blendMode.factorCount = 1;
    compositeTransparentRenderPassDesc.blendMode.srcFactors[0] = sBlendMode::eBlendFactor::SRC_ALPHA;
    compositeTransparentRenderPassDesc.blendMode.dstFactors[0] = sBlendMode::eBlendFactor::INV_SRC_ALPHA;
    compositeTransparentRenderPassDesc.renderTarget = _opaqueRenderTarget;
    _compositeTransparent = CreateRenderPass(compositeTransparentRenderPassDesc);

    sRenderPassDescriptor compositeFinalRenderPassDesc;
    auto& opaqueColorAttachments = _opaqueRenderTarget->GetColorAttachments();
    compositeFinalRenderPassDesc.inputVertexFormat = eCategory::VERTEX_BUFFER_FORMAT_NONE;
    compositeFinalRenderPassDesc.inputTextures.emplace_back(opaqueColorAttachments[0]);
    compositeFinalRenderPassDesc.inputTextureNames.emplace_back("ColorTexture");
    compositeFinalRenderPassDesc.shaderBase = nullptr;
    compositeFinalRenderPassDesc.shaderRenderPath = sRenderPassDescriptor::eRenderPath::QUAD_PATH;
    compositeFinalRenderPassDesc.shaderVertexPath = "C:/My/My Projects Programming/TritonEngine/runtime/data/shaders/main_vertex.shader";
    compositeFinalRenderPassDesc.shaderFragmentPath = "C:/My/My Projects Programming/TritonEngine/runtime/data/shaders/main_fragment.shader";
    compositeFinalRenderPassDesc.viewport = viewport;
    compositeFinalRenderPassDesc.depthMode.useDepthTest = K_FALSE;
    compositeFinalRenderPassDesc.depthMode.useDepthWrite = K_FALSE;
    compositeFinalRenderPassDesc.blendMode.factorCount = 1;
    compositeFinalRenderPassDesc.blendMode.srcFactors[0] = sBlendMode::eBlendFactor::ONE;
    compositeFinalRenderPassDesc.blendMode.dstFactors[0] = sBlendMode::eBlendFactor::ZERO;
    compositeFinalRenderPassDesc.renderTarget = nullptr;
    _compositeFinal = CreateRenderPass(compositeFinalRenderPassDesc);*/
}

void triton::cGraphics::Shutdown()
{
    _geometryStorage.Free();

    cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();
    iGraphicsResourceBackend* gfxResourceBackend = _context->GetBackend<iGraphicsResourceBackend>();
    iGraphicsPipelineBackend* gfxPipelineBackend = _context->GetBackend<iGraphicsPipelineBackend>();

    DestroyRenderPass(_compositeFinal);
    DestroyRenderPass(_compositeTransparent);
    DestroyRenderPass(_text);
    DestroyRenderPass(_transparent);
    DestroyRenderPass(_opaque);

    gfxPipelineBackend->DestroyRenderTarget(_transparentRenderTarget);
    gfxPipelineBackend->DestroyRenderTarget(_opaqueRenderTarget);

    gfxResourceBackend->DestroyTexture(_transparentRenderTarget->GetColorAttachments()[0]);
    gfxResourceBackend->DestroyTexture(_transparentRenderTarget->GetColorAttachments()[1]);
    gfxResourceBackend->DestroyTexture(_opaqueRenderTarget->GetColorAttachments()[0]);
    gfxResourceBackend->DestroyTexture(_opaqueRenderTarget->GetDepthAttachment());

    delete _materialsMap; // TODO: Temporary solution

    memoryAllocator->Deallocate(_transparentTextureAtlasTextures);
    memoryAllocator->Deallocate(_opaqueTextureAtlasTextures);
    memoryAllocator->Deallocate(_lights);
    memoryAllocator->Deallocate(_textMaterials);
    memoryAllocator->Deallocate(_transparentMaterials);
    memoryAllocator->Deallocate(_opaqueMaterials);
    memoryAllocator->Deallocate(_textInstances);
    memoryAllocator->Deallocate(_transparentInstances);
    memoryAllocator->Deallocate(_opaqueInstances);
    memoryAllocator->Deallocate(_indices);
    memoryAllocator->Deallocate(_vertices);

    gfxResourceBackend->DestroyBuffer(_transparentTextureAtlasTexturesBuffer);
    gfxResourceBackend->DestroyBuffer(_opaqueTextureAtlasTexturesBuffer);
    gfxResourceBackend->DestroyBuffer(_lightBuffer);
    gfxResourceBackend->DestroyBuffer(_transparentMaterialBuffer);
    gfxResourceBackend->DestroyBuffer(_textMaterialBuffer);
    gfxResourceBackend->DestroyBuffer(_opaqueMaterialBuffer);
    gfxResourceBackend->DestroyBuffer(_textInstanceBuffer);
    gfxResourceBackend->DestroyBuffer(_transparentInstanceBuffer);
    gfxResourceBackend->DestroyBuffer(_opaqueInstanceBuffer);
    gfxResourceBackend->DestroyBuffer(_indexBuffer);
    gfxResourceBackend->DestroyBuffer(_vertexBuffer);
}

// TODO: Remove material creation from cGraphics
/*cCacheObject<cMaterial> cGraphics::CreateMaterial(const std::string& id, cTextureAtlasTexture* diffuseTexture, const glm::vec4& diffuseColor, const glm::vec4& highlightColor, eCategory customShaderRenderPath, const std::string& customVertexFuncPath, const std::string& customFragmentFuncPath)
{
    cShader* customShader = nullptr;
    if (customVertexFuncPath != "" || customFragmentFuncPath != "")
    {
        std::string vertexFunc = "";
        std::string fragmentFunc = "";
        LoadShaderFiles(customVertexFuncPath, customFragmentFuncPath, vertexFunc, fragmentFunc);

        if (customShaderRenderPath == eCategory::RENDER_PATH_OPAQUE)
            customShader = _gfx->CreateShader(_opaque->GetShader(), vertexFunc, fragmentFunc);
        else if (customShaderRenderPath == eCategory::RENDER_PATH_TRANSPARENT)
            customShader = _gfx->CreateShader(_transparent->GetShader(), vertexFunc, fragmentFunc);
    }

    return _materialsCPU->Create(id, diffuseTexture, diffuseColor, highlightColor, customShader);
}*/

triton::cVertexArray* triton::cGraphics::CreateDefaultVertexArray()
{
    iGraphicsResourceBackend* gfxResourceBackend = _context->GetBackend<iGraphicsResourceBackend>();
    iGraphicsPipelineBackend* gfxPipelineBackend = _context->GetBackend<iGraphicsPipelineBackend>();
    cVertexArray* vertexArray = gfxPipelineBackend->CreateVertexArray();
    std::vector<cBuffer*> buffersToBind = { _vertexBuffer, _indexBuffer };

    gfxPipelineBackend->BindVertexArray(vertexArray);
    for (auto buffer : buffersToBind)
        gfxResourceBackend->BindBuffer(buffer);
    gfxPipelineBackend->BindDefaultInputLayout();
    gfxPipelineBackend->UnbindVertexArray();

    return vertexArray;
}

std::optional<triton::SGeometryView> triton::cGraphics::CreateGeometry(EGraphicsBufferFormat format, const u8* vertices, usize verticesByteSize, const u8* indices, usize indicesByteSize)
{
    return _geometryStorage->CreateGeometry(format, vertices, verticesByteSize, indices, indicesByteSize);
}

triton::XRenderPass* triton::cGraphics::CreateRenderPass(const SRenderPassDescriptor& desc)
{
    iGraphicsPipelineBackend* gfxPipelineBackend = _context->GetBackend<iGraphicsPipelineBackend>();
    XRenderPassGPU* renderPass = gfxPipelineBackend->CreateRenderPass(desc);

    return _context->Create<XRenderPass>(_context, desc, renderPass);
}

triton::sPrimitive* triton::cGraphics::CreatePrimitive(eCategory primitive)
{
    cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();
    const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();

    sPrimitive* primitiveObject = _context->Create<sPrimitive>(_context);

    if (primitive == eCategory::PRIMITIVE_TRIANGLE)
    {
        primitiveObject->_format = eCategory::VERTEX_BUFFER_FORMAT_POS_TEX_NRM_VEC3_VEC2_VEC3;
        primitiveObject->_vertices = (sVertex*)memoryAllocator->Allocate(sizeof(sVertex) * 3, caps->memoryAlignment);
        primitiveObject->_indices = (index*)memoryAllocator->Allocate(sizeof(index) * 3, caps->memoryAlignment);
        primitiveObject->_vertexCount = 3;
        primitiveObject->_indexCount = 3;
        primitiveObject->_verticesByteSize = sizeof(sVertex) * 3;
        primitiveObject->_indicesByteSize = sizeof(index) * 3;
        primitiveObject->_vertices[0]._position[0] = -1.0f; primitiveObject->_vertices[0]._position[1] = -1.0f; primitiveObject->_vertices[0]._position[2] = 0.0f;
        primitiveObject->_vertices[0]._texcoord[0] = 0.0f; primitiveObject->_vertices[0]._texcoord[1] = 0.0f;
        primitiveObject->_vertices[0]._normal[0] = 0.0f; primitiveObject->_vertices[0]._normal[1] = 0.0f; primitiveObject->_vertices[0]._normal[2] = 1.0f;
        primitiveObject->_vertices[1]._position[0] = 0.0f; primitiveObject->_vertices[1]._position[1] = 1.0f; primitiveObject->_vertices[1]._position[2] = 0.0f;
        primitiveObject->_vertices[1]._texcoord[0] = 0.5f; primitiveObject->_vertices[1]._texcoord[1] = 1.0f;
        primitiveObject->_vertices[1]._normal[0] = 0.0f; primitiveObject->_vertices[1]._normal[1] = 0.0f; primitiveObject->_vertices[1]._normal[2] = 1.0f;
        primitiveObject->_vertices[2]._position[0] = 1.0f; primitiveObject->_vertices[2]._position[1] = -1.0f; primitiveObject->_vertices[2]._position[2] = 0.0f;
        primitiveObject->_vertices[2]._texcoord[0] = 1.0f; primitiveObject->_vertices[2]._texcoord[1] = 0.0f;
        primitiveObject->_vertices[2]._normal[0] = 0.0f; primitiveObject->_vertices[2]._normal[1] = 0.0f; primitiveObject->_vertices[2]._normal[2] = 1.0f;
        primitiveObject->_indices[0] = 0;
        primitiveObject->_indices[1] = 1;
        primitiveObject->_indices[2] = 2;
    }
    else if (primitive == eCategory::PRIMITIVE_QUAD)
    {
        primitiveObject->_format = eCategory::VERTEX_BUFFER_FORMAT_POS_TEX_NRM_VEC3_VEC2_VEC3;
        primitiveObject->_vertices = (sVertex*)memoryAllocator->Allocate(sizeof(sVertex) * 4, caps->memoryAlignment);
        primitiveObject->_indices = (index*)memoryAllocator->Allocate(sizeof(index) * 6, caps->memoryAlignment);
        primitiveObject->_vertexCount = 4;
        primitiveObject->_indexCount = 6;
        primitiveObject->_verticesByteSize = sizeof(sVertex) * 4;
        primitiveObject->_indicesByteSize = sizeof(index) * 6;

        primitiveObject->_vertices[0]._position[0] = -1.0f; primitiveObject->_vertices[0]._position[1] = -1.0f; primitiveObject->_vertices[0]._position[2] = 0.0f;
        primitiveObject->_vertices[0]._texcoord[0] = 0.0f; primitiveObject->_vertices[0]._texcoord[1] = 0.0f;
        primitiveObject->_vertices[0]._normal[0] = 0.0f; primitiveObject->_vertices[0]._normal[1] = 0.0f; primitiveObject->_vertices[0]._normal[2] = 1.0f;
        primitiveObject->_vertices[1]._position[0] = -1.0f; primitiveObject->_vertices[1]._position[1] = 1.0f; primitiveObject->_vertices[1]._position[2] = 0.0f;
        primitiveObject->_vertices[1]._texcoord[0] = 0.0f; primitiveObject->_vertices[1]._texcoord[1] = 1.0f;
        primitiveObject->_vertices[1]._normal[0] = 0.0f; primitiveObject->_vertices[1]._normal[1] = 0.0f; primitiveObject->_vertices[1]._normal[2] = 1.0f;
        primitiveObject->_vertices[2]._position[0] = 1.0f; primitiveObject->_vertices[2]._position[1] = -1.0f; primitiveObject->_vertices[2]._position[2] = 0.0f;
        primitiveObject->_vertices[2]._texcoord[0] = 1.0f; primitiveObject->_vertices[2]._texcoord[1] = 0.0f;
        primitiveObject->_vertices[2]._normal[0] = 0.0f; primitiveObject->_vertices[2]._normal[1] = 0.0f; primitiveObject->_vertices[2]._normal[2] = 1.0f;
        primitiveObject->_vertices[3]._position[0] = 1.0f; primitiveObject->_vertices[3]._position[1] = 1.0f; primitiveObject->_vertices[3]._position[2] = 0.0f;
        primitiveObject->_vertices[3]._texcoord[0] = 1.0f; primitiveObject->_vertices[3]._texcoord[1] = 1.0f;
        primitiveObject->_vertices[3]._normal[0] = 0.0f; primitiveObject->_vertices[3]._normal[1] = 0.0f; primitiveObject->_vertices[3]._normal[2] = 1.0f;
        primitiveObject->_indices[0] = 0;
        primitiveObject->_indices[1] = 1;
        primitiveObject->_indices[2] = 2;
        primitiveObject->_indices[3] = 1;
        primitiveObject->_indices[4] = 3;
        primitiveObject->_indices[5] = 2;
    }

    return primitiveObject;
}

// TODO: Remove model creation from cGraphics
/*sModel* cGraphics::CreateModel(const std::string& filename)
{
    cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();
    const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();

    // Create model
    sModel* pModel = (sModel*)_context->Create<sModel>();

    model->_format = eCategory::VERTEX_BUFFER_FORMAT_POS_TEX_NRM_VEC3_VEC2_VEC3;

    // Load model
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        filename.data(),
        0
    );

    if (scene == nullptr)
        return nullptr;

    // Load vertices
    usize totalVertexCount = 0;
    model->_vertices = (sVertex*)memoryAllocator->Allocate(scene->mMeshes[0]->mNumVertices * sizeof(sVertex), caps->memoryAlignment);
    memset(model->_vertices, 0, scene->mMeshes[0]->mNumVertices * sizeof(sVertex));
    for (usize i = 0; i < scene->mMeshes[0]->mNumVertices; i++)
    {
        const aiVector3D pos = scene->mMeshes[0]->mVertices[i];
        const aiVector3D uv = scene->mMeshes[0]->mTextureCoords[0][i];
        const aiVector3D normal = scene->mMeshes[0]->HasNormals() ? scene->mMeshes[0]->mNormals[i] : aiVector3D(1.0f, 1.0f, 1.0f);

        model->_vertices[totalVertexCount]._position = glm::vec3(pos.x, pos.y, pos.z);
        model->_vertices[totalVertexCount]._texcoord = glm::vec2(uv.x, uv.y);
        model->_vertices[totalVertexCount]._normal = glm::vec3(normal.x, normal.y, normal.z);

        totalVertexCount += 1;
    }

    // Load indices
    usize totalIndexCount = 0;
    model->_indices = (index*)memoryAllocator->Allocate(scene->mMeshes[0]->mNumFaces * 3 * sizeof(index), caps->memoryAlignment);
    memset(model->_indices, 0, scene->mMeshes[0]->mNumFaces * 3 * sizeof(index));
    for (usize i = 0; i < scene->mMeshes[0]->mNumFaces; i++)
    {
        const aiFace face = scene->mMeshes[0]->mFaces[i];
        for (usize j = 0; j < face.mNumIndices; j++)
        {
            model->_indices[totalIndexCount] = face.mIndices[j];

            totalIndexCount += 1;
        }
    }

    model->_vertexCount = totalVertexCount;
    model->_verticesByteSize = totalVertexCount * sizeof(sVertex);
    model->_indexCount = totalIndexCount;
    model->_indicesByteSize = totalIndexCount * sizeof(index);

    return model;
}*/

// TODO: Remove material finding from cGraphics
/*cCacheObject<cMaterial> cGraphics::FindMaterial(const cTag& id)
{
    return _materialsCPU->Find(id);
}*/

// TODO: Remove material destroying from cGraphics
/*void cGraphics::DestroyMaterial(const cTag& id)
{
    cCacheObject<cMaterial> material = _materialsCPU->Find(id);
    cMaterial* matObj = material.object;
    if (matObj == nullptr)
        return;

    if (matObj->GetCustomShader() != nullptr)
        _gfx->DestroyShader(matObj->GetCustomShader());

    _materialsCPU->Destroy(id);
}*/

void triton::cGraphics::DestroyGeometry(sVertexBufferGeometry* geometry)
{
    _context->Destroy<sVertexBufferGeometry>(geometry);
}

void triton::cGraphics::DestroyRenderPass(cRenderPass* renderPass)
{
    iGraphicsPipelineBackend* gfxPipelineBackend = _context->GetBackend<iGraphicsPipelineBackend>();
    gfxPipelineBackend->DestroyRenderPass(renderPass->GetRenderPassGPU());
    _context->Destroy<cRenderPass>(renderPass);
}

void triton::cGraphics::DestroyPrimitive(sPrimitive* primitiveObject)
{
    cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();

    if (primitiveObject->_vertices)
        memoryAllocator->Deallocate(primitiveObject->_vertices);
    if (primitiveObject->_indices)
        memoryAllocator->Deallocate(primitiveObject->_indices);
    _context->Destroy<sPrimitive>(primitiveObject);
}

void triton::cGraphics::ClearGeometryBuffer()
{
    _verticesByteSize = 0;
    _indicesByteSize = 0;
}

void triton::cGraphics::ClearRenderTarget(
    const cRenderPass* renderPass,
    types::boolean clearColor,
    usize bufferIndex,
    const glm::vec4& color,
    types::boolean clearDepth,
    f32 depth
)
{
    iGraphicsPipelineBackend* gfxPipelineBackend = _context->GetBackend<iGraphicsPipelineBackend>();
    iGraphicsDrawcallBackend* gfxDrawcallBackend = _context->GetBackend<iGraphicsDrawcallBackend>();

    gfxPipelineBackend->BindRenderPass(renderPass);
    if (clearColor == K_TRUE)
        gfxDrawcallBackend->ClearFramebufferColor(bufferIndex, cVector4(color));
    if (clearDepth == K_TRUE)
        gfxDrawcallBackend->ClearFramebufferDepth(depth);
    gfxPipelineBackend->UnbindRenderPass(renderPass);
}

void triton::cGraphics::ClearRenderTargets(const glm::vec4& clearColor, f32 clearDepth)
{
    iGraphicsPipelineBackend* gfxPipelineBackend = _context->GetBackend<iGraphicsPipelineBackend>();
    iGraphicsDrawcallBackend* gfxDrawcallBackend = _context->GetBackend<iGraphicsDrawcallBackend>();

    gfxPipelineBackend->BindRenderPass(_opaque);
    gfxDrawcallBackend->ClearFramebufferColor(0, cVector4(clearColor));
    gfxDrawcallBackend->ClearFramebufferDepth(clearDepth);

    gfxPipelineBackend->BindRenderPass(_transparent);
    gfxDrawcallBackend->ClearFramebufferColor(0, cVector4(0.0f));
    gfxDrawcallBackend->ClearFramebufferColor(1, cVector4(1.0f));

    gfxPipelineBackend->BindRenderPass(_text);
    gfxDrawcallBackend->ClearFramebufferColor(0, cVector4(clearColor));
}

void triton::cGraphics::ResizeRenderTargets(const glm::vec2& size)
{
    iGraphicsPipelineBackend* gfxPipelineBackend = _context->GetBackend<iGraphicsPipelineBackend>();

    gfxPipelineBackend->UnbindRenderPass(_opaque);
    gfxPipelineBackend->UnbindRenderPass(_transparent);

    _opaque->ResizeViewport(size);
    _opaque->ResizeColorAttachments(size);
    _opaque->ResizeDepthAttachment(size);
    _transparent->ResizeViewport(size);
    _transparent->ResizeColorAttachments(size);
    cTexture* opaqueDepthAttachment = _opaque->GetRenderTarget()->GetDepthAttachment();
    _transparent->GetRenderTarget()->SetDepthAttachment(opaqueDepthAttachment);

    _text->ResizeViewport(size);

    cRenderTarget* newOpaqueRenderTarget = _opaque->GetRenderTarget();
    cRenderTarget* newTransparentRenderTarget = _transparent->GetRenderTarget();
    gfxPipelineBackend->UpdateRenderTargetBuffers(newOpaqueRenderTarget);
    gfxPipelineBackend->UpdateRenderTargetBuffers(newTransparentRenderTarget);
    _opaque->SetRenderTarget(newOpaqueRenderTarget);
    _transparent->SetRenderTarget(newTransparentRenderTarget);

    _compositeTransparent->ResizeViewport(size);
    auto& transparentColorAttachments = _transparent->GetRenderTarget()->GetColorAttachments();
    _compositeTransparent->SetInputTexture(0, transparentColorAttachments[0]);
    _compositeTransparent->SetInputTexture(1, transparentColorAttachments[1]);
    //_compositeTransparent->_desc._inputTextureNames[0] = "AccumulationTexture";
    //_compositeTransparent->_desc._inputTextureNames[1] = "RevealageTexture";

    _compositeFinal->ResizeViewport(size);
    auto& opaqueColorAttachments = _opaque->GetRenderTarget()->GetColorAttachments();
    _compositeFinal->SetInputTexture(0, opaqueColorAttachments[0]);
    //_compositeFinal->_desc._inputTextureNames[0] = "ColorTexture";
}

void triton::cGraphics::LoadShaderFiles(const std::string& vertexFuncPath, const std::string& fragmentFuncPath, std::string& vertexFunc, std::string& fragmentFunc)
{
    cFileSystem* fileSystem = _context->GetSubsystem<cFileSystem>();

    cDataFile* vertexFuncFile = fileSystem->CreateDataFile(vertexFuncPath, K_TRUE);
    cDataFile* fragmentFuncFile = fileSystem->CreateDataFile(fragmentFuncPath, K_TRUE);

    vertexFunc = std::string((const char*)vertexFuncFile->GetData());
    fragmentFunc = std::string((const char*)fragmentFuncFile->GetData());

    fileSystem->DestroyDataFile(vertexFuncFile);
    fileSystem->DestroyDataFile(fragmentFuncFile);
}

void triton::cGraphics::UpdateLights()
{
    /*_lightsByteSize = 16; // because vec4 (16 bytes) goes first (contains light count)
    memset(_lights, 0, 16 + (sizeof(sLightInstance) * 16));

    glm::uvec4 lightCount = glm::uvec4(0);

    for (auto& it : _app->GetGameObjectManager()->GetObjects())
    {
        if (it.GetLight() != nullptr)
        {
            sLightInstance li(&it);

            memcpy((void*)((usize)_lights + (usize)_lightsByteSize), &li, sizeof(sLightInstance));
            _lightsByteSize += sizeof(sLightInstance);

            lightCount.x += 1;
        }
    }

    memcpy((void*)(usize)_lights, &lightCount, sizeof(glm::uvec4));

    _context->WriteBuffer(_lightBuffer, 0, _lightsByteSize, _lights);*/
}

// TODO: Implement new CPU->GPU geometry buffer communication
/*void cGraphics::WriteObjectsToOpaqueBuffers(cIdVector<cGameObject>& objects, cRenderPass* renderPass)
{
    _opaqueInstanceCount = 0;
    _opaqueInstancesByteSize = 0;
    _opaqueMaterialsByteSize = 0;
    _opaqueTextureAtlasTexturesByteSize = 0;
    _materialsMap->clear();

    cGameObject* objectsArray = objects.GetElements();

    for (usize i = 0; i < objects.GetElementCount(); i++)
    {
        const cGameObject& go = objectsArray[i];

        cTransform transform(&go);
        transform.Transform();

        s32 materialIndex = -1;
        cMaterial* material = go.GetMaterial();
        sVertexBufferGeometry* geometry = go.GetGeometry();

        if (geometry == nullptr)
            continue;

        if (material != nullptr)
        {
            auto it = _materialsMap->find(material);
            if (it == _materialsMap->end())
            {
                materialIndex = _materialsMap->size();

                cMaterialInstance mi(materialIndex, material);

                _materialsMap->insert({ material, materialIndex });

                memcpy((void*)((usize)_opaqueMaterials + (usize)_opaqueMaterialsByteSize), &mi, sizeof(cMaterialInstance));
                _opaqueMaterialsByteSize += sizeof(cMaterialInstance);
            }
            else
            {
                materialIndex = it->second;
            }
        }

        const sRenderInstance ri(materialIndex, transform);

        memcpy((void*)((usize)_opaqueInstances + _opaqueInstancesByteSize), &ri, sizeof(sRenderInstance));
        _opaqueInstancesByteSize += sizeof(sRenderInstance);

        _opaqueInstanceCount += 1;
    }

    const std::vector<cTextureAtlasTexture*>& renderPassTextureAtlasTextures = renderPass->GetInputTextureAtlasTextures();
    for (const auto textureAtlasTexture : renderPassTextureAtlasTextures)
    {
        sTextureAtlasTextureGPU tatGPU;
        tatGPU._textureInfo = glm::vec4(
            textureAtlasTexture->GetOffset().x,
            textureAtlasTexture->GetOffset().y,
            textureAtlasTexture->GetSize().x,
            textureAtlasTexture->GetSize().y
        );
        tatGPU._textureLayerInfo = textureAtlasTexture->GetOffset().z;

        memcpy((void*)((usize)_opaqueTextureAtlasTextures + _opaqueTextureAtlasTexturesByteSize), &tatGPU, sizeof(sTextureAtlasTextureGPU));
        _opaqueTextureAtlasTexturesByteSize += sizeof(sTextureAtlasTextureGPU);
    }

    _gfx->WriteBuffer(_opaqueInstanceBuffer, 0, _opaqueInstancesByteSize, _opaqueInstances);
    _gfx->WriteBuffer(_opaqueMaterialBuffer, 0, _opaqueMaterialsByteSize, _opaqueMaterials);
    _gfx->WriteBuffer(_opaqueTextureAtlasTexturesBuffer, 0, _opaqueTextureAtlasTexturesByteSize, _opaqueTextureAtlasTextures);
}

void cGraphics::WriteObjectsToTransparentBuffers(cIdVector<cGameObject>& objects, cRenderPass* renderPass)
{
    _transparentInstanceCount = 0;
    _transparentInstancesByteSize = 0;
    _transparentMaterialsByteSize = 0;
    _materialsMap->clear();

    cGameObject* objectsArray = objects.GetElements();

    for (usize i = 0; i < objects.GetElementCount(); i++)
    {
        const cGameObject& go = objectsArray[i];

        cTransform transform(&go);
        transform.Transform();

        s32 materialIndex = -1;
        cMaterial* material = go.GetMaterial();
        sVertexBufferGeometry* geometry = go.GetGeometry();

        if (geometry == nullptr)
            continue;

        if (material != nullptr)
        {
            auto it = _materialsMap->find(material);
            if (it == _materialsMap->end())
            {
                materialIndex = _materialsMap->size();

                cMaterialInstance mi(materialIndex, material);

                _materialsMap->insert({ material, materialIndex });

                memcpy((void*)((usize)_transparentMaterials + _transparentMaterialsByteSize), &mi, sizeof(cMaterialInstance));
                _transparentMaterialsByteSize += sizeof(cMaterialInstance);
            }
            else
            {
                materialIndex = it->second;
            }
        }

        const sRenderInstance ri(materialIndex, transform);

        memcpy((void*)((usize)_transparentInstances + _transparentInstancesByteSize), &ri, sizeof(sRenderInstance));
        _transparentInstancesByteSize += sizeof(sRenderInstance);

        _transparentInstanceCount += 1;
    }

    const std::vector<cTextureAtlasTexture*>& renderPassTextureAtlasTextures = renderPass->GetInputTextureAtlasTextures();
    for (const auto textureAtlasTexture : renderPassTextureAtlasTextures)
    {
        sTextureAtlasTextureGPU tatGPU;
        tatGPU._textureInfo = glm::vec4(
            textureAtlasTexture->GetOffset().x,
            textureAtlasTexture->GetOffset().y,
            textureAtlasTexture->GetSize().x,
            textureAtlasTexture->GetSize().y
        );
        tatGPU._textureLayerInfo = textureAtlasTexture->GetOffset().z;

        memcpy((void*)((usize)_transparentTextureAtlasTextures + (usize)_transparentTextureAtlasTexturesByteSize), &tatGPU, sizeof(sTextureAtlasTextureGPU));
        _transparentTextureAtlasTexturesByteSize += sizeof(sTextureAtlasTextureGPU);
    }

    _gfx->WriteBuffer(_transparentInstanceBuffer, 0, _transparentInstancesByteSize, _transparentInstances);
    _gfx->WriteBuffer(_transparentMaterialBuffer, 0, _transparentMaterialsByteSize, _transparentMaterials);
    _gfx->WriteBuffer(_transparentTextureAtlasTexturesBuffer, 0, _transparentTextureAtlasTexturesByteSize, _transparentTextureAtlasTextures);
}*/

void triton::cGraphics::DrawGeometryOpaque(const sVertexBufferGeometry* geometry, const cGameObject* cameraObject, cRenderPass* renderPass)
{
    iGraphicsPipelineBackend* gfxPipelineBackend = _context->GetBackend<iGraphicsPipelineBackend>();
    iGraphicsDrawcallBackend* gfxDrawcallBackend = _context->GetBackend<iGraphicsDrawcallBackend>();

    if (renderPass == nullptr)
    {
        gfxPipelineBackend->BindRenderPass(_opaque);
        gfxPipelineBackend->SetShaderUniform(_opaque->GetShader(), "ViewProjection", cameraObject->GetViewProjectionMatrix());
    }
    else
    {
        gfxPipelineBackend->BindRenderPass(renderPass);
        gfxPipelineBackend->SetShaderUniform(renderPass->GetShader(), "ViewProjection", cameraObject->GetViewProjectionMatrix());
    }

    gfxDrawcallBackend->Draw(
        geometry->_indexCount,
        geometry->_offsetVertex,
        geometry->_offsetIndex,
       _opaqueInstanceCount
    );

    if (renderPass == nullptr)
        gfxPipelineBackend->UnbindRenderPass(_opaque);
    else
        gfxPipelineBackend->UnbindRenderPass(renderPass);
}

void triton::cGraphics::DrawGeometryOpaque(const sVertexBufferGeometry* geometry, const cGameObject* cameraObject, cShader* singleShader)
{
    iGraphicsPipelineBackend* gfxPipelineBackend = _context->GetBackend<iGraphicsPipelineBackend>();
    iGraphicsDrawcallBackend* gfxDrawcallBackend = _context->GetBackend<iGraphicsDrawcallBackend>();

    gfxPipelineBackend->BindRenderPass(_opaque, singleShader);

    if (singleShader == nullptr)
        gfxPipelineBackend->SetShaderUniform(_opaque->GetShader(), "ViewProjection", cameraObject->GetViewProjectionMatrix());
    else
        gfxPipelineBackend->SetShaderUniform(singleShader, "ViewProjection", cameraObject->GetViewProjectionMatrix());

    gfxDrawcallBackend->Draw(
        geometry->_indexCount,
        geometry->_offsetVertex,
        geometry->_offsetIndex,
        _opaqueInstanceCount
    );

    gfxPipelineBackend->UnbindRenderPass(_opaque);
}

void triton::cGraphics::DrawGeometryTransparent(const sVertexBufferGeometry* geometry, const std::vector<cGameObject>& objects, const cGameObject* cameraObject, cRenderPass* renderPass)
{
    iGraphicsPipelineBackend* gfxPipelineBackend = _context->GetBackend<iGraphicsPipelineBackend>();
    iGraphicsDrawcallBackend* gfxDrawcallBackend = _context->GetBackend<iGraphicsDrawcallBackend>();

    if (renderPass == nullptr)
    {
        gfxPipelineBackend->BindRenderPass(_transparent);
        gfxPipelineBackend->SetShaderUniform(_transparent->GetShader(), "ViewProjection", cameraObject->GetViewProjectionMatrix());
    }
    else
    {
        gfxPipelineBackend->BindRenderPass(renderPass);
        gfxPipelineBackend->SetShaderUniform(renderPass->GetShader(), "ViewProjection", cameraObject->GetViewProjectionMatrix());
    }

    gfxDrawcallBackend->Draw(
        geometry->_indexCount,
        geometry->_offsetVertex,
        geometry->_offsetIndex,
        _transparentInstanceCount
    );

    if (renderPass == nullptr)
        gfxPipelineBackend->UnbindRenderPass(_transparent);
    else
        gfxPipelineBackend->UnbindRenderPass(renderPass);

    CompositeTransparent();
}

void triton::cGraphics::DrawGeometryTransparent(const sVertexBufferGeometry* geometry, const cGameObject* cameraObject, cShader* singleShader)
{
    iGraphicsPipelineBackend* gfxPipelineBackend = _context->GetBackend<iGraphicsPipelineBackend>();
    iGraphicsDrawcallBackend* gfxDrawcallBackend = _context->GetBackend<iGraphicsDrawcallBackend>();

    gfxPipelineBackend->BindRenderPass(_transparent, singleShader);

    if (singleShader != nullptr)
        gfxPipelineBackend->SetShaderUniform(singleShader, "ViewProjection", cameraObject->GetViewProjectionMatrix());
    else
        gfxPipelineBackend->SetShaderUniform(_transparent->GetShader(), "ViewProjection", cameraObject->GetViewProjectionMatrix());

    gfxDrawcallBackend->Draw(
        geometry->_indexCount,
        geometry->_offsetVertex,
        geometry->_offsetIndex,
        _transparentInstanceCount
    );

    gfxPipelineBackend->UnbindRenderPass(_transparent);
}

// TODO: Implement new text drawing approach
/*void cGraphics::DrawTexts(const std::vector<cGameObject>& objects)
{
    for (auto& it : objects)
    {
        if (it.GetText() == nullptr)
            continue;

        const cText* text = it.GetText();
        const cFontFace* textFont = text->GetFont();
        const std::string& textString = text->GetText();
        const auto& alphabet = textFont->GetAlphabet();
        const cTexture* atlas = text->GetFont()->GetAtlas();

        _textInstancesByteSize = 0;
        _materialsMap->clear();

        const cTransform transform(&it);

        IApplication* app = _context->GetSubsystem<cEngine>()->GetApplication();
        const glm::vec2 windowSize = app->GetWindow()->GetSize();
        const glm::vec2 textPosition = glm::vec2((transform._position.x * 2.0f) - 1.0f, (transform._position.y * 2.0f) - 1.0f);
        const glm::vec2 textScale = glm::vec2(
            (1.0f / windowSize.x) * it.GetTransform()->_scale.x,
            (1.0f / windowSize.y) * it.GetTransform()->_scale.y
        );

        const usize charCount = textString.length();
        usize actualCharCount = 0;
        glm::vec2 offset = glm::vec2(0.0f);
        for (usize i = 0; i < charCount; i++)
        {
            const u8 glyphChar = textString[i];

            if (glyphChar == '\t')
            {
                offset.x += textFont->GetOffsetTab() * textScale.x;
                continue;
            }
            else if (glyphChar == '\n')
            {
                offset.x = 0.0f;
                offset.y -= textFont->GetOffsetNewline() * textScale.y;
                continue;
            }
            else if (glyphChar == ' ')
            {
                offset.x += textFont->GetOffsetSpace() * textScale.x;
                continue;
            }

            const auto alphabetEntry = alphabet.find(glyphChar);
            if (alphabetEntry == alphabet.end())
                continue;
            const sGlyph& glyph = alphabetEntry->second;

            sTextInstance t;
            t._info.x = textPosition.x + offset.x;
            t._info.y = textPosition.y + (offset.y - (float)((glyph._height - glyph._top) * textScale.y));
            t._info.z = (float)glyph._width * textScale.x;
            t._info.w = (float)glyph._height * textScale.y;
            t._atlasInfo.x = (float)glyph._atlasXOffset / (float)atlas->GetWidth();
            t._atlasInfo.y = (float)glyph._atlasYOffset / (float)atlas->GetHeight();
            t._atlasInfo.z = (float)glyph._width / (float)atlas->GetWidth();
            t._atlasInfo.w = (float)glyph._height / (float)atlas->GetHeight();

            offset.x += glyph._advanceX * textScale.x;

            memcpy((void*)((usize)_textInstances + _textInstancesByteSize), &t, sizeof(sTextInstance));
            _textInstancesByteSize += sizeof(sTextInstance);

            actualCharCount += 1;
        }

        const cMaterialInstance mi(0, it.GetMaterial());
        memcpy(_textMaterials, &mi, sizeof(cMaterialInstance));
        _textMaterialsByteSize += sizeof(cMaterialInstance);

        _gfx->WriteBuffer(_textInstanceBuffer, 0, _textInstancesByteSize, _textInstances);
        _gfx->WriteBuffer(_textMaterialBuffer, 0, _textMaterialsByteSize, _textMaterials);

        _gfx->BindRenderPass(_text);
        _gfx->BindTexture(_text->GetShader(), "FontAtlas", atlas, 0);
        _gfx->DrawQuads(actualCharCount);
        _gfx->UnbindRenderPass(_text);
    }
}*/

void triton::cGraphics::CompositeTransparent()
{
    iGraphicsPipelineBackend* gfxPipelineBackend = _context->GetBackend<iGraphicsPipelineBackend>();
    iGraphicsDrawcallBackend* gfxDrawcallBackend = _context->GetBackend<iGraphicsDrawcallBackend>();

    gfxPipelineBackend->BindRenderPass(_compositeTransparent);
    gfxDrawcallBackend->DrawQuad();
    gfxPipelineBackend->UnbindRenderPass(_compositeTransparent);
}

void triton::cGraphics::CompositeFinal()
{
    iGraphicsPipelineBackend* gfxPipelineBackend = _context->GetBackend<iGraphicsPipelineBackend>();
    iGraphicsDrawcallBackend* gfxDrawcallBackend = _context->GetBackend<iGraphicsDrawcallBackend>();

    gfxPipelineBackend->BindRenderPass(_compositeFinal);
    gfxDrawcallBackend->DrawQuad();
    gfxPipelineBackend->UnbindRenderPass(_compositeFinal);

    gfxPipelineBackend->UnbindShader();
}

void triton::cGraphics::CreateDefaultRenderTargets()
{
    iGraphicsResourceBackend* gfxResourceBackend = _context->GetBackend<iGraphicsResourceBackend>();
    iGraphicsPipelineBackend* gfxPipelineBackend = _context->GetBackend<iGraphicsPipelineBackend>();

    cVector2 windowSize = _context->GetSubsystem<cInput>()->GetWindows()->At(0)->GetSize();

    cTexture* color = gfxResourceBackend->CreateTexture(
        cVector3(windowSize.GetX(), windowSize.GetY(), 0),
        cTexture::eDimension::TEXTURE_2D,
        cTexture::eFormat::RGBA8,
        nullptr,
        0
    );
    cTexture* accumulation = gfxResourceBackend->CreateTexture(
        cVector3(windowSize.GetX(), windowSize.GetY(), 0),
        cTexture::eDimension::TEXTURE_2D,
        cTexture::eFormat::RGBA16F,
        nullptr,
        0
    );
    cTexture* revealage = gfxResourceBackend->CreateTexture(
        cVector3(windowSize.GetX(), windowSize.GetY(), 0),
        cTexture::eDimension::TEXTURE_2D,
        cTexture::eFormat::R8F,
        nullptr,
        0
    );
    cTexture* depth = gfxResourceBackend->CreateTexture(
        cVector3(windowSize.GetX(), windowSize.GetY(), 0),
        cTexture::eDimension::TEXTURE_2D,
        cTexture::eFormat::DEPTH_STENCIL,
        nullptr,
        0
    );

    _opaqueRenderTarget = gfxPipelineBackend->CreateRenderTarget({ color }, depth);
    _transparentRenderTarget = gfxPipelineBackend->CreateRenderTarget({ accumulation, revealage }, depth);
}