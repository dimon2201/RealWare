#pragma once

#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>
#include <typeinfo>
#include <functional>

#include "../../thirdparty/glm/glm/gtc/quaternion.hpp"
#include "../../thirdparty/glm/glm/gtx/quaternion.hpp"
#include "../../thirdparty/glm/glm/glm.hpp"
#include "layer.hpp"
#include "types.hpp"

namespace physx
{
    class PxScene;
    class PxActor;
    class PxControllerManager;
    class PxController;
    class PxMaterial;
}

namespace realware
{
    namespace render
    {
        struct sVertexBufferGeometry;
        struct sMaterial;
    }

    namespace font
    {
        struct sFont;
    }

    namespace sound
    {
        struct sSound;
    }

    namespace core
    {
        struct sArea;
        class cApplication;
        class cUserInput;
        class cScene;

        using entity = u64;

        struct sComponent
        {
            entity Owner = {};
        };

        struct sEntityScenePair
        {
            sEntityScenePair() = default;
            sEntityScenePair(entity entity_, cScene* scene) : Entity(entity_), Scene(scene) {}

            entity Entity;
            cScene* Scene = nullptr;
        };

        struct sCAssetName : public sComponent
        {
            u8 AssetName[256] = {};
        };

        struct sCTransform : public sComponent
        {
            void Init(boolean use2D, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale)
            {
                Use2D = use2D;
                Position = position;
                Rotation = rotation;
                Scale = scale;
            }

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

        struct sCMaterial : public sComponent
        {
            void Init(sArea* diffuseTexture, const glm::vec4& diffuseColor)
            {
                DiffuseTexture = diffuseTexture;
                DiffuseColor = diffuseColor;
            }

            sArea* DiffuseTexture = nullptr;
            glm::vec4 DiffuseColor = glm::vec4(1.0f);
            glm::vec4 HighlightColor = glm::vec4(1.0f);
        };

        struct sCWidget : public sComponent
        {
            enum eType
            {
                CAPTION = 0,
                BUTTON = 1,
                POPUP_MENU = 2,
                CHECKBOX = 3,
                SLIDER = 4,
                SLIDER_BUTTON = 5
            };

            void Init(const eType& type) { Type = type; }

            inline boolean CheckStateActive() { return StateBits & 1 == 1; }
            inline boolean CheckStateUnactive() { return StateBits == 0; }

            entity Parent = 0;
            eType Type;
            boolean IsHovered = core::K_FALSE;
            boolean IsVisible = core::K_TRUE;
            word StateBits = 0;
            std::function<void(cApplication*, cScene*, sCWidget*)> OnUpdate;
            std::function<void(cApplication*, cScene*, sCWidget*)> OnHover;
            std::function<void(cApplication*, cScene*, sCWidget*)> OnClick;
        };

        struct sCCaption : public sCWidget
        {
            inline void Init(const eType& type)
            {
                IsVisible = core::K_TRUE;
                Type = type;
            }

            font::sFont* Font;
            const char* Text;
            float Scale;
            glm::vec2 Position;
        };

        struct sCButton : public sCWidget
        {
            inline void Init(const eType& type)
            {
                IsVisible = core::K_TRUE;
                Type = type;
            }

            core::boolean AlignTextToCenter = core::K_TRUE;
        };

        struct sCPopupMenu : public sCWidget
        {
            inline void Init(const eType& type)
            {
                IsVisible = core::K_TRUE;
                Type = type;
            }

            core::boolean HideElements = core::K_TRUE;
            core::s32 SelectedElementIndex = -1;
            core::usize ElementCount = 0;
            core::entity Elements[16] = {};
        };

        struct sCCheckbox : public sCWidget
        {
            inline void Init(const eType& type)
            {
                IsVisible = core::K_TRUE;
                Type = type;
            }

            core::boolean Value = core::K_FALSE;
        };

        struct sCAnimation : public sComponent
        {
            inline void Init()
            {
                Tick = -1.0f;
                MaxTick = 0.0f;
                Fade = 0.0f;
            }

            core::s32 CurrentAnimationIndex = 0;
            std::vector<sArea*>* Frames[16] = {};
            core::s32 CurrentFrameIndex[16] = {};
            float Tick;
            float MaxTick;
            float Fade;
        };

        struct sCGeometryInfo : public sComponent
        {
            boolean IsVisible;
            boolean IsOpaque;
        };

        struct sCGeometry : public sComponent
        {
            render::sVertexBufferGeometry* Geometry;
        };

        struct sCText : public sComponent
        {
            font::sFont* Font;
            const char* Text;
        };

        struct sCSound : public sComponent
        {
            sound::sSound* Sound;
        };

        struct sCCamera : public sComponent
        {
            float FOV = 0.0f;
            float ZNear = 0.0f;
            float ZFar = 0.0f;
            glm::vec3 Direction = glm::vec3(0.0f, 0.0f, -1.0f);
            glm::vec3 Euler = glm::vec3(0.0f);
            glm::mat4 View = glm::mat4(1.0f);
            glm::mat4 Projection = glm::mat4(1.0f);
            glm::mat4 ViewProjection = glm::mat4(1.0f);
            boolean IsMoving = K_FALSE;
        };

