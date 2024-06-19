#include <GLFW/glfw3.h>
#include "application.hpp"
#include "camera_manager.hpp"
#include "render_context.hpp"
#include "sound_context.hpp"
#include "font_manager.hpp"
#include "sound_manager.hpp"
#include "filesystem_manager.hpp"
#include "physics_manager.hpp"
#include "gameobject_manager.hpp"

namespace realware
{
    namespace core
    {
        static cApplication* s_app = nullptr;

        s32 g_keys[256] = {};
        boolean g_isFocused = K_FALSE;
        sApplicationDescriptor::sWindowDescriptor g_windowDesc;
        glm::vec2 g_cursorPosition = glm::vec2(0.0f);

        void ErrorCallback(int error_code, const char* description)
        {
            std::cout << error_code << " " << description << std::endl;
        }

        void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            if (action == GLFW_PRESS) {
                g_keys[key] = K_TRUE;
            }
            else if (action == GLFW_RELEASE) {
                g_keys[key] = K_FALSE;
            }
        }

        void FocusCallback(GLFWwindow* window, int focused)
        {
            if (focused)
            {
                if (g_isFocused == K_FALSE)
                {
                    g_isFocused = K_TRUE;

                    glm::vec2 size = s_app->GetMonitorSize();
                    g_windowDesc.Width = size.x;
                    g_windowDesc.Height = size.y;
                }
            }
            else
            {
                g_isFocused = K_FALSE;
            }
        }

        void CursorCallback(GLFWwindow* window, double xpos, double ypos)
        {
            g_cursorPosition.x = xpos;
            g_cursorPosition.y = ypos;
        }

        void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
        {
        }

        void WindowSizeCallback(GLFWwindow* window, int width, int height)
        {
            s_app->m_desc.WindowDesc.Width = width;
            s_app->m_desc.WindowDesc.Height = height;
            s_app->GetRenderManager()->ResizeWindow(glm::vec2(width, height));
        }

        cApplication::cApplication(const sApplicationDescriptor& desc)
        {
            s_app = this;
            m_desc = desc;
            CreateAppWindow();
            m_renderContext = new render::cOpenGLRenderContext(this);
            m_soundContext = new sound::cOpenALSoundContext();
            m_cameraManager = new mCamera(this);
            m_textureManager = new mTexture(this, m_renderContext, desc.TextureAtlasWidth, desc.TextureAtlasHeight, desc.TextureAtlasDepth);
            m_renderManager = new mRender(
                this,
                m_renderContext,
                desc.VertexBufferSize,
                desc.IndexBufferSize,
                desc.InstanceBufferSize,
                desc.MaterialBufferSize,
                desc.LightBufferSize,
                1024,
                glm::vec2(desc.WindowDesc.Width, desc.WindowDesc.Height)
            );
            m_fontManager = new mFont(this, m_renderContext);
            m_soundManager = new mSound(this, m_soundContext);
            m_fileSystemManager = new mFileSystem(this);
            m_physicsManager = new mPhysics(this);
            m_gameObjectManager = new mGameObject(this, 1024);
        }

        cApplication::~cApplication()
        {
        }

        void cApplication::CreateAppWindow()
        {
            glfwInit();
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
            glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
            glfwSetErrorCallback(ErrorCallback);

            if (m_desc.WindowDesc.IsFullscreen == K_FALSE)
            {
                m_window = (void*)glfwCreateWindow(m_desc.WindowDesc.Width, m_desc.WindowDesc.Height, m_desc.WindowDesc.Title, nullptr, nullptr);
            }
            else
            {
                glfwWindowHint(GLFW_DECORATED, 0);

                glm::vec2 monitorSize = GetMonitorSize();
                m_desc.WindowDesc.Width = monitorSize.x;
                m_desc.WindowDesc.Height = monitorSize.y;
                m_window = glfwCreateWindow(m_desc.WindowDesc.Width, m_desc.WindowDesc.Height, m_desc.WindowDesc.Title, glfwGetPrimaryMonitor(), nullptr);
            }

            glfwMakeContextCurrent((GLFWwindow*)m_window);
            glfwSwapInterval(1);
            glfwSetKeyCallback((GLFWwindow*)m_window, &KeyCallback);
            glfwSetWindowFocusCallback((GLFWwindow*)m_window, &FocusCallback);
            glfwSetCursorPosCallback((GLFWwindow*)m_window, &CursorCallback);
            glfwSetMouseButtonCallback((GLFWwindow*)m_window, &MouseButtonCallback);
            glfwSetWindowSizeCallback((GLFWwindow*)m_window, &WindowSizeCallback);
        }

        void cApplication::Run()
        {
            Init();

            while (GetRunState() == K_FALSE)
            {
                m_renderContext->ClearColor(0, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
                m_renderContext->ClearDepth(1.0f);

                Update();
            }

            Free();
        }

        void cApplication::PollEvents()
        {
            glfwPollEvents();
        }

        void cApplication::SwapBuffers()
        {
            glfwSwapBuffers((GLFWwindow*)m_window);
        }

        void cApplication::FreeManagers()
        {
            delete m_cameraManager;
            delete m_textureManager;
            delete m_renderManager;
            delete m_fontManager;
            delete m_soundManager;
            delete m_fileSystemManager;
            delete m_physicsManager;
        }

        glm::vec2 cApplication::GetMonitorSize()
        {
            return glm::vec2(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
        }

        glm::vec2 cApplication::GetCursorPosition()
        {
            return glm::vec2(g_cursorPosition.x, g_cursorPosition.y);
        }

        cApplication::eButtonState cApplication::GetMouseKey(int key)
        {
            int state = glfwGetMouseButton((GLFWwindow*)m_window, key);

            if (state == GLFW_PRESS)
            {
                return eButtonState::PRESSED;
            }
            else if (state == GLFW_RELEASE)
            {
                return eButtonState::RELEASED;
            }

            return eButtonState::RELEASED;
        }

        boolean cApplication::GetKey(int key)
        {
            return g_keys[key];
        }
    }
}