#pragma once

#include "RenderCommandQueue.h"
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

        template<typename Command>
        static void Schedule(Command&& command)
        {
            GetSubmissionCommandQueue()->Submit(std::forward<Command>(command));
        }

        static void Shutdown();

    private:
        static RenderCommandQueue* GetSubmissionCommandQueue();
        static RenderCommandQueue* GetRenderCommandQueue();
        static void RenderThreadProc(RenderThread* renderThread);
        static void WaitAndRender(RenderThread* renderThread);
        static void SwapQueues();

        friend class RenderThread;
    };

}