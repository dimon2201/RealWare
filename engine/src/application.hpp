#pragma once

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <windows.h>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "types.hpp"

namespace realware
{
    namespace render
    {
        class cRenderContext;
        class cRenderer;
        class mRender;
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

    using namespace render;
    using namespace sound;
    using namespace font;
    using namespace physics;

    namespace core
    {
        class mUserInput;
        class mCamera;
        class mTexture;
        class mFileSystem;
        class mGameObject;

        void WindowSizeCallback(GLFWwindow* window, int width, int height);

        struct sApplicationDescriptor
        {
            struct sWindowDescriptor
            {
                const char* Title = "DefaultWindow";
                u32 Width = 640;
                u32 Height = 480;
                boolean IsFullscreen = K_FALSE;
            };

            sWindowDescriptor WindowDesc;
            u32 TextureAtlasWidth = 1920;
            u32 TextureAtlasHeight = 1080;
            u32 TextureAtlasDepth = 12;
            u32 VertexBufferSize = 65536;
            u32 IndexBufferSize = 65536;
            u32 InstanceBufferSize = 65536;
            u32 MaterialBufferSize = 65536;
            u32 LightBufferSize = 65536;
            u32 MaxMaterialCount = 256;
            u32 MaxGameObjectCount = 4294967295;
        };

        class cApplication
        {
        public:
            explicit cApplication(const sApplicationDescriptor* const desc);
            ~cApplication();

            virtual void Pre() = 0;
            virtual void Update() = 0;
            virtual void Post() = 0;

            void Run();

            inline boolean GetRunState() { return glfwWindowShouldClose((GLFWwindow*)_window); }

            inline mCamera* GetCameraManager() { return _camera; }
            inline mTexture* GetTextureManager() { return _texture; }
            inline mRender* GetRenderManager() { return _render; }
            inline mFont* GetFontManager() { return _font; }
            inline mSound* GetSoundManager() { return _sound; }
            inline mFileSystem* GetFileSystemManager() { return _fileSystem; }
            inline mPhysics* GetPhysicsManager() { return _physics; }
            inline mGameObject* GetGameObjectManager() { return _gameObject; }

            inline void* GetWindow() { return _window; }
            inline glm::vec2 GetWindowSize() { return glm::vec2(_desc.WindowDesc.Width, _desc.WindowDesc.Height); }
            inline const char* GetWindowTitle() { return _desc.WindowDesc.Title; }
            inline HWND GetWindowHWND() { return glfwGetWin32Window((GLFWwindow*)_window); }
            boolean GetKey(int key);

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
            void DestroyAppWindow();
            void DestroyContexts();
            void DestroyAppManagers();

            glm::vec2 GetMonitorSize();

            inline void SetKey(const int key, const boolean value) { _keys[key] = value; }
            inline void SetWindowFocus(const boolean value) { _isFocused = value; }
            inline boolean GetWindowFocus() { return _isFocused; }
            inline void SetCursorPosition(const glm::vec2& cursorPosition) { _cursorPosition = cursorPosition; }

        protected:
            sApplicationDescriptor _desc = {};
            void* _window = nullptr;
            cRenderContext* _renderContext = nullptr;
            cSoundContext* _soundContext = nullptr;
            mCamera* _camera = nullptr;
            mRender* _render = nullptr;
            mTexture* _texture = nullptr;
            mFont* _font = nullptr;
            mSound* _sound = nullptr;
            mFileSystem* _fileSystem = nullptr;
            mPhysics* _physics = nullptr;
            mGameObject* _gameObject = nullptr;
            s32 _keys[256] = {};
            boolean _isFocused = K_FALSE;
            glm::vec2 _cursorPosition = glm::vec2(0.0f);
        };
    }
}