        struct sCInitCallback : public sComponent
        {
            std::function<void(cApplication*, cScene*, sCInitCallback*)> OnExecute;
        };

        struct sCUpdateCallback : public sComponent
        {
            std::function<void(cApplication*, cScene*, sCUpdateCallback*)> OnExecute;
        };

        struct sCPhysicsScene : public sComponent
        {
            physx::PxScene* Scene;
            physx::PxControllerManager* ControllerManager;
        };

        struct sCPhysicsActor : public sComponent
        {
            physx::PxActor* Actor;
            physx::PxMaterial* Material;
        };

        struct sCPhysicsCharacterController : public sComponent
        {
            physx::PxController* Controller;
            physx::PxMaterial* Material;
            float GravitySpeed = 0.05;
            boolean IsGravityEnabled = K_TRUE;
        };

        struct sCLight : public sComponent
        {
            glm::vec3 Color;
            glm::vec3 Direction;
            float Scale;
        };

        struct sComponentArray
        {
            sComponentArray(core::usize componentCount, void* memory) : ComponentCount(componentCount), Memory(memory)
            {
            }

            core::usize ComponentCount;
            void* Memory;
        };

        class cScene
        {

        public:
            cScene(core::usize arrayByteSize)
            {
                m_arrayByteSize = arrayByteSize;

                RegisterComponent<sCAssetName>();
                RegisterComponent<sCTransform>();
                RegisterComponent<sCMaterial>();
                RegisterComponent<sCWidget>();
                RegisterComponent<sCCaption>();
                RegisterComponent<sCButton>();
                RegisterComponent<sCPopupMenu>();
                RegisterComponent<sCCheckbox>();
                RegisterComponent<sCAnimation>();
                RegisterComponent<sCGeometryInfo>();
                RegisterComponent<sCGeometry>();
                RegisterComponent<sCText>();
                RegisterComponent<sCSound>();
                RegisterComponent<sCCamera>();
                RegisterComponent<sCInitCallback>();
                RegisterComponent<sCUpdateCallback>();
                RegisterComponent<sCPhysicsScene>();
                RegisterComponent<sCPhysicsActor>();
                RegisterComponent<sCPhysicsCharacterController>();
                RegisterComponent<sCLight>();
            }

            ~cScene()
            {
                for (auto it = m_arrays.begin(); it != m_arrays.end(); it++) {
                    free(it->second.Memory);
                }
            }

            entity CreateEntity(const std::string& name)
            {
                static u64 uniqueIndex = 1;
                entity ent = uniqueIndex++;
                m_entities.insert({ name, ent });

                return ent;
            }

            void RemoveEntity(const std::string& name)
            {
                for (auto it = m_entities.begin(); it != m_entities.end(); it++)
                {
                    if ((*it).first == name)
                    {
                        m_entities.erase(it);
                        break;
                    }
                }
            }

            void RemoveEntities(const std::vector<entity>& exclude)
            {
                for (auto it = m_entities.begin(); it != m_entities.end();)
                {
                    boolean excluded = K_FALSE;

                    for (auto it2 = exclude.begin(); it2 != exclude.end(); it2++)
                    {
                        if ((*it).second == *it2)
                        {
                            excluded = K_TRUE;
                            break;
                        }
                    }

                    if (excluded == K_TRUE) {
                        it++;
                    } else {
                        it = m_entities.erase(it);
                    }
                }
            }

            entity GetEntity(const std::string& name)
            {
                auto it = m_entities.find(name);
                if (it != m_entities.end()) {
                    return it->second;
                } else {
                    return 0;
                }
            }

            std::string GetEntityName(entity entity_)
            {
                for (auto it = m_entities.begin(); it != m_entities.end(); it++)
                {
                    if (it->second == entity_) {
                        return it->first;
                    }
                }

                return "";
            }

            core::usize GetEntityCount()
            {
                core::usize count = 0;
                for (auto it = m_entities.begin(); it != m_entities.end(); it++) count += 1;

                return count;
            }

            template <typename ComponentType>
            void RegisterComponent()
            {
                std::string componentName = std::string(typeid(ComponentType).name());
                if (m_arrays.find(componentName) != m_arrays.end()) {
                    return;
                }

                void* memory = malloc(m_arrayByteSize);
                memset(memory, 0, m_arrayByteSize);
                m_arrays.insert({ componentName, { 0, memory } });
            }

            template <typename ComponentType>
            ComponentType* Add(entity owner)
            {
                std::string componentName = std::string(typeid(ComponentType).name());
                sComponentArray& array = m_arrays.find(componentName)->second;
                ComponentType* memory = (ComponentType*)array.Memory;
                core::usize index = array.ComponentCount;
                memory[index].Owner = owner;
                array.ComponentCount += 1;

                return &memory[index];
            }

