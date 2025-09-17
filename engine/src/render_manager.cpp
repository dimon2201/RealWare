#include <GL/glew.h>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include "../../thirdparty/glm/glm/gtc/quaternion.hpp"
#include "../../thirdparty/glm/glm/gtx/quaternion.hpp"
#include "render_manager.hpp"
#include "render_context.hpp"
#include "texture_manager.hpp"
#include "gameobject_manager.hpp"
#include "font_manager.hpp"
#include "filesystem_manager.hpp"
#include "application.hpp"
#include "memory_pool.hpp"

using namespace types;

namespace realware
{
    using namespace app;
    using namespace game;
    using namespace font;
    using namespace fs;
    using namespace utils;

    namespace render
    {
        sTransform::sTransform(const cGameObject* const gameObject)
        {
            Use2D = gameObject->GetIs2D();
            Position = gameObject->GetPosition();
            Rotation = gameObject->GetRotation();
            Scale = gameObject->GetScale();
        }

        void sTransform::Transform()
        {
            const glm::quat quatX = glm::angleAxis(Rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
            const glm::quat quatY = glm::angleAxis(Rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
            const glm::quat quatZ = glm::angleAxis(Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

            World = glm::translate(glm::mat4(1.0f), Position) * glm::toMat4(quatZ * quatY * quatX) * glm::scale(glm::mat4(1.0f), Scale);
        }

        sRenderInstance::sRenderInstance(s32 materialIndex, const sTransform& transform)
        {
            Use2D = transform.Use2D;
            MaterialIndex = materialIndex;
            World = transform.World;
        }

        sMaterialInstance::sMaterialInstance(s32 materialIndex, const sMaterial* const material)
        {
            BufferIndex = materialIndex;
            DiffuseColor = material->DiffuseColor;
            HighlightColor = material->HighlightColor;
        }

        void sMaterialInstance::SetDiffuseTexture(const sTextureAtlasTexture& area)
        {
            DiffuseTextureLayerInfo = area.Offset.z;
            DiffuseTextureInfo = glm::vec4(area.Offset.x, area.Offset.y, area.Size.x, area.Size.y);
        }

        sLightInstance::sLightInstance(const cGameObject* const object)
        {
            const sLight* const light = object->GetLight();
            Position = glm::vec4(object->GetPosition(), 0.0f);
            Color = glm::vec4(light->Color, 0.0f);
            DirectionAndScale = glm::vec4(light->Direction, light->Scale);
            Attenuation = glm::vec4(
                light->AttenuationConstant,
                light->AttenuationLinear,
                light->AttenuationQuadratic,
                0.0f
            );
        }

        mRender::mRender(const cApplication* const app, const cRenderContext* const context) :
            _app((cApplication*)app), _context((cRenderContext*)context), _materialsCPU((cApplication*)app, ((cApplication*)app)->GetDesc()->MaxMaterialCount)
        {
            cMemoryPool* const memoryPool = _app->GetMemoryPool();

            sApplicationDescriptor* desc = _app->GetDesc();
            const glm::vec2 windowSize = _app->GetWindowSize();

            _vertexBuffer = _context->CreateBuffer(desc->VertexBufferSize, sBuffer::eType::VERTEX, nullptr);
            _indexBuffer = _context->CreateBuffer(desc->IndexBufferSize, sBuffer::eType::INDEX, nullptr);
            _instanceBuffer = _context->CreateBuffer(desc->InstanceBufferSize, sBuffer::eType::LARGE, nullptr);
            _instanceBuffer->Slot = 0;
            _materialBuffer = _context->CreateBuffer(desc->MaterialBufferSize, sBuffer::eType::LARGE, nullptr);
            _materialBuffer->Slot = 1;
            _lightBuffer = _context->CreateBuffer(desc->LightBufferSize, sBuffer::eType::LARGE, nullptr);
            _lightBuffer->Slot = 2;
            _textureAtlasTexturesBuffer = _context->CreateBuffer(desc->TextureAtlasTexturesBufferSize, sBuffer::eType::LARGE, nullptr);
            _textureAtlasTexturesBuffer->Slot = 3;
            _vertices = memoryPool->Allocate(desc->VertexBufferSize);
            _verticesByteSize = 0;
            _indices = memoryPool->Allocate(desc->IndexBufferSize);
            _indicesByteSize = 0;
            _instances = memoryPool->Allocate(desc->InstanceBufferSize);
            _instancesByteSize = 0;
            _materials = memoryPool->Allocate(desc->MaterialBufferSize);
            _materialsByteSize = 0;
            _lights = memoryPool->Allocate(desc->LightBufferSize);
            _lightsByteSize = 0;
            _textureAtlasTextures = memoryPool->Allocate(desc->TextureAtlasTexturesBufferSize);
            _textureAtlasTexturesByteSize = 0;
            std::unordered_map<render::sMaterial*, s32>* pMaterialsMap = (std::unordered_map<render::sMaterial*, s32>*)_app->GetMemoryPool()->Allocate(sizeof(std::unordered_map<render::sMaterial*, s32>));
            _materialsMap = new (pMaterialsMap) std::unordered_map<render::sMaterial*, s32>();

            sTexture* color = _context->CreateTexture(windowSize.x, windowSize.y, 0, render::sTexture::eType::TEXTURE_2D, render::sTexture::eFormat::RGBA8, nullptr);
            sTexture* accumulation = _context->CreateTexture(windowSize.x, windowSize.y, 0, render::sTexture::eType::TEXTURE_2D, render::sTexture::eFormat::RGBA16F, nullptr);
            sTexture* ui = _context->CreateTexture(windowSize.x, windowSize.y, 0, render::sTexture::eType::TEXTURE_2D, render::sTexture::eFormat::RGBA8, nullptr);
            sTexture* revealage = _context->CreateTexture(windowSize.x, windowSize.y, 0, render::sTexture::eType::TEXTURE_2D, render::sTexture::eFormat::R8F, nullptr);
            sTexture* depth = _context->CreateTexture(windowSize.x, windowSize.y, 0, render::sTexture::eType::TEXTURE_2D, render::sTexture::eFormat::DEPTH_STENCIL, nullptr);

            _opaqueRenderTarget = _context->CreateRenderTarget({ color }, depth);
            _transparentRenderTarget = _context->CreateRenderTarget({ accumulation, revealage }, depth);

            {
                sRenderPass::sDescriptor renderPassDesc;
                renderPassDesc.InputVertexFormat = Category::VERTEX_BUFFER_FORMAT_POS_TEX_NRM_VEC3_VEC2_VEC3;
                renderPassDesc.InputBuffers.emplace_back(mRender::GetVertexBuffer());
                renderPassDesc.InputBuffers.emplace_back(mRender::GetIndexBuffer());
                renderPassDesc.InputBuffers.emplace_back(mRender::GetInstanceBuffer());
                renderPassDesc.InputBuffers.emplace_back(mRender::GetMaterialBuffer());
                renderPassDesc.InputBuffers.emplace_back(mRender::GetLightBuffer());
                renderPassDesc.InputBuffers.emplace_back(mRender::GetTextureAtlasTexturesBuffer());
                renderPassDesc.InputTextures.emplace_back(_app->GetTextureManager()->GetAtlas());
                renderPassDesc.InputTextureNames.emplace_back("TextureAtlas");
                renderPassDesc.ShaderBase = nullptr;
                renderPassDesc.ShaderRenderPath = Category::RENDER_PATH_OPAQUE;
                renderPassDesc.ShaderVertexPath = "C:/DDD/RealWare/build_vs/samples/Sample01/Debug/data/shaders/main_vertex.shader";
                renderPassDesc.ShaderFragmentPath = "C:/DDD/RealWare/build_vs/samples/Sample01/Debug/data/shaders/main_fragment.shader";
                renderPassDesc.RenderTarget = _opaqueRenderTarget;
                renderPassDesc.Viewport = glm::vec4(0.0f, 0.0f, windowSize);
                renderPassDesc.DepthMode.UseDepthTest = K_TRUE;
                renderPassDesc.DepthMode.UseDepthWrite = K_TRUE;
                renderPassDesc.BlendMode.FactorCount = 1;
                renderPassDesc.BlendMode.SrcFactors[0] = sBlendMode::eFactor::ONE;
                renderPassDesc.BlendMode.DstFactors[0] = sBlendMode::eFactor::ZERO;
                _opaque = _context->CreateRenderPass(renderPassDesc);
            }
            {
                sRenderPass::sDescriptor renderPassDesc;
                renderPassDesc.InputVertexFormat = Category::VERTEX_BUFFER_FORMAT_POS_TEX_NRM_VEC3_VEC2_VEC3;
                renderPassDesc.InputBuffers.emplace_back(mRender::GetVertexBuffer());
                renderPassDesc.InputBuffers.emplace_back(mRender::GetIndexBuffer());
                renderPassDesc.InputBuffers.emplace_back(mRender::GetInstanceBuffer());
                renderPassDesc.InputBuffers.emplace_back(mRender::GetMaterialBuffer());
                renderPassDesc.InputBuffers.emplace_back(mRender::GetTextureAtlasTexturesBuffer());
                renderPassDesc.InputTextures.emplace_back(_app->GetTextureManager()->GetAtlas());
                renderPassDesc.InputTextureNames.emplace_back("TextureAtlas");
                renderPassDesc.ShaderBase = nullptr;
                renderPassDesc.ShaderRenderPath = Category::RENDER_PATH_TRANSPARENT;
                renderPassDesc.ShaderVertexPath = "C:/DDD/RealWare/build_vs/samples/Sample01/Debug/data/shaders/main_vertex.shader";
                renderPassDesc.ShaderFragmentPath = "C:/DDD/RealWare/build_vs/samples/Sample01/Debug/data/shaders/main_fragment.shader";
                renderPassDesc.RenderTarget = _transparentRenderTarget;
                renderPassDesc.Viewport = glm::vec4(0.0f, 0.0f, windowSize);
                renderPassDesc.DepthMode.UseDepthTest = K_TRUE;
                renderPassDesc.DepthMode.UseDepthWrite = K_FALSE;
                renderPassDesc.BlendMode.FactorCount = 2;
                renderPassDesc.BlendMode.SrcFactors[0] = sBlendMode::eFactor::ONE;
                renderPassDesc.BlendMode.DstFactors[0] = sBlendMode::eFactor::ONE;
                renderPassDesc.BlendMode.SrcFactors[1] = sBlendMode::eFactor::ZERO;
                renderPassDesc.BlendMode.DstFactors[1] = sBlendMode::eFactor::INV_SRC_COLOR;
                _transparent = _context->CreateRenderPass(renderPassDesc);
            }
            {
                sRenderPass::sDescriptor renderPassDesc;
                renderPassDesc.InputVertexFormat = Category::VERTEX_BUFFER_FORMAT_NONE;
                renderPassDesc.InputBuffers.emplace_back(mRender::GetInstanceBuffer());
                renderPassDesc.InputBuffers.emplace_back(mRender::GetMaterialBuffer());
                renderPassDesc.InputBuffers.emplace_back(mRender::GetTextureAtlasTexturesBuffer());
                renderPassDesc.ShaderBase = nullptr;
                renderPassDesc.ShaderRenderPath = Category::RENDER_PATH_TEXT;
                renderPassDesc.ShaderVertexPath = "C:/DDD/RealWare/build_vs/samples/Sample01/Debug/data/shaders/main_vertex.shader";
                renderPassDesc.ShaderFragmentPath = "C:/DDD/RealWare/build_vs/samples/Sample01/Debug/data/shaders/main_fragment.shader";
                renderPassDesc.RenderTarget = _opaqueRenderTarget;
                renderPassDesc.Viewport = glm::vec4(0.0f, 0.0f, windowSize);
                renderPassDesc.DepthMode.UseDepthTest = K_FALSE;
                renderPassDesc.DepthMode.UseDepthWrite = K_FALSE;
                _text = _context->CreateRenderPass(renderPassDesc);
            }
            {
                sRenderPass::sDescriptor renderPassDesc;
                renderPassDesc.InputVertexFormat = Category::VERTEX_BUFFER_FORMAT_NONE;
                renderPassDesc.InputTextures.emplace_back(_transparentRenderTarget->ColorAttachments[0]);
                renderPassDesc.InputTextureNames.emplace_back("AccumulationTexture");
                renderPassDesc.InputTextures.emplace_back(_transparentRenderTarget->ColorAttachments[1]);
                renderPassDesc.InputTextureNames.emplace_back("RevealageTexture");
                renderPassDesc.ShaderBase = nullptr;
                renderPassDesc.ShaderRenderPath = Category::RENDER_PATH_TRANSPARENT_COMPOSITE;
                renderPassDesc.ShaderVertexPath = "C:/DDD/RealWare/build_vs/samples/Sample01/Debug/data/shaders/main_vertex.shader";
                renderPassDesc.ShaderFragmentPath = "C:/DDD/RealWare/build_vs/samples/Sample01/Debug/data/shaders/main_fragment.shader";
                renderPassDesc.RenderTarget = _opaqueRenderTarget;
                renderPassDesc.Viewport = glm::vec4(0.0f, 0.0f, windowSize);
                renderPassDesc.DepthMode.UseDepthTest = K_FALSE;
                renderPassDesc.DepthMode.UseDepthWrite = K_FALSE;
                renderPassDesc.BlendMode.FactorCount = 1;
                renderPassDesc.BlendMode.SrcFactors[0] = sBlendMode::eFactor::SRC_ALPHA;
                renderPassDesc.BlendMode.DstFactors[0] = sBlendMode::eFactor::INV_SRC_ALPHA;
                _compositeTransparent = _context->CreateRenderPass(renderPassDesc);
            }
            {
                sRenderPass::sDescriptor renderPassDesc;
                renderPassDesc.InputVertexFormat = Category::VERTEX_BUFFER_FORMAT_NONE;
                renderPassDesc.InputTextures.emplace_back(_opaqueRenderTarget->ColorAttachments[0]);
                renderPassDesc.InputTextureNames.emplace_back("ColorTexture");
                renderPassDesc.ShaderBase = nullptr;
                renderPassDesc.ShaderRenderPath = Category::RENDER_PATH_QUAD;
                renderPassDesc.ShaderVertexPath = "C:/DDD/RealWare/build_vs/samples/Sample01/Debug/data/shaders/main_vertex.shader";
                renderPassDesc.ShaderFragmentPath = "C:/DDD/RealWare/build_vs/samples/Sample01/Debug/data/shaders/main_fragment.shader";
                renderPassDesc.RenderTarget = nullptr;
                renderPassDesc.Viewport = glm::vec4(0.0f, 0.0f, windowSize);
                renderPassDesc.DepthMode.UseDepthTest = K_FALSE;
                renderPassDesc.DepthMode.UseDepthWrite = K_FALSE;
                renderPassDesc.BlendMode.FactorCount = 1;
                renderPassDesc.BlendMode.SrcFactors[0] = sBlendMode::eFactor::ONE;
                renderPassDesc.BlendMode.DstFactors[0] = sBlendMode::eFactor::ZERO;
                _compositeFinal = _context->CreateRenderPass(renderPassDesc);
            }
        }

        mRender::~mRender()
        {
            cMemoryPool* const memoryPool = _app->GetMemoryPool();

            _context->DestroyBuffer(_vertexBuffer);
            _context->DestroyBuffer(_indexBuffer);
            _context->DestroyBuffer(_instanceBuffer);
            _materialsMap->~unordered_map<render::sMaterial*, s32>();
            memoryPool->Free(_materialsMap);
            memoryPool->Free(_vertices);
            memoryPool->Free(_indices);
            memoryPool->Free(_instances);
        }

        sMaterial* mRender::AddMaterial(const std::string& id, const sTextureAtlasTexture* const diffuseTexture, const glm::vec4& diffuseColor, const glm::vec4& highlightColor, const Category& customShaderRenderPath, const std::string& customVertexFuncPath, const std::string& customFragmentFuncPath)
        {
            sShader* customShader = nullptr;
            if (customVertexFuncPath != "" || customFragmentFuncPath != "")
            {
                std::string vertexFunc = "";
                std::string fragmentFunc = "";
                LoadVertexFragmentFuncs(customVertexFuncPath, customFragmentFuncPath, vertexFunc, fragmentFunc);

                if (customShaderRenderPath == Category::RENDER_PATH_OPAQUE)
                    customShader = _context->CreateShader(_opaque->Desc.Shader, vertexFunc, fragmentFunc);
                else if (customShaderRenderPath == Category::RENDER_PATH_TRANSPARENT)
                    customShader = _context->CreateShader(_transparent->Desc.Shader, vertexFunc, fragmentFunc);
            }

            return _materialsCPU.Add(id, diffuseTexture, diffuseColor, highlightColor, customShader);
        }

        sMaterial* mRender::FindMaterial(const std::string& id)
        {
            return _materialsCPU.Find(id);
        }

        void mRender::DeleteMaterial(const std::string& id)
        {
            sMaterial* const material = _materialsCPU.Find(id);
            if (material->CustomShader != nullptr)
                _context->DestroyShader(material->CustomShader);

            _materialsCPU.Delete(id);
        }

        sVertexArray* mRender::CreateDefaultVertexArray()
        {
            sVertexArray* vertexArray = _context->CreateVertexArray();
            std::vector<sBuffer*> buffersToBind = { _vertexBuffer, _indexBuffer };

            vertexArray = _context->CreateVertexArray();
            _context->BindVertexArray(vertexArray);
            for (auto buffer : buffersToBind)
                _context->BindBuffer(buffer);
            _context->BindDefaultInputLayout();
            _context->UnbindVertexArray();

            return vertexArray;
        }

        sVertexBufferGeometry* mRender::CreateGeometry(const sVertexBufferGeometry::eFormat& format, const usize verticesByteSize, const void* const vertices, const usize indicesByteSize, const void* const indices)
        {
            sVertexBufferGeometry* pGeometry = (sVertexBufferGeometry*)_app->GetMemoryPool()->Allocate(sizeof(sVertexBufferGeometry));
            sVertexBufferGeometry* geometry = new (pGeometry) sVertexBufferGeometry();

            memcpy((void*)((usize)_vertices + (usize)_verticesByteSize), vertices, verticesByteSize);
            memcpy((void*)((usize)_indices + (usize)_indicesByteSize), indices, indicesByteSize);

            _context->WriteBuffer(_vertexBuffer, _verticesByteSize, verticesByteSize, vertices);
            _context->WriteBuffer(_indexBuffer, _indicesByteSize, indicesByteSize, indices);

            geometry->VertexCount = verticesByteSize / (usize)format;
            geometry->IndexCount = indicesByteSize / sizeof(u32);
            geometry->VertexPtr = _vertices;
            geometry->IndexPtr = _indices;
            geometry->VertexOffset = _verticesByteSize;
            geometry->IndexOffset = _indicesByteSize;
            geometry->Format = format;

            _verticesByteSize += verticesByteSize;
            _indicesByteSize += indicesByteSize;

            return geometry;
        }

        void mRender::DestroyGeometry(sVertexBufferGeometry* geometry)
        {
            geometry->~sVertexBufferGeometry();
            _app->GetMemoryPool()->Free(geometry);
        }

        void mRender::ClearGeometryBuffer()
        {
            _verticesByteSize = 0;
            _indicesByteSize = 0;
        }

        void mRender::ClearRenderPass(const sRenderPass* const renderPass, const types::boolean clearColor, const usize bufferIndex, const glm::vec4& color, const types::boolean clearDepth, const f32 depth)
        {
            _context->BindRenderPass(renderPass);
            if (clearColor == K_TRUE)
                _context->ClearFramebufferColor(bufferIndex, color);
            if (clearDepth == K_TRUE)
                _context->ClearFramebufferDepth(depth);
            _context->UnbindRenderPass(renderPass);
        }

        void mRender::ClearRenderPasses(const glm::vec4& clearColor, const f32 clearDepth)
        {
            _context->BindRenderPass(_opaque);
            _context->ClearFramebufferColor(0, clearColor);
            _context->ClearFramebufferDepth(clearDepth);
			
			_context->BindRenderPass(_text);
			_context->ClearFramebufferColor(0, clearColor);

            _context->BindRenderPass(_transparent);
            _context->ClearFramebufferColor(0, glm::vec4(0.0f));
            _context->ClearFramebufferColor(1, glm::vec4(1.0f));
        }

        void mRender::UpdateLights()
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

        void mRender::DrawGeometryOpaque(const sVertexBufferGeometry* const geometry, const std::vector<cGameObject>& objects, const cGameObject* const cameraObject, sRenderPass* const renderPass)
        {
            usize instanceCount = 0;
            _instancesByteSize = 0;
            _materialsByteSize = 0;
            _textureAtlasTexturesByteSize = 0;
            _materialsMap->clear();

            for (auto& it : objects)
            {
                if (it.GetGeometry() == geometry)
                {
                    if (it.GetVisible() == K_TRUE && it.GetOpaque() == K_TRUE)
                    {
                        sTransform transform(&it);
                        transform.Transform();

                        s32 materialIndex = -1;

                        sMaterial* material = it.GetMaterial();

                        auto it = _materialsMap->find(material);
                        if (it == _materialsMap->end())
                        {
                            materialIndex = _materialsMap->size();

                            sMaterialInstance mi(materialIndex, material);
                            if (material->DiffuseTexture)
                            {
                                sTextureAtlasTexture* frame = material->DiffuseTexture;
                                mi.SetDiffuseTexture(*frame);
                            }
                            else
                            {
                                mi.DiffuseTextureLayerInfo = -1.0f;
                            }

                            _materialsMap->insert({ material, materialIndex });

                            memcpy((void*)((usize)_materials + (usize)_materialsByteSize), &mi, sizeof(sMaterialInstance));
                            _materialsByteSize += sizeof(sMaterialInstance);
                        }
                        else
                        {
                            materialIndex = it->second;
                        }
                        
                        sRenderInstance ri(materialIndex, transform);

                        memcpy((void*)((usize)_instances + (usize)_instancesByteSize), &ri, sizeof(sRenderInstance));
                        _instancesByteSize += sizeof(sRenderInstance);

                        instanceCount += 1;
                    }
                }
            }

            std::vector<sTextureAtlasTexture*>& renderPassTextureAtlasTextures = renderPass->Desc.InputTextureAtlasTextures;
            for (auto& textureAtlasTexture : renderPassTextureAtlasTextures)
            {
                sTextureAtlasTextureGPU tatGPU;
                tatGPU.TextureInfo = glm::vec4(
                    textureAtlasTexture->Offset.x,
                    textureAtlasTexture->Offset.y,
                    textureAtlasTexture->Size.x,
                    textureAtlasTexture->Size.y
                );
                tatGPU.TextureLayerInfo = textureAtlasTexture->Offset.z;

                memcpy((void*)((usize)_textureAtlasTextures + (usize)_textureAtlasTexturesByteSize), &tatGPU, sizeof(sTextureAtlasTextureGPU));
                _textureAtlasTexturesByteSize += sizeof(sTextureAtlasTextureGPU);
            }

            _context->WriteBuffer(_instanceBuffer, 0, _instancesByteSize, _instances);
            _context->WriteBuffer(_materialBuffer, 0, _materialsByteSize, _materials);
            _context->WriteBuffer(_textureAtlasTexturesBuffer, 0, _textureAtlasTexturesByteSize, _textureAtlasTextures);

            if (renderPass == nullptr)
            {
                _context->BindRenderPass(_opaque);
                _context->SetShaderUniform(_opaque->Desc.Shader, "ViewProjection", cameraObject->GetViewProjectionMatrix());
            }
            else
            {
                _context->BindRenderPass(renderPass);
                _context->SetShaderUniform(renderPass->Desc.Shader, "ViewProjection", cameraObject->GetViewProjectionMatrix());
            }

            _context->Draw(
                geometry->IndexCount,
                geometry->VertexOffset / (usize)geometry->Format,
                geometry->IndexOffset,
                instanceCount
            );

            if (renderPass == nullptr)
                _context->UnbindRenderPass(_opaque);
            else
                _context->UnbindRenderPass(renderPass);
        }

        void mRender::DrawGeometryOpaque(const sVertexBufferGeometry* const geometry, const std::vector<cGameObject>& objects, const cGameObject* const cameraObject, sShader* const singleShader)
        {
            usize instanceCount = 0;
            _instancesByteSize = 0;
            _materialsByteSize = 0;
            _materialsMap->clear();

            for (auto& it : objects)
            {
                if (it.GetGeometry() == geometry)
                {
                    if (it.GetVisible() == K_TRUE && it.GetOpaque() == K_TRUE)
                    {
                        sTransform transform(&it);
                        transform.Transform();

                        s32 materialIndex = -1;

                        sMaterial* material = it.GetMaterial();

                        auto it = _materialsMap->find(material);
                        if (it == _materialsMap->end())
                        {
                            materialIndex = _materialsMap->size();

                            sMaterialInstance mi(materialIndex, material);
                            if (material->DiffuseTexture)
                            {
                                sTextureAtlasTexture* frame = material->DiffuseTexture;
                                mi.SetDiffuseTexture(*frame);
                            }
                            else
                            {
                                mi.DiffuseTextureLayerInfo = -1.0f;
                            }

                            _materialsMap->insert({ material, materialIndex });

                            memcpy((void*)((usize)_materials + (usize)_materialsByteSize), &mi, sizeof(sMaterialInstance));
                            _materialsByteSize += sizeof(sMaterialInstance);
                        }
                        else
                        {
                            materialIndex = it->second;
                        }

                        sRenderInstance ri(materialIndex, transform);

                        memcpy((void*)((usize)_instances + (usize)_instancesByteSize), &ri, sizeof(sRenderInstance));
                        _instancesByteSize += sizeof(sRenderInstance);

                        instanceCount += 1;
                    }
                }
            }

            _context->WriteBuffer(_instanceBuffer, 0, _instancesByteSize, _instances);
            _context->WriteBuffer(_materialBuffer, 0, _materialsByteSize, _materials);

            _context->BindRenderPass(_opaque, singleShader);

            if (singleShader == nullptr)
                _context->SetShaderUniform(_opaque->Desc.Shader, "ViewProjection", cameraObject->GetViewProjectionMatrix());
            else
                _context->SetShaderUniform(singleShader, "ViewProjection", cameraObject->GetViewProjectionMatrix());
            
            _context->Draw(
                geometry->IndexCount,
                geometry->VertexOffset / (usize)geometry->Format,
                geometry->IndexOffset,
                instanceCount
            );

            _context->UnbindRenderPass(_opaque);
        }

        void mRender::DrawGeometryTransparent(const sVertexBufferGeometry* const geometry, const std::vector<cGameObject>& objects, const cGameObject* const cameraObject, sShader* const singleShader)
        {
            usize instanceCount = 0;
            _instancesByteSize = 0;
            _materialsByteSize = 0;
            _materialsMap->clear();

            for (auto& it : objects)
            {
                if (it.GetGeometry() == geometry)
                {
                    types::boolean isVisible = it.GetVisible();
                    types::boolean isOpaque = it.GetOpaque();
                    if (isVisible == K_TRUE && isOpaque == K_FALSE)
                    {
                        sTransform transform(&it);
                        transform.Transform();

                        sMaterial* material = it.GetMaterial();

                        s32 materialIndex = -1;

                        auto it = _materialsMap->find(material);
                        if (it == _materialsMap->end())
                        {
                            materialIndex = _materialsMap->size();

                            sMaterialInstance mi(materialIndex, material);
                            if (material->DiffuseTexture != nullptr)
                            {
                                sTextureAtlasTexture* frame = material->DiffuseTexture;
                                mi.SetDiffuseTexture(*frame);
                            }
                            else
                            {
                                mi.DiffuseTextureLayerInfo = -1.0f;
                            }

                            _materialsMap->insert({ material, materialIndex });

                            memcpy((void*)((usize)_materials + (usize)_materialsByteSize), &mi, sizeof(sMaterialInstance));
                            _materialsByteSize += sizeof(sMaterialInstance);
                        }
                        else
                        {
                            materialIndex = it->second;
                        }

                        sRenderInstance ri(materialIndex, transform);

                        memcpy((void*)((usize)_instances + (usize)_instancesByteSize), &ri, sizeof(sRenderInstance));
                        _instancesByteSize += sizeof(sRenderInstance);

                        instanceCount += 1;
                    }
                }
            }

            _context->WriteBuffer(_instanceBuffer, 0, _instancesByteSize, _instances);
            _context->WriteBuffer(_materialBuffer, 0, _materialsByteSize, _materials);

            _context->BindRenderPass(_transparent, singleShader);

            if (singleShader != nullptr)
                _context->SetShaderUniform(singleShader, "ViewProjection", cameraObject->GetViewProjectionMatrix());
            else
                _context->SetShaderUniform(_transparent->Desc.Shader, "ViewProjection", cameraObject->GetViewProjectionMatrix());

            _context->Draw(
                geometry->IndexCount,
                geometry->VertexOffset / (usize)geometry->Format,
                geometry->IndexOffset,
                instanceCount
            );

            _context->UnbindRenderPass(_transparent);
        }

        void mRender::DrawTexts(const std::vector<cGameObject>& objects)
        {
            for (auto& it : objects)
            {
                if (it.GetText() == nullptr)
                    continue;

                sText* text = it.GetText();
                sFont* textFont = text->Font;
                const std::string& textString = text->Text;
                const auto& alphabet = textFont->Alphabet;
                const sTexture* atlas = text->Font->Atlas;

                _instancesByteSize = 0;
                _materialsByteSize = 0;
                _materialsMap->clear();

                sTransform transform(&it);

                glm::vec2 windowSize = _app->GetWindowSize();
                glm::vec2 textPosition = glm::vec2((transform.Position.x * 2.0f) - 1.0f, (transform.Position.y * 2.0f) - 1.0f);
                glm::vec2 textScale = glm::vec2(
                    (1.0f / windowSize.x) * it.GetScale().x,
                    (1.0f / windowSize.y) * it.GetScale().y
                );

                usize charCount = textString.length();
                usize actualCharCount = 0;
                glm::vec2 offset = glm::vec2(0.0f);
                for (usize i = 0; i < charCount; i++)
                {
                    char glyphChar = textString[i];
                   
                    if (glyphChar == '\t')
                    {
                        offset.x += textFont->OffsetTab * textScale.x;
                        continue;
                    }
                    else if (glyphChar == '\n')
                    {
                        s32 maxHeight = 0;
                        s32 cnt = 1;
                        offset.x = 0.0f;
                        offset.y -= textFont->OffsetNewline * textScale.y;
                        continue;
                    }
                    else if (glyphChar == ' ')
                    {
                        offset.x += textFont->OffsetSpace * textScale.x;
                        continue;
                    }

                    auto alphabetEntry = alphabet.find(glyphChar);
                    if (alphabetEntry == alphabet.end())
                        continue;
                    const sGlyph& glyph = alphabetEntry->second;

                    sTextInstance t;
                    t.Info.x = textPosition.x + offset.x;
                    t.Info.y = textPosition.y + (offset.y - (float)((glyph.Height - glyph.Top) * textScale.y));
                    t.Info.z = (float)glyph.Width * textScale.x;
                    t.Info.w = (float)glyph.Height * textScale.y;
                    t.AtlasInfo.x = (float)glyph.AtlasXOffset / (float)atlas->Width;
                    t.AtlasInfo.y = (float)glyph.AtlasYOffset / (float)atlas->Height;
                    t.AtlasInfo.z = (float)glyph.Width / (float)atlas->Width;
                    t.AtlasInfo.w = (float)glyph.Height / (float)atlas->Height;

                    offset.x += glyph.AdvanceX * textScale.x;

                    memcpy((void*)((usize)_instances + (usize)_instancesByteSize), &t, sizeof(sTextInstance));
                    _instancesByteSize += sizeof(sTextInstance);

                    actualCharCount += 1;
                }

                sMaterial* material = it.GetMaterial();
                sMaterialInstance mi(0, material);
                memcpy(_materials, &mi, sizeof(sMaterialInstance));
                _materialsByteSize += sizeof(sMaterialInstance);

                _context->WriteBuffer(_instanceBuffer, 0, _instancesByteSize, _instances);
                _context->WriteBuffer(_materialBuffer, 0, _materialsByteSize, _materials);

                _context->BindRenderPass(_text);
                _context->BindTexture(_text->Desc.Shader, "FontAtlas", atlas, 0);
                _context->DrawQuads(actualCharCount);
                _context->UnbindRenderPass(_text);
            }
        }

        void mRender::CompositeTransparent()
        {
            _context->BindRenderPass(_compositeTransparent);
            _context->DrawQuad();
            _context->UnbindRenderPass(_compositeTransparent);
        }

        void mRender::CompositeFinal()
        {
            _context->BindRenderPass(_compositeFinal);
            _context->DrawQuad();
            _context->UnbindRenderPass(_compositeFinal);
			
            _context->UnbindShader();
        }

        sPrimitive* mRender::CreatePrimitive(const Category& primitive)
        {
            cMemoryPool* const memoryPool = _app->GetMemoryPool();

            sPrimitive* pPrimitiveObject = (sPrimitive*)_app->GetMemoryPool()->Allocate(sizeof(sPrimitive));
            sPrimitive* primitiveObject = new (pPrimitiveObject) sPrimitive();

            if (primitive == Category::PRIMITIVE_TRIANGLE)
            {
                primitiveObject->Format = render::sVertexBufferGeometry::eFormat::POSITION_TEXCOORD_NORMAL_VEC3_VEC2_VEC3;
                primitiveObject->Vertices = (render::sVertex*)memoryPool->Allocate(sizeof(render::sVertex) * 3);
                primitiveObject->Indices = (render::index*)memoryPool->Allocate(sizeof(render::index) * 3);
                primitiveObject->VertexCount = 3;
                primitiveObject->IndexCount = 3;
                primitiveObject->VerticesByteSize = sizeof(render::sVertex) * 3;
                primitiveObject->IndicesByteSize = sizeof(render::index) * 3;

                primitiveObject->Vertices[0].Position[0] = -1.0f; primitiveObject->Vertices[0].Position[1] = -1.0f; primitiveObject->Vertices[0].Position[2] = 0.0f;
                primitiveObject->Vertices[0].Texcoord[0] = 0.0f; primitiveObject->Vertices[0].Texcoord[1] = 0.0f;
                primitiveObject->Vertices[0].Normal[0] = 0.0f; primitiveObject->Vertices[0].Normal[1] = 0.0f; primitiveObject->Vertices[0].Normal[2] = 1.0f;
                primitiveObject->Vertices[1].Position[0] = 0.0f; primitiveObject->Vertices[1].Position[1] = 1.0f; primitiveObject->Vertices[1].Position[2] = 0.0f;
                primitiveObject->Vertices[1].Texcoord[0] = 0.5f; primitiveObject->Vertices[1].Texcoord[1] = 1.0f;
                primitiveObject->Vertices[1].Normal[0] = 0.0f; primitiveObject->Vertices[1].Normal[1] = 0.0f; primitiveObject->Vertices[1].Normal[2] = 1.0f;
                primitiveObject->Vertices[2].Position[0] = 1.0f; primitiveObject->Vertices[2].Position[1] = -1.0f; primitiveObject->Vertices[2].Position[2] = 0.0f;
                primitiveObject->Vertices[2].Texcoord[0] = 1.0f; primitiveObject->Vertices[2].Texcoord[1] = 0.0f;
                primitiveObject->Vertices[2].Normal[0] = 0.0f; primitiveObject->Vertices[2].Normal[1] = 0.0f; primitiveObject->Vertices[2].Normal[2] = 1.0f;
                primitiveObject->Indices[0] = 0;
                primitiveObject->Indices[1] = 1;
                primitiveObject->Indices[2] = 2;
            }
            else if (primitive == Category::PRIMITIVE_QUAD)
            {
                primitiveObject->Format = render::sVertexBufferGeometry::eFormat::POSITION_TEXCOORD_NORMAL_VEC3_VEC2_VEC3;
                primitiveObject->Vertices = (render::sVertex*)memoryPool->Allocate(sizeof(render::sVertex) * 4);
                primitiveObject->Indices = (render::index*)memoryPool->Allocate(sizeof(render::index) * 6);
                primitiveObject->VertexCount = 4;
                primitiveObject->IndexCount = 6;
                primitiveObject->VerticesByteSize = sizeof(render::sVertex) * 4;
                primitiveObject->IndicesByteSize = sizeof(render::index) * 6;

                primitiveObject->Vertices[0].Position[0] = -1.0f; primitiveObject->Vertices[0].Position[1] = -1.0f; primitiveObject->Vertices[0].Position[2] = 0.0f;
                primitiveObject->Vertices[0].Texcoord[0] = 0.0f; primitiveObject->Vertices[0].Texcoord[1] = 0.0f;
                primitiveObject->Vertices[0].Normal[0] = 0.0f; primitiveObject->Vertices[0].Normal[1] = 0.0f; primitiveObject->Vertices[0].Normal[2] = 1.0f;
                primitiveObject->Vertices[1].Position[0] = -1.0f; primitiveObject->Vertices[1].Position[1] = 1.0f; primitiveObject->Vertices[1].Position[2] = 0.0f;
                primitiveObject->Vertices[1].Texcoord[0] = 0.0f; primitiveObject->Vertices[1].Texcoord[1] = 1.0f;
                primitiveObject->Vertices[1].Normal[0] = 0.0f; primitiveObject->Vertices[1].Normal[1] = 0.0f; primitiveObject->Vertices[1].Normal[2] = 1.0f;
                primitiveObject->Vertices[2].Position[0] = 1.0f; primitiveObject->Vertices[2].Position[1] = -1.0f; primitiveObject->Vertices[2].Position[2] = 0.0f;
                primitiveObject->Vertices[2].Texcoord[0] = 1.0f; primitiveObject->Vertices[2].Texcoord[1] = 0.0f;
                primitiveObject->Vertices[2].Normal[0] = 0.0f; primitiveObject->Vertices[2].Normal[1] = 0.0f; primitiveObject->Vertices[2].Normal[2] = 1.0f;
                primitiveObject->Vertices[3].Position[0] = 1.0f; primitiveObject->Vertices[3].Position[1] = 1.0f; primitiveObject->Vertices[3].Position[2] = 0.0f;
                primitiveObject->Vertices[3].Texcoord[0] = 1.0f; primitiveObject->Vertices[3].Texcoord[1] = 1.0f;
                primitiveObject->Vertices[3].Normal[0] = 0.0f; primitiveObject->Vertices[3].Normal[1] = 0.0f; primitiveObject->Vertices[3].Normal[2] = 1.0f;
                primitiveObject->Indices[0] = 0;
                primitiveObject->Indices[1] = 1;
                primitiveObject->Indices[2] = 2;
                primitiveObject->Indices[3] = 1;
                primitiveObject->Indices[4] = 3;
                primitiveObject->Indices[5] = 2;
            }

            return primitiveObject;
        }

        sModel* mRender::CreateModel(const std::string& filename)
        {
            cMemoryPool* const memoryPool = _app->GetMemoryPool();

            // Create model
            sModel* pModel = (sModel*)_app->GetMemoryPool()->Allocate(sizeof(sModel));
            sModel* model = new (pModel) sModel();

            model->Format = render::sVertexBufferGeometry::eFormat::POSITION_TEXCOORD_NORMAL_VEC3_VEC2_VEC3;

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
            model->Vertices = (render::sVertex*)memoryPool->Allocate(scene->mMeshes[0]->mNumVertices * sizeof(render::sVertex));
            memset(model->Vertices, 0, scene->mMeshes[0]->mNumVertices * sizeof(render::sVertex));
            for (usize i = 0; i < scene->mMeshes[0]->mNumVertices; i++)
            {
                aiVector3D pos = scene->mMeshes[0]->mVertices[i];
                aiVector3D uv = scene->mMeshes[0]->mTextureCoords[0][i];
                aiVector3D normal = scene->mMeshes[0]->HasNormals() ? scene->mMeshes[0]->mNormals[i] : aiVector3D(1.0f, 1.0f, 1.0f);

                model->Vertices[totalVertexCount].Position = glm::vec3(pos.x, pos.y, pos.z);
                model->Vertices[totalVertexCount].Texcoord = glm::vec2(uv.x, uv.y);
                model->Vertices[totalVertexCount].Normal = glm::vec3(normal.x, normal.y, normal.z);

                totalVertexCount += 1;
            }

            // Load indices
            usize totalIndexCount = 0;
            model->Indices = (render::index*)memoryPool->Allocate(scene->mMeshes[0]->mNumFaces * 3 * sizeof(render::index));
            memset(model->Indices, 0, scene->mMeshes[0]->mNumFaces * 3 * sizeof(render::index));
            for (usize i = 0; i < scene->mMeshes[0]->mNumFaces; i++)
            {
                aiFace face = scene->mMeshes[0]->mFaces[i];
                for (usize j = 0; j < face.mNumIndices; j++)
                {
                    model->Indices[totalIndexCount] = face.mIndices[j];

                    totalIndexCount += 1;
                }
            }

            model->VertexCount = totalVertexCount;
            model->VerticesByteSize = totalVertexCount * sizeof(render::sVertex);
            model->IndexCount = totalIndexCount;
            model->IndicesByteSize = totalIndexCount * sizeof(render::index);

            return model;
        }

        void mRender::DestroyPrimitive(sPrimitive* primitiveObject)
        {
            cMemoryPool* const memoryPool = _app->GetMemoryPool();

            if (primitiveObject->Vertices)
                memoryPool->Free(primitiveObject->Vertices);
            if (primitiveObject->Indices)
                memoryPool->Free(primitiveObject->Indices);
            primitiveObject->~sPrimitive();
            memoryPool->Free(primitiveObject);
        }

        void mRender::LoadVertexFragmentFuncs(const std::string& vertexFuncPath, const std::string& fragmentFuncPath, std::string& vertexFunc, std::string& fragmentFunc)
        {
            sFile* vertexFuncFile = _app->GetFileSystemManager()->CreateDataFile(vertexFuncPath, K_TRUE);
            sFile* fragmentFuncFile = _app->GetFileSystemManager()->CreateDataFile(fragmentFuncPath, K_TRUE);
            vertexFunc = std::string((const char*)vertexFuncFile->Data);
            fragmentFunc = std::string((const char*)fragmentFuncFile->Data);
            _app->GetFileSystemManager()->DestroyDataFile(vertexFuncFile);
            _app->GetFileSystemManager()->DestroyDataFile(fragmentFuncFile);
        }

        void mRender::ResizeWindow(const glm::vec2& size)
        {
            _context->UnbindRenderPass(_opaque);
            _context->UnbindRenderPass(_transparent);

            _opaque->Desc.Viewport[2] = size.x;
            _opaque->Desc.Viewport[3] = size.y;
            _context->ResizeRenderTargetColors(_opaque->Desc.RenderTarget, size);
            _transparent->Desc.Viewport[2] = size.x;
            _transparent->Desc.Viewport[3] = size.y;
            _context->ResizeRenderTargetColors(_transparent->Desc.RenderTarget, size);
            _text->Desc.Viewport[2] = size.x;
            _text->Desc.Viewport[3] = size.y;
            _opaque->Desc.RenderTarget->DepthAttachment = _context->ResizeTexture(_opaque->Desc.RenderTarget->DepthAttachment, size);
            _transparent->Desc.RenderTarget->DepthAttachment = _opaque->Desc.RenderTarget->DepthAttachment;
            _context->UpdateRenderTargetBuffers(_opaque->Desc.RenderTarget);
            _context->UpdateRenderTargetBuffers(_transparent->Desc.RenderTarget);

            _compositeTransparent->Desc.Viewport[2] = size.x;
            _compositeTransparent->Desc.Viewport[3] = size.y;
            _compositeTransparent->Desc.InputTextures[0] = _transparent->Desc.RenderTarget->ColorAttachments[0];
            _compositeTransparent->Desc.InputTextureNames[0] = "AccumulationTexture";
            _compositeTransparent->Desc.InputTextures[1] = _transparent->Desc.RenderTarget->ColorAttachments[1];
            _compositeTransparent->Desc.InputTextureNames[1] = "RevealageTexture";

            _compositeFinal->Desc.Viewport[2] = size.x;
            _compositeFinal->Desc.Viewport[3] = size.y;
            _compositeFinal->Desc.InputTextures[0] = _opaque->Desc.RenderTarget->ColorAttachments[0];
            _compositeFinal->Desc.InputTextureNames[0] = "ColorTexture";
        }
    }
}