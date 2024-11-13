#pragma once

#include "window.h"
#include "renderer/renderer.h"
#include "renderer/render_thread.h"

namespace Dodo {

    struct CommandLineArgs
    {
        const int count = 0;
        const char* const* values = nullptr;

        const char* operator[](size_t index) const
        {
            DODO_ASSERT((index >= 0) && (index < count), "Index out of range!");
            return values[index];
        }
    };

    class Engine
    {
    public:
        Engine(const CommandLineArgs& cmd_line_args);
        ~Engine();

        void run();

    private:
        void OnEvent(Event& e);
        bool OnWindowResize(WindowResizeEvent& e);

        Ref<RenderWindow> m_Window = nullptr;
        bool m_IsRunning = true;
        RenderThread render_thread{};
        Ref<Renderer> m_Renderer = nullptr;
    };

}