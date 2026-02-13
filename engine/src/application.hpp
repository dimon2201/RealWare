// application.hpp

#pragma once

#include <windows.h>
#include <chrono>
#include "object.hpp"
#include "capabilities.hpp"
#include "math.hpp"
#include "types.hpp"

struct GLFWwindow;

namespace triton
{
    class cEngine;

    class cWindow : public iObject
    {
        TRITON_OBJECT(cWindow)

        friend void WindowSizeCallback(GLFWwindow* window, int width, int height);

    public:
        explicit cWindow(cContext* context, const std::string& title, types::usize width, types::usize height, types::boolean fullscreen);
        virtual ~cWindow() override final;

        void Resize(const glm::vec2& size);
        void SwapBuffers();
        void PollEvents();

        types::boolean GetRunState() const;
        HWND GetWin32Window() const;
        cVector2 GetCursorPosition() const;
        inline GLFWwindow* GetWindow() const { return _window; }
        inline const std::string& GetTitle() const { return _title; }
        inline cVector2 GetSize() const { return cVector2(_width, _height); }
        inline types::usize GetWidth() const { return _width; }
        inline types::usize GetHeight() const { return _height; }

    private:
        GLFWwindow* _window = nullptr;
        std::string _title = "";
        types::usize _width = 0;
        types::usize _height = 0;
        types::boolean _fullscreen = types::K_FALSE;
    };

    class iApplication : public iObject
    {
        TRITON_OBJECT(iApplication)

    public:
        enum class eMouseButton
        {
            LEFT,
            RIGHT,
            MIDDLE
        };

        explicit iApplication(cContext* context, const sCapabilities* caps);
        virtual ~iApplication() override;

        virtual void Setup() = 0;
        virtual void Stop() = 0;

        void Run();

        inline const sCapabilities* GetCapabilities() const { return _caps; }
        inline cEngine* GetEngine() const { return _engine; }
        inline cWindow* GetWindow() const { return _window; }

    protected:
        const sCapabilities* _caps = nullptr;
        cEngine* _engine = nullptr;
        cWindow* _window = nullptr;
    };
}