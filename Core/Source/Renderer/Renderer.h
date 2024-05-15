#pragma once

#include "RenderSettings.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDER THREAD ///////////////////////////////////////////////
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

        static void BeginFrame();

        static void Resize(uint32_t width, uint32_t height);

        static void EndFrame();

        static void Dispose();
    };

}