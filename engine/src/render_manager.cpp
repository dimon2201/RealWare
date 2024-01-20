#include <GL/glew.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include "render_manager.hpp"
#include "render_context.hpp"
#include "user_input_manager.hpp"
#include "ecs.hpp"
#include "texture_manager.hpp"
#include "font_manager.hpp"

extern realware::core::mTexture* textureManager;
extern realware::core::mUserInput* userInputManager;
extern realware::font::mFont* fontManager;

namespace realware
{
    namespace render
    {
        void mRender::Init(
            const cRenderContext* context,
            core::usize vertexBufferSize,
            core::usize indexBufferSize,
            core::usize instanceBufferSize,
            core::usize materialBufferSize,
            const glm::vec2& windowSize
        )
        {
            m_context = (cOpenGLRenderContext*)context;
            m_vertexBuffer = m_context->CreateBuffer(vertexBufferSize, sBuffer::eType::VERTEX, nullptr);
            m_indexBuffer = m_context->CreateBuffer(indexBufferSize, sBuffer::eType::INDEX, nullptr);
            m_instanceBuffer = m_context->CreateBuffer(instanceBufferSize, sBuffer::eType::LARGE, nullptr);
            m_instanceBuffer->Slot = 0;
            m_materialBuffer = m_context->CreateBuffer(materialBufferSize, sBuffer::eType::LARGE, nullptr);
            m_materialBuffer->Slot = 1;
            m_vertices = malloc(vertexBufferSize);
            m_verticesByteSize = 0;
            m_indices = malloc(indexBufferSize);
            m_indicesByteSize = 0;
            m_instances = malloc(instanceBufferSize);
            m_instancesByteSize = 0;
            m_materials = malloc(materialBufferSize);
            m_materialsByteSize = 0;
            m_materialsMap = new std::unordered_map<core::sCMaterial*, core::s32>();

            sTexture* color = m_context->CreateTexture(windowSize.x, windowSize.y, 0, render::sTexture::eType::TEXTURE_2D, render::sTexture::eFormat::RGBA8, nullptr);
            sTexture* accumulation = m_context->CreateTexture(windowSize.x, windowSize.y, 0, render::sTexture::eType::TEXTURE_2D, render::sTexture::eFormat::RGBA16F, nullptr);
            sTexture* revealage = m_context->CreateTexture(windowSize.x, windowSize.y, 0, render::sTexture::eType::TEXTURE_2D, render::sTexture::eFormat::R8F, nullptr);
            sTexture* depth = m_context->CreateTexture(windowSize.x, windowSize.y, 0, render::sTexture::eType::TEXTURE_2D, render::sTexture::eFormat::DEPTH_STENCIL, nullptr);

            sRenderTarget* opaqueRenderTarget = m_context->CreateRenderTarget({ color }, depth);
            sRenderTarget* transparentRenderTarget = m_context->CreateRenderTarget({ accumulation, revealage }, depth);

            {
                sRenderPass::sDescriptor renderPassDesc;
                renderPassDesc.InputVertexFormat = sVertexBufferGeometry::eFormat::POSITION_TEXCOORD_NORMAL_VEC3_VEC2_VEC3;
                renderPassDesc.InputBuffers.emplace_back(mRender::GetVertexBuffer());
                renderPassDesc.InputBuffers.emplace_back(mRender::GetIndexBuffer());
                renderPassDesc.InputBuffers.emplace_back(mRender::GetInstanceBuffer());
                renderPassDesc.InputBuffers.emplace_back(mRender::GetMaterialBuffer());
                renderPassDesc.InputTextures.emplace_back(textureManager->GetAtlas());
                renderPassDesc.InputTextureNames.emplace_back("TextureAtlas");
                renderPassDesc.Shader = m_context->BindOpaqueShader();
                renderPassDesc.RenderTarget = opaqueRenderTarget;
                renderPassDesc.Viewport = glm::vec4(0.0f, 0.0f, windowSize);
                renderPassDesc.DepthMode.UseDepthTest = core::K_TRUE;
                renderPassDesc.DepthMode.UseDepthWrite = core::K_TRUE;
                renderPassDesc.BlendMode.FactorCount = 1;
                renderPassDesc.BlendMode.SrcFactors[0] = sBlendMode::eFactor::ONE;
                renderPassDesc.BlendMode.DstFactors[0] = sBlendMode::eFactor::ZERO;
                m_opaque = m_context->CreateRenderPass(renderPassDesc);
            }
            {
                sRenderPass::sDescriptor renderPassDesc;
                renderPassDesc.InputVertexFormat = sVertexBufferGeometry::eFormat::POSITION_TEXCOORD_NORMAL_VEC3_VEC2_VEC3;
                renderPassDesc.InputBuffers.emplace_back(mRender::GetVertexBuffer());
                renderPassDesc.InputBuffers.emplace_back(mRender::GetIndexBuffer());
                renderPassDesc.InputBuffers.emplace_back(mRender::GetInstanceBuffer());
                renderPassDesc.InputBuffers.emplace_back(mRender::GetMaterialBuffer());
                renderPassDesc.InputTextures.emplace_back(textureManager->GetAtlas());
                renderPassDesc.InputTextureNames.emplace_back("TextureAtlas");
                renderPassDesc.Shader = m_context->BindTransparentShader();
                renderPassDesc.RenderTarget = transparentRenderTarget;
                renderPassDesc.Viewport = glm::vec4(0.0f, 0.0f, windowSize);
                renderPassDesc.DepthMode.UseDepthTest = core::K_TRUE;
                renderPassDesc.DepthMode.UseDepthWrite = core::K_FALSE;
                renderPassDesc.BlendMode.FactorCount = 2;
                renderPassDesc.BlendMode.SrcFactors[0] = sBlendMode::eFactor::ONE;
                renderPassDesc.BlendMode.DstFactors[0] = sBlendMode::eFactor::ONE;
                renderPassDesc.BlendMode.SrcFactors[1] = sBlendMode::eFactor::ZERO;
                renderPassDesc.BlendMode.DstFactors[1] = sBlendMode::eFactor::INV_SRC_COLOR;
                m_transparent = m_context->CreateRenderPass(renderPassDesc);
            }
            {
                sRenderPass::sDescriptor renderPassDesc;
                renderPassDesc.InputVertexFormat = sVertexBufferGeometry::eFormat::NONE;
                renderPassDesc.InputBuffers.emplace_back(mRender::GetInstanceBuffer());
                renderPassDesc.InputBuffers.emplace_back(mRender::GetMaterialBuffer());
                renderPassDesc.InputTextures.emplace_back(textureManager->GetAtlas());
                renderPassDesc.InputTextureNames.emplace_back("TextureAtlas");
                renderPassDesc.Shader = m_context->BindWidgetShader();
                renderPassDesc.RenderTarget = opaqueRenderTarget;
                renderPassDesc.Viewport = glm::vec4(0.0f, 0.0f, windowSize);
                m_widget = m_context->CreateRenderPass(renderPassDesc);
            }
            {
                sRenderPass::sDescriptor renderPassDesc;
                renderPassDesc.InputVertexFormat = sVertexBufferGeometry::eFormat::NONE;
                renderPassDesc.InputBuffers.emplace_back(mRender::GetInstanceBuffer());
                renderPassDesc.InputBuffers.emplace_back(mRender::GetMaterialBuffer());
                renderPassDesc.Shader = m_context->BindTextShader();
                renderPassDesc.RenderTarget = opaqueRenderTarget;
                renderPassDesc.Viewport = glm::vec4(0.0f, 0.0f, windowSize);
                m_text = m_context->CreateRenderPass(renderPassDesc);
            }
            {
                transparentRenderTarget->ColorAttachments[0]->Slot = 0;
                transparentRenderTarget->ColorAttachments[1]->Slot = 1;
                sRenderPass::sDescriptor renderPassDesc;
                renderPassDesc.InputVertexFormat = sVertexBufferGeometry::eFormat::NONE;
                renderPassDesc.InputTextures.emplace_back(transparentRenderTarget->ColorAttachments[0]);
                renderPassDesc.InputTextureNames.emplace_back("AccumulationTexture");
                renderPassDesc.InputTextures.emplace_back(transparentRenderTarget->ColorAttachments[1]);
                renderPassDesc.InputTextureNames.emplace_back("RevealageTexture");
                renderPassDesc.Shader = m_context->BindCompositeTransparentShader();
                renderPassDesc.RenderTarget = opaqueRenderTarget;
                renderPassDesc.Viewport = glm::vec4(0.0f, 0.0f, windowSize);
                renderPassDesc.DepthMode.UseDepthTest = core::K_FALSE;
                renderPassDesc.DepthMode.UseDepthWrite = core::K_FALSE;
                renderPassDesc.BlendMode.FactorCount = 1;
                renderPassDesc.BlendMode.SrcFactors[0] = sBlendMode::eFactor::SRC_ALPHA;
                renderPassDesc.BlendMode.DstFactors[0] = sBlendMode::eFactor::INV_SRC_ALPHA;
                m_compositeTransparent = m_context->CreateRenderPass(renderPassDesc);
            }
            {
                sRenderPass::sDescriptor renderPassDesc;
                renderPassDesc.InputVertexFormat = sVertexBufferGeometry::eFormat::NONE;
                renderPassDesc.InputTextures.emplace_back(opaqueRenderTarget->ColorAttachments[0]);
                renderPassDesc.InputTextureNames.emplace_back("ColorTexture");
                renderPassDesc.Shader = m_context->BindQuadShader();
                renderPassDesc.RenderTarget = nullptr;
                renderPassDesc.Viewport = glm::vec4(0.0f, 0.0f, windowSize);
                renderPassDesc.DepthMode.UseDepthTest = core::K_FALSE;
                renderPassDesc.DepthMode.UseDepthWrite = core::K_FALSE;
                renderPassDesc.BlendMode.FactorCount = 1;
                renderPassDesc.BlendMode.SrcFactors[0] = sBlendMode::eFactor::ONE;
                renderPassDesc.BlendMode.DstFactors[0] = sBlendMode::eFactor::ZERO;
                m_compositeFinal = m_context->CreateRenderPass(renderPassDesc);
            }
        }

