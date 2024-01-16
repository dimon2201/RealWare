#include <GLFW/glfw3.h>

#include "application.hpp"
#include "user_input_manager.hpp"
#include "camera_manager.hpp"
#include "render_context.hpp"
#include "sound_context.hpp"

extern realware::core::mUserInput* userInputManager;

namespace realware
{
    namespace core
    {
        cApplication::cApplication(const sApplicationDescriptor& desc)
        {
            m_desc = desc;
            userInputManager->CreateAppWindow(desc.WindowDesc);
            m_renderContext = new render::cOpenGLRenderContext();
            m_soundContext = new sound::cOpenALSoundContext();
        }

        cApplication::~cApplication()
        {
        }

        void cApplication::Run()
        {
            Init();

            while (userInputManager->GetRunState() == K_FALSE)
            {
                m_renderContext->ClearColor(0, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
                m_renderContext->ClearDepth(1.0f);

                Update();
            }

            Free();
        }
    }
}