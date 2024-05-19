#pragma once

#include "RenderCommandBuffer.h"
#include "RenderSettings.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // FORWARD DECLARATIONS ////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class RenderThread;

    ////////////////////////////////////////////////////////////////
    // RENDERER ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class Renderer
    {
    public:
        static const RenderSettings& GetSettings();

        static void SetSettings(const RenderSettings& settings);

        static void Init();

        static void RenderThreadProc(RenderThread* renderThread);

        static void WaitAndRender(RenderThread* renderThread);

        template<typename Task>
        static void SubmitTask(Task&& task)
        {
            auto cmd = [](void* memory) {
                auto task = reinterpret_cast<Task*>(memory);
                (*task)();

                task->~Task();
            };

            auto memory = GetSubmissionCommandBuffer()->Allocate(cmd, sizeof(task));
            new (memory) Task(std::forward<Task>(task));
        }

        static void BeginFrame();

        static void Resize(uint32_t width, uint32_t height);

        static void SwapBuffers();

        static void EndFrame();

        static void Dispose();

    private:
        static RenderCommandBuffer* GetSubmissionCommandBuffer();

        static RenderCommandBuffer* GetRenderCommandBuffer();

        static RenderCommandBuffer* GetCommandBuffer(size_t index);
    };

}