        void mRender::Free()
        {
            m_context->DeleteBuffer(m_vertexBuffer);
            m_context->DeleteBuffer(m_indexBuffer);
            m_context->DeleteBuffer(m_instanceBuffer);
            delete m_vertexBuffer;
            delete m_indexBuffer;
            delete m_instanceBuffer;
            delete m_materialsMap;
            free(m_vertices);
            free(m_indices);
            free(m_instances);
        }

        sVertexArray* mRender::CreateDefaultVertexArray()
        {
            sVertexArray* vertexArray = m_context->CreateVertexArray();
            std::vector<sBuffer*> buffersToBind = { m_vertexBuffer, m_indexBuffer };

            vertexArray = m_context->CreateVertexArray();
            m_context->BindVertexArray(vertexArray);
            for (auto buffer : buffersToBind) {
                m_context->BindBuffer(buffer);
            }
            m_context->BindDefaultInputLayout();
            m_context->UnbindVertexArray();

            return vertexArray;
        }

        sVertexBufferGeometry* mRender::AddGeometry(const sVertexBufferGeometry::eFormat& format, core::usize verticesByteSize, const void* vertices, core::usize indicesByteSize, const void* indices)
        {
            sVertexBufferGeometry* geometry = new sVertexBufferGeometry();

            memcpy((void*)((core::usize)m_vertices + (core::usize)m_verticesByteSize), vertices, verticesByteSize);
            memcpy((void*)((core::usize)m_indices + (core::usize)m_indicesByteSize), indices, indicesByteSize);

            m_context->WriteBuffer(m_vertexBuffer, m_verticesByteSize, verticesByteSize, vertices);
            m_context->WriteBuffer(m_indexBuffer, m_indicesByteSize, indicesByteSize, indices);

            geometry->VertexCount = verticesByteSize / (core::usize)format;
            geometry->IndexCount = indicesByteSize / sizeof(core::u32);
            geometry->VertexPtr = m_vertices;
            geometry->IndexPtr = m_indices;
            geometry->VertexOffset = m_verticesByteSize;
            geometry->IndexOffset = m_indicesByteSize;
            geometry->Format = format;

            m_verticesByteSize += verticesByteSize;
            m_indicesByteSize += indicesByteSize;

            return geometry;
        }

