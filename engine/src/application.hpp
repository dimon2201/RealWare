#pragma once

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <windows.h>
#include <chrono>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "types.hpp"

namespace realware
{
    namespace game
    {
        class mCamera;
        class mGameObject;
    }

    namespace render
    {
        class cRenderContext;
        class cRenderer;
        class mRender;
		class mTexture;
    }

    namespace sound
    {
        class cSoundContext;
        class mSound;
    }

    namespace font
    {
        class cFontManager;
        class mFont;
    }

    namespace physics
    {
        class mPhysics;
    }

    namespace fs
    {
        class mFileSystem;
    }

    namespace utils
    {
        class cMemoryPool;
    }

    namespace app
    {
        void WindowSizeCallback(GLFWwindow* window, int width, int height);

        struct sApplicationDescriptor
        {
            struct sWindowDescriptor
            {
                std::string Title = "DefaultWindow";
                types::u32 Width = 640;
                types::u32 Height = 480;
                types::boolean IsFullscreen = types::K_FALSE;
            };

            sWindowDescriptor WindowDesc;
            types::u32 MemoryPoolByteSize = 1 * 1024 * 1024;
            types::u32 MemoryPoolReservedAllocations = 65536;
            types::u32 MemoryPoolAlignment = 64;
            types::u32 TextureAtlasWidth = 1920;
            types::u32 TextureAtlasHeight = 1080;
            types::u32 TextureAtlasDepth = 16;
            types::u32 VertexBufferSize = 65536;
            types::u32 IndexBufferSize = 65536;
            types::u32 InstanceBufferSize = 65536;
            types::u32 MaterialBufferSize = 65536;
            types::u32 LightBufferSize = 65536;
            types::u32 MaxMaterialCount = 256;
            types::u32 MaxGameObjectCount = 65536;
            types::u32 MaxPhysicsSceneCount = 4;
            types::u32 MaxPhysicsSubstanceCount = 256;
            types::u32 MaxPhysicsActorCount = 65536;
            types::u32 MaxPhysicsControllerCount = 4;
            types::u32 MaxSoundCount = 65536;
            types::u32 MaxTextureCount = 65536;
        };

        class cApplication
        {
        public:
            explicit cApplication(const sApplicationDescriptor* const desc);
            ~cApplication();

            virtual void Start() = 0;
            virtual void FrameUpdate() = 0;
            virtual void Finish() = 0;

            void Run();

            inline types::boolean GetRunState() const { return glfwWindowShouldClose((GLFWwindow*)_window); }

            inline game::mCamera* GetCameraManager() const { return _camera; }
            inline render::mTexture* GetTextureManager() const { return _texture; }
            inline render::mRender* GetRenderManager() const { return _render; }
            inline font::mFont* GetFontManager() const { return _font; }
            inline sound::mSound* GetSoundManager() const { return _sound; }
            inline fs::mFileSystem* GetFileSystemManager() const { return _fileSystem; }
            inline physics::mPhysics* GetPhysicsManager() const { return _physics; }
            inline game::mGameObject* GetGameObjectManager() const { return _gameObject; }
            inline utils::cMemoryPool* GetMemoryPool() const { return _memoryPool; }

            inline void* GetWindow() const { return _window; }
            inline glm::vec2 GetWindowSize() const { return glm::vec2(_desc.WindowDesc.Width, _desc.WindowDesc.Height); }
            inline const std::string& GetWindowTitle() const { return _desc.WindowDesc.Title; }
            inline HWND GetWindowHWND() const { return glfwGetWin32Window((GLFWwindow*)_window); }
            inline types::boolean GetKey(int key) const { return _keys[key]; }

            types::f32 GetDeltaTime() const { return _deltaTime; };

            sApplicationDescriptor* GetDesc() { return &_desc; }

            friend void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
            friend void WindowFocusCallback(GLFWwindow* window, int focused);
            friend void WindowSizeCallback(GLFWwindow* window, int width, int height);
            friend void CursorCallback(GLFWwindow* window, double xpos, double ypos);
            friend void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

        private:
            void CreateAppWindow();
            void CreateContexts();
            void CreateAppManagers();
            void CreateMemoryPool();
            void DestroyAppWindow();
            void DestroyContexts();
            void DestroyAppManagers();
            void DestroyMemoryPool();

            inline glm::vec2 cApplication::GetMonitorSize() const { return glm::vec2(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)); }

            inline void SetKey(const int key, const types::boolean value) { _keys[key] = value; }
            inline void SetWindowFocus(const types::boolean value) { _isFocused = value; }
            inline types::boolean GetWindowFocus() { return _isFocused; }
            inline void SetCursorPosition(const glm::vec2& cursorPosition) { _cursorPosition = cursorPosition; }

            static constexpr types::usize MAX_KEY_COUNT = 256;
            static constexpr types::usize KEY_BUFFER_MASK = 0xFF;

        protected:
            sApplicationDescriptor _desc = {};
            void* _window = nullptr;
            render::cRenderContext* _renderContext = nullptr;
            sound::cSoundContext* _soundContext = nullptr;
            game::mCamera* _camera = nullptr;
            render::mRender* _render = nullptr;
            render::mTexture* _texture = nullptr;
            font::mFont* _font = nullptr;
            sound::mSound* _sound = nullptr;
            fs::mFileSystem* _fileSystem = nullptr;
            physics::mPhysics* _physics = nullptr;
            game::mGameObject* _gameObject = nullptr;
            utils::cMemoryPool* _memoryPool = nullptr;
            types::s32 _keys[MAX_KEY_COUNT] = {};
            types::f32 _deltaTime = 0.0;
            std::chrono::steady_clock::time_point _timepointLast;
            types::boolean _isFocused = types::K_FALSE;
            glm::vec2 _cursorPosition = glm::vec2(0.0f);
        };
    }
}