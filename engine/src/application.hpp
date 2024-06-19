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
                s32 Width = 640;
                s32 Height = 480;
                boolean IsFullscreen = K_FALSE;

            };

            sWindowDescriptor WindowDesc;
            s32 TextureAtlasWidth = 1920;
            s32 TextureAtlasHeight = 1080;
            s32 TextureAtlasDepth = 12;
            s32 VertexBufferSize = 65536;
            s32 IndexBufferSize = 65536;
            s32 InstanceBufferSize = 65536;
            s32 MaterialBufferSize = 65536;
            s32 LightBufferSize = 65536;
        };

        class cApplication
        {

        public:
            friend void WindowSizeCallback(GLFWwindow* window, int width, int height);

            enum eButtonState
            {
                PRESSED = 0,
                RELEASED = 1,
                REPEAT = 2
            };

            explicit cApplication(const sApplicationDescriptor& desc);
            ~cApplication();

            virtual void Init() = 0;
            virtual void Update() = 0;
            virtual void Free() = 0;

            void CreateAppWindow();
            void Run();
            void PollEvents();
            void SwapBuffers();
            void FreeManagers();
            glm::vec2 GetMonitorSize();
            glm::vec2 GetCursorPosition();
            boolean GetKey(int key);
            cApplication::eButtonState GetMouseKey(int key);
            inline mCamera* GetCameraManager() { return m_cameraManager; }
            inline mTexture* GetTextureManager() { return m_textureManager; }
            inline mRender* GetRenderManager() { return m_renderManager; }
            inline mFont* GetFontManager() { return m_fontManager; }
            inline mSound* GetSoundManager() { return m_soundManager; }
            inline mFileSystem* GetFileSystemManager() { return m_fileSystemManager; }
            inline mPhysics* GetPhysicsManager() { return m_physicsManager; }
            inline mGameObject* GetGameObjectManager() { return m_gameObjectManager; }
            inline void* GetWindow() { return m_window; }
            inline glm::vec2 GetWindowSize() { return glm::vec2(m_desc.WindowDesc.Width, m_desc.WindowDesc.Height); }
            inline const char* GetWindowTitle() { return m_desc.WindowDesc.Title; }
            inline HWND GetWindowHWND() { return glfwGetWin32Window((GLFWwindow*)m_window); }
            inline boolean GetRunState() { return glfwWindowShouldClose((GLFWwindow*)m_window); }
            inline void FocusWindow() { glfwFocusWindow((GLFWwindow*)m_window); }
            inline void SetWindowPosition(const glm::vec2& position)
            {
                glfwSetWindowPos((GLFWwindow*)m_window, position.x, position.y);
            }

        protected:
            sApplicationDescriptor m_desc;
            void* m_window;
            cRenderContext* m_renderContext;
            cSoundContext* m_soundContext;
            mCamera* m_cameraManager;
            mTexture* m_textureManager;
            mRender* m_renderManager;
            mFont* m_fontManager;
            mSound* m_soundManager;
            mFileSystem* m_fileSystemManager;
            mPhysics* m_physicsManager;
            mGameObject* m_gameObjectManager;

        };
    }
}