        void mRender::ClearGeometryBuffer()
        {
            m_verticesByteSize = 0;
            m_indicesByteSize = 0;
        }

        void mRender::ClearRenderPass(sRenderPass* renderPass, core::boolean clearColor, core::usize bufferIndex, const glm::vec4& color, core::boolean clearDepth, float depth)
        {
            m_context->BindRenderPass(renderPass);
            if (clearColor == core::K_TRUE) {
                m_context->ClearColor(bufferIndex, color);
            }
            if (clearDepth == core::K_TRUE) {
                m_context->ClearDepth(depth);
            }
            m_context->UnbindRenderPass(renderPass);
        }

        void mRender::ClearRenderPasses(const glm::vec4& clearColor, const float clearDepth)
        {
            m_context->BindRenderPass(m_opaque);
            m_context->ClearColor(0, clearColor);
            m_context->ClearDepth(clearDepth);
            m_context->UnbindRenderPass(m_opaque);

            m_context->BindRenderPass(m_transparent);
            m_context->ClearColor(0, glm::vec4(0.0f));
            m_context->ClearColor(1, glm::vec4(1.0f));
            m_context->UnbindRenderPass(m_transparent);
        }

        void mRender::DrawGeometryOpaque(core::cApplication* application, sVertexBufferGeometry* geometry, core::cScene* scene)
        {
            core::sCCamera* cam = scene->Get<core::sCCamera>(m_camera);

            m_instancesByteSize = 0;
            m_materialsByteSize = 0;
            m_materialsMap->clear();

            sInstanceList* list = new sInstanceList((sVertexBufferGeometry*)geometry, scene, 0);
            scene->ForEach<core::sCGeometry>(
                application,
                [this, geometry, list]
                (core::cApplication* app, core::cScene* scene_, core::sCGeometry* geometry_)
                {
                    core::sCGeometryInfo* geometryInfo = scene_->Get<core::sCGeometryInfo>(geometry_->Owner);
                    if (geometry_->Geometry == geometry &&
                        geometryInfo->IsVisible == core::K_TRUE &&
                        geometryInfo->IsOpaque == core::K_TRUE)
                    {
                        core::sCTransform* transform = list->Scene->Get<core::sCTransform>(geometry_->Owner);
                        core::sCMaterial* material = list->Scene->Get<core::sCMaterial>(geometry_->Owner);
                        core::sCAnimation* animation = list->Scene->Get<core::sCAnimation>(geometry_->Owner);
                        transform->Transform();

                        core::s32 materialIndex = -1;
                        auto it = m_materialsMap->find(material);
                        if (it == m_materialsMap->end())
                        {
                            materialIndex = m_materialsMap->size();

                            sMaterialInstance m;
                            m.BufferIndex = materialIndex;
                            m.DiffuseColor = material->DiffuseColor;
                            m.HighlightColor = material->HighlightColor;
                            if (material->DiffuseTexture != nullptr)
                            {
                                core::sArea* frame = material->DiffuseTexture;
                                m.SetDiffuseTexture(textureManager->CalculateNormalizedArea(*frame));
                            }
                            else
                            {
                                m.DiffuseTextureLayerInfo = -1.0f;
                            }

                            if (animation != nullptr)
                            {
                                core::sArea* frame = animation->Frames[animation->CurrentAnimationIndex]->at(animation->CurrentFrameIndex[animation->CurrentAnimationIndex]);
                                m.SetDiffuseTexture(*frame);
                                m.DiffuseColor.a = 1.0f - animation->Fade;
                            }

                            m_materialsMap->insert({ material, materialIndex });

                            memcpy((void*)((core::usize)m_materials + (core::usize)m_materialsByteSize), &m, sizeof(sMaterialInstance));
                            m_materialsByteSize += sizeof(sMaterialInstance);
                        }
                        else
                        {
                            materialIndex = it->second;
                        }

                        sRenderInstance i;
                        i.Use2D = transform->Use2D;
                        i.MaterialIndex = materialIndex;
                        i.World = transform->World;

                        memcpy((void*)((core::usize)m_instances + (core::usize)m_instancesByteSize), &i, sizeof(sRenderInstance));
                        m_instancesByteSize += sizeof(sRenderInstance);

                        list->InstanceCount += 1;
                    }
                }
            );

            m_context->WriteBuffer(m_instanceBuffer, 0, m_instancesByteSize, m_instances);
            m_context->WriteBuffer(m_materialBuffer, 0, m_materialsByteSize, m_materials);

            m_context->BindRenderPass(m_opaque);

            m_context->SetShaderUniform(m_opaque->Desc.Shader, "ViewProjection", cam->ViewProjection);

            m_context->Draw(
                list->Geometry->IndexCount,
                list->Geometry->VertexOffset / (core::usize)list->Geometry->Format,
                list->Geometry->IndexOffset,
                list->InstanceCount
            );

            m_context->UnbindRenderPass(m_opaque);

            delete list;
        }