            template <typename ComponentType>
            void Copy(entity owner, ComponentType* src, ComponentType* dst)
            {
                std::string componentName = std::string(typeid(ComponentType).name());
                if (componentName == std::string(typeid(sCAnimation).name()))
                {
                    for (core::usize i = 0; i < 16; i++)
                    {
                        for (auto frame: *((sCAnimation*)src)->Frames[i]) {
                            ((sCAnimation*)dst)->Frames[i]->emplace_back(frame);
                        }
                        ((sCAnimation*)dst)->CurrentFrameIndex[i] = ((sCAnimation*)src)->CurrentFrameIndex[i];
                    }

                    ((sCAnimation*)dst)->Owner = owner;
                    ((sCAnimation*)dst)->CurrentAnimationIndex = ((sCAnimation*)src)->CurrentAnimationIndex;
                    ((sCAnimation*)dst)->Tick = ((sCAnimation*)src)->Tick;
                    ((sCAnimation*)dst)->MaxTick = ((sCAnimation*)src)->MaxTick;
                    ((sCAnimation*)dst)->Fade = ((sCAnimation*)src)->Fade;
                }
                else
                {
                    memcpy(dst, src, sizeof(ComponentType));
                }
            }

            template <typename ComponentType>
            ComponentType* Get(entity owner)
            {
                std::string componentName = std::string(typeid(ComponentType).name());
                sComponentArray& array = m_arrays.find(componentName)->second;
                ComponentType* memory = (ComponentType*)array.Memory;
                for (core::s32 i = 0; i < array.ComponentCount; i++)
                {
                    if (owner == memory[i].Owner) {
                        return &memory[i];
                    }
                }

                return nullptr;
            }

            template <typename ComponentType>
            void Remove(entity owner)
            {
                std::string componentName = std::string(typeid(ComponentType).name());
                sComponentArray& array = m_arrays.find(componentName)->second;
                ComponentType* memory = (ComponentType*)array.Memory;
                core::s32 indexToRemove = -1;
                for (core::s32 i = 0; i < array.ComponentCount; i++) {
                    if (owner == memory[i].Owner) {
                        indexToRemove = i;
                        break;
                    }
                }

                if (indexToRemove != -1)
                {
                    memory[indexToRemove] = memory[array.ComponentCount - 1];
                    array.ComponentCount -= 1;
                }
            }

            template <typename ComponentType>
            void ForEach(cApplication* application, std::function<void(cApplication*, cScene*, ComponentType*)> lambda)
            {
                std::string componentName = std::string(typeid(ComponentType).name());
                sComponentArray& array = m_arrays.find(componentName)->second;
                ComponentType* memory = (ComponentType*)array.Memory;
                for (core::s32 i = 0; i < array.ComponentCount; i++) {
                    lambda(application, this, &memory[i]);
                }
            }

            void ForEachEntity(cApplication* application, std::function<void(cApplication*, cScene*, const std::string&, entity)> lambda)
            {
                for (auto it = m_entities.begin(); it != m_entities.end(); it++)
                {
                    lambda(application, this, it->first, it->second);
                }
            }

            void InitComponents(cApplication* application)
            {
                std::string componentName = std::string(typeid(sCInitCallback).name());
                sComponentArray& array = m_arrays.find(componentName)->second;
                sCInitCallback* memory = (sCInitCallback*)array.Memory;
                for (core::s32 i = 0; i < array.ComponentCount; i++) {
                    memory[i].OnExecute(application, this, &memory[i]);
                }
            }

            void UpdateComponents(cApplication* application)
            {
                std::string componentName = std::string(typeid(sCUpdateCallback).name());
                sComponentArray& array = m_arrays.find(componentName)->second;
                sCUpdateCallback* memory = (sCUpdateCallback*)array.Memory;
                for (core::s32 i = 0; i < array.ComponentCount; i++) {
                    memory[i].OnExecute(application, this, &memory[i]);
                }
            }

            void AddGlobal(const std::string& tag, void* global) { m_globals.insert({ tag, global }); }
            void* GetGlobal(const std::string& tag) { return m_globals.find(tag)->second; }
            void AddLayer(const std::string& tag, cLayer* layer) { m_layers.insert({ tag, layer }); }
            cLayer* GetLayer(const std::string& tag) { return m_layers.find(tag)->second; }
            void ToggleLayer(const std::string& tag, boolean isEnabled) { GetLayer(tag)->Toggle(isEnabled); }
            void InitLayers() { for (auto& layer : m_layers) { if (layer.second->GetState() == K_TRUE) { layer.second->Init(this); } } }
            void FreeLayers() { for (auto& layer : m_layers) { if (layer.second->GetState() == K_TRUE) { layer.second->Free(this); } } }
            void UpdateLayers() { for (auto& layer : m_layers) { if (layer.second->GetState() == K_TRUE) { layer.second->Update(this); } } }

        private:
            core::usize m_arrayByteSize;
            std::unordered_map<std::string, entity> m_entities;
            std::unordered_map<std::string, sComponentArray> m_arrays;
            std::unordered_map<std::string, void*> m_globals;
            std::unordered_map<std::string, cLayer*> m_layers;

        };
    }
}