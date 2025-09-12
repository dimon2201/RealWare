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
#include "application.hpp"

namespace realware
{
    using namespace app;
    using namespace game;
    using namespace font;
    using namespace types;

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

        sMaterialInstance::sMaterialInstance(s32 materialIndex, const cMaterial* const material)
        {
            BufferIndex = materialIndex;
            DiffuseColor = material->GetDiffuseColor();
            HighlightColor = material->GetHighlightColor();
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

        mRender::mRender(const cApplication* const app, const cRenderContext* const context) : _app((cApplication*)app), _context((cRenderContext*)context)
        {
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
            _vertices = malloc(desc->VertexBufferSize);
            _verticesByteSize = 0;
            _indices = malloc(desc->IndexBufferSize);
            _indicesByteSize = 0;
            _instances = malloc(desc->InstanceBufferSize);
            _instancesByteSize = 0;
            _materials = malloc(desc->MaterialBufferSize);
            _materialsByteSize = 0;
            _lights = malloc(desc->LightBufferSize);
            _lightsByteSize = 0;
            _materialsMap = new std::unordered_map<render::cMaterial*, s32>();
            _materialsCPU.resize(desc->MaxMaterialCount);

            sTexture* color = _context->CreateTexture(windowSize.x, windowSize.y, 0, render::sTexture::eType::TEXTURE_2D, render::sTexture::eFormat::RGBA8, nullptr);
            sTexture* accumulation = _context->CreateTexture(windowSize.x, windowSize.y, 0, render::sTexture::eType::TEXTURE_2D, render::sTexture::eFormat::RGBA16F, nullptr);
            sTexture* ui = _context->CreateTexture(windowSize.x, windowSize.y, 0, render::sTexture::eType::TEXTURE_2D, render::sTexture::eFormat::RGBA8, nullptr);
            sTexture* revealage = _context->CreateTexture(windowSize.x, windowSize.y, 0, render::sTexture::eType::TEXTURE_2D, render::sTexture::eFormat::R8F, nullptr);
            sTexture* depth = _context->CreateTexture(windowSize.x, windowSize.y, 0, render::sTexture::eType::TEXTURE_2D, render::sTexture::eFormat::DEPTH_STENCIL, nullptr);

            sRenderTarget* opaqueRenderTarget = _context->CreateRenderTarget({ color }, depth);
            sRenderTarget* transparentRenderTarget = _context->CreateRenderTarget({ accumulation, revealage }, depth);

            {
                sRenderPass::sDescriptor renderPassDesc;
                renderPassDesc.InputVertexFormat = GameObjectFeatures::VERTEX_BUFFER_FORMAT_POS_TEX_NRM_VEC3_VEC2_VEC3;
                renderPassDesc.InputBuffers.emplace_back(mRender::GetVertexBuffer());
                renderPassDesc.InputBuffers.emplace_back(mRender::GetIndexBuffer());
                renderPassDesc.InputBuffers.emplace_back(mRender::GetInstanceBuffer());
                renderPassDesc.InputBuffers.emplace_back(mRender::GetMaterialBuffer());
                renderPassDesc.InputBuffers.emplace_back(mRender::GetLightBuffer());
                renderPassDesc.InputTextures.emplace_back(_app->GetTextureManager()->GetAtlas());
                renderPassDesc.InputTextureNames.emplace_back("TextureAtlas");
                renderPassDesc.Shader = ((cOpenGLRenderContext*)context)->LoadShader(
                    "RENDER_PATH_OPAQUE",
                    "C:/DDD/RealWare/build/samples/Sample01/Debug/data/shaders/main_vertex.shader",
                    "C:/DDD/RealWare/build/samples/Sample01/Debug/data/shaders/main_fragment.shader"
                );
                renderPassDesc.RenderTarget = opaqueRenderTarget;
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
                renderPassDesc.InputVertexFormat = GameObjectFeatures::VERTEX_BUFFER_FORMAT_POS_TEX_NRM_VEC3_VEC2_VEC3;
                renderPassDesc.InputBuffers.emplace_back(mRender::GetVertexBuffer());
                renderPassDesc.InputBuffers.emplace_back(mRender::GetIndexBuffer());
                renderPassDesc.InputBuffers.emplace_back(mRender::GetInstanceBuffer());
                renderPassDesc.InputBuffers.emplace_back(mRender::GetMaterialBuffer());
                renderPassDesc.InputTextures.emplace_back(_app->GetTextureManager()->GetAtlas());
                renderPassDesc.InputTextureNames.emplace_back("TextureAtlas");
                renderPassDesc.Shader = ((cOpenGLRenderContext*)context)->LoadShader(
                    "RENDER_PATH_TRANSPARENT",
                    "C:/DDD/RealWare/build/samples/Sample01/Debug/data/shaders/main_vertex.shader",
                    "C:/DDD/RealWare/build/samples/Sample01/Debug/data/shaders/main_fragment.shader"
                );
                renderPassDesc.RenderTarget = transparentRenderTarget;
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
                renderPassDesc.InputVertexFormat = GameObjectFeatures::VERTEX_BUFFER_FORMAT_NONE;
                renderPassDesc.InputBuffers.emplace_back(mRender::GetInstanceBuffer());
                renderPassDesc.InputBuffers.emplace_back(mRender::GetMaterialBuffer());
                renderPassDesc.Shader = ((cOpenGLRenderContext*)context)->LoadShader(
                    "RENDER_PATH_TEXT",
                    "C:/DDD/RealWare/build/samples/Sample01/Debug/data/shaders/main_vertex.shader",
                    "C:/DDD/RealWare/build/samples/Sample01/Debug/data/shaders/main_fragment.shader"
                );
                renderPassDesc.RenderTarget = opaqueRenderTarget;
                renderPassDesc.Viewport = glm::vec4(0.0f, 0.0f, windowSize);
                renderPassDesc.DepthMode.UseDepthTest = K_FALSE;
                renderPassDesc.DepthMode.UseDepthWrite = K_FALSE;
                _text = _context->CreateRenderPass(renderPassDesc);
            }
            {
                sRenderPass::sDescriptor renderPassDesc;
                renderPassDesc.InputVertexFormat = GameObjectFeatures::VERTEX_BUFFER_FORMAT_NONE;
                renderPassDesc.InputTextures.emplace_back(transparentRenderTarget->ColorAttachments[0]);
                renderPassDesc.InputTextureNames.emplace_back("AccumulationTexture");
                renderPassDesc.InputTextures.emplace_back(transparentRenderTarget->ColorAttachments[1]);
                renderPassDesc.InputTextureNames.emplace_back("RevealageTexture");
                renderPassDesc.Shader = ((cOpenGLRenderContext*)context)->LoadShader(
                    "RENDER_PATH_TRANSPARENT_COMPOSITE",
                    "C:/DDD/RealWare/build/samples/Sample01/Debug/data/shaders/main_vertex.shader",
                    "C:/DDD/RealWare/build/samples/Sample01/Debug/data/shaders/main_fragment.shader"
                );
                renderPassDesc.RenderTarget = opaqueRenderTarget;
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
                renderPassDesc.InputVertexFormat = GameObjectFeatures::VERTEX_BUFFER_FORMAT_NONE;
                renderPassDesc.InputTextures.emplace_back(opaqueRenderTarget->ColorAttachments[0]);
                renderPassDesc.InputTextureNames.emplace_back("ColorTexture");
                renderPassDesc.Shader = ((cOpenGLRenderContext*)context)->LoadShader(
                    "RENDER_PATH_QUAD",
                    "C:/DDD/RealWare/build/samples/Sample01/Debug/data/shaders/main_vertex.shader",
                    "C:/DDD/RealWare/build/samples/Sample01/Debug/data/shaders/main_fragment.shader"
                );
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
            _context->DestroyBuffer(_vertexBuffer);
            _context->DestroyBuffer(_indexBuffer);
            _context->DestroyBuffer(_instanceBuffer);
            delete _materialsMap;
            free(_vertices);
            free(_indices);
            free(_instances);
        }

        cMaterial* mRender::AddMaterial(const std::string& id, const sTextureAtlasTexture* const diffuseTexture, const glm::vec4& diffuseColor, const glm::vec4& highlightColor)
        {
            _materialsCPU[_materialCountCPU] = cMaterial(id, diffuseTexture, diffuseColor, highlightColor);
            _materialCountCPU += 1;

            return &_materialsCPU[_materialCountCPU - 1];
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
            sVertexBufferGeometry* geometry = new sVertexBufferGeometry;

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

        void mRender::DeleteMaterial(const std::string& id)
        {
            for (usize i = 0; i < _materialCountCPU; i++)
            {
                if (_materialsCPU[i].GetID() == id)
                {
                    if (_materialCountCPU > 1)
                        _materialsCPU[i] = _materialsCPU[_materialCountCPU - 1];

                    return;
                }
            }
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
            _lightsByteSize = 16; // because vec4 (16 bytes) goes first (contains light count)
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

            _context->WriteBuffer(_lightBuffer, 0, _lightsByteSize, _lights);
        }

        void mRender::DrawGeometryOpaque(const sVertexBufferGeometry* const geometry, std::vector<cGameObject>& objects, const cGameObject* const cameraObject)
        {
            s32 instanceCount = 0;
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
                        cMaterial* material = it.GetMaterial();
                        transform.Transform();

                        s32 materialIndex = -1;
                        auto it = _materialsMap->find(material);
                        if (it == _materialsMap->end())
                        {
                            materialIndex = _materialsMap->size();

                            sMaterialInstance mi(materialIndex, material);
                            if (material->GetDiffuseTexture())
                            {
                                sTextureAtlasTexture* frame = material->GetDiffuseTexture();
                                mi.SetDiffuseTexture(_app->GetTextureManager()->CalculateNormalizedArea(*frame));
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

            _context->BindRenderPass(_opaque);

            _context->SetShaderUniform(_opaque->Desc.Shader, "ViewProjection", cameraObject->GetViewProjectionMatrix());

            _context->Draw(
                geometry->IndexCount,
                geometry->VertexOffset / (usize)geometry->Format,
                geometry->IndexOffset,
                instanceCount
            );

            _context->UnbindRenderPass(_opaque);
        }

        void mRender::DrawGeometryTransparent(const sVertexBufferGeometry* const geometry, std::vector<cGameObject>& objects, const cGameObject* const cameraObject)
        {
            s32 instanceCount = 0;
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
                        cMaterial* material = it.GetMaterial();
                        transform.Transform();

                        s32 materialIndex = -1;
                        auto it = _materialsMap->find(material);
                        if (it == _materialsMap->end())
                        {
                            materialIndex = _materialsMap->size();

                            sMaterialInstance mi(materialIndex, material);
                            if (material->GetDiffuseTexture() != nullptr)
                            {
                                sTextureAtlasTexture* frame = material->GetDiffuseTexture();
                                mi.SetDiffuseTexture(_app->GetTextureManager()->CalculateNormalizedArea(*frame));
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

            _context->BindRenderPass(_transparent);

            _context->SetShaderUniform(_transparent->Desc.Shader, "ViewProjection", cameraObject->GetViewProjectionMatrix());

            _context->Draw(
                geometry->IndexCount,
                geometry->VertexOffset / (usize)geometry->Format,
                geometry->IndexOffset,
                instanceCount
            );

            _context->UnbindRenderPass(_transparent);
        }

        void mRender::DrawTexts(std::vector<cGameObject>& objects)
        {
            for (auto& it : objects)
            {
                if (it.GetText() == nullptr)
                    continue;

                cText* text = it.GetText();

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

                usize charCount = text->GetText().length();
                usize actualCharCount = 0;
                glm::vec2 offset = glm::vec2(0.0f);
                for (usize i = 0; i < charCount; i++)
                {
                    char glyphChar = text->GetText()[i];
                   
                    if (glyphChar == '\t')
                    {
                        offset.x += text->GetFont()->OffsetTab * textScale.x;
                        continue;
                    }
                    else if (glyphChar == '\n')
                    {
                        s32 maxHeight = 0;
                        s32 cnt = 1;
                        offset.x = 0.0f;
                        offset.y -= text->GetFont()->OffsetNewline * textScale.y;
                        continue;
                    }
                    else if (glyphChar == ' ')
                    {
                        offset.x += text->GetFont()->OffsetSpace * textScale.x;
                        continue;
                    }

                    auto alphabetEntry = text->GetFont()->Alphabet.find(glyphChar);
                    if (alphabetEntry == text->GetFont()->Alphabet.end())
                        continue;
                    const font::sFont::sGlyph& glyph = alphabetEntry->second;

                    sTextInstance t;
                    t.Info.x = textPosition.x + offset.x;
                    t.Info.y = textPosition.y + (offset.y - (float)((glyph.Height - glyph.Top) * textScale.y));
                    t.Info.z = (float)glyph.Width * textScale.x;
                    t.Info.w = (float)glyph.Height * textScale.y;
                    t.AtlasInfo.x = (float)glyph.AtlasXOffset / (float)text->GetFont()->Atlas->Width;
                    t.AtlasInfo.y = (float)glyph.AtlasYOffset / (float)text->GetFont()->Atlas->Height;
                    t.AtlasInfo.z = (float)glyph.Width / (float)text->GetFont()->Atlas->Width;
                    t.AtlasInfo.w = (float)glyph.Height / (float)text->GetFont()->Atlas->Height;

                    offset.x += glyph.AdvanceX * textScale.x;

                    memcpy((void*)((usize)_instances + (usize)_instancesByteSize), &t, sizeof(sTextInstance));
                    _instancesByteSize += sizeof(sTextInstance);

                    actualCharCount += 1;
                }

                cMaterial* material = it.GetMaterial();
                sMaterialInstance mi(0, material);
                memcpy(_materials, &mi, sizeof(sMaterialInstance));
                _materialsByteSize += sizeof(sMaterialInstance);

                _context->WriteBuffer(_instanceBuffer, 0, _instancesByteSize, _instances);
                _context->WriteBuffer(_materialBuffer, 0, _materialsByteSize, _materials);

                _context->BindRenderPass(_text);
                _context->BindTexture(_text->Desc.Shader, "FontAtlas", text->GetFont()->Atlas, 0);
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

        sPrimitive* mRender::CreatePrimitive(const ePrimitive& primitive)
        {
            sPrimitive* primitiveObject = new sPrimitive;

            if (primitive == ePrimitive::TRIANGLE)
            {
                primitiveObject->Format = render::sVertexBufferGeometry::eFormat::POSITION_TEXCOORD_NORMAL_VEC3_VEC2_VEC3;
                primitiveObject->Vertices = (render::sVertex*)malloc(sizeof(render::sVertex) * 3);
                primitiveObject->Indices = (render::index*)malloc(sizeof(render::index) * 3);
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
            else if (primitive == ePrimitive::QUAD)
            {
                primitiveObject->Format = render::sVertexBufferGeometry::eFormat::POSITION_TEXCOORD_NORMAL_VEC3_VEC2_VEC3;
                primitiveObject->Vertices = (render::sVertex*)malloc(sizeof(render::sVertex) * 4);
                primitiveObject->Indices = (render::index*)malloc(sizeof(render::index) * 6);
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
            // Create model
            sModel* model = new sModel();
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
            model->Vertices = (render::sVertex*)malloc(scene->mMeshes[0]->mNumVertices * sizeof(render::sVertex));
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
            model->Indices = (render::index*)malloc(scene->mMeshes[0]->mNumFaces * 3 * sizeof(render::index));
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
            if (primitiveObject->Vertices)
                free(primitiveObject->Vertices);
            if (primitiveObject->Indices)
                free(primitiveObject->Indices);
            delete primitiveObject;
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

        cMaterial* mRender::GetMaterial(const std::string& id)
        {
            for (usize i = 0; i < _materialCountCPU; i++)
            {
                if (_materialsCPU[i].GetID() == id)
                    return &_materialsCPU[i];
            }

            return nullptr;
        }
    }
}