        void mRender::DrawGeometryTransparent(core::cApplication* application, sVertexBufferGeometry* geometry, core::cScene* scene)
        {
            core::sCCamera* cam = scene->Get<core::sCCamera>(m_camera);

            m_instancesByteSize = 0;
            m_materialsByteSize = 0;
            m_materialsMap->clear();

            sInstanceList* list = new sInstanceList((sVertexBufferGeometry*)geometry, scene, 0);
            scene->ForEach<core::sCGeometry>(
                application,
                [this, geometry, list]
                        (core::cApplication* app, core::cScene* scene_, core::sCGeometry* geometry_)
                {
                    core::sCGeometryInfo* geometryInfo = scene_->Get<core::sCGeometryInfo>(geometry_->Owner);
                    if (geometry_->Geometry == geometry &&
                        geometryInfo->IsVisible == core::K_TRUE &&
                        geometryInfo->IsOpaque == core::K_FALSE)
                    {
                        core::sCTransform* transform = list->Scene->Get<core::sCTransform>(geometry_->Owner);
                        core::sCMaterial* material = list->Scene->Get<core::sCMaterial>(geometry_->Owner);
                        core::sCAnimation* animation = list->Scene->Get<core::sCAnimation>(geometry_->Owner);
                        transform->Transform();

                        core::s32 materialIndex = -1;
                        auto it = m_materialsMap->find(material);
                        if (it == m_materialsMap->end())
                        {
                            materialIndex = m_materialsMap->size();

                            sMaterialInstance m;
                            m.BufferIndex = materialIndex;
                            m.DiffuseColor = material->DiffuseColor;
                            if (material->DiffuseTexture != nullptr)
                            {
                                core::sArea* frame = material->DiffuseTexture;
                                m.SetDiffuseTexture(textureManager->CalculateNormalizedArea(*frame));
                            }
                            else
                            {
                                m.DiffuseTextureLayerInfo = -1.0f;
                            }

                            if (animation != nullptr)
                            {
                                core::sArea* frame = animation->Frames[animation->CurrentAnimationIndex]->at(animation->CurrentFrameIndex[animation->CurrentAnimationIndex]);
                                m.SetDiffuseTexture(*frame);
                                m.DiffuseColor.a = 1.0f - animation->Fade;
                            }

                            m_materialsMap->insert({ material, materialIndex });

                            memcpy((void*)((core::usize)m_materials + (core::usize)m_materialsByteSize), &m, sizeof(sMaterialInstance));
                            m_materialsByteSize += sizeof(sMaterialInstance);
                        }
                        else
                        {
                            materialIndex = it->second;
                        }

                        sRenderInstance i;
                        i.Use2D = transform->Use2D;
                        i.MaterialIndex = materialIndex;
                        i.World = transform->World;

                        memcpy((void*)((core::usize)m_instances + (core::usize)m_instancesByteSize), &i, sizeof(sRenderInstance));
                        m_instancesByteSize += sizeof(sRenderInstance);

                        list->InstanceCount += 1;
                    }
                }
            );

            m_context->WriteBuffer(m_instanceBuffer, 0, m_instancesByteSize, m_instances);
            m_context->WriteBuffer(m_materialBuffer, 0, m_materialsByteSize, m_materials);

            m_context->BindRenderPass(m_transparent);

            m_context->SetShaderUniform(m_transparent->Desc.Shader, "ViewProjection", cam->ViewProjection);

            m_context->Draw(
                    list->Geometry->IndexCount,
                    list->Geometry->VertexOffset / (core::usize)list->Geometry->Format,
                    list->Geometry->IndexOffset,
                    list->InstanceCount
            );

            m_context->UnbindRenderPass(m_transparent);
        }

        void mRender::DrawTexts(core::cApplication* app, core::cScene* scene)
        {
            scene->ForEach<core::sCText>(
                app,
                [this](core::cApplication* app_, core::cScene* scene_, core::sCText* text_)
                {
                    m_instancesByteSize = 0;
                    m_materialsByteSize = 0;
                    m_materialsMap->clear();

                    core::sCTransform* transform = scene_->Get<core::sCTransform>(text_->Owner);

                    glm::vec2 windowSize = userInputManager->GetWindowSize();
                    glm::vec2 position = glm::vec2((transform->Position.x * 2.0f) - 1.0f, (transform->Position.y * 2.0f) - 1.0f);
                    glm::vec2 scaleFactor = glm::vec2(
                        (transform->Scale.x - fontManager->GetTextWidth(text_->Font, text_->Text)) / (float)fontManager->GetCharacterCount(text_->Text),
                        (transform->Scale.y - fontManager->GetTextHeight(text_->Font, text_->Text)) / ((float)fontManager->GetNewlineCount(text_->Text) + 1)
                    );

                    core::usize charCount = strlen(text_->Text);

                    core::usize actualCharCount = 0;
                    glm::vec2 offset = glm::vec2(0.0f);

                    for (core::s32 i = 0; i < charCount; i++) {
                        if (text_->Text[i] == '\n') {
                            offset.y += scaleFactor.y + (float)text_->Font->OffsetNewline;
                        }
                    }

                    for (core::s32 i = 0; i < charCount; i++)
                    {
                        char glyphChar = text_->Text[i];
                        if (glyphChar == '\n') { glyphChar = ' '; }

                        if (text_->Text[i] == '\t')
                        {
                            offset.x += scaleFactor.x + (float)text_->Font->OffsetWhitespace;
                            continue;
                        }
                        else if (text_->Text[i] == '\n')
                        {
                            offset.x = 0.0f;
                            offset.y -= scaleFactor.y + (float)text_->Font->OffsetNewline;
                            continue;
                        }
                        else if (text_->Text[i] == ' ')
                        {
                            offset.x += scaleFactor.x + (float)text_->Font->OffsetSpace;
                            continue;
                        }

                        const font::sFont::sGlyph& glyph = text_->Font->Alphabet.find(glyphChar)->second;

                        sTextInstance t;
                        t.Info.x = position.x + offset.x;
                        t.Info.y = position.y + (offset.y - (float)((glyph.Height - glyph.Top) / windowSize.y));
                        t.Info.z = scaleFactor.x + ((float)glyph.Width / windowSize.x);
                        t.Info.w = scaleFactor.y + ((float)glyph.Height / windowSize.y);
                        t.AtlasInfo.x = (float)glyph.AtlasXOffset / (float)text_->Font->Atlas->Width;
                        t.AtlasInfo.y = (float)glyph.AtlasYOffset / (float)text_->Font->Atlas->Height;
                        t.AtlasInfo.z = (float)glyph.Width / (float)text_->Font->Atlas->Width;
                        t.AtlasInfo.w = (float)glyph.Height / (float)text_->Font->Atlas->Height;

                        offset.x += scaleFactor.x + ((float)glyph.Width / windowSize.x) + (0.5f * (glyph.Left / windowSize.x));

                        memcpy((void*)((core::usize)m_instances + (core::usize)m_instancesByteSize), &t, sizeof(sTextInstance));
                        m_instancesByteSize += sizeof(sTextInstance);

                        actualCharCount += 1;
                    }

                    core::sCMaterial* material = scene_->Get<core::sCMaterial>(text_->Owner);
                    sMaterialInstance m;
                    m.BufferIndex = 0;
                    m.DiffuseColor = material->DiffuseColor;

                    memcpy(m_materials, &m, sizeof(sMaterialInstance));
                    m_materialsByteSize += sizeof(sMaterialInstance);

                    m_context->WriteBuffer(m_instanceBuffer, 0, m_instancesByteSize, m_instances);
                    m_context->WriteBuffer(m_materialBuffer, 0, m_materialsByteSize, m_materials);

                    m_context->BindRenderPass(m_text);
                    m_context->BindTexture(m_text->Desc.Shader, "FontAtlas", text_->Font->Atlas);
                    m_context->DrawQuads(actualCharCount);
                    m_context->UnbindRenderPass(m_text);
                }
            );
        }

