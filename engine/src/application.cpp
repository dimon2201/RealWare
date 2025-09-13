#include <iostream>
#include <GLFW/glfw3.h>
#include "application.hpp"
#include "camera_manager.hpp"
#include "render_manager.hpp"
#include "render_context.hpp"
#include "sound_context.hpp"
#include "font_manager.hpp"
#include "sound_manager.hpp"
#include "filesystem_manager.hpp"
#include "physics_manager.hpp"
#include "gameobject_manager.hpp"
#include "texture_manager.hpp"

namespace realware
{
    using namespace game;
    using namespace render;
    using namespace sound;
    using namespace font;
    using namespace physics;
    using namespace fs;
    using namespace types;

    namespace app
    {
        void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            cApplication* app = (cApplication*)glfwGetWindowUserPointer(window);
            key &= app->KEY_BUFFER_MASK;

            if (action == GLFW_PRESS)
                app->SetKey(key, K_TRUE);
            else if (action == GLFW_RELEASE)
                app->SetKey(key, K_FALSE);
        }

        void WindowFocusCallback(GLFWwindow* window, int focused)
        {
            cApplication* app = (cApplication*)glfwGetWindowUserPointer(window);

            if (focused)
            {
                if (app->GetWindowFocus() == K_FALSE)
                    app->SetWindowFocus(K_TRUE);
            }
            else
            {
                app->SetWindowFocus(K_FALSE);
            }
        }

        void WindowSizeCallback(GLFWwindow* window, int width, int height)
        {
            cApplication* app = (cApplication*)glfwGetWindowUserPointer(window);

            app->_desc.WindowDesc.Width = width;
            app->_desc.WindowDesc.Height = height;
            
            app->_render->ResizeWindow(glm::vec2(width, height));
        }

        void CursorCallback(GLFWwindow* window, double xpos, double ypos)
        {
            cApplication* app = (cApplication*)glfwGetWindowUserPointer(window);

            app->SetCursorPosition(glm::vec2(xpos, ypos));
        }

        void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
        {
        }

        cApplication::cApplication(const sApplicationDescriptor* const desc) : _desc(*desc)
        {
            CreateAppWindow();
            CreateContexts();
            CreateAppManagers();
        }

        cApplication::~cApplication()
        {
            DestroyAppManagers();
            DestroyContexts();
            DestroyAppWindow();
        }

        void cApplication::Run()
        {
            Start();

            _timepointLast = std::chrono::high_resolution_clock::now();

            while (GetRunState() == K_FALSE)
            {
                auto currentTime = std::chrono::high_resolution_clock::now();
                std::chrono::duration<f32> elapsed = currentTime - _timepointLast;
                _deltaTime = elapsed.count();
                _timepointLast = currentTime;

                FrameUpdate();

                glfwSwapBuffers((GLFWwindow*)_window);
                glfwPollEvents();
            }

            End();
        }

        void cApplication::CreateAppWindow()
        {
            glfwInit();
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

            if (_desc.WindowDesc.IsFullscreen == K_FALSE)
            {
                _window = (void*)glfwCreateWindow(_desc.WindowDesc.Width, _desc.WindowDesc.Height, _desc.WindowDesc.Title, nullptr, nullptr);
            }
            else
            {
                glfwWindowHint(GLFW_DECORATED, 0);

                glm::vec2 monitorSize = GetMonitorSize();
                _desc.WindowDesc.Width = monitorSize.x;
                _desc.WindowDesc.Height = monitorSize.y;
                _window = glfwCreateWindow(_desc.WindowDesc.Width, _desc.WindowDesc.Height, _desc.WindowDesc.Title, glfwGetPrimaryMonitor(), nullptr);
            }

            glfwSetWindowUserPointer((GLFWwindow*)_window, this);

            if (!_window)
            {
                std::cout << "Error: incompatible GL version!" << std::endl;
                return;
            }

            glfwMakeContextCurrent((GLFWwindow*)_window);

            glfwSwapInterval(1);

            glfwSetKeyCallback((GLFWwindow*)_window, &KeyCallback);
            glfwSetWindowFocusCallback((GLFWwindow*)_window, &WindowFocusCallback);
            glfwSetWindowSizeCallback((GLFWwindow*)_window, &WindowSizeCallback);
            glfwSetCursorPosCallback((GLFWwindow*)_window, &CursorCallback);
            glfwSetMouseButtonCallback((GLFWwindow*)_window, &MouseButtonCallback);
        }

        void cApplication::CreateContexts()
        {
            _renderContext = new cOpenGLRenderContext(this);
            _soundContext = new cOpenALSoundContext();
        }

        void cApplication::CreateAppManagers()
        {
            _camera = new mCamera(this);
            _texture = new mTexture(this, _renderContext);
            _render = new mRender(this, _renderContext);
            _font = new mFont(this, _renderContext);
            _sound = new mSound(this, _soundContext);
            _fileSystem = new mFileSystem(this);
            _physics = new mPhysics(this);
            _gameObject = new mGameObject(this);
        }

        void cApplication::DestroyAppWindow()
        {
            glfwDestroyWindow((GLFWwindow*)_window);
        }

        void cApplication::DestroyContexts()
        {
            delete _soundContext;
            delete _renderContext;
        }

        void cApplication::DestroyAppManagers()
        {
            delete _gameObject;
            delete _physics;
            delete _fileSystem;
            delete _sound;
            delete _font;
            delete _render;
            delete _texture;
            delete _camera;
        }
    }
}