        void mRender::CompositeTransparent()
        {
            m_context->BindRenderPass(m_compositeTransparent);
            m_context->DrawQuad();
            m_context->UnbindRenderPass(m_compositeTransparent);
        }

        void mRender::CompositeFinal()
        {
            m_context->BindRenderPass(m_compositeFinal);
            m_context->DrawQuad();
            m_context->UnbindRenderPass(m_compositeFinal);
        }

        void mRender::DrawCaptions(core::cApplication* app, core::cScene* scene)
        {
            m_context->BindRenderPass(m_text);

            scene->ForEach<core::sCCaption>(
                app,
                [this](core::cApplication* app_, core::cScene* scene_, core::sCCaption* widget_)
                {
                    m_instancesByteSize = 0;
                    m_materialsByteSize = 0;
                    m_materialsMap->clear();

                    if (widget_->IsVisible == core::K_FALSE) {
                        return;
                    }

                    glm::vec2 windowSize = userInputManager->GetWindowSize();
                    glm::vec2 position = (widget_->Position * 2.0f) - 1.0f;

                    core::usize charCount = strlen(widget_->Text);

                    core::usize actualCharCount = 0;
                    glm::vec2 offset = glm::vec2(0.0f);
                    for (core::s32 i = 0; i < charCount; i++)
                    {
                        char glyphChar = widget_->Text[i];
                        if (glyphChar == '\n') { glyphChar = ' '; }

                        if (widget_->Text[i] == '\t')
                        {
                            offset.x += widget_->Scale * (float)widget_->Font->OffsetWhitespace;
                            continue;
                        }
                        else if (widget_->Text[i] == '\n')
                        {
                            offset.x = 0.0f;
                            offset.y -= widget_->Scale * (float)widget_->Font->OffsetNewline;
                            continue;
                        }
                        else if (widget_->Text[i] == ' ')
                        {
                            offset.x += widget_->Scale * (float)widget_->Font->OffsetSpace;
                            continue;
                        }

                        const font::sFont::sGlyph& glyph = widget_->Font->Alphabet.find(glyphChar)->second;

                        sTextInstance t;
                        t.Info.x = position.x + offset.x;
                        t.Info.y = position.y + (offset.y - (widget_->Scale * (float)((glyph.Height - glyph.Top) / windowSize.y)));
                        t.Info.z = widget_->Scale * ((float)glyph.Width / windowSize.x);
                        t.Info.w = widget_->Scale * ((float)glyph.Height / windowSize.y);
                        t.AtlasInfo.x = (float)glyph.AtlasXOffset / (float)widget_->Font->Atlas->Width;
                        t.AtlasInfo.y = (float)glyph.AtlasYOffset / (float)widget_->Font->Atlas->Height;
                        t.AtlasInfo.z = (float)glyph.Width / (float)widget_->Font->Atlas->Width;
                        t.AtlasInfo.w = (float)glyph.Height / (float)widget_->Font->Atlas->Height;

                        offset.x += widget_->Scale * (((float)glyph.Width / windowSize.x) + (0.5f * (glyph.Left / windowSize.x)));

                        memcpy((void*)((core::usize)m_instances + (core::usize)m_instancesByteSize), &t, sizeof(sTextInstance));
                        m_instancesByteSize += sizeof(sTextInstance);

                        actualCharCount += 1;
                    }

                    core::sCMaterial* material = scene_->Get<core::sCMaterial>(widget_->Owner);
                    sMaterialInstance m;
                    m.BufferIndex = 0;
                    m.DiffuseColor = material->DiffuseColor;

                    memcpy(m_materials, &m, sizeof(sMaterialInstance));
                    m_materialsByteSize += sizeof(sMaterialInstance);

                    m_context->WriteBuffer(m_instanceBuffer, 0, m_instancesByteSize, m_instances);
                    m_context->WriteBuffer(m_materialBuffer, 0, m_materialsByteSize, m_materials);

                    m_context->BindTexture(m_text->Desc.Shader, "FontAtlas", widget_->Font->Atlas);
                    m_context->DrawQuads(actualCharCount);
                }
            );

            m_context->UnbindRenderPass(m_text);
        }

        void mRender::DrawButtons(core::cApplication* app, core::cScene* scene)
        {
            m_instancesByteSize = 0;
            m_materialsByteSize = 0;
            m_materialsMap->clear();

            core::usize widgetCount = 0;
            scene->ForEach<core::sCButton>(app, [this, scene, &widgetCount](core::cApplication* app_, core::cScene* scene_, core::sCButton* widget_)
                {
                    const core::sCAnimation* animation = scene->Get<core::sCAnimation>(widget_->Owner);

                    if (widget_->IsVisible == core::K_TRUE && animation != nullptr)
                    {
                        core::sCTransform* transform = scene->Get<core::sCTransform>(widget_->Owner);
                        const core::sCMaterial* material = scene->Get<core::sCMaterial>(widget_->Owner);
                        glm::vec3 position = transform->Position;
                        glm::vec3 scale = transform->Scale;
                        position.x = (position.x * 2.0f) - 1.0f;
                        position.y = (position.y * 2.0f) - 1.0f;

                        core::s32 materialIndex = -1;
                        auto it = m_materialsMap->find((core::sCMaterial*)material);
                        if (it == m_materialsMap->end())
                        {
                            materialIndex = m_materialsMap->size();

                            sMaterialInstance m;
                            m.BufferIndex = materialIndex;
                            m.DiffuseColor = material->DiffuseColor;
                            if (material->DiffuseTexture != nullptr)
                            {
                                core::sArea* frame = material->DiffuseTexture;
                                m.SetDiffuseTexture(textureManager->CalculateNormalizedArea(*frame));
                            }
                            else if (animation != nullptr)
                            {
                                core::sArea* frame = animation->Frames[animation->CurrentAnimationIndex]->at(animation->CurrentFrameIndex[animation->CurrentAnimationIndex]);
                                m.SetDiffuseTexture(*frame);
                                m.DiffuseColor.a = 1.0f - animation->Fade;
                            }

                            m_materialsMap->insert({ (core::sCMaterial*)material, materialIndex });

                            memcpy((void*)((core::usize)m_materials + (core::usize)m_materialsByteSize), &m, sizeof(sMaterialInstance));
                            m_materialsByteSize += sizeof(sMaterialInstance);
                        }
                        else
                        {
                            materialIndex = it->second;
                        }

                        sRenderInstance i;
                        i.Use2D = transform->Use2D;
                        i.MaterialIndex = materialIndex;
                        i.World = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), scale);

                        memcpy((void*)((core::usize)m_instances + (core::usize)m_instancesByteSize), &i, sizeof(sRenderInstance));
                        m_instancesByteSize += sizeof(sRenderInstance);

                        widgetCount += 1;
                    }
                }
            );

            m_context->WriteBuffer(m_instanceBuffer, 0, m_instancesByteSize, m_instances);
            m_context->WriteBuffer(m_materialBuffer, 0, m_materialsByteSize, m_materials);

            m_context->BindRenderPass(m_widget);
            m_context->DrawQuads(widgetCount);
            m_context->UnbindRenderPass(m_widget);
        }

        void mRender::DrawPopupMenus(core::cApplication* app, core::cScene* scene)
        {
            m_instancesByteSize = 0;
            m_materialsByteSize = 0;
            m_materialsMap->clear();

            core::usize widgetCount = 0;
            scene->ForEach<core::sCPopupMenu>(app, [this, scene, &widgetCount](core::cApplication* app_, core::cScene* scene_, core::sCPopupMenu* widget_)
                {
                    core::sCAnimation* animation = scene->Get<core::sCAnimation>(widget_->Owner);

                    if (widget_->IsVisible == core::K_TRUE && animation != nullptr)
                    {
                        core::sCTransform* transform = scene->Get<core::sCTransform>(widget_->Owner);
                        const core::sCMaterial* material = scene->Get<core::sCMaterial>(widget_->Owner);
                        glm::vec3 position = transform->Position;
                        glm::vec3 scale = transform->Scale;
                        position.x = (position.x * 2.0f) - 1.0f;
                        position.y = (position.y * 2.0f) - 1.0f;

                        core::s32 materialIndex = -1;
                        auto it = m_materialsMap->find((core::sCMaterial*)material);
                        if (it == m_materialsMap->end())
                        {
                            materialIndex = m_materialsMap->size();

                            sMaterialInstance m;
                            m.BufferIndex = materialIndex;
                            m.DiffuseColor = material->DiffuseColor;
                            if (material->DiffuseTexture != nullptr)
                            {
                                core::sArea* frame = material->DiffuseTexture;
                                m.SetDiffuseTexture(textureManager->CalculateNormalizedArea(*frame));
                            }
                            else if (animation != nullptr)
                            {
                                core::sArea* frame = animation->Frames[animation->CurrentAnimationIndex]->at(animation->CurrentFrameIndex[animation->CurrentAnimationIndex]);
                                m.SetDiffuseTexture(*frame);
                                m.DiffuseColor.a = 1.0f - animation->Fade;
                            }

                            m_materialsMap->insert({ (core::sCMaterial*)material, materialIndex });

                            memcpy((void*)((core::usize)m_materials + (core::usize)m_materialsByteSize), &m, sizeof(sMaterialInstance));
                            m_materialsByteSize += sizeof(sMaterialInstance);
                        }
                        else
                        {
                            materialIndex = it->second;
                        }

                        sRenderInstance i;
                        i.Use2D = transform->Use2D;
                        i.MaterialIndex = materialIndex;
                        i.World = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), scale);

                        memcpy((void*)((core::usize)m_instances + (core::usize)m_instancesByteSize), &i, sizeof(sRenderInstance));
                        m_instancesByteSize += sizeof(sRenderInstance);

                        widgetCount += 1;
                    }
                }
            );

            m_context->WriteBuffer(m_instanceBuffer, 0, m_instancesByteSize, m_instances);
            m_context->WriteBuffer(m_materialBuffer, 0, m_materialsByteSize, m_materials);

            m_context->BindRenderPass(m_widget);
            m_context->DrawQuads(widgetCount);
            m_context->UnbindRenderPass(m_widget);
        }

        void mRender::DrawCheckboxes(core::cApplication* app, core::cScene* scene)
        {
            m_instancesByteSize = 0;
            m_materialsByteSize = 0;
            m_materialsMap->clear();

            core::usize widgetCount = 0;
            scene->ForEach<core::sCCheckbox>(app, [this, scene, &widgetCount](core::cApplication* app_, core::cScene* scene_, core::sCCheckbox* widget_)
               {
                   const core::sCAnimation* animation = scene->Get<core::sCAnimation>(widget_->Owner);

                   if (widget_->IsVisible == core::K_TRUE && animation != nullptr)
                   {
                       core::sCTransform* transform = scene->Get<core::sCTransform>(widget_->Owner);
                       const core::sCMaterial* material = scene->Get<core::sCMaterial>(widget_->Owner);
                       glm::vec3 position = transform->Position;
                       glm::vec3 scale = transform->Scale;
                       position.x = (position.x * 2.0f) - 1.0f;
                       position.y = (position.y * 2.0f) - 1.0f;

                       core::s32 materialIndex = -1;
                       auto it = m_materialsMap->find((core::sCMaterial*)material);
                       if (it == m_materialsMap->end())
                       {
                           materialIndex = m_materialsMap->size();

                           sMaterialInstance m;
                           m.BufferIndex = materialIndex;
                           m.DiffuseColor = material->DiffuseColor;
                           if (material->DiffuseTexture != nullptr)
                           {
                               core::sArea* frame = material->DiffuseTexture;
                               m.SetDiffuseTexture(textureManager->CalculateNormalizedArea(*frame));
                           }
                           else if (animation != nullptr)
                           {
                               core::sArea* frame = animation->Frames[animation->CurrentAnimationIndex]->at(animation->CurrentFrameIndex[animation->CurrentAnimationIndex]);
                               m.SetDiffuseTexture(*frame);
                               m.DiffuseColor.a = 1.0f - animation->Fade;
                           }

                           m_materialsMap->insert({ (core::sCMaterial*)material, materialIndex });

                           memcpy((void*)((core::usize)m_materials + (core::usize)m_materialsByteSize), &m, sizeof(sMaterialInstance));
                           m_materialsByteSize += sizeof(sMaterialInstance);
                       }
                       else
                       {
                           materialIndex = it->second;
                       }

                       sRenderInstance i;
                       i.Use2D = transform->Use2D;
                       i.MaterialIndex = materialIndex;
                       i.World = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), scale);

                       memcpy((void*)((core::usize)m_instances + (core::usize)m_instancesByteSize), &i, sizeof(sRenderInstance));
                       m_instancesByteSize += sizeof(sRenderInstance);

                       widgetCount += 1;
                   }
               }
            );

            m_context->WriteBuffer(m_instanceBuffer, 0, m_instancesByteSize, m_instances);
            m_context->WriteBuffer(m_materialBuffer, 0, m_materialsByteSize, m_materials);

            m_context->BindRenderPass(m_widget);
            m_context->DrawQuads(widgetCount);
            m_context->UnbindRenderPass(m_widget);
        }

        void mRender::DrawWidgets(core::cApplication* app, core::cScene* scene)
        {
            DrawCaptions(app, scene);
            DrawButtons(app, scene);
            DrawPopupMenus(app, scene);
            DrawCheckboxes(app, scene);
        }

        sPrimitive* mRender::CreateTriangle()
        {
            sPrimitive* primitive = new sPrimitive();
            primitive->Format = render::sVertexBufferGeometry::eFormat::POSITION_TEXCOORD_NORMAL_VEC3_VEC2_VEC3;
            primitive->Vertices = (render::sVertex*)malloc(sizeof(render::sVertex) * 3);
            primitive->Indices = (render::index*)malloc(sizeof(render::index) * 3);
            primitive->VertexCount = 3;
            primitive->IndexCount = 3;
            primitive->VerticesByteSize = sizeof(render::sVertex) * 3;
            primitive->IndicesByteSize = sizeof(render::index) * 3;

            primitive->Vertices[0].Position[0] = -1.0f; primitive->Vertices[0].Position[1] = -1.0f; primitive->Vertices[0].Position[2] = 0.0f;
            primitive->Vertices[0].Texcoord[0] = 0.0f; primitive->Vertices[0].Texcoord[1] = 0.0f;
            primitive->Vertices[0].Normal[0] = 0.0f; primitive->Vertices[0].Normal[1] = 0.0f; primitive->Vertices[0].Normal[2] = 1.0f;
            primitive->Vertices[1].Position[0] = 0.0f; primitive->Vertices[1].Position[1] = 1.0f; primitive->Vertices[1].Position[2] = 0.0f;
            primitive->Vertices[1].Texcoord[0] = 0.5f; primitive->Vertices[1].Texcoord[1] = 1.0f;
            primitive->Vertices[1].Normal[0] = 0.0f; primitive->Vertices[1].Normal[1] = 0.0f; primitive->Vertices[1].Normal[2] = 1.0f;
            primitive->Vertices[2].Position[0] = 1.0f; primitive->Vertices[2].Position[1] = -1.0f; primitive->Vertices[2].Position[2] = 0.0f;
            primitive->Vertices[2].Texcoord[0] = 1.0f; primitive->Vertices[2].Texcoord[1] = 0.0f;
            primitive->Vertices[2].Normal[0] = 0.0f; primitive->Vertices[2].Normal[1] = 0.0f; primitive->Vertices[2].Normal[2] = 1.0f;
            primitive->Indices[0] = 0;
            primitive->Indices[1] = 1;
            primitive->Indices[2] = 2;

            return primitive;
        }

        sPrimitive* mRender::CreateQuad()
        {
            sPrimitive* primitive = new sPrimitive();
            primitive->Format = render::sVertexBufferGeometry::eFormat::POSITION_TEXCOORD_NORMAL_VEC3_VEC2_VEC3;
            primitive->Vertices = (render::sVertex*)malloc(sizeof(render::sVertex) * 4);
            primitive->Indices = (render::index*)malloc(sizeof(render::index) * 6);
            primitive->VertexCount = 4;
            primitive->IndexCount = 6;
            primitive->VerticesByteSize = sizeof(render::sVertex) * 4;
            primitive->IndicesByteSize = sizeof(render::index) * 6;

            primitive->Vertices[0].Position[0] = -1.0f; primitive->Vertices[0].Position[1] = -1.0f; primitive->Vertices[0].Position[2] = 0.0f;
            primitive->Vertices[0].Texcoord[0] = 0.0f; primitive->Vertices[0].Texcoord[1] = 0.0f;
            primitive->Vertices[0].Normal[0] = 0.0f; primitive->Vertices[0].Normal[1] = 0.0f; primitive->Vertices[0].Normal[2] = 1.0f;
            primitive->Vertices[1].Position[0] = -1.0f; primitive->Vertices[1].Position[1] = 1.0f; primitive->Vertices[1].Position[2] = 0.0f;
            primitive->Vertices[1].Texcoord[0] = 0.0f; primitive->Vertices[1].Texcoord[1] = 1.0f;
            primitive->Vertices[1].Normal[0] = 0.0f; primitive->Vertices[1].Normal[1] = 0.0f; primitive->Vertices[1].Normal[2] = 1.0f;
            primitive->Vertices[2].Position[0] = 1.0f; primitive->Vertices[2].Position[1] = -1.0f; primitive->Vertices[2].Position[2] = 0.0f;
            primitive->Vertices[2].Texcoord[0] = 1.0f; primitive->Vertices[2].Texcoord[1] = 0.0f;
            primitive->Vertices[2].Normal[0] = 0.0f; primitive->Vertices[2].Normal[1] = 0.0f; primitive->Vertices[2].Normal[2] = 1.0f;
            primitive->Vertices[3].Position[0] = 1.0f; primitive->Vertices[3].Position[1] = 1.0f; primitive->Vertices[3].Position[2] = 0.0f;
            primitive->Vertices[3].Texcoord[0] = 1.0f; primitive->Vertices[3].Texcoord[1] = 1.0f;
            primitive->Vertices[3].Normal[0] = 0.0f; primitive->Vertices[3].Normal[1] = 0.0f; primitive->Vertices[3].Normal[2] = 1.0f;
            primitive->Indices[0] = 0;
            primitive->Indices[1] = 1;
            primitive->Indices[2] = 2;
            primitive->Indices[3] = 1;
            primitive->Indices[4] = 3;
            primitive->Indices[5] = 2;

            return primitive;
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
            {
                return nullptr;
            }

            // Load vertices
            core::usize totalVertexCount = 0;
            model->Vertices = (render::sVertex*)malloc(scene->mMeshes[0]->mNumVertices * sizeof(render::sVertex));
            memset(model->Vertices, 0, scene->mMeshes[0]->mNumVertices * sizeof(render::sVertex));
            for (core::s32 i = 0; i < scene->mMeshes[0]->mNumVertices; i++)
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
            core::usize totalIndexCount = 0;
            model->Indices = (render::index*)malloc(scene->mMeshes[0]->mNumFaces * 3 * sizeof(render::index));
            memset(model->Indices, 0, scene->mMeshes[0]->mNumFaces * 3 * sizeof(render::index));
            for (core::s32 i = 0; i < scene->mMeshes[0]->mNumFaces; i++)
            {
                aiFace face = scene->mMeshes[0]->mFaces[i];
                for (core::s32 j = 0; j < face.mNumIndices; j++)
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

        void mRender::FreePrimitive(sPrimitive* primitive)
        {
            free(primitive->Vertices);
            free(primitive->Indices);
            delete primitive;
        }

        void mRender::ResizeWindow(const glm::vec2& size)
        {
            m_context->UnbindRenderPass(m_opaque);
            m_context->UnbindRenderPass(m_transparent);

            m_opaque->Desc.Viewport[2] = size.x;
            m_opaque->Desc.Viewport[3] = size.y;
            m_context->ResizeRenderTargetColors(m_opaque->Desc.RenderTarget, size);
            m_transparent->Desc.Viewport[2] = size.x;
            m_transparent->Desc.Viewport[3] = size.y;
            m_context->ResizeRenderTargetColors(m_transparent->Desc.RenderTarget, size);
            m_opaque->Desc.RenderTarget->DepthAttachment = m_context->ResizeTexture(m_opaque->Desc.RenderTarget->DepthAttachment, size);
            m_transparent->Desc.RenderTarget->DepthAttachment = m_opaque->Desc.RenderTarget->DepthAttachment;
            m_context->UpdateRenderTargetBuffers(m_opaque->Desc.RenderTarget);
            m_context->UpdateRenderTargetBuffers(m_transparent->Desc.RenderTarget);
            m_transparent->Desc.RenderTarget->ColorAttachments[0]->Slot = 0;
            m_transparent->Desc.RenderTarget->ColorAttachments[1]->Slot = 1;

            m_compositeTransparent->Desc.Viewport[2] = size.x;
            m_compositeTransparent->Desc.Viewport[3] = size.y;
            m_compositeTransparent->Desc.InputTextures.clear();
            m_compositeTransparent->Desc.InputTextureNames.clear();
            m_compositeTransparent->Desc.InputTextures.emplace_back(m_transparent->Desc.RenderTarget->ColorAttachments[0]);
            m_compositeTransparent->Desc.InputTextureNames.emplace_back("AccumulationTexture");
            m_compositeTransparent->Desc.InputTextures.emplace_back(m_transparent->Desc.RenderTarget->ColorAttachments[1]);
            m_compositeTransparent->Desc.InputTextureNames.emplace_back("RevealageTexture");

            m_compositeFinal->Desc.Viewport[2] = size.x;
            m_compositeFinal->Desc.Viewport[3] = size.y;
            m_compositeFinal->Desc.InputTextures.clear();
            m_compositeFinal->Desc.InputTextureNames.clear();
            m_compositeFinal->Desc.InputTextures.emplace_back(m_opaque->Desc.RenderTarget->ColorAttachments[0]);
            m_compositeFinal->Desc.InputTextureNames.emplace_back("ColorTexture");
        }